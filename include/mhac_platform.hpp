#pragma once
#include "mhac_audio.hpp"
#include "mhac_timing.hpp"
namespace mhac {
extern Exchange exchange;
constexpr uint32_t audio_block_budget_us=(block*1000000u+rate-1u)/rate;
extern TimingMailbox audio_timing;
extern std::atomic<uint32_t> audio_blocks, audio_errors;
}
// Optional visual lane overrides; all UI calls on Arduino loop thread only.
void mhac_ui_begin();
void mhac_ui_tick();
