#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
namespace mhac::visual {
constexpr int width=320, height=240, strip=8;
struct Control { unsigned x=2048,y=2048; bool gate=false,mute=true; };
struct Calibration {
    int left=200,right=3900,top=3900,bottom=200;
    static int map(int v,int a,int b,int n) noexcept {
        if(a==b) return 0;
        return std::clamp((std::clamp(v,0,4095)-a)*(n-1)/(b-a),0,n-1);
    }
    void landscape(int rawX,int rawY,int& x,int& y) const noexcept {
        x=map(rawY,left,right,width); y=map(rawX,top,bottom,height);
    }
};
class Surface {
    Control c_{}; unsigned mode_=0, count_=0; bool held_=false, blocked_=false;
    uint32_t last_=0;
public:
    Control control() const noexcept { return c_; }
    unsigned mode() const noexcept { return mode_; }
    void forceMute() noexcept { c_.mute=true; c_.gate=false; blocked_=true; count_=0; }
    void expire(uint32_t now) noexcept {
        if(held_ && uint32_t(now-last_)>100) { forceMute(); held_=false; }
    }
    void sample(bool down,int x,int y,uint32_t now) noexcept {
        expire(now); last_=now;
        if(!down) { held_=blocked_=false; count_=0; c_.gate=false; return; }
        x=std::clamp(x,0,width-1); y=std::clamp(y,0,height-1);
        if(blocked_) return;
        const bool fresh=!held_; held_=true;
        if(fresh && y<24 && (x<24 || x>=width-24)) {
            if(x<24) mode_=(mode_+1)%3; else c_.mute=!c_.mute;
            c_.gate=false; blocked_=true; return;
        }
        if(count_<2) ++count_;
        const unsigned tx=unsigned(x)*4095/(width-1), ty=unsigned(height-1-y)*4095/(height-1);
        if(count_==1) { c_.x=tx; c_.y=ty; }
        else { c_.x=(c_.x+tx*3+2)/4; c_.y=(c_.y+ty*3+2)/4; }
        c_.gate=count_>=2 && !c_.mute;
    }
};
struct Frame {
    Control control{}; unsigned mode=0; uint32_t analysis=0, tick=0;
};
inline int tri(int v) noexcept { const unsigned u=unsigned(v)&255u; return int(u<128?u*2:(255-u)*2); }
inline uint16_t rgb(int r,int g,int b) noexcept {
    return uint16_t((std::clamp(r,0,255)>>3)<<11 | (std::clamp(g,0,255)>>2)<<5 | (std::clamp(b,0,255)>>3));
}
inline uint16_t pixel(const Frame& f,int x,int y) noexcept {
    const int px=int(f.control.x*319/4095), py=239-int(f.control.y*239/4095);
    const int dx=x-px,dy=y-py, t=int(f.tick&255u), a=int((f.analysis&4095u)>>4), ph=int((f.analysis>>16)&255u);
    int v=0,r=0,g=0,b=0;
    if(f.mode==0) { // Resonance: two moving interference centres.
        const int d=std::abs(dx)+std::abs(dy), e=std::abs(x-160)+std::abs(y-120);
        v=(tri(d*7-t*3+ph)+tri(e*5+t*2)+tri((dx-dy)*3+ph))/3;
        r=v/7+a/5; g=v*(80+a/2)/255; b=v*(150+a/3)/255;
    } else if(f.mode==1) { // Prism: folded diamond lattice.
        const int u=std::abs(x-160), w=std::abs(y-120), p=std::min(u,w), q=std::max(u,w);
        v=tri((q+p)*4+t*2+ph) ^ tri((q-p)*9-int(f.control.y>>5));
        r=v*(130+a/2)/255; g=tri(v+int(f.control.x>>5))/3; b=v;
    } else { // Glyph: quantised phase weave; abstract, not decoded symbols.
        const int u=x/12, w=y/12, local=(x%12-6)*(x%12-6)+(y%12-6)*(y%12-6);
        const int k=tri(u*19+w*31+t+ph), h=tri(u*47-w*13-t);
        v=(local<10+(k>>4) && ((k^h)&64))?k:tri(x+y+t)/12;
        r=v; g=v*(110+a/2)/255; b=v/5+a/8;
    }
    const int d2=dx*dx+dy*dy;
    if(d2>=72 && d2<=125) { r=180;g=235;b=225; }
    if(f.control.gate && d2<16) { r=255;g=255;b=255; }
    // Tiny controls, visible without covering the performance surface.
    if(x<24 && y<24 && (x==4||x==19||y==4||y==19) && x>=4&&x<=19&&y>=4&&y<=19) return rgb(130,180,210);
    if(x>=296 && y<24 && x>=300&&x<=315&&y>=4&&y<=19 && (x<304||x>=312||y<8||y>=16|| (f.control.mute&&std::abs((x-300)-(y-4))<=2))) return f.control.mute?rgb(255,72,48):rgb(64,220,150);
    return rgb(r,g,b);
}
// Caller provides exactly width*strip words. Last strip is always full (240/8).
inline void renderStrip(const Frame& f,int y,unsigned step,uint16_t* out) noexcept {
    step=step==4?4:step==2?2:1;
    for(int j=0;j<strip;j+=int(step)) {
        for(int x=0;x<width;x+=int(step)) {
            const auto c=pixel(f,x,y+j);
            for(unsigned dy=0;dy<step;++dy) for(unsigned dx=0;dx<step;++dx) out[(j+dy)*width+x+dx]=c;
        }
    }
}
}
