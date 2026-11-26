#include "mhac_preset_store.hpp"
#include <cassert>
#include <cstdio>
using namespace mhac;
struct Fake {
    PresetRecord record{};
    bool exists=false, refuse_write=false, corrupt_write=false, lose_readback=false;
    unsigned writes=0;
    bool read(PresetRecord& out) {
        if(!exists || (lose_readback && writes)) return false;
        out=record; return true;
    }
    bool write(const PresetRecord& in) {
        ++writes;
        if(refuse_write) return false;
        record=in; exists=true;
        if(corrupt_write) record[6]^=1;
        return true;
    }
};
static bool eq(SoundPreset a, SoundPreset b) { return a.patch==b.patch && a.x==b.x && a.y==b.y; }
int main() {
    unsigned cases=0;
    for(auto entry:factory_presets) {
        const auto p=entry.sound; const SoundPreset sentinel{5,4095,4095};
        Fake f; auto dst=sentinel;
        assert(recall_sound(f,dst)==StoreResult::unavailable && eq(dst,sentinel));
        assert(save_sound(f,p)==StoreResult::ok && f.writes==1);
        assert(save_sound(f,p)==StoreResult::unchanged && f.writes==1);
        Fake reboot; reboot.exists=true; reboot.record=f.record;
        assert(recall_sound(reboot,dst)==StoreResult::ok && eq(dst,p));
        for(unsigned bit=0;bit<96;++bit) {
            Fake bad=reboot; bad.record[bit/8]^=uint8_t(1u<<(bit%8)); dst=sentinel;
            assert(recall_sound(bad,dst)==StoreResult::invalid && eq(dst,sentinel));
            ++cases;
        }
        Fake denied; denied.refuse_write=true;
        assert(save_sound(denied,p)==StoreResult::write_failed && denied.writes==1);
        Fake corrupt; corrupt.corrupt_write=true;
        assert(save_sound(corrupt,p)==StoreResult::verify_failed);
        Fake lost; lost.lose_readback=true;
        assert(save_sound(lost,p)==StoreResult::verify_failed);
        assert(save_sound(f,{6,0,0})==StoreResult::invalid && f.writes==1);
        assert(save_sound(f,{0,4096,0})==StoreResult::invalid && f.writes==1);
    }
    for(uint32_t start: {0u,1000u,0xffffff80u,0xffffffffu}) {
        for(uint32_t elapsed=0;elapsed<=500;++elapsed) {
            assert(storage_allowed(true,false,start+elapsed,start)==(elapsed>=250));
            assert(!storage_allowed(false,false,start+elapsed,start));
            assert(!storage_allowed(true,true,start+elapsed,start));
        }
    }
    std::printf("PASS: 12 save/reboot/recall/duplicate/failure sets, %u corrupt records, 6012 quiet gate checks\n",cases);
}
