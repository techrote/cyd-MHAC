#include "mhac_audio.hpp"
// Unmodified audio-002 fixture, namespaced only in this translation unit.
// Source commit 1fec8bc503806513dbfb71103d6b1dda97a90e3f.
#define mhac mhac_audio_002_reference
#include "reference/audio_002.hpp"
#undef mhac
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>

int main() {
    mhac::Exchange io;
    mhac::Engine e;
    mhac_audio_002_reference::Exchange ref;
    mhac_audio_002_reference::Engine r;
    std::array<int16_t,mhac::block> a{},b{};
    uint64_t ha=14695981039346656037ull,hb=ha;
    unsigned total=0,nonzero=0;
    auto compare=[&](unsigned n,bool record) {
        e.render(io,a.data(),n);r.render(ref,b.data(),n);
        for(unsigned j=0;j<n;++j) {
            if(a[j]!=b[j]) {
                std::fprintf(stderr,"legacy PCM mismatch at sample %u: candidate=%d reference=%d\n",total+j,int(a[j]),int(b[j]));
                std::exit(1);
            }
            assert(a[j]>=-8191 && a[j]<=8191);
            if(record) {ha=(ha^uint16_t(a[j]))*1099511628211ull;hb=(hb^uint16_t(b[j]))*1099511628211ull;nonzero+=a[j]!=0;}
        }
        assert(io.snapshot()==ref.snapshot());
        assert(io.watchdog_trips.load()==ref.watchdog_trips.load());
        total+=n;
    };
    auto publish=[&](unsigned x,unsigned y,bool gate,bool mute) {
        io.publish(x,y,gate,mute);ref.publish(x,y,gate,mute);
        assert(io.control.load()==ref.control.load());
    };
    compare(mhac::block,false);
    for(unsigned k=0;k<1000;++k) {
        publish(k*29,k*17,true,false);compare(mhac::block,true);
    }
    assert(nonzero>120000);
    std::printf("legacy trajectory candidate=%016llx reference=%016llx nonzero=%u\n",(unsigned long long)ha,(unsigned long long)hb,nonzero);
    for(unsigned k=0;k<110;++k) compare(mhac::block,false);
    assert(io.watchdog_trips.load()==1);
    for(auto s:a) assert(s==0);
    publish(4095,4095,true,false);compare(mhac::block,false);
    for(auto s:a) assert(s==0);
    publish(4095,4095,false,false);compare(7,false);
    publish(2048,1024,true,false);compare(mhac::block,false);
    for(unsigned k=0;k<300;++k) {
        publish((k*619u)&4095u,(k*997u)&4095u,k%17!=0,k%31==0);
        compare(1+k%mhac::block,false);
    }
    publish(0,0,false,true);
    for(unsigned k=0;k<100;++k) compare(mhac::block,false);
    for(auto s:a) assert(s==0);
    std::printf("PASS: %u exact predecessor PCM samples, packed analysis and watchdog equivalence\n",total);
}
