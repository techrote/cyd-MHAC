#pragma once
#include <algorithm>
namespace mhac::visual {
enum class XYMap : unsigned { Direct, Swapped, Fine };
constexpr unsigned xy_map_count=3;
constexpr XYMap nextXYMap(XYMap m) noexcept {
    return static_cast<XYMap>((unsigned(m)+1)%xy_map_count);
}
constexpr const char* xyMapName(XYMap m) noexcept {
    switch(m) {
    case XYMap::Swapped: return "Swapped";
    case XYMap::Fine: return "Fine relative";
    default: return "Direct";
    }
}
constexpr bool xyMapSelector(int x,int y) noexcept {
    return x>=256 && x<288 && y>=216 && y<240;
}
struct XYPoint { unsigned x,y; };
constexpr XYPoint touchPoint(int x,int y) noexcept {
    return {unsigned(std::clamp(x,0,319))*4095u/319u,
            unsigned(239-std::clamp(y,0,239))*4095u/239u};
}
class XYMapper {
    XYPoint anchor_{0,0}, base_{2048,2048};
    static constexpr unsigned fine(unsigned value,unsigned origin,unsigned base) noexcept {
        return unsigned(std::clamp(int(base)+(int(value)-int(origin))/2,0,4095));
    }
public:
    void begin(XYPoint p,XYPoint current) noexcept { anchor_=p; base_=current; }
    XYPoint target(XYMap m,XYPoint p) const noexcept {
        if(m==XYMap::Swapped) return {p.y,p.x};
        if(m==XYMap::Fine) return {fine(p.x,anchor_.x,base_.x),fine(p.y,anchor_.y,base_.y)};
        return p;
    }
};
}
