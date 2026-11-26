#pragma once
#include <atomic>
#include <cstdint>
#include <cmath>
#include <algorithm>
namespace mhac {
constexpr unsigned rate = 24000, block = 128, contract_version = 1;
// Optional patch extension v1. Existing four-argument publishers retain phase morph.
constexpr unsigned patch_version = 1, patch_count = 4;
enum class Patch : unsigned { PhaseMorph, HarmonicLattice, RingDialect, DetunedSwarm };
constexpr unsigned control_lease_samples = rate / 2;
static_assert(std::atomic<uint32_t>::is_always_lock_free, "32-bit mailbox required");
// Single producer per mailbox; entire control snapshot transferred in one atomic word.
struct Exchange {
    std::atomic<uint32_t> control{1u << 25}, analysis{0};
    std::atomic<uint32_t> heartbeat{0}, watchdog_trips{0};
    void publish(unsigned x, unsigned y, bool gate, bool mute) noexcept {
        publish_patch(x,y,gate,mute,Patch::PhaseMorph);
    }
    void publish_patch(unsigned x, unsigned y, bool gate, bool mute, Patch patch) noexcept {
        const unsigned id=static_cast<unsigned>(patch);
        const unsigned valid=id<patch_count?id:0u;
        // Odd sequence marks an in-progress write; even commits a complete publication.
        heartbeat.fetch_add(1,std::memory_order_seq_cst);
        control.store(std::min(x,4095u) | (std::min(y,4095u)<<12) |
                      (uint32_t(gate)<<24) | (uint32_t(mute)<<25) | (valid<<26), std::memory_order_seq_cst);
        heartbeat.fetch_add(1,std::memory_order_seq_cst);
    }
    uint32_t snapshot() const noexcept { return analysis.load(std::memory_order_relaxed); }
};
// Audio-thread owned state. Original v1 oscillator and independent gate lease.
class Engine {
    uint32_t phase_=0, mod_=0, seq_=0, last_heartbeat_=0;
    uint32_t accepted_control_=1u<<25;
    unsigned age_=control_lease_samples;
    bool expired_=false;
    float hz_=110.f, morph_=0.f, env_=0.f;
    uint32_t swarm_lo_=0, swarm_hi_=0;
    // Integer simplex: exactly 256 total, up to 256 samples per full transition.
    unsigned weights_[patch_count]={256,0,0,0};
    static float sine(uint32_t p) noexcept {
        // Parabolic sine approximation, bounded to [-1,1].
        const float t=float(p>>16)*(1.f/32768.f)-1.f;
        const float y=4.f*t*(1.f-std::fabs(t));
        return y + .225f*(y*std::fabs(y)-y);
    }
public:
    void render(Exchange& io, int16_t* out, unsigned n) noexcept {
        // One bounded attempt; a writer paused mid-publication never blocks audio.
        const auto before=io.heartbeat.load(std::memory_order_seq_cst);
        const auto candidate=io.control.load(std::memory_order_seq_cst);
        const auto after=io.heartbeat.load(std::memory_order_seq_cst);
        if(before==after && !(after&1u) && after!=last_heartbeat_) {
            last_heartbeat_=after; accepted_control_=candidate; age_=0;
        }
        const auto c=accepted_control_;
        const unsigned patch=(c>>26)&3u;
        const float x=float(c&4095u)/4095.f, y=float((c>>12)&4095u)/4095.f;
        const float target=110.f*std::exp2(3.f*x);
        const bool requested=(c&(1u<<24)) && !(c&(1u<<25));
        if(!requested) expired_=false;
        // Expire at a sample boundary, regardless of render partition. No clock or UI calls.
        const unsigned remaining=control_lease_samples-age_;
        const unsigned active_samples=requested && !expired_?std::min(n,remaining):0u;
        unsigned peak=0;
        for(unsigned i=0;i<n;++i) {
            const bool active=i<active_samples;
            hz_+=(target-hz_)*.005f; morph_+=(y-morph_)*.005f;
            // Faster but still smoothed release on an expired gate; ordinary release unchanged.
            env_+=((active?1.f:0.f)-env_)*(active?.008f:(requested?.008f:.002f));
            if(!active && env_<.0001f) env_=0.f;
            const auto step=uint32_t(hz_*(4294967296.0f/rate));
            phase_+=step; mod_+=step*2u;
            const auto offset=int32_t(sine(mod_)*morph_*805306368.f);
            const float a=sine(phase_), b=sine(phase_+uint32_t(offset));
            const float original=a+(b-a)*morph_;
            // Advance detuned phases even when inactive; never reset on selection.
            const auto detune=uint32_t(float(step)*(.004f+.024f*morph_));
            swarm_lo_+=step-detune; swarm_hi_+=step+detune;
            if(weights_[patch]<256u) {
                for(unsigned j=0;j<patch_count;++j) if(j!=patch && weights_[j]) {
                    --weights_[j]; ++weights_[patch]; break;
                }
            }
            float sound=original;
            if(weights_[0]!=256u) {
                sound=original*float(weights_[0]);
                if(weights_[1]) {
                    const float harmonics=(sine(phase_*2u)+.6f*sine(phase_*3u)+.4f*sine(phase_*5u))*.5f;
                    sound+=float(weights_[1])*(a*(1.f-.75f*morph_)+harmonics*.75f*morph_);
                }
                if(weights_[2]) {
                    const float ring=a*sine(phase_*4u);
                    sound+=float(weights_[2])*(a*(1.f-morph_)+ring*morph_);
                }
                if(weights_[3]) {
                    const float swarm=(a+sine(swarm_lo_)+sine(swarm_hi_))*(1.f/3.f);
                    sound+=float(weights_[3])*swarm;
                }
                sound*=1.f/256.f;
            }
            const int v=int(8191.f*env_*sound);
            out[i]=int16_t(v); peak=std::max(peak,unsigned(v<0?-v:v));
        }
        age_+=std::min(n,remaining);
        if(requested && !expired_ && age_==control_lease_samples && n>0) {
            expired_=true;
            io.watchdog_trips.fetch_add(1,std::memory_order_relaxed);
        }
        io.analysis.store((std::min(peak,8191u)>>1) | ((phase_>>20)<<12) |
                          ((++seq_&255u)<<24), std::memory_order_relaxed);
    }
};
}
