#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    CellularField a,b;
    a.advance(0,true,0,0,false,4095); b.advance(0,true,0,0,false,4095);
    for(unsigned t=50;t<=200;t+=50) a.advance(t,true,0,0,false,4095);
    b.advance(200,true,0,0,false,4095);
    for(int y=0;y<30;++y) for(int x=0;x<40;++x) assert(a.cell(x,y)==b.cell(x,y));
    a.advance(0,false,0,0,false,0); a.advance(0xfffffff0u,true,0,0,false,4095);
    a.advance(34,true,0,0,false,4095); b.advance(0,false,0,0,false,0);
    b.advance(0,true,0,0,false,4095); b.advance(50,true,0,0,false,4095);
    for(int y=0;y<30;++y) for(int x=0;x<40;++x) assert(a.cell(x,y)==b.cell(x,y));
    a.advance(100000,true,4095,4095,true,4095);
    b.advance(250,true,4095,4095,true,4095);
    for(int y=0;y<30;++y) for(int x=0;x<40;++x) assert(a.cell(x,y)==b.cell(x,y));
    PerformanceSurface ui; mhac::Exchange io; mhac::Engine synth; CellularField field;
    uint32_t t=0; unsigned active=0; std::array<int16_t,mhac::block> pcm{};
    auto touch=[&](bool d,int x,int y){ui.sample(d,x,y,t+=8);};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
    tap(310,10); for(unsigned i=0;i<cellular_mode;++i) tap(10,10);
    assert(ui.mode()==cellular_mode);
    std::array<uint16_t,width*strip+2> buf{}; buf.front()=0x1234; buf.back()=0x5678;
    std::array<uint64_t,3> hashes{};
    for(unsigned n=0;n<3;++n) {
        for(unsigned k=0;k<192;++k) {
            touch(true,60+int(k%120),180-int(n)*65);
            const auto c=ui.effective(t); publishControl(io,c); synth.render(io,pcm.data(),pcm.size());
            for(auto v:pcm){assert(v>=-8191&&v<=8191);active+=v!=0;}
            field.advance(t,true,c.x,c.y,c.gate&&!c.mute,io.snapshot()&4095u);
        }
        Frame f{}; f.control=ui.control(); f.mode=ui.mode(); f.analysis=io.snapshot(); f.cellular=&field;
        unsigned lit=0; for(int y=0;y<30;++y)for(int x=0;x<40;++x){assert(field.cell(x,y)<=15);lit+=field.cell(x,y)!=0;} assert(lit>0);
        std::ofstream out; if(argc>1){out.open(std::string(argv[1])+std::to_string(n)+".ppm",std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";}
        uint64_t h=14695981039346656037ull;
        for(unsigned q:{1u,2u,4u}) for(int y=0;y<height;y+=strip) {
            renderStrip(f,y,q,buf.data()+1);assert(buf.front()==0x1234&&buf.back()==0x5678);
            for(int j=0;j<strip;++j)for(int x=0;x<width;++x){
                const auto v=buf[1+j*width+x]; assert(v==pixel(f,x/int(q)*int(q),y+j/int(q)*int(q)));
                if(q==1){h=(h^v)*1099511628211ull;if(out.is_open()){
                    const unsigned char c[]={static_cast<unsigned char>(((v>>11)&31u)*255/31),static_cast<unsigned char>(((v>>5)&63u)*255/63),static_cast<unsigned char>((v&31u)*255/31)};
                    out.write(reinterpret_cast<const char*>(c),3);
                }}
            }
        }
        hashes[n]=h; for(unsigned k=0;k<n;++k)assert(hashes[k]!=h);
        if(out.is_open())assert(out);
        f.control.mute=true;f.watchdog=true; for(unsigned q:{1u,2u,4u}){renderStrip(f,0,q,buf.data()+1);assert(buf[1+4*width+300]==rgb(255,184,32));}
        std::printf("frame %u hash %016llx occupied %u/1200\n",n,static_cast<unsigned long long>(h),lit);
    }
    touch(false,0,0);tap(10,10);assert(ui.mode()==diffusion_mode);tap(10,10);assert(ui.mode()==biolume_mode);tap(10,10);assert(ui.mode()==0);assert(active>0&&!io.watchdog_trips.load());
    std::printf("cellular PASS: 691200 pixel comparisons, 3600 state comparisons, %u active PCM samples, %zu bytes\n",active,sizeof(CellularField));
    const auto start=std::chrono::steady_clock::now();unsigned sum=0;
    for(unsigned i=0;i<10000;++i){field.advance(i*50u,true,i%4096u,2048,true,2048);sum+=field.cell(i%40u,i%30u);}
    std::printf("cellular step %.5f ms checksum %u\n",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/10000,sum);
}
