#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
static void rhythmIntegration(const char* prefix) {
    unsigned points=0, jitter=0;
    for(int y=-1;y<=240;++y) {
        const auto b=rhythmBand(y); assert(b<4 && rhythmNearest(rhythmValue(b))==b);
        assert(rhythmBand(rhythmCenter(b))==b); ++points;
    }
    for(int edge:{60,120,180}) {
        const unsigned low=unsigned(edge/60-1), high=low+1;
        for(int d=-3;d<=2;++d) {
            assert(rhythmTracked(239-edge-d,low,false)==low);
            assert(rhythmTracked(239-edge-d,high,false)==high); jitter+=2;
        }
        assert(rhythmTracked(239-edge-3,low,false)==high);
        assert(rhythmTracked(239-edge+4,high,false)==low);
        assert(rhythmTracked(239-edge,low,true)==high);
    }
    PerformanceSurface ui; uint32_t t=0;
    auto touch=[&](bool down,int x,int y){ui.sample(down,x,y,t+=8);};
    auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
    tap(310,10); for(unsigned i=0;i<4;++i) tap(12,230);
    tap(160,10); assert(ui.loopState()==GestureLoop::State::Armed);
    const auto initial=ui.control();
    for(unsigned i=0;i<4;++i) tap(290,230);
    assert(ui.mapping()==XYMap::Rhythm && ui.loopState()==GestureLoop::State::Idle);
    assert(ui.control().patch==initial.patch && ui.control().x==initial.x && ui.control().y==initial.y && !ui.control().gate);
    std::array<uint64_t,4> hashes{}; unsigned active=0;
    for(unsigned b=0;b<4;++b) {
        mhac::Engine synth; mhac::Exchange io; std::array<int16_t,mhac::block> pcm{};
        const int y=rhythmCenter(b);
        touch(true,140,y); assert(!ui.control().gate); touch(true,140,y);
        assert(ui.control().gate && ui.control().y==rhythmValue(b));
        uint64_t h=14695981039346656037ull; unsigned nactive=0;
        for(unsigned n=0;n<192;++n) {
            touch(true,140,y); publishControl(io,ui.effective(t)); synth.render(io,pcm.data(),pcm.size());
            for(auto v:pcm) { assert(v>=-8191 && v<=8191); h=(h^uint16_t(v))*1099511628211ull; nactive+=v!=0; }
        }
        assert(nactive && !io.watchdog_trips.load()); active+=nactive; hashes[b]=h;
        for(unsigned j=0;j<b;++j) assert(hashes[j]!=h);
        Frame f{}; f.control=ui.control(); f.mode=ui.mode(); f.mapping=ui.mapping(); f.analysis=io.snapshot(); f.tick=80;
        assert(frameCursor(f).y==y);
        for(unsigned q:{1u,2u,4u}) {
            std::array<uint16_t,width*strip+2> p{}; p.front()=0x1234; p.back()=0x5678;
            renderStrip(f,224,q,p.data()+1); assert(p[1+288]==rgb(180,162,240));
            renderStrip(f,24,q,p.data()+1); assert(p[1+4*width+8]==rgb(18+255/2,18+255*3/4,18+255));
            renderStrip(f,204,q,p.data()+1); assert(p[1+4*width+20]==rgb(18+255/2,18+255*3/4,18+255));
            renderStrip(f,24,q,p.data()+1); assert(p[1+4*width+20]==rgb(18,18,18));
            assert(p.front()==0x1234 && p.back()==0x5678);
        }
        if(prefix) {
            std::ofstream out(std::string(prefix)+std::to_string(b)+".ppm",std::ios::binary);
            out<<"P6\n320 240\n255\n"; std::array<uint16_t,width*strip> p{};
            for(int j=0;j<height;j+=strip) { renderStrip(f,j,1,p.data()); for(auto v:p) {
                const unsigned char c[]={static_cast<unsigned char>(((v>>11)&31u)*255/31),static_cast<unsigned char>(((v>>5)&63u)*255/63),static_cast<unsigned char>((v&31u)*255/31)};
                out.write(reinterpret_cast<const char*>(c),3);
            }} assert(out);
        }
        touch(false,0,0); assert(!ui.control().gate);
    }
    touch(true,140,150); touch(true,140,150); assert(ui.control().gate);
    ui.expire(t+101); assert(ui.control().mute && !ui.control().gate);
    std::printf("rhythm PASS: %u coordinates, %u jitter checks, four distinct PCM hashes, %u active samples, all three strip tiers and stale mute\n",points,jitter,active);
}
int main(int argc,char** argv) {
    rhythmIntegration(argc>1?argv[1]:nullptr);
    XYMapper mapper;
    unsigned hits=0, points=0, relative=0;
    XYMap m=XYMap::Direct;
    for(unsigned i=0;i<3000;++i) { assert(unsigned(m)==i%xy_map_count); m=nextXYMap(m); }
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        hits+=xyMapSelector(x,y);
        const auto p=touchPoint(x,y), d=mapper.target(XYMap::Direct,p), s=mapper.target(XYMap::Swapped,p);
        assert(d.x==unsigned(std::clamp(x,0,319))*4095/319 && d.y==unsigned(239-std::clamp(y,0,239))*4095/239);
        assert(s.x==d.y && s.y==d.x && d.x<=4095 && d.y<=4095); ++points;
    }
    assert(hits==960);
    for(unsigned a:{0u,1u,1024u,2048u,4094u,4095u}) for(unsigned b:{0u,1u,1024u,2048u,4094u,4095u}) {
        mapper.begin({a,4095-a},{b,4095-b});
        const auto start=mapper.target(XYMap::Fine,{a,4095-a});
        assert(start.x==b && start.y==4095-b);
        unsigned previous=0;
        for(unsigned v=0;v<=4095;++v) {
            const auto p=mapper.target(XYMap::Fine,{v,4095-v});
            const auto expected=unsigned(std::clamp(int(b)+(int(v)-int(a))/2,0,4095));
            assert(p.x==expected && p.y==4095-expected && p.x>=previous);
            previous=p.x; ++relative;
        }
        const auto back=mapper.target(XYMap::Fine,{a,4095-a});
        assert(back.x==b && back.y==4095-b);
    }
    std::printf("XY mapping PASS: %u coordinate pairs, %u relative targets, %u selector pixels, 3000 cycles\n",points,relative,hits);
}
