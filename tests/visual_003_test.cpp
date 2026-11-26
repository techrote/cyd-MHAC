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
    auto publish=[&]{publishControl(io,ui.control()); assert(((io.control.load()>>26)&7u)==ui.control().patch);};
    auto touch=[&](bool d,int x,int y){now+=8;ui.sample(d,x,y,now);publish();};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,x,y);};
    auto render=[&]{publish();synth.render(io,pcm.data(),pcm.size());};
    auto hold=[&]{touch(true,190,70);touch(true,190,70);assert(ui.control().gate);};
    std::array<uint64_t,sound_count> hashes{};
    for(unsigned p=0;p<sound_count;++p) {
        assert(ui.control().patch==p && ui.mode()==paired_mode[p]);
        assert(ui.control().mute);tap(310,10);hold();
        uint64_t h=14695981039346656037ull; uint32_t best=0;
        for(unsigned i=0;i<192;++i) {render();auto a=io.snapshot();if((a&4095u)>(best&4095u))best=a;for(auto v:pcm){assert(v>=-8191&&v<=8191);h=(h^uint16_t(v))*1099511628211ull;}}
        assert(io.watchdog_trips.load()==0 && (best&4095u)>0);
        hashes[p]=h;for(unsigned k=0;k<p;++k)assert(hashes[k]!=h);
        std::array<uint16_t,width*strip+2> buf{};buf.front()=0x1234;buf.back()=0x5678;
        InkField ink; ink.advance(0,1000,1000,true,best&4095u); ink.advance(25,ui.control().x,ui.control().y,true,best&4095u);
        Frame f{ui.control(),ui.mode(),best,17,false,&ink,ui.scale()};
        for(unsigned q:{1u,2u,4u}) {renderStrip(f,232,q,buf.data()+1);for(unsigned k=0;k<sound_count;++k)assert(buf[1+k*4]==(k==p?rgb(238,220,128):rgb(65,80,94)));assert(buf.front()==0x1234&&buf.back()==0x5678);}
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
    // Scale control consumes a fresh touch but never a musical drag.
    Surface musical; mhac::Exchange scaledIO; uint32_t ms=0;
    auto press=[&](bool d,int x,int y){musical.sample(d,x,y,ms+=8);publishControl(scaledIO,musical.control());};
    auto click=[&](int x,int y){press(true,x,y);press(false,x,y);};
    assert(musical.scale()==Scale::Free);
    press(true,310,230); assert(musical.scale()==Scale::Chromatic&&!musical.control().gate);
    press(true,150,100); assert(!musical.control().gate);press(false,0,0);
    click(310,10);press(true,101,100);press(true,101,100);
    const unsigned raw=101u*4095u/319u;
    assert(musical.control().gate&&musical.control().x==mapPitch(raw,Scale::Chromatic));
    press(true,310,230);assert(musical.scale()==Scale::Chromatic&&musical.control().gate);
    assert((scaledIO.control.load()&4095u)==musical.control().x);press(false,0,0);
    click(310,230);assert(musical.scale()==Scale::MinorPentatonic);
    press(true,101,100);press(true,102,100);
    const unsigned raw2=(raw+3u*(102u*4095u/319u)+2)/4;
    assert(musical.control().x==mapPitch(raw2,Scale::MinorPentatonic));
    press(false,0,0);click(310,230);assert(musical.scale()==Scale::Free&&musical.control().x==raw2);
    for(unsigned s=0;s<scale_count;++s) for(unsigned q:{1u,2u,4u}) {
        Frame f{};f.scale=static_cast<Scale>(s);std::array<uint16_t,width*strip> pixels{};
        renderStrip(f,232,q,pixels.data());
        for(unsigned k=0;k<scale_count;++k) assert(pixels[296+8*k]==(k==s?rgb(116,210,240):rgb(65,80,94)));
    }
    std::cout<<"PASS six paired modes, independent visual selection, gesture ownership, pitch maps, heartbeat, watchdog and mute\n";
}
