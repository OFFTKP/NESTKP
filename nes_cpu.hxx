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
    prefetch(); \
}
#define indirect(x, func) template<> void CPU::x<Indirect>() { \
    auto b1 = read(PC++); \
    uint16_t b2 = read(PC++); \
    auto ind = b1 | (b2 << 8); \
    auto b1_p = read(ind++); \
    uint16_t b2_p = read(ind); \
    uint16_t addr = b1_p | (b2_p << 8); \
    uint8_t data = read_no_d(addr); \
    func; \
}
#define absolute(x, func) template<> void CPU::x<Absolute>() {\
    auto b1 = read(PC++); \
    uint16_t b2 = read(PC++); \
    uint16_t addr = b1 | (b2 << 8); \
    uint8_t data = read_no_d(addr); \
    func; \
}
#define zeropage(x, func) template<> void CPU::x<ZeroPage>() {\
    auto b1 = read(PC++); \
    uint16_t addr = 0xFF00 + b1; \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define indirectx(x, func) template<> void CPU::x<IndirectX>() {\
    auto b1 = read(PC++); \
    uint16_t b1_p = read(b1 + X); \
    uint16_t b2_p = read(b1 + X + 1); \
    uint16_t addr = b1_p | (b2_p << 8); \
    uint8_t data = read_no_d(addr); \
    func; \
}

// Addressing modes
class Indirect;
class Immediate;
class Absolute;
class ZeroPage;
class IndirectX;

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
            RTS(), SEI(), SED(), CLD(), PHP(), PLA(), PLP(), PHA(),
            CLV(), INY(), INX();
        // Template functions to match addressing modes
        t(JMP); t(LDX); t(STX); t(LDA); t(STA); t(BIT); t(CMP); t(AND);
        t(ORA); t(EOR); t(ADC); t(LDY); t(CPY); t(CPX); t(SBC);
        __always_inline void delay(uint8_t i);
        uint8_t A, X, Y, SP;
        std::bitset<8> P;
        uint16_t PC;
        uint64_t cycles_ = 0;
        uint8_t fetched_ = 0;
        bool was_prefetched_ = false;

        uint8_t read_no_d(uint16_t addr);
        uint8_t read(uint16_t addr);
        void push(uint8_t data);
        uint8_t pull();
        void write(uint16_t addr, uint8_t data);

        __always_inline void fetch();
        __always_inline void prefetch();
        __always_inline void execute();
        friend class TKPEmu::NES::NES;
    };
}
#endif