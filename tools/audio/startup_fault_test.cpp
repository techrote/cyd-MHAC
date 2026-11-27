#include "mhac_audio_start.hpp"
#include <cassert>
#include <cstdio>

int main() {
    // Failure at each stage, plus success. Calls are recorded in decimal order.
    for (unsigned failed = 0; failed <= 3; ++failed) {
        std::atomic<uint32_t> errors{0};
        unsigned calls = 0;
        bool installed = false, enabled = false;
        const bool ok = mhac::start_audio(errors,
            [&] { calls = calls * 10 + 1; return installed = failed != 1; },
            [&] { assert(installed); calls = calls * 10 + 2; return enabled = failed != 2; },
            [&] { assert(installed && enabled); calls = calls * 10 + 3; return failed != 3; },
            [&] { assert(installed); calls = calls * 10 + 4; installed = enabled = false; });
        constexpr unsigned expected[] = {123, 1, 124, 1234};
        assert(calls == expected[failed]);
        assert(ok == (failed == 0));
        assert(errors.load() == unsigned(failed != 0));
        assert(installed == ok && enabled == ok);
    }
    std::puts("PASS startup success, install failure, DAC enable failure, task launch failure; cleanup order and terminal error publication");
}
