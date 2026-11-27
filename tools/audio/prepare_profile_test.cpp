#include "mhac_prepare_profile.hpp"
#include <cassert>
#include <cstdio>
#include <thread>
using namespace mhac;
int main() {
    for(uint32_t us=0;us<20000;++us) {
        const auto expected=us>=5334?4u:unsigned(uint64_t(us)*4/5334);
        assert(PrepareProfiler::bin(us)==expected);
    }
    assert(PrepareProfiler::bin(0xffffffffu)==4);
    PrepareProfiler meter; PrepareProfileMailbox box; PrepareProfile p{};
    p.attempts=42; assert(!box.read(p) && p.attempts==42);
    meter.record(0xfffffff0u,0x10u,true,box);
    assert(box.read(p) && p.attempts==1 && p.bins[0]==1 && p.max_us==32);
    const uint32_t samples[]={1334,2667,4001,5334,0};
    for(unsigned i=0;i<126;++i) meter.record(0,samples[i%5],true,box);
    assert(box.read(p) && p.attempts==1);
    meter.record(0,1,true,box);
    assert(box.read(p) && p.attempts==128 && p.max_us==5334 && p.failed==0);
    assert(p.bins[0]==27 && p.bins[1]==26 && p.bins[2]==25 && p.bins[3]==25 && p.bins[4]==25);
    meter.record(0,6000,false,box);
    assert(box.read(p) && p.attempts==129 && p.failed==1 && p.max_us==6000 && p.bins[4]==26);
    PrepareProfileMailbox concurrent;
    std::atomic<bool> done{false};
    std::thread writer([&] { for(uint32_t i=1;i<=200000;++i) {
        PrepareProfile q{}; q.attempts=i; q.bins[i%5]=i; q.max_us=i; q.failed=i/3;
        concurrent.publish(q);
    } done.store(true); });
    unsigned reads=0;
    do { PrepareProfile q{}; if(concurrent.read(q)) {
        const auto i=q.attempts; assert(i && q.max_us==i && q.failed==i/3);
        for(unsigned j=0;j<5;++j) assert(q.bins[j]==(j==i%5?i:0));
        ++reads;
    } } while(!done.load());
    writer.join(); assert(concurrent.read(p) && p.attempts==200000);
    std::printf("PASS 20000 bins, wrap, cadence, terminal publication, 200000 concurrent publications, %u coherent reads; profiler=%zu mailbox=%zu bytes\n",reads,sizeof(PrepareProfiler),sizeof(PrepareProfileMailbox));
}
