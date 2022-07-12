#include "nes_cpu.hxx"
#include <stdexcept>
constexpr uint8_t C = 0;
constexpr uint8_t Z = 1;
constexpr uint8_t I = 2;
constexpr uint8_t D = 3;
constexpr uint8_t V = 6;
constexpr uint8_t N = 7;
#define check_nz(x) P.set(Z, x == 0); P.set(N, x & 0x80)
#define ADC_impl int8_t cur_carry = P.test(C); \
            int8_t data_cast = data;\
            int8_t A_cast = A;\
            bool carry = ((uint16_t)A + (uint16_t)data + cur_carry) > 0xFF;\
            bool temp = __builtin_add_overflow(data_cast, cur_carry, &data_cast);\
            bool overflow = __builtin_add_overflow(A_cast, data_cast, &A_cast) | temp;\
            A = A_cast;\
            P.set(C, carry); P.set(V, overflow); check_nz(A)
#define SBC_impl data = data ^ 0xFF; \
        ADC_impl


namespace TKPEmu::NES::Devices {
    immediate(LDX, X = data; check_nz(X));
    immediate(LDY, Y = data; check_nz(Y));
    immediate(LDA, A = data; check_nz(A));
    immediate(CMP, check_nz(A - data); P.set(C, A >= data));
    immediate(CPX, check_nz(X - data); P.set(C, X >= data));
    immediate(CPY, check_nz(Y - data); P.set(C, Y >= data));
    immediate(AND, A = A & data; check_nz(A));
    immediate(ORA, A = A | data; check_nz(A));
    indirectx(ORA, A = A | data; check_nz(A));
    zeropage(ORA, A = A | data; check_nz(A));
    immediate(EOR, A = A ^ data; check_nz(A));
    immediate(ADC, ADC_impl);
    immediate(SBC, SBC_impl);
    indirect(JMP, PC = addr);
    absolute(JMP, PC = addr);
    zeropage(BIT, P.set(N, data & 0b1000'0000); P.set(V, data & 0b0100'0000); P.set(Z, !(A & data)));

    // Store instructions
    #define prefetch();
    zeropage(STX, write(addr, X));
    zeropage(STA, write(addr, A));
    #undef prefetch

    void CPU::SEC() {
        delay(1);
        P.set(C, true);
    }

    void CPU::SEI() {
        delay(1);
        P.set(I, true);
    }

    void CPU::SED() {
        delay(1);
        P.set(D, true);
    }

    void CPU::CLC() {
        delay(1);
        P.set(C, false);
    }

    void CPU::CLD() {
        delay(1);
        P.set(D, false);
    }

    void CPU::CLV() {
        delay(1);
        P.set(V, false);
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
        check_nz(A);
    }

    void CPU::PLP() {
        delay(2);
        P &= 0b0011'0000;
        P |= pull() & 0b1100'1111;
    }

    void CPU::JSR() {
        auto b1 = read(PC++);
        delay(1);
        push(PC >> 8);
        push(PC & 0xFF);
        uint16_t b2 = read(PC);
        uint16_t addr = b1 | (b2 << 8);
        PC = addr;
    }

    void CPU::RTS() {
        delay(2);
        auto pc_l = pull();
        auto pc_h = pull();
        PC = (pc_l | (pc_h << 8)) + 1;
        delay(1);
    }

    void CPU::TAX() {
        delay(1);
        X = A;
        check_nz(X);
    }

    void CPU::TXA() {
        delay(1);
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

    void CPU::INX() {
        X++;
        check_nz(X);
    }

    void CPU::INY() {
        Y++;
        check_nz(Y);
    }

    void CPU::fetch() {
        if (!was_prefetched_) [[unlikely]] {
            prefetch();
        }
        was_prefetched_ = false;
        PC++;
    }

    void CPU::prefetch() {
        fetched_ = read(PC);
        was_prefetched_ = true;
    }

    void CPU::delay(uint8_t i) {
        cycles_ += i;
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
        return read(0x0100 | ++SP);
    }

    void CPU::execute() {
        #define NOP(); delay(1);
        #define ins(num, instr) case num: instr(); break
        switch (fetched_) {
            ins(0x05, ORA<ZeroPage>);
            ins(0x08, PHP);
            ins(0x09, ORA<Immediate>);
            ins(0x10, BPL);
            ins(0x18, CLC);
            ins(0x20, JSR);
            ins(0x24, BIT<ZeroPage>);
            ins(0x28, PLP);
            ins(0x29, AND<Immediate>);
            ins(0x30, BMI);
            ins(0x38, SEC);
            ins(0x48, PHA);
            ins(0x49, EOR<Immediate>);
            ins(0x4C, JMP<Absolute>);
            ins(0x50, BVC);
            ins(0x60, RTS);
            ins(0x68, PLA);
            ins(0x6C, JMP<Indirect>);
            ins(0x69, ADC<Immediate>);
            ins(0x70, BVS);
            ins(0x78, SEI);
            ins(0x85, STA<ZeroPage>);
            ins(0x86, STX<ZeroPage>);
            ins(0x8A, TXA);
            ins(0x90, BCC);
            ins(0xA0, LDY<Immediate>);
            ins(0xA2, LDX<Immediate>);
            ins(0xA9, LDA<Immediate>);
            ins(0xAA, TAX);
            ins(0xB0, BCS);
            ins(0xB8, CLV);
            ins(0xC0, CPY<Immediate>);
            ins(0xC8, INY);
            ins(0xC9, CMP<Immediate>);
            ins(0xD0, BNE);
            ins(0xD8, CLD);
            ins(0xE0, CPX<Immediate>);
            ins(0xE8, INX);
            ins(0xE9, SBC<Immediate>);
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
        cycles_ = 7;
        was_prefetched_ = false;
        bus_.Reset();
    }
}