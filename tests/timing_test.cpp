#include "mhac_timing.hpp"
#include <cassert>
#include <cstdio>
#include <thread>
using namespace mhac;
int main() {
    TimingMailbox box; TimingMeter meter(5334); AudioTiming s{};
    s.attempts=999;
    assert(!box.read(s) && s.attempts==999);
    meter.record(0xfffffff0u,0x10u,0x20u,0x120u,true,box);
    assert(box.read(s) && s.attempts==1 && s.failed_writes==0);
    assert(s.max_render_us==32 && s.max_prepare_us==48 && s.max_write_us==256);
    assert(s.max_start_gap_us==0 && s.long_start_gaps==0);
    meter.record(0x120u,0x130u,0x140u,0x150u,true,box);
    assert(box.read(s) && s.max_start_gap_us==304 && s.long_start_gaps==0);
    meter.record(0x120u+5333u,0x120u+5333u+5000u,0x120u+5333u+5333u,
                 0x120u+5333u+5333u,true,box);
    assert(box.read(s) && s.prepare_over_budget==0 && s.long_start_gaps==0);
    const uint32_t start=0x120u+5333u+5334u;
    meter.record(start,start+5000u,start+5334u,start+6000u,false,box);
    assert(box.read(s) && s.attempts==4 && s.failed_writes==1);
    assert(s.prepare_over_budget==1 && s.long_start_gaps==1);
    assert(s.max_render_us==5000 && s.max_prepare_us==5334 && s.max_write_us==666 && s.max_start_gap_us==5334);
    // A normal paced blocking write is not counted as a preparation overrun.
    meter.record(start+6000u,start+6010u,start+6020u,start+36020u,false,box);
    assert(box.read(s) && s.attempts==5 && s.failed_writes==2);
    assert(s.prepare_over_budget==1 && s.long_start_gaps==2 && s.max_write_us==30000);
    TimingMailbox concurrent;
    std::atomic<bool> done{false};
    std::thread writer([&]{for(uint32_t i=1;i<=200000;++i) concurrent.publish({i,i+1,i+2,i+3,i+4,i+5,i+6,i+7}); done.store(true);});
    unsigned reads=0;
    do {
        AudioTiming t{};
        if(concurrent.read(t)) {
            const auto i=t.attempts;
            assert(t.failed_writes==i+1 && t.prepare_over_budget==i+2 && t.long_start_gaps==i+3);
            assert(t.max_render_us==i+4 && t.max_prepare_us==i+5 && t.max_write_us==i+6 && t.max_start_gap_us==i+7);
            ++reads;
        }
    } while(!done.load());
    writer.join();
    assert(concurrent.read(s) && s.attempts==200000 && s.max_start_gap_us==200007);
    std::printf("PASS timing boundaries, clock wrap, write failure, 200000 publications, %u coherent reads; meter=%zu mailbox=%zu bytes\n",reads,sizeof(TimingMeter),sizeof(TimingMailbox));
}
