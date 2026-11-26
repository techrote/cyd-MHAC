#include "mhac_trace.hpp"
#include <cassert>
#include <cstdio>
using mhac::visual::Trace;
// Synthetic observations isolate history semantics from the actual audio engine.
struct Observation { uint32_t sample_end; unsigned mean_abs,mean_delta,onset; };
int main() {
    Trace t;
    assert(!t.fresh(0) && !t.at(79).seen);
    assert(t.observe(Observation{0xffffff80u,8192,4096,4096},0xfffffff0u));
    assert(t.at(79).amplitude==255 && t.at(79).activity==127 && t.at(79).onset==255);
    assert(!t.observe(Observation{0xffffff80u,0,0,0},100));
    assert(t.fresh(100) && !t.fresh(135));
    assert(t.observe(Observation{0x380u,0,0,0},140)); // wrap +1024
    assert(t.at(78).amplitude==255 && t.at(79).seen && !t.at(79).amplitude);
    assert(t.observe(Observation{0x400u,4096,8192,0},145));
    assert(t.at(79).amplitude==127 && t.at(79).activity==255);
    assert(!t.observe(Observation{0x380u,8192,8192,8192},146));
    assert(t.observe(Observation{0x1000u,1,1,1},150));
    assert(t.at(76).amplitude==127 && !t.at(77).seen && !t.at(78).seen && t.at(79).seen);
    assert(t.observe(Observation{0x1000u+Trace::columns*Trace::bin_samples,65535,65535,65535},155));
    for(unsigned i=0;i<79;++i) assert(!t.at(i).seen);
    assert(t.at(79).amplitude==255 && t.at(79).activity==255 && t.at(79).onset==255);
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) { auto c=t.shade(x,y); assert(c.r>=0&&c.r<=255&&c.g>=0&&c.g<=255&&c.b>=0&&c.b<=255); }
    std::printf("PASS trace gaps, duplicate/stale observations, sample and wall-clock rollover, saturation, bounds; sizeof=%zu\n",sizeof(Trace));
}
