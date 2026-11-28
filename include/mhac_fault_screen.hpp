#pragma once
#include "mhac_visual.hpp"
namespace mhac::visual {
// Full-resolution terminal fault screen, independent of performance state and palette.
constexpr uint8_t faultGlyph(char c,unsigned row) noexcept {
    constexpr uint8_t glyphs[][7]={
        {14,17,17,31,17,17,17}, // A
        {17,17,17,17,17,17,14}, // U
        {30,17,17,17,17,17,30}, // D
        {31,4,4,4,4,4,31}, // I
        {14,17,17,17,17,17,14}, // O
        {31,16,16,30,16,16,16}, // F
        {16,16,16,16,16,16,31}, // L
        {31,4,4,4,4,4,4}, // T
        {30,17,17,30,20,18,17}, // R
        {31,16,16,30,16,16,31}, // E
        {30,17,17,30,17,17,30} // B
    };
    constexpr char letters[]="AUDIOFLTREB";
    for(unsigned i=0;i<sizeof(letters)-1;++i) if(letters[i]==c) return glyphs[i][row];
    return 0;
}
inline bool faultText(const char* text,unsigned length,int x,int y,int top) noexcept {
    constexpr int scale=3, cell=18;
    const int dx=x-(width-int(length)*cell+scale)/2,dy=y-top;
    if(dx<0 || dy<0 || dx>=int(length)*cell || dy>=7*scale || dx%cell>=5*scale) return false;
    return (faultGlyph(text[dx/cell],unsigned(dy/scale)) & (16u>>(dx%cell/scale)))!=0;
}
inline void renderFaultStrip(int top,uint16_t* out) noexcept {
    for(int j=0;j<strip;++j) for(int x=0;x<width;++x) {
        const int y=top+j;
        const bool border=x<4 || x>=width-4 || y<4 || y>=height-4;
        const bool mark=y>=40 && y<78 && x>=141 && x<179 &&
            (std::abs(x-141-(y-40))<4 || std::abs(178-x-(y-40))<4);
        out[j*width+x]=border || mark?rgb(255,72,48):
            faultText("AUDIO FAULT",11,x,y,101)?rgb(255,224,200):
            faultText("REBOOT",6,x,y,143)?rgb(255,184,32):rgb(12,6,8);
    }
}
}
