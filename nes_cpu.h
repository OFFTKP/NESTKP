#pragma once
#ifndef TKPEMU_NES_CPU_H
#define TKPEMU_NES_CPU_H
#include <cstdint>
#include <limits>
#include <type_traits>
#include <array>
#include <queue>
namespace TKPEmu::NES::Devices {
    union FlagUnion {
    public:
        struct {
            uint8_t Carry        : 1;
            uint8_t Zero         : 1;
            uint8_t InterruptDis : 1;
            uint8_t Decimal      : 1;
            uint8_t Unused       : 2;
            uint8_t Overflow     : 1;
            uint8_t Negative     : 1;
        } Flags;
        operator uint8_t&() {
            return value_;
        }
        uint8_t& operator=(const uint8_t& rhs) {
            return value_ = rhs;
        }
    private:
        uint8_t value_;
    };
    enum Operation {
        ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC,
        CLD, CLI, CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP,
        JSR, LDA, LDX, LDY, LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI,
        RTS, SBC, SEC, SED, SEI, STA, STX, STY, TAX, TAY, TSX, TXA, TXS, TYA,
        XXX
    };
    enum AddressingMode {
        IMP, IMM, ZPG, ZPX,
        ZPY, ABS, ABX, ABY,
        IND, INX, INY, ACC,
        REL, XXX
    };
    struct Instruction {
        Operation op;
        AddressingMode addr;
    }
    class NES {
    private:
        std::array<Instruction, 0x100> instructions_ = {
            #define I(A,B) { A, B }
            I(BRK,IMP), I(ORA,IND), I(XXX,XXX), I(XXX,XXX), I(XXX,XXX), I(ORA,ZPG), I(ASL,ZPG), I(XXX,XXX), I(PHP,IMP), I(ORA,IMM), I(ASL,ACC), I(XXX,XXX), I(XXX,XXX), I(ORA,ABS), I(ASL,ABS), I(XXX,XXX),
            I(BPL,REL), I(ORA,IND), I(XXX,XXX), I(XXX,XXX), I(XXX,XXX), I(ORA,ZPX), I(ASL,ZPX), I(XXX,XXX), I(CLC,IMP), I(ORA,ABY), I(XXX,XXX), I(XXX,XXX), I(XXX,XXX), I(ORA,ABX), I(ASL,ABX), I(XXX,XXX),
            I(JSR,ABS), I(AND,IND), I(XXX,XXX), I(XXX,XXX), I(BIT,ZPG), I(AND,ZPG), I(ROL,ZPG), I(XXX,XXX), I(PLP,IMP), I(AND,IMM), I(ROL,ACC), I(XXX,XXX), I(BIT,ABS), I(AND,ABS), I(ROL,ABS), I(XXX,XXX),
            I(BMI,REL), I(AND,IND), I(XXX,XXX), I(XXX,XXX), I(XXX,XXX), I(AND,ZPX), I(ROL,ZPX), I(XXX,XXX), I(SEC,IMP), I(AND,ABY), I(XXX,XXX), I(XXX,XXX), I(XXX,XXX), I(AND,ABX), I(ROL,ABX), I(XXX,XXX),

            #undef I
        };
    public:
        FlagUnion P;
        uint8_t A, X, Y, SP;
        uint16_t PC;
    private:
        // We split the instruction into it's parts and update APU&PPU in between instructions
        // like on real hardware
        std::queue<void(*f)()> instruction_queue_;
        template<typename T>
        uint8_t inc(T& mem, FlagUnion* P_ptr = nullptr) {
            ++mem;
            if (P_ptr) {
                P_ptr.Flags.Zero = !static_cast<bool>(mem);
                P_ptr.Flags.Negative = (mem >> 7);
            }
            return 1;
        }
        template<typename T>
        uint8_t dec(T& mem, FlagUnion* P_ptr = nullptr) {
            --mem;
            if (P_ptr) {
                P_ptr.Flags.Zero = !static_cast<bool>(mem);
                P_ptr.Flags.Negative = (mem >> 7);
            }
            return 1;
        }
    };
}
#endif