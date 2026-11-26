#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
namespace mhac::visual {
// UI-owned history of OBSERVED windows, not a full PCM history or spectrum.
class Trace {
public:
    static constexpr unsigned columns=80, bin_samples=1024, stale_ms=150;
    struct Bin { uint8_t amplitude=0, activity=0, onset=0, seen=0; };
    struct Shade { int r,g,b; };
private:
    std::array<Bin,columns> bins_{};
    uint32_t stamp_=0, wall_=0, remainder_=0;
    unsigned head_=0;
    bool valid_=false;
    static uint8_t level(unsigned x,unsigned scale) noexcept {
        return uint8_t(std::min(x,scale)*255u/scale);
    }
public:
    template<class Features> bool observe(const Features& f,uint32_t now) noexcept {
        if(valid_) {
            const uint32_t dt=f.sample_end-stamp_;
            if(dt==0 || dt>=0x80000000u) return false;
            const uint32_t elapsed=dt+remainder_;
            const unsigned advance=elapsed/bin_samples;
            remainder_=elapsed%bin_samples;
            if(advance>=columns) { bins_.fill({}); head_=0; }
            else for(unsigned i=0;i<advance;++i) { head_=(head_+1)%columns; bins_[head_]={}; }
        }
        valid_=true; stamp_=f.sample_end; wall_=now;
        auto& b=bins_[head_]; b.seen=1;
        b.amplitude=std::max(b.amplitude,level(f.mean_abs,8192));
        b.activity=std::max(b.activity,level(f.mean_delta,8192));
        b.onset=std::max(b.onset,level(f.onset,4096));
        return true;
    }
    bool fresh(uint32_t now) const noexcept { return valid_ && uint32_t(now-wall_)<=stale_ms; }
    Bin at(unsigned x) const noexcept { return bins_[(head_+1+std::min(x,columns-1))%columns]; }
    Shade shade(int x,int y) const noexcept {
        const auto b=at(unsigned(std::clamp(x,0,319))/4);
        if(!b.seen) return {5,8,15};
        const unsigned row=unsigned(std::clamp(y,0,239));
        const unsigned band=row/80, d=row%80;
        const unsigned value=band==0?b.amplitude:band==1?b.activity:b.onset;
        const unsigned h=(value*72u+254u)/255u;
        if(d>=76) return {24,34,42};
        if(d+ h<76 || h==0) return {9,16,24};
        const int v=32+int(value)*223/255;
        if(band==0) return {v/5,v,v*3/4};
        if(band==1) return {v/2,v/3,v};
        return {v,v*2/3,v/8};
    }
};
}
