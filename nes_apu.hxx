#ifndef TKP_NES_APU_H
#define TKP_NES_APU_H
#include <cstdint>

namespace TKPEmu::NES::Devices {
    class APU {
    public:
        void Tick();
        void Reset();
    private:
        __always_inline void tick_impl();
        bool should_tick_ = false;
        uint32_t clock_ = 0;
    };
}
#endif