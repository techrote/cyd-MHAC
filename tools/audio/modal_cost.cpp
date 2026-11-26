#include "mhac_audio.hpp"
#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>
using namespace mhac;
int main() {
    constexpr unsigned blocks=24000;
    std::printf("scenario,blocks,hash,p50_ns,p99_ns,engine_bytes,modal_bytes\n");
    for(unsigned mode=0;mode<3;++mode) {
        Engine e; Exchange io; std::array<int16_t,block> pcm{};
        std::vector<long long> times; times.reserve(blocks);
        uint64_t h=14695981039346656037ull;
        for(unsigned k=0;k<blocks;++k) {
            unsigned x=2048,y=2600; bool gate=true;
            if(mode==1) { x=(k*137u)&4095u; y=(k*311u)&4095u; }
            if(mode==2) { x=((k/12)*683u)&4095u; y=((k/12)*1024u)&4095u; gate=k%12<5; }
            io.publish_modal(x,y,gate,false);
            const auto t=std::chrono::steady_clock::now();
            e.render(io,pcm.data(),block);
            times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-t).count());
            for(int s:pcm) { if(std::abs(s)>8191) return 1; h=(h^uint16_t(s))*1099511628211ull; }
        }
        if(io.watchdog_trips.load()!=0) return 2;
        std::sort(times.begin(),times.end());
        std::printf("%u,%u,%016llx,%lld,%lld,%zu,%zu\n",mode,blocks,(unsigned long long)h,times[blocks/2],times[blocks*99/100],sizeof(Engine),sizeof(ModalChime));
    }
}
