#include "mhac_performance.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace mhac::visual;
using L=GestureLoop;
static void setup(PerformanceSurface& u,unsigned m,unsigned s,uint32_t start){
 uint32_t t=start; auto tap=[&](int x,int y){u.sample(true,x,y,t+=8);u.sample(false,0,0,t+=8);};
 tap(310,10);for(unsigned i=0;i<m;++i)tap(270,230);for(unsigned i=0;i<s;++i)tap(184,10);
 t=start+512;tap(160,10);for(unsigned i=0;i<40;++i)u.sample(true,40+int(i)*5,190-int(i)*3,t+=8);
 u.sample(false,0,0,t+=8);assert(u.loopState()==L::State::Playing);
}
int main(){
 // A rest cannot mutate any stored coordinate; a one-point paint preserves X.
 L l; l.toggle(); l.observe({100,200,true},0); l.observe({300,400,true},16); l.observe({},32);
 assert(l.paintTimbre(32,9999)); auto p=l.preview(); assert(p.count==2 && p.points[0].x==100 && p.points[0].y==4095 && p.points[1].y==400);
 assert(!l.paintTimbre(64,0)); auto q=l.preview(); assert(q.points[0].y==4095 && q.points[1].y==400);
 unsigned checks=0,edits=0,rest=0,pcmChanges=0;
 for(uint32_t start:{0u,0xfffffd00u})for(unsigned m=0;m<xy_map_count;++m)for(unsigned s=0;s<L::style_count;++s){
  PerformanceSurface a,b;setup(a,m,s,start);setup(b,m,s,start);
  mhac::Engine ea,eb;mhac::Exchange ia,ib;std::array<int16_t,mhac::block> pa{},pb{};
  const auto original=b.loopPreview();
  for(unsigned i=0;i<400;++i){
   uint32_t t=start+856+i*5;a.sample(false,0,0,t);
   const bool arm=i>=4 && i<6,paint=i>=8 && i<148;
   b.sample(arm||paint,arm?220:130,arm?10:32+int(i%30),t);
   auto ca=a.effective(t),cb=b.effective(t);
   assert(b.loopState()==L::State::Playing && !cb.mute && ca.x==cb.x && ca.gate==cb.gate && ca.patch==cb.patch);
   assert(b.loopPreview().overdub==(i>=4 && i<148));
   if(m==4)assert(cb.y%1365==0);
   if(ca.y!=cb.y)++edits;
   if(!cb.gate)++rest;
   publishControl(ia,ca);publishControl(ib,cb);ea.render(ia,pa.data(),pa.size());eb.render(ib,pb.data(),pb.size());
   for(unsigned k=0;k<pa.size();++k){assert(pb[k]>=-8191 && pb[k]<=8191);pcmChanges+=pa[k]!=pb[k];}
   ++checks;
  }
  auto after=b.loopPreview();assert(after.count==original.count);for(unsigned i=0;i<after.count;++i)assert(after.points[i].x==original.points[i].x);
  b.sample(true,220,10,start+2900);b.sample(false,0,0,start+2908);b.sample(true,120,100,start+2916);b.expire(start+3017);
  assert(b.control().mute && b.loopState()==L::State::Idle && !b.loopPreview().overdub && !b.effective(start+3017).gate);
 }
 assert(edits && rest && pcmChanges);std::printf("overdub PASS: %u pitch/gate comparisons, %u edited control blocks, %u rest blocks, %u changed PCM samples, preview=%zu loop=%zu surface=%zu\n",checks,edits,rest,pcmChanges,sizeof(L::Preview),sizeof(L),sizeof(PerformanceSurface));
}
