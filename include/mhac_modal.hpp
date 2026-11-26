#pragma once
#include <cstdint>

namespace mhac {
// Four damped sinusoidal modes: original struck-instrument approximation.
// Called only when the modal patch contributes to the output crossfade.
class ModalChime {
    uint32_t phase_[4]{};
    float amplitude_[4]{};
    unsigned excite_=0;
    bool held_=false;
public:
    void reset() noexcept { *this=ModalChime{}; }
    template<class Sine>
    float sample(uint32_t step,float brightness,bool gate,Sine sine) noexcept {
        if(gate && !held_) excite_=64;
        held_=gate;
        if(!gate) excite_=0;
        constexpr float ratios[4]={1.f,2.01f,3.97f,5.43f};
        float modes[4];
        for(unsigned j=0;j<4;++j) {
            phase_[j]+=uint32_t(float(step)*ratios[j]);
            float& a=amplitude_[j];
            if(excite_) a+=(1.f-a)*.06f;
            else a*=1.f-(.00014f+.00018f*brightness)*float(j+1);
            if(a<.00001f) a=0.f;
            modes[j]=a*sine(phase_[j]);
        }
        if(excite_) --excite_;
        const float metal=.65f*brightness;
        return (1.f-metal)*modes[0]+metal*(.5f*modes[1]+.3f*modes[2]+.2f*modes[3]);
    }
};
}
