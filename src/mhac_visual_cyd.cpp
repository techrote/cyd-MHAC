#if defined(MHAC_VISUAL_TARGET)
#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <LovyanGFX.hpp>
#include "mhac_platform.hpp"
#include "mhac_visual_audio.hpp"
#include "mhac_performance.hpp"
namespace {
using namespace mhac::visual;
static_assert(mhac::feature_version==1 && mhac::rate==24000 && mhac::feature_window==128,"feature contract pin required");
static_assert(mhac::contract_version==1,"visual contract pin required");
class Display : public lgfx::LGFX_Device {
    lgfx::Bus_SPI bus; lgfx::Panel_ILI9341_2 panel;
public:
    Display() {
        auto b=bus.config(); b.spi_host=SPI2_HOST; b.freq_write=40000000; b.freq_read=8000000;
        b.spi_3wire=false; b.pin_sclk=14; b.pin_mosi=13; b.pin_miso=12; b.pin_dc=2;
        bus.config(b); panel.setBus(&bus);
        auto p=panel.config(); p.pin_cs=15; p.pin_rst=-1; p.invert=true; p.bus_shared=false;
        panel.config(p); setPanel(&panel);
    }
};
Display lcd; SPIClass touchBus(VSPI); XPT2046_Touchscreen touch(33);
PerformanceSurface ui; Calibration cal; Frame frame; InkField ink; Trace trace; uint16_t pixels[width*strip];
bool ready=false, fault=false; int row=0; unsigned quality=2, fastFrames=0;
uint32_t lastPoll=0, frameStart=0, frameUs=0, maxStripUs=0, frames=0;
void publish() { publishControl(mhac::exchange,ui.control()); }
}
void mhac_ui_begin() {
    pinMode(0,INPUT_PULLUP); pinMode(21,OUTPUT); digitalWrite(21,LOW);
    ready=lcd.init();
    if(ready) { lcd.setRotation(5); ready=lcd.width()==width && lcd.height()==height; }
    if(!ready) { ui.forceMute(); publish(); return; }
    lcd.fillScreen(0); digitalWrite(21,HIGH);
    touchBus.begin(25,39,32,33); touch.begin(touchBus); touch.setRotation(0);
    publish();
}
void mhac_ui_tick() {
    if(!ready) return;
    const uint32_t now=millis(); ui.expire(now);
    ui.observeWatchdog(mhac::exchange.watchdog_trips.load(std::memory_order_relaxed));
    if(mhac::audio_errors.load(std::memory_order_relaxed)!=0) fault=true;
    if(fault || digitalRead(0)==LOW) ui.forceMute();
    if(uint32_t(now-lastPoll)>=8) {
        lastPoll=now; const auto p=touch.getPoint(); int x=0,y=0;
        cal.landscape(p.x,p.y,x,y); ui.sample(p.z>=400,x,y,now);
        if(fault || digitalRead(0)==LOW) ui.forceMute();
    }
    ui.observeWatchdog(mhac::exchange.watchdog_trips.load(std::memory_order_relaxed));
    const auto active=ui.effective(now);
    publishControl(mhac::exchange,active);
    if(!active.gate || ui.mode()!=5) ink.disconnect();
    if(row==0) {
        frameStart=micros(); maxStripUs=0;
        mhac::AudioFeatures latest{};
        if(mhac::exchange.features.read(latest)) trace.observe(latest,now);
        frame={active,ui.mode(),mhac::exchange.snapshot(),now/25,ui.watchdogLatched(),&ink,ui.scale(),&trace,trace.fresh(now),ui.loopState(),ui.factoryIndex(),ui.factoryAudition()};
        if(frame.mode==5) ink.advance(now,frame.control.x,frame.control.y,frame.control.gate&&!frame.control.mute,frame.analysis&4095u);
    }
    const uint32_t start=micros();
    renderStrip(frame,row,quality,pixels); lcd.pushImage(0,row,width,strip,pixels);
    maxStripUs=std::max(maxStripUs,uint32_t(micros()-start));
    row+=strip;
    if(row==height) {
        row=0; frameUs=micros()-frameStart; ++frames;
        if(frameUs>85000 || maxStripUs>5000) { quality=4; fastFrames=0; }
        else if(quality==4 && frameUs<55000 && maxStripUs<3000) { if(++fastFrames>=60) { quality=2;fastFrames=0; } }
        else fastFrames=0;
        // Low-rate telemetry only while silent and when the UART has space.
        if(frames%120==0 && !active.gate && Serial.availableForWrite()>=120)
            Serial.printf("visual frame_us=%lu strip_us=%lu step=%u blocks=%lu errors=%lu watchdog=%lu\n",(unsigned long)frameUs,(unsigned long)maxStripUs,quality,(unsigned long)mhac::audio_blocks.load(),(unsigned long)mhac::audio_errors.load(),(unsigned long)mhac::exchange.watchdog_trips.load());
        if(frames%120==60 && !active.gate && Serial.availableForWrite()>=120) {
            mhac::AudioTiming t{};
            if(mhac::audio_timing.read(t)) Serial.printf("audio prep_us=%lu render_us=%lu over=%lu failed=%lu\n",(unsigned long)t.max_prepare_us,(unsigned long)t.max_render_us,(unsigned long)t.prepare_over_budget,(unsigned long)t.failed_writes);
        }
    }
}
#endif
