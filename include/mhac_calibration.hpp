#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
namespace mhac::visual {
struct Calibration {
    int left=200,right=3900,top=3900,bottom=200;
    static int map(int v,int a,int b,int n) noexcept {
        if(a==b) return 0;
        return std::clamp((std::clamp(v,0,4095)-a)*(n-1)/(b-a),0,n-1);
    }
    void landscape(int rawX,int rawY,int& x,int& y) const noexcept {
        x=map(rawY,left,right,320); y=map(rawX,top,bottom,240);
    }
};
// Fixed orientation only: the current CYD adapter swaps X/Y and reverses raw X.
// A bad fit never replaces the previous calibration. No persistence or allocation.
class CalibrationWizard {
public:
    enum class State : uint8_t { Idle, Collecting, Accepted, Rejected };
    struct Point { int x,y; };
    static constexpr Point targets[3]={{32,32},{287,207},{160,120}};
    static constexpr unsigned required=6;
private:
    State state_=State::Idle;
    unsigned index_=0,count_=0;
    bool release_=true,ready_=false;
    uint32_t start_=0;
    Point first_{},sum_{},samples_[3]{};
    Calibration result_{};
    static int endpoint(int a,int b,int p,int q,int screen) noexcept {
        return a+(b-a)*(screen-p)/(q-p);
    }
    bool fit() noexcept {
        const auto a=samples_[0], b=samples_[1];
        if(b.y-a.y<1200 || a.x-b.x<1200) return false;
        Calibration c{endpoint(a.y,b.y,32,287,0),endpoint(a.y,b.y,32,287,319),
                      endpoint(a.x,b.x,32,207,0),endpoint(a.x,b.x,32,207,239)};
        for(int v:{c.left,c.right,c.top,c.bottom}) if(v< -256 || v>4351) return false;
        int x=0,y=0;
        c.landscape(samples_[2].x,samples_[2].y,x,y);
        if(std::abs(x-160)>12 || std::abs(y-120)>12) return false;
        result_=c; return true;
    }
    void resetSamples() noexcept { count_=0; ready_=false; sum_={0,0}; }
public:
    State state() const noexcept { return state_; }
    unsigned index() const noexcept { return index_; }
    bool ready() const noexcept { return ready_; }
    bool active() const noexcept { return state_==State::Collecting; }
    Calibration result() const noexcept { return result_; }
    void begin(uint32_t now) noexcept { state_=State::Collecting; index_=0; start_=now; release_=true; resetSamples(); }
    void cancel() noexcept { state_=State::Idle; resetSamples(); }
    void expire(uint32_t now) noexcept { if(active() && uint32_t(now-start_)>=30000u) state_=State::Rejected; }
    void sample(bool down,int rx,int ry,uint32_t now) noexcept {
        expire(now); if(!active()) return;
        if(!down) {
            if(release_) { release_=false; resetSamples(); return; }
            if(ready_) {
                if(++index_==3) state_=fit()?State::Accepted:State::Rejected;
            }
            resetSamples(); return;
        }
        if(release_) return;
        if(rx<0 || rx>4095 || ry<0 || ry>4095) { resetSamples(); return; }
        if(count_ && (std::abs(rx-first_.x)>48 || std::abs(ry-first_.y)>48)) resetSamples();
        if(ready_) return;
        if(!count_) first_={rx,ry};
        sum_.x+=rx; sum_.y+=ry;
        if(++count_==required) { samples_[index_]={sum_.x/int(required),sum_.y/int(required)}; ready_=true; }
    }
};
}
