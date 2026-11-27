#pragma once
#include <algorithm>
#include <cstdlib>
namespace mhac::visual {
// Screen-bounded endpoints; emits a strip-local dashed segment in recorded order.
// Canonicalize geometry by ascending Y (then X), retaining dash direction.
// Clip the integer sample-index range before rasterizing; no full-line rescan.
template<class Plot>
inline void trajectorySegment(int x0,int y0,int x1,int y1,int row,int rows,Plot plot) noexcept {
    const bool reverse=y0>y1 || (y0==y1 && x0>x1);
    if(reverse) { std::swap(x0,x1); std::swap(y0,y1); }
    if(y1<row || y0>=row+rows) return;
    const int dx=x1-x0,dy=y1-y0,ax=std::abs(dx),n=std::max(ax,dy);
    if(n==0) return;
    int first=0,last=n;
    if(dy) {
        if(row>y0) first=((row-y0)*n+dy-1)/dy;
        if(row+rows<=y1) last=((row+rows-y0)*n+dy-1)/dy-1;
    }
    for(int i=first;i<=last;++i) {
        const int along=reverse?n-i:i;
        if(along%6>=3) continue;
        const int x=x0+(dx<0?-1:1)*((ax*i+n/2)/n);
        plot(x,y0+dy*i/n);
    }
}
}
