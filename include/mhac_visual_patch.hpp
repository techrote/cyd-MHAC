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
    const unsigned p=std::min(peak,4095u);
    // Piecewise compression reveals the quiet tail without masking full strikes.
    const int a=int(p<=512u?p/4u:128u+(p-512u)/28u);
    const int y=int(std::min(brightness,4095u)>>4);
    const int u=std::abs(dx), v=std::abs(dy)*5/4;
    const int radius=std::max(u,v)+std::min(u,v)/3;
    const int facet=std::abs(u-v);
    const int wobble=int((phase+tick)&7u)-3;
    // Rings are 24 pixels apart and each halo has radius 10: only the nearest contributes.
    const int j=std::clamp((radius-23-wobble+12)/24,0,3);
    const int dist=std::abs(radius-(23+j*24+wobble));
    const int gain=j==0?255-y/2:70+y/(j+1);
    const int rim=std::max(0,3-dist)*gain/3;
    const int halo=std::max(0,10-dist)*gain/10;
    // Faceted etching is decorative, driven by aggregate audio, not per-voice data.
    const int etch=((facet+int(tick&31u))&15)<2?1:0;
    const int glow=std::min(255,(rim*(12+a)+halo*a/3)/255);
    const int grain=etch*(3+a/24);
    return {std::min(255,4+glow*(90+y/2)/255+grain),
            std::min(255,7+glow*(220-y/3)/255+grain),
            std::min(255,12+glow*(245-y/2)/255+grain)};
}
}
