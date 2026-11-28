#include "mhac_palette.hpp"
#include <cassert>
#include <cstdio>
#include <cstdint>
using namespace mhac::visual;
int main() {
    Palette p=Palette::Original;
    for(unsigned i=0;i<4000;++i) { assert(unsigned(p)==i%palette_count); p=nextPalette(p); }
    unsigned hits=0;
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        const bool hit=paletteSelector(x,y);
        assert(hit==(x>=208&&x<256&&y>=216&&y<240)); hits+=hit;
    }
    assert(hits==1152);
    uint64_t sums[palette_count]{};
    for(int r=0;r<256;++r) for(int g=0;g<256;++g) for(int b=0;b<256;++b) {
        for(unsigned i=0;i<palette_count;++i) {
            const auto c=paletteColor(static_cast<Palette>(i),r,g,b);
            assert(c.r>=0&&c.r<=255&&c.g>=0&&c.g<=255&&c.b>=0&&c.b<=255);
            if(i==0) assert(c.r==r&&c.g==g&&c.b==b);
            if(i==1) assert(c.r>=c.g&&c.r>=c.b);
            if(i==2) assert(c.g>=c.r&&c.g>=c.b);
            if(i==3) assert(c.r==c.g&&c.g==c.b);
            sums[i]+=uint64_t((c.r<<16)|(c.g<<8)|c.b);
        }
    }
    for(unsigned i=0;i<palette_count;++i) {
        const auto black=paletteColor(static_cast<Palette>(i),0,0,0);
        assert(black.r==0&&black.g==0&&black.b==0);
        for(unsigned j=0;j<i;++j) assert(sums[i]!=sums[j]);
    }
    const auto original=paletteColor(Palette::Original,-32,270,999);
    assert(original.r==-32&&original.g==270&&original.b==999);
    for(unsigned i=1;i<palette_count;++i) {
        const auto c=paletteColor(static_cast<Palette>(i),-32,270,999);
        assert(c.r>=0&&c.r<=255&&c.g>=0&&c.g<=255&&c.b>=0&&c.b<=255);
    }
    std::printf("palette PASS: %u selector pixels, 4000 transitions, 67108864 RGB mappings\n",hits);
}
