#include "mhac_audio.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace mhac;
static void require(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
static std::vector<int16_t> render(unsigned y,unsigned chunk) {
    Engine e; Exchange io; std::vector<int16_t> pcm(rate*5);
    for(unsigned off=0;off<pcm.size();) {
        const unsigned n=std::min<unsigned>(block,pcm.size()-off);
        io.publish_patch(1365,y,off>=rate/4 && off<rate*4,off<rate/4 || off>=rate*4,Patch::ErasurePulse);
        for(unsigned i=0;i<n;) { const unsigned take=std::min(chunk,n-i); e.render(io,pcm.data()+off+i,take); i+=take; }
        off+=n;
    }
    require(io.watchdog_trips.load()==0,"unexpected watchdog");
    require(std::all_of(pcm.end()-block,pcm.end(),[](int16_t v){return v==0;}),"final silence");
    for(auto v:pcm) require(std::abs(int(v))<=8191,"PCM bound");
    return pcm;
}
static uint64_t energy(const std::vector<int16_t>& pcm,unsigned start,unsigned end) {
    uint64_t sum=0; for(unsigned i=start;i<end;++i) sum+=int64_t(pcm[i])*pcm[i]; return sum;
}
static void raw(const std::string& path,const std::vector<int16_t>& pcm) {
    std::ofstream f(path,std::ios::binary); for(auto v:pcm) { auto u=uint16_t(v); f.put(char(u&255u)); f.put(char(u>>8)); }
    f.close(); require(bool(f),"PCM write");
}
int main(int argc,char** argv) {
    require(argc<=2,"usage: erasure_relocation_test [output-prefix]");
    const auto dry=render(0,128), middle=render(2048,128), full=render(4095,128);
    unsigned checks=0;
    for(unsigned y:{0u,2048u,4095u}) {
        const auto p=render(y,128);
        require(p==render(y,31) && p==render(y,97),"partition mismatch"); checks+=2*p.size();
        if(argc==2) raw(std::string(argv[1])+std::to_string(y)+".pcm",p);
    }
    // advance_motif increments before evaluating, so t=5100 is sample 5099
    // in each 12000-sample period. Use settled windows well inside the pulses.
    for(unsigned start=rate;start<rate*4;start+=motif_period) {
        auto second=[&](const auto& p){return energy(p,start+3100,start+3800);};
        auto ghost=[&](const auto& p){return energy(p,start+5300,start+5600);};
        require(second(dry)>0 && second(full)<second(dry)/100,"second onset not erased");
#ifndef MHAC_ERASURE_PARENT
        require(ghost(full)>ghost(middle) && ghost(middle)>ghost(dry),"relocation energy ordering");
        require(ghost(full)>100000000u && ghost(dry)<ghost(full)/100,"ghost energy below threshold");
        require(energy(full,start+4700,start+4900)<ghost(full)/100,"unexpected early pulse");
#endif
        std::cout<<"cycle="<<start/motif_period<<" second="<<second(dry)<<','<<second(full)<<" ghost="<<ghost(dry)<<','<<ghost(middle)<<','<<ghost(full)<<'\n';
    }
    std::cout<<"partition_comparisons="<<checks<<" engine_bytes="<<sizeof(Engine)<<'\n';
}
