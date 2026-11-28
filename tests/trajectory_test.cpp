#include "mhac_trajectory.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
using namespace mhac::visual;
int main() {
    unsigned profiles=0; uint64_t points=0;
    uint32_t seed=0x12345678u;
    auto next=[&](unsigned n){seed=seed*1664525u+1013904223u;return int((seed>>8)%n);};
    for(int k=0;k<2000;++k) {
        int x0=next(320),y0=next(240),x1=next(320),y1=next(240);
        if(k%7==0) y1=y0;
        if(k%11==0) x1=x0;
        if(k%13==0) {x1=x0;y1=y0;}
        std::array<uint8_t,320*240> full{},split{},reference{};
        trajectorySegment(x0,y0,x1,y1,0,240,[&](int x,int y){assert(x>=0&&x<320&&y>=0&&y<240);++full[y*320+x];});
        for(int row=0;row<240;row+=8)
            trajectorySegment(x0,y0,x1,y1,row,8,[&](int x,int y){assert(x>=0&&x<320&&y>=row&&y<row+8);++split[y*320+x];});
        // Independent full-segment reference; no clipping arithmetic.
        const bool rev=y0>y1 || (y0==y1&&x0>x1);
        const int sx=rev?x1:x0,sy=rev?y1:y0,ex=rev?x0:x1,ey=rev?y0:y1;
        const int n=std::max(std::abs(ex-sx),ey-sy);
        for(int i=0;n&&i<=n;++i) {
            if((rev?n-i:i)%6>=3) continue;
            const int d=ex-sx;
            const int x=sx+(d<0?-1:1)*((2*std::abs(d)*i+n)/(2*n));
            const int y=sy+(ey-sy)*i/n;
            ++reference[y*320+x]; ++points;
        }
        assert(full==split&&full==reference); ++profiles;
    }
    std::printf("trajectory profiles=%u plotted=%llu strip equivalence PASS\n",profiles,static_cast<unsigned long long>(points));
}
