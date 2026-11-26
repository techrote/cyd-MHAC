#pragma once
#include <atomic>
#include <cstdint>
#include <cmath>
#include <algorithm>
namespace mhac {
constexpr unsigned rate = 24000, block = 128, contract_version = 1;
static_assert(std::atomic<uint32_t>::is_always_lock_free, "32-bit mailbox required");
// Single producer per mailbox; entire snapshot transferred in one atomic word.
struct Exchange {
    std::atomic<uint32_t> control{1u << 25}, analysis{0};
    void publish(unsigned x, unsigned y, bool gate, bool mute) noexcept {
        control.store(std::min(x,4095u) | (std::min(y,4095u)<<12) |
                      (uint32_t(gate)<<24) | (uint32_t(mute)<<25), std::memory_order_relaxed);
    }
    uint32_t snapshot() const noexcept { return analysis.load(std::memory_order_relaxed); }
};
// Audio-thread owned state. Triangle/sine phase modulation morph, attack/release and gain smoothing.
class Engine {
    uint32_t phase_=0, mod_=0, seq_=0;
    float hz_=110.f, morph_=0.f, env_=0.f;
    static float sine(uint32_t p) noexcept {
        // Parabolic sine approximation, bounded to [-1,1].
        const float t=float(p>>16)*(1.f/32768.f)-1.f;
        const float y=4.f*t*(1.f-std::fabs(t));
        return y + .225f*(y*std::fabs(y)-y);
    }
public:
    void render(Exchange& io, int16_t* out, unsigned n) noexcept {
        const auto c=io.control.load(std::memory_order_relaxed);
        const float x=float(c&4095u)/4095.f, y=float((c>>12)&4095u)/4095.f;
        const float target=110.f*std::exp2(3.f*x);
        const bool active=(c&(1u<<24)) && !(c&(1u<<25));
        unsigned peak=0;
        for(unsigned i=0;i<n;++i) {
            hz_+=(target-hz_)*.005f; morph_+=(y-morph_)*.005f;
            env_+=((active?1.f:0.f)-env_)*(active?.008f:.002f);
            if(!active && env_<.0001f) env_=0.f;
            const auto step=uint32_t(hz_*(4294967296.0f/rate));
            phase_+=step; mod_+=step*2u;
            const auto offset=int32_t(sine(mod_)*morph_*805306368.f);
            const float a=sine(phase_), b=sine(phase_+uint32_t(offset));
            const int v=int(8191.f*env_*(a+(b-a)*morph_));
            out[i]=int16_t(v); peak=std::max(peak,unsigned(v<0?-v:v));
        }
        io.analysis.store((std::min(peak,8191u)>>1) | ((phase_>>20)<<12) |
                          ((++seq_&255u)<<24), std::memory_order_relaxed);
    }
};
}
