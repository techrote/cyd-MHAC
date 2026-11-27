#include "mhac_audio.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <chrono>
#include <vector>
using namespace mhac;
static uint64_t hash(const std::vector<int16_t>& v) {
    uint64_t h=14695981039346656037ull;
    for(auto s:v) { assert(s>=-8191 && s<=8191); h=(h^uint16_t(s))*1099511628211ull; }
    return h;
}
static std::vector<int16_t> render(Patch p, bool split) {
    Exchange io; Engine e; std::vector<int16_t> v(rate*2);
    for(unsigned k=0;k<v.size();k+=block) {
        unsigned n=std::min<unsigned>(block,v.size()-k);
        io.publish_patch(2300,3400,k<rate,false,p);
        if(split){unsigned m=n/3;e.render(io,v.data()+k,m);e.render(io,v.data()+k+m,n-m);}
        else e.render(io,v.data()+k,n);
    }
    assert(io.watchdog_trips.load()==0);
    for(unsigned k=rate+6000;k<v.size();++k) assert(v[k]==0);
    return v;
}
int main(int argc,char** argv) {
    std::array<uint64_t,patch_count> hashes{};
    for(unsigned i=0;i<patch_count;++i){
        auto t=std::chrono::steady_clock::now();
        auto v=render(static_cast<Patch>(i),false);auto u=render(static_cast<Patch>(i),true);
        assert(v==u);hashes[i]=hash(v);for(unsigned j=0;j<i;++j)assert(hashes[i]!=hashes[j]);
        std::printf("patch %u hash %016llx two renders %.3f ms\n",i,(unsigned long long)hashes[i],std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count());
        if(argc>1){char filename[256];std::snprintf(filename,sizeof(filename),"%s/patch%u.raw",argv[1],i);auto f=std::fopen(filename,"wb");assert(f);assert(std::fwrite(v.data(),sizeof(int16_t),v.size(),f)==v.size());assert(std::fclose(f)==0);}
    }
    assert(render(static_cast<Patch>(99),false)==render(Patch::PhaseMorph,false));
    Exchange io; Engine e; std::array<int16_t,block> out{};
    for(unsigned k=0;k<1000;++k){io.publish_patch(4095,4095,true,false,static_cast<Patch>(k%patch_count));e.render(io,out.data(),block);for(auto s:out)assert(s>=-8191&&s<=8191);}
    for(unsigned k=0;k<110;++k)e.render(io,out.data(),block);
    assert(io.watchdog_trips.load()==1);for(auto s:out)assert(s==0);
    io.publish_patch(4095,4095,true,false,Patch::HarmonicLattice);e.render(io,out.data(),block);for(auto s:out)assert(s==0);
    io.publish_patch(4095,4095,false,false,Patch::RingDialect);e.render(io,out.data(),block);
    io.publish_patch(4095,4095,true,false,Patch::RingDialect);e.render(io,out.data(),block);assert((io.snapshot()&4095u)>0);
    // Legacy four-argument publisher retains the original v1 word and waveform.
    Exchange a,b;Engine ea,eb;std::array<int16_t,block> x{},y{};
    uint64_t legacy=14695981039346656037ull;
    ea.render(a,x.data(),block);eb.render(b,y.data(),block);
    for(unsigned k=0;k<1000;++k){a.publish(k*29,k*17,true,false);b.publish_patch(k*29,k*17,true,false,Patch::PhaseMorph);ea.render(a,x.data(),block);eb.render(b,y.data(),block);assert(x==y);for(auto s:x)legacy=(legacy^uint16_t(s))*1099511628211ull;}
    // Toolchain-specific receipt; exact historical PCM comparison is in legacy_compare.cpp.
    std::printf("legacy publisher hash %016llx\n",(unsigned long long)legacy);
    std::puts("PASS: four distinct modes, partition equality, invalid patch fallback, rapid switching, watchdog/rearm and legacy publisher equivalence");
}
