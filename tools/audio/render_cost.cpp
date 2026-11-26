// CYD-AUDIO-020: native render + DAC cost and exact transition receipts.
#include "mhac_audio.hpp"
#include "mhac_dac.hpp"
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <vector>
using namespace mhac;
using Clock=std::chrono::steady_clock;
static void require(bool ok) { if(!ok) std::exit(1); }
int main() {
    constexpr unsigned count=12000;
    std::cout<<"scenario,blocks,hash,p50_ns,p99_ns,max_ns\n";
    for(unsigned scenario=0;scenario<=performance_patch_count;++scenario) {
        Engine engine; Exchange io; DacQuantizer dac;
        std::array<int16_t,block> pcm{};
        std::vector<long long> elapsed; elapsed.reserve(count);
        uint64_t hash=14695981039346656037ull;
        for(unsigned k=0;k<count;++k) {
            // Mixed scenario traverses every ordered pair, including sub-block fades.
            const unsigned patch=scenario<performance_patch_count?scenario:
                ((k/16)%49/7+(k%16>=8?(k/16)%7+7-(k/16)%49/7:0))%7;
            const unsigned x=(k*137u)&4095u,y=(k*311u)&4095u;
            const bool gate=k%19!=0, mute=k%251==0;
            if(patch==modal_patch_index) io.publish_modal(x,y,gate,mute);
            else io.publish_patch(x,y,gate,mute,static_cast<Patch>(patch));
            const auto start=Clock::now();
            engine.render(io,pcm.data(),block);
            for(auto v:pcm) { hash^=uint16_t(v); hash*=1099511628211ull;
                hash^=dac.pack(v); hash*=1099511628211ull; }
            elapsed.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-start).count());
            for(int v:pcm) require(std::abs(v)<=8191);
        }
        require(io.watchdog_trips.load()==0);
        std::sort(elapsed.begin(),elapsed.end());
        std::cout<<scenario<<','<<count<<','<<std::hex<<hash<<std::dec<<','
                 <<elapsed[count/2]<<','<<elapsed[count*99/100]<<','<<elapsed.back()<<'\n';
    }
}
