#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
using L=GestureLoop;
static void setup(PerformanceSurface& ui,unsigned mapping,unsigned style,uint32_t start) {
    uint32_t t=start;
    auto tap=[&](int x,int y){ui.sample(true,x,y,t+=8);ui.sample(false,0,0,t+=8);};
    tap(310,10);
    for(unsigned i=0;i<mapping;++i) tap(270,230);
    for(unsigned i=0;i<style;++i) tap(184,10);
    // Common absolute recording start for all combinations.
    t=start+512;
    tap(160,10);
    for(unsigned i=0;i<40;++i) ui.sample(true,40+int(i)*5,190-int(i)*3,t+=8);
    ui.sample(false,0,0,t+=8);
    assert(ui.loopState()==L::State::Playing);
}
int main() {
    unsigned blocks=0,active=0,rests=0,changes=0;
    for(uint32_t start:{0u,0xfffffd00u}) for(unsigned mapping=0;mapping<xy_map_count;++mapping)
    for(unsigned style=0;style<L::style_count;++style) {
        PerformanceSurface a,b; setup(a,mapping,style,start); setup(b,mapping,style,start);
        mhac::Engine ea,eb; mhac::Exchange ia,ib;
        std::array<int16_t,mhac::block> pa{},pb{};
        for(unsigned i=0;i<400;++i) {
            const uint32_t t=start+856+i*5;
            const unsigned phase=i%40;
            const bool down=phase>=4 && phase<12;
            const bool visual=(i/40)%2==0;
            const unsigned oldMode=b.mode(); const auto oldPalette=b.palette();
            a.sample(false,0,0,t);
            // Drag into the mute corner after the selector is captured.
            b.sample(down,phase==4?(visual?10:230):310,phase==4?(visual?10:230):10,t);
            if(phase==4) {
                assert(visual?b.mode()==(oldMode+1)%mode_count:b.palette()==nextPalette(oldPalette));
                ++changes;
            } else assert(b.mode()==oldMode && b.palette()==oldPalette);
            assert(b.loopState()==L::State::Playing);
            const auto ca=a.effective(t),cb=b.effective(t);
            assert(ca.x==cb.x && ca.y==cb.y && ca.patch==cb.patch && ca.mute==cb.mute && ca.gate==cb.gate);
            assert(!cb.mute);
            publishControl(ia,ca); publishControl(ib,cb);
            ea.render(ia,pa.data(),pa.size()); eb.render(ib,pb.data(),pb.size());
            assert(pa==pb); assert(ia.snapshot()==ib.snapshot());
            assert(!ia.watchdog_trips.load() && !ib.watchdog_trips.load());
            ++blocks; active+=cb.gate; rests+=!cb.gate;
        }
        const uint32_t t=start+856+400*5;
        b.sample(true,10,10,t); b.expire(t+101);
        assert(b.control().mute && b.loopState()==L::State::Idle && !b.effective(t+101).gate);
        a.sample(true,230,230,t); a.observeWatchdog(1);
        assert(a.control().mute && a.loopState()==L::State::Idle && !a.effective(t).gate);
    }
    // A fresh mute remains immediate, and a fresh performance touch still cancels.
    PerformanceSurface u,v; setup(u,0,0,0); setup(v,0,0,0);
    u.sample(true,310,10,900); assert(u.control().mute && u.loopState()==L::State::Idle);
    v.sample(true,110,110,900); assert(v.loopState()==L::State::Idle && !v.effective(900).gate);
    assert(active && rests);
    std::printf("live visual PASS: %u exact PCM blocks (%u samples), %u active, %u rest, %u selector changes; surface=%zu B\n",blocks,blocks*unsigned(mhac::block),active,rests,changes,sizeof(PerformanceSurface));
}
