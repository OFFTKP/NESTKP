#ifndef TKP_NES_APU_H
#define TKP_NES_APU_H
#include <cstdint>

namespace TKPEmu::NES::Devices {
    class CPUBus;
    class APU {
    public:
        void Tick();
        void Reset();
    private:
        void invalidate(uint8_t addr, uint8_t data);
        __always_inline void tick_impl();
        bool should_tick_ = false;
        uint32_t clock_ = 0;
        uint16_t sq1_timer_ = 0;

        friend class CPUBus;
    };
}
#endif