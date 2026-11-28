#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    DiffusionField a,b;
    auto equal=[&](){for(int y=0;y<24;++y)for(int x=0;x<32;++x)assert(a.cell(x,y)==b.cell(x,y));};
    a.advance(0,true,4095,4095,true,1024);b.advance(0,true,4095,4095,true,1024);
    for(unsigned t=12;t<=48;t+=12)a.advance(t,true,4095,4095,true,1024);
    b.advance(48,true,4095,4095,true,1024);equal();
    a.advance(49,false,0,0,false,0);b.advance(49,false,0,0,false,0);
    a.advance(0xfffffff0u,true,0,0,false,0);b.advance(0,true,0,0,false,0);
    a.advance(32,true,0,0,false,0);b.advance(48,true,0,0,false,0);equal();
    a.advance(100000,true,0,4095,true,65535);b.advance(96,true,0,4095,true,65535);equal();
    a.advance(0,false,0,0,false,0);b.advance(0,false,0,0,false,0);
    a.advance(0,true,0,0,false,0);b.advance(0,true,0,0,false,0);equal();
    for(unsigned i=1;i<=120;++i){a.advance(i*12,true,2048,2048,true,0);b.advance(i*12,true,2048,2048,true,4095);}
    unsigned changed=0;for(int y=0;y<24;++y)for(int x=0;x<32;++x)changed+=a.cell(x,y)!=b.cell(x,y);assert(changed>0);
    std::printf("peak-modulated cells differing %u/768\n",changed);
    PerformanceSurface ui; mhac::Exchange io; mhac::Engine synth; DiffusionField field;
    uint32_t t=0; unsigned active=0;std::array<int16_t,mhac::block> pcm{};
    auto touch=[&](bool d,int x,int y){ui.sample(d,x,y,t+=8);};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
    tap(310,10);for(unsigned i=0;i<diffusion_mode;++i)tap(10,10);assert(ui.mode()==diffusion_mode);
    std::array<uint16_t,width*strip+2> buf{};buf.front()=0x1234;buf.back()=0x5678;
    std::array<uint64_t,3> hashes{};
    for(unsigned n=0;n<3;++n) {
        for(unsigned k=0;k<384;++k) {
            touch(true,55+int((k/2)%210),50+int(n)*65+int(k%37));
            const auto c=ui.effective(t);publishControl(io,c);synth.render(io,pcm.data(),pcm.size());
            for(auto sample:pcm){assert(sample>=-8191&&sample<=8191);active+=sample!=0;}
            field.advance(t,true,c.x,c.y,c.gate&&!c.mute,io.snapshot()&4095u);
        }
        unsigned occupied=0;for(int y=0;y<24;++y)for(int x=0;x<32;++x){auto c=field.cell(x,y);assert((c&8191u)<=4096u&&(c>>13)<=4096u);occupied+=(c>>13)>256;}
        assert(occupied>0&&occupied<768);
        Frame f{};f.control=ui.control();f.mode=ui.mode();f.analysis=io.snapshot();f.diffusion=&field;
        std::ofstream out;if(argc>1){out.open(std::string(argv[1])+std::to_string(n)+".ppm",std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";}
        uint64_t h=14695981039346656037ull;
        for(unsigned q:{1u,2u,4u})for(int y=0;y<height;y+=strip){
            renderStrip(f,y,q,buf.data()+1);assert(buf.front()==0x1234&&buf.back()==0x5678);
            for(int j=0;j<strip;++j)for(int x=0;x<width;++x){
                const auto v=buf[1+j*width+x];assert(v==pixel(f,x/int(q)*int(q),y+j/int(q)*int(q)));
                if(q==1){h=(h^v)*1099511628211ull;if(out.is_open()){
                    const unsigned char c[]={static_cast<unsigned char>(((v>>11)&31u)*255/31),static_cast<unsigned char>(((v>>5)&63u)*255/63),static_cast<unsigned char>((v&31u)*255/31)};
                    out.write(reinterpret_cast<const char*>(c),3);
                }}
            }
        }
        hashes[n]=h;for(unsigned k=0;k<n;++k)assert(hashes[k]!=h);if(out.is_open())assert(out);
        f.control.mute=true;f.watchdog=true;for(unsigned q:{1u,2u,4u}){renderStrip(f,0,q,buf.data()+1);assert(buf[1+4*width+300]==rgb(255,184,32));}
        std::printf("frame %u hash %016llx occupied %u/768\n",n,static_cast<unsigned long long>(h),occupied);
    }
    touch(false,0,0);tap(10,10);assert(ui.mode()==biolume_mode);tap(10,10);assert(ui.mode()==0);assert(active>0&&!io.watchdog_trips.load());
    std::printf("diffusion PASS: 691200 pixel comparisons, 3072 state equivalences, 2304 cell bounds, %u active PCM samples, %zu bytes\n",active,sizeof(DiffusionField));
    Frame f{};f.diffusion=&field;f.analysis=io.snapshot();f.control=ui.control();
    // Same frozen pigment, control and tick sequence for each optical mode.
    std::array<uint32_t,768> frozen{};
    for(int y=0;y<24;++y)for(int x=0;x<32;++x)frozen[y*32+x]=field.cell(x,y);
    for(unsigned mode:{diffusion_mode,biolume_mode}) {
        f.mode=mode;
        for(unsigned q:{1u,2u,4u}) {
            uint64_t sum=0;const auto start=std::chrono::steady_clock::now();
            for(unsigned i=0;i<180;++i) {
                f.tick=i;
                for(int y=0;y<height;y+=strip){renderStrip(f,y,q,buf.data()+1);sum+=buf[1+i%(width*strip)];}
            }
            std::printf("mode %u step %u %.5f ms/frame checksum %llu\n",mode,q,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/180,static_cast<unsigned long long>(sum));
            assert(buf.front()==0x1234&&buf.back()==0x5678);
            for(int y=0;y<24;++y)for(int x=0;x<32;++x)assert(frozen[y*32+x]==field.cell(x,y));
        }
    }
    const auto start=std::chrono::steady_clock::now();unsigned sum=0;
    field.advance(0,false,0,0,false,0);field.advance(0,true,0,0,false,0);
    for(unsigned i=1;i<=10000;++i){field.advance(i*12u,true,i%4096u,2048,true,2048);sum+=field.cell(i%32u,i%24u);}
    std::printf("diffusion step %.5f ms checksum %u\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/10000,sum);
}
