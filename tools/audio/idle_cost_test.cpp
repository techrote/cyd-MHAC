#include "mhac_audio.hpp"
#include <array>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
using namespace mhac;
static void check(bool ok,const char* why) { if(!ok) { std::cerr<<why<<'\n'; std::exit(1); } }
static void publish(Exchange& io,unsigned mode,unsigned x,unsigned y,bool gate,bool mute) {
    if(mode==8) io.publish_counterquotation(x,y,gate,mute);
    else if(mode==7) io.publish_quotation(x,y,gate,mute);
    else if(mode==6) io.publish_modal(x,y,gate,mute);
    else io.publish_patch(x,y,gate,mute,static_cast<Patch>(mode));
}
static void put(std::ostream& o,uint32_t v,unsigned n) { for(unsigned i=0;i<n;++i) o.put(char((v>>(8*i))&255u)); }
static bool same(const AudioFeatures& a,const AudioFeatures& b) {
    return a.sample_end==b.sample_end && a.peak==b.peak && a.mean_abs==b.mean_abs && a.mean_delta==b.mean_delta && a.onset==b.onset && a.crossings==b.crossings;
}
static void exercise(std::ostream* out) {
    unsigned frames=0, windows=0;
    for(unsigned mode=0;mode<9;++mode) for(unsigned next=0;next<9;++next) {
        Engine e,f; Exchange a,b; std::array<int16_t,block> p{},q{};
        unsigned first_active=0, rearm_active=0;
        // Initial mute; sustained sound (including quotation refill); release;
        // idle morph to another patch; rearm; producer stalls; stale renewal;
        // observed release/rearm; explicit mute. Control updates at block edges.
        for(unsigned k=0;k<720;++k) {
            const unsigned m=k<315?mode:next;
            const unsigned x=(k*137+mode*113)&4095u, y=(k*319+next*127)&4095u;
            const bool gate=(k>=32 && k<200)||(k>=400 && k<600)||(k>=601 && k<650);
            const bool mute=k<32 || k>=650;
            if(k<450 || k>=570) { publish(a,m,x,y,gate,mute); publish(b,m,x,y,gate,mute); }
            e.render(a,p.data(),block);
            const unsigned cut=1+(k*37)%(block-1);
            f.render(b,q.data(),cut); f.render(b,q.data()+cut,block-cut);
            check(p==q,"partition PCM");
            for(auto v:p) { check(std::abs(int(v))<=8191,"PCM bound"); if(out) put(*out,uint16_t(v),2); }
            if(k>=32 && k<200) first_active+=std::count_if(p.begin(),p.end(),[](int16_t v){return v!=0;});
            if(k>=601 && k<650) rearm_active+=std::count_if(p.begin(),p.end(),[](int16_t v){return v!=0;});
            AudioFeatures fa{},fb{};
            check(a.features.read(fa)&&b.features.read(fb)&&same(fa,fb),"feature partition");
            check(fa.sample_end==(k+1)*block,"feature timestamp");
            check((a.snapshot()&0x00fff000u)==(b.snapshot()&0x00fff000u),"phase partition");
            check(a.watchdog_trips.load()==b.watchdog_trips.load(),"watchdog partition");
            if(k<32 || (k>=308 && k<400 && mode!=6) || (k>=560 && k<600) || k>=710) {
                check(std::all_of(p.begin(),p.end(),[](int16_t v){return v==0;}),"expected silence");
            }
            if(out) { put(*out,a.snapshot(),4); put(*out,fa.sample_end,4); put(*out,fa.peak,2); put(*out,fa.mean_abs,2); put(*out,fa.mean_delta,2); put(*out,fa.onset,2); put(*out,fa.crossings,1); put(*out,a.watchdog_trips.load(),4); }
            frames+=block; ++windows;
        }
        check(first_active>0 && rearm_active>0,"missing sound or failed rearm");
        check(a.watchdog_trips.load()==1,"one stalled-producer trip");
    }
    std::cout<<"pairs=81 frames="<<frames<<" feature_windows="<<windows<<'\n';
}
static void benchmark() {
    using Clock=std::chrono::steady_clock;
    std::cout<<"mode,idle_or_active,median_ns_per_block,checksum\n";
    for(unsigned m=0;m<9;++m) for(bool active:{false,true}) {
        std::array<long long,7> times{}; uint64_t sum=0;
        for(auto& elapsed:times) {
            Engine e; Exchange io; std::array<int16_t,block> p{};
            publish(io,m,1777,3890,active,!active);
            for(unsigned k=0;k<150;++k) { publish(io,m,1777,3890,active,!active); e.render(io,p.data(),block); }
            const auto t=Clock::now();
            for(unsigned k=0;k<10000;++k) {
                publish(io,m,1777,3890,active,!active); e.render(io,p.data(),block);
                sum+=uint16_t(p[k%block]);
            }
            elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-t).count()/10000;
        }
        std::sort(times.begin(),times.end());
        std::cout<<m<<','<<(active?"active":"idle")<<','<<times[3]<<','<<sum<<'\n';
    }
}
int main(int argc,char** argv) {
    if(argc==2 && std::string(argv[1])=="--bench") { benchmark(); return 0; }
    check(argc==1 || argc==2,"usage: idle_cost_test [output.bin | --bench]");
    if(argc==2) { std::ofstream o(argv[1],std::ios::binary); check(bool(o),"open output"); exercise(&o); o.close(); check(bool(o),"write output"); }
    else exercise(nullptr);
}
