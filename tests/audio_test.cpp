#include "mhac_audio.hpp"
#include <cassert>
#include <cstdio>
#include <array>
int main(){
    mhac::Exchange io, other; mhac::Engine a,b;
    std::array<int16_t,mhac::block> x{},y{};
    a.render(io,x.data(),x.size()); for(auto v:x) assert(v==0);
    b.render(other,y.data(),y.size());
    unsigned nonzero=0; uint64_t hash=14695981039346656037ull;
    for(unsigned k=0;k<1000;++k){
        io.publish(k*29,k*17,true,false); other.publish(k*29,k*17,true,false);
        a.render(io,x.data(),x.size()); b.render(other,y.data(),y.size());
        assert(x==y); assert(io.snapshot()==other.snapshot());
        for(auto v:x){assert(v>=-8191 && v<=8191); nonzero+=v!=0; hash=(hash^uint16_t(v))*1099511628211ull;}
    }
    assert(nonzero>120000); io.publish(4095,4095,true,true);
    for(unsigned k=0;k<100;++k) a.render(io,x.data(),x.size());
    for(auto v:x) assert(v==0);
    std::printf("PASS: %u nonzero samples, FNV-1a over sample words %016llx\n",nonzero,(unsigned long long)hash);
}
