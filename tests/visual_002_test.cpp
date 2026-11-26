#include "mhac_visual.hpp"
#include "mhac_audio.hpp"
#include <array>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    Surface ui; mhac::Exchange io; mhac::Engine synth; std::array<int16_t,mhac::block> pcm{};
    auto publish=[&]{auto c=ui.control();io.publish(c.x,c.y,c.gate,c.mute);};
    auto render=[&]{synth.render(io,pcm.data(),pcm.size());};
    uint32_t tick=1;
    auto touch=[&](bool d,int x,int y){ui.sample(d,x,y,tick);tick+=8;};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,x,y);};
    for(unsigned i=0;i<mode_count;++i){assert(ui.mode()==i);tap(10,10);}assert(ui.mode()==0);
    tap(310,10);assert(!ui.control().mute);touch(true,215,145);touch(true,215,145);assert(ui.control().gate);
    for(unsigned i=0;i<160;++i){publish();render();} // >500 ms, renewed even stationary.
    assert(io.watchdog_trips.load()==0);const auto analysis=io.snapshot();assert((analysis&4095u)>0);
    for(unsigned i=0;i<120;++i){render();}assert(io.watchdog_trips.load()==1);
    ui.observeWatchdog(io.watchdog_trips.load());assert(ui.watchdogLatched()&&ui.control().mute&&!ui.control().gate);
    touch(true,310,10);assert(ui.control().mute);publish();render();
    touch(false,0,0);publish();render();tap(310,10);assert(!ui.watchdogLatched()&&!ui.control().mute&&!ui.control().gate);
    touch(true,215,145);touch(true,215,145);publish();for(unsigned i=0;i<32;++i)render();assert((io.snapshot()&4095u)>0);
    ui.observeWatchdog(1);assert(ui.control().gate);ui.observeWatchdog(0xffffffffu);assert(ui.control().mute);
    ui.observeWatchdog(0);assert(ui.watchdogLatched());
    const Control c{2760,1620,true,false};std::array<uint16_t,width*strip+2> buf{};buf.front()=0x1234;buf.back()=0x5678;
    InkField ink;
    ink.advance(0,1000,1000,true,analysis&4095u);ink.advance(25,2760,1620,true,analysis&4095u);
    std::array<uint64_t,mode_count> hashes{};
    for(unsigned m=0;m<mode_count;++m){
        const Frame f{c,m,analysis,17,false,&ink};uint64_t h=14695981039346656037ull;
        std::ofstream out;if(argc==2){out.open(std::string(argv[1])+"/mode"+std::to_string(m)+".ppm",std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";}
        for(int y=0;y<height;y+=strip){renderStrip(f,y,1,buf.data()+1);for(int i=1;i<=width*strip;++i){auto v=buf[i];h=(h^v)*1099511628211ull;if(out){const unsigned char rgb8[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)};out.write(reinterpret_cast<const char*>(rgb8),3);}}}
        hashes[m]=h;for(unsigned k=0;k<m;++k)assert(hashes[k]!=h);
        for(unsigned q:{1u,2u,4u}){for(int y=0;y<height;y+=strip){renderStrip(f,y,q,buf.data()+1);assert(buf.front()==0x1234&&buf.back()==0x5678);}Frame muted=f;muted.control.mute=true;muted.watchdog=true;renderStrip(muted,0,q,buf.data()+1);assert(buf[1+4*width+300]==rgb(255,184,32));}
        std::cout<<"mode "<<m<<" hash "<<std::hex<<h<<std::dec<<'\n';
    }
    for(unsigned q:{1u,2u,4u}){auto start=std::chrono::steady_clock::now();uint64_t sum=0;for(unsigned i=0;i<200;++i)for(int y=0;y<height;y+=strip){renderStrip(Frame{c,3+i%2,analysis,i},y,q,buf.data()+1);sum+=buf[1+i%(width*strip)];}std::cout<<"new modes step "<<q<<" ms/frame "<<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/200<<" checksum "<<sum<<'\n';}
    std::cout<<"PASS eight-mode replay and real audio watchdog/rearm\n";
}
