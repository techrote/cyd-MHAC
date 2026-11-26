#include <Arduino.h>
#include <driver/i2s.h>
#include <esp_timer.h>
#include "mhac_platform.hpp"

namespace mhac {
Exchange exchange;
TimingMailbox audio_timing;
std::atomic<uint32_t> audio_blocks{0}, audio_errors{0};
}
static mhac::Engine engine;
static constexpr i2s_port_t port = I2S_NUM_0;
static void audio_task(void*) {
    int16_t samples[mhac::block];
    uint16_t slots[mhac::block*2];
    mhac::TimingMeter timing(mhac::audio_block_budget_us);
    for (;;) {
        const auto start=uint32_t(esp_timer_get_time());
        engine.render(mhac::exchange,samples,mhac::block);
        const auto rendered=uint32_t(esp_timer_get_time());
        for(unsigned i=0;i<mhac::block;++i) {
            const auto u=uint16_t((int32_t(samples[i])+32768)&0xff00);
            slots[i*2]=slots[i*2+1]=u;
        }
        size_t written=0;
        // Blocking only on hardware DMA in dedicated task, not in DSP render.
        const auto prepared=uint32_t(esp_timer_get_time());
        const auto rc=i2s_write(port,slots,sizeof(slots),&written,pdMS_TO_TICKS(30));
        const auto end=uint32_t(esp_timer_get_time());
        const bool ok=rc==ESP_OK && written==sizeof(slots);
        timing.record(start,rendered,prepared,end,ok,mhac::audio_timing);
        if(!ok) {
            mhac::audio_errors.fetch_add(1,std::memory_order_relaxed);
            i2s_stop(port);
            i2s_set_dac_mode(I2S_DAC_CHANNEL_DISABLE);
            vTaskDelete(nullptr); // fail closed; reboot required
            return;
        }
        mhac::audio_blocks.fetch_add(1,std::memory_order_relaxed);
    }
}
static bool audio_begin() {
    i2s_config_t c{};
    c.mode=static_cast<i2s_mode_t>(I2S_MODE_MASTER|I2S_MODE_TX|I2S_MODE_DAC_BUILT_IN);
    c.sample_rate=mhac::rate;
    c.bits_per_sample=I2S_BITS_PER_SAMPLE_16BIT;
    c.channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT;
    c.communication_format=I2S_COMM_FORMAT_STAND_MSB;
    c.dma_buf_count=4;
    c.dma_buf_len=mhac::block;
    c.tx_desc_auto_clear=true;
    if(i2s_driver_install(port,&c,0,nullptr)!=ESP_OK) return false;
    if(i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN)==ESP_OK &&
       xTaskCreatePinnedToCore(audio_task,"mhac-audio",4096,nullptr,5,nullptr,0)==pdPASS) return true;
    i2s_set_dac_mode(I2S_DAC_CHANNEL_DISABLE);
    i2s_driver_uninstall(port);
    return false;
}
// Default diagnostic UI. Visual lane replaces both weak hooks; single control producer.
void __attribute__((weak)) mhac_ui_begin() {}
void __attribute__((weak)) mhac_ui_tick() {
    // Explicit keyboard gate: 1/2/3 phase morph, 4–8 other modes, 0 releases, m mutes.
    // Bounded work; no serial operations on audio task.
    static uint32_t last=0; static bool sounding=false;
    static unsigned preset=0;
    auto play=[&] {
        if(preset<3) mhac::exchange.publish(512+preset*1024,preset*1800,true,false);
        else mhac::exchange.publish_patch(2300,3400,true,false,static_cast<mhac::Patch>(preset-2));
    };
    for(unsigned i=0;i<16 && Serial.available();++i) {
        const int c=Serial.read();
        if(c>='1' && c<='8') {
            preset=unsigned(c-'1');
            play();
            sounding=true; last=millis();
        } else if(c=='d') {
            mhac::AudioTiming t{};
            if(mhac::audio_timing.read(t)) {
                Serial.printf("timing attempts=%lu failed=%lu prep_over=%lu long_gap=%lu max_us render=%lu prep=%lu write=%lu gap=%lu\n",
                    (unsigned long)t.attempts,(unsigned long)t.failed_writes,
                    (unsigned long)t.prepare_over_budget,(unsigned long)t.long_start_gaps,
                    (unsigned long)t.max_render_us,(unsigned long)t.max_prepare_us,
                    (unsigned long)t.max_write_us,(unsigned long)t.max_start_gap_us);
            } else Serial.println("timing unavailable; retry");
        } else if(c=='0' || c=='m') {
            mhac::exchange.publish_patch(512,0,false,c=='m',static_cast<mhac::Patch>(preset<3?0:preset-2)); sounding=false;
        }
    }
    if(sounding && uint32_t(millis()-last)>=2000) {
        mhac::exchange.publish_patch(512,0,false,true,static_cast<mhac::Patch>(preset<3?0:preset-2)); sounding=false;
    } else if(sounding) {
        // Renew only while the diagnostic UI is running, never from audio_task.
        play();
    }
}
void setup() {
    Serial.begin(115200);
    const bool ready=audio_begin();
    Serial.println(ready?"MHAC audio initialized; serial 1-8, 0, m, d":"MHAC audio initialization FAILED");
    mhac_ui_begin();
}
void loop() { mhac_ui_tick(); delay(1); }
