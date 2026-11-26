#pragma once
#include "mhac_audio.hpp"
namespace mhac {
extern Exchange exchange;
extern std::atomic<uint32_t> audio_blocks, audio_errors;
}
// Optional visual lane overrides; all UI calls on Arduino loop thread only.
void mhac_ui_begin();
void mhac_ui_tick();
