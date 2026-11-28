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
    Calibration cal; int x,y; cal.landscape(3900,200,x,y); assert(x==0&&y==0);
    cal.landscape(200,3900,x,y); assert(x==319&&y==239);
    cal.landscape(-1,6000,x,y); assert(x==319&&y==239);
    Surface s; assert(s.control().mute&&!s.control().gate);
    s.sample(true,310,10,1); assert(!s.control().mute&&!s.control().gate);
    s.sample(true,150,120,10); assert(!s.control().gate); // control press consumes drag
    s.sample(false,0,0,20); s.sample(true,120,80,30); assert(!s.control().gate);
    s.sample(true,200,120,38); assert(s.control().gate&&s.control().x>1500);
    s.expire(139); assert(!s.control().gate&&s.control().mute);
    s.sample(true,200,120,140); assert(!s.control().gate);
    s.sample(false,0,0,141); s.sample(true,10,10,150); assert(s.mode()==1);
    s.sample(true,10,10,160); assert(s.mode()==1); s.sample(false,0,0,170);
    s.sample(true,310,10,180); s.sample(false,0,0,190);
    s.sample(true,215,145,200); s.sample(true,215,145,208); assert(s.control().gate);
    mhac::Exchange io; mhac::Engine engine; std::array<int16_t,mhac::block> pcm{};
    const auto c=s.control(); io.publish(c.x,c.y,c.gate,c.mute);
    for(int i=0;i<64;++i) engine.render(io,pcm.data(),pcm.size());
    const auto analysis=io.snapshot(); assert((analysis&4095)>0);
    std::array<uint16_t,width*strip+2> guard{}; guard.front()=0x1234;guard.back()=0x5678;
    uint64_t hashes[3]{};
    for(unsigned mode=0;mode<3;++mode) {
        Frame f{c,mode,analysis,17};
        std::ofstream out;
        if(argc==2) { out.open(std::string(argv[1])+"/mode"+std::to_string(mode)+".ppm",std::ios::binary); assert(out);out<<"P6\n320 240\n255\n"; }
        uint64_t h=14695981039346656037ull;
        for(int j=0;j<height;j+=strip) {
            renderStrip(f,j,1,guard.data()+1);
            for(int k=1;k<=width*strip;++k) {
                const uint16_t v=guard[k]; h=(h^v)*1099511628211ull;
                if(out.is_open()) { const unsigned char rgb8[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)};out.write(reinterpret_cast<const char*>(rgb8),3); }
            }
        }
        hashes[mode]=h; std::cout<<"mode "<<mode<<" hash "<<std::hex<<h<<std::dec<<'\n';
        for(unsigned q:{2u,4u}) for(int j=0;j<height;j+=strip) {
            renderStrip(f,j,q,guard.data()+1); assert(guard[1]==guard[2]);
        }
        assert(guard.front()==0x1234&&guard.back()==0x5678);
    }
    assert(hashes[0]!=hashes[1]&&hashes[1]!=hashes[2]&&hashes[0]!=hashes[2]);
    for(unsigned q:{1u,2u,4u}) {
        Frame f{c,0,analysis,17}; f.control.mute=false; renderStrip(f,0,q,guard.data()+1);
        assert(guard[1+4*width+300]==rgb(64,220,150));
        f.control.mute=true; renderStrip(f,0,q,guard.data()+1);
        assert(guard[1+4*width+300]==rgb(255,72,48));
    }
    for(unsigned q:{1u,2u,4u}) {
        const auto start=std::chrono::steady_clock::now(); uint64_t sum=0;
        for(unsigned i=0;i<180;++i) for(int j=0;j<height;j+=strip) {
            renderStrip(Frame{c,i%3,analysis,i},j,q,guard.data()+1);sum+=guard[1+(i% (width*strip))];
        }
        const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/180;
        std::cout<<"step "<<q<<" host render ms/frame "<<ms<<" checksum "<<sum<<'\n';
    }
    // The palette selector must consume a whole contact, not trigger sound or alter its coordinates.
    Surface ps;
    uint32_t pt=0;
    for(bool muted:{true,false}) {
        if(!muted) { ps.sample(true,310,10,pt+=8); ps.sample(false,0,0,pt+=8); }
        const auto before=ps.control(); assert(before.mute==muted);
        for(unsigned i=0;i<4000;++i) {
            ps.sample(true,230,228,pt+=8);
            ps.sample(true,160,100,pt+=8);
            const auto after=ps.control();
            assert(unsigned(ps.palette())==(i+1)%palette_count);
            assert(!after.gate && after.mute==before.mute && after.patch==before.patch && after.x==before.x && after.y==before.y);
            ps.sample(false,0,0,pt+=8);
        }
    }
    uint64_t paletteHashes[palette_count]{};
    for(unsigned i=0;i<palette_count;++i) {
        Frame f{c,0,analysis,17}; f.palette=static_cast<Palette>(i);
        std::ofstream out;
        if(argc==2) { out.open(std::string(argv[1])+"/palette-"+paletteName(f.palette)+".ppm",std::ios::binary); assert(out); out<<"P6\n320 240\n255\n"; }
        uint64_t h=14695981039346656037ull;
        for(int j=0;j<height;j+=strip) {
            renderStrip(f,j,1,guard.data()+1);
            for(int k=1;k<=width*strip;++k) {
                const auto v=guard[k]; h=(h^v)*1099511628211ull;
                if(out.is_open()) { const unsigned char rgb8[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)}; out.write(reinterpret_cast<const char*>(rgb8),3); }
            }
        }
        paletteHashes[i]=h;
        for(unsigned j=0;j<i;++j) assert(paletteHashes[j]!=h);
        for(unsigned q:{1u,2u,4u}) {
            f.control.mute=true;
            renderStrip(f,0,q,guard.data()+1); assert(guard[1+4*width+300]==rgb(255,72,48));
            renderStrip(f,224,q,guard.data()+1);
            const auto sw=paletteSwatch(f.palette);
            assert(guard[1+216+8*i]==rgb(sw.r,sw.g,sw.b));
            assert(guard.front()==0x1234&&guard.back()==0x5678);
        }
        f.mode=6; Frame original=f; original.palette=Palette::Original;
        for(int y=0;y<height;++y) for(int x=0;x<width;++x)
            if(!(x>=216&&x<248&&y>=224&&y<236)) assert(pixel(f,x,y)==pixel(original,x,y));
        std::cout<<"palette "<<paletteName(f.palette)<<" hash "<<std::hex<<h<<std::dec<<'\n';
    }
    std::cout<<"PASS 8000 safe palette contacts, four actual DSP-driven captures, unchanged Trace and mute overlays\n";
    s.sample(false,0,0,220); assert(!s.control().gate); s.forceMute();assert(s.control().mute);
    std::cout<<"PASS visual replay, real DSP snapshot, bounds and release\n";
}
