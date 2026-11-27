#include "mhac_fault_screen.hpp"
#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
int main() {
    std::array<uint16_t,width*strip+2> guarded{};
    unsigned red=0,light=0,amber=0,background=0;
    for(int y=0;y<height;y+=strip) {
        guarded.front()=0x1234; guarded.back()=0xabcd;
        renderFaultStrip(y,guarded.data()+1);
        assert(guarded.front()==0x1234 && guarded.back()==0xabcd);
        for(int j=0;j<strip;++j) for(int x=0;x<width;++x) {
            const auto p=guarded[1+j*width+x];
            if(x<4 || x>=width-4 || y+j<4 || y+j>=height-4) assert(p==rgb(255,72,48));
            if(p==rgb(255,72,48)) ++red;
            else if(p==rgb(255,224,200)) { assert(y+j>=101 && y+j<122); ++light; }
            else if(p==rgb(255,184,32)) { assert(y+j>=143 && y+j<164); ++amber; }
            else { assert(p==rgb(12,6,8)); ++background; }
        }
    }
    assert(red>4000 && light>1000 && amber>500 && background>60000);
    PerformanceSurface ui;
    ui.sample(true,310,10,8); ui.sample(false,0,0,16);
    ui.sample(true,160,10,24); ui.sample(false,0,0,32);
    ui.sample(true,80,100,40); ui.sample(true,80,100,48);
    ui.sample(true,120,130,80); ui.sample(false,0,0,100);
    assert(ui.loopState()==GestureLoop::State::Playing);
    ui.forceMute(); assert(ui.loopState()==GestureLoop::State::Idle);
    for(unsigned t=100;t<1000;t+=8) { ui.expire(t); ui.forceMute(); const auto c=ui.effective(t); assert(c.mute && !c.gate); }
    std::printf("fault screen PASS: %u red, %u title, %u reboot, %u background pixels; loop cancelled and mute maintained\n",red,light,amber,background);
}
