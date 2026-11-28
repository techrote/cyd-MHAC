#include "mhac_glyph.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    unsigned edges=0,active=0;
    for(int x=16;x<320;x+=16) for(int y=0;y<240;++y) {
        const bool left=glyphInk(x-1,y)!=0,right=glyphInk(x,y)!=0;
        assert(left==right); active+=left; ++edges;
    }
    for(int y=16;y<240;y+=16) for(int x=0;x<320;++x) {
        const bool top=glyphInk(x,y-1)!=0,bottom=glyphInk(x,y)!=0;
        assert(top==bottom); active+=top; ++edges;
    }
    assert(active>0&&active<edges);
    uint64_t hashes[6]{}; unsigned checks=0;
    for(unsigned n=0;n<6;++n) {
        const int px=n==0?0:n==1?319:n==2?70:160,py=n==0?0:n==1?239:n==2?180:120;
        const int a=n==3?0:255,ph=n==4?255:17,t=n==4?255:23,depth=n==5?0:127;
        std::ofstream out;
        if(argc==2) {out.open(std::string(argv[1])+std::to_string(n)+".ppm",std::ios::binary); assert(out);out<<"P6\n320 240\n255\n";}
        uint64_t h=14695981039346656037ull;
        for(int y=0;y<240;++y) for(int x=0;x<320;++x) {
            const auto c=glyphShade(x,y,px,py,a,ph,t,depth);
            const auto q=glyphShade(x,y,px,py,a,ph,t,depth);
            assert(c.r==q.r&&c.g==q.g&&c.b==q.b);
            assert(c.r>=0&&c.r<=255&&c.g>=0&&c.g<=255&&c.b>=0&&c.b<=255);
            const uint16_t v=uint16_t((c.r>>3)<<11 | (c.g>>2)<<5 | (c.b>>3));
            h=(h^v)*1099511628211ull; ++checks;
            if(out.is_open()) {const unsigned char rgb[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)};out.write(reinterpret_cast<const char*>(rgb),3);}
        }
        for(unsigned j=0;j<n;++j) assert(hashes[j]!=h);
        hashes[n]=h; if(out.is_open()) assert(out);
        std::printf("frame %u hash %016llx\n",n,static_cast<unsigned long long>(h));
    }
    std::printf("PASS %u boundary comparisons (%u active), %u deterministic bounded RGB565 pixels\n",edges,active,checks);
}
