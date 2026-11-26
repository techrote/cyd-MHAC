#pragma once
#include "mhac_preset_store.hpp"
namespace mhac {
// Explicit extension: the legacy codec, bank and NVS slot remain unchanged.
constexpr unsigned performance_preset_version = 2, performance_preset_modal_id = 6;
inline constexpr std::array<NamedPreset,4> modal_presets{{
    {"Stone Bell",{6,0,256}}, {"Hollow Gong",{6,683,900}},
    {"Copper Rain",{6,1365,2200}}, {"Glass Pins",{6,2730,4095}}
}};
constexpr bool encode_performance_preset(SoundPreset p, PresetRecord& dst) noexcept {
    if(p.patch!=performance_preset_modal_id) return encode_preset(p,dst);
    if(p.x>4095 || p.y>4095) return false;
    // Use the existing layout and CRC; v2 is canonical only for Modal Chime.
    PresetRecord b{};
    if(!encode_preset({0,p.x,p.y},b)) return false;
    b[4]=performance_preset_version; b[5]=p.patch;
    const auto crc=preset_crc(b); b[10]=uint8_t(crc); b[11]=uint8_t(crc>>8);
    dst=b; return true;
}
constexpr bool decode_performance_preset(const PresetRecord& b, SoundPreset& dst) noexcept {
    if(b[4]==preset_format_version) return decode_preset(b,dst);
    if(b[4]!=performance_preset_version || b[5]!=performance_preset_modal_id ||
       preset_crc(b)!=(uint16_t(b[10])|(uint16_t(b[11])<<8))) return false;
    auto legacy=b; legacy[4]=preset_format_version; legacy[5]=0;
    const auto crc=preset_crc(legacy); legacy[10]=uint8_t(crc); legacy[11]=uint8_t(crc>>8);
    SoundPreset p{};
    if(!decode_preset(legacy,p)) return false;
    p.patch=performance_preset_modal_id; dst=p; return true;
}
// UI-thread only, explicitly separate backend key (sound-v2 on ESP32).
template<class Backend> StoreResult recall_performance_sound(Backend& backend, SoundPreset& dst) {
    PresetRecord b{};
    if(!backend.read(b)) return StoreResult::unavailable;
    return decode_performance_preset(b,dst)?StoreResult::ok:StoreResult::invalid;
}
template<class Backend> StoreResult save_performance_sound(Backend& backend, SoundPreset p) {
    PresetRecord b{}, old{};
    if(!encode_performance_preset(p,b)) return StoreResult::invalid;
    if(backend.read(old) && old==b) return StoreResult::unchanged;
    if(!backend.write(b)) return StoreResult::write_failed;
    if(!backend.read(old) || old!=b) return StoreResult::verify_failed;
    return StoreResult::ok;
}
}
