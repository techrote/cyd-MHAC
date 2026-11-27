#pragma once
#include "mhac_audio.hpp"
#include "mhac_timing.hpp"
#include "mhac_prepare_profile.hpp"
namespace mhac {
extern Exchange exchange;
constexpr uint32_t audio_block_budget_us=(block*1000000u+rate-1u)/rate;
extern TimingMailbox audio_timing;
static_assert(audio_block_budget_us==prepare_profile_budget_us,"preparation bins require the current block/rate");
extern PrepareProfileMailbox audio_profile;
extern std::atomic<uint32_t> audio_blocks, audio_errors;
}
// Optional visual lane overrides; all UI calls on Arduino loop thread only.
void mhac_ui_begin();
void mhac_ui_tick();
