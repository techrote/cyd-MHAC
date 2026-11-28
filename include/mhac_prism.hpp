#pragma once
#include <algorithm>
#include <cstdlib>
namespace mhac::visual {
struct PrismShade { int r,g,b; };
// Distance to a power-of-two lattice boundary, including negative coordinates.
inline int prismFold(int x,unsigned period) noexcept {
    const unsigned p=unsigned(x)&(period-1u);
    return int(std::min(p,period-p));
}
// Three nested folded scales. Geometry follows the gesture; phase and peak
// only illuminate the facets. This is decorative, not an audio spectrum.
inline PrismShade prismShade(int x,int y,int px,int py,int a,int ph,int t,int depth) noexcept {
    const int dx=x-px,dy=y-py,u=dx+dy,v=dx-dy;
    const int U=prismFold(u,128),V=prismFold(v,128);
    const int mU=prismFold(u,64),mV=prismFold(v,64);
    const int fU=prismFold(u,32),fV=prismFold(v,32);
    const int edge=std::max(0,7-std::abs(U-V));
    const int seam=std::max(0,6-std::min(U,V));
    const int middle=std::max(0,6-std::abs(mU-mV));
    const int fine=std::max(0,5-std::abs(fU-fV));
    const bool split=((unsigned(u)^unsigned(v))&64u)!=0;
    const bool refine=split && ((unsigned(u)^unsigned(v))&32u)!=0;
    const int pulse=prismFold((std::abs(dx)+std::abs(dy))*(2+depth/32)-t*3+ph,256);
    const int light=32+pulse+a/2;
    const int facet=split?U+V:128-U-V;
    const int glass=facet*light/128;
    const int gold=edge*light/8+seam*light/16;
    const int blue=middle*light/12+(refine?fine*light/16:0);
    return {std::min(255,7+glass/(split?5:2)+gold),
            std::min(255,11+glass/3+gold/2+blue/2),
            std::min(255,23+glass/(split?1:3)+blue)};
}
}
