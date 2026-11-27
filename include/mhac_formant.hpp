#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace mhac {
// Original harmonic-domain vowel model, not a measured vocal tract.
// Two moving spectral windows, eight harmonics; highest harmonic is 7040 Hz
// at the engine's maximum 880 Hz fundamental (24 kHz sample rate).
inline float formant_window(float hz,float center,float inverse_width) noexcept {
    return std::max(0.f,1.f-std::fabs(hz-center)*inverse_width);
}
inline float formant_gain(unsigned harmonic,float hz,float y) noexcept {
    return (harmonic==1u?.28f:0.f)
        +.75f*formant_window(hz,350.f+650.f*y,1.f/420.f)
        +.55f*formant_window(hz,2300.f-1000.f*y,1.f/720.f);
}
template<class Sine>
float formant_reed(uint32_t phase,float fundamental,float y,Sine sine) noexcept {
    float sum=0.f, gain=.20f;
    for(unsigned h=1;h<=8;++h) {
        const float g=formant_gain(h,fundamental*float(h),y);
        if(g>0.f) { sum+=g*sine(phase*h); gain+=g; }
    }
    return sum/gain;
}
}
