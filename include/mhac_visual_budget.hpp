#pragma once
#include <cstdint>
namespace mhac::visual {
// Frame-boundary policy: task preparation pressure, not measured DAC underruns.
class VisualBudget {
    uint32_t attempts_=0, over_=0, failed_=0;
    unsigned step_=2, clean_=0;
    bool seen_=false;
public:
    unsigned step() const noexcept { return step_; }
    void finish(uint32_t frame_us,uint32_t strip_us,bool valid,
                uint32_t attempts,uint32_t over,uint32_t failed) noexcept {
        const bool progressing=valid && seen_ && attempts!=attempts_;
        const bool pressure=valid && (over!=over_ || failed!=failed_);
        if(valid) { attempts_=attempts; over_=over; failed_=failed; seen_=true; }
        if(pressure || frame_us>85000 || strip_us>5000) {
            step_=4; clean_=0;
        } else if(step_==4 && progressing && frame_us<55000 && strip_us<3000) {
            if(++clean_>=60) { step_=2; clean_=0; }
        } else clean_=0;
    }
};
static_assert(sizeof(VisualBudget)<=32,"bounded visual budget state");
}
