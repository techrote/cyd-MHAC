// CYD-AUDIO-019: offline performance fixtures, never a physical DAC recording.
#include "mhac_audio.hpp"
#include "mhac_dac.hpp"
#include "mhac_performance_presets.hpp"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
using namespace mhac;
constexpr unsigned ticks=1125; // 6 seconds at 128 samples / 24 kHz
void check(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
void put(std::ostream& s,uint32_t v,unsigned n) {
    for(unsigned i=0;i<n;++i) s.put(char((v>>(8*i))&255u));
}
void wav(const std::string& path,const std::vector<int16_t>& pcm) {
    std::ofstream s(path,std::ios::binary);
    s.write("RIFF",4); put(s,36+2*pcm.size(),4); s.write("WAVEfmt ",8);
    put(s,16,4); put(s,1,2); put(s,1,2); put(s,rate,4); put(s,rate*2,4);
    put(s,2,2); put(s,16,2); s.write("data",4); put(s,2*pcm.size(),4);
    for(auto v:pcm) put(s,uint16_t(v),2);
    check(bool(s),"WAV write failed");
}
std::vector<int16_t> perform(SoundPreset p,unsigned chunk) {
    Engine engine; Exchange io;
    std::vector<int16_t> result(ticks*block);
    for(unsigned t=0;t<ticks;++t) {
        // Three strikes, a moving gesture, silence, then a held-contact mute.
        const bool gate=(t>=32 && t<112)||(t>=160 && t<240)||
            (t>=272 && t<352)||(t>=400 && t<680)||(t>=800 && t<940);
        const bool mute=t<32 || t>=880;
        int x=p.x,y=p.y;
        if(t>=160 && t<240) x+=683;
        if(t>=272 && t<352) x+=1365;
        if(t>=400 && t<680) { x+=int(t-400)*1365/279; y+=int(t-400)*2048/279; }
        if(t>=800 && t<940) y-=1024;
        x=std::clamp(x,0,4095); y=std::clamp(y,0,4095);
        if(p.patch==modal_patch_index) io.publish_modal(x,y,gate,mute);
        else io.publish_patch(x,y,gate,mute,static_cast<Patch>(p.patch));
        for(unsigned k=0;k<block;) { unsigned n=std::min(chunk,block-k);
            engine.render(io,result.data()+t*block+k,n); k+=n; }
    }
    check(io.watchdog_trips.load()==0,"unexpected watchdog");
    return result;
}
uint64_t hash(const std::vector<int16_t>& a) {
    uint64_t h=14695981039346656037ull;
    for(auto v:a) { h^=uint16_t(v); h*=1099511628211ull; } return h;
}
void audition(const NamedPreset& p,unsigned id,const char* dir) {
    auto pcm=perform(p.sound,block);
    check(pcm==perform(p.sound,31) && pcm==perform(p.sound,97),"partition mismatch");
    check(std::all_of(pcm.begin(),pcm.begin()+32*block,[](int16_t v){return v==0;}),"boot not silent");
    check(std::all_of(pcm.begin()+980*block,pcm.end(),[](int16_t v){return v==0;}),"mute not silent");
    DacQuantizer dac; std::vector<int16_t> converted,legacy;
    converted.reserve(pcm.size()); legacy.reserve(pcm.size());
    unsigned peak=0,active=0; int64_t error=0, floor_error=0; uint64_t square=0, floor_square=0;
    for(auto v:pcm) {
        peak=std::max(peak,unsigned(std::abs(int(v)))); active+=v!=0;
        const auto q=int(dac.pack(v))-32768;
        converted.push_back(int16_t(q)); const int e=q-int(v); error+=e; square+=uint64_t(e*e);
        const int f=int(dac_floor(v))-32768; legacy.push_back(int16_t(f));
        const int d=f-int(v); floor_error+=d; floor_square+=uint64_t(d*d);
    }
    check(active>1000 && peak<=8191,"unexpected PCM range/activity");
    check(std::all_of(converted.begin()+980*block,converted.end(),[](int16_t v){return v==0;}),"DAC not silent");
    std::cout<<id<<','<<p.name<<','<<peak<<','<<active<<','<<error<<','<<square<<','<<floor_error<<','<<floor_square<<','<<std::hex<<hash(pcm)<<','<<hash(converted)<<','<<hash(legacy)<<std::dec<<'\n';
    if(dir) { const std::string base=std::string(dir)+"/"+std::to_string(id)+"-"+p.name;
        wav(base+"-PCM.wav",pcm); wav(base+"-DAC.wav",converted); wav(base+"-floor.wav",legacy); }
}
}
int main(int argc,char** argv) {
    check(argc<=2,"usage: gesture_audition [existing-output-directory]");
    std::cout<<"id,preset,peak,nonzero,quant_error_sum,quant_error_squared_sum,floor_error_sum,floor_error_squared_sum,pcm_hash,dac_hash,floor_hash\n";
    unsigned id=0;
    for(const auto& p:mhac::factory_presets) audition(p,id++,argc==2?argv[1]:nullptr);
    for(const auto& p:mhac::modal_presets) audition(p,id++,argc==2?argv[1]:nullptr);
}
