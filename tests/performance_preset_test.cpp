#include "mhac_audio.hpp"
#include "mhac_performance_presets.hpp"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace mhac;
static_assert(performance_preset_modal_id==modal_patch_index);
static bool same(SoundPreset a,SoundPreset b) { return a.patch==b.patch && a.x==b.x && a.y==b.y; }
static void fix_crc(PresetRecord& b) { auto crc=preset_crc(b); b[10]=uint8_t(crc); b[11]=uint8_t(crc>>8); }
struct Fake {
    PresetRecord data{}; bool exists=false, deny=false, corrupt=false, lose=false;
    unsigned writes=0;
    bool read(PresetRecord& b) { if(!exists || (lose && writes)) return false; b=data; return true; }
    bool write(const PresetRecord& b) { ++writes; if(deny) return false; data=b; exists=true; if(corrupt) data[6]^=1; return true; }
};
static std::vector<int16_t> render(SoundPreset p,bool split) {
    Engine engine; Exchange io; std::vector<int16_t> v(rate*4);
    for(unsigned k=0;k<v.size();k+=block) {
        const unsigned n=std::min<unsigned>(block,v.size()-k);
        const bool gate=k<block*12 || (k>=block*36 && k<block*48) || (k>=block*72 && k<block*84);
        io.publish_modal(p.x,p.y,gate,false);
        if(split) { const unsigned a=n/3; engine.render(io,v.data()+k,a); engine.render(io,v.data()+k+a,n-a); }
        else engine.render(io,v.data()+k,n);
    }
    assert(io.watchdog_trips.load()==0);
    return v;
}
int main(int argc,char**) {
    const SoundPreset sentinel{5,123,456}; unsigned roundtrips=0,corruptions=0;
    for(unsigned patch=0;patch<7;++patch) for(unsigned x=0;x<4096;++x) {
        SoundPreset p{uint8_t(patch),uint16_t(x),uint16_t(4095-x)},q=sentinel;
        PresetRecord b{},legacy{};
        assert(encode_performance_preset(p,b) && decode_performance_preset(b,q) && same(p,q));
        if(patch<6) { assert(encode_preset(p,legacy) && legacy==b); }
        else { q=sentinel; assert(!decode_preset(b,q) && same(q,sentinel)); }
        ++roundtrips;
    }
    PresetRecord b{}; assert(encode_performance_preset({6,0,0},b));
    for(unsigned version=0;version<256;++version) for(unsigned patch=0;patch<256;++patch) {
        auto bad=b; bad[4]=uint8_t(version); bad[5]=uint8_t(patch); fix_crc(bad); auto q=sentinel;
        const bool valid=(version==1 && patch<6) || (version==2 && patch==6);
        assert(decode_performance_preset(bad,q)==valid);
        assert(valid ? q.patch==patch : same(q,sentinel));
    }
    for(unsigned patch=7;patch<256;++patch) { auto keep=b; assert(!encode_performance_preset({uint8_t(patch),0,0},keep) && keep==b); }
    for(unsigned index: {0u,7u,9u}) { auto bad=b; bad[index]=16; fix_crc(bad); auto q=sentinel; assert(!decode_performance_preset(bad,q) && same(q,sentinel)); }
    for(auto entry:modal_presets) {
        const auto p=entry.sound; auto dst=sentinel; Fake f;
        assert(recall_performance_sound(f,dst)==StoreResult::unavailable && same(dst,sentinel));
        assert(save_performance_sound(f,p)==StoreResult::ok && f.writes==1);
        assert(save_performance_sound(f,p)==StoreResult::unchanged && f.writes==1);
        assert(recall_sound(f,dst)==StoreResult::invalid && same(dst,sentinel));
        Fake reboot; reboot.exists=true; reboot.data=f.data;
        assert(recall_performance_sound(reboot,dst)==StoreResult::ok && same(dst,p));
        for(unsigned bit=0;bit<96;++bit) {
            Fake bad=reboot; bad.data[bit/8]^=uint8_t(1u<<(bit%8)); dst=sentinel;
            assert(recall_performance_sound(bad,dst)==StoreResult::invalid && same(dst,sentinel)); ++corruptions;
        }
        Fake denied; denied.deny=true; assert(save_performance_sound(denied,p)==StoreResult::write_failed);
        Fake corrupt; corrupt.corrupt=true; assert(save_performance_sound(corrupt,p)==StoreResult::verify_failed);
        Fake lost; lost.lose=true; assert(save_performance_sound(lost,p)==StoreResult::verify_failed);
        assert(save_performance_sound(f,{6,4096,0})==StoreResult::invalid && f.writes==1);
        assert(save_performance_sound(f,{6,0,65535})==StoreResult::invalid && f.writes==1);
        const auto pcm=render(p,false); assert(pcm==render(p,true));
        unsigned peak=0,last=0; uint64_t energy=0;
        for(unsigned i=0;i<pcm.size();++i) {
            const auto s=int(pcm[i]); assert(s>=-8191 && s<=8191);
            peak=std::max(peak,unsigned(s<0?-s:s)); if(s) last=i;
            energy+=int64_t(s)*s;
        }
        assert(peak>100 && last>block*84 && last<pcm.size()-1);
        std::printf("%s: peak=%u energy=%llu last=%u\n",entry.name,peak,(unsigned long long)energy,last);
        if(argc>1) {
            char name[64]; std::snprintf(name,sizeof name,"%s.pcm",entry.name);
            auto* file=std::fopen(name,"wb"); assert(file);
            for(auto s:pcm) { const auto u=uint16_t(s); assert(std::fputc(u&255,file)!=EOF); assert(std::fputc(u>>8,file)!=EOF); }
            assert(std::fclose(file)==0);
            std::snprintf(name,sizeof name,"%s.mhps",entry.name); file=std::fopen(name,"wb"); assert(file);
            assert(std::fwrite(f.data.data(),1,f.data.size(),file)==f.data.size()); assert(std::fclose(file)==0);
        }
    }
    std::printf("PASS %u roundtrips, 65536 version/ID pairs, %u corruptions, 4 exact PCM partition comparisons\n",roundtrips,corruptions);
}
