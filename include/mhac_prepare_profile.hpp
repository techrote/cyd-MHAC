#pragma once
#include <atomic>
#include <cstdint>

namespace mhac {
// Independent of timing v1. Counts are lifetime, modulo 2^32; max is lifetime.
constexpr uint32_t prepare_profile_budget_us=5334;
struct PrepareProfile {
    uint32_t attempts=0, bins[5]{}, max_us=0, failed=0;
};
class PrepareProfileMailbox {
    std::atomic<uint32_t> seq_{0}, ready_{0};
    std::atomic<uint32_t> words_[8]{{0},{0},{0},{0},{0},{0},{0},{0}};
public:
    void publish(const PrepareProfile& p) noexcept {
        seq_.fetch_add(1,std::memory_order_seq_cst);
        words_[0].store(p.attempts,std::memory_order_seq_cst);
        for(unsigned i=0;i<5;++i) words_[i+1].store(p.bins[i],std::memory_order_seq_cst);
        words_[6].store(p.max_us,std::memory_order_seq_cst);
        words_[7].store(p.failed,std::memory_order_seq_cst);
        ready_.store(1,std::memory_order_seq_cst);
        seq_.fetch_add(1,std::memory_order_seq_cst);
    }
    bool read(PrepareProfile& p) const noexcept {
        const auto a=seq_.load(std::memory_order_seq_cst);
        if(a&1u) return false;
        PrepareProfile v{};
        v.attempts=words_[0].load(std::memory_order_seq_cst);
        for(unsigned i=0;i<5;++i) v.bins[i]=words_[i+1].load(std::memory_order_seq_cst);
        v.max_us=words_[6].load(std::memory_order_seq_cst);
        v.failed=words_[7].load(std::memory_order_seq_cst);
        const auto ready=ready_.load(std::memory_order_seq_cst);
        if(a!=seq_.load(std::memory_order_seq_cst) || !ready) return false;
        p=v; return true;
    }
};
class PrepareProfiler {
    PrepareProfile stats_{};
public:
    static constexpr unsigned bin(uint32_t us) noexcept {
        return us<1334?0:us<2667?1:us<4001?2:us<prepare_profile_budget_us?3:4;
    }
    void record(uint32_t start,uint32_t prepared,bool ok,PrepareProfileMailbox& out) noexcept {
        const uint32_t elapsed=prepared-start;
        ++stats_.attempts;
        ++stats_.bins[bin(elapsed)];
        if(elapsed>stats_.max_us) stats_.max_us=elapsed;
        if(!ok) ++stats_.failed;
        // First, every 128 attempts, and immediately on terminal write failure.
        if(stats_.attempts==1 || (stats_.attempts&127u)==0 || !ok) out.publish(stats_);
    }
};
static_assert(std::atomic<uint32_t>::is_always_lock_free,"32-bit profile mailbox required");
}
