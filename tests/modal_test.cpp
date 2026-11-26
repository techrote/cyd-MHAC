#include "mhac_audio.hpp"
#include <cassert>
#include <cstdio>
#include <array>
#include <vector>
#include <algorithm>
using namespace mhac;
static std::vector<int16_t> render(unsigned x,unsigned y,bool split,bool retrigger) {
    Exchange io; Engine e; std::vector<int16_t> v(rate*3);
    for(unsigned k=0;k<v.size();k+=block) {
        const unsigned n=std::min<unsigned>(block,v.size()-k);
        const bool gate=k<rate*2 && (!retrigger || k<rate/2 || k>=rate);
        io.publish_modal(x,y,gate,false);
        if(split) { unsigned a=n/3; e.render(io,v.data()+k,a); e.render(io,v.data()+k+a,n-a); }
        else e.render(io,v.data()+k,n);
    }
    for(auto s:v) assert(s>=-8191 && s<=8191);
    for(unsigned i=rate*2+6000;i<v.size();++i) assert(v[i]==0);
    assert(io.watchdog_trips.load()==0);
    return v;
}
static uint64_t energy(const std::vector<int16_t>& v,unsigned a,unsigned b) {
    uint64_t sum=0; for(unsigned i=a;i<b;++i) sum+=int64_t(v[i])*v[i]; return sum;
}
// Direct voice tests isolate the modal bank from the existing global gate envelope.
static void overlap_test() {
    auto sine=[](uint32_t p) { return std::sin(float(p>>16)*6.28318530718f/65536.f); };
    constexpr uint32_t lo=19685267u, hi=78405362u;
    ModalChime both, tail, fresh;
    for(unsigned i=0;i<128;++i) {
        assert(both.sample(lo,.2f,true,sine)==tail.sample(lo,.2f,true,sine));
    }
    for(unsigned i=0;i<32;++i) {
        assert(both.sample(lo,.2f,false,sine)==tail.sample(lo,.2f,false,sine));
    }
    double tail_energy=0, new_energy=0;
    for(unsigned i=0;i<1024;++i) {
        float a=both.sample(hi,.8f,true,sine);
        // Released voice ignores incoming pitch/brightness.
        float b=tail.sample(0,.99f,false,sine);
        float c=fresh.sample(hi,.8f,true,sine);
        assert(std::fabs(a-b-c)<2e-7f);
        assert(std::fabs(a)<=1.f);
        tail_energy+=b*b; new_energy+=c*c;
    }
    assert(tail_energy>1. && new_energy>1.);
    // Constant phase probe isolates amplitude continuity from oscillator motion.
    auto unity=[](uint32_t) { return 1.f; };
    ModalChime steal; float last=0.f;
    for(unsigned i=0;i<12000;++i) {
        bool gate=i%129u!=128u;
        float v=steal.sample(hi,.4f,gate,unity);
        assert(v>=0.f && v<=1.f);
        assert(std::fabs(v-last)<=.061f); last=v;
    }
    // Inhibit while input stays held, then verify silence and fresh rearm.
    for(unsigned i=0;i<1024;++i) last=steal.sample(hi,.4f,true,unity,true);
    assert(last==0.f);
    ModalChime clean;
    for(unsigned i=0;i<256;++i)
        assert(steal.sample(lo,.2f,true,sine)==clean.sample(lo,.2f,true,sine));
    std::printf("PASS two-voice pitch lock/additivity (1024 samples), 12000 stealing continuity samples, inhibit/rearm; tail energy %.3f new %.3f\n",tail_energy,new_energy);
}
int main(int argc,char** argv) {
    overlap_test();
    unsigned count=0;
    for(unsigned x:{0u,2048u,4095u}) for(unsigned y:{0u,2048u,4095u}) {
        auto v=render(x,y,false,false); assert(v==render(x,y,true,false));
        assert(energy(v,512,4096)>0);
        assert(energy(v,rate,rate+3584)<energy(v,512,4096)/10);
        auto r=render(x,y,false,true); assert(r==render(x,y,true,true));
        assert(energy(r,rate+512,rate+4096)>energy(v,rate+512,rate+4096)*10);
        if(argc>1) {
            char name[256]; std::snprintf(name,sizeof(name),"%s/modal-%u-%u.raw",argv[1],x,y);
            auto f=std::fopen(name,"wb"); assert(f); assert(std::fwrite(r.data(),sizeof(int16_t),r.size(),f)==r.size()); assert(std::fclose(f)==0);
        }
        ++count;
    }
    assert(render(2048,0,false,false)!=render(2048,4095,false,false));
    Exchange io; Engine e; std::array<int16_t,block> a{};
    for(unsigned k=0;k<20;++k) { io.publish_modal(2048,4095,true,false); e.render(io,a.data(),block); }
    for(unsigned k=0;k<110;++k) e.render(io,a.data(),block);
    assert(io.watchdog_trips.load()==1); for(auto s:a) assert(s==0);
    io.publish_modal(2048,4095,true,false); e.render(io,a.data(),block); for(auto s:a) assert(s==0);
    io.publish_modal(2048,4095,false,false); e.render(io,a.data(),block);
    io.publish_modal(2048,4095,true,false); e.render(io,a.data(),block);
    assert(std::any_of(a.begin(),a.end(),[](int16_t s){return s!=0;}));
    for(unsigned k=0;k<3000;++k) {
        if(k%7==6) io.publish_modal(k&4095u,(k*31)&4095u,true,false);
        else io.publish_patch(k&4095u,(k*31)&4095u,true,false,static_cast<Patch>(k%6));
        e.render(io,a.data(),block); for(auto s:a) assert(s>=-8191 && s<=8191);
    }
    Exchange bad,good; Engine be,ge; std::array<int16_t,block> b{};
    bad.publish_patch(0,0,true,false,static_cast<Patch>(6)); good.publish(0,0,true,false);
    assert(bad.control.load()==good.control.load());
    bad.control.fetch_or(6u<<26); be.render(bad,a.data(),block); ge.render(good,b.data(),block); assert(a==b);
    std::printf("PASS %u pitch/brightness pairs, decay, restrike, exact split invariance, 3000 switches, watchdog and v2 fallback; Engine=%zu ModalChime=%zu\n",count,sizeof(Engine),sizeof(ModalChime));
}
