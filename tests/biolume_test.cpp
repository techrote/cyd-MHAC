#include "mhac_biolume.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    unsigned checks=0; uint64_t hashes[4]{};
    for(int p=9;p<=200;++p) for(int a=0;a<=255;++a) {
        const auto s=biolumeShade(319,239,0,0,p,a,255,255);
        assert(s.r>=0 && s.r<=255 && s.g>=0 && s.g<=255 && s.b>=0 && s.b<=255);
        if(p==9) assert(s.r==4 && s.g==8 && s.b==16);
        ++checks;
    }
    for(int n=0;n<4;++n) {
        std::ofstream out;
        if(argc==2) {out.open(std::string(argv[1])+std::to_string(n)+".ppm",std::ios::binary); assert(out); out<<"P6\n320 240\n255\n";}
        uint64_t h=14695981039346656037ull;
        for(int y=0;y<240;++y) for(int x=0;x<320;++x) {
            // Synthetic pigment field, not a simulation capture.
            const int pigment=9+std::max(0,biolumeTri((x-160)*(x-160)/110+(y-120)*(y-120)/80)-70);
            const auto s=biolumeShade(x,y,n==3?0:160,n==3?239:120,pigment,n==1?255:0,n==2?120:15,40);
            const auto t=biolumeShade(x,y,n==3?0:160,n==3?239:120,pigment,n==1?255:0,n==2?120:15,40);
            assert(s.r==t.r && s.g==t.g && s.b==t.b);
            assert(s.r>=0 && s.r<=255 && s.g>=0 && s.g<=255 && s.b>=0 && s.b<=255);
            const uint16_t v=uint16_t((s.r>>3)<<11 | (s.g>>2)<<5 | (s.b>>3));
            h=(h^v)*1099511628211ull; ++checks;
            if(out.is_open()) {const unsigned char c[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)};out.write(reinterpret_cast<const char*>(c),3);}
        }
        for(int i=0;i<n;++i) assert(hashes[i]!=h);
        hashes[n]=h; if(out.is_open()) assert(out);
        std::printf("frame %d %016llx\n",n,static_cast<unsigned long long>(h));
    }
    std::printf("PASS %u bounded/deterministic shade checks\n",checks);
}
