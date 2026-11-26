#include "mhac_audio.hpp"
#include <cassert>
#include <cstdio>
#include <array>
#include <thread>
#include <vector>
using namespace mhac;
static void watchdog() {
    Exchange io; Engine a;
    std::array<int16_t,block> x{};
    auto render=[&](unsigned n){a.render(io,x.data(),n);};
    auto silent=[&](){for(auto v:x) assert(v==0);};
    io.publish(1800,3000,true,false);
    for(unsigned k=0;k<200;++k){io.publish(1800,3000,true,false);render(block);}
    assert(io.watchdog_trips.load()==0 && (io.snapshot()&4095u)>0);
    // Age is 128 samples since most recent observed publication.
    for(unsigned k=0;k<92;++k) render(block);
    assert(io.watchdog_trips.load()==0);
    render(block); assert(io.watchdog_trips.load()==1);
    for(unsigned k=0;k<12;++k) render(block);
    silent();
    // A recovered UI publishing the same held gate cannot restart audio.
    for(unsigned k=0;k<150;++k){io.publish(1800,3000,true,false);render(block);silent();}
    assert(io.watchdog_trips.load()==1);
    io.publish(1800,3000,false,false);render(block);silent();
    io.publish(1800,3000,true,false);render(block);assert((io.snapshot()&4095u)>0);
    // A gate with no renewed publication expires even over a single large render.
    Exchange p,q; Engine e,f;
    p.publish(4095,4095,true,false);q.publish(4095,4095,true,false);
    std::vector<int16_t> u(control_lease_samples+2048),v(u.size());
    e.render(p,u.data(),u.size());
    for(unsigned i=0;i<v.size();){unsigned n=std::min<unsigned>(1+(i%251),v.size()-i);f.render(q,v.data()+i,n);i+=n;}
    assert(u==v);assert(p.watchdog_trips.load()==1 && q.watchdog_trips.load()==1);
    for(unsigned i=control_lease_samples+1200;i<u.size();++i) assert(u[i]==0);
    // Unsigned heartbeat rollover is a normal fresh observation.
    Exchange w; Engine g; w.heartbeat.store(UINT32_MAX-3);
    w.publish(1,2,true,false);g.render(w,x.data(),block);
    w.publish(1,2,true,false);g.render(w,x.data(),block);
    assert(w.heartbeat.load()==0 && w.watchdog_trips.load()==0 && (w.snapshot()&4095u)>0);
    // A paused writer cannot make a partially published gate audible or renew the lease.
    Exchange j; Engine h;
    j.heartbeat.store(1);j.control.store((1u<<24)|2048u);
    h.render(j,x.data(),block);silent();
    j.heartbeat.store(2);h.render(j,x.data(),block);assert((j.snapshot()&4095u)>0);
    j.heartbeat.store(3); // producer stalled before publishing completion
    for(unsigned k=0;k<110;++k) h.render(j,x.data(),block);
    silent();assert(j.watchdog_trips.load()==1);
    std::puts("PASS: independent gate lease, frozen UI, re-arm, partition equality, heartbeat rollover, interrupted publication");
}
static void concurrent() {
    Exchange io; Engine a; std::atomic<bool> done{false};
    std::thread producer([&]{for(unsigned k=0;k<200000;++k) io.publish(k&4095u,(k*7)&4095u,(k&16u)!=0,(k&64u)!=0);io.publish(0,0,false,true);done.store(true,std::memory_order_release);});
    std::array<int16_t,block> x{};
    unsigned frames=0;
    do { a.render(io,x.data(),block);for(auto s:x) assert(s>=-8191 && s<=8191);++frames; } while(!done.load(std::memory_order_acquire));
    producer.join();
    for(unsigned k=0;k<100;++k) a.render(io,x.data(),block);
    for(auto s:x) assert(s==0);
    std::printf("PASS: concurrent 200000 publications, %u audio blocks, final silence\n",frames);
}
int main(){
    Exchange io, other; Engine a,b;
    std::array<int16_t,block> x{},y{};
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
    watchdog(); concurrent();
}
