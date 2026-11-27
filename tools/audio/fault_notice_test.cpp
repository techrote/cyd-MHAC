#include "mhac_audio_fault.hpp"
#include <cassert>
#include <cstdio>
#include <initializer_list>

int main() {
    for (unsigned delay : {0u,1u,100u}) {
        std::atomic<uint32_t> errors{0};
        mhac::AudioFaultNotice notice;
        unsigned calls=0;
        auto report=[&] { ++calls; };
        for(unsigned i=0;i<delay;++i) notice.poll(errors,report);
        assert(calls==0);
        errors.fetch_add(1,std::memory_order_relaxed);
        notice.poll(errors,report);
        assert(calls==1);
        for(unsigned i=0;i<1000;++i) {
            if(i%100==0) errors.fetch_add(1,std::memory_order_relaxed);
            notice.poll(errors,report);
        }
        assert(calls==1);
    }
    std::puts("fault notice: startup and delayed faults, exactly one report per session PASS");
}
