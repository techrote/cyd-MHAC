#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    PerformanceSurface ui; mhac::Exchange io; mhac::Engine synth;
    uint32_t t=0;
    auto touch=[&](bool d,int x,int y){ui.sample(d,x,y,t+=8);};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
    for(unsigned p=1;p<sound_count;++p) {
        touch(true,10,230); touch(true,180,110);
        assert(ui.control().patch==p && !ui.control().gate);
        touch(false,0,0);
    }
    assert(ui.mode()==quotation_mode && ui.control().mute);
    std::array<int16_t,mhac::block> pcm{};
    auto block=[&]{publishControl(io,ui.effective(t)); synth.render(io,pcm.data(),pcm.size());};
    block(); assert((io.control.load()&mhac::quotation_opt_in)!=0 && !(io.control.load()&mhac::modal_opt_in));
    tap(310,10);
    unsigned active=0; std::array<uint64_t,3> hashes{};
    std::array<uint16_t,width*strip+2> buf{};buf.front()=0x1234;buf.back()=0x5678;
    for(unsigned n=0;n<3;++n) {
        for(unsigned k=0;k<192;++k) {
            touch(true,60+int(k%120),180-int(n)*65);block();
            for(auto v:pcm){assert(v>=-8191 && v<=8191);active+=v!=0;}
        }
        assert(ui.control().gate && (io.snapshot()&4095u)>0 && !io.watchdog_trips.load());
        Frame f{};f.control=ui.control();f.mode=ui.mode();f.analysis=io.snapshot();f.tick=t/25;
        std::ofstream out;
        if(argc>1){out.open(std::string(argv[1])+std::to_string(n)+".ppm",std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";}
        uint64_t h=14695981039346656037ull;
        for(unsigned q:{1u,2u,4u}) for(int y=0;y<height;y+=strip) {
            renderStrip(f,y,q,buf.data()+1);assert(buf.front()==0x1234&&buf.back()==0x5678);
            for(int j=0;j<strip;++j)for(int x=0;x<width;++x){
                const auto v=buf[1+j*width+x];assert(v==pixel(f,x/int(q)*int(q),y+j/int(q)*int(q)));
                if(q==1){h=(h^v)*1099511628211ull;if(out.is_open()){
                    const unsigned char c[]={static_cast<unsigned char>(((v>>11)&31u)*255/31),static_cast<unsigned char>(((v>>5)&63u)*255/63),static_cast<unsigned char>((v&31u)*255/31)};
                    out.write(reinterpret_cast<const char*>(c),3);
                }}
            }
        }
        hashes[n]=h;for(unsigned k=0;k<n;++k)assert(hashes[k]!=h);
        if(out.is_open())assert(out);
        std::printf("frame %u hash %016llx\n",n,static_cast<unsigned long long>(h));
    }
    touch(false,0,0); tap(310,10);for(unsigned k=0;k<128;++k)block();for(auto v:pcm)assert(v==0);
    tap(10,230);assert(ui.control().patch==0&&ui.mode()==0&&ui.control().mute);block();
    assert(!(io.control.load()&(mhac::quotation_opt_in|mhac::modal_opt_in)));
    tap(100,230);assert(ui.factoryIndex()==0 && ui.control().patch==factoryEntry(0).sound.patch);
    assert(factory_count==16&&active>0);
    std::printf("quotation UI PASS: 3 frames, 691200 pixel equivalences, %u active PCM samples, mute, selection and legacy factory\n",active);
}
