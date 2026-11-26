#include "mhac_audio.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac;
int main() {
    static_assert(feature_window==block);
    for(unsigned p=0;p<extended_patch_count;++p) {
        Exchange a,b; Engine e,r;
        AudioFeatures fa{},fb{};
        std::array<int16_t,block> x{},y{};
        unsigned previous_mean=0; int previous=0, sign=0;
        assert(!a.features.read(fa));
        for(unsigned k=0;k<375;++k) {
            const unsigned px=(k*31u)&4095u,py=(k*17u)&4095u;
            const bool gate=k<250;
            a.publish_patch(px,py,gate,false,static_cast<Patch>(p));
            b.publish_patch(px,py,gate,false,static_cast<Patch>(p));
            e.render(a,x.data(),block);
            const unsigned split=1+k%(block-1);
            r.render(b,y.data(),split);
            if(k==0) assert(!b.features.read(fb));
            else assert(b.features.read(fb)&&fb.sample_end==k*block);
            r.render(b,y.data()+split,block-split);
            assert(x==y && a.features.read(fa) && b.features.read(fb));
            unsigned peak=0,sum=0,delta=0,crossings=0;
            for(int s:x) {
                peak=std::max(peak,unsigned(std::abs(s))); sum+=std::abs(s);
                delta+=std::abs(s-previous); previous=s;
                if(s!=0) {const int next=s<0?-1:1; if(sign==-next) ++crossings; sign=next;}
            }
            const unsigned mean=sum/block, onset=mean>previous_mean?mean-previous_mean:0;
            previous_mean=mean;
            for(const auto f:{fa,fb}) {
                assert(f.sample_end==(k+1)*block && f.peak==peak && f.mean_abs==mean);
                assert(f.mean_delta==delta/block && f.crossings==crossings && f.onset==onset);
                if(k>=300) assert(f.peak==0 && f.mean_abs==0 && f.mean_delta==0 && f.onset==0 && f.crossings==0);
            }
        }
    }
    std::puts("PASS six patches; 288000 samples per partition; 2250 independent feature windows; release silence");
}
