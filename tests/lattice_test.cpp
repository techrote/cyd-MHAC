#include "mhac_visual.hpp"
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
static void hysteresis() {
    unsigned jitter=0, crossings=0;
    for(auto s:{Scale::Free,Scale::Chromatic,Scale::MinorPentatonic}) {
        const unsigned cols=latticeColumns(s);
        for(unsigned col=1;col<cols;++col) {
            const int edge=int((col*320u+cols-1u)/cols);
            auto low=latticeCell(edge-1,120,s), high=latticeCell(edge,120,s);
            for(int i=0;i<100;++i) {
                low=latticeTrackedCell(edge+i%6-3,120,s,low,false);
                high=latticeTrackedCell(edge+i%6-3,120,s,high,false);
                assert(low.col==col-1 && high.col==col); jitter+=2;
            }
            assert(latticeTrackedCell(edge+3,120,s,low,false).col==col);
            assert(latticeTrackedCell(edge-4,120,s,high,false).col==col-1);
            assert(latticeTrackedCell(edge,120,s,low,true).col==col); crossings+=3;
        }
        for(int edge:{80,160}) {
            auto low=latticeCell(100,240-edge,s), high=latticeCell(100,239-edge,s);
            for(int i=0;i<100;++i) {
                const int y=239-(edge+i%6-3);
                low=latticeTrackedCell(100,y,s,low,false);
                high=latticeTrackedCell(100,y,s,high,false);
                assert(low.octave==unsigned(edge/80-1) && high.octave==unsigned(edge/80));
                if(i%6>=3) assert(low.timbre==4095); else assert(high.timbre==0);
                jitter+=2;
            }
            assert(latticeTrackedCell(100,239-edge-3,s,low,false).octave==high.octave);
            assert(latticeTrackedCell(100,239-edge+4,s,high,false).octave==low.octave); crossings+=2;
        }
        auto first=latticeCell(0,239,s);
        auto last=latticeTrackedCell(319,0,s,first,false);
        assert(last.note==latticeCell(319,0,s).note && last.timbre==4095); ++crossings;
    }
    // Exercise the actual Surface contact lifecycle, not just the mapper.
    Surface ui; uint32_t t=0;
    auto touch=[&](bool d,int x,int y){ui.sample(d,x,y,t+=8);};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
    tap(310,10); for(int i=0;i<3;++i) tap(270,230);
    touch(true,79,120); touch(true,79,120);
    const auto note=ui.control().x; assert(ui.control().gate);
    for(int i=0;i<100;++i) {touch(true,78+i%5,120); assert(ui.control().x==note && ui.control().gate); ++jitter;}
    touch(true,83,120); assert(ui.control().x==pitchCoordinate(latticeCell(83,120,Scale::Free).note));
    touch(false,0,0); touch(true,79,120); touch(true,79,120); assert(ui.control().x==note && ui.control().gate);
    ui.expire(t+101); assert(ui.control().mute && !ui.control().gate);
    std::printf("hysteresis PASS: %u jitter samples, %u deliberate crossings, Surface reanchor and stale mute\n",jitter,crossings);
}
int main() {
    hysteresis();
    unsigned points=0, notes=0, marks=0;
    for(auto s:{Scale::Free,Scale::Chromatic,Scale::MinorPentatonic}) {
        bool seen[36]={};
        for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
            auto c=latticeCell(x,y,s);
            assert(c.col<latticeColumns(s) && c.octave<3 && c.note<36 && c.timbre<=4095);
            assert(c.note/12==c.octave);
            if(s==Scale::MinorPentatonic) assert(pentatonic(c.note));
            auto p=latticeCursor(pitchCoordinate(c.note),c.timbre,s);
            auto back=latticeCell(p.x,p.y,s);
            assert(back.note==c.note && back.timbre==c.timbre);
            seen[c.note]=true; ++points;
        }
        for(unsigned n=0;n<36;++n) {assert(seen[n]==(s!=Scale::MinorPentatonic||pentatonic(n))); notes+=seen[n];}
        for(unsigned q:{1u,2u,4u}) {
            for(int y=0;y<240;y+=int(q)) {
                unsigned vertical=0;
                for(int x=0;x<320;x+=int(q)) {
                    auto m=latticeMark(x,y,s); assert(m<=2);
                    if(y%80<4) assert(m==2); else if(m==1) ++vertical;
                    ++marks;
                }
                if(y%80>=4) assert(vertical==latticeColumns(s)*4/q);
            }
        }
        for(unsigned p=0;p<=4095;++p) {
            auto c=latticeCursor(p,4095,s);
            assert(c.x>=0 && c.x<320 && c.y>=0 && c.y<240);
        }
        assert(latticeCell(0,239,s).note==0 && latticeCell(0,239,s).timbre==0);
        assert(latticeCell(0,160,s).note==0 && latticeCell(0,160,s).timbre==4095);
        assert(latticeCell(0,159,s).note==12 && latticeCell(0,159,s).timbre==0);
    }
    std::printf("lattice PASS: %u coordinate cases, %u reachable notes across scales, %u sampled grid marks\n",points,notes,marks);
}
