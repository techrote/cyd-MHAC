#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include "mhac_calibration.hpp"
#include "mhac_palette.hpp"
#include "mhac_xy_map.hpp"
#include "mhac_lattice.hpp"
#include "mhac_ink.hpp"
#include "mhac_cellular.hpp"
#include "mhac_diffusion.hpp"
#include "mhac_scale.hpp"
#include "mhac_trace.hpp"
#include "mhac_gesture.hpp"
#include "mhac_factory_ui.hpp"
#include "mhac_visual_patch.hpp"
#include "mhac_scene.hpp"
namespace mhac::visual {
constexpr int width=320, height=240, strip=8;
class Surface {
    Control c_{}; unsigned mode_=0, count_=0; bool held_=false, blocked_=false;
    uint32_t last_=0, trips_=0; bool watchdog_=false;
    Scale scale_=Scale::Free; unsigned raw_x_=2048;
    Palette palette_=Palette::Original;
    XYMap mapping_=XYMap::Direct; XYMapper mapper_; LatticeCell lattice_{}; unsigned rhythm_=0;
    FactorySelection factory_; bool audition_=false;
public:
    Control control() const noexcept { return c_; }
    unsigned mode() const noexcept { return mode_; }
    Scale scale() const noexcept { return scale_; }
    Palette palette() const noexcept { return palette_; }
    XYMap mapping() const noexcept { return mapping_; }
    unsigned factoryIndex() const noexcept { return factory_.index(); }
    bool factoryAudition() const noexcept { return audition_ && c_.gate; }
    bool watchdogLatched() const noexcept { return watchdog_; }
    void observeWatchdog(uint32_t trips) noexcept {
        if(trips!=trips_) { trips_=trips; watchdog_=true; forceMute(); }
    }
    void forceMute() noexcept { c_.mute=true; c_.gate=false; blocked_=true; count_=0; audition_=false; }
    void expire(uint32_t now) noexcept {
        if(held_ && uint32_t(now-last_)>100) { forceMute(); held_=false; }
    }
    void sample(bool down,int x,int y,uint32_t now) noexcept {
        expire(now); last_=now;
        if(!down) { held_=blocked_=audition_=false; count_=0; c_.gate=false; return; }
        x=std::clamp(x,0,width-1); y=std::clamp(y,0,height-1);
        if(blocked_) return;
        const bool fresh=!held_; held_=true;
        if(fresh && y<24 && (x<24 || x>=width-24)) {
            if(x<24) mode_=(mode_+1)%mode_count; else { c_.mute=!c_.mute; if(!c_.mute) watchdog_=false; }
            c_.gate=false; blocked_=true; return;
        }
        if(fresh && xyMapSelector(x,y)) {
            mapping_=nextXYMap(mapping_); c_.gate=false; blocked_=true; return;
        }
        if(fresh && paletteSelector(x,y)) {
            palette_=nextPalette(palette_); c_.gate=false; blocked_=true; return;
        }
        if(fresh && sceneSelector(x,y)) {
            const auto i=sceneIndex(c_.patch,mode_,palette_,mapping_,scale_);
            const auto& scene=scenes[i==scene_count?0:(i+1)%scene_count];
            c_.patch=scene.patch; mode_=scene.mode; palette_=scene.palette;
            mapping_=scene.mapping; scale_=scene.scale; raw_x_=scene.x;
            c_.x=mapPitch(raw_x_,scale_); c_.y=scene.y;
            factory_=FactorySelection{}; c_.gate=false; blocked_=true; return;
        }
        if(fresh && soundSelector(x,y)) {
            c_.patch=(c_.patch+1)%sound_count; mode_=paired_mode[c_.patch];
            c_.gate=false; blocked_=true; return;
        }
        if(fresh && x>=width-24 && y>=height-24) {
            scale_=static_cast<Scale>((unsigned(scale_)+1)%scale_count);
            c_.x=mapPitch(raw_x_,scale_); c_.gate=false; blocked_=true; return;
        }
        if(fresh) {
            const auto action=factoryAction(x,y);
            if(action==FactoryAction::Select) {
                factory_.next(); factory_.load(c_); raw_x_=c_.x;
                mode_=paired_mode[c_.patch]; blocked_=true; return;
            }
            if(action==FactoryAction::Audition) {
                factory_.ensure(); factory_.load(c_); raw_x_=c_.x;
                audition_=true; count_=1; return;
            }
        }
        if(audition_) {
            c_.gate=!c_.mute; return;
        }
        if(mapping_==XYMap::Lattice) {
            const auto cell=latticeTrackedCell(x,y,scale_,lattice_,count_==0);
            lattice_=cell;
            raw_x_=c_.x=pitchCoordinate(cell.note);
            if(count_<2) ++count_;
            c_.y=count_==1?cell.timbre:(c_.y+cell.timbre*3u+2u)/4u;
            c_.gate=count_>=2 && !c_.mute;
            return;
        }
        if(mapping_==XYMap::Rhythm) {
            rhythm_=rhythmTracked(y,rhythm_,count_==0);
            const auto point=touchPoint(x,y);
            if(count_<2) ++count_;
            raw_x_=count_==1?point.x:(raw_x_+point.x*3u+2u)/4u;
            c_.x=mapPitch(raw_x_,scale_); c_.y=rhythmValue(rhythm_);
            c_.gate=count_>=2 && !c_.mute;
            return;
        }
        const auto point=touchPoint(x,y);
        if(fresh) mapper_.begin(point,{raw_x_,c_.y});
        const auto target=mapper_.target(mapping_,point);
        if(count_<2) ++count_;
        const unsigned tx=target.x, ty=target.y;
        if(count_==1) { raw_x_=tx; c_.y=ty; }
        else { raw_x_=(raw_x_+tx*3+2)/4; c_.y=(c_.y+ty*3+2)/4; }
        c_.x=mapPitch(raw_x_,scale_);
        c_.gate=count_>=2 && !c_.mute;
    }
};
struct Frame {
    Control control{}; unsigned mode=0; uint32_t analysis=0, tick=0; bool watchdog=false; const InkField* ink=nullptr; Scale scale=Scale::Free; const Trace* trace=nullptr; bool trace_fresh=false; GestureLoop::State loop_state=GestureLoop::State::Idle;
    unsigned factory_index=factory_count; bool factory_audition=false;
    Palette palette=Palette::Original;
    XYMap mapping=XYMap::Direct;
    const CellularField* cellular=nullptr;
    const DiffusionField* diffusion=nullptr;
    GestureLoop::Style loop_style=GestureLoop::Style::Original;
};
inline int tri(int v) noexcept { const unsigned u=unsigned(v)&255u; return int(u<128?u*2:(255-u)*2); }
inline uint16_t rgb(int r,int g,int b) noexcept {
    return uint16_t((std::clamp(r,0,255)>>3)<<11 | (std::clamp(g,0,255)>>2)<<5 | (std::clamp(b,0,255)>>3));
}
inline LatticeCursor frameCursor(const Frame& f) noexcept {
    if(f.mapping==XYMap::Lattice && !f.factory_audition)
        return latticeCursor(f.control.x,f.control.y,f.scale);
    if(f.mapping==XYMap::Rhythm && !f.factory_audition)
        return {int(f.control.x*319/4095),rhythmCenter(rhythmNearest(f.control.y))};
    return {int(f.control.x*319/4095),239-int(f.control.y*239/4095)};
}
inline uint16_t pixelAt(const Frame& f,int x,int y,LatticeCursor cursor) noexcept {
    const int px=cursor.x, py=cursor.y;
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
    } else if(f.mode==2) { // Glyph: quantised phase weave; abstract, not decoded symbols.
        const int u=x/12, w=y/12, local=(x%12-6)*(x%12-6)+(y%12-6)*(y%12-6);
        const int k=tri(u*19+w*31+t+ph), h=tri(u*47-w*13-t);
        v=(local<10+(k>>4) && ((k^h)&64))?k:tri(x+y+t)/12;
        r=v; g=v*(110+a/2)/255; b=v/5+a/8;
    } else if(f.mode==3) { // Orbit: curved resonant shells centred on the gesture.
        const int radius=(dx*dx+dy*dy)/96;
        const int twist=tri((dx+dy)*2+t*2)-128;
        const int shell=tri(radius-t*4+ph+twist/4);
        const int veil=tri((dx-dy)*3+t+ph);
        v=std::max(0,shell-180)*3;
        r=v*(160+a/3)/255; g=(v+veil/7)*(110+a/2)/255; b=veil/5+v/4;
    } else if(f.mode==4) { // Loom: warped threads with an interleaved colour structure.
        const int u=dx+ (tri(dy*3+t+ph)-128)/3;
        const int w=dy+ (tri(dx*2-t*2)-128)/3;
        const int warp=tri(u*11+ph), weft=tri(w*13-t*2);
        const bool over=((unsigned(u/12)^unsigned(w/12))&1u)!=0;
        const int thread=over?warp:weft, under=over?weft:warp;
        v=std::max(0,thread-150)*2;
        r=(over?v/5:v)+under/10; g=v*(95+a/2)/255; b=(over?v:v/4)+under/12;
    } else if(f.mode==5) { // Ink: persistent gesture energy, not an audio waveform.
        const auto s=f.ink?f.ink->shade(x,y):InkField::Shade{5,12,22};
        r=s.r;g=s.g;b=s.b;
    } else if(f.mode==chime_mode) { // Chime: stylized resonating plates.
        const auto s=chimeShade(dx,dy,f.control.y,f.analysis&4095u,unsigned(ph),f.tick);
        r=s.r;g=s.g;b=s.b;
    } else if(f.mode==caustic_mode) { // Caustic: intersecting curved light sheets.
        // Decorative phase/peak modulation, not an acoustic field measurement.
        const int u=x-160, w=y-120;
        const int bend=w*w/96;
        const int left=tri(u*3+bend+t*2+ph);
        const int right=tri(u*3-bend-t*2+int(f.control.y>>5));
        const int ridge=std::max(0,std::min(left,right)-120);
        const int ring=std::max(0,tri((dx*dx+dy*dy)/160-t*2+ph)-204);
        const int glow=ridge*(80+a)/128;
        r=6+glow/3+ring;
        g=10+glow+ring/2;
        b=22+glow*3/4+ring*2;
    } else if(f.mode==quotation_mode) { // Palimpsest: stylized recalled contours.
        // Procedural echoes of the current cursor; not a readout of DSP history.
        const int depth=int(f.control.y>>4);
        const int bend=dy*dy/180;
        const int live=tri(dx*6+bend+ph);
        const int echo=tri((dx-20-depth/8)*6-bend+t*2);
        const int ink=std::max(0,live-212)*5;
        const int ghost=std::max(0,echo-196)*depth/64;
        const int grain=tri(dy*7+t)/24;
        r=5+ghost+ink/5; g=9+ink*(96+a/2)/255+ghost/3;
        b=18+ink+ghost*3/4+grain;
        if(f.control.patch==mhac::counterquotation_session_index) {
            // Complementary crossing contours: artistic counterpoint, not recalled pitch telemetry.
            const int counter=std::max(0,tri(dx*4+dy*3+bend-t*3)-208)*depth/64;
            r+=counter/3; g+=counter; b+=counter/4;
        }
    } else if(f.mode==cellular_mode) { // Excitable cellular field.
        const auto s=f.cellular?f.cellular->shade(x,y):CellularField::Shade{5,9,16};
        r=s.r;g=s.g;b=s.b;
    } else if(f.mode==diffusion_mode) { // Diffusion: two-species generative medium.
        const auto s=f.diffusion?f.diffusion->shade(x,y):DiffusionField::Shade{4,9,18};
        r=s.r;g=s.g;b=s.b;
    } else { // Trace: observed PCM feature history, not spectral bands.
        const auto s=f.trace?f.trace->shade(x,y):Trace::Shade{5,8,15};
        r=s.r;g=s.g;b=s.b;
    }
    // Preserve measured Trace colours and all performance/safety overlays.
    if(f.mode!=6 && f.palette!=Palette::Original) {
        const auto c=paletteColor(f.palette,r,g,b); r=c.r; g=c.g; b=c.b;
    }
    if(f.mapping==XYMap::Lattice) {
        const unsigned mark=latticeMark(x,y,f.scale);
        if(mark==2) { r=(r+100)/2; g=(g+190)/2; b=(b+210)/2; }
        if(mark==1) { r=(r+60)/2; g=(g+100)/2; b=(b+130)/2; }
    }
    // Static band landmarks and nominal motif weights, not a live beat meter.
    if(f.mapping==XYMap::Rhythm) {
        const unsigned band=rhythmBand(y);
        if(y%60<4) { r=(r+90)/2; g=(g+160)/2; b=(b+190)/2; }
        if(x>=8 && x<52 && y%60>=28 && y%60<36 && f.control.patch==unsigned(mhac::Patch::ErasurePulse)) {
            const unsigned onset=unsigned(x-8)/12u;
            if((x-8)%12<8) {
                // Nominal gain at each onset, drawn as brightness, not PCM.
                const unsigned gain=onset==0?255u:onset==1?255u-band*85u:onset==2?204u:166u-band*27u;
                r=int(gain/2u)+18; g=int(gain*3u/4u)+18; b=int(gain)+18;
            }
        }
    }
    const int d2=dx*dx+dy*dy;
    if(d2>=72 && d2<=125) { r=180;g=235;b=225; }
    if(f.control.gate && d2<16) { r=255;g=255;b=255; }
    if(f.mode==6 && x>=32 && x<48 && y>=4 && y<12) return f.trace_fresh?rgb(64,220,150):rgb(255,184,32);
    // Gesture quotation indicator and fresh-touch control, aligned to 4x shading.
    if(x>=152 && x<168 && y>=8 && y<16) {
        switch(f.loop_state) {
        case GestureLoop::State::Armed: return rgb(255,184,32);
        case GestureLoop::State::Recording: return rgb(255,72,48);
        case GestureLoop::State::Playing: return rgb(80,220,200);
        default: return rgb(65,80,94);
        }
    }
    // Five aligned transport bars; original/step, glide, half, reverse, double.
    if(x>=176 && x<196 && y>=8 && y<16) {
        const unsigned i=unsigned(x-176)/4;
        return i==unsigned(f.loop_style)?rgb(160,202,236):rgb(65,80,94);
    }
    // Tiny controls, visible without covering the performance surface.
    if(x<24 && y<24 && (x==4||x==19||y==4||y==19) && x>=4&&x<=19&&y>=4&&y<=19) return rgb(130,180,210);
    if(x>=296 && y<24 && x>=300&&x<=315&&y>=4&&y<=19 && (x<304||x>=312||y<8||y>=16|| (f.control.mute&&std::abs((x-300)-(y-4))<=2))) return f.control.mute?(f.watchdog?rgb(255,184,32):rgb(255,72,48)):rgb(64,220,150);
    // Eight aligned bars, plus three pitch-map indicators. Visible at 4x shading.
    if(x<int(sound_count*4) && y>=220 && y<236) {
        const unsigned index=unsigned(x)/4;
        if(x%4==3) return rgb(12,20,28);
        if(y>=228 || index==f.control.patch) return index==f.control.patch?rgb(238,220,128):rgb(65,80,94);
        return rgb(12,20,28);
    }
    if(x>=296 && y>=220 && y<236) {
        const unsigned index=unsigned(x-296)/8;
        if(x%8>=6) return rgb(12,20,28);
        if(y>=228 || index==unsigned(f.scale)) return index==unsigned(f.scale)?rgb(116,210,240):rgb(65,80,94);
        return rgb(12,20,28);
    }
    // Session scene bars, independent of the persisted sound-only factory bank.
    if(x>=40 && x<88 && y>=224 && y<236) {
        const unsigned i=unsigned(x-40)/8;
        const unsigned selected=sceneIndex(f.control.patch,f.mode,f.palette,f.mapping,f.scale);
        if(x%8>=4) return rgb(12,20,28);
        return i==selected?rgb(120,224,180):rgb(65,80,94);
    }
    // Separate factory cursor and hold-to-audition key, aligned to all quality tiers.
    if(x>=92 && x<156 && y>=224 && y<236) {
        const unsigned i=unsigned(x-92)/4;
        return i==f.factory_index?rgb(238,220,128):rgb(65,80,94);
    }
    if(x>=172 && x<188 && y>=224 && y<236)
        return f.factory_audition?rgb(64,220,150):rgb(110,156,184);
    if(x>=216 && x<248 && y>=224 && y<236) {
        const auto index=static_cast<Palette>(unsigned(x-216)/8);
        if(y<232 && index!=f.palette) return rgb(38,48,58);
        if(y>=232 && index!=f.palette) return rgb(12,20,28);
        const auto c=paletteSwatch(index); return rgb(c.r,c.g,c.b);
    }
    // Five aligned mapping bars: Direct, Swapped, Fine, Lattice, Rhythm.
    if(x>=256 && x<296 && y>=224 && y<236) {
        const unsigned i=unsigned(x-256)/8;
        return i==unsigned(f.mapping)?rgb(180,162,240):rgb(65,80,94);
    }
    return rgb(r,g,b);
}
inline uint16_t pixel(const Frame& f,int x,int y) noexcept { return pixelAt(f,x,y,frameCursor(f)); }
// Caller provides exactly width*strip words. Last strip is always full (240/8).
inline void renderStrip(const Frame& f,int y,unsigned step,uint16_t* out) noexcept {
    step=step==4?4:step==2?2:1;
    const auto cursor=frameCursor(f);
    for(int j=0;j<strip;j+=int(step)) {
        for(int x=0;x<width;x+=int(step)) {
            const auto c=pixelAt(f,x,y+j,cursor);
            for(unsigned dy=0;dy<step;++dy) for(unsigned dx=0;dx<step;++dx) out[(j+dy)*width+x+dx]=c;
        }
    }
}
}
