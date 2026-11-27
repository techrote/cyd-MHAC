// AUDIO-030: compare exact PCM, DAC and mailbox fields between two source pins.
#include "mhac_audio.hpp"
#include "mhac_dac.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
using namespace mhac;
static void require(bool ok) { if(!ok) std::abort(); }
static void put(uint32_t v) {
    const unsigned char bytes[]={static_cast<unsigned char>(v),static_cast<unsigned char>(v>>8),
        static_cast<unsigned char>(v>>16),static_cast<unsigned char>(v>>24)};
    require(std::fwrite(bytes,1,sizeof(bytes),stdout)==sizeof(bytes));
}
static void publish(Exchange& io,unsigned mode,unsigned x,unsigned y,bool gate,bool mute) {
    if(mode==8) io.publish_counterquotation(x,y,gate,mute);
    else if(mode==7) io.publish_quotation(x,y,gate,mute);
    else if(mode==6) io.publish_modal(x,y,gate,mute);
    else io.publish_patch(x,y,gate,mute,static_cast<Patch>(mode));
}
int main() {
    Engine engine; Exchange io; DacQuantizer dac; std::array<int16_t,block> pcm{};
    unsigned total=0, windows=0;
    auto render=[&](unsigned n) {
        engine.render(io,pcm.data(),n);
        for(unsigned k=0;k<n;++k) {
            require(std::abs(int(pcm[k]))<=8191);
            put(uint16_t(pcm[k]) | (uint32_t(dac.pack(pcm[k]))<<16));
        }
        total+=n; AudioFeatures f{};
        if(io.features.read(f)) {
            require(f.sample_end==total/feature_window*feature_window);
            put(f.sample_end); put(f.peak | (uint32_t(f.mean_abs)<<16));
            put(f.mean_delta | (uint32_t(f.onset)<<16)); put(f.crossings); ++windows;
        }
        put(io.snapshot()); put(io.watchdog_trips.load());
    };
    // Every code in both directions, including the initial zero and both endpoints.
    for(unsigned pass=0;pass<2;++pass) for(unsigned j=0;j<4096;++j) {
        const unsigned x=pass?4095-j:j;
        publish(io,(j/467)%9,x,(j*127)&4095,true,false); render(block);
        publish(io,(j/467)%9,x,(j*311)&4095,true,false); render(31);
        render(97); // no renewal, same pitch code
    }
    for(unsigned mode=0;mode<9;++mode) {
        // Stationary gate, Y-only motion, release and rapid mode switches.
        for(unsigned j=0;j<192;++j) {
            publish(io,mode,2048,(j*79)&4095,j%19!=0,false); render(block);
        }
        for(unsigned j=0;j<64;++j) {
            publish(io,(mode+j)%9,2048,3072,true,false); render(17);
        }
        // Interrupted publisher changes raw pitch but must not alter accepted control.
        io.heartbeat.fetch_add(1); io.control.store((io.control.load()&~4095u)|4095u);
        for(unsigned j=0;j<120;++j) render(block);
        io.heartbeat.fetch_add(1); render(block);
        publish(io,mode,0,0,false,true);
        for(unsigned j=0;j<64;++j) render(block);
        for(auto s:pcm) require(s==0);
    }
    require(io.watchdog_trips.load()==9);
    std::fprintf(stderr,"samples=%u feature_reads=%u watchdog=%u Engine=%zu bytes\n",total,windows,io.watchdog_trips.load(),sizeof(Engine));
}
