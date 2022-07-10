#pragma once
#ifndef TKP_NES_NES_H
#define TKP_NES_NES_H
#include <include/emulator.h>
#include "nes_cpu.hxx"
#include "nes_cpubus.hxx"

namespace TKPEmu::NES {
    class NES : public Emulator {
        TKP_EMULATOR(NES);
    private:
        void update();
        void v_log() override;
        Devices::CPU cpu_;
        bool should_draw_ = false;
    };
}
#endif