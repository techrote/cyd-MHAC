#include "mhac_audio.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace mhac;
static void check(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
// Isolate the harmonic term with a synthetic oscillator (not an audition).
static void colour_test() {
    unsigned comparisons=0;
    for(float stored:{0.f,.125f,.5f,.875f,1.f}) for(bool counter:{false,true}) {
        Quotation q; uint32_t p=0; constexpr uint32_t step=20000000;
        for(unsigned i=0;i<3*Quotation::history_samples;++i) {
            p+=step; unsigned calls=0;
            const auto harmonic=[&](uint32_t) { return calls++==0?0.f:1.f; };
            const float y=i<Quotation::history_samples?stored:1.f;
            const bool transformed=counter && i<2*Quotation::history_samples;
            const float v=q.sample(p,step,y,0.f,true,false,harmonic,transformed);
            check(v>=0.f && v<=.24501f,"colour output bound");
            if(i<Quotation::history_samples) check(v==0.f,"premature recall");
            if(i>=Quotation::history_samples && i%Quotation::interval==Quotation::interval-1) {
                const bool answer=bool((i/Quotation::interval)&1u);
                const float raw=i<2*Quotation::history_samples?stored:1.f;
                const float target=transformed?(answer?.5f+.5f*raw:.25f*raw):raw;
                check(calls==2 && std::abs(v-.245f*target)<.00002f,"recalled colour target or raw history changed");
                ++comparisons;
            }
        }
    }
    std::cout<<"colour_targets="<<comparisons<<'\n';
}
static void publish(Exchange& io,unsigned mode,unsigned x,unsigned y,bool gate,bool mute) {
    if(mode==8) io.publish_counterquotation(x,y,gate,mute);
    else if(mode==7) io.publish_quotation(x,y,gate,mute);
    else if(mode==6) io.publish_modal(x,y,gate,mute);
    else io.publish_patch(x,y,gate,mute,static_cast<Patch>(mode));
}
static std::vector<int16_t> render(unsigned mode,unsigned chunk) {
    Engine e; Exchange io; std::vector<int16_t> pcm(6*rate);
    constexpr unsigned notes[]={683,1365,1593,2048,1138,2730,2275,1365};
    constexpr unsigned depths[]={4095,2400,3600,1000,3900,2000,3200,4095};
    for(unsigned off=0;off<pcm.size();) {
        const unsigned t=off/block, k=(t/47)%8;
        const bool gate=t>=16 && t<970 && !(t>=560 && t<600), mute=t<16 || t>=970;
        const unsigned x=notes[k], y=depths[k];
        publish(io,mode,x,y,gate,mute);
        const unsigned n=std::min<unsigned>(block,pcm.size()-off);
        for(unsigned i=0;i<n;) { const unsigned take=std::min(chunk,n-i); e.render(io,pcm.data()+off+i,take); i+=take; }
        off+=n;
    }
    check(io.watchdog_trips.load()==0,"unexpected watchdog");
    unsigned active=0;
    for(auto v:pcm) { check(std::abs(int(v))<=8191,"PCM bounds"); active+=v!=0; }
    check(active>rate,"silent instrument");
    check(std::all_of(pcm.begin(),pcm.begin()+16*block,[](int16_t v){return v==0;}),"initial mute");
    check(std::all_of(pcm.end()-block,pcm.end(),[](int16_t v){return v==0;}),"final mute");
    return pcm;
}
static void write_pcm(const std::string& path,const std::vector<int16_t>& pcm) {
    std::ofstream out(path,std::ios::binary);
    for(auto v:pcm) { const auto u=uint16_t(v); out.put(char(u&255u)); out.put(char(u>>8)); }
    out.close(); check(bool(out),"PCM write");
}
int main(int argc,char** argv) {
    check(argc==1 || (argc==3 && std::string(argv[1])=="--render"),"usage: test [--render PREFIX]");
    if(argc==1) colour_test();
    unsigned frames=0;
    for(unsigned mode=0;mode<9;++mode) {
        auto pcm=render(mode,128);
        check(pcm==render(mode,31) && pcm==render(mode,97),"partition mismatch");
        frames+=2*pcm.size();
        if(argc==3) write_pcm(std::string(argv[2])+std::to_string(mode)+".pcm",pcm);
    }
    std::cout<<"partition_comparisons="<<frames<<" quotation_bytes="<<sizeof(Quotation)<<" engine_bytes="<<sizeof(Engine)<<'\n';
}
