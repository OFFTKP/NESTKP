#include "nes_cpu.hxx"
#include <stdexcept>
constexpr uint8_t C = 0;
constexpr uint8_t Z = 1;
constexpr uint8_t I = 2;
constexpr uint8_t D = 3;
constexpr uint8_t V = 6;
constexpr uint8_t N = 7;
#define check_nz(x) P.set(Z, x == 0); P.set(N, x & 0x80)

namespace TKPEmu::NES::Devices {
    immediate(LDX, X = data; check_nz(X));
    immediate(LDA, A = data; check_nz(A));
    immediate(CMP, );
    indirect(JMP, PC = addr);
    absolute(JMP, PC = addr);
    zeropage(STX, write(addr, X));
    zeropage(STA, write(addr, A));
    zeropage(BIT, auto data = read_no_d(addr); P.set(N, data & 0b1000'0000); P.set(V, data & 0b0100'0000); P.set(Z, A & data));

    void CPU::SEC() {
        P.set(C, true);
    }

    void CPU::SEI() {
        P.set(I, true);
    }

    void CPU::SED() {
        P.set(D, true);
    }

    void CPU::CLC() {
        P.set(C, false);
    }

    void CPU::CLD() {
        P.set(D, false);
    }

    void CPU::PHA() {
        delay(1);
        push(A);
    }

    void CPU::PHP() {
        delay(1);
        uint8_t p = P.to_ulong() | 0b0011'0000;
        push(p);
    }

    void CPU::PLA() {
        delay(2);
        A = pull();
    }

    void CPU::PLP() {
        delay(2);
        P = pull();
    }

    void CPU::JSR() {
        auto b1 = read(PC++);
        delay(1);
        push(PC >> 8);
        push(PC & 0xFF);
        uint16_t b2 = read_no_d(PC);
        uint16_t addr = b1 | (b2 << 8);
        PC = addr;
    }

    void CPU::RTS() {
        delay(2);
        auto pc_l = pull();
        auto pc_h = pull();
        PC = pc_l | (pc_h << 8);
        delay(1);
    }

    void CPU::TAX() {
        X = A;
        check_nz(X);
    }

    void CPU::TXA() {
        A = X;
        check_nz(A);
    }

    void CPU::BCS() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(C)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BCC() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(C)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BEQ() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(Z)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BNE() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(Z)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BVS() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(V)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BVC() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(V)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BPL() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(N)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BMI() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(N)) {
            PC += b1;
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::fetch() {
        fetched_ = read(PC++);
    }

    void CPU::delay(uint8_t i) {

    }

    uint8_t CPU::read_no_d(uint16_t addr) {
        return bus_.read(addr);
    }

    uint8_t CPU::read(uint16_t addr) {
        auto ret = read_no_d(addr);
        delay(1);
        return ret;
    }
    
    void CPU::write(uint16_t addr, uint8_t data) {
        bus_.write(addr, data);
        delay(1);
    }

    void CPU::push(uint8_t data) {
        write(0x0100 | SP--, data);
    }

    uint8_t CPU::pull() {
        return read(0x0100 | SP++);
    }

    void CPU::execute() {
        #define NOP();
        #define ins(num, instr) case num: instr(); break
        switch (fetched_) {
            ins(0x08, PHP);
            ins(0x10, BPL);
            ins(0x18, CLC);
            ins(0x20, JSR);
            ins(0x24, BIT<ZeroPage>);
            ins(0x28, PLP);
            ins(0x30, BMI);
            ins(0x38, SEC);
            ins(0x48, PHA);
            ins(0x4C, JMP<Absolute>);
            ins(0x50, BVC);
            ins(0x60, RTS);
            ins(0x68, PLA);
            ins(0x6C, JMP<Indirect>);
            ins(0x70, BVS);
            ins(0x78, SEI);
            ins(0x85, STA<ZeroPage>);
            ins(0x86, STX<ZeroPage>);
            ins(0x8A, TXA);
            ins(0x90, BCC);
            ins(0xA2, LDX<Immediate>);
            ins(0xA9, LDA<Immediate>);
            ins(0xAA, TAX);
            ins(0xB0, BCS);
            ins(0xD0, BNE);
            ins(0xD8, CLD);
            ins(0xEA, NOP);
            ins(0xF0, BEQ);
            ins(0xF8, SED);
            default: throw std::runtime_error(std::string("Unimplemented instr:") + std::to_string(fetched_));
        }
        #undef ins
        #undef NOP
    }

    void CPU::Tick() {
        fetch();
        execute();
    }
    
    void CPU::Reset() {
        PC = 0xC000;
        SP = 0xFD;
        A = 0;
        X = 0;
        Y = 0;
        P = 0x24;
        bus_.Reset();
    }
}