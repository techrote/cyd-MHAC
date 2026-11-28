#include "mhac_scale.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace mhac::visual;
int main() {
    unsigned changes[3]={}; double worst=0;
    for(unsigned s=0;s<scale_count;++s) {
        auto scale=static_cast<Scale>(s); unsigned prev=0;
        for(unsigned x=0;x<=4095;++x) {
            auto q=mapPitch(x,scale); assert(q>=prev&&q<=4095);
            if(x&&q!=prev) ++changes[s];
            assert(mapPitch(q,scale)==q);
            if(s==0) assert(q==x);
            else {
                unsigned n=(q*36+2047)/4095;
                assert(q==pitchCoordinate(n));
                if(s==2) assert(pentatonic(n));
                const unsigned d=unsigned(std::abs(int(x*36)-int(n*4095)));
                for(unsigned k=0;k<=36;++k) if(s==1||pentatonic(k))
                    assert(d<=unsigned(std::abs(int(x*36)-int(k*4095))));
                worst=std::max(worst,std::abs(3600.0*q/4095-100.0*n));
            }
            prev=q;
        }
        assert(mapPitch(99999,scale)==4095);
    }
    assert(changes[0]==4095&&changes[1]==36&&changes[2]==15);
    assert(worst<0.45);
    assert(mapPitch(1234,static_cast<Scale>(99))==1234);
    std::cout<<"PASS 12288 inputs; coordinate counts "<<changes[0]+1<<' '<<changes[1]+1<<' '<<changes[2]+1<<"; max rounding cents "<<worst<<'\n';
}
