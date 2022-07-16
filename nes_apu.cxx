#include "nes_apu.hxx"

namespace TKPEmu::NES::Devices {
    void APU::Tick() {
        if (should_tick_) {
            tick_impl();
        }
        should_tick_ ^= true;
    }
    
    void APU::tick_impl() {
        clock_ += 1;
    }
}