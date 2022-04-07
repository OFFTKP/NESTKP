#pragma once
#ifndef TKP_NES_CPU_H
#define TKPEMU_NES_CPU_H
#include <cstdint>
#include <limits>
#include <type_traits>
#include <array>
#include <queue>
#include <string>
namespace TKPEmu::NES::Devices {
    using FuncRet = uint8_t;
    using FuncRetPtr = FuncRet (*)();
    using TickRet = bool;
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
            XXX
        };
        enum AddressingMode {
            IMP, IMM, ZP0, ZPX,
            ZPY, ABS, ABX, ABY,
            IND, IZX, IZY, ACC,
            REL
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
            I(BRK, IMM), I(ORA, IZX), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(ORA, ZP0), I(ASL, ZP0), I(XXX, IMP), I(PHP, IMP), I(ORA, IMM), I(ASL, IMP), I(XXX, IMP), I(NOP, IMP), I(ORA, ABS), I(ASL, ABS), I(XXX, IMP),
            I(BPL, REL), I(ORA, IZY), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(ORA, ZPX), I(ASL, ZPX), I(XXX, IMP), I(CLC, IMP), I(ORA, ABY), I(NOP, IMP), I(XXX, IMP), I(NOP, IMP), I(ORA, ABX), I(ASL, ABX), I(XXX, IMP),
            I(JSR, ABS), I(AND, IZX), I(XXX, IMP), I(XXX, IMP), I(BIT, ZP0), I(AND, ZP0), I(ROL, ZP0), I(XXX, IMP), I(PLP, IMP), I(AND, IMM), I(ROL, IMP), I(XXX, IMP), I(BIT, ABS), I(AND, ABS), I(ROL, ABS), I(XXX, IMP),
            I(BMI, REL), I(AND, IZY), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(AND, ZPX), I(ROL, ZPX), I(XXX, IMP), I(SEC, IMP), I(AND, ABY), I(NOP, IMP), I(XXX, IMP), I(NOP, IMP), I(AND, ABX), I(ROL, ABX), I(XXX, IMP),
            I(RTI, IMP), I(EOR, IZX), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(EOR, ZP0), I(LSR, ZP0), I(XXX, IMP), I(PHA, IMP), I(EOR, IMM), I(LSR, IMP), I(XXX, IMP), I(JMP, ABS), I(EOR, ABS), I(LSR, ABS), I(XXX, IMP),
            I(BVC, REL), I(EOR, IZY), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(EOR, ZPX), I(LSR, ZPX), I(XXX, IMP), I(CLI, IMP), I(EOR, ABY), I(NOP, IMP), I(XXX, IMP), I(NOP, IMP), I(EOR, ABX), I(LSR, ABX), I(XXX, IMP),
            I(RTS, IMP), I(ADC, IZX), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(ADC, ZP0), I(ROR, ZP0), I(XXX, IMP), I(PLA, IMP), I(ADC, IMM), I(ROR, IMP), I(XXX, IMP), I(JMP, IND), I(ADC, ABS), I(ROR, ABS), I(XXX, IMP),
            I(BVS, REL), I(ADC, IZY), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(ADC, ZPX), I(ROR, ZPX), I(XXX, IMP), I(SEI, IMP), I(ADC, ABY), I(NOP, IMP), I(XXX, IMP), I(NOP, IMP), I(ADC, ABX), I(ROR, ABX), I(XXX, IMP),
            I(NOP, IMP), I(STA, IZX), I(NOP, IMP), I(XXX, IMP), I(STY, ZP0), I(STA, ZP0), I(STX, ZP0), I(XXX, IMP), I(DEY, IMP), I(NOP, IMP), I(TXA, IMP), I(XXX, IMP), I(STY, ABS), I(STA, ABS), I(STX, ABS), I(XXX, IMP),
            I(BCC, REL), I(STA, IZY), I(XXX, IMP), I(XXX, IMP), I(STY, ZPX), I(STA, ZPX), I(STX, ZPY), I(XXX, IMP), I(TYA, IMP), I(STA, ABY), I(TXS, IMP), I(XXX, IMP), I(NOP, IMP), I(STA, ABX), I(XXX, IMP), I(XXX, IMP),
            I(LDY, IMM), I(LDA, IZX), I(LDX, IMM), I(XXX, IMP), I(LDY, ZP0), I(LDA, ZP0), I(LDX, ZP0), I(XXX, IMP), I(TAY, IMP), I(LDA, IMM), I(TAX, IMP), I(XXX, IMP), I(LDY, ABS), I(LDA, ABS), I(LDX, ABS), I(XXX, IMP),
            I(BCS, REL), I(LDA, IZY), I(XXX, IMP), I(XXX, IMP), I(LDY, ZPX), I(LDA, ZPX), I(LDX, ZPY), I(XXX, IMP), I(CLV, IMP), I(LDA, ABY), I(TSX, IMP), I(XXX, IMP), I(LDY, ABX), I(LDA, ABX), I(LDX, ABY), I(XXX, IMP),
            I(CPY, IMM), I(CMP, IZX), I(NOP, IMP), I(XXX, IMP), I(CPY, ZP0), I(CMP, ZP0), I(DEC, ZP0), I(XXX, IMP), I(INY, IMP), I(CMP, IMM), I(DEX, IMP), I(XXX, IMP), I(CPY, ABS), I(CMP, ABS), I(DEC, ABS), I(XXX, IMP),
            I(BNE, REL), I(CMP, IZY), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(CMP, ZPX), I(DEC, ZPX), I(XXX, IMP), I(CLD, IMP), I(CMP, ABY), I(NOP, IMP), I(XXX, IMP), I(NOP, IMP), I(CMP, ABX), I(DEC, ABX), I(XXX, IMP),
            I(CPX, IMM), I(SBC, IZX), I(NOP, IMP), I(XXX, IMP), I(CPX, ZP0), I(SBC, ZP0), I(INC, ZP0), I(XXX, IMP), I(INX, IMP), I(SBC, IMM), I(NOP, IMP), I(SBC, IMP), I(CPX, ABS), I(SBC, ABS), I(INC, ABS), I(XXX, IMP),
            I(BEQ, REL), I(SBC, IZY), I(XXX, IMP), I(XXX, IMP), I(NOP, IMP), I(SBC, ZPX), I(INC, ZPX), I(XXX, IMP), I(SED, IMP), I(SBC, ABY), I(NOP, IMP), I(XXX, IMP), I(NOP, IMP), I(SBC, ABX), I(INC, ABX), I(XXX, IMP),
            #undef I
        } };
    public:
        FlagUnion P;
        uint8_t A, X, Y, SP;
        uint16_t PC;
        TickRet tick();
    private:
        // We split the instruction into it's parts and update APU&PPU in between instructions
        // like on real hardware
        std::queue<FuncRetPtr> instruction_queue_;
        FuncRetPtr get_addr_mode(AddressingMode addr);
        FuncRet read(uint16_t address);
        template<typename T>
        FuncRet inc(T& mem, FlagUnion* P_ptr = nullptr) {
            ++mem;
            if (P_ptr) {
                P_ptr->Flags.Zero = !static_cast<bool>(mem);
                P_ptr->Flags.Negative = (mem >> 7);
            }
            return 1;
        }
        template<typename T>
        FuncRet dec(T& mem, FlagUnion* P_ptr = nullptr) {
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