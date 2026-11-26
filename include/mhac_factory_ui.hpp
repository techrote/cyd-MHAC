#pragma once
#include "mhac_presets.hpp"
namespace mhac::visual {
constexpr unsigned factory_count=unsigned(mhac::factory_presets.size());
static_assert(factory_count==12,"factory UI layout requires 12 entries");
enum class FactoryAction { None, Select, Audition };
constexpr FactoryAction factoryAction(int x,int y) noexcept {
    if(y<216 || y>=240) return FactoryAction::None;
    if(x>=104 && x<160) return FactoryAction::Select;
    if(x>=168 && x<192) return FactoryAction::Audition;
    return FactoryAction::None;
}
class FactorySelection {
    unsigned index_=factory_count;
public:
    unsigned index() const noexcept { return index_; }
    void next() noexcept { index_=index_==factory_count?0:(index_+1)%factory_count; }
    void ensure() noexcept { if(index_==factory_count) index_=0; }
    const mhac::NamedPreset& entry() const noexcept {
        return mhac::factory_presets[index_<factory_count?index_:0];
    }
    template<class C> void load(C& c) const noexcept {
        const auto p=entry().sound;
        c.patch=p.patch; c.x=p.x; c.y=p.y; c.gate=false;
    }
};
}
