#include "mhac_audio.hpp"
#include <array>
#include <cstdlib>
#include <iostream>
using namespace mhac;
static void check(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
// Oscillator-phase witness: constant carrier is an injected control, not audio.
struct Probe {
    unsigned calls=0; uint32_t base=0, harmonic=0;
    float operator()(uint32_t p) { if(calls++==0) base=p; else harmonic=p; return 0.f; }
};
int main() {
    unsigned checks=0, nonzero=0; int32_t max_offset=0;
    for(bool counter:{false,true}) for(float y:{0.f,.25f,1.f}) {
        Quotation q; uint32_t phase=0; constexpr uint32_t step=20000000u;
        int32_t prev=0;
        for(unsigned i=0;i<4*Quotation::history_samples;++i) {
            Probe p; phase+=step;
            const auto v=q.sample(phase,step,y,1.f,true,false,[&](uint32_t a){return p(a);},counter);
            check(v>=0.f && v<=1.f,"output bound");
            if(i<Quotation::history_samples || y==0.f) { check(p.calls==0,"premature/dry voice"); continue; }
            check(p.calls==2,"oscillator count");
            const auto offset=int32_t(p.harmonic-p.base*2u);
            check(offset>=0 && offset<=int32_t(536870912.f*y),"phase bound");
            const auto mark=i/Quotation::interval, at=i%Quotation::interval;
            if(!counter || mark==4) check(offset==0,"unexpected coupling");
            if(counter && (mark&1u) && at==1799u) {
                check(offset>int32_t(536870912.f*y*.70f),"upper answer absent"); ++nonzero;
            }
            if(counter && !(mark&1u) && at==1799u) check(offset==0,"lower answer coupled");
            check(std::abs(int64_t(offset)-prev)<=2684356,"depth slew exceeded");
            prev=offset; max_offset=std::max(max_offset,offset); ++checks;
        }
        // Inhibit during maximum coupling in a fresh voice, then verify smooth decay.
        if(counter && y>0.f) {
            q.reset(); phase=0;
            for(unsigned i=0;i<5*Quotation::interval+1800;++i)
                q.sample(phase+=step,step,y,1.f,true,false,[](uint32_t){return 0.f;},true);
            prev=0;
            for(unsigned i=0;i<3000;++i) {
                Probe p; q.sample(phase+=step,step,y,1.f,false,true,[&](uint32_t a){return p(a);},true);
                if(p.calls==2) {
                    auto offset=int32_t(p.harmonic-p.base*2u);
                    if(i==0) check(offset>0,"hard coupling cut");
                    else check(offset<=prev,"inhibit coupling grew");
                    prev=offset;
                } else check(i>500,"premature blend cut");
            }
        }
    }
    check(nonzero==12,"upper answer witness count");
    std::cout<<"phase_checks="<<checks<<" upper_witnesses="<<nonzero<<" max_offset="<<max_offset<<" quotation_bytes="<<sizeof(Quotation)<<" engine_bytes="<<sizeof(Engine)<<'\n';
}
