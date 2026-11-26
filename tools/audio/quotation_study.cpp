// AUDIO-022: deterministic quotation engine and audition, C++17, no board required.
#include "mhac_audio.hpp"
#include "mhac_dac.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
using namespace mhac;
static void check(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
static float sine(uint32_t p) { const float t=float(p>>16)*(1.f/32768.f)-1.f; const float y=4.f*t*(1.f-std::fabs(t)); return y+.225f*(y*std::fabs(y)-y); }
static uint64_t hash(const std::vector<int16_t>& a) { uint64_t h=14695981039346656037ull; for(auto v:a) { h^=uint16_t(v); h*=1099511628211ull; } return h; }
static void put(std::ostream& s,uint32_t v,unsigned n) { for(unsigned i=0;i<n;++i) s.put(char((v>>(8*i))&255u)); }
static void wav(const char* path,const std::vector<int16_t>& a) { std::ofstream s(path,std::ios::binary); s.write("RIFF",4); put(s,36+2*a.size(),4); s.write("WAVEfmt ",8); put(s,16,4); put(s,1,2); put(s,1,2); put(s,rate,4); put(s,rate*2,4); put(s,2,2); put(s,16,2); s.write("data",4); put(s,2*a.size(),4); for(auto v:a) put(s,uint16_t(v),2); check(bool(s),"WAV write"); }
static void raw(Exchange& io,unsigned x,unsigned y,bool gate,bool mute,uint32_t flags) {
    io.heartbeat.fetch_add(1); io.control.store(x|(y<<12)|(uint32_t(gate)<<24)|(uint32_t(mute)<<25)|(7u<<26)|flags); io.heartbeat.fetch_add(1);
}
static std::vector<int16_t> perform(unsigned chunk,unsigned mode,unsigned depth) {
    Engine engine; Exchange io; std::vector<int16_t> out(6*rate);
    for(unsigned off=0;off<out.size();) {
        const unsigned t=off/block; const bool gate=t>=16 && t<940 && !(t>=600 && t<700), mute=t<16 || t>=900;
        const unsigned notes[]={0,1365,796,1934,1138,2730,1593,455};
        const unsigned x=notes[(t/47)%8], y=depth;
        if(mode==0) io.publish_quotation(x,y,gate,mute);
        else if(mode==1) io.publish(x,y,gate,mute);
        else if(mode==4) io.publish_counterquotation(x,y,gate,mute);
        else raw(io,x,y,gate,mute,mode==2?0u:mode==5?counterquotation_opt_in:mode==6?counterquotation_opt_in|quotation_opt_in|modal_opt_in:quotation_opt_in|modal_opt_in);
        const unsigned n=std::min<unsigned>(block,out.size()-off);
        for(unsigned i=0;i<n;) { const auto k=std::min(chunk,n-i); engine.render(io,out.data()+off+i,k); i+=k; }
        off+=n;
    }
    check(io.watchdog_trips.load()==0,"unexpected lease expiry");
    for(auto v:out) check(std::abs(int(v))<=8191,"PCM amplitude");
    for(unsigned i=rate*5;i<out.size();++i) check(out[i]==0,"post-mute silence");
    return out;
}
int main(int argc,char** argv) {
    // Before the first recalled mark, the quotation is exactly dry. Thereafter
    // a constant test carrier of zero exposes the independently clocked voice.
    Quotation q; uint32_t phase=0; unsigned active=0;
    for(unsigned i=0;i<2*Quotation::history_samples;++i) {
        const uint32_t step=20000000u+(i/Quotation::interval)*4000000u; phase+=step;
        const float a=q.sample(phase,step,1.f,0.f,true,false,sine);
        check(std::abs(a)<=1.f,"quotation bound");
        if(i<Quotation::history_samples) check(a==0.f,"premature quotation"); else active+=a!=0.f;
    }
    check(active>11000,"no recalled voice");
    // Safety invalidate; renewed capture must wait a complete new history.
    for(unsigned i=0;i<3000;++i) q.sample(phase,20000000u,1.f,0.f,false,true,sine);
    for(unsigned i=0;i<Quotation::history_samples;++i) check(q.sample(phase,20000000u,1.f,0.f,true,false,sine)==0.f,"stale history after inhibit");
    // An inhibited tail can still have nonzero blend on a rapid rearm. Fresh
    // history must not jump the fading oscillator to the live carrier phase.
    unsigned retriggers=0;
    for(unsigned gap:{1u,8u,32u,128u,512u,3000u}) {
        for(uint32_t offset:{0u,0x40000000u,0x80000000u,0xc0000000u}) {
            Quotation tail; uint32_t p=0; constexpr uint32_t s=20000000u;
            for(unsigned i=0;i<2*Quotation::history_samples;++i) {
                p+=s; tail.sample(p,s,1.f,0.f,true,false,sine);
            }
            for(unsigned i=0;i<gap;++i) { p+=s; tail.sample(p,s,1.f,0.f,false,true,sine); }
            auto reference=tail;
            p+=s;
            const float expected=reference.sample(p,s,1.f,0.f,false,false,sine);
            const float actual=tail.sample(p+offset,s,1.f,0.f,true,false,sine);
            check(actual==expected,"rapid rearm reset audible fading phase");
            for(unsigned i=1;i<Quotation::history_samples;++i) {
                p+=s;
                const float v=tail.sample(p+offset,s,1.f,0.f,true,false,sine);
                check(std::abs(v)<=1.f,"rapid rearm bound");
                if(i>=3000u) check(v==0.f,"rapid rearm reused invalidated history");
            }
            p+=s;
            unsigned heard=0;
            for(unsigned i=0;i<1000;++i) { heard+=tail.sample(p+offset,s,1.f,0.f,true,false,sine)!=0.f; p+=s; }
            check(heard>900,"rapid rearm failed fresh recall");
            ++retriggers;
        }
    }
    std::cout<<"rapid_rearm_profiles="<<retriggers<<'\n';
    // Observe actual recalled oscillator increments after each mark settles.
    unsigned pitch_checks=0;
    for(uint32_t s:{20000000u,50000000u,150000000u}) {
        for(bool counter:{false,true}) {
            Quotation voice; uint32_t p=0, last_phase=0, increment=0;
            unsigned calls=0;
            auto probe=[&](uint32_t v) { if(calls++==0) { increment=v-last_phase; last_phase=v; } return sine(v); };
            for(unsigned i=0;i<3*Quotation::history_samples;++i) {
                p+=s; calls=0;
                const float value=voice.sample(p,s,1.f,0.f,true,false,probe,counter);
                check(std::abs(value)<=1.f,"counterquotation bound");
                if(i<Quotation::history_samples) check(calls==0,"counterquotation premature recall");
                if(i>=Quotation::history_samples && i%Quotation::interval==Quotation::interval-1) {
                    const unsigned mark=(i-Quotation::history_samples)/Quotation::interval;
                    const uint32_t expected=counter?((mark&1u)?s+s/2u:s/2u):s;
                    check(calls==2 && std::abs(int64_t(increment)-expected)<=63,"recalled interval"); ++pitch_checks;
                }
            }
        }
    }
    auto dry=perform(128,0,0), half=perform(128,0,2048), wet=perform(128,0,4095);
    for(unsigned y:{0u,2048u,4095u}) {
        auto a=perform(128,0,y); check(a==perform(31,0,y) && a==perform(97,0,y),"partition");
    }
    check(dry==perform(128,1,0),"dry must be legacy carrier");
    check(half!=wet && dry!=wet,"depth contrast");
    auto legacy=perform(128,1,4095); check(legacy==perform(128,2,4095) && legacy==perform(128,3,4095),"legacy opt-in fallback");
    auto counter=perform(128,4,4095);
    check(counter!=wet && counter==perform(31,4,4095) && counter==perform(97,4,4095),"counterquotation contrast/partition");
    check(dry==perform(128,4,0),"counterquotation dry");
    check(legacy==perform(128,5,4095) && legacy==perform(128,6,4095),"v5 flag fallback");
    std::cout<<"counterquotation_pitch_checks="<<pitch_checks<<" hash="<<std::hex<<hash(counter)<<std::dec<<'\n';
    // Stop producer while active, renew the same stale gate, then release/rearm.
    Engine e; Exchange io; std::array<int16_t,block> b{};
    for(unsigned k=0;k<120;++k) { io.publish_quotation(k*31%4096,4095,true,false); e.render(io,b.data(),block); }
    for(unsigned k=0;k<150;++k) e.render(io,b.data(),block);
    check(io.watchdog_trips.load()==1,"lease trip"); for(auto v:b) check(v==0,"lease silence");
    io.publish_quotation(1000,4095,true,false); e.render(io,b.data(),block); for(auto v:b) check(v==0,"stale renewal rearmed");
    io.publish_quotation(1000,4095,false,false); e.render(io,b.data(),block); io.publish_quotation(1000,4095,true,false); e.render(io,b.data(),block);
    check(std::any_of(b.begin(),b.end(),[](int16_t v){return v!=0;}),"fresh rearm failed");
    std::cout<<"history="<<Quotation::history_samples<<" active_recall="<<active<<" engine_bytes="<<sizeof(Engine)<<" hashes="<<std::hex<<hash(dry)<<','<<hash(half)<<','<<hash(wet)<<std::dec<<'\n';
    if(argc==4 || argc==5) { wav(argv[1],dry); wav(argv[2],half); wav(argv[3],wet); if(argc==5) wav(argv[4],counter); }
}
