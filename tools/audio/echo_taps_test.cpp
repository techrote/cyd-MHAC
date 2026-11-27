#include "mhac_audio.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace mhac;
static void check(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
#ifndef MHAC_ECHO_PARENT
static void tap_test() {
    std::array<int16_t,echo_samples> ring{};
    for(unsigned i=0;i<ring.size();++i) ring[i]=int16_t(i+1);
    unsigned checks=0;
    for(unsigned pos=0;pos<ring.size();++pos) {
        unsigned index=(pos+echo_samples-576)%echo_samples;
        for(unsigned valid:{0u,575u,576u,577u,echo_samples}) {
            check(echo_early_read(ring.data(),pos,valid)==(valid<576?0.f:float(ring[index])/32768.f),"tap wrap/validity"); ++checks;
        }
    }
    // Independent impulse witness: a write is read precisely 576 samples later.
    ring.fill(0); unsigned pos=0,valid=0;
    for(unsigned i=0;i<3*echo_samples;++i) {
        const float want=i==576?0.5f:0.f;
        check(echo_early_read(ring.data(),pos,valid)==want,"tap impulse timing"); ++checks;
        ring[pos]=i==0?16384:0; pos=(pos+1)%echo_samples; valid=std::min(valid+1,echo_samples);
    }
    // Invalidation deliberately leaves nonzero old data in the buffer.
    ring.fill(1234); pos=0; valid=0;
    for(unsigned i=0;i<echo_samples*2;++i) {
        check(echo_early_read(ring.data(),pos,valid)==0.f,"stale tap data"); ++checks;
        ring[pos]=0; pos=(pos+1)%echo_samples; valid=std::min(valid+1,echo_samples);
    }
    std::cout<<"tap_checks="<<checks<<'\n';
}
#endif
static void publish(Exchange& io,unsigned mode,unsigned x,unsigned y,bool gate,bool mute) {
    if(mode==8) io.publish_counterquotation(x,y,gate,mute);
    else if(mode==7) io.publish_quotation(x,y,gate,mute);
    else if(mode==6) io.publish_modal(x,y,gate,mute);
    else io.publish_patch(x,y,gate,mute,static_cast<Patch>(mode));
}
static std::vector<int16_t> perform(unsigned mode,unsigned chunk,bool dry) {
    Engine e; Exchange io; std::vector<int16_t> pcm(rate*6);
    constexpr unsigned notes[]={683,1365,1593,2048,1138,2730,2275,1365};
    constexpr unsigned depths[]={4095,2400,3600,1000,3900,2000,3200,4095};
    for(unsigned off=0;off<pcm.size();) {
        const unsigned t=off/block,k=(t/47)%8;
        const bool gate=t>=32 && t<970 && !(t>=560 && t<600),mute=t<32 || t>=970;
        if(t<710 || t>=830) publish(io,mode,notes[k],dry?0u:depths[k],gate,mute);
        const unsigned n=std::min<unsigned>(block,pcm.size()-off);
        for(unsigned i=0;i<n;) { const unsigned take=std::min(chunk,n-i); e.render(io,pcm.data()+off+i,take); i+=take; }
        off+=n;
    }
    check(io.watchdog_trips.load()==1,"lease trip");
    check(std::all_of(pcm.begin(),pcm.begin()+32*block,[](int16_t v){return v==0;}),"initial silence");
    check(std::all_of(pcm.begin()+820*block,pcm.begin()+970*block,[](int16_t v){return v==0;}),"stale gate rearmed");
    check(std::all_of(pcm.end()-block,pcm.end(),[](int16_t v){return v==0;}),"final silence");
    unsigned active=0; for(auto v:pcm) { check(std::abs(int(v))<=8191,"PCM bound"); active+=v!=0; }
    check(active>rate,"silent mode"); return pcm;
}
static void raw(const std::string& path,const std::vector<int16_t>& pcm) {
    std::ofstream f(path,std::ios::binary); for(auto v:pcm) { const auto u=uint16_t(v); f.put(char(u&255u)); f.put(char(u>>8)); }
    f.close(); check(bool(f),"PCM write");
}
int main(int argc,char** argv) {
    check(argc<=2,"usage: echo_taps_test [output-prefix]");
#ifndef MHAC_ECHO_PARENT
    tap_test();
#endif
    unsigned comparisons=0;
    for(unsigned mode=0;mode<10;++mode) {
        const bool dry=mode==9; const unsigned m=dry?5:mode;
        auto pcm=perform(m,128,dry);
        check(pcm==perform(m,31,dry) && pcm==perform(m,97,dry),"partition PCM");
        comparisons+=2*pcm.size();
        if(argc==2) raw(std::string(argv[1])+std::to_string(mode)+".pcm",pcm);
    }
    std::cout<<"partition_comparisons="<<comparisons<<" engine_bytes="<<sizeof(Engine)<<'\n';
}
