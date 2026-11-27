#include "mhac_audio.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>
using namespace mhac;
static void send(Exchange& io,unsigned mode,unsigned x,unsigned y,bool gate,bool mute) {
    if(mode==6) io.publish_modal(x,y,gate,mute);
    else if(mode==7) io.publish_quotation(x,y,gate,mute);
    else if(mode==8) io.publish_counterquotation(x,y,gate,mute);
#ifndef LEGACY_ONLY
    else if(mode==9) io.publish_formant(x,y,gate,mute);
#endif
    else io.publish_patch(x,y,gate,mute,static_cast<Patch>(mode));
}
static uint64_t hash(const std::vector<int16_t>& v) {
    uint64_t h=14695981039346656037ull;
    for(auto s:v){assert(s>=-8191 && s<=8191);h=(h^uint16_t(s))*1099511628211ull;}
    return h;
}
static std::vector<int16_t> score(unsigned mode,unsigned variant,bool split) {
    Exchange io; Engine e; std::vector<int16_t> v(rate*6);
    for(unsigned k=0;k<v.size();k+=block) {
        const unsigned b=k/block,n=std::min<unsigned>(block,v.size()-k);
        const unsigned m=mode==10?b%10:mode==11?b%9:mode;
        const unsigned x=variant==0?((b*29u)&4095u):1365u;
        const unsigned y=variant==0?((b*41u)&4095u):variant==1?0u:4095u;
        // Control blackout 3.0..4.0s, stale held renewal, then explicit release/rearm.
        if(k<rate*3 || k>=rate*4) send(io,m,x,y,k<rate*5 && !(k>=rate*4+2048 && k<rate*4+4096),k>=rate*5+rate/2);
        if(split) {const unsigned p=n/3;e.render(io,v.data()+k,p);e.render(io,v.data()+k+p,n-p);}
        else e.render(io,v.data()+k,n);
    }
    assert(io.watchdog_trips.load()==1);
    for(unsigned k=rate*4-2048;k<rate*4+2048;++k) assert(v[k]==0);
    for(unsigned k=rate*6-2048;k<v.size();++k) assert(v[k]==0);
    return v;
}
int main(int argc,char** argv) {
    for(unsigned m=0;m<12;++m) {
#ifdef LEGACY_ONLY
        if(m==9 || m==10) continue;
#endif
        for(unsigned variant=0;variant<3;++variant) {
            const auto v=score(m,variant,false);assert(v==score(m,variant,true));
            std::printf("%u %u %016llx\n",m,variant,(unsigned long long)hash(v));
            if(argc>1 && (m==1 || m==9)) {
                char path[256];std::snprintf(path,sizeof(path),"%s/%u-%u.raw",argv[1],m,variant);
                auto f=std::fopen(path,"wb");assert(f);assert(std::fwrite(v.data(),sizeof(int16_t),v.size(),f)==v.size());assert(std::fclose(f)==0);
            }
        }
    }
#ifndef LEGACY_ONLY
    // Exact opt-in, legacy ID and partial extension flags retain Harmonic Lattice.
    for(unsigned flags=0;flags<7;++flags) {
        Exchange a,b;Engine ea,eb;std::array<int16_t,block> va{},vb{};
        for(unsigned k=0;k<40;++k) {
            a.publish_patch(2048,4095,true,false,Patch::HarmonicLattice);b.publish_patch(2048,4095,true,false,Patch::HarmonicLattice);
            b.control.fetch_or(flags<<29);ea.render(a,va.data(),block);eb.render(b,vb.data(),block);assert(va==vb);
        }
    }
    assert(score(9,1,false)!=score(9,2,false));assert(score(9,1,false)!=score(1,1,false));
    // Gain envelope and output bound over the legal frequency/morph domain.
    for(unsigned x=0;x<=4095;x+=15)for(unsigned y=0;y<=4095;y+=15) {
        const float f=110.f*std::exp2(3.f*float(x)/4095.f),t=float(y)/4095.f;
        const float a=formant_reed(x*1048576u,f,t,[](uint32_t){return 1.f;});
        assert(a>=0.f && a<1.f);
    }
    std::printf("PASS opt-in, spectral distinction, bounded gains, sizeof Engine=%zu\n",sizeof(Engine));
#endif
}
