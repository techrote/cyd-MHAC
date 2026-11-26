#pragma once
#include <algorithm>
namespace mhac::visual {
enum class Palette : unsigned { Original, Ember, Verdigris, Silver };
constexpr unsigned palette_count=4;
constexpr Palette nextPalette(Palette p) noexcept {
    return static_cast<Palette>((unsigned(p)+1)%palette_count);
}
constexpr const char* paletteName(Palette p) noexcept {
    switch(p) {
    case Palette::Ember: return "Ember";
    case Palette::Verdigris: return "Verdigris";
    case Palette::Silver: return "Silver";
    default: return "Original";
    }
}
constexpr bool paletteSelector(int x,int y) noexcept {
    return x>=208 && x<256 && y>=216 && y<240;
}
struct PaletteColor { int r,g,b; };
// Decorative RGB mapping only. No signal or UI-state colour interpretation.
// Original is an exact passthrough; other palettes use bounded integer mixes.
constexpr PaletteColor paletteColor(Palette p,int r,int g,int b) noexcept {
    if(p==Palette::Original || unsigned(p)>=palette_count) return {r,g,b};
    r=std::clamp(r,0,255); g=std::clamp(g,0,255); b=std::clamp(b,0,255);
    const int peak=std::max(r,std::max(g,b));
    switch(p) {
    case Palette::Ember: return {peak,(r+2*g+b)/6,(r+g+2*b)/10};
    case Palette::Verdigris: return {(r+2*b)/5,peak,(2*r+g+b)/4};
    case Palette::Silver: {
        const int grey=(peak+(r+2*g+b)/4)/2;
        return {grey,grey,grey};
    }
    default: return {r,g,b};
    }
}
constexpr PaletteColor paletteSwatch(Palette p) noexcept {
    switch(p) {
    case Palette::Ember: return {240,130,55};
    case Palette::Verdigris: return {60,225,170};
    case Palette::Silver: return {205,205,205};
    default: return {130,180,210};
    }
}
}
