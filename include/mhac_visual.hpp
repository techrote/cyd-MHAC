#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include "mhac_calibration.hpp"
#include "mhac_palette.hpp"
#include "mhac_xy_map.hpp"
#include "mhac_rhythm_legend.hpp"
#include "mhac_lattice.hpp"
#include "mhac_ink.hpp"
#include "mhac_cellular.hpp"
#include "mhac_diffusion.hpp"
#include "mhac_biolume.hpp"
#include "mhac_scale.hpp"
#include "mhac_trace.hpp"
#include "mhac_gesture.hpp"
#include "mhac_trajectory.hpp"
#include "mhac_caustic.hpp"
#include "mhac_glyph.hpp"
#include "mhac_prism.hpp"
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
    GestureLoop::Preview loop_preview{};
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
    } else if(f.mode==1) { // Prism: nested optical facets centred on the gesture.
        const auto s=prismShade(x,y,px,py,a,ph,t,int(f.control.y>>5));
        r=s.r;g=s.g;b=s.b;
    } else if(f.mode==2) { // Glyph: stable connected routing motifs; artistic only.
        const auto s=glyphShade(x,y,px,py,a,ph,t,int(f.control.y>>5));
        r=s.r;g=s.g;b=s.b;
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
    } else if(f.mode==caustic_mode) { // Caustic: layered gesture-centred light sheets.
        const auto s=causticShade(x,y,px,py,a,ph,t,int(f.control.y>>5));
        r=s.r;g=s.g;b=s.b;
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
    } else if(f.mode==biolume_mode) { // Biolume: persistent pigment and artistic light sheets.
        const int pigment=f.diffusion?f.diffusion->shade(x,y).g:9;
        const auto s=biolumeShade(x,y,px,py,pigment,a,ph,t);
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
            // Static five-mark motif including the displaced amber fragment.
            const auto mark=rhythmLegend(x,band);
            r=mark.r; g=mark.g; b=mark.b;
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
    // Six aligned transport bars; original/step, glide, half, reverse, double, pendulum.
    if(x>=176 && x<200 && y>=8 && y<16) {
        const unsigned i=unsigned(x-176)/4;
        return i==unsigned(f.loop_style)?rgb(160,202,236):rgb(65,80,94);
    }
    // Y-only gesture overdub: amber when armed; available only during playback.
    if(x>=212 && x<228 && y>=8 && y<16 && f.loop_state==GestureLoop::State::Playing)
        return f.loop_preview.overdub?rgb(255,184,32):rgb(100,132,150);
    // Pitch overdub: cyan when armed, distinct from amber timbre.
    if(x>=244 && x<260 && y>=8 && y<16 && f.loop_state==GestureLoop::State::Playing)
        return f.loop_preview.pitchOverdub?rgb(80,220,240):rgb(100,132,150);
    // Tiny controls, visible without covering the performance surface.
    // Timbre mirror: two symmetric wings divided by a horizontal axis.
    if(x>=116 && x<132 && y>=4 && y<20 && f.loop_state==GestureLoop::State::Playing) {
        const int u=(x-116)/4, v=(y-4)/4;
        return (v==1 || v==2 || (u==1 || u==2))?rgb(230,170,96):rgb(12,20,28);
    }
    // Retrograde pitch: opposing arrows, only available during playback.
    if(x>=276 && x<292 && y>=5 && y<19 && f.loop_state==GestureLoop::State::Playing) {
        const int u=x-276,v=y-5;
        if(v==3 || v==10 || (u>=12 && std::abs(v-3)==15-u) ||
           (u<=3 && std::abs(v-10)==u)) return rgb(170,220,170);
    }
    if(x<24 && y<24 && (x==4||x==19||y==4||y==19) && x>=4&&x<=19&&y>=4&&y<=19) return rgb(130,180,210);
    if(x>=296 && y<24 && x>=300&&x<=315&&y>=4&&y<=19 && (x<304||x>=312||y<8||y>=16|| (f.control.mute&&std::abs((x-300)-(y-4))<=2))) return f.control.mute?(f.watchdog?rgb(255,184,32):rgb(255,72,48)):rgb(64,220,150);
    // Ten aligned bars, plus three pitch-map indicators. Visible at 4x shading.
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
        const unsigned selected=sceneIndex(f.control.patch,f.mode,f.palette,f.mapping,f.scale);
        for(unsigned i=0;i<scene_count;++i) {
            const int start=int(sceneMarkerX(i));
            if(x>=start && x<start+4) return i==selected?rgb(120,224,180):rgb(65,80,94);
        }
        return rgb(12,20,28);
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
// Decimated recorded control-coordinate path, not measured audio or raw touch.
// Full-resolution post-pass: at most 15 dashed links and 16 five-pixel beads.
inline void drawLoopPreview(const Frame& f,int row,uint16_t* out) noexcept {
    if(f.control.mute || (f.loop_state!=GestureLoop::State::Recording && f.loop_state!=GestureLoop::State::Playing)) return;
    const auto live=frameCursor(f);
    const unsigned count=std::min<unsigned>(f.loop_preview.count,GestureLoop::preview_capacity);
    const auto coordinate=[&](unsigned i) {
        const auto p=f.loop_preview.points[i];
        const unsigned x=std::min<unsigned>(p.x,4095), y=std::min<unsigned>(p.y,4095);
        return f.mapping==XYMap::Lattice?latticeCursor(x,y,f.scale):
            LatticeCursor{int(x*319/4095),f.mapping==XYMap::Rhythm?rhythmCenter(rhythmNearest(y)):239-int(y*239/4095)};
    };
    const auto safe=[&](int x,int y) {
        if(x<24 || x>295 || y<24 || y>215) return false;
        const int dx=x-live.x,dy=y-live.y;
        return dx*dx+dy*dy>144 && !(f.mapping==XYMap::Rhythm && x<52 && y%60>=28 && y%60<36);
    };
    if(count>1) {
        auto previous=coordinate(0);
        for(unsigned i=1;i<count;++i) {
            const auto q=coordinate(i);
            const unsigned phase=(i-1)*255u/(count-1);
            const auto link=rgb(160-int(phase)/3,100+int(phase)/3,65+int(phase)/2);
            trajectorySegment(previous.x,previous.y,q.x,q.y,row,strip,[&](int x,int y) {
                if(safe(x,y)) out[(y-row)*width+x]=link;
            });
            previous=q;
        }
    }
    for(unsigned i=0;i<count;++i) {
        const auto q=coordinate(i);
        if(q.y+2<row || q.y-2>=row+strip) continue;
        const unsigned phase=count>1?i*255u/(count-1):0u;
        const auto bead=rgb(255-int(phase)*3/5,160+int(phase)/3,64+int(phase)*3/4);
        for(int py=std::max({q.y-2,row,24});py<=std::min({q.y+2,row+strip-1,215});++py)
            for(int px=std::max(q.x-2,24);px<=std::min(q.x+2,295);++px) {
                // Keep the live cursor and rhythm legends intact.
                if(!safe(px,py)) continue;
                out[(py-row)*width+px]=(std::abs(px-q.x)==2 || std::abs(py-q.y)==2)?rgb(8,16,20):bead;
            }
    }
}
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
    drawLoopPreview(f,y,out);
}
}
