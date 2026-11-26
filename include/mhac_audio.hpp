#pragma once
#include <atomic>
#include <cstdint>
#include <cmath>
#include <algorithm>
#include "mhac_features.hpp"
#include "mhac_modal.hpp"
namespace mhac {
constexpr unsigned rate = 24000, block = 128, contract_version = 1;
// Optional patch extension v1. Existing four-argument publishers retain phase morph.
constexpr unsigned patch_version = 1, patch_count = 4;
// v1 constants intentionally remain the four-mode compatibility catalogue.
// v2 clients opt into the extended catalogue; bit 28 was unused in v1.
constexpr unsigned patch_extension_version = 2, extended_patch_count = 6;
enum class Patch : unsigned { PhaseMorph, HarmonicLattice, RingDialect, DetunedSwarm, ErasurePulse, RecursiveEcho };
// Explicit opt-in to v3; v1/v2 callers and their six-patch catalogue remain unchanged.
constexpr unsigned modal_extension_version = 3, modal_patch_index = 6, performance_patch_count = 7;
constexpr uint32_t modal_opt_in = 1u<<29;
constexpr unsigned motif_period = rate / 2, echo_samples = 1536;
constexpr unsigned control_lease_samples = rate / 2;
static_assert(std::atomic<uint32_t>::is_always_lock_free, "32-bit mailbox required");
// Single producer per mailbox; entire control snapshot transferred in one atomic word.
struct Exchange {
    std::atomic<uint32_t> control{1u << 25}, analysis{0};
    std::atomic<uint32_t> heartbeat{0}, watchdog_trips{0};
    FeatureMailbox features;
    void publish(unsigned x, unsigned y, bool gate, bool mute) noexcept {
        publish_patch(x,y,gate,mute,Patch::PhaseMorph);
    }
    void publish_patch(unsigned x, unsigned y, bool gate, bool mute, Patch patch) noexcept {
        const unsigned id=static_cast<unsigned>(patch);
        const unsigned valid=id<extended_patch_count?id:0u;
        publish_word(x,y,gate,mute,valid,0);
    }
    void publish_modal(unsigned x,unsigned y,bool gate,bool mute) noexcept {
        publish_word(x,y,gate,mute,modal_patch_index,modal_opt_in);
    }
private:
    void publish_word(unsigned x,unsigned y,bool gate,bool mute,unsigned id,uint32_t extension) noexcept {
        // Odd sequence marks an in-progress write; even commits a complete publication.
        heartbeat.fetch_add(1,std::memory_order_seq_cst);
        control.store(std::min(x,4095u) | (std::min(y,4095u)<<12) |
                      (uint32_t(gate)<<24) | (uint32_t(mute)<<25) | (id<<26) | extension, std::memory_order_seq_cst);
        heartbeat.fetch_add(1,std::memory_order_seq_cst);
    }
public:
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
    unsigned weights_[performance_patch_count]={256,0,0,0,0,0,0};
    ModalChime modal_;
    bool modal_used_=false;
    unsigned motif_phase_=0, echo_pos_=0, echo_valid_=0;
    float rhythm_gain_=0.f;
    int16_t echo_[echo_samples]={};
    FeatureAccumulator features_;
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
        const unsigned id=(c>>26)&7u;
        const unsigned patch=id<extended_patch_count?id:
            (id==modal_patch_index && (c&modal_opt_in)?modal_patch_index:0u);
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
            // Advance detuned phases even when inactive; never reset on selection.
            const auto detune=uint32_t(float(step)*(.004f+.024f*morph_));
            swarm_lo_+=step-detune; swarm_hi_+=step+detune;
            if(weights_[patch]<256u) {
                for(unsigned j=0;j<performance_patch_count;++j) if(j!=patch && weights_[j]) {
                    --weights_[j]; ++weights_[patch]; break;
                }
            }
            // Evaluate only oscillators consumed by the current mix, but keep all
            // phase accumulators advancing. Use post-transition weights so the
            // first sample of a newly selected patch still has its exact source.
            const bool carrier_needed=weights_[modal_patch_index]!=256u;
            const float a=carrier_needed?sine(phase_):0.f;
            float original=0.f;
            if(weights_[0] || weights_[4]) {
                const auto offset=int32_t(sine(mod_)*morph_*805306368.f);
                const float b=sine(phase_+uint32_t(offset));
                original=a+(b-a)*morph_;
            }
            // Free-running 2 Hz motif clock; no dependency on control or render block cadence.
            if(++motif_phase_==motif_period) motif_phase_=0;
            float rhythm=0.f, wet=0.f;
            if(weights_[4]) {
                // Uneven four-onset motif; Y continuously erases the second onset.
                const unsigned t=motif_phase_;
                float g=0.f;
                if(t<1100u) g=1.f;
                else if(t>=2832u && t<3932u) g=1.f-morph_;
                else if(t>=7416u && t<8516u) g=.8f;
                else if(t>=10248u && t<11348u) g=.65f*(1.f-.5f*morph_);
                rhythm_gain_+=(g-rhythm_gain_)*(g>rhythm_gain_?.015f:.003f);
                if(rhythm_gain_<.00001f) rhythm_gain_=0.f;
                rhythm=original*rhythm_gain_;
            } else rhythm_gain_=0.f;
            if(weights_[5] && env_>0.f) {
                // Fixed 64 ms delay, bounded feedback and lazy invalidation (no bulk clear).
                const float delayed=echo_valid_==echo_samples?float(echo_[echo_pos_])*(1.f/32768.f):0.f;
                const float next=(active?.35f*a:0.f)+(.20f+.40f*morph_)*delayed;
                echo_[echo_pos_]=int16_t(next*32768.f);
                if(++echo_pos_==echo_samples) echo_pos_=0;
                if(echo_valid_<echo_samples) ++echo_valid_;
                const float mix=.65f*morph_;
                wet=a*(1.f-mix)+delayed*mix;
            } else { echo_pos_=0; echo_valid_=0; }
            float modal=0.f;
            // Modal strikes have their own finite decay, independent of ordinary gate release.
            if(weights_[modal_patch_index] && (active || modal_used_)) {
                const bool inhibit=(c&(1u<<25)) || (requested && !active) || patch!=modal_patch_index;
                modal=modal_.sample(step,morph_,active && patch==modal_patch_index,sine,inhibit);
                modal_used_=true;
            } else if(modal_used_) { modal_.reset(); modal_used_=false; }
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
                if(weights_[4]) sound+=float(weights_[4])*rhythm;
                if(weights_[5]) sound+=float(weights_[5])*wet;
                sound*=1.f/256.f;
            }
            // Preserve the exact legacy arithmetic when no modal contribution is mixed.
            const int v=weights_[modal_patch_index]
                ? int(8191.f*(env_*sound+modal*(float(weights_[modal_patch_index])/256.f)))
                : int(8191.f*env_*sound);
            out[i]=int16_t(v); peak=std::max(peak,unsigned(v<0?-v:v));
            features_.push(out[i],io.features);
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
