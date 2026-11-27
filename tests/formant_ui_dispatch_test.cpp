#include "mhac_visual_patch.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
using namespace mhac::visual;
int main() {
    mhac::Exchange ui, direct;
    assert(sound_count==10 && paired_mode[9]==caustic_mode && std::strcmp(soundName(9),"Formant Reed")==0);
    unsigned words=0, area=0;
    for(unsigned p=0;p<=sound_count;++p) for(bool mute:{false,true}) for(bool gate:{false,true}) {
        publishControl(ui,{1234,3456,gate,mute,p});
        if(p==6) direct.publish_modal(1234,3456,gate,mute);
        else if(p==7) direct.publish_quotation(1234,3456,gate,mute);
        else if(p==8) direct.publish_counterquotation(1234,3456,gate,mute);
        else if(p==9) direct.publish_formant(1234,3456,gate,mute);
        else direct.publish_patch(1234,3456,gate,mute,static_cast<mhac::Patch>(p));
        assert(ui.control.load()==direct.control.load()); ++words;
    }
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        const bool hit=soundSelector(x,y); assert(hit==(x>=0&&x<40&&y>=216&&y<240)); area+=hit;
    }
    mhac::Engine a,b; std::array<int16_t,mhac::block> x{},y{};
    unsigned count=0,active=0;
    for(unsigned k=0;k<512;++k) {
        const unsigned pitch=(k*29)%4096,timbre=(k*71)%4096;
        const bool gate=k<384,mute=k>=384;
        publishControl(ui,{pitch,timbre,gate,mute,9}); direct.publish_formant(pitch,timbre,gate,mute);
        a.render(ui,x.data(),x.size()); b.render(direct,y.data(),y.size());
        assert(x==y); count+=x.size();
        for(auto s:x) { assert(s>=-8191&&s<=8191); active+=s!=0; }
    }
    assert(active>0 && !ui.watchdog_trips.load()); for(auto s:x) assert(s==0);
    std::printf("formant UI dispatch PASS: %u wire words, %u selector pixels, %u exact PCM samples, %u active, final mute\n",words,area,count,active);
}
