#pragma once
#include <array>
#include <cstdint>
#include <cstddef>

namespace mhac {
// Portable sound-only record. Never stores gate, mute, heartbeat or UI state.
constexpr unsigned preset_format_version = 1, preset_catalog_v1 = 6;
struct SoundPreset { uint8_t patch; uint16_t x, y; };
using PresetRecord = std::array<uint8_t,12>;
constexpr bool valid_preset(SoundPreset p) noexcept {
    return p.patch < preset_catalog_v1 && p.x <= 4095 && p.y <= 4095;
}
constexpr uint16_t preset_crc(const PresetRecord& b) noexcept {
    uint16_t crc=0xffff;
    for(unsigned i=0;i<10;++i) {
        crc=uint16_t(crc ^ (uint16_t(b[i])<<8));
        for(unsigned bit=0;bit<8;++bit)
            crc=uint16_t((uint32_t(crc)<<1) ^ ((crc&0x8000u)?0x1021u:0u));
    }
    return crc;
}
constexpr bool encode_preset(SoundPreset p, PresetRecord& dst) noexcept {
    if(!valid_preset(p)) return false;
    PresetRecord b{{'M','H','P','S',preset_format_version,p.patch,
                    uint8_t(p.x),uint8_t(p.x>>8),uint8_t(p.y),uint8_t(p.y>>8),0,0}};
    const auto crc=preset_crc(b);
    b[10]=uint8_t(crc); b[11]=uint8_t(crc>>8);
    dst=b; return true;
}
constexpr bool decode_preset(const PresetRecord& b, SoundPreset& dst) noexcept {
    if(b[0]!='M'||b[1]!='H'||b[2]!='P'||b[3]!='S'||b[4]!=preset_format_version) return false;
    if(preset_crc(b)!=(uint16_t(b[10])|(uint16_t(b[11])<<8))) return false;
    const SoundPreset p{b[5],uint16_t(b[6]|(uint16_t(b[7])<<8)),uint16_t(b[8]|(uint16_t(b[9])<<8))};
    if(!valid_preset(p)) return false;
    dst=p; return true;
}
struct NamedPreset { const char* name; SoundPreset sound; };
inline constexpr std::array<NamedPreset,12> factory_presets{{
    {"Soft Current",{0,512,256}}, {"Glass Fold",{0,2730,3500}},
    {"Low Organ",{1,0,1200}}, {"Harmonic Lantern",{1,2048,3900}},
    {"Copper Bell",{2,1365,3100}}, {"Ring Dust",{2,3072,4095}},
    {"Slow Swarm",{3,683,900}}, {"Double Orbit",{3,2048,3850}},
    {"Four Marks",{4,1365,0}}, {"Missing Mark",{4,1365,4095}},
    {"Near Reflection",{5,683,1500}}, {"Recursive Wire",{5,2730,4095}}
}};
}
