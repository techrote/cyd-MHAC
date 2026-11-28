#pragma once
#include <algorithm>
namespace mhac::visual {
enum class XYMap : unsigned { Direct, Swapped, Fine, Lattice, Rhythm };
constexpr unsigned xy_map_count=5;
constexpr XYMap nextXYMap(XYMap m) noexcept {
    return static_cast<XYMap>((unsigned(m)+1)%xy_map_count);
}
constexpr const char* xyMapName(XYMap m) noexcept {
    switch(m) {
    case XYMap::Swapped: return "Swapped";
    case XYMap::Fine: return "Fine relative";
    case XYMap::Lattice: return "Note lattice";
    case XYMap::Rhythm: return "Rhythm bands";
    default: return "Direct";
    }
}
constexpr bool xyMapSelector(int x,int y) noexcept {
    return x>=256 && x<296 && y>=216 && y<240;
}
// Four stable timbre targets. Erasure Pulse interprets these as four degrees
// of motif erasure; the audio clock is never driven by the UI.
constexpr unsigned rhythmBand(int y) noexcept { return unsigned(239-std::clamp(y,0,239))/60u; }
constexpr unsigned rhythmValue(unsigned band) noexcept { return std::min(band,3u)*1365u; }
constexpr unsigned rhythmNearest(unsigned value) noexcept { return (std::min(value,4095u)+682u)/1365u; }
constexpr int rhythmCenter(unsigned band) noexcept { return 239-int(std::min(band,3u)*60u+30u); }
constexpr unsigned rhythmTracked(int y,unsigned previous,bool fresh) noexcept {
    const int up=239-std::clamp(y,0,239);
    const int base=int(std::min(previous,3u)*60u);
    return !fresh && previous<4 && up>=base-3 && up<base+63?previous:rhythmBand(y);
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
