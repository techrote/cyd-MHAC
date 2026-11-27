#pragma once
#include <atomic>
#include <cstdint>

namespace mhac {
// Setup-thread only. A failed install owns no driver; once installed, every
// later failure releases it. Publish the terminal fault before returning to UI.
template<class Install, class Enable, class Launch, class Cleanup>
bool start_audio(std::atomic<uint32_t>& errors, Install install, Enable enable,
                 Launch launch, Cleanup cleanup) {
    if (install()) {
        if (enable() && launch()) return true;
        cleanup();
    }
    errors.fetch_add(1, std::memory_order_relaxed);
    return false;
}
}
