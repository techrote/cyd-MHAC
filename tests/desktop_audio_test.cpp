#include "mhac_desktop_audio.hpp"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace mhac;
using namespace mhac::visual;
int main() {
    for(unsigned p=0;p<extended_patch_count;++p) {
        DesktopAudio audio; Exchange reference; Engine engine;
        std::vector<unsigned char> bytes(2053,0x5a);
        std::array<int16_t,1025> expected{};
        unsigned active=0;
        for(unsigned k=0;k<200;++k) {
            const unsigned n=1+(k*83)%1025;
            const bool gate=k<150;
            audio.exchange.publish_patch(1400,3000,gate,!gate,Patch(p));
            reference.publish_patch(1400,3000,gate,!gate,Patch(p));
            for(unsigned i=0;i<n;) {
                const unsigned m=std::min(block,n-i);
                engine.render(reference,expected.data()+i,m); i+=m;
            }
            bytes.assign(bytes.size(),0x5a);
            audio.render(bytes.data()+1,2*n+1);
            assert(bytes.front()==0x5a && bytes[2*n+1]==0 && bytes[2*n+2]==0x5a);
            assert(!std::memcmp(bytes.data()+1,expected.data(),2*n));
            active+=audio.last_peak.load()!=0;
            assert(audio.last_peak.load()<=8191);
        }
        assert(audio.callbacks.load()==200 && audio.active_callbacks.load()==active && active>0);
        assert(audio.last_peak.load()==0 && audio.exchange.watchdog_trips.load()==0);
    }
    DesktopAudio stalled;
    std::array<int16_t,block> pcm{};
    stalled.exchange.publish(2048,2048,true,false);
    for(unsigned n=0;n<160;++n) { stalled.exchange.publish(2048,2048,true,false); stalled.render(pcm.data(),sizeof pcm); }
    assert(stalled.last_peak.load()>0);
    for(unsigned n=0;n<200;++n) stalled.render(pcm.data(),sizeof pcm);
    assert(stalled.exchange.watchdog_trips.load()==1 && stalled.last_peak.load()==0);
    std::puts("desktop audio PASS: six patches, 1200 variable/unaligned callbacks, exact reference PCM, independent watchdog silence");
}
