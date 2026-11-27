#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
using L=GestureLoop;
int main() {
    static_assert(sizeof(L)==1516,"no new loop state");
    static_assert(sizeof(L::Preview)<=68,"bounded frame snapshot");
    unsigned checks=0,changed=0,notes=0;
    for(uint32_t start:{0u,0xfffff800u}) {
        L l; assert(l.preview().count==0); l.toggle(); assert(l.preview().count==0);
        l.observe({0,4095,true},start); assert(l.preview().count==1);
        for(unsigned t=16;t<=4000;t+=16) l.observe({uint16_t(t),uint16_t(4095-t),true},start+t);
        l.observe({},start+4001); assert(l.count()==251);
        const auto v=l.preview(); assert(v.count==16);
        for(unsigned i=0;i<v.count;++i) {
            const unsigned expected=i*250/15*16;
            assert(v.points[i].x==expected && v.points[i].y==4095-expected); ++checks;
        }
        for(unsigned i=0;i<L::style_count;++i) {
            const auto p=l.play(start+4001); const auto copy=l.preview(); const auto q=l.play(start+4001);
            assert(p.x==q.x && p.y==q.y && p.gate==q.gate && copy.count==16); l.nextStyle(); ++checks;
        }
        l.stop(); assert(l.preview().count==0 && v.count==16);
    }
    for(unsigned mapping=0;mapping<xy_map_count;++mapping) {
        PerformanceSurface ui; uint32_t t=0;
        auto touch=[&](bool down,int x,int y){ui.sample(down,x,y,t+=8);};
        auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
        tap(310,10); for(unsigned i=0;i<mapping;++i) tap(270,230);
        tap(160,10); touch(true,55,170); touch(true,55,170);
        for(unsigned i=0;i<40;++i) touch(true,55+int(i)*5,170-int(i)*3);
        touch(false,0,0); assert(ui.loopState()==L::State::Playing && ui.loopPreview().count==16);
        mhac::Engine engine; mhac::Exchange io; std::array<int16_t,mhac::block> pcm{};
        for(unsigned k=0;k<12;++k) {
            t+=8; auto c=ui.effective(t); publishControl(io,c); engine.render(io,pcm.data(),pcm.size());
            for(auto p:pcm) { assert(p>=-8191 && p<=8191); notes+=p!=0; }
        }
        Frame f{}; f.control=ui.effective(t); f.loop_state=ui.loopState(); f.mapping=ui.mapping(); f.scale=ui.scale(); f.loop_preview=ui.loopPreview();
        Frame baseline=f; baseline.loop_preview={};
        std::array<uint16_t,width*strip> a{},b{};
        unsigned mappingChanges=0;
        for(unsigned step:{1u,2u,4u}) for(int row=0;row<height;row+=strip) {
            renderStrip(f,row,step,a.data()); renderStrip(baseline,row,step,b.data());
            for(int i=0;i<width*strip;++i) if(a[i]!=b[i]) {
                const int x=i%width,y=row+i/width; const auto live=frameCursor(f);
                assert(x>=24 && x<=295 && y>=24 && y<=215);
                assert((x-live.x)*(x-live.x)+(y-live.y)*(y-live.y)>144);
                assert(!(mapping==4 && x<52 && y%60>=28 && y%60<36));
                ++mappingChanges;
            }
        }
        assert(mappingChanges>0); changed+=mappingChanges;
        f.control.mute=true; baseline.control.mute=true;
        for(int row=0;row<height;row+=strip) { renderStrip(f,row,1,a.data()); renderStrip(baseline,row,1,b.data()); assert(a==b); }
        ui.observeWatchdog(1); assert(ui.loopPreview().count==0 && ui.control().mute);
    }
    assert(notes && changed); std::printf("loop preview PASS: %u bounded sample checks, %u changed RGB565 pixels, %u active PCM samples, preview=%zu loop=%zu\n",checks,changed,notes,sizeof(L::Preview),sizeof(L));
}
