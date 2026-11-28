#pragma once
#include <algorithm>
namespace mhac::visual {
// Nominal target gains, not measured envelopes. Positions quantized to the
// 4-pixel shading grid from AUDIO-039's 12000-sample motif.
struct RhythmMark { int x, width; unsigned gain; };
constexpr RhythmMark rhythmMark(unsigned i,unsigned band) noexcept {
    band=std::min(band,3u);
    switch(i) {
    case 0: return {8,8,255};
    case 1: return {16,8,255-85*band};
    case 2: return {24,4,(255*9*band+30)/60}; // displaced .45*Y
    case 3: return {32,8,204};
    case 4: return {44,8,(255*13*(6-band)+60)/120}; // .65*(1-.5*Y)
    default: return {0,0,0};
    }
}
struct RhythmLegendColor { int r,g,b; };
constexpr RhythmLegendColor rhythmLegend(int x,unsigned band) noexcept {
    for(unsigned i=0;i<5;++i) {
        const auto m=rhythmMark(i,band);
        if(x<m.x || x>=m.x+m.width || !m.gain) continue;
        const int g=int(m.gain);
        if(i==2) return {18+g,12+g*3/4,8+g/4}; // amber ghost
        return {18+g/2,18+g*3/4,std::min(255,18+g)};
    }
    return {12,20,28};
}
}
