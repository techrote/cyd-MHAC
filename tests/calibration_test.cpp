#include "mhac_calibration.hpp"
#include <cassert>
#include <iostream>
using namespace mhac::visual;
using W=CalibrationWizard;
static W::Point raw(const Calibration& c,W::Point p) {
    return {c.top+(c.bottom-c.top)*p.y/239,c.left+(c.right-c.left)*p.x/319};
}
static void contact(W& w,W::Point p,uint32_t& t) {
    for(unsigned i=0;i<W::required;++i) w.sample(true,p.x+int(i%3)-1,p.y,t+=8);
    assert(w.ready());
    w.sample(false,0,0,t+=8);
}
int main() {
    unsigned fits=0, points=0;
    for(int l=100;l<=500;l+=100) for(int r=3500;r<=4000;r+=100)
    for(int top=3500;top<=4000;top+=100) for(int bot=100;bot<=500;bot+=100) {
        Calibration c{l,r,top,bot}; W w; uint32_t t=0xfffffff0u;
        w.begin(t); w.sample(true,2000,2000,t+=8); assert(!w.ready());
        w.sample(false,0,0,t+=8);
        for(auto p:W::targets) contact(w,raw(c,p),t);
        assert(w.state()==W::State::Accepted); ++fits;
        for(int y=0;y<240;y+=17) for(int x=0;x<320;x+=19) {
            auto p=raw(c,{x,y}); int xx,yy; w.result().landscape(p.x,p.y,xx,yy);
            assert(std::abs(xx-x)<=2 && std::abs(yy-y)<=2); ++points;
        }
    }
    W w; uint32_t t=0; w.begin(t); w.sample(false,0,0,t+=8);
    for(int i=0;i<50;++i) w.sample(true,i%2?1000:2000,2000,t+=8);
    assert(!w.ready() && w.index()==0); w.sample(false,0,0,t+=8);
    for(int i=0;i<5;++i) w.sample(true,1000,1000,t+=8);
    w.sample(false,0,0,t+=8); assert(w.index()==0);
    contact(w,{1000,1000},t); contact(w,{1100,1100},t); contact(w,{1050,1050},t);
    assert(w.state()==W::State::Rejected);
    w.begin(t); w.sample(false,0,0,t+=8); Calibration c;
    contact(w,raw(c,W::targets[0]),t); contact(w,raw(c,W::targets[1]),t); contact(w,{800,800},t);
    assert(w.state()==W::State::Rejected);
    w.begin(t); w.expire(t+29999); assert(w.active()); w.expire(t+30000); assert(w.state()==W::State::Rejected);
    w.begin(t); w.cancel(); assert(w.state()==W::State::Idle);
    std::cout<<"PASS "<<fits<<" calibration fits, "<<points<<" mapped points; jitter, short touch, wrong span, wrong centre, timeout, cancel and wrap checks\n";
}
