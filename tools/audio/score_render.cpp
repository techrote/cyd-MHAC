// Offline CYD gesture-score renderer; no serial, device, or audio driver access.
#include "mhac_audio.hpp"
#include "mhac_dac.hpp"
#include <array>
#include <charconv>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace mhac;
namespace {
struct Row { unsigned blocks,mode,x,y,state,curve; };
constexpr unsigned max_blocks=11250; // 60 seconds at 24 kHz / 128 samples
void require(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
std::vector<Row> read_score(std::istream& in) {
    std::vector<Row> rows; std::string line; unsigned total=0;
    while(std::getline(in,line)) {
        require(line.size()<=80,"score row too long");
        if(!line.empty() && line.back()=='\r') line.pop_back();
        if(line.empty() || line[0]=='#') continue;
        std::array<unsigned,6> v{}; const char* p=line.data(); const char* end=p+line.size();
        for(unsigned i=0;i<v.size();++i) {
            auto r=std::from_chars(p,end,v[i]);
            require(r.ec==std::errc{} && r.ptr!=p,"invalid integer field");
            p=r.ptr;
            if(i==4 && p==end) break; // Legacy five-field row: linear.
            if(i+1<v.size()) { require(p!=end && *p==',',"expected comma"); ++p; }
        }
        require(p==end,"trailing score data");
        require(v[0]>0 && v[0]<=max_blocks-total && v[1]<=counterquotation_session_index &&
                v[2]<=4095 && v[3]<=4095 && v[4]<=2 && v[5]<=2,"score field out of range");
        require(rows.size()<512,"too many score rows");
        total+=v[0]; rows.push_back({v[0],v[1],v[2],v[3],v[4],v[5]});
    }
    require(in.eof() && !in.bad(),"score read failure");
    require(rows.size()>=2 && rows.front().state==2 && rows.back().state==2 && rows.back().blocks>=48,
            "require initial mute and >=48 final muted blocks");
    return rows;
}
// Curves are evaluated at control-block boundaries, never in the DSP loop.
// 0: legacy linear; 1: immediate target; 2: integer cubic smoothstep.
unsigned position(unsigned a,unsigned b,unsigned k,unsigned n,unsigned curve) {
    if(curve==1) return b;
    if(curve==0) return unsigned(int(a)+(int(b)-int(a))*int(k)/int(n));
    const int64_t t=k,d=n;
    return unsigned(int64_t(a)+(int64_t(b)-a)*t*t*(3*d-2*t)/(d*d*d));
}
void curve_test() {
    for(unsigned n:{1u,2u,7u,128u,max_blocks}) for(unsigned curve=0;curve<3;++curve) {
        unsigned prev=0;
        for(unsigned k=1;k<=n;++k) {
            const auto up=position(0,4095,k,n,curve),down=position(4095,0,k,n,curve);
            require(up>=prev && up<=4095 && up+down==4095,"curve monotonicity/bounds"); prev=up;
        }
        require(prev==4095,"curve endpoint");
    }
    require(position(0,4095,1,8,1)==4095,"step onset");
    require(position(0,4095,1,8,2)<position(0,4095,1,8,0),"ease onset");
    require(position(0,4095,7,8,2)>position(0,4095,7,8,0),"ease ending");
}
constexpr char example[]=R"SCORE(# blocks,mode,x,y,state,curve (0 linear,1 step,2 smoothstep)
# Glass Meridian: resonant strikes, spectral arcs, remembered gestures.
32,6,1024,1800,2,1
24,6,1024,1800,1,1
48,6,1024,1800,0,1
24,6,2218,3400,1,1
72,6,2218,3400,0,1
128,1,1706,3900,1,2
128,1,3413,900,1,2
32,1,3413,900,0,1
96,7,1365,3900,1,2
96,7,2730,4095,1,2
96,8,683,3600,1,2
64,8,2048,4095,1,1
48,8,2048,4095,0,1
128,5,1365,3500,1,2
128,5,683,4095,1,2
48,5,683,4095,0,1
96,4,1365,4095,1,1
96,4,2048,1700,1,2
64,4,2048,0,2,1
)SCORE";
void send(Exchange& io,const Row& r,unsigned x,unsigned y) {
    const bool gate=r.state==1,mute=r.state==2;
    if(r.mode==counterquotation_session_index) io.publish_counterquotation(x,y,gate,mute);
    else if(r.mode==quotation_patch_index) io.publish_quotation(x,y,gate,mute);
    else if(r.mode==modal_patch_index) io.publish_modal(x,y,gate,mute);
    else io.publish_patch(x,y,gate,mute,static_cast<Patch>(r.mode));
}
std::vector<int16_t> render(const std::vector<Row>& rows,unsigned chunk) {
    Engine engine; Exchange io; std::array<int16_t,block> pcm{};
    std::vector<int16_t> out; unsigned x=rows.front().x,y=rows.front().y;
    for(const auto& r:rows) {
        for(unsigned k=1;k<=r.blocks;++k) {
            const auto ramp=[&](unsigned a,unsigned b) { return position(a,b,k,r.blocks,r.curve); };
            send(io,r,ramp(x,r.x),ramp(y,r.y));
            for(unsigned at=0;at<block;) {
                const unsigned n=std::min(chunk,block-at); engine.render(io,pcm.data()+at,n); at+=n;
            }
            for(auto v:pcm) require(std::abs(int(v))<=8191,"PCM bound exceeded");
            out.insert(out.end(),pcm.begin(),pcm.end());
        }
        x=r.x; y=r.y;
    }
    require(io.watchdog_trips.load()==0,"unexpected watchdog");
    require(std::all_of(out.begin(),out.begin()+rows.front().blocks*block,[](int16_t v){return v==0;}),"initial mute failure");
    require(std::all_of(out.end()-block,out.end(),[](int16_t v){return v==0;}),"final mute failure");
    return out;
}
void put(std::ostream& s,uint32_t v,unsigned n) { for(unsigned i=0;i<n;++i) s.put(char((v>>(8*i))&255u)); }
void wav(const std::string& path,const std::vector<int16_t>& pcm) {
    std::ofstream s(path,std::ios::binary); require(bool(s),"cannot open WAV");
    const auto bytes=uint32_t(2*pcm.size());
    s.write("RIFF",4); put(s,36+bytes,4); s.write("WAVEfmt ",8); put(s,16,4); put(s,1,2); put(s,1,2);
    put(s,rate,4); put(s,rate*2,4); put(s,2,2); put(s,16,2); s.write("data",4); put(s,bytes,4);
    for(auto v:pcm) put(s,uint16_t(v),2);
    s.close(); require(bool(s),"WAV write failed");
}
uint64_t hash(const std::vector<int16_t>& pcm) { uint64_t h=14695981039346656037ull; for(auto v:pcm) h=(h^uint16_t(v))*1099511628211ull; return h; }
}
int main(int argc,char** argv) {
    try {
        curve_test();
        if(argc==2 && std::string(argv[1])=="--example") { std::cout<<example; return 0; }
        require(argc==2 || argc==3,"usage: score_render score.csv [output-prefix] | --example");
        std::ifstream in(argv[1],std::ios::binary|std::ios::ate);
        require(bool(in) && in.tellg()>=0 && in.tellg()<=65536,"missing or oversized score"); in.seekg(0);
        const auto rows=read_score(in); auto pcm=render(rows,block);
        require(pcm==render(rows,31) && pcm==render(rows,97),"partition mismatch");
        DacQuantizer quantizer; std::vector<int16_t> dac; dac.reserve(pcm.size()); unsigned peak=0,active=0;
        for(auto v:pcm) { peak=std::max(peak,unsigned(std::abs(int(v)))); active+=v!=0; dac.push_back(int16_t(int(quantizer.pack(v))-32768)); }
        require(std::all_of(dac.end()-block,dac.end(),[](int16_t v){return v==0;}),"DAC mute failure");
        std::cout<<"rows="<<rows.size()<<" samples="<<pcm.size()<<" peak="<<peak<<" active="<<active<<" pcm_hash="<<std::hex<<hash(pcm)<<" dac_hash="<<hash(dac)<<'\n';
        if(argc==3) { wav(std::string(argv[2])+"-PCM.wav",pcm); wav(std::string(argv[2])+"-DAC.wav",dac); }
    } catch(const std::exception& e) { std::cerr<<"FAIL: "<<e.what()<<'\n'; return 1; }
}
