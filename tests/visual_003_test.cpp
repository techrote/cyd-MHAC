#include "mhac_visual_audio.hpp"
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    Surface ui; mhac::Exchange io; mhac::Engine synth;
    std::array<int16_t,mhac::block> pcm{};
    uint32_t now=0;
    auto publish=[&]{publishControl(io,ui.control()); assert(((io.control.load()>>26)&3u)==ui.control().patch);};
    auto touch=[&](bool d,int x,int y){now+=8;ui.sample(d,x,y,now);publish();};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,x,y);};
    auto render=[&]{publish();synth.render(io,pcm.data(),pcm.size());};
    auto hold=[&]{touch(true,190,70);touch(true,190,70);assert(ui.control().gate);};
    std::array<uint64_t,sound_count> hashes{};
    for(unsigned p=0;p<sound_count;++p) {
        assert(ui.control().patch==p && ui.mode()==paired_mode[p]);
        assert(ui.control().mute);tap(310,10);hold();
        uint64_t h=14695981039346656037ull;
        for(unsigned i=0;i<192;++i) {render();for(auto v:pcm){assert(v>=-8191&&v<=8191);h=(h^uint16_t(v))*1099511628211ull;}}
        assert(io.watchdog_trips.load()==0 && (io.snapshot()&4095u)>0);
        hashes[p]=h;for(unsigned k=0;k<p;++k)assert(hashes[k]!=h);
        std::array<uint16_t,width*strip+2> buf{};buf.front()=0x1234;buf.back()=0x5678;
        Frame f{ui.control(),ui.mode(),io.snapshot(),17};
        for(unsigned q:{1u,2u,4u}) {renderStrip(f,232,q,buf.data()+1);for(unsigned k=0;k<sound_count;++k)assert(buf[1+4+k*4]==(k==p?rgb(238,220,128):rgb(65,80,94)));assert(buf.front()==0x1234&&buf.back()==0x5678);}
        if(argc==2){std::ofstream out(std::string(argv[1])+"/pair"+std::to_string(p)+".ppm",std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";for(int y=0;y<height;y+=strip){renderStrip(f,y,1,buf.data()+1);for(int i=1;i<=width*strip;++i){auto v=buf[i];const unsigned char a[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)};out.write(reinterpret_cast<const char*>(a),3);}}}
        touch(true,10,230);assert(ui.control().gate&&ui.control().patch==p); // drag into selector is still music
        touch(false,0,0);assert(!ui.control().gate);render();
        tap(310,10);assert(ui.control().mute&&ui.control().patch==p);
        touch(true,10,230);const unsigned next=(p+1)%sound_count;assert(ui.control().patch==next&&!ui.control().gate);
        touch(true,180,120);assert(!ui.control().gate&&ui.control().patch==next); // consumed until release
        touch(false,0,0);assert(ui.control().patch==next);
        std::cout<<"pair "<<p<<" pcm hash "<<std::hex<<h<<std::dec<<'\n';
    }
    tap(10,10);assert(ui.mode()==1 && ui.control().patch==0); // independent visual choice
    tap(10,230);assert(ui.control().patch==1 && ui.mode()==1);
    tap(310,10);hold();for(unsigned i=0;i<32;++i)render();
    for(unsigned i=0;i<120;++i)synth.render(io,pcm.data(),pcm.size());
    assert(io.watchdog_trips.load()==1);ui.observeWatchdog(io.watchdog_trips.load());publish();
    touch(true,10,230);assert(ui.control().patch==1 && ui.control().mute);
    touch(false,0,0);render();tap(10,230);assert(ui.control().patch==2&&ui.watchdogLatched()&&ui.control().mute);
    tap(310,10);assert(!ui.watchdogLatched()&&!ui.control().mute&&ui.control().patch==2);
    hold();for(unsigned i=0;i<32;++i)render();assert((io.snapshot()&4095u)>0);
    ui.forceMute();publish();assert(ui.control().patch==2 && ui.control().mute);for(unsigned i=0;i<64;++i)render();for(auto v:pcm)assert(v==0);
    std::cout<<"PASS four paired modes, independent visual selection, gesture ownership, heartbeat, watchdog and mute\n";
}
