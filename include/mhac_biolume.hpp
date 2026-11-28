#pragma once
#include <algorithm>
namespace mhac::visual {
struct BiolumeShade { int r,g,b; };
inline int biolumeTri(int phase) noexcept {
    const unsigned p=unsigned(phase)&255u;
    return int(p<128?p*2:(255-p)*2);
}
// Pigment-gated optical sheets. pigment is DiffusionField::shade().g;
// peak is aggregate PCM, phase/tick drive decorative (not measured) wavefronts.
inline BiolumeShade biolumeShade(int x,int y,int px,int py,int pigment,int peak,int phase,int tick) noexcept {
    const int d=std::clamp((pigment-9)*4/3,0,255);
    const int dx=x-px,dy=y-py;
    const int bend=dy*dy/96;
    const int sheet=biolumeTri(dx*4+bend+phase+tick*2+d);
    const int cross=biolumeTri(dy*5-dx*dx/128-tick*3+phase);
    const int rim=std::max(0,sheet-172)*3;
    const int intersection=std::min(rim,std::max(0,cross-176)*3);
    const int body=d*d/255;
    const int light=(d*rim/255)*(64+peak)/319;
    const int flare=d*intersection/255;
    return {std::min(255,4+body/5+light/2+flare),
            std::min(255,8+body/2+light+flare/2),
            std::min(255,16+d/2+light/2+flare)};
}
}
