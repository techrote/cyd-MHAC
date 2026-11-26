#include "mhac_performance.hpp"
#include "mhac_visual_audio.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
static void factoryIntegration() {
    PerformanceSurface ui; uint32_t t=0;
    auto touch=[&](bool d,int x,int y){ui.sample(d,x,y,t+=8);};
    assert(ui.factoryIndex()==factory_count);
    for(unsigned i=0;i<factory_count;++i) {
        touch(true,120,230); touch(true,220,80); touch(false,0,0);
        const auto p=factoryEntry(i).sound;
        const auto c=ui.control();
        assert(ui.factoryIndex()==i && c.patch==p.patch && c.x==p.x && c.y==p.y && c.mute && !c.gate && ui.mode()==paired_mode[p.patch]);
        touch(true,180,230); touch(true,200,80);
        assert(!ui.control().gate && !ui.factoryAudition());
        touch(false,0,0);
    }
    touch(true,310,10); touch(false,0,0);
    touch(true,160,10); touch(false,0,0); assert(ui.loopState()==GestureLoop::State::Armed);
    mhac::Engine synth; mhac::Exchange io; std::array<int16_t,mhac::block> pcm{};
    for(unsigned i=0;i<factory_count;++i) {
        touch(true,120,230); touch(false,0,0);
        assert(ui.loopState()==GestureLoop::State::Idle && ui.factoryIndex()==i && !ui.control().gate);
        touch(true,180,230); assert(!ui.control().gate);
        touch(true,220,120); assert(ui.factoryAudition());
        const auto p=factoryEntry(i).sound;
        const auto c=ui.effective(t);
        assert(c.gate && !c.mute && c.x==p.x && c.y==p.y && c.patch==p.patch);
        unsigned active=0;
        for(unsigned n=0;n<64;++n) {
            touch(true,230,100); publishControl(io,ui.effective(t));
            synth.render(io,pcm.data(),pcm.size());
            for(int x:pcm){assert(x>=-8191&&x<=8191); active+=x!=0;}
        }
        assert(active>0 && !io.watchdog_trips.load());
        touch(false,0,0); assert(!ui.control().gate && !ui.factoryAudition());
        publishControl(io,ui.effective(t)); synth.render(io,pcm.data(),pcm.size());
    }
    touch(true,180,230); touch(true,180,230); assert(ui.factoryAudition());
    ui.expire(t+101); assert(ui.control().mute && !ui.control().gate && !ui.factoryAudition());
    t+=102; touch(false,0,0); touch(true,310,10); touch(false,0,0);
    touch(true,180,230); touch(true,180,230); ui.observeWatchdog(1);
    assert(ui.control().mute && !ui.factoryAudition()); touch(false,0,0);
    Frame f{}; f.factory_audition=true;
    std::array<uint16_t,width*strip> pixels{};
    for(unsigned index=0;index<factory_count;++index) for(unsigned q:{1u,2u,4u}) {
        f.factory_index=index;
        renderStrip(f,224,q,pixels.data());
        for(unsigned k=0;k<factory_count;++k)
            assert(pixels[92+4*k]==(k==index?rgb(238,220,128):rgb(65,80,94)));
        assert(pixels[172]==rgb(64,220,150));
    }
    std::puts("factory integration PASS: 16 exact auditions, loop cancellation, stale/watchdog mute and 768 indicator checks");
}
static void mappingIntegration() {
    PerformanceSurface ui; uint32_t t=0;
    auto touch=[&](bool down,int x,int y){ui.sample(down,x,y,t+=8);};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
    assert(ui.mapping()==XYMap::Direct);
    tap(310,10);
    touch(true,120,80); touch(true,120,80);
    const auto pt=touchPoint(120,80);
    assert(ui.control().gate && ui.control().x==pt.x && ui.control().y==pt.y);
    touch(false,0,0);
    tap(160,10); assert(ui.loopState()==GestureLoop::State::Armed);
    const auto before=ui.control();
    touch(true,270,230); touch(true,100,100);
    assert(ui.mapping()==XYMap::Swapped && ui.loopState()==GestureLoop::State::Idle);
    assert(!ui.control().gate && !ui.control().mute && ui.control().x==before.x && ui.control().y==before.y);
    touch(false,0,0);
    touch(true,120,80); touch(true,120,80);
    assert(ui.control().gate && ui.control().x==pt.y && ui.control().y==pt.x);
    touch(false,0,0);
    tap(270,230); assert(ui.mapping()==XYMap::Fine);
    const auto anchored=ui.control();
    touch(true,50,150); touch(true,50,150);
    assert(ui.control().gate && ui.control().x==anchored.x && ui.control().y==anchored.y);
    touch(true,250,60);
    assert(ui.control().x>anchored.x && ui.control().y>anchored.y);
    touch(false,0,0);
    const auto lifted=ui.control();
    touch(true,90,90); touch(true,90,90);
    assert(ui.control().gate && ui.control().x==lifted.x && ui.control().y==lifted.y);
    ui.expire(t+101);
    assert(ui.control().mute && !ui.control().gate);
    t+=102; touch(false,0,0); tap(270,230); assert(ui.mapping()==XYMap::Direct && ui.control().mute);
    Frame f{}; std::array<uint16_t,width*strip> pixels{};
    for(unsigned i=0;i<xy_map_count;++i) for(unsigned q:{1u,2u,4u}) {
        f.mapping=static_cast<XYMap>(i);
        renderStrip(f,224,q,pixels.data());
        for(unsigned k=0;k<xy_map_count;++k)
            assert(pixels[260+8*k]==(i==k?rgb(180,162,240):rgb(65,80,94)));
    }
    std::puts("mapping integration PASS: silent selection, swapped XY, fine re-anchor, stale mute, 27 indicator checks");
}
int main(){
    factoryIntegration();
    mappingIntegration();
    PerformanceSurface ui; mhac::Exchange io; mhac::Engine synth;
    std::array<int16_t,mhac::block> pcm{};
    ui.sample(true,160,10,0); ui.sample(false,0,0,8); assert(ui.loopState()==GestureLoop::State::Idle);
    ui.sample(true,310,10,16); ui.sample(false,0,0,24);
    ui.sample(true,160,10,32); ui.sample(true,200,100,40); assert(!ui.control().gate); ui.sample(false,0,0,48);
    assert(ui.loopState()==GestureLoop::State::Armed);
    ui.sample(true,100,100,56); ui.sample(true,100,100,64); const auto first=ui.control();
    for(unsigned t=72;t<=128;t+=8) ui.sample(true,200,130,t);
    ui.sample(false,0,0,136); assert(ui.loopState()==GestureLoop::State::Playing);
    assert(ui.effective(136).gate && ui.effective(136).x==first.x && !ui.control().gate);
    unsigned active=0,rest=0; uint32_t nonzero=0;
    for(unsigned n=0;n<1000;++n){
        const uint32_t t=136+n*mhac::block*1000/mhac::rate;
        ui.sample(false,0,0,t); const auto c=ui.effective(t);
        if(c.gate) ++active; else ++rest;
        publishControl(io,c); synth.render(io,pcm.data(),pcm.size());
        for(auto p:pcm) { assert(p>=-8191 && p<=8191); nonzero+=(p!=0); }
    }
    assert(active && rest && nonzero && io.watchdog_trips.load()==0);
    ui.sample(true,190,100,5500); assert(ui.loopState()==GestureLoop::State::Idle && !ui.effective(5500).gate);
    ui.sample(true,190,100,5508); assert(ui.effective(5508).gate); ui.sample(false,0,0,5516);
    ui.sample(true,160,10,5524); ui.sample(false,0,0,5532);
    ui.sample(true,190,100,5540); ui.sample(true,190,100,5548);
    ui.expire(5650); assert(ui.control().mute && ui.loopState()==GestureLoop::State::Idle);
    ui.sample(false,0,0,5651); ui.sample(true,310,10,5660); ui.sample(false,0,0,5668);
    ui.sample(true,160,10,5676); ui.sample(false,0,0,5684);
    ui.sample(true,190,100,5692); ui.sample(true,190,100,5700); ui.sample(true,190,100,5732); ui.sample(false,0,0,5740);
    assert(ui.effective(5740).gate); ui.observeWatchdog(1); assert(ui.loopState()==GestureLoop::State::Idle && ui.effective(5740).mute);
    ui.forceMute(); publishControl(io,ui.effective(5740));
    for(unsigned i=0;i<128;++i) synth.render(io,pcm.data(),pcm.size());
    for(auto p:pcm) assert(p==0);
    std::array<uint16_t,width*strip> pixels{};
    Frame f{}; f.loop_state=GestureLoop::State::Playing;
    for(unsigned q:{1u,2u,4u}) { renderStrip(f,8,q,pixels.data()); assert(pixels[152]==rgb(80,220,200)); }
    std::printf("performance PASS: active blocks=%u rest=%u nonzero=%u\n",active,rest,nonzero);
}
