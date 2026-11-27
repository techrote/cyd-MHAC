#include "mhac_audio.hpp"
#define mhac prior
#include "reference/audio_004.hpp"
#undef mhac
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>
using namespace mhac;
static uint64_t hash(const std::vector<int16_t>& v) {
    uint64_t h=14695981039346656037ull;
    for(auto s:v){assert(s>=-8191 && s<=8191);h=(h^uint16_t(s))*1099511628211ull;}
    return h;
}
static std::vector<int16_t> score(Patch p,bool split,unsigned y) {
    Exchange io; Engine e; std::vector<int16_t> v(rate*4);
    for(unsigned k=0;k<v.size();k+=block){
        const unsigned n=std::min<unsigned>(block,v.size()-k);
        const unsigned x=k<rate?1500:k<rate*2?2800:2000;
        io.publish_patch(x,y,k<rate*3,false,p);
        if(split){unsigned m=n/3;e.render(io,v.data()+k,m);e.render(io,v.data()+k+m,n-m);}
        else e.render(io,v.data()+k,n);
    }
    assert(io.watchdog_trips.load()==0);
    for(unsigned k=rate*3+6000;k<v.size();++k)assert(v[k]==0);
    return v;
}
int main(int argc,char** argv){
    Exchange io; Engine e; prior::Exchange ref; prior::Engine r;
    std::array<int16_t,block> a{},b{};
    for(unsigned k=0;k<3000;++k){
        const unsigned p=(k/43)%patch_count;
        const unsigned x=(k*619u)&4095u,y=(k*997u)&4095u,n=1+k%block;
        const bool gate=k%17!=0,mute=k%31==0;
        io.publish_patch(x,y,gate,mute,static_cast<Patch>(p));
        ref.publish_patch(x,y,gate,mute,static_cast<prior::Patch>(p));
        e.render(io,a.data(),n);r.render(ref,b.data(),n);
        assert(a==b && io.snapshot()==ref.snapshot() && io.control.load()==ref.control.load());
    }
    std::puts("PASS v1 four-patch exact mixed-switch differential");
    std::array<uint64_t,extended_patch_count> hashes{};
    for(unsigned p=0;p<extended_patch_count;++p){
        auto v=score(static_cast<Patch>(p),false,4095);assert(v==score(static_cast<Patch>(p),true,4095));
        hashes[p]=hash(v);for(unsigned j=0;j<p;++j)assert(hashes[p]!=hashes[j]);
        std::printf("patch %u hash %016llx\n",p,(unsigned long long)hashes[p]);
        if(argc>1 && p>=4){char filename[256];std::snprintf(filename,sizeof(filename),"%s/patch%u.raw",argv[1],p);auto f=std::fopen(filename,"wb");assert(f);assert(std::fwrite(v.data(),sizeof(int16_t),v.size(),f)==v.size());assert(std::fclose(f)==0);}
    }
    assert(score(Patch::ErasurePulse,false,0)!=score(Patch::ErasurePulse,false,4095));
    assert(score(Patch::RecursiveEcho,false,0)!=score(Patch::RecursiveEcho,false,4095));
    assert(score(static_cast<Patch>(6),false,4095)==score(Patch::PhaseMorph,false,4095));
    // Raw reserved IDs also fail safe, even if a publisher bypasses the typed API.
    Exchange bad,good; Engine be,ge; bad.publish_patch(0,0,true,false,Patch::PhaseMorph);good.publish(0,0,true,false);bad.control.fetch_or(7u<<26);be.render(bad,a.data(),block);ge.render(good,b.data(),block);
    assert(a==b && bad.snapshot()==good.snapshot());
    // The second onset is attenuated by Y; compare energy inside its window after settling.
    auto full=score(Patch::ErasurePulse,false,0), erased=score(Patch::ErasurePulse,false,4095);
    uint64_t ef=0,ee=0;
    for(unsigned i=rate+3000;i<rate+3800;++i){ef+=int64_t(full[i])*full[i];ee+=int64_t(erased[i])*erased[i];}
    assert(ef>0 && ee<ef/100);
    std::printf("second onset energy full=%llu erased=%llu\n",(unsigned long long)ef,(unsigned long long)ee);
    for(unsigned k=0;k<1000;++k){io.publish_patch(4095,4095,true,false,static_cast<Patch>(k%extended_patch_count));e.render(io,a.data(),block);for(auto s:a)assert(s>=-8191&&s<=8191);}
    for(auto p:{Patch::ErasurePulse,Patch::RecursiveEcho}){
        for(unsigned k=0;k<20;++k){io.publish_patch(4095,4095,true,false,p);e.render(io,a.data(),block);}
        unsigned before=io.watchdog_trips.load();
        for(unsigned k=0;k<110;++k)e.render(io,a.data(),block);
        assert(io.watchdog_trips.load()==before+1);for(auto s:a)assert(s==0);
        io.publish_patch(4095,4095,true,false,p);e.render(io,a.data(),block);for(auto s:a)assert(s==0);
        io.publish_patch(4095,4095,false,false,p);e.render(io,a.data(),block);
        io.publish_patch(4095,4095,true,false,p);e.render(io,a.data(),block);
    }
    std::printf("PASS six modes, split invariance, modulation, invalid IDs, rapid switches, watchdog; sizeof Engine=%zu (prior=%zu)\n",sizeof(Engine),sizeof(prior::Engine));
}
