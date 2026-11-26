#include "mhac_factory_ui.hpp"
#include "mhac_audio.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace mhac::visual;
struct Control { unsigned x=2048,y=2048,patch=0; bool gate=true,mute=true; };
int main() {
    FactorySelection s;
    assert(s.index()==factory_count); s.ensure(); assert(s.index()==0);
    assert(&factoryEntry(factory_count)==&mhac::factory_presets[0]);
    assert(&factoryEntry(~0u)==&mhac::factory_presets[0]);
    FactorySelection cycle;
    for(unsigned i=0;i<16000;++i) {
        cycle.next(); assert(cycle.index()==i%factory_count);
        for(bool muted:{false,true}) {
            Control c; c.mute=muted; cycle.load(c);
            const auto& entry=i%factory_count<legacy_factory_count?
                mhac::factory_presets[i%factory_count]:mhac::modal_presets[i%factory_count-legacy_factory_count];
            const auto p=entry.sound;
            assert(&cycle.entry()==&entry && std::strcmp(cycle.entry().name,entry.name)==0);
            assert(c.x==p.x && c.y==p.y && c.patch==p.patch && !c.gate && c.mute==muted);
        }
    }
    unsigned select=0,audition=0;
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        const auto a=factoryAction(x,y);
        select+=a==FactoryAction::Select; audition+=a==FactoryAction::Audition;
        assert((a==FactoryAction::Select)==(y>=216&&y<240&&x>=88&&x<160));
        assert((a==FactoryAction::Audition)==(y>=216&&y<240&&x>=168&&x<192));
    }
    assert(select==1728 && audition==576);
    std::array<int16_t,mhac::block> pcm{};
    FactorySelection bank;
    std::array<uint64_t,4> modal_hashes{};
    for(unsigned i=0;i<factory_count;++i) {
        bank.next(); Control c; bank.load(c); mhac::Engine engine; mhac::Exchange io;
        io.publish_patch(c.x,c.y,c.gate,c.mute,static_cast<mhac::Patch>(c.patch));
        engine.render(io,pcm.data(),pcm.size()); for(auto p:pcm) assert(p==0);
        unsigned active=0; uint64_t hash=14695981039346656037ull;
        for(unsigned n=0;n<64;++n) {
            io.publish_patch(c.x,c.y,true,false,static_cast<mhac::Patch>(c.patch));
            engine.render(io,pcm.data(),pcm.size());
            for(auto p:pcm) { assert(p>=-8191&&p<=8191); active+=p!=0; hash=(hash^uint16_t(p))*1099511628211ull; }
        }
        assert(active>0 && io.watchdog_trips.load()==0);
        if(i>=legacy_factory_count) {
            const unsigned k=i-legacy_factory_count;
            assert(c.patch==6); for(unsigned j=0;j<k;++j) assert(modal_hashes[j]!=hash);
            modal_hashes[k]=hash;
            std::printf("%s PCM hash %016llx\n",bank.entry().name,(unsigned long long)hash);
        }
    }
    std::puts("factory UI PASS: 32000 safe loads, 77924 coordinate checks, 16 actual DSP auditions");
}
