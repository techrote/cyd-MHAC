#pragma once
#include <atomic>
#include <cstdint>

namespace mhac {
constexpr unsigned timing_version=1;
// All times are microseconds; lifetime counters wrap modulo 2^32.
struct AudioTiming {
    uint32_t attempts=0, failed_writes=0, prepare_over_budget=0, long_start_gaps=0;
    uint32_t max_render_us=0, max_prepare_us=0, max_write_us=0, max_start_gap_us=0;
};
// Single audio producer, optional readers. A failed read leaves dst unchanged.
class TimingMailbox {
    std::atomic<uint32_t> seq_{0}, ready_{0};
    std::atomic<uint32_t> words_[8]{{0},{0},{0},{0},{0},{0},{0},{0}};
public:
    void publish(const AudioTiming& s) noexcept {
        const uint32_t w[]={s.attempts,s.failed_writes,s.prepare_over_budget,s.long_start_gaps,
                            s.max_render_us,s.max_prepare_us,s.max_write_us,s.max_start_gap_us};
        seq_.fetch_add(1,std::memory_order_seq_cst);
        for(unsigned i=0;i<8;++i) words_[i].store(w[i],std::memory_order_seq_cst);
        ready_.store(1,std::memory_order_seq_cst);
        seq_.fetch_add(1,std::memory_order_seq_cst);
    }
    bool read(AudioTiming& dst) const noexcept {
        const auto a=seq_.load(std::memory_order_seq_cst);
        if(a&1u) return false;
        uint32_t w[8];
        for(unsigned i=0;i<8;++i) w[i]=words_[i].load(std::memory_order_seq_cst);
        const auto ready=ready_.load(std::memory_order_seq_cst);
        const auto b=seq_.load(std::memory_order_seq_cst);
        if(a!=b || !ready) return false;
        dst={w[0],w[1],w[2],w[3],w[4],w[5],w[6],w[7]};
        return true;
    }
};
// Task-only measurements, not DMA occupancy or physical playback timestamps.
class TimingMeter {
    AudioTiming stats_{};
    uint32_t previous_start_=0;
    const uint32_t budget_;
    bool seen_=false;
    static void maximum(uint32_t& old,uint32_t value) noexcept { if(value>old) old=value; }
public:
    explicit TimingMeter(uint32_t budget_us) noexcept : budget_(budget_us) {}
    void record(uint32_t start,uint32_t rendered,uint32_t prepared,uint32_t end,
                bool write_ok,TimingMailbox& mailbox) noexcept {
        ++stats_.attempts;
        if(!write_ok) ++stats_.failed_writes;
        const uint32_t prepare=prepared-start;
        maximum(stats_.max_render_us,rendered-start);
        maximum(stats_.max_prepare_us,prepare);
        maximum(stats_.max_write_us,end-prepared);
        if(prepare>=budget_) ++stats_.prepare_over_budget;
        if(seen_) {
            const uint32_t gap=start-previous_start_;
            maximum(stats_.max_start_gap_us,gap);
            if(gap>=budget_) ++stats_.long_start_gaps;
        }
        previous_start_=start; seen_=true;
        mailbox.publish(stats_);
    }
};
static_assert(std::atomic<uint32_t>::is_always_lock_free,"32-bit timing mailbox required");
}
