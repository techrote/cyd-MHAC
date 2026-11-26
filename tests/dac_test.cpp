#include "mhac_dac.hpp"
#include "mhac_audio.hpp"
#include "mhac_performance_presets.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cstdlib>

static int pcm(uint16_t slot) { return (int(slot>>8)-128)*256; }
int main() {
    // Every reachable residual crossed with every signed 16-bit sample.
    for(int r=-128;r<=127;++r) for(int s=-32768;s<=32767;++s) {
        mhac::DacQuantizer q;
        assert(pcm(q.pack(int16_t(r)))==0 && q.residual()==r);
        const uint16_t slot=q.pack(int16_t(s));
        const int out=pcm(slot), e=s+r-out;
        assert((slot&255u)==0 && out>=-32768 && out<=32512);
        assert(pcm(mhac::dac_floor(int16_t(s)))==s-((s+32768)%256));
        if(s==0) assert(out==0 && q.residual()==0);
        else {
            assert(q.residual()==(e>127?127:e< -128? -128:e));
            if(s+r>=-32768 && s+r<=32512) assert(e==q.residual());
        }
    }
    for(int sign : {-1,1}) {
        mhac::DacQuantizer q; int sum=0;
        for(int i=0;i<65536;++i) sum+=pcm(q.pack(int16_t(sign)));
        assert(sum==sign*65536 && q.residual()==0);
        q.pack(127); assert(q.pack(0)==0x8000 && q.residual()==0);
    }
    // Actual engine PCM, all 16 factory presets, fixed touch/release/mute score.
    for(unsigned p=0;p<16;++p) {
        const auto preset=p<12?mhac::factory_presets[p]:mhac::modal_presets[p-12];
        mhac::Engine engine; mhac::Exchange io; mhac::DacQuantizer a,b;
        std::array<int16_t,mhac::block> samples{};
        std::array<uint16_t,mhac::block> packed{};
        unsigned nonzero=0; int64_t floor_bias=0,feedback_bias=0;
        for(unsigned k=0;k<750;++k) {
            const bool gate=k<8 || (k>=30 && k<38) || (k>=64 && k<72);
            const bool mute=k>=300;
            const auto s=preset.sound;
            if(s.patch==6) io.publish_modal(s.x,s.y,gate,mute);
            else io.publish_patch(s.x,s.y,gate,mute,static_cast<mhac::Patch>(s.patch));
            engine.render(io,samples.data(),samples.size());
            for(unsigned i=0;i<samples.size();++i) packed[i]=a.pack(samples[i]);
            // Independent grouping, state retained across chunks and DMA boundaries.
            for(unsigned base=0;base<samples.size();base+=17)
                for(unsigned i=base;i<samples.size() && i<base+17;++i) {
                    const int16_t s16=samples[i]; const auto w=b.pack(s16);
                    assert(w==packed[i]);
                    const int y=pcm(w);
                    assert(std::abs(y-int(s16))<=255);
                    if(s16==0) assert(y==0);
                    nonzero+=y!=0;
                    floor_bias+=pcm(mhac::dac_floor(s16))-s16;
                    feedback_bias+=y-s16;
                    if(k>=749) assert(y==0);
                }
        }
        assert(nonzero>0 && std::llabs(feedback_bias)<std::llabs(floor_bias));
        std::printf("%s: floor_error_sum=%lld feedback_error_sum=%lld\n",preset.name,(long long)floor_bias,(long long)feedback_bias);
    }
    std::puts("DAC PASS: 16777216 state/sample transitions, 1536000 engine samples, partition and silence checks");
}
