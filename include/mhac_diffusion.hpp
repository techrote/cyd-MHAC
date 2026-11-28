#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
namespace mhac::visual {
// Original two-species Gray-Scott-inspired visual field, never on audio thread.
class DiffusionField {
    static constexpr int w=32,h=24,one=4096;
    using Grid=std::array<uint32_t,w*h>;
    Grid cells_[2]{};
    unsigned front_=0;
    uint32_t last_=0,remainder_=0;
    bool active_=false, initialized_=false, stroke_=false;
    int8_t last_x_=0,last_y_=0;
    static int u(uint32_t v) noexcept { return int(v&8191u); }
    static int v(uint32_t c) noexcept { return int(c>>13); }
    static uint32_t pack(int a,int b) noexcept {
        return uint32_t(std::clamp(a,0,one))|(uint32_t(std::clamp(b,0,one))<<13);
    }
    void seed(int x,int y) noexcept {
        auto& g=cells_[front_];
        for(int dy=-2;dy<=2;++dy) for(int dx=-2;dx<=2;++dx) if(dx*dx+dy*dy<=5) {
            const int i=((y+dy+h)%h)*w+(x+dx+w)%w;
            g[i]=pack(std::min(u(g[i]),2048),std::max(v(g[i]),1536));
        }
    }
    // Stamp at most nine points along a held gesture, with <=4 cells per interval.
    // No interpolation across lift, mute, or visual deselection.
    void paint(int x,int y) noexcept {
        if(stroke_) {
            const int dx=x-last_x_,dy=y-last_y_;
            const int span=std::max(dx<0?-dx:dx,dy<0?-dy:dy);
            const int count=(span+3)/4;
            for(int i=1;i<count;++i) seed(last_x_+dx*i/count,last_y_+dy*i/count);
        }
        seed(x,y); last_x_=int8_t(x); last_y_=int8_t(y); stroke_=true;
    }
    void evolve(unsigned peak) noexcept {
        const auto& g=cells_[front_]; auto& n=cells_[front_^1u];
        const int feed=112+int(std::min(peak,4095u)/128u),kill=246;
        for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
            const int i=y*w+x,a=u(g[i]),b=v(g[i]);
            const auto l=g[y*w+(x+w-1)%w],r=g[y*w+(x+1)%w],t=g[((y+h-1)%h)*w+x],d=g[((y+1)%h)*w+x];
            const int la=u(l)+u(r)+u(t)+u(d)-4*a,lb=v(l)+v(r)+v(t)+v(d)-4*b;
            const int reaction=((a*b/one)*b)/one;
            n[i]=pack(a+la*655/one-reaction+feed*(one-a)/one,
                      b+lb*328/one+reaction-(feed+kill)*b/one);
        }
        front_^=1u;
    }
public:
    static constexpr unsigned period=12,max_steps=4;
    void advance(uint32_t now,bool selected,unsigned x,unsigned y,bool gate,unsigned peak) noexcept {
        if(!selected) { active_=false; stroke_=false; return; }
        if(!active_) {
            active_=true; last_=now; remainder_=0; stroke_=false;
            if(!initialized_) {
                front_=0; cells_[0].fill(pack(one,0)); initialized_=true;
                seed(8,6); seed(23,17);
            }
        }
        if(!gate) stroke_=false;
        const uint32_t elapsed=std::min(now-last_,period*max_steps)+remainder_;
        last_=now; remainder_=elapsed%period;
        for(unsigned i=0;i<elapsed/period;++i) {
            if(gate) paint(int(std::min(x,4095u)*31u/4095u),23-int(std::min(y,4095u)*23u/4095u));
            evolve(peak);
        }
    }
    uint32_t cell(int x,int y) const noexcept {
        return cells_[front_][std::clamp(y,0,h-1)*w+std::clamp(x,0,w-1)];
    }
    struct Shade { int r,g,b; };
    Shade shade(int x,int y) const noexcept {
        x=std::clamp(x,0,319); y=std::clamp(y,0,239);
        const int ix=x/10,iy=y/10,fx=x%10,fy=y%10;
        const auto& g=cells_[front_];
        const int j=iy*w+ix,k=iy*w+(ix+1)%w,m=((iy+1)%h)*w+ix,n=((iy+1)%h)*w+(ix+1)%w;
        const int b=((v(g[j])*(10-fx)+v(g[k])*fx)*(10-fy)+(v(g[m])*(10-fx)+v(g[n])*fx)*fy)/100;
        const int a=std::clamp(b*255/1900,0,255),crest=std::max(0,a-145);
        return {4+a/8+crest*2,9+a*3/4,18+a-crest*2};
    }
};
static_assert(sizeof(DiffusionField)<=6176,"bounded visual state budget");
}
