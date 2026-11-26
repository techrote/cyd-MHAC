#pragma once
#include <algorithm>
#include <cstdlib>
#include "mhac_audio.hpp"
namespace mhac::visual {
constexpr unsigned mode_count=8, sound_count=7, chime_mode=7;
constexpr unsigned paired_mode[sound_count]={0,1,4,3,2,5,chime_mode};
struct Control { unsigned x=2048,y=2048; bool gate=false,mute=true; unsigned patch=0; };
static_assert(mhac::contract_version==1 && mhac::patch_version==1 &&
    mhac::patch_extension_version==2 && mhac::extended_patch_count==6 &&
    mhac::modal_extension_version==3 && mhac::modal_patch_index==6 &&
    mhac::performance_patch_count==sound_count,"patch contract pin required");
constexpr bool soundSelector(int x,int y) noexcept {
    return x>=0 && x<int(sound_count*4) && y>=216 && y<240;
}
inline void publishControl(mhac::Exchange& io,const Control& c) noexcept {
    if(c.patch==mhac::modal_patch_index) io.publish_modal(c.x,c.y,c.gate,c.mute);
    else io.publish_patch(c.x,c.y,c.gate,c.mute,static_cast<mhac::Patch>(c.patch));
}
struct ChimeShade { int r,g,b; };
// Artistic concentric metal plates, not a measurement of individual modal amplitudes.
// Peak is measured block PCM; phase and time drive decorative interference.
inline ChimeShade chimeShade(int dx,int dy,unsigned brightness,unsigned peak,unsigned phase,unsigned tick) noexcept {
    const int a=std::min(255u,peak>>4), y=int(std::min(brightness,4095u)>>4);
    const int u=std::abs(dx), v=std::abs(dy)*5/4;
    const int radius=std::max(u,v)+std::min(u,v)/3;
    const int wobble=int((phase+tick)&7u)-3;
    int light=0;
    for(int j=0;j<4;++j) {
        const int dist=std::abs(radius-(23+j*24+wobble));
        const int band=std::max(0,6-dist);
        const int gain=j==0?255-y/2:45+y/(j+1);
        light+=band*gain/6;
    }
    const int spokes=((u+v*2+int(tick&31u))&15)<2?16:0;
    const int glow=std::min(255,light*(28+a)/283);
    return {7+glow*(100+y/2)/255+spokes/2,10+glow*160/255+spokes/2,17+glow*(225-y/3)/255+spokes};
}
}
