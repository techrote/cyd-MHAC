#pragma once
#include "mhac_visual.hpp"
namespace mhac::visual {
// Gesture controls are deliberately outside the audio contract.
class PerformanceSurface {
    Surface surface_;
    GestureLoop loop_;
    bool down_=false, consumed_=false, transport_=false;
    uint32_t transportLast_=0;
    void cancelIfMuted() noexcept { if(surface_.control().mute) loop_.stop(); }
public:
    Control control() const noexcept { return surface_.control(); }
    unsigned mode() const noexcept { return surface_.mode(); }
    Scale scale() const noexcept { return surface_.scale(); }
    Palette palette() const noexcept { return surface_.palette(); }
    XYMap mapping() const noexcept { return surface_.mapping(); }
    unsigned factoryIndex() const noexcept { return surface_.factoryIndex(); }
    bool factoryAudition() const noexcept { return surface_.factoryAudition(); }
    bool watchdogLatched() const noexcept { return surface_.watchdogLatched(); }
    GestureLoop::State loopState() const noexcept { return loop_.state(); }
    GestureLoop::Preview loopPreview() const noexcept { return loop_.preview(); }
    GestureLoop::Style loopStyle() const noexcept { return loop_.style(); }
    void expire(uint32_t now) noexcept {
        surface_.expire(now);
        if(transport_ && down_ && uint32_t(now-transportLast_)>100u) forceMute();
        cancelIfMuted();
    }
    void observeWatchdog(uint32_t trips) noexcept { surface_.observeWatchdog(trips); cancelIfMuted(); }
    void forceMute() noexcept { surface_.forceMute(); loop_.stop(); }
    void sample(bool down,int x,int y,uint32_t now) noexcept {
        expire(now);
        if(down && !down_) {
            if(loopStyleSelector(x,y)) {
                consumed_=transport_=true; transportLast_=now; loop_.nextStyle();
            } else {
                const bool wasPlaying=loop_.state()==GestureLoop::State::Playing;
                if(wasPlaying || sceneSelector(x,y) || xyMapSelector(x,y) || soundSelector(x,y) || factoryAction(x,y)!=FactoryAction::None) loop_.stop();
                if(x>=148 && x<172 && y>=0 && y<24) {
                    consumed_=true;
                    if(!wasPlaying && !surface_.control().mute) loop_.toggle();
                }
            }
        }
        down_=down;
        if(consumed_) {
            surface_.sample(false,0,0,now);
            if(transport_ && down) transportLast_=now;
            if(!down) {
                if(transport_) loop_.restartAfterRest(now);
                consumed_=transport_=false;
            }
            return;
        }
        surface_.sample(down,x,y,now);
        cancelIfMuted();
        const auto c=surface_.control();
        loop_.observe({uint16_t(c.x),uint16_t(c.y),c.gate},now);
    }
    Control effective(uint32_t now) noexcept {
        auto c=surface_.control();
        if(c.mute || down_ || loop_.state()!=GestureLoop::State::Playing) return c;
        const bool smoothX=surface_.scale()==Scale::Free && surface_.mapping()!=XYMap::Lattice;
        const bool smoothY=surface_.mapping()!=XYMap::Rhythm;
        const auto p=loop_.play(now,smoothX,smoothY);
        c.x=p.x; c.y=p.y; c.gate=p.gate;
        return c;
    }
};
}
