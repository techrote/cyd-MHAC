#pragma once
#include <algorithm>
#include <cstdint>
namespace mhac::visual {
// UI-owned, single-gesture quotation; no allocation, audio or wall-clock I/O.
class GestureLoop {
public:
    enum class State : uint8_t { Idle, Armed, Recording, Playing };
    struct Point { uint16_t x=0,y=0; bool gate=false; };
    static constexpr uint32_t limit_ms=4000, interval_ms=16, rest_ms=100;
private:
    struct Entry { uint16_t t,x,y; };
    Entry points_[limit_ms/interval_ms+1]{};
    State state_=State::Idle;
    uint16_t count_=0, duration_=0;
    uint32_t origin_=0;
    void append(uint16_t t,Point p) noexcept {
        if(count_>=sizeof(points_)/sizeof(points_[0])) return;
        points_[count_++]={t,uint16_t(std::min<unsigned>(p.x,4095)),uint16_t(std::min<unsigned>(p.y,4095))};
    }
public:
    State state() const noexcept { return state_; }
    unsigned count() const noexcept { return count_; }
    unsigned duration() const noexcept { return duration_; }
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
    Point play(uint32_t now) noexcept {
        if(state_!=State::Playing) return {};
        const uint32_t period=uint32_t(duration_)+rest_ms;
        const uint32_t elapsed=now-origin_, phase=elapsed%period;
        origin_+=elapsed-phase; // rebase each cycle, including millisecond wrap
        if(phase>=duration_) return {points_[count_-1].x,points_[count_-1].y,false};
        unsigned lo=0,hi=count_;
        while(lo+1<hi) { const unsigned mid=(lo+hi)/2; if(points_[mid].t<=phase) lo=mid; else hi=mid; }
        return {points_[lo].x,points_[lo].y,true};
    }
};
}
