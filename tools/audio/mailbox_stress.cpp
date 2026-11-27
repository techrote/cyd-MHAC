// Native concurrent and paused-writer witness; no target scheduling claim.
#include "mhac_audio.hpp"
#include <array>
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <thread>
using namespace mhac;
static void check(bool b) { if(!b) std::abort(); }
static bool equal(const AudioFeatures& a,const AudioFeatures& b) {
    return a.sample_end==b.sample_end && a.peak==b.peak && a.mean_abs==b.mean_abs &&
           a.mean_delta==b.mean_delta && a.onset==b.onset && a.crossings==b.crossings;
}
static AudioFeatures pattern(uint32_t n) {
    return {n,uint16_t(n*3u),uint16_t(n*5u),uint16_t(n*7u),uint16_t(n*11u),uint8_t(n*13u)};
}
int main() {
    // A writer deliberately stops between the odd heartbeat and final commit.
    Exchange io; Engine engine; std::array<int16_t,block> pcm{};
    io.publish(2000,2000,true,false); engine.render(io,pcm.data(),block);
    io.heartbeat.fetch_add(1); io.control.store((1u<<24)|4095u);
    for(unsigned i=0;i<200;++i) engine.render(io,pcm.data(),block);
    check(io.watchdog_trips.load()==1);
    check(std::all_of(pcm.begin(),pcm.end(),[](int16_t x){return x==0;}));
    io.heartbeat.fetch_add(1); // completed active command must not rearm expired gate
    for(unsigned i=0;i<8;++i) engine.render(io,pcm.data(),block);
    check(std::all_of(pcm.begin(),pcm.end(),[](int16_t x){return x==0;}));
    io.publish(2000,2000,false,true); engine.render(io,pcm.data(),block);
    io.publish(2000,2000,true,false); engine.render(io,pcm.data(),block);
    check(std::any_of(pcm.begin(),pcm.end(),[](int16_t x){return x!=0;}));
    // Force publication-sequence wrap without billions of transactions.
    Exchange wrap; Engine e2; wrap.heartbeat.store(0xfffffffcu);
    wrap.publish(0,0,false,true); e2.render(wrap,pcm.data(),block);
    wrap.publish(2000,1000,true,false); check(wrap.heartbeat.load()==0);
    e2.render(wrap,pcm.data(),block);
    check(std::any_of(pcm.begin(),pcm.end(),[](int16_t x){return x!=0;}));
    // FeatureMailbox: two independent readers, one writer, correlated fields.
    FeatureMailbox mailbox; auto initial=pattern(0xdeadbeefu), f=initial;
    check(!mailbox.read(f) && equal(f,initial));
    constexpr uint32_t count=200000;
    std::atomic<unsigned> ready{0}; std::atomic<bool> done{false};
    std::array<unsigned,2> accepted{}, rejected{};
    auto reader=[&](unsigned id) {
        AudioFeatures last=initial; ++ready;
        do {
            auto v=last;
            if(mailbox.read(v)) {
                check(equal(v,pattern(v.sample_end))); ++accepted[id]; last=v;
            } else { check(equal(v,last)); ++rejected[id]; }
        } while(!done.load());
        check(mailbox.read(last) && equal(last,pattern(count)));
    };
    std::thread a(reader,0), b(reader,1);
    while(ready.load()!=2) std::this_thread::yield();
    for(uint32_t n=1;n<=count;++n) mailbox.publish(pattern(n));
    done=true; a.join(); b.join();
    std::cout<<"PASS paused writer, watchdog, explicit rearm, sequence wrap; publications="<<count
             <<" coherent reads="<<accepted[0]+accepted[1]<<" rejected reads="<<rejected[0]+rejected[1]<<'\n';
}
