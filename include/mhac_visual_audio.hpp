#pragma once
#include "mhac_visual.hpp"
#include "mhac_audio.hpp"
namespace mhac::visual {
static_assert(mhac::contract_version==1 && mhac::patch_version==1 && mhac::patch_count==sound_count,"patch contract pin required");
inline void publishControl(mhac::Exchange& io, const Control& c) noexcept {
    io.publish_patch(c.x,c.y,c.gate,c.mute,static_cast<mhac::Patch>(c.patch));
}
}
