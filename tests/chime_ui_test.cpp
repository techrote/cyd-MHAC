#include "mhac_visual_patch.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
int main() {
    mhac::Exchange io;
    for(unsigned p=0;p<10;++p) for(bool mute:{false,true}) for(bool gate:{false,true}) {
        publishControl(io,Control{1234,3456,gate,mute,p});
        const auto w=io.control.load();
        assert((w&4095u)==1234 && ((w>>12)&4095u)==3456);
        assert(bool(w&(1u<<24))==gate && bool(w&(1u<<25))==mute);
        assert(((w>>26)&7u)==(p<sound_count?p:0));
        assert(bool(w&mhac::modal_opt_in)==(p==6));
    }
    unsigned region=0;
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        const bool yes=soundSelector(x,y); region+=yes;
        assert(yes==(x>=0&&x<28&&y>=216&&y<240));
    }
    assert(region==672 && paired_mode[6]==chime_mode);
    mhac::Engine engine; std::array<int16_t,mhac::block> pcm{};
    auto render=[&](bool gate,bool mute,unsigned n) {
        unsigned peak=0;
        for(unsigned k=0;k<n;++k) {
            publishControl(io,Control{2048,2500,gate,mute,6});
            engine.render(io,pcm.data(),pcm.size());
            for(int v:pcm){assert(v>=-8191&&v<=8191);peak=std::max(peak,unsigned(std::abs(v)));}
        }
        return peak;
    };
    assert(render(true,true,64)==0);
    const auto strike=render(true,false,8), decay=render(true,false,256);
    assert(strike>0 && decay>0 && render(true,false,64)<strike);
    render(false,false,32); const auto restrike=render(true,false,8);
    assert(restrike>0); render(false,true,128); assert(render(false,true,8)==0);
    assert(io.watchdog_trips.load()==0);
    uint64_t idle=0, loud=0;
    for(int y=0;y<240;++y) for(int x=0;x<320;++x) {
        const auto a=chimeShade(x-160,y-120,4095,0,17,2);
        const auto b=chimeShade(x-160,y-120,4095,4095,17,2);
        for(auto s:{a,b}) {assert(s.r>=0&&s.r<=255&&s.g>=0&&s.g<=255&&s.b>=0&&s.b<=255);}
        idle+=a.r+a.g+a.b; loud+=b.r+b.g+b.b;
    }
    assert(loud>idle);
    std::printf("chime UI PASS: 40 control words, %u selector pixels, strike=%u restrike=%u, shade sums=%llu/%llu\n",region,strike,restrike,(unsigned long long)idle,(unsigned long long)loud);
}
