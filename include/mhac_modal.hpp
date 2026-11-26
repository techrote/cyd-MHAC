#pragma once
#include <cstdint>

namespace mhac {
// Original two-strike resonator. Single-contact input, not a polyphonic keyboard.
// Each released voice retains its last pitch and brightness; fixed half gain per
// voice bounds the sum. A busy slot fades before reuse (at most 64 samples).
class ModalChime {
    struct Voice {
        uint32_t phase[4]{};
        float amplitude[4]{};
        uint32_t step=0;
        float brightness=0.f;
        unsigned excite=0;
        bool damped=false; // Safety damping is sticky until a fresh strike.
        bool quiet() const noexcept { return amplitude[0]==0.f && excite==0; }
        void start(uint32_t s,float b) noexcept {
            *this=Voice{}; step=s; brightness=b; excite=64;
        }
        template<class Sine>
        float sample(bool inhibit,float fade,Sine sine) noexcept {
            if(inhibit) { damped=true; excite=0; }
            if(quiet()) return 0.f;
            constexpr float ratios[4]={1.f,2.01f,3.97f,5.43f};
            float modes[4];
            for(unsigned j=0;j<4;++j) {
                phase[j]+=uint32_t(float(step)*ratios[j]);
                float& a=amplitude[j];
                if(excite) a+=(1.f-a)*.06f;
                else a*=1.f-(damped?.02f:(.00014f+.00018f*brightness)*float(j+1));
                a*=fade;
                if(a<.00001f) a=0.f;
                modes[j]=a*sine(phase[j]);
            }
            if(excite) --excite;
            const float metal=.65f*brightness;
            return (1.f-metal)*modes[0]+metal*(.5f*modes[1]+.3f*modes[2]+.2f*modes[3]);
        }
    };
    Voice voices_[2]{};
    int current_=-1, pending_=-1;
    unsigned remaining_=0;
    bool held_=false;
public:
    void reset() noexcept { *this=ModalChime{}; }
    template<class Sine>
    float sample(uint32_t step,float brightness,bool gate,Sine sine,bool inhibit=false) noexcept {
        gate=gate && !inhibit;
        if(!gate) {
            if(current_>=0) voices_[current_].excite=0;
            current_=-1; pending_=-1; remaining_=0;
        }
        if(gate && !held_) {
            const int slot=voices_[0].amplitude[0]<=voices_[1].amplitude[0]?0:1;
            if(voices_[slot].quiet()) {
                voices_[slot].start(step,brightness); current_=slot;
            } else {
                pending_=slot; remaining_=64; voices_[slot].excite=0;
            }
        }
        held_=gate;
        if(current_>=0) {
            voices_[current_].step=step;
            voices_[current_].brightness=brightness;
        }
        float sum=0.f;
        for(int j=0;j<2;++j) {
            const float fade=j==pending_?float(remaining_-1)/float(remaining_):1.f;
            sum+=voices_[j].sample(inhibit,fade,sine);
        }
        if(pending_>=0 && --remaining_==0) {
            current_=pending_; pending_=-1;
            voices_[current_].start(step,brightness);
        }
        return .5f*sum;
    }
};
}
