#pragma once
#include "mhac_scale.hpp"
namespace mhac::visual {
// Three octaves, one cell per note; within each cell Y sweeps timbre.
// Free selects the chromatic lattice (not continuous pitch); other maps retain Free.
constexpr unsigned latticeColumns(Scale s) noexcept { return s==Scale::MinorPentatonic?5u:12u; }
constexpr unsigned latticeSemitone(unsigned col,Scale s) noexcept {
    constexpr unsigned minor[]={0,3,5,7,10};
    return s==Scale::MinorPentatonic?minor[std::min(col,4u)]:std::min(col,11u);
}
struct LatticeCell { unsigned col,octave,note,timbre; };
constexpr LatticeCell latticeCell(int x,int y,Scale s) noexcept {
    const unsigned up=unsigned(239-std::clamp(y,0,239));
    const unsigned col=unsigned(std::clamp(x,0,319))*latticeColumns(s)/320u;
    const unsigned octave=up/80u;
    return {col,octave,octave*12u+latticeSemitone(col,s),(up%80u)*4095u/79u};
}
struct LatticeCursor { int x,y; };
constexpr LatticeCursor latticeCursor(unsigned pitch,unsigned timbre,Scale s) noexcept {
    // Factory coordinates may be off-scale or at the top endpoint; the cursor
    // approximates the nearest available cell in that case, not a measurement.
    const unsigned note=std::min((std::min(pitch,4095u)*36u+2047u)/4095u,35u);
    const unsigned octave=note/12u, pc=note%12u;
    const unsigned col=s==Scale::MinorPentatonic?(pc>=9?4u:pc>=7?3u:pc>=5?2u:pc>=2?1u:0u):pc;
    return {int((2u*col+1u)*320u/(2u*latticeColumns(s))),
            239-int(octave*80u+(std::min(timbre,4095u)*79u+2047u)/4095u)};
}
// Four-pixel-wide marks survive 1x, 2x and 4x strip sampling.
// 2=octave divider, 1=note divider, 0=interior.
constexpr unsigned latticeMark(int x,int y,Scale s) noexcept {
    if(y%80<4) return 2;
    if(x<4 || unsigned(x)*latticeColumns(s)/320u!=unsigned(x-4)*latticeColumns(s)/320u) return 1;
    return 0;
}
}
