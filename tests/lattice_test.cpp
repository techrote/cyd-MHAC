#include "mhac_lattice.hpp"
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
int main() {
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
