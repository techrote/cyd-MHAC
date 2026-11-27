#include "mhac_presets.hpp"
#include <cassert>
#include <cstdio>
using namespace mhac;
static bool same(SoundPreset a,SoundPreset b) {
    return a.patch==b.patch && a.x==b.x && a.y==b.y;
}
static void fix_crc(PresetRecord& b) {
    const auto crc=preset_crc(b); b[10]=uint8_t(crc); b[11]=uint8_t(crc>>8);
}
int main() {
    const SoundPreset sentinel{5,123,456};
    unsigned roundtrips=0, corruptions=0;
    for(unsigned patch=0;patch<6;++patch) {
        for(unsigned x=0;x<4096;++x) {
            const SoundPreset p{uint8_t(patch),uint16_t(x),uint16_t(4095-x)};
            PresetRecord b{}; SoundPreset q=sentinel;
            assert(encode_preset(p,b) && decode_preset(b,q) && same(p,q)); ++roundtrips;
        }
    }
    for(const auto& entry:factory_presets) {
        PresetRecord b{}; assert(encode_preset(entry.sound,b));
        SoundPreset q=sentinel; assert(decode_preset(b,q) && same(q,entry.sound));
        for(unsigned bit=0;bit<96;++bit) {
            auto bad=b; bad[bit/8]^=uint8_t(1u<<(bit%8)); q=sentinel;
            assert(!decode_preset(bad,q) && same(q,sentinel)); ++corruptions;
        }
    }
    PresetRecord b{}; assert(encode_preset({0,0,0},b));
    const PresetRecord golden{{0x4d,0x48,0x50,0x53,1,0,0,0,0,0,0xef,0xd1}};
    assert(b==golden);
    for(unsigned patch=6;patch<256;++patch) {
        auto bad=b; bad[5]=uint8_t(patch); fix_crc(bad); auto q=sentinel;
        assert(!decode_preset(bad,q) && same(q,sentinel));
        auto keep=b; assert(!encode_preset({uint8_t(patch),0,0},keep) && keep==b);
    }
    for(unsigned index: {4u,7u,9u}) {
        auto bad=b; bad[index]=index==4?2:16; fix_crc(bad); auto q=sentinel;
        assert(!decode_preset(bad,q) && same(q,sentinel));
    }
    auto keep=b; assert(!encode_preset({0,4096,0},keep) && keep==b);
    assert(!encode_preset({0,0,65535},keep) && keep==b);
    std::printf("PASS %u coordinate round trips, %u single-bit corruptions, 250 invalid IDs, 12 factory presets, golden CRC\n",roundtrips,corruptions);
}
