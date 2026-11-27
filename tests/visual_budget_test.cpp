#include "mhac_visual_budget.hpp"
#include <cassert>
#include <cstdio>
using mhac::visual::VisualBudget;
int main() {
    VisualBudget b;
    assert(b.step()==2);
    b.finish(85000,5000,true,1,0,0); assert(b.step()==2);
    b.finish(85001,5000,true,2,0,0); assert(b.step()==4);
    for(unsigned i=0;i<59;++i) { b.finish(54999,2999,true,3+i,0,0); assert(b.step()==4); }
    b.finish(54999,2999,true,62,0,0); assert(b.step()==2);
    b.finish(10000,1000,true,63,1,0); assert(b.step()==4);
    for(unsigned i=0;i<100;++i) { b.finish(10000,1000,true,63,1,0); assert(b.step()==4); }
    for(unsigned i=64;i<123;++i) b.finish(10000,1000,true,i,1,0);
    b.finish(10000,1000,false,0,0,0); assert(b.step()==4);
    for(unsigned i=123;i<182;++i) b.finish(10000,1000,true,i,1,0);
    b.finish(55000,1000,true,182,1,0); assert(b.step()==4);
    for(unsigned i=183;i<242;++i) b.finish(10000,1000,true,i,1,0);
    b.finish(10000,3000,true,242,1,0); assert(b.step()==4);
    for(unsigned i=243;i<303;++i) b.finish(10000,1000,true,i,1,0);
    assert(b.step()==2);
    b.finish(10000,5001,true,303,1,0); assert(b.step()==4);
    for(unsigned i=304;i<364;++i) b.finish(10000,1000,true,i,1,0);
    assert(b.step()==2);
    b.finish(10000,1000,true,364,1,1); assert(b.step()==4);
    for(unsigned i=365;i<424;++i) b.finish(10000,1000,true,i,1,1);
    b.finish(10000,1000,true,424,2,1); assert(b.step()==4);
    for(unsigned i=425;i<485;++i) b.finish(10000,1000,true,i,2,1);
    assert(b.step()==2);
    VisualBudget wrap;
    wrap.finish(10000,1000,true,0xfffffff0u,0xffffffffu,0xffffffffu); assert(wrap.step()==4);
    for(unsigned i=1;i<=60;++i) wrap.finish(10000,1000,true,0xfffffff0u+i,0xffffffffu,0xffffffffu);
    assert(wrap.step()==2);
    wrap.finish(10000,1000,true,45,0,0); assert(wrap.step()==4);
    for(unsigned i=46;i<106;++i) wrap.finish(10000,1000,true,i,0,0);
    assert(wrap.step()==2);
    std::printf("visual budget PASS: display thresholds, audio pressure, stale/missing snapshots, 60-frame recovery, rollover; %zu bytes\n",sizeof(VisualBudget));
}
