#include "mhac_visual_audio.hpp"
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
using namespace mhac::visual;
int main(int argc,char** argv) {
    static_assert(mhac::feature_version==1 && mhac::rate==24000 && mhac::feature_window==128);
    mhac::Engine engine; mhac::Exchange io; Trace trace;
    mhac::AudioFeatures f{}; std::array<int16_t,mhac::block> pcm{};
    assert(!io.features.read(f));
    unsigned observed=0;
    for(unsigned k=0;k<640;++k) {
        io.publish_patch(2500, (k*17u)&4095u, k<560, false, mhac::Patch::ErasurePulse);
        engine.render(io,pcm.data(),pcm.size());
        if(k%7==0 && !(k>=260 && k<300)) {
            assert(io.features.read(f)); assert(f.sample_end==(k+1)*mhac::block);
            assert(trace.observe(f,k*128u/24u)); ++observed;
            assert(!trace.observe(f,k*128u/24u+1));
        }
    }
    unsigned seen=0,active=0,onset=0;
    for(unsigned x=0;x<Trace::columns;++x){auto b=trace.at(x);seen+=b.seen;active+=b.amplitude!=0;onset+=b.onset!=0;}
    assert(seen>60 && seen<80 && active>10 && onset>5);
    assert(trace.fresh(3414) && !trace.fresh(3700));
    Frame frame{}; frame.mode=6; frame.trace=&trace; frame.trace_fresh=true;
    std::array<uint16_t,width*strip+2> pixels{};pixels.front()=0x1234;pixels.back()=0x5678;
    std::ofstream out;if(argc==2){out.open(argv[1],std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";}
    for(int y=0;y<height;y+=strip) {
        renderStrip(frame,y,1,pixels.data()+1);
        assert(pixels.front()==0x1234&&pixels.back()==0x5678);
        if(out.is_open()) for(int i=1;i<=width*strip;++i){auto v=pixels[i];const unsigned char c[]={static_cast<unsigned char>(((v>>11)&31)*255/31),static_cast<unsigned char>(((v>>5)&63)*255/63),static_cast<unsigned char>((v&31)*255/31)};out.write(reinterpret_cast<const char*>(c),3);}
    }
    for(unsigned q:{1u,2u,4u}){
        renderStrip(frame,0,q,pixels.data()+1);assert(pixels[1+4*width+32]==rgb(64,220,150));
        frame.trace_fresh=false;renderStrip(frame,0,q,pixels.data()+1);assert(pixels[1+4*width+32]==rgb(255,184,32));frame.trace_fresh=true;
        for(int y=0;y<height;y+=strip) {renderStrip(frame,y,q,pixels.data()+1);assert(pixels.front()==0x1234&&pixels.back()==0x5678);}
    }
    std::cout<<"PASS real Erasure Pulse trace; observations "<<observed<<" seen bins "<<seen<<" active bins "<<active<<" onset bins "<<onset<<'\n';
}
