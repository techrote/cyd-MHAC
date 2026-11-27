// Host-only audio render + DAC preparation benchmark. Refs #1, #2.
// This does not model ESP32 scheduling, DMA, SPI or actual display load.
#include "mhac_audio.hpp"
#include "mhac_dac.hpp"
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

namespace {
using Clock=std::chrono::steady_clock;
constexpr unsigned blocks=4096, warmup=64, modes=10;
constexpr double budget_ns=1e9*mhac::block/mhac::rate;
struct Result { uint64_t hash=14695981039346656037ull; unsigned peak=0, trips=0; };
void send(mhac::Exchange& io,unsigned mode,unsigned x,unsigned y,bool gate,bool mute) {
    if(mode==6) io.publish_modal(x,y,gate,mute);
    else if(mode==7) io.publish_quotation(x,y,gate,mute);
    else if(mode==8) io.publish_counterquotation(x,y,gate,mute);
    else if(mode==9) io.publish_formant(x,y,gate,mute);
    else io.publish_patch(x,y,gate,mute,static_cast<mhac::Patch>(mode));
}
Result run(unsigned scenario,bool contention) {
    mhac::Exchange io; mhac::Engine engine; mhac::DacQuantizer dac;
    std::array<int16_t,mhac::block> pcm{};
    std::array<uint16_t,mhac::block*2> slots{};
    std::vector<double> timings(blocks);
    std::atomic<bool> stop{false},ready{false};
    unsigned reads=0;
    std::thread reader;
    if(contention) {
        reader=std::thread([&] {
            mhac::AudioFeatures f{};
            ready.store(true,std::memory_order_release);
            while(!stop.load(std::memory_order_acquire)) {
                if(io.features.read(f)) ++reads;
                (void)io.snapshot();
            }
        });
        while(!ready.load(std::memory_order_acquire)) std::this_thread::yield();
    }
    Result result;
    for(unsigned b=0;b<blocks+warmup;++b) {
        // Sustained modes include repeated notes; rapid switching defeats the
        // single-mode shortcut. Lease scenario deliberately stops publication.
        const unsigned mode=scenario<modes?scenario:scenario==modes?b%modes:5u;
        const unsigned x=(b*193u)&4095u,y=(b*317u)&4095u;
        if(scenario!=modes+1 || b==0 || b==1024 || b==2048 || b==3072) {
            const bool gate=scenario==modes+1?b!=1024&&b!=3072:b%128u<112u;
            send(io,mode,x,y,gate,!gate && b%256u==255u);
        }
        const auto start=Clock::now();
        engine.render(io,pcm.data(),mhac::block);
        for(unsigned i=0;i<mhac::block;++i) slots[i*2]=slots[i*2+1]=dac.pack(pcm[i]);
        const auto end=Clock::now();
        if(b>=warmup) timings[b-warmup]=std::chrono::duration<double,std::nano>(end-start).count();
        for(unsigned i=0;i<mhac::block;++i) {
            const int s=pcm[i]; const unsigned mag=unsigned(s<0?-s:s);
            result.peak=std::max(result.peak,mag);
            if(mag>8191u || slots[i*2]!=slots[i*2+1] || (slots[i*2]&255u)) std::abort();
            result.hash=(result.hash^uint16_t(s))*1099511628211ull;
            result.hash=(result.hash^slots[i*2])*1099511628211ull;
        }
        if(scenario==modes+1 && (b==512 || b==2560)) {
            for(unsigned i=0;i<mhac::block;++i) if(pcm[i]!=0 || slots[i*2]!=0x8000u) std::abort();
        }
    }
    stop.store(true,std::memory_order_release);
    if(reader.joinable()) reader.join();
    result.trips=io.watchdog_trips.load();
    if(result.trips!=(scenario==modes+1?2u:0u) || (contention && reads==0)) std::abort();
    std::sort(timings.begin(),timings.end());
    unsigned over=0; for(double t:timings) if(t>budget_ns) ++over;
    std::printf("%u,%u,%.3f,%.3f,%.3f,%u,%u,%u,%u,%016llx\n",scenario,unsigned(contention),
        timings[blocks/2]/1000.,timings[(blocks*99)/100]/1000.,timings.back()/1000.,over,
        result.peak,result.trips,reads,static_cast<unsigned long long>(result.hash));
    return result;
}
}
int main() {
    std::printf("scenario,reader,median_us,p99_us,max_us,over_budget,peak,trips,reads,pcm_dac_hash\n");
    for(unsigned s=0;s<modes+2;++s) {
        const auto a=run(s,false),b=run(s,true);
        if(a.hash!=b.hash || a.peak!=b.peak || a.trips!=b.trips) return 1;
    }
}
