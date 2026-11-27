#include "mhac_diffusion.hpp"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <string>
using mhac::visual::DiffusionField;
static void equal(const DiffusionField& a,const DiffusionField& b) {
    for(int y=0;y<24;++y) for(int x=0;x<32;++x) assert(a.cell(x,y)==b.cell(x,y));
}
static unsigned pigment(const DiffusionField& f,int x,int y) { return f.cell(x,y)>>13; }
static void capture(const DiffusionField& f,const char* prefix,const char* suffix) {
    if(!prefix) return;
    std::ofstream out(std::string(prefix)+suffix+".ppm",std::ios::binary);
    assert(out); out<<"P6\n320 240\n255\n";
    for(int y=0;y<240;++y) for(int x=0;x<320;++x) {
        const auto s=f.shade(x,y);
        assert(s.r>=0&&s.r<=255&&s.g>=0&&s.g<=255&&s.b>=0&&s.b<=255);
        const unsigned char rgb[]={static_cast<unsigned char>((s.r>>3)*255/31),static_cast<unsigned char>((s.g>>2)*255/63),static_cast<unsigned char>((s.b>>3)*255/31)};
        out.write(reinterpret_cast<const char*>(rgb),3);
    }
    assert(out);
}
int main(int argc,char** argv) {
    const char* prefix=argc>1?argv[1]:nullptr;
    DiffusionField held,lifted;
    for(auto* f:{&held,&lifted}) { f->advance(0,true,0,0,false,0); f->advance(12,true,0,2048,true,0); }
    lifted.advance(13,true,0,0,false,0); // lift between simulation ticks
    held.advance(24,true,4095,2048,true,0); lifted.advance(24,true,4095,2048,true,0);
    unsigned continuous=0,disconnected=0;
    for(int x=4;x<28;++x) { continuous+=pigment(held,x,12)>512; disconnected+=pigment(lifted,x,12)>512; }
    assert(continuous==24&&disconnected==0);
    capture(held,prefix,"held"); capture(lifted,prefix,"lifted");
    DiffusionField paused=held,reference=held;
    paused.advance(25,false,4095,0,true,4095);
    paused.advance(1000000,false,4095,0,true,4095); equal(paused,held);
    paused.advance(1000001,true,0,0,false,0); equal(paused,held);
    reference.advance(24,true,0,0,false,0);
    for(unsigned i=1;i<=120;++i) {
        paused.advance(1000001+i*12,true,0,0,false,1024);
        reference.advance(24+i*12,true,0,0,false,1024); equal(paused,reference);
    }
    capture(paused,prefix,"evolved");
    DiffusionField noBridge=held,explicitLift=held;
    noBridge.advance(25,false,0,0,false,0);
    noBridge.advance(26,true,0,0,true,0);
    explicitLift.advance(24,true,0,0,false,0);
    noBridge.advance(38,true,0,0,true,0);
    explicitLift.advance(36,true,0,0,true,0); equal(noBridge,explicitLift);
    DiffusionField wrap,plain;
    wrap.advance(0xfffffff0u,true,0,0,false,0); plain.advance(0,true,0,0,false,0);
    for(unsigned i=1;i<=128;++i) {
        const unsigned x=(i*997u)%4096u,y=(i*383u)%4096u;
        wrap.advance(0xfffffff0u+i*12,true,x,y,true,65535);
        plain.advance(i*12,true,x,y,true,65535); equal(wrap,plain);
        for(int j=0;j<24;++j) for(int k=0;k<32;++k) {const auto c=wrap.cell(k,j);assert((c&8191u)<=4096u&&(c>>13)<=4096u);}
    }
    capture(wrap,prefix,"diagonal");
    std::printf("diffusion memory PASS: %u connected, %u disconnected interior cells; 251 exact grid comparisons; 98304 cell bounds; %zu bytes\n",continuous,disconnected,sizeof(DiffusionField));
}
