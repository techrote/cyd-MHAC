#pragma once
#include <algorithm>
#include <cstdint>
namespace mhac::visual {
// UI-owned, single-gesture quotation; no allocation, audio or wall-clock I/O.
class GestureLoop {
public:
    enum class State : uint8_t { Idle, Armed, Recording, Playing };
    enum class Style : uint8_t { Original, Glide, Half, Reverse, Double, Pendulum };
    static constexpr unsigned style_count=6;
    struct Point { uint16_t x=0,y=0; bool gate=false; };
    static constexpr unsigned preview_capacity=16;
    struct Preview {
        struct Coordinate { uint16_t x=0,y=0; };
        Coordinate points[preview_capacity]{};
        unsigned count=0;
    };
    static constexpr uint32_t limit_ms=4000, interval_ms=16, rest_ms=100;
private:
    struct Entry { uint16_t t,x,y; };
    Entry points_[limit_ms/interval_ms+1]{};
    State state_=State::Idle;
    Style style_=Style::Original;
    uint32_t activeWall() const noexcept {
        if(style_==Style::Pendulum) return uint32_t(duration_)*2u-2u;
        return style_==Style::Half?uint32_t(duration_)*2u:style_==Style::Double?(uint32_t(duration_)+1u)/2u:duration_;
    }
    static uint16_t lerp(uint16_t a,uint16_t b,uint32_t n,uint32_t d) noexcept {
        return uint16_t((uint32_t(a)*(d-n)+uint32_t(b)*n+d/2u)/d);
    }
    uint16_t count_=0, duration_=0;
    uint32_t origin_=0;
    void append(uint16_t t,Point p) noexcept {
        if(count_>=sizeof(points_)/sizeof(points_[0])) return;
        points_[count_++]={t,uint16_t(std::min<unsigned>(p.x,4095)),uint16_t(std::min<unsigned>(p.y,4095))};
    }
public:
    State state() const noexcept { return state_; }
    Style style() const noexcept { return style_; }
    void nextStyle() noexcept { style_=static_cast<Style>((unsigned(style_)+1u)%style_count); }
    // Transport changes restart from a silent 100 ms rest on contact release.
    void restartAfterRest(uint32_t now) noexcept { if(state_==State::Playing) origin_=now-activeWall(); }
    unsigned count() const noexcept { return count_; }
    unsigned duration() const noexcept { return duration_; }
    // Snapshot in recorded order, never changes playback phase or the audio gate.
    Preview preview() const noexcept {
        Preview v{};
        v.count=std::min<unsigned>(count_,preview_capacity);
        for(unsigned i=0;i<v.count;++i) {
            const unsigned at=v.count==1?0:i*(count_-1)/(v.count-1);
            v.points[i]={points_[at].x,points_[at].y};
        }
        return v;
    }
    void stop() noexcept { state_=State::Idle; count_=duration_=0; }
    void toggle() noexcept { if(state_==State::Idle) state_=State::Armed; else stop(); }
    void observe(Point p,uint32_t now) noexcept {
        if(state_==State::Armed && p.gate) {
            state_=State::Recording; origin_=now; append(0,p); return;
        }
        if(state_!=State::Recording) return;
        const uint32_t elapsed=std::min(now-origin_,limit_ms);
        if(!p.gate) {
            if(elapsed<32) { stop(); return; }
            duration_=uint16_t(elapsed); origin_=now; state_=State::Playing; return;
        }
        if(elapsed>=uint32_t(points_[count_-1].t)+interval_ms) append(uint16_t(elapsed),p);
    }
    Point play(uint32_t now,bool smoothX=true,bool smoothY=true) noexcept {
        if(state_!=State::Playing) return {};
        const uint32_t active=activeWall(), period=active+rest_ms;
        const uint32_t elapsed=now-origin_, phase=elapsed%period;
        origin_+=elapsed-phase; // rebase each cycle, including millisecond wrap
        const bool reverse=style_==Style::Reverse, pendulum=style_==Style::Pendulum;
        if(phase>=active) { const auto& p=points_[(reverse || pendulum)?0:count_-1]; return {p.x,p.y,false}; }
        uint32_t t=style_==Style::Half?phase/2u:style_==Style::Double?phase*2u:phase;
        if(reverse) t=uint32_t(duration_)-1u-t;
        if(pendulum && t>=duration_) t=active-t;
        unsigned lo=0,hi=count_;
        while(lo+1<hi) { const unsigned mid=(lo+hi)/2; if(points_[mid].t<=t) lo=mid; else hi=mid; }
        const auto& a=points_[lo];
        if((style_==Style::Glide || style_==Style::Half || pendulum) && lo+1<count_) {
            const auto& b=points_[lo+1];
            const uint32_t n=t-a.t,d=b.t-a.t;
            return {smoothX?lerp(a.x,b.x,n,d):a.x,smoothY?lerp(a.y,b.y,n,d):a.y,true};
        }
        return {a.x,a.y,true};
    }
};
constexpr const char* loopStyleName(GestureLoop::Style s) noexcept {
    switch(s) {
    case GestureLoop::Style::Glide: return "Glide";
    case GestureLoop::Style::Half: return "Half glide";
    case GestureLoop::Style::Reverse: return "Reverse";
    case GestureLoop::Style::Double: return "Double";
    case GestureLoop::Style::Pendulum: return "Pendulum";
    default: return "Original";
    }
}
constexpr bool loopStyleSelector(int x,int y) noexcept { return x>=176 && x<200 && y>=0 && y<24; }
}
