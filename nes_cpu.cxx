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
    indirect(JMP, PC = data);
    absolute(JMP, PC = data);
    zeropage(STX, write(addr, X));

    void CPU::SEC() {
        P.set(C, true);
    }

    void CPU::CLC() {
        P.set(C, false);
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

    void CPU::execute() {
        #define NOP();
        #define ins(num, instr) case num: instr(); break
        switch (fetched_) {
            ins(0x18, CLC);
            ins(0x20, JSR);
            ins(0x38, SEC);
            ins(0x4C, JMP<Absolute>);
            ins(0x6C, JMP<Indirect>);
            ins(0x86, STX<ZeroPage>);
            ins(0x8A, TXA);
            ins(0xA2, LDX<Immediate>);
            ins(0xAA, TAX);
            ins(0xB0, BCS);
            ins(0xEA, NOP);
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
        bus_.Reset();
    }
}