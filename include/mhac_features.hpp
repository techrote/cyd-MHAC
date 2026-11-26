#pragma once
#include <atomic>
#include <cstdint>

namespace mhac {
// Additive, optional analysis contract; existing packed v1 analysis is untouched.
constexpr unsigned feature_version = 1, feature_window = 128;
struct AudioFeatures {
    uint32_t sample_end = 0; // Exclusive rendered-sample clock, modulo 2^32.
    uint16_t peak = 0, mean_abs = 0, mean_delta = 0, onset = 0;
    uint8_t crossings = 0;
};
class FeatureMailbox {
    std::atomic<uint32_t> seq_{0}, stamp_{0}, levels_{0}, motion_{0}, flags_{0};
public:
    // One audio producer. All fields atomic; reader never waits or observes torn data.
    void publish(const AudioFeatures& f) noexcept {
        seq_.fetch_add(1, std::memory_order_seq_cst);
        stamp_.store(f.sample_end, std::memory_order_seq_cst);
        levels_.store(uint32_t(f.peak) | (uint32_t(f.mean_abs)<<16), std::memory_order_seq_cst);
        motion_.store(uint32_t(f.mean_delta) | (uint32_t(f.onset)<<16), std::memory_order_seq_cst);
        flags_.store(256u | f.crossings, std::memory_order_seq_cst);
        seq_.fetch_add(1, std::memory_order_seq_cst);
    }
    // False leaves caller's previous snapshot unchanged. Zero timestamp is valid after wrap.
    bool read(AudioFeatures& dst) const noexcept {
        const auto a=seq_.load(std::memory_order_seq_cst);
        const auto t=stamp_.load(std::memory_order_seq_cst);
        const auto l=levels_.load(std::memory_order_seq_cst);
        const auto m=motion_.load(std::memory_order_seq_cst);
        const auto f=flags_.load(std::memory_order_seq_cst);
        const auto b=seq_.load(std::memory_order_seq_cst);
        if(a!=b || (a&1u) || !(f&256u)) return false;
        dst={t,uint16_t(l),uint16_t(l>>16),uint16_t(m),uint16_t(m>>16),uint8_t(f)};
        return true;
    }
};
// Fixed sample windows independent of render partition and UI cadence. All units PCM16.
// Mean delta is a rough brightness/activity proxy, NOT a spectral-band measurement.
// Onset is positive window-to-window mean absolute amplitude change, NOT a note detector.
class FeatureAccumulator {
    uint32_t clock_, sum_=0, delta_=0;
    unsigned count_=0, peak_=0, crossings_=0, previous_mean_=0;
    int previous_=0, sign_=0;
public:
    explicit FeatureAccumulator(uint32_t initial_clock=0) noexcept : clock_(initial_clock) {}
    void push(int16_t sample, FeatureMailbox& mailbox) noexcept {
        const int s=sample, d=s-previous_;
        const unsigned mag=unsigned(s<0?-s:s), diff=unsigned(d<0?-d:d);
        sum_+=mag; delta_+=diff;
        if(mag>peak_) peak_=mag;
        const int sign=(s>0)-(s<0);
        if(sign) { if(sign_ && sign!=sign_) ++crossings_; sign_=sign; }
        previous_=s; ++clock_;
        if(++count_!=feature_window) return;
        const unsigned mean=sum_/feature_window;
        const unsigned rise=mean>previous_mean_?mean-previous_mean_:0;
        mailbox.publish({clock_,uint16_t(peak_),uint16_t(mean),uint16_t(delta_/feature_window),uint16_t(rise),uint8_t(crossings_)});
        previous_mean_=mean; count_=peak_=crossings_=sum_=delta_=0;
    }
};
static_assert(std::atomic<uint32_t>::is_always_lock_free,"32-bit feature mailbox required");
}
