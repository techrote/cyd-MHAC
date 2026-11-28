#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
using L=GestureLoop;
static unsigned core() {
    unsigned checks=0;
    for(unsigned style=0;style<L::style_count;++style) for(uint32_t start:{0u,0xffffffe0u}) {
        L l; for(unsigned i=0;i<style;++i) l.nextStyle();
        l.toggle(); l.observe({0,4095,true},start); l.observe({1600,2495,true},start+16);
        l.observe({3200,895,true},start+32); l.observe({},start+49);
        const unsigned active=style==2?98:style==4?25:style==5?96:49;
        for(unsigned cycle=0;cycle<3;++cycle) for(unsigned p=0;p<active+100;++p) {
            const uint32_t now=start+49+cycle*(active+100)+p;
            const auto v=l.play(now), q=l.play(now,false,false);
            assert(v.gate==(p<active) && q.gate==v.gate);
            if(v.gate) {
                unsigned t=style==2?p/2:style==4?p*2:style==3?48-p:style==5 && p>=49?96-p:p;
                unsigned held=std::min(t/16,2u)*1600;
                unsigned x=(style==1||style==2||style==5)?std::min(t*100,3200u):held;
                assert(v.x==x && v.y==4095-x && q.x==held && q.y==4095-held);
            }
            if(style==5 && !v.gate) assert(v.x==0 && v.y==4095);
            ++checks;
        }
        l.nextStyle(); l.restartAfterRest(start+1000);
        assert(!l.play(start+1000).gate && !l.play(start+1099).gate && l.play(start+1100).gate);
        l.stop(); assert(!l.play(start+1101).gate);
    }
    // Full recording capacity, irregular sampling and maximum-duration turnaround.
    for(uint32_t start:{0u,0xfffff800u}) {
        L l; for(unsigned i=0;i<5;++i) l.nextStyle();
        l.toggle(); l.observe({0,4095,true},start);
        for(unsigned t=16;t<=4000;t+=16) l.observe({uint16_t(t),uint16_t(4095-t),true},start+t);
        l.observe({4095,0,true},start+4500); l.observe({},start+5000);
        assert(l.count()==251 && l.duration()==4000);
        for(unsigned p=0;p<8098;++p) {
            auto v=l.play(start+5000+p);
            const unsigned t=p<4000?p:p<7998?7998-p:0;
            assert(v.gate==(p<7998) && v.x==t && v.y==4095-t);
            ++checks;
        }
    }
    return checks;
}
static unsigned surface() {
    unsigned notes=0;
    for(unsigned mapping:{0u,3u,4u}) {
        PerformanceSurface ui; uint32_t t=0xffffff00u;
        auto touch=[&](bool down,int x,int y){ui.sample(down,x,y,t+=8);};
        auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
        tap(310,10); for(unsigned j=0;j<mapping;++j) tap(270,230);
        tap(160,10); assert(ui.loopState()==L::State::Armed);
        tap(184,10); assert(ui.loopStyle()==L::Style::Glide && ui.loopState()==L::State::Armed);
        touch(true,60,190); touch(true,60,190);
        for(unsigned j=0;j<20;++j) touch(true,60+int(j)*10,190-int(j)*7);
        touch(false,0,0); assert(ui.loopState()==L::State::Playing);
        mhac::Engine engine; mhac::Exchange io; std::array<int16_t,mhac::block> pcm{};
        for(unsigned style=0;style<L::style_count;++style) {
            touch(true,184,10); touch(true,310,10); assert(!ui.effective(t).gate && !ui.control().mute);
            touch(false,0,0); assert(ui.loopState()==L::State::Playing && !ui.effective(t).gate);
            const auto release=t;
            unsigned active=0,rest=0;
            for(unsigned k=0;k<160;++k) {
                t=release+k*mhac::block*1000/mhac::rate;
                ui.sample(false,0,0,t); auto c=ui.effective(t);
                if(t-release<100) assert(!c.gate);
                if(c.gate) {
                    ++active;
                    if(mapping==3) assert(mapPitch(c.x,Scale::Chromatic)==c.x);
                    if(mapping==4) assert(c.y%1365==0);
                } else ++rest;
                publishControl(io,c); engine.render(io,pcm.data(),pcm.size());
                for(auto p:pcm){assert(p>=-8191 && p<=8191); notes+=p!=0;}
            }
            assert(active && rest && !io.watchdog_trips.load());
            Frame f{}; f.loop_style=ui.loopStyle();
            std::array<uint16_t,width*strip> raster{};
            for(unsigned step:{1u,2u,4u}) {
                renderStrip(f,8,step,raster.data());
                for(unsigned j=0;j<L::style_count;++j) assert(raster[176+4*j]==(j==unsigned(f.loop_style)?rgb(160,202,236):rgb(65,80,94)));
            }
        }
        touch(true,184,10); ui.expire(t+101); assert(ui.control().mute && ui.loopState()==L::State::Idle);
        t+=102; touch(false,0,0); assert(ui.effective(t).mute && !ui.effective(t).gate);
        tap(310,10); tap(160,10); touch(true,100,100); touch(true,100,100);
        for(unsigned j=0;j<8;++j) touch(true,120,120);
        touch(false,0,0); touch(true,184,10); ui.observeWatchdog(1); touch(false,0,0);
        assert(ui.effective(t).mute && ui.loopState()==L::State::Idle);
    }
    return notes;
}
int main() {
    unsigned hit=0;
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) if(loopStyleSelector(x,y)) {
        ++hit; assert(!(x>=148 && x<172) && !sceneSelector(x,y) && !soundSelector(x,y) && !xyMapSelector(x,y) && !paletteSelector(x,y) && factoryAction(x,y)==FactoryAction::None);
    }
    assert(hit==576); const auto checks=core(),notes=surface(); assert(notes);
    std::printf("loop transport PASS: %u phase checks, %u active PCM samples, %u selector pixels, 324 indicator checks, sizeof loop=%zu surface=%zu\n",checks,notes,hit,sizeof(L),sizeof(PerformanceSurface));
}
