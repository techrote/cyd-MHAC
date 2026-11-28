#include "mhac_prism.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
// Exact predecessor Prism arithmetic, with its original fixed centre.
static PrismShade legacy(int x,int y,int a,int ph,int t,int depth) noexcept {
    const auto tri=[](int v) { const unsigned u=unsigned(v)&255u; return int(u<128?u*2:(255-u)*2); };
    const int u=std::abs(x-160),w=std::abs(y-120),p=std::min(u,w),q=std::max(u,w);
    const int v=tri((q+p)*4+t*2+ph)^tri((q-p)*9-depth);
    return {v*(130+a/2)/255,tri(v+64)/3,v};
}
int main(int argc,char** argv) {
    unsigned folds=0;
    for(unsigned p:{32u,64u,128u,256u}) for(int i=-2048;i<=2048;++i) {
        const int q=((i%int(p))+int(p))%int(p);
        const int expected=std::min(q,int(p)-q);
        assert(prismFold(i,p)==expected);
        assert(prismFold(-i,p)==expected); ++folds;
    }
    std::array<uint64_t,6> hashes{}; unsigned pixels=0;
    for(unsigned n=0;n<hashes.size();++n) {
        const int px=n==0?0:n==1?319:n==2?70:160,py=n==0?0:n==1?239:n==2?180:120;
        const int a=n==3?0:255,ph=n==4?255:17,t=n==4?255:23,depth=n==5?0:127;
        std::ofstream out;
        if(argc==2) {out.open(std::string(argv[1])+std::to_string(n)+".ppm",std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";}
        uint64_t h=14695981039346656037ull;
        for(int y=0;y<240;++y) for(int x=0;x<320;++x) {
            const auto c=prismShade(x,y,px,py,a,ph,t,depth);
            const auto repeat=prismShade(x,y,px,py,a,ph,t,depth);
            assert(c.r==repeat.r&&c.g==repeat.g&&c.b==repeat.b);
            assert(c.r>=0&&c.r<=255&&c.g>=0&&c.g<=255&&c.b>=0&&c.b<=255);
            const uint16_t v=uint16_t((c.r>>3)<<11 | (c.g>>2)<<5 | (c.b>>3));
            h=(h^v)*1099511628211ull; ++pixels;
            if(out.is_open()) {const unsigned char rgb[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)};out.write(reinterpret_cast<const char*>(rgb),3);}
        }
        for(unsigned j=0;j<n;++j) assert(hashes[j]!=h);
        hashes[n]=h; if(out.is_open()) assert(out);
        std::printf("frame %u hash %016llx\n",n,static_cast<unsigned long long>(h));
    }
    std::printf("PASS %u fold profiles, %u deterministic bounded RGB565 pixels\n",folds,pixels);
    for(bool candidate:{false,true}) for(unsigned step:{1u,2u,4u}) {
        const auto start=std::chrono::steady_clock::now(); uint64_t sum=0;
        for(int frame=0;frame<120;++frame) for(int y=0;y<240;y+=int(step)) for(int x=0;x<320;x+=int(step)) {
            const auto c=candidate?prismShade(x,y,160,120,180,frame,frame,90):legacy(x,y,180,frame,frame,90); sum+=unsigned(c.r+c.g+c.b);
        }
        const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/120;
        std::printf("%s shade-only step %u host ms/frame %.3f checksum %llu\n",candidate?"candidate":"predecessor",step,ms,static_cast<unsigned long long>(sum));
    }
}
