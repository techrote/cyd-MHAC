#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
using namespace mhac::visual;
int main(int argc,char** argv) {
    unsigned area=0, pixels=0, notes=0;
    for(int y=-1;y<=240;++y) for(int x=-1;x<=320;++x) {
        const bool hit=sceneSelector(x,y); area+=hit;
        assert(!hit || (!soundSelector(x,y) && !paletteSelector(x,y) && !xyMapSelector(x,y) && factoryAction(x,y)==FactoryAction::None));
    }
    assert(area==1152);
    for(bool muted:{true,false}) {
        PerformanceSurface ui; uint32_t now=0;
        auto touch=[&](bool down,int x,int y){ui.sample(down,x,y,now+=8);};
        auto tap=[&](int x,int y){touch(true,x,y);touch(false,0,0);};
        if(!muted) tap(310,10);
        for(unsigned j=0;j<scene_count*2;++j) {
            const auto i=j%scene_count; const auto& s=scenes[i];
            touch(true,60,230); touch(true,180,120);
            const auto c=ui.control();
            assert(c.patch==s.patch && ui.mode()==s.mode && ui.palette()==s.palette && ui.mapping()==s.mapping && ui.scale()==s.scale);
            assert(c.x==mapPitch(s.x,s.scale) && c.y==s.y && c.mute==muted && !c.gate && ui.factoryIndex()==factory_count);
            touch(false,0,0);
            Frame f{}; f.control=c; f.mode=ui.mode(); f.palette=ui.palette(); f.mapping=ui.mapping(); f.scale=ui.scale();
            for(unsigned step:{1u,2u,4u}) {
                std::array<uint16_t,width*strip> raster{}; renderStrip(f,224,step,raster.data());
                for(unsigned k=0;k<scene_count;++k) {
                    assert(raster[40+k*8]==(k==i?rgb(120,224,180):rgb(65,80,94))); ++pixels;
                }
            }
            touch(true,120,100); assert(!ui.control().gate);
            touch(true,120,100); assert(ui.control().gate==!muted);
            mhac::Exchange io; mhac::Engine engine; std::array<int16_t,mhac::block> pcm{};
            unsigned active=0; InkField ink; CellularField cellular; DiffusionField diffusion;
            for(unsigned k=0;k<160;++k) {
                touch(true,70+int(k%100)*2,55+int(k%90));
                const auto live=ui.effective(now); publishControl(io,live); engine.render(io,pcm.data(),pcm.size());
                const auto peak=io.snapshot()&4095u;
                if(s.mode==5) ink.advance(now,live.x,live.y,live.gate,peak);
                cellular.advance(now,s.mode==cellular_mode,live.x,live.y,live.gate,peak);
                diffusion.advance(now,s.mode==diffusion_mode,live.x,live.y,live.gate,peak);
                for(auto p:pcm) { assert(p>=-8191 && p<=8191); active+=p!=0; }
            }
            assert((active>0)==!muted && !io.watchdog_trips.load()); notes+=active;
            if(!muted && j<scene_count) {
                f.control=ui.control(); f.analysis=io.snapshot(); f.tick=now/25;
                f.ink=&ink; f.cellular=&cellular; f.diffusion=&diffusion;
                std::ofstream out;
                if(argc>1) { out.open(std::string(argv[1])+std::to_string(i)+".ppm",std::ios::binary); assert(out); out<<"P6\n320 240\n255\n"; }
                std::array<uint16_t,width*strip> raster{}; uint64_t hash=14695981039346656037ull;
                for(int y=0;y<height;y+=strip) {
                    renderStrip(f,y,1,raster.data());
                    for(auto v:raster) {
                        hash=(hash^v)*1099511628211ull;
                        if(out.is_open()) {
                            const unsigned char rgb8[]={static_cast<unsigned char>(((v>>11)&31u)*255/31),static_cast<unsigned char>(((v>>5)&63u)*255/63),static_cast<unsigned char>((v&31u)*255/31)};
                            out.write(reinterpret_cast<const char*>(rgb8),3);
                        }
                    }
                }
                if(out.is_open()) assert(out);
                std::printf("%s frame %016llx\n",s.name,static_cast<unsigned long long>(hash));
            }
            touch(false,0,0); assert(!ui.control().gate);
        }
        tap(230,230); assert(sceneIndex(ui.control().patch,ui.mode(),ui.palette(),ui.mapping(),ui.scale())==scene_count);
        tap(60,230); assert(ui.control().patch==scenes[0].patch && ui.mode()==scenes[0].mode);
        if(!muted) {
            tap(160,10); assert(ui.loopState()==GestureLoop::State::Armed);
            tap(60,230); assert(ui.loopState()==GestureLoop::State::Idle && !ui.control().gate);
            tap(160,10); touch(true,100,100); touch(true,100,100);
            for(unsigned k=0;k<8;++k) touch(true,140,100);
            touch(false,0,0); assert(ui.loopState()==GestureLoop::State::Playing);
            tap(60,230); assert(ui.loopState()==GestureLoop::State::Idle && !ui.effective(now).gate);
            ui.observeWatchdog(1); touch(false,0,0); tap(60,230); assert(ui.control().mute && !ui.control().gate);
        }
    }
    std::printf("scene PASS: %u selector pixels, %u overlay checks, %u active PCM samples, mute/loop/watchdog interlocks\n",area,pixels,notes);
}
