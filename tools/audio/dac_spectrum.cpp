// Offline digital DAC error audit; never opens a serial port or device.
#include "mhac_dac.hpp"
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace {
constexpr unsigned N=16384, rate=24000;
constexpr double pi=3.14159265358979323846;
using Samples=std::vector<int16_t>;
void require(bool ok,const char* msg) { if(!ok) { std::cerr<<msg<<'\n'; std::exit(1); } }
void fft(std::vector<std::complex<double>>& a) {
    for(unsigned i=1,j=0;i<N;++i) {
        unsigned bit=N>>1; for(;j&bit;bit>>=1) j^=bit; j^=bit;
        if(i<j) std::swap(a[i],a[j]);
    }
    for(unsigned len=2;len<=N;len*=2) {
        const auto w=std::polar(1.,-2*pi/len);
        for(unsigned i=0;i<N;i+=len) {
            std::complex<double> u=1.;
            for(unsigned j=0;j<len/2;++j) {
                const auto p=a[i+j],q=a[i+j+len/2]*u;
                a[i+j]=p+q; a[i+j+len/2]=p-q; u*=w;
            }
        }
    }
}
struct Metrics { double dc,rms,low,peak_hz,peak_rms; };
Metrics measure(const Samples& src,const Samples& dst) {
    std::vector<std::complex<double>> a(N); double sum=0,sq=0;
    for(unsigned i=0;i<N;++i) { const double e=int(dst[i])-int(src[i]); a[i]=e; sum+=e; sq+=e*e; }
    fft(a); double total=0,low=0,peak=-1; unsigned bin=0;
    for(unsigned k=0;k<=N/2;++k) {
        const double p=std::norm(a[k])/(double(N)*N)*(k==0||k==N/2?1.:2.);
        total+=p;
        if(k<=N/8) low+=p; // Includes DC, through exactly 3 kHz.
        if(k && p>peak) { peak=p; bin=k; }
    }
    require(std::abs(total-sq/N)<1e-6*std::max(1.,sq/N),"Parseval mismatch");
    return {sum/N,std::sqrt(sq/N),std::sqrt(low),peak>1e-18?double(bin)*rate/N:0.,std::sqrt(peak)};
}
void put(std::ostream& s,uint32_t v,unsigned n) { for(unsigned i=0;i<n;++i) s.put(char((v>>(8*i))&255u)); }
void wav(const std::string& path,const Samples& data) {
    std::ofstream s(path,std::ios::binary); const uint32_t bytes=uint32_t(2*data.size());
    s.write("RIFF",4); put(s,36+bytes,4); s.write("WAVEfmt ",8); put(s,16,4); put(s,1,2); put(s,1,2);
    put(s,rate,4); put(s,2*rate,4); put(s,2,2); put(s,16,2); s.write("data",4); put(s,bytes,4);
    for(auto v:data) put(s,uint16_t(v),2);
    s.close(); require(bool(s),"WAV write failure");
}
void append(Samples& out,const Samples& in) { out.insert(out.end(),in.begin(),in.end()); out.insert(out.end(),6000,0); }
}
int main(int argc,char** argv) {
    require(argc<=2,"usage: dac_spectrum [output-prefix]");
    Samples ref_wav,floor_wav,shaped_wav;
    std::cout<<"stimulus,packer,dc_pcm,rms_pcm,dc_to_3khz_rms_pcm,largest_ac_error_hz,largest_ac_error_rms_pcm\n"<<std::fixed<<std::setprecision(6);
    for(unsigned c=0;c<6;++c) {
        const char* names[]={"dc+1","dc-1","dc+64","quiet375","mid750","loud1500"};
        Samples src(N),floor(N),shaped(N); mhac::DacQuantizer q; int64_t balance=0;
        for(unsigned i=0;i<N;++i) {
            const int v=c==0?1:c==1?-1:c==2?64:int(std::lround((c==3?64.:c==4?512.:4096.)*std::sin(2*pi*(i%(c==3?64u:c==4?32u:16u))/(c==3?64.:c==4?32.:16.))));
            src[i]=int16_t(v); floor[i]=int16_t(int(mhac::dac_floor(src[i]))-32768); shaped[i]=int16_t(int(q.pack(src[i]))-32768);
            if(v==0) { require(shaped[i]==0 && q.residual()==0,"zero reset"); balance=0; }
            else { balance+=v-shaped[i]; require(balance==q.residual(),"unsaturated residual conservation"); }
            require(std::abs(int(shaped[i])-v)<=255,"shaped error bound");
        }
        const auto f=measure(src,floor),s=measure(src,shaped);
        require(std::abs(s.dc)<=std::abs(f.dc)+1e-9,"unexpected DC regression");
        if(c<3) require(s.dc==0.,"coherent constant DC residual");
        if(c<2) require(std::abs(s.low-8.)<1e-6,"unit DC idle comb changed");
        if(c==2) require(s.low<1e-6,"quarter-code DC low-band error");
        if(c>=3) require(s.low<f.low*.12,"tone low-band error regression");
        for(const auto& m:{f,s}) require(std::isfinite(m.rms)&&m.low<=m.rms+1e-9,"invalid spectrum");
        auto print=[&](const char* packer,Metrics m) { std::cout<<names[c]<<','<<packer<<','<<m.dc<<','<<m.rms<<','<<m.low<<','<<m.peak_hz<<','<<m.peak_rms<<'\n'; };
        print("floor",f); print("feedback",s);
        // Only AC stimuli in the audition WAV; no normalization or DC playback.
        if(c>=3) { append(ref_wav,src); append(floor_wav,floor); append(shaped_wav,shaped); }
    }
    if(argc==2) { const std::string p=argv[1]; wav(p+"-reference.wav",ref_wav); wav(p+"-floor.wav",floor_wav); wav(p+"-feedback.wav",shaped_wav); }
}
