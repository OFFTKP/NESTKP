#pragma once
#ifndef TKP_NES_CPU_H
#define TKP_NES_CPU_H
#include <cstdint>
#include <bitset>
#include "nes_cpubus.hxx"
#define t(x) template<class T> void x()
#define immediate(x, func) template<> void CPU::x<Immediate>() { \
    auto data = read_no_d(PC++); \
    func; \
}
#define indirect(x, func) template<> void CPU::x<Indirect>() { \
    auto b1 = read(PC++); \
    uint16_t b2 = read(PC++); \
    auto ind = b1 | (b2 << 8); \
    auto b1_p = read(ind++); \
    uint16_t b2_p = read_no_d(ind); \
    uint16_t addr = b1_p | (b2_p << 8); \
    func; \
}
#define absolute(x, func) template<> void CPU::x<Absolute>() {\
    auto b1 = read(PC++); \
    uint16_t b2 = read_no_d(PC++); \
    uint16_t addr = b1 | (b2 << 8); \
    func; \
}
#define zeropage(x, func) template<> void CPU::x<ZeroPage>() {\
    auto b1 = read(PC++); \
    uint16_t addr = 0xFF00 + b1; \
    func; \
}

// Addressing modes
class Indirect;
class Immediate;
class Absolute;
class ZeroPage;

namespace TKPEmu::NES {
    class NES;
}

namespace TKPEmu::NES::Devices {
    class CPU {
    public:
        void Tick();
        void Reset();
    private:
        Bus bus_;
        __always_inline void TAX(), TXA(), JSR(), SEC(), BCS(),
            BCC(), CLC(), BEQ(), BNE(), BVS(), BVC(), BMI(), BPL(),
            RTS(), SEI(), SED(), CLD(), PHP(), PLA(), PLP(), PHA();
        // Template functions to match addressing modes
        t(JMP); t(LDX); t(STX); t(LDA); t(STA); t(BIT); t(CMP);
        __always_inline void delay(uint8_t i);
        uint8_t A, X, Y, SP;
        std::bitset<8> P;
        uint16_t PC;

        uint8_t fetched_ = 0;

        uint8_t read_no_d(uint16_t addr);
        uint8_t read(uint16_t addr);
        void push(uint8_t data);
        uint8_t pull();
        void write(uint16_t addr, uint8_t data);

        __always_inline void fetch();
        __always_inline void execute();

        friend class TKPEmu::NES::NES;
    };
}
#endif