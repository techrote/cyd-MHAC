#include "mhac_features.hpp"
#include <cassert>
#include <cstdio>
#include <thread>
#include <chrono>
#include <array>
using namespace mhac;
static bool equal(const AudioFeatures& a,const AudioFeatures& b) {
    return a.sample_end==b.sample_end && a.peak==b.peak && a.mean_abs==b.mean_abs && a.mean_delta==b.mean_delta && a.onset==b.onset && a.crossings==b.crossings;
}
int main() {
    FeatureMailbox m; FeatureAccumulator a; AudioFeatures f{42,1,2,3,4,5}, old=f;
    assert(!m.read(f)&&equal(f,old));
    for(unsigned i=0;i<127;++i) a.push(0,m);
    assert(!m.read(f)); a.push(0,m); assert(m.read(f)&&equal(f,{128,0,0,0,0,0}));
    for(unsigned i=0;i<128;++i) a.push(4096,m);
    assert(m.read(f)&&equal(f,{256,4096,4096,32,4096,0}));
    for(unsigned i=0;i<128;++i) a.push(i%2?int16_t(-32768):int16_t(32767),m);
    assert(m.read(f)&&equal(f,{384,32768,32767,65247,28671,127}));
    for(unsigned i=0;i<128;++i) a.push(0,m);
    assert(m.read(f)&&equal(f,{512,0,0,256,0,0}));
    for(unsigned i=0;i<128;++i) a.push(0,m);
    assert(m.read(f)&&equal(f,{640,0,0,0,0,0}));
    FeatureAccumulator wrap(0xffffff80u); FeatureMailbox wm;
    for(unsigned i=0;i<128;++i) wrap.push(7,wm);
    assert(wm.read(f)&&equal(f,{0,7,7,0,7,0}));
    FeatureMailbox c; std::atomic<bool> done{false}; std::atomic<unsigned> seen{0};
    std::thread reader([&]{ AudioFeatures v; while(!done.load()) if(c.read(v)) {assert(v.peak==uint16_t(v.sample_end)); assert(v.mean_abs==uint16_t(v.sample_end+1)); assert(v.mean_delta==uint16_t(v.sample_end+2)); assert(v.onset==uint16_t(v.sample_end+3)); assert(v.crossings==uint8_t(v.sample_end)); ++seen;} });
    for(uint32_t i=1;i<=200000;++i) c.publish({i,uint16_t(i),uint16_t(i+1),uint16_t(i+2),uint16_t(i+3),uint8_t(i)});
    done.store(true); reader.join(); assert(c.read(f)&&f.sample_end==200000);
    // Fixed deterministic signal workload, no allocation within push or publish.
    FeatureAccumulator perf; FeatureMailbox pm; uint32_t seed=1;
    const auto start=std::chrono::steady_clock::now();
    for(unsigned i=0;i<2400000;++i) {seed=seed*1664525u+1013904223u; perf.push(int16_t(int(seed>>16)-32768),pm);}
    const auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
    assert(pm.read(f)&&f.sample_end==2400000);
    std::printf("features PASS; 200000 publications; %u coherent concurrent reads; %zu accumulator + %zu mailbox bytes; 2.4M samples %lld us\n",seen.load(),sizeof(FeatureAccumulator),sizeof(FeatureMailbox),static_cast<long long>(us));
}
