#include "mhac_gesture.hpp"
#include <cassert>
#include <cstdio>
using L=mhac::visual::GestureLoop;
int main() {
    unsigned profiles=0, checks=0, changes=0, rests=0, pixels=0;
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x)
        pixels+=mhac::visual::timbreMirrorSelector(x,y);
    assert(pixels==576);
    for(uint32_t start:{0u,0xffffff00u}) for(unsigned style=0;style<L::style_count;++style)
    for(unsigned n:{1u,2u,3u,20u,250u,251u}) for(bool rhythm:{false,true}) {
        L original,mirror,expected;
        for(auto* l:{&original,&mirror,&expected}) {
            assert(!l->mirrorTimbre()); l->toggle(); assert(!l->mirrorTimbre());
            for(unsigned j=0;j<style;++j) l->nextStyle();
            for(unsigned i=0;i<n;++i) {
                const unsigned y=rhythm?(i%4)*1365u:(i*37u+123u)%4096u;
                l->observe({uint16_t(i*17u%4096),uint16_t(l==&expected?4095-y:y),true},start+i*16u);
            }
            assert(!l->mirrorTimbre());
            l->observe({},start+std::max(32u,n*16u));
            assert(l->state()==L::State::Playing && l->count()==n);
        }
        assert(mirror.mirrorTimbre());
        auto a=original.preview(),b=mirror.preview(); assert(a.count==b.count);
        for(unsigned i=0;i<a.count;++i) assert(a.points[i].x==b.points[i].x && a.points[i].y+b.points[i].y==4095);
        const uint32_t origin=start+std::max(32u,n*16u);
        for(unsigned t=0;t<18000;t+=7) {
            auto p=original.play(origin+t,true,!rhythm),q=mirror.play(origin+t,true,!rhythm),r=expected.play(origin+t,true,!rhythm);
            assert(q.x==r.x && q.y==r.y && q.gate==r.gate && p.x==q.x && p.gate==q.gate);
            assert(unsigned(p.y)+q.y>=4095 && unsigned(p.y)+q.y<=4096);
            if(rhythm) assert(q.y%1365u==0);
            changes+=p.y!=q.y; rests+=!q.gate; ++checks;
        }
        assert(mirror.mirrorTimbre());
        for(unsigned t=18000;t<36000;t+=7) {
            auto p=original.play(origin+t,true,!rhythm),q=mirror.play(origin+t,true,!rhythm);
            assert(p.x==q.x && p.y==q.y && p.gate==q.gate); ++checks;
        }
        assert(original.count()==mirror.count() && original.duration()==mirror.duration());
        mirror.stop(); assert(!mirror.mirrorTimbre()); ++profiles;
    }
    assert(changes && rests);
    std::printf("mirror PASS: profiles=%u comparisons=%u changed=%u rests=%u selector=%u loop_bytes=%zu\n",profiles,checks,changes,rests,pixels,sizeof(L));
}
