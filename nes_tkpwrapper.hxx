#pragma once
#ifndef TKP_NES_NES_H
#define TKP_NES_NES_H
#include <include/emulator.h>
#include "nes_cpu.hxx"
#include "nes_cpubus.hxx"
#include "nes_ppu.hxx"
#include "nes_apu.hxx"

namespace TKPEmu::NES {
    class NES_TKPWrapper : public Emulator {
        TKP_EMULATOR(NES_TKPWrapper);
    private:
        void update();
        void v_log() override;
        Devices::PPU ppu_ { DrawMutex };
        Devices::APU apu_ {};
        Devices::CPUBus cpubus_ { ppu_, apu_ };
        Devices::CPU cpu_ { cpubus_, Paused };
    };
}
#endif