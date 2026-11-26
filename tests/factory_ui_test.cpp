#include "mhac_factory_ui.hpp"
#include "mhac_audio.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
struct Control { unsigned x=2048,y=2048,patch=0; bool gate=true,mute=true; };
int main() {
    FactorySelection s;
    assert(s.index()==factory_count); s.ensure(); assert(s.index()==0);
    FactorySelection cycle;
    for(unsigned i=0;i<12000;++i) {
        cycle.next(); assert(cycle.index()==i%factory_count);
        for(bool muted:{false,true}) {
            Control c; c.mute=muted; cycle.load(c);
            const auto p=mhac::factory_presets[i%factory_count].sound;
            assert(c.x==p.x && c.y==p.y && c.patch==p.patch && !c.gate && c.mute==muted);
        }
    }
    unsigned select=0,audition=0;
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        const auto a=factoryAction(x,y);
        select+=a==FactoryAction::Select; audition+=a==FactoryAction::Audition;
        assert((a==FactoryAction::Select)==(y>=216&&y<240&&x>=104&&x<160));
        assert((a==FactoryAction::Audition)==(y>=216&&y<240&&x>=168&&x<192));
    }
    assert(select==1344 && audition==576);
    std::array<int16_t,mhac::block> pcm{};
    FactorySelection bank;
    for(unsigned i=0;i<factory_count;++i) {
        bank.next(); Control c; bank.load(c); mhac::Engine engine; mhac::Exchange io;
        io.publish_patch(c.x,c.y,c.gate,c.mute,static_cast<mhac::Patch>(c.patch));
        engine.render(io,pcm.data(),pcm.size()); for(auto p:pcm) assert(p==0);
        unsigned active=0;
        for(unsigned n=0;n<64;++n) {
            io.publish_patch(c.x,c.y,true,false,static_cast<mhac::Patch>(c.patch));
            engine.render(io,pcm.data(),pcm.size());
            for(auto p:pcm) { assert(p>=-8191&&p<=8191); active+=p!=0; }
        }
        assert(active>0);
    }
    std::puts("factory UI PASS: 24000 safe loads, 77924 coordinate checks, 12 actual DSP auditions");
}
