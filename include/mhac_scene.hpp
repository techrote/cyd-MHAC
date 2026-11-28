#pragma once
#include "mhac_visual_patch.hpp"
#include "mhac_palette.hpp"
#include "mhac_scale.hpp"
#include "mhac_xy_map.hpp"
namespace mhac::visual {
// Session-only visual recipes; no audio preset or persistence IDs are assigned.
struct Scene { const char* name; unsigned patch,mode; Palette palette; XYMap mapping; Scale scale; unsigned x,y; };
inline constexpr Scene scenes[]{
    {"Lantern Tide",1,diffusion_mode,Palette::Verdigris,XYMap::Lattice,Scale::MinorPentatonic,1365,2200},
    {"Cinder Clock",4,cellular_mode,Palette::Ember,XYMap::Rhythm,Scale::Chromatic,1365,1365},
    {"Silver Bells",6,chime_mode,Palette::Silver,XYMap::Lattice,Scale::MinorPentatonic,0,900},
    {"Memory Crossing",8,quotation_mode,Palette::Original,XYMap::Direct,Scale::Chromatic,1365,3000},
    {"Afterwater",5,5,Palette::Verdigris,XYMap::Fine,Scale::Free,683,1500},
    {"Dust Prism",2,caustic_mode,Palette::Ember,XYMap::Swapped,Scale::Free,2048,2000},
    {"Vowel Glass",mhac::formant_session_index,caustic_mode,Palette::Silver,XYMap::Direct,Scale::Free,1365,2200},
};
constexpr unsigned scene_count=sizeof(scenes)/sizeof(scenes[0]);
// Preserve the original six marker positions; the seventh uses the final 4-pixel slot.
constexpr unsigned sceneMarkerX(unsigned i) noexcept { return 40+8*i-(i==6?4:0); }
constexpr bool sceneSelector(int x,int y) noexcept { return x>=40 && x<88 && y>=216 && y<240; }
constexpr unsigned sceneIndex(unsigned patch,unsigned mode,Palette palette,XYMap mapping,Scale scale) noexcept {
    for(unsigned i=0;i<scene_count;++i) {
        const auto& s=scenes[i];
        if(s.patch==patch && s.mode==mode && s.palette==palette && s.mapping==mapping && s.scale==scale) return i;
    }
    return scene_count;
}
constexpr bool validScenes() noexcept {
    for(unsigned i=0;i<scene_count;++i) {
        const auto& s=scenes[i];
        if(s.patch>=sound_count || s.mode>=mode_count || unsigned(s.palette)>=palette_count ||
           unsigned(s.mapping)>=xy_map_count || unsigned(s.scale)>=scale_count || s.x>4095 || s.y>4095 ||
           sceneIndex(s.patch,s.mode,s.palette,s.mapping,s.scale)!=i) return false;
    }
    return true;
}
static_assert(scene_count==7 && validScenes(),"scene layout/contract invalid");
}
