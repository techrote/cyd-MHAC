#pragma once
#include <algorithm>
#include <cstdlib>
namespace mhac::visual {
struct GlyphShade { int r,g,b; };
inline unsigned glyphHash(unsigned x,unsigned y,unsigned salt) noexcept {
    unsigned h=x*0x9e3779b9u ^ y*0x85ebca6bu ^ salt;
    h^=h>>16; h*=0x7feb352du; return h^(h>>15);
}
// Shared lattice edges use identical coordinates on either side of a tile.
// No temporal reseeding: the topology remains stable during a performance.
inline bool glyphEdge(unsigned x,unsigned y,bool vertical) noexcept {
    return (glyphHash(x,y,vertical?0xa512u:0x67d3u)&7u)<5u;
}
inline unsigned glyphInk(int x,int y) noexcept {
    const unsigned cx=unsigned(x)>>4,cy=unsigned(y)>>4;
    const int u=x&15,v=y&15;
    const bool midX=u==7||u==8,midY=v==7||v==8;
    if(midX&&midY) return 3;
    if(midY&&glyphEdge(cx+unsigned(u>8),cy,false)) return 1;
    if(midX&&glyphEdge(cx,cy+unsigned(v>8),true)) return 1;
    const int d=std::abs(2*u-15)+std::abs(2*v-15);
    const unsigned motif=glyphHash(cx,cy,0x3bd9u)&3u;
    if(motif==0 && d>=10&&d<=12) return 2;
    if(motif==1 && d==8) return 2;
    if(motif==2 && (u==4||u==11) && v>=4&&v<=11) return 2;
    if(motif==3 && (v==4||v==11) && u>=4&&u<=11) return 2;
    return 0;
}
inline int glyphTri(int phase) noexcept {
    const unsigned p=unsigned(phase)&255u;
    return int(p<128?p*2:(255-p)*2);
}
// Artistic routing lattice, not decoded symbols or measured propagation.
// The cursor originates a travelling illumination; Y sets its spatial density.
inline GlyphShade glyphShade(int x,int y,int px,int py,int a,int ph,int t,int depth) noexcept {
    const unsigned ink=glyphInk(x,y);
    const int d=std::abs(x-px)+std::abs(y-py);
    const int halo=std::max(0,112-d);
    if(!ink) return {4+halo/24,7+halo/16,15+halo/10};
    const int wave=glyphTri(d*(2+depth/32)-t*4+ph);
    const int pulse=std::max(0,wave-144);
    const int light=pulse*(64+a)/128+halo/2;
    if(ink==1) return {std::min(255,16+light/3),std::min(255,30+light),std::min(255,45+light)};
    if(ink==2) return {std::min(255,22+light),std::min(255,18+light/2),std::min(255,38+light/3)};
    return {std::min(255,42+light),std::min(255,62+light),std::min(255,65+light/2)};
}
}
