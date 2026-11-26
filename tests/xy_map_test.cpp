#include "mhac_xy_map.hpp"
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
int main() {
    XYMapper mapper;
    unsigned hits=0, points=0, relative=0;
    XYMap m=XYMap::Direct;
    for(unsigned i=0;i<3000;++i) { assert(unsigned(m)==i%xy_map_count); m=nextXYMap(m); }
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        hits+=xyMapSelector(x,y);
        const auto p=touchPoint(x,y), d=mapper.target(XYMap::Direct,p), s=mapper.target(XYMap::Swapped,p);
        assert(d.x==unsigned(std::clamp(x,0,319))*4095/319 && d.y==unsigned(239-std::clamp(y,0,239))*4095/239);
        assert(s.x==d.y && s.y==d.x && d.x<=4095 && d.y<=4095); ++points;
    }
    assert(hits==768);
    for(unsigned a:{0u,1u,1024u,2048u,4094u,4095u}) for(unsigned b:{0u,1u,1024u,2048u,4094u,4095u}) {
        mapper.begin({a,4095-a},{b,4095-b});
        const auto start=mapper.target(XYMap::Fine,{a,4095-a});
        assert(start.x==b && start.y==4095-b);
        unsigned previous=0;
        for(unsigned v=0;v<=4095;++v) {
            const auto p=mapper.target(XYMap::Fine,{v,4095-v});
            const auto expected=unsigned(std::clamp(int(b)+(int(v)-int(a))/2,0,4095));
            assert(p.x==expected && p.y==4095-expected && p.x>=previous);
            previous=p.x; ++relative;
        }
        const auto back=mapper.target(XYMap::Fine,{a,4095-a});
        assert(back.x==b && back.y==4095-b);
    }
    std::printf("XY mapping PASS: %u coordinate pairs, %u relative targets, %u selector pixels, 3000 cycles\n",points,relative,hits);
}
