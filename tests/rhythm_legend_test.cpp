#include "mhac_rhythm_legend.hpp"
#include <cassert>
#include <iostream>
using namespace mhac::visual;
static bool same(RhythmLegendColor a,RhythmLegendColor b) {
    return a.r==b.r && a.g==b.g && a.b==b.b;
}
int main() {
    constexpr unsigned targets[4][5]={{255,255,0,204,166},{255,170,38,204,138},
                                      {255,85,77,204,111},{255,0,115,204,83}};
    constexpr unsigned phase[5]={0,2832,5100,7416,10248};
    const RhythmLegendColor background{12,20,28};
    unsigned checks=0;
    for(unsigned band=0;band<4;++band) {
        for(unsigned i=0;i<5;++i) {
            auto m=rhythmMark(i,band);
            assert(m.gain==targets[band][i]);
            assert(m.x==8+4*int((phase[i]*10+6000)/12000));
            assert(m.width==(i==2?4:8));
            assert(same(rhythmLegend(m.x,band),background)==(m.gain==0));
            if(i==2 && band) { auto c=rhythmLegend(m.x,band); assert(c.r>c.g && c.g>c.b); }
        }
        for(int x=8;x<52;++x) {
            auto c=rhythmLegend(x,band);
            assert(c.r>=0 && c.r<=255 && c.g>=0 && c.g<=255 && c.b>=0 && c.b<=255);
            for(int q:{1,2,4}) { assert(same(c,rhythmLegend(x/q*q,band))); ++checks; }
        }
        assert(same(rhythmLegend(28,band),background) && same(rhythmLegend(40,band),background));
    }
    std::cout<<"rhythm legend PASS: 20 nominal gains and positions, "<<checks<<" tier comparisons\n";
}
