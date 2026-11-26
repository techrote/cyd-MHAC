#include "mhac_performance.hpp"
#include "mhac_visual_audio.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
int main(){
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
