#pragma once
#include "mhac_presets.hpp"
namespace mhac {
// UI-thread only. Backend read/write return true only for complete records.
// No control publication; callers must explicitly audition after recall.
enum class StoreResult { ok, unchanged, unavailable, invalid, write_failed, verify_failed };
constexpr bool storage_allowed(bool muted, bool sounding, uint32_t now, uint32_t quiet_since) noexcept {
    return muted && !sounding && uint32_t(now-quiet_since)>=250u;
}
template<class Backend> StoreResult recall_sound(Backend& backend, SoundPreset& dst) {
    PresetRecord b{};
    if(!backend.read(b)) return StoreResult::unavailable;
    return decode_preset(b,dst)?StoreResult::ok:StoreResult::invalid;
}
template<class Backend> StoreResult save_sound(Backend& backend, SoundPreset p) {
    PresetRecord b{}, old{};
    if(!encode_preset(p,b)) return StoreResult::invalid;
    if(backend.read(old) && old==b) return StoreResult::unchanged;
    if(!backend.write(b)) return StoreResult::write_failed;
    if(!backend.read(old) || old!=b) return StoreResult::verify_failed;
    return StoreResult::ok;
}
inline const char* store_result_name(StoreResult s) noexcept {
    switch(s) {
    case StoreResult::ok: return "ok";
    case StoreResult::unchanged: return "unchanged";
    case StoreResult::unavailable: return "unavailable";
    case StoreResult::invalid: return "invalid";
    case StoreResult::write_failed: return "write failed";
    case StoreResult::verify_failed: return "verify failed";
    }
    return "unknown";
}
}
