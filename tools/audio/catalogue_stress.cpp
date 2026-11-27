// AUDIO-025: all session sound pairs, PCM/DAC/feature partition equivalence.
#include "mhac_audio.hpp"
#include "mhac_dac.hpp"
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace mhac;
using Clock=std::chrono::steady_clock;
constexpr unsigned modes=counterquotation_session_index+1, dwell=160, tail=48;
static void check(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
static void publish(Exchange& e,unsigned mode,unsigned x,unsigned y,bool gate,bool mute) {
    if(mode==counterquotation_session_index) e.publish_counterquotation(x,y,gate,mute);
    else if(mode==quotation_patch_index) e.publish_quotation(x,y,gate,mute);
    else if(mode==modal_patch_index) e.publish_modal(x,y,gate,mute);
    else e.publish_patch(x,y,gate,mute,static_cast<Patch>(mode));
}
static bool equal(const AudioFeatures& a,const AudioFeatures& b) {
    return a.sample_end==b.sample_end && a.peak==b.peak && a.mean_abs==b.mean_abs &&
        a.mean_delta==b.mean_delta && a.onset==b.onset && a.crossings==b.crossings;
}
int main() {
    std::array<std::vector<long long>,modes> times;
    for(auto& t:times) t.reserve(2*modes*dwell);
    uint64_t hash=14695981039346656037ull, samples=0, windows=0;
    std::array<uint64_t,modes> diagonal{};
    for(unsigned a=0;a<modes;++a) for(unsigned b=0;b<modes;++b) {
        Engine full, split; Exchange i,j; DacQuantizer da,db;
        std::array<int16_t,block> p{},q{};
        std::array<uint16_t,block> packed{};
        uint64_t pairhash=14695981039346656037ull;
        unsigned active_a=0,active_b=0;
        for(unsigned k=0;k<2*dwell+tail;++k) {
            const unsigned mode=k<dwell?a:b;
            const bool mute=k>=2*dwell,gate=!mute && !(k>=2*dwell-16 && k%4==0);
            const unsigned x=(k*137u+500u)&4095u,y=3072u+((k*311u)&1023u);
            publish(i,mode,x,y,gate,mute); publish(j,mode,x,y,gate,mute);
            const auto start=Clock::now();
            full.render(i,p.data(),block);
            for(unsigned n=0;n<block;++n) packed[n]=da.pack(p[n]);
            const auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-start).count();
            if(!mute && (k%dwell)>=100 && k<2*dwell-16) times[mode].push_back(elapsed);
            const unsigned cut=1+(k*37u)% (block-2);
            split.render(j,q.data(),cut);
            split.render(j,q.data()+cut,1);
            split.render(j,q.data()+cut+1,block-cut-1);
            check(p==q,"PCM partition mismatch");
            for(unsigned n=0;n<block;++n) {
                check(std::abs(int(p[n]))<=8191,"PCM bound");
                check(packed[n]==db.pack(q[n]),"DAC partition mismatch");
                pairhash=(pairhash^uint16_t(p[n]))*1099511628211ull;
                pairhash=(pairhash^packed[n])*1099511628211ull;
                if(k<dwell) active_a+=p[n]!=0;
                else if(!mute) active_b+=p[n]!=0;
                if(k==2*dwell+tail-1) check(p[n]==0 && packed[n]==0x8000,"mute did not settle");
            }
            AudioFeatures fa{},fb{};
            check(i.features.read(fa)&&j.features.read(fb)&&equal(fa,fb),"feature partition mismatch");
            check(fa.sample_end==(k+1)*block,"feature clock mismatch");
            check(da.residual()==db.residual(),"DAC residual mismatch");
            ++windows; samples+=block;
        }
        check(active_a>0 && active_b>0,"silent sound segment");
        check(i.watchdog_trips.load()==0 && j.watchdog_trips.load()==0,"unexpected watchdog");
        if(a==b) diagonal[a]=pairhash;
        hash=(hash^pairhash)*1099511628211ull;
    }
    for(unsigned a=0;a<modes;++a) for(unsigned b=0;b<a;++b) check(diagonal[a]!=diagonal[b],"identical sound signatures");
    std::cout<<"pairs="<<modes*modes<<" samples="<<samples<<" feature_windows="<<windows<<" hash="<<std::hex<<hash<<std::dec<<'\n';
    std::cout<<"mode,observations,p50_ns,p99_ns\n";
    for(unsigned m=0;m<modes;++m) {
        auto& t=times[m]; std::sort(t.begin(),t.end());
        check(!t.empty(),"no cost observations");
        std::cout<<m<<','<<t.size()<<','<<t[t.size()/2]<<','<<t[t.size()*99/100]<<'\n';
    }
}
