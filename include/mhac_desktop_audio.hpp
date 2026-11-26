#pragma once
#include "mhac_audio.hpp"
#include <array>
#include <cstddef>
#include <cstring>
namespace mhac::visual {
// Native audition only. UI is sole Exchange publisher; callback owns Engine.
class DesktopAudio {
    Engine engine_;
public:
    Exchange exchange;
    std::atomic<uint32_t> callbacks{0}, active_callbacks{0}, last_peak{0};
    void render(void* stream, std::size_t bytes) noexcept {
        auto* dst=static_cast<unsigned char*>(stream);
        std::array<int16_t,block> tmp{};
        unsigned peak=0;
        while(bytes>=sizeof(int16_t)) {
            const auto n=unsigned(std::min(bytes/sizeof(int16_t),std::size_t(block)));
            engine_.render(exchange,tmp.data(),n);
            for(unsigned i=0;i<n;++i) peak=std::max(peak,unsigned(std::abs(int(tmp[i]))));
            const auto count=n*sizeof(int16_t);
            std::memcpy(dst,tmp.data(),count); dst+=count; bytes-=count;
        }
        if(bytes) *dst=0;
        last_peak.store(peak,std::memory_order_relaxed);
        if(peak) active_callbacks.fetch_add(1,std::memory_order_relaxed);
        callbacks.fetch_add(1,std::memory_order_relaxed);
    }
};
}
