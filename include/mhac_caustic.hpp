#pragma once
#include <algorithm>
namespace mhac::visual {
struct CausticShade { int r,g,b; };
inline int causticTri(int phase) noexcept {
    const unsigned p=unsigned(phase)&255u;
    return int(p<128?p*2:(255-p)*2);
}
// Inputs: screen x/y, cursor px/py (320x240), 8-bit peak, phase,
// tick and timbre. Artistic light sheets, not an acoustic simulation.
inline CausticShade causticShade(int x,int y,int px,int py,int a,int ph,int t,int depth) noexcept {
    const int u=x-160,w=y-120,dx=x-px,dy=y-py;
    const int bend=w*w/96;
    const int left=causticTri(u*3+bend+t*2+ph);
    const int right=causticTri(u*3-bend-t*2+depth);
    const int ridge=std::max(0,std::min(left,right)-120);
    // A gesture-centred curved sheet crosses the fixed lens. Narrow bright
    // intersections and a dim continuous filament distinguish the layers.
    const int sheet=causticTri(dy*5+dx*dx/128-t*3+ph);
    const int filament=std::max(0,sheet-192);
    const int knot=std::min(ridge,filament);
    const int glow=ridge*(80+a)/128;
    const int flare=knot*(64+depth)/128;
    return {std::min(255,6+glow/3+filament/2+flare*2),
            std::min(255,10+glow+filament/3+flare),
            std::min(255,22+glow*3/4+filament*2+flare/2)};
}
}
