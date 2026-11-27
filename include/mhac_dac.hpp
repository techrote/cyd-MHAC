#pragma once
#include <cstdint>

namespace mhac {
// Classic ESP32 built-in DAC consumes the high byte of a 16-bit I2S slot.
constexpr uint16_t dac_floor(int16_t sample) noexcept {
    return uint16_t((int32_t(sample)+32768)&0xff00);
}
// First-order error feedback, with bounded integer state and exact zero reset.
// One instance per mono stream, retained across DMA blocks. Not a DSP gain change.
class DacQuantizer {
    int32_t residual_=0;
public:
    uint16_t pack(int16_t sample) noexcept {
        if(sample==0) { residual_=0; return 0x8000; }
        const int32_t v=int32_t(sample)+residual_;
        // v >= -32896, so biased rounding never shifts a negative integer.
        const uint32_t rounded=uint32_t(v+32896)>>8;
        const uint32_t code=rounded>255u?255u:rounded;
        const int32_t error=v-(int32_t(code)-128)*256;
        residual_=error>127?127:error< -128? -128:error;
        return uint16_t(code<<8);
    }
    int32_t residual() const noexcept { return residual_; }
};
}
