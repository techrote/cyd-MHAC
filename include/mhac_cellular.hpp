#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
namespace mhac::visual {
// Excitable visual medium, UI thread only. Not an acoustic simulation.
class CellularField {
    static constexpr int w=40,h=30;
    std::array<uint8_t,w*h> cells_{},next_{};
    uint32_t last_=0, remainder_=0;
    bool active_=false;
    void seed(int x,int y) noexcept {
        for(int j=std::max(0,y-1);j<=std::min(h-1,y+1);++j)
            for(int i=std::max(0,x-1);i<=std::min(w-1,x+1);++i)
                if(cells_[j*w+i]==0) cells_[j*w+i]=1;
    }
    void evolve(unsigned peak) noexcept {
        const unsigned threshold=peak>=1024u?1u:2u;
        for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
            const int i=y*w+x;
            const unsigned v=cells_[i];
            if(v) { next_[i]=uint8_t(v==15?0:v+1); continue; }
            const unsigned n=unsigned(cells_[y*w+(x+w-1)%w]==1)+unsigned(cells_[y*w+(x+1)%w]==1)
                +unsigned(cells_[((y+h-1)%h)*w+x]==1)+unsigned(cells_[((y+1)%h)*w+x]==1);
            next_[i]=uint8_t(n>=threshold?1:0);
        }
        cells_.swap(next_);
    }
public:
    static constexpr unsigned period=50, max_steps=4;
    void advance(uint32_t now,bool selected,unsigned x,unsigned y,bool gate,unsigned peak) noexcept {
        if(!selected) { active_=false; return; }
        if(!active_) {
            cells_.fill(0); active_=true; last_=now; remainder_=0;
            seed(10,9); seed(29,20);
        }
        const uint32_t elapsed=std::min(now-last_,period*max_steps)+remainder_;
        last_=now; remainder_=elapsed%period;
        for(unsigned i=0;i<elapsed/period;++i) {
            if(gate) seed(int(std::min(x,4095u)*39u/4095u),29-int(std::min(y,4095u)*29u/4095u));
            evolve(peak);
        }
    }
    uint8_t cell(int x,int y) const noexcept {
        return cells_[std::clamp(y,0,h-1)*w+std::clamp(x,0,w-1)];
    }
    struct Shade { int r,g,b; };
    Shade shade(int x,int y) const noexcept {
        const int v=cell(x/8,y/8);
        if(!v) return {5,9,16};
        const int a=16-v;
        const int edge=((x&7)==0||(y&7)==0)?3:4;
        return {6+(v==1?176:a*3)*edge/4,10+a*a*edge/4,18+a*12*edge/4};
    }
};
static_assert(sizeof(CellularField)<=2432,"bounded visual state budget");
}
