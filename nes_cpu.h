#pragma once
#ifndef TKPEMU_NES_CPU_H
#define TKPEMU_NES_CPU_H
#include <cstdint>
#include <limits>
#include <type_traits>
#include <array>
#include <queue>
#include <string>
namespace TKPEmu::NES::Devices {
    namespace {
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
            XXO
        };
        enum AddressingMode {
            IMP, IMM, ZPG, ZPX,
            ZPY, ABS, ABX, ABY,
            IND, IDX, IDY, ACC,
            REL, XXA
        };
        struct Instruction {
            Operation op;
            AddressingMode addr;
            std::string name;
        };
    }
    class NES {
    private:
        std::array<Instruction, 0x100> instructions_ = { {
            #define I(A,B) { A, B, #A ", " #B }
            I(BRK,IMP), I(ORA,IND), I(XXO,XXA), I(XXO,XXA), I(XXO,XXA), I(ORA,ZPG), I(ASL,ZPG), I(XXO,XXA), I(PHP,IMP), I(ORA,IMM), I(ASL,ACC), I(XXO,XXA), I(XXO,XXA), I(ORA,ABS), I(ASL,ABS), I(XXO,XXA),
            I(BPL,REL), I(ORA,IND), I(XXO,XXA), I(XXO,XXA), I(XXO,XXA), I(ORA,ZPX), I(ASL,ZPX), I(XXO,XXA), I(CLC,IMP), I(ORA,ABY), I(XXO,XXA), I(XXO,XXA), I(XXO,XXA), I(ORA,ABX), I(ASL,ABX), I(XXO,XXA),
            I(JSR,ABS), I(AND,IND), I(XXO,XXA), I(XXO,XXA), I(BIT,ZPG), I(AND,ZPG), I(ROL,ZPG), I(XXO,XXA), I(PLP,IMP), I(AND,IMM), I(ROL,ACC), I(XXO,XXA), I(BIT,ABS), I(AND,ABS), I(ROL,ABS), I(XXO,XXA),
            I(BMI,REL), I(AND,IND), I(XXO,XXA), I(XXO,XXA), I(XXO,XXA), I(AND,ZPX), I(ROL,ZPX), I(XXO,XXA), I(SEC,IMP), I(AND,ABY), I(XXO,XXA), I(XXO,XXA), I(XXO,XXA), I(AND,ABX), I(ROL,ABX), I(XXO,XXA),

            #undef I
        } };
    public:
        FlagUnion P;
        uint8_t A, X, Y, SP;
        uint16_t PC;
    private:
        // We split the instruction into it's parts and update APU&PPU in between instructions
        // like on real hardware
        std::queue<void (*)()> instruction_queue_;
        template<typename T>
        uint8_t inc(T& mem, FlagUnion* P_ptr = nullptr) {
            ++mem;
            if (P_ptr) {
                P_ptr->Flags.Zero = !static_cast<bool>(mem);
                P_ptr->Flags.Negative = (mem >> 7);
            }
            return 1;
        }
        template<typename T>
        uint8_t dec(T& mem, FlagUnion* P_ptr = nullptr) {
            --mem;
            if (P_ptr) {
                P_ptr->Flags.Zero = !static_cast<bool>(mem);
                P_ptr->Flags.Negative = (mem >> 7);
            }
            return 1;
        }
    };
}
#endif