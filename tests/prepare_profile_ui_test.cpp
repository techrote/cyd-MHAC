#include "mhac_visual_patch.hpp"
#include "mhac_prepare_profile.hpp"
#include <cassert>
#include <cstdio>
using namespace mhac;
using namespace mhac::visual;
int main() {
    unsigned controls=0, pcm=0;
    for(unsigned patch=0;patch<sound_count;++patch) {
        Exchange io, reference;
        Engine engine, baseline;
        PrepareProfiler meter;
        PrepareProfileMailbox mailbox;
        PrepareProfile p{};
        for(unsigned gate=0;gate<2;++gate) for(unsigned mute=0;mute<2;++mute) {
            publishControl(io,{1234,3456,bool(gate),bool(mute),patch});
            const auto word=io.control.load();
            // Same packed predicate as the common Arduino loop.
            assert(((word & (3u<<24))==(1u<<25))==(!gate && mute));
            ++controls;
        }
        for(unsigned i=0;i<256;++i) {
            const Control c{(i*97u)%4096,(i*137u)%4096,i<128,i>=128,patch};
            publishControl(io,c); publishControl(reference,c);
            int16_t a[block],b[block];
            engine.render(io,a,block); baseline.render(reference,b,block);
            // Synthetic timing only: exercise all bins without claiming target timings.
            const uint32_t elapsed[]={0,1334,2667,4001,5334};
            meter.record(0xfffffff0u,0xfffffff0u+elapsed[i%5],true,mailbox);
            assert(mailbox.read(p));
            assert(p.attempts==(i==255?256u:i>=127?128u:1u));
            uint32_t sum=0; for(auto n:p.bins) sum+=n;
            assert(sum==p.attempts && p.failed==0);
            for(unsigned j=0;j<block;++j) { assert(a[j]==b[j]); if(i==255) assert(a[j]==0); ++pcm; }
        }
        assert(p.bins[0]==52 && p.bins[1]==51 && p.bins[2]==51 && p.bins[3]==51 && p.bins[4]==51);
        assert(p.max_us==5334);
        meter.record(0,6000,false,mailbox);
        assert(mailbox.read(p) && p.attempts==257 && p.failed==1 && p.bins[4]==52 && p.max_us==6000);
    }
    std::printf("PASS %u gate/mute comparisons, %u exact PCM samples, ten synthetic profile sequences and terminal publications\n",controls,pcm);
}
