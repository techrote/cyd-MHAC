#include "mhac_gesture.hpp"
#include <cassert>
#include <cstdio>
using L=mhac::visual::GestureLoop;
int main() {
    unsigned active=0, rests=0, changed=0;
    for(uint32_t start:{0u,0xffffff00u}) for(unsigned style=0;style<L::style_count;++style) {
        L a,b;
        for(unsigned j=0;j<style;++j){a.nextStyle();b.nextStyle();}
        for(auto* l:{&a,&b}) {
            assert(!l->paintPitch(start,100)); l->toggle();
            for(unsigned i=0;i<20;++i) l->observe({uint16_t(100+i*100),uint16_t(300+i*100),true},start+i*16);
            l->observe({},start+320); assert(l->state()==L::State::Playing);
        }
        for(unsigned t=0;t<2500;++t) {
            const auto now=start+320+t; const auto ref=a.play(now);
            const auto before=b.preview(); bool edited=b.paintPitch(now,9999);
            const auto got=b.play(now); const auto after=b.preview();
            assert(edited==ref.gate && got.gate==ref.gate && got.y==ref.y);
            assert(a.count()==b.count() && a.duration()==b.duration());
            for(unsigned i=0;i<after.count;++i) {assert(after.points[i].y==before.points[i].y); if(!edited)assert(after.points[i].x==before.points[i].x);}
            if(edited){++active; changed+=got.x!=ref.x; assert(got.x<=4095);} else ++rests;
        }
        b.stop(); assert(!b.paintPitch(start+3000,0));
    }
    assert(active && rests && changed); std::printf("pitch core: %u active %u rest %u changed\n",active,rests,changed);
}
