#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
namespace mhac::visual {
// Persistent scalar field, one byte per 4x4 LCD cell. UI-thread only.
class InkField {
    static constexpr int w=80,h=60;
    std::array<uint8_t,w*h> ink_{};
    uint32_t last_=0, remainder_=0;
    int px_=0,py_=0;
    bool initialized_=false, connected_=false;
    void stamp(int x,int y,int strength,int radius) noexcept {
        for(int j=std::max(0,y-radius);j<=std::min(h-1,y+radius);++j)
            for(int i=std::max(0,x-radius);i<=std::min(w-1,x+radius);++i) {
                const int d=std::abs(i-x)+std::abs(j-y);
                if(d<=radius) {
                    const auto v=uint8_t(strength*(radius+1-d)/(radius+1));
                    auto& p=ink_[j*w+i]; p=std::max(p,v);
                }
            }
    }
public:
    void advance(uint32_t now,unsigned x,unsigned y,bool gate,unsigned peak) noexcept {
        const uint32_t dt=initialized_?now-last_:0;
        initialized_=true; last_=now;
        // Linear 25 ms decay, with fractional time retained and long gaps bounded.
        const uint32_t elapsed=std::min(dt,1100u)+remainder_;
        const unsigned loss=std::min(255u,(elapsed/25u)*6u);
        remainder_=elapsed%25u;
        if(loss) for(auto& p:ink_) p=uint8_t(p>loss?p-loss:0);
        if(dt>100 || !gate) connected_=false;
        if(!gate) return;
        const int nx=int(std::min(x,4095u)*79u/4095u);
        const int ny=59-int(std::min(y,4095u)*59u/4095u);
        const unsigned a=std::min(peak,4095u);
        const int strength=128+int(a*127u/4095u),radius=1+int(a*2u/4095u);
        int ix=connected_?px_:nx,iy=connected_?py_:ny;
        const int dx=std::abs(nx-ix),dy=-std::abs(ny-iy),sx=ix<nx?1:-1,sy=iy<ny?1:-1;
        int error=dx+dy;
        for(;;) {
            stamp(ix,iy,strength,radius);
            if(ix==nx && iy==ny) break;
            const int e=2*error;
            if(e>=dy) {error+=dy;ix+=sx;}
            if(e<=dx) {error+=dx;iy+=sy;}
        }
        px_=nx;py_=ny;connected_=true;
    }
    void disconnect() noexcept { connected_=false; }
    uint8_t cell(int x,int y) const noexcept {
        return ink_[std::clamp(y,0,h-1)*w+std::clamp(x,0,w-1)];
    }
    struct Shade {int r,g,b;};
    Shade shade(int x,int y) const noexcept {
        const int v=cell(x/4,y/4);
        return {5+v*v/255,12+v*3/4,22+v/2+std::min(v,255-v)};
    }
};
}
