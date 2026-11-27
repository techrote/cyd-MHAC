#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    PerformanceSurface ui; mhac::Engine synth; mhac::Exchange io;
    std::array<int16_t,mhac::block> pcm{};
    std::array<uint16_t,width*strip+2> buf{}; buf.front()=0x1234; buf.back()=0x5678;
    uint32_t t=0;
    auto touch=[&](bool d,int x,int y){ui.sample(d,x,y,t+=8);};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
    tap(310,10);
    for(unsigned m=0;m<caustic_mode;++m) tap(10,10);
    assert(ui.mode()==caustic_mode);
    unsigned active=0; std::array<uint64_t,3> hashes{};
    for(unsigned n=0;n<3;++n) {
        for(unsigned k=0;k<96;++k) {
            touch(true,80+int(n)*75,180-int(n)*55);
            publishControl(io,ui.effective(t)); synth.render(io,pcm.data(),pcm.size());
            for(auto v:pcm) {assert(v>=-8191 && v<=8191); active+=v!=0;}
        }
        assert(ui.control().gate && (io.snapshot()&4095u)>0 && !io.watchdog_trips.load());
        Frame f{}; f.control=ui.control(); f.mode=ui.mode(); f.analysis=io.snapshot(); f.tick=t/25;
        uint64_t h=14695981039346656037ull;
        std::ofstream out;
        if(argc>1) {out.open(std::string(argv[1])+std::to_string(n)+".ppm",std::ios::binary); assert(out); out<<"P6\n320 240\n255\n";}
        for(unsigned q:{1u,2u,4u}) for(int y=0;y<height;y+=strip) {
            renderStrip(f,y,q,buf.data()+1);
            assert(buf.front()==0x1234 && buf.back()==0x5678);
            for(int j=0;j<strip;++j) for(int x=0;x<width;++x) {
                const auto v=buf[1+j*width+x];
                assert(v==pixel(f,x/int(q)*int(q),y+j/int(q)*int(q)));
                if(q==1) {h=(h^v)*1099511628211ull; if(out.is_open()) {
                    const unsigned char c[]={static_cast<unsigned char>(((v>>11)&31u)*255/31),static_cast<unsigned char>(((v>>5)&63u)*255/63),static_cast<unsigned char>((v&31u)*255/31)};
                    out.write(reinterpret_cast<const char*>(c),3);
                }}
            }
        }
        hashes[n]=h; for(unsigned j=0;j<n;++j) assert(hashes[j]!=h);
        if(out.is_open()) assert(out);
        f.control.mute=true; f.watchdog=true;
        for(unsigned q:{1u,2u,4u}) {renderStrip(f,0,q,buf.data()+1); assert(buf[1+4*width+300]==rgb(255,184,32));}
        touch(false,0,0);
        std::printf("frame %u hash %016llx\n",n,static_cast<unsigned long long>(h));
    }
    assert(active>0); tap(10,10); assert(ui.mode()==quotation_mode); tap(10,10); assert(ui.mode()==cellular_mode); tap(10,10); assert(ui.mode()==diffusion_mode); tap(10,10); assert(ui.mode()==0);
    std::printf("caustic PASS: 3 frames, 691200 pixel equivalence checks, %u active PCM samples, mode cycle and safety overlays\n",active);
    Frame f{}; f.mode=caustic_mode; f.analysis=io.snapshot(); f.control=ui.control();
    for(unsigned q:{1u,2u,4u}) {
        uint64_t sum=0; const auto start=std::chrono::steady_clock::now();
        for(unsigned i=0;i<180;++i) {f.tick=i; for(int y=0;y<height;y+=strip) {renderStrip(f,y,q,buf.data()+1); sum+=buf[1+(i%(width*strip))];}}
        std::printf("step %u %.5f ms/frame checksum %llu\n",q,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/180,static_cast<unsigned long long>(sum));
    }
}
