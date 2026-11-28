#pragma once
#include <algorithm>
namespace mhac::visual {
enum class Scale : unsigned { Free, Chromatic, MinorPentatonic };
constexpr unsigned scale_count=3;
constexpr unsigned pitch_steps=36; // AUDIO-005: 110 Hz * 2^(3*x/4095).
constexpr bool pentatonic(unsigned n) noexcept {
    const unsigned p=n%12;
    return p==0 || p==3 || p==5 || p==7 || p==10;
}
constexpr unsigned pitchCoordinate(unsigned n) noexcept {
    return (std::min(n,pitch_steps)*4095u+pitch_steps/2)/pitch_steps;
}
constexpr unsigned mapPitch(unsigned x,Scale scale) noexcept {
    x=std::min(x,4095u);
    if(scale==Scale::Free) return x;
    const unsigned target=x*pitch_steps;
    if(scale==Scale::Chromatic) return pitchCoordinate((target+2047u)/4095u);
    if(scale!=Scale::MinorPentatonic) return x;
    unsigned best=0, error=target;
    for(unsigned n=1;n<=pitch_steps;++n) if(pentatonic(n)) {
        const unsigned candidate=n*4095u;
        const unsigned distance=candidate>target?candidate-target:target-candidate;
        if(distance<error) {best=n;error=distance;}
    }
    return pitchCoordinate(best);
}
}
