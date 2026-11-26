#include "mhac_gesture.hpp"
#include <cassert>
#include <cstdio>
using L=mhac::visual::GestureLoop;
int main(){
    L l; assert(!l.play(0).gate); l.toggle(); l.observe({1,2,false},8); assert(l.state()==L::State::Armed);
    l.observe({100,200,true},16); l.observe({150,250,true},24); l.observe({300,400,true},32);
    l.observe({500,600,true},48); l.observe({},64); assert(l.duration()==48 && l.count()==3);
    assert(l.play(64).x==100 && l.play(79).x==100 && l.play(80).x==300 && l.play(96).x==500);
    assert(!l.play(112).gate && !l.play(211).gate && l.play(212).x==100);
    l.stop(); assert(!l.play(213).gate); l.toggle(); l.observe({1,2,true},0); l.observe({},16); assert(l.state()==L::State::Idle);
    l.toggle(); const uint32_t start=0xfffffff0u; l.observe({65535,65535,true},start);
    for(uint32_t t=8;t<=8000;t+=8) l.observe({uint16_t(t),2,true},start+t);
    assert(l.state()==L::State::Recording && l.count()==251); l.observe({},start+8008);
    assert(l.duration()==4000 && l.play(start+8008).x==4095);
    assert(l.play(start+8008+3999).gate && !l.play(start+8008+4000).gate && l.play(start+8008+4100).gate);
    l.stop(); l.toggle(); l.observe({123,456,true},0xffffffd0u); l.observe({},0xfffffff0u);
    assert(l.play(0xfffffff0u).gate && !l.play(0x10u).gate && l.play(0x74u).gate);
    l.toggle(); assert(!l.play(0x75u).gate);
    for(unsigned i=0;i<10000;++i){ l.toggle(); l.observe({42,99,true},i*100u); l.observe({},i*100u+40); assert(l.play(i*100u+40).gate); l.stop(); }
    std::printf("gesture PASS: 251 point bound, wrap, rest, cancel, 10000 sessions; sizeof=%zu\n",sizeof(L));
}
