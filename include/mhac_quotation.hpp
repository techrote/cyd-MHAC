#pragma once
#include <cstdint>
namespace mhac {
// Creative exaptation: four samples of gesture history, not a research protocol.
// Capture cadence is audio-domain; no allocation, frame clock or external input.
class Quotation {
    struct Mark { uint32_t step=0; float timbre=0.f; };
    Mark marks_[4]{}, target_{};
    unsigned cursor_=0, filled_=0, until_=0;
    uint32_t phase_=0, step_=0;
    float timbre_=0.f, blend_=0.f;
public:
    static constexpr unsigned interval=3000, history_samples=4*interval;
    void reset() noexcept { *this=Quotation{}; }
    template<class Sine>
    float sample(uint32_t phase,uint32_t step,float y,float carrier,bool capture,bool inhibit,Sine sine) noexcept {
        if(inhibit) { filled_=0; cursor_=0; until_=0; }
        if(capture && !inhibit) {
            if(until_==0) {
                if(filled_==0) {
                    target_={step,y};
                    // A rapid rearm can overlap the invalidated voice's fade.
                    // Do not reset its audible phase; only seed a silent voice.
                    if(blend_==0.f) { step_=step; timbre_=y; phase_=phase-step; }
                }
                if(filled_==4) { target_=marks_[cursor_]; heard_=true; }
                marks_[cursor_]={step,y}; cursor_=(cursor_+1u)&3u;
                if(filled_<4) ++filled_;
                until_=interval;
            }
            --until_;
        }
        // No audible quotation until a full history interval has elapsed.
        if(inhibit) heard_=false;
        const bool ready=heard_;
        blend_+=((ready && !inhibit?.70f*y:0.f)-blend_)*.005f;
        if(blend_<.00001f) blend_=0.f;
        const int32_t difference=int32_t(target_.step)-int32_t(step_);
        step_=uint32_t(int32_t(step_)+difference/64);
        timbre_+=(target_.timbre-timbre_)*.005f;
        phase_+=step_;
        if(blend_==0.f) return carrier;
        const float q=sine(phase_)*(1.f-.35f*timbre_)+sine(phase_*2u)*(.35f*timbre_);
        return carrier+(q-carrier)*blend_;
    }
private:
    bool heard_=false;
};
}
