#include "mhac_gesture.hpp"
#include <cassert>
#include <cstdio>
using L=mhac::visual::GestureLoop;
static void same(L::Point a,L::Point b) { assert(a.x==b.x && a.y==b.y && a.gate==b.gate); }
int main() {
    unsigned comparisons=0,changed=0,profiles=0;
    for(uint32_t start:{0u,0xffffff00u}) for(unsigned style=0;style<L::style_count;++style)
    for(unsigned n:{1u,2u,3u,20u,250u,251u}) {
        L original,edited,expected;
        for(auto* l:{&original,&edited,&expected}) {
            assert(!l->retrogradePitch()); l->toggle(); assert(!l->retrogradePitch());
            for(unsigned j=0;j<style;++j) l->nextStyle();
            for(unsigned i=0;i<n;++i) {
                const unsigned index=l==&expected?n-1-i:i;
                const unsigned t=i*16u+(n<20?i%3u:0u);
                l->observe({uint16_t(index*17u%4096),uint16_t(i*37u%4096),true},start+t);
            }
            assert(!l->retrogradePitch());
            l->observe({},start+std::max(32u,n*16u));
            assert(l->state()==L::State::Playing && l->count()==n);
        }
        assert(edited.retrogradePitch()==(n>1));
        const uint32_t origin=start+std::max(32u,n*16u);
        for(unsigned t=0;t<18000;t+=7) {
            auto a=original.play(origin+t), b=edited.play(origin+t), c=expected.play(origin+t);
            same(b,c); assert(a.y==b.y && a.gate==b.gate);
            changed+=a.x!=b.x; ++comparisons;
        }
        assert(edited.retrogradePitch()==(n>1));
        for(unsigned t=18000;t<36000;t+=7) { same(original.play(origin+t),edited.play(origin+t)); ++comparisons; }
        assert(original.count()==edited.count() && original.duration()==edited.duration());
        edited.stop(); assert(!edited.retrogradePitch()); ++profiles;
    }
    assert(changed>0); std::printf("profiles=%u exact_comparisons=%u changed_pitch=%u loop_bytes=%zu\n",profiles,comparisons,changed,sizeof(L));
}
