#pragma once
#include <atomic>
#include <cstdint>

namespace mhac {
// UI/loop-thread owned; no logging or additional work on the audio task.
class AudioFaultNotice {
    bool reported_=false;
public:
    template<class Report>
    void poll(const std::atomic<uint32_t>& errors, Report report) {
        if (!reported_ && errors.load(std::memory_order_relaxed)!=0) {
            reported_=true;
            report();
        }
    }
};
}
