#pragma once
#ifndef TKP_NES_CPU_H
#define TKP_NES_CPU_H
#include <cstdint>
#include <bitset>
#include <iostream>
#include "nes_cpubus.hxx"
#define t(x) template<class T> void x()
#define implied(x, func) template<> void CPU::x<Implied>() { \
    func; \
    prefetch(); \
}
#define accumulator(x, func) template<> void CPU::x<Accumulator>() {\
    auto data = A; \
    func; \
    prefetch(); \
}
#define immediate(x, func) template<> void CPU::x<Immediate>() { \
    uint16_t addr; \
    auto data = read_no_d(PC++); \
    func; \
    prefetch(); \
}
#define indirect(x, func) template<> void CPU::x<Indirect>() { \
    auto b1 = read(PC++); \
    uint16_t b2 = read(PC++); \
    auto ind = b1 | (b2 << 8); \
    auto b1_p = read(ind); \
    uint16_t b2_p = read(((ind + 1) & 0xFF) | (ind & 0xFF00)); \
    uint16_t addr = b1_p | (b2_p << 8); \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define absolute(x, func) template<> void CPU::x<Absolute>() {\
    auto b1 = read(PC++); \
    uint16_t b2 = read(PC++); \
    uint16_t addr = b1 | (b2 << 8); \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define absolutey(x, func) template<> void CPU::x<AbsoluteY>() {\
    auto b1 = read(PC++); \
    uint16_t b2 = read(PC++); \
    uint16_t addr = (b1 | (b2 << 8)) + Y; \
    delay((addr >> 8) != b2); \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define absolutex(x, func) template<> void CPU::x<AbsoluteX>() {\
    auto b1 = read(PC++); \
    uint16_t b2 = read(PC++); \
    uint16_t addr = (b1 | (b2 << 8)) + X; \
    delay((addr >> 8) != b2); \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define zeropage(x, func) template<> void CPU::x<ZeroPage>() {\
    auto b1 = read(PC++); \
    uint16_t addr = b1; \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define zeropagex(x, func) template<> void CPU::x<ZeroPageX>() {\
    auto b1 = read(PC++); \
    uint16_t addr = (b1 + X) & 0xFF; \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define zeropagey(x, func) template<> void CPU::x<ZeroPageY>() {\
    auto b1 = read(PC++); \
    uint16_t addr = (b1 + Y) & 0xFF; \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define indirectx(x, func) template<> void CPU::x<IndirectX>() {\
    auto b1 = read(PC++); \
    uint16_t b1_p = read((b1 + X) & 0xFF); \
    uint16_t b2_p = read((b1 + X + 1) & 0xFF); \
    uint16_t addr = b1_p | (b2_p << 8); \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}
#define indirecty(x, func) template<> void CPU::x<IndirectY>() {\
    auto b1 = read(PC++); \
    uint16_t b1_p = read((b1) & 0xFF); \
    uint16_t b2_p = read((b1 + 1) & 0xFF); \
    uint16_t addr = (b1_p | (b2_p << 8)) + Y; \
    uint8_t data = read_no_d(addr); \
    func; \
    prefetch(); \
}

// Addressing modes
class Implied;
class Immediate;
class Absolute;
class AbsoluteX;
class AbsoluteY;
class ZeroPage;
class ZeroPageX;
class ZeroPageY;
class Indirect;
class IndirectX;
class IndirectY;
class Accumulator;

namespace TKPEmu::NES {
    class NES_TKPWrapper;
}

namespace TKPEmu::NES::Devices {
    class CPU {
    public:
        CPU(CPUBus& bus);
        void Tick();
        void Reset();
        void NMI();
    private:
        CPUBus& bus_;
        __always_inline void TAY(), TAX(), TYA(), TXA(), JSR(), SEC(), 
            BCC(), CLC(), BEQ(), BNE(), BVS(), BVC(), BMI(), BPL(),
            RTS(), SEI(), SED(), CLD(), PHP(), PLA(), PLP(), PHA(),
            CLV(), INY(), INX(), DEY(), DEX(), BCS(), TSX(), TXS(),
            RTI();
        // Template functions to match addressing modes
        t(JMP); t(LDX); t(STX); t(LDA); t(STA); t(BIT); t(CMP); t(AND);
        t(ORA); t(EOR); t(ADC); t(LDY); t(CPY); t(CPX); t(SBC); t(LSR);
        t(ASL); t(ROR); t(ROL); t(STY); t(INC); t(DEC); t(NOP); t(LAX);
        t(SAX); t(DCP); t(ISC); t(SLO); t(RLA); t(RRA); t(SRE);
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
        friend class TKPEmu::NES::NES_TKPWrapper;
    };
}
#endif