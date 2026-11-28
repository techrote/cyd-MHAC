#include "mhac_ink.hpp"
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
using mhac::visual::InkField;
int main(int argc,char** argv) {
    static_assert(sizeof(InkField)<=4832);
    InkField a,b;
    a.advance(0,0,0,true,4095); assert(a.cell(0,59)==255);
    a.advance(25,4095,4095,true,4095); assert(a.cell(79,0)==255 && a.cell(39,30)>0);
    a.advance(50,0,0,false,0); assert(a.cell(79,0)==249);
    a.advance(1125,0,0,false,0); assert(a.cell(79,0)==0);
    a.advance(1150,0,0,true,0); a.advance(1175,0,0,false,0); a.advance(1200,4095,4095,true,0); assert(a.cell(39,30)==0);
    b.advance(0xfffffff0u,4095,4095,true,4095); b.advance(9,0,0,false,0); assert(b.cell(79,0)==249);
    InkField disconnected; disconnected.advance(0,0,0,true,4095); disconnected.disconnect(); disconnected.advance(25,4095,4095,true,4095); assert(disconnected.cell(39,30)==0);
    InkField c,d;
    c.advance(0,2048,2048,true,4095);d=c;
    c.advance(100,0,0,false,0);for(unsigned t=10;t<=100;t+=10)d.advance(t,0,0,false,0);
    assert(c.cell(39,30)==d.cell(39,30));
    // Repeatable gesture trajectory; synthetic peak values, not audio measurement.
    auto draw=[](InkField& f,unsigned i){const unsigned x=(i*193u)%4096u,y=(i*97u+(i%17u)*160u)%4096u;f.advance(i*25u,x,y,i%19u!=0,(i*137u)%4096u);};
    InkField f,g;for(unsigned i=0;i<180;++i){draw(f,i);draw(g,i);}
    uint64_t hash=14695981039346656037ull;
    for(int y=0;y<60;++y)for(int x=0;x<80;++x){assert(f.cell(x,y)==g.cell(x,y));hash=(hash^f.cell(x,y))*1099511628211ull;}
    assert(f.cell(-100,100)==f.cell(0,59));
    if(argc==2){std::ofstream out(argv[1],std::ios::binary);assert(out);out<<"P6\n320 240\n255\n";for(int y=0;y<240;++y)for(int x=0;x<320;++x){auto s=f.shade(x,y);const unsigned char p[]={static_cast<unsigned char>((std::clamp(s.r,0,255)>>3)*255/31),static_cast<unsigned char>((std::clamp(s.g,0,255)>>2)*255/63),static_cast<unsigned char>((std::clamp(s.b,0,255)>>3)*255/31)};out.write(reinterpret_cast<const char*>(p),3);}}
    auto start=std::chrono::steady_clock::now();for(unsigned i=180;i<10180;++i)draw(f,i);
    std::cout<<"PASS ink field bytes="<<sizeof(InkField)<<" hash="<<std::hex<<hash<<std::dec<<" update us="<<std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count()/10000<<"\n";
}
