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

#define all(name, func) immediate(name, func); \
                        indirectx(name, func); \
                        indirecty(name, func); \
                        zeropage(name, func); \
                        zeropagex(name, func); \
                        zeropagey(name, func); \
                        absolute(name, func); \
                        absolutex(name, func); \
                        absolutey(name, func);



namespace TKPEmu::NES::Devices {
    immediate(LDX, X = data; check_nz(X));
    absolute(LDX, X = data; check_nz(X));
    absolutey(LDX, X = data; check_nz(X));
    zeropage(LDX, X = data; check_nz(X));
    zeropagey(LDX, X = data; check_nz(X));
    immediate(LDY, Y = data; check_nz(Y));
    absolute(LDY, Y = data; check_nz(Y));
    absolutex(LDY, Y = data; check_nz(Y));
    zeropage(LDY, Y = data; check_nz(Y));
    zeropagex(LDY, Y = data; check_nz(Y));
    zeropage(LAX, A = data; X = data; check_nz(data));
    zeropagey(LAX, A = data; X = data; check_nz(data));
    absolute(LAX, A = data; X = data; check_nz(data));
    absolutey(LAX, A = data; X = data; check_nz(data));
    indirectx(LAX, A = data; X = data; check_nz(data));
    indirecty(LAX, A = data; X = data; check_nz(data));
    immediate(CPX, check_nz(X - data); P.set(C, X >= data));
    zeropage(CPX, check_nz(X - data); P.set(C, X >= data));
    absolute(CPX, check_nz(X - data); P.set(C, X >= data));
    immediate(CPY, check_nz(Y - data); P.set(C, Y >= data));
    zeropage(CPY, check_nz(Y - data); P.set(C, Y >= data));
    absolute(CPY, check_nz(Y - data); P.set(C, Y >= data));
    all(CMP, check_nz(A - data); P.set(C, A >= data));
    all(LDA, A = data; check_nz(A));
    all(AND, A = A & data; check_nz(A));
    all(ORA, A = A | data; check_nz(A));
    all(EOR, A = A ^ data; check_nz(A));
    all(ADC, ADC_impl);
    all(SBC, SBC_impl);
    accumulator(LSR, A = data >> 1; check_nz(A); P.set(C, data & 1));
    accumulator(ASL, A = data << 1; check_nz(A); P.set(C, data & 0x80));
    accumulator(ROR, A = (data >> 1) | ((uint8_t)P.test(C) << 7); check_nz(A); P.set(C, data & 1));
    accumulator(ROL, A = (data << 1) | P.test(C); check_nz(A); P.set(C, data & 0x80));
    indirect(JMP, PC = addr);
    absolute(JMP, PC = addr);
    zeropage(BIT, P.set(N, data & 0b1000'0000); P.set(V, data & 0b0100'0000); P.set(Z, !(A & data)));
    absolute(BIT, P.set(N, data & 0b1000'0000); P.set(V, data & 0b0100'0000); P.set(Z, !(A & data)));
    implied(NOP, );
    immediate(NOP, );
    zeropage(NOP, );
    zeropagex(NOP, );
    absolute(NOP, );
    absolutex(NOP, );
    // Store instructions
    #define prefetch();
    zeropage(STX, write(addr, X));
    zeropagey(STX, write(addr, X));
    absolute(STX, write(addr, X));
    zeropage(STY, write(addr, Y));
    zeropagex(STY, write(addr, Y));
    absolute(STY, write(addr, Y));
    all(STA, write(addr, A));
    zeropage(SAX, write(addr, A & X));
    zeropagey(SAX, write(addr, A & X));
    absolute(SAX, write(addr, A & X));
    indirectx(SAX, write(addr, A & X));
    zeropage(LSR, uint8_t temp = data >> 1; write(addr, temp); check_nz(temp);  P.set(C, data & 1));
    zeropagex(LSR, uint8_t temp = data >> 1; write(addr, temp); check_nz(temp);  P.set(C, data & 1));
    absolute(LSR, uint8_t temp = data >> 1; write(addr, temp); check_nz(temp);  P.set(C, data & 1));
    absolutex(LSR, uint8_t temp = data >> 1; write(addr, temp); check_nz(temp);  P.set(C, data & 1));
    zeropage(ASL, uint8_t temp = data << 1; write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    zeropagex(ASL, uint8_t temp = data << 1; write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    absolute(ASL, uint8_t temp = data << 1; write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    absolutex(ASL, uint8_t temp = data << 1; write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    zeropage(ROR, uint8_t temp = (data >> 1) | ((uint8_t)P.test(C) << 7); write(addr, temp); check_nz(temp); P.set(C, data & 1));
    zeropagex(ROR, uint8_t temp = (data >> 1) | ((uint8_t)P.test(C) << 7); write(addr, temp); check_nz(temp); P.set(C, data & 1));
    absolute(ROR, uint8_t temp = (data >> 1) | ((uint8_t)P.test(C) << 7); write(addr, temp); check_nz(temp); P.set(C, data & 1));
    absolutex(ROR, uint8_t temp = (data >> 1) | ((uint8_t)P.test(C) << 7); write(addr, temp); check_nz(temp); P.set(C, data & 1));
    zeropage(ROL, uint8_t temp = (data << 1) | P.test(C); write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    zeropagex(ROL, uint8_t temp = (data << 1) | P.test(C); write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    absolute(ROL, uint8_t temp = (data << 1) | P.test(C); write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    absolutex(ROL, uint8_t temp = (data << 1) | P.test(C); write(addr, temp); check_nz(temp); P.set(C, data & 0x80));
    zeropage(INC, uint8_t temp = data + 1; write(addr, temp); check_nz(temp));
    zeropagex(INC, uint8_t temp = data + 1; write(addr, temp); check_nz(temp));
    absolute(INC, uint8_t temp = data + 1; write(addr, temp); check_nz(temp));
    absolutex(INC, uint8_t temp = data + 1; write(addr, temp); check_nz(temp));
    zeropage(DEC, uint8_t temp = data - 1; write(addr, temp); check_nz(temp));
    zeropagex(DEC, uint8_t temp = data - 1; write(addr, temp); check_nz(temp));
    absolute(DEC, uint8_t temp = data - 1; write(addr, temp); check_nz(temp));
    absolutex(DEC, uint8_t temp = data - 1; write(addr, temp); check_nz(temp));
    all(DCP, data = data - 1; write(addr, data); check_nz(A - data); P.set(C, A >= data));
    all(ISC, data = data + 1; write(addr, data); SBC_impl);
    all(SLO, P.set(C, data & 0x80); data = data << 1; write(addr, data); A = A | data; check_nz(A));
    all(RLA, uint8_t old_c = P.test(C); P.set(C, data & 0x80); data = (data << 1) | old_c; write(addr, data); A = A & data; check_nz(A));
    all(RRA, uint8_t old_c = P.test(C); P.set(C, data & 1); data = (data >> 1) | (old_c << 7); write(addr, data); ADC_impl);
    all(SRE, P.set(C, data & 1); data = data >> 1; write(addr, data); A = A ^ data; check_nz(A));
    #undef prefetch

    CPU::CPU(CPUBus& bus) : bus_(bus) {};

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

    void CPU::RTI() {
        delay(1);
        P &= 0b0011'0000;
        P |= pull() & 0b1100'1111;
        auto pc_l = pull();
        auto pc_h = pull();
        PC = (pc_l | (pc_h << 8));
        delay(1);
    }

    void CPU::TAY() {
        delay(1);
        Y = A;
        check_nz(Y);
    }

    void CPU::TAX() {
        delay(1);
        X = A;
        check_nz(X);
    }

    void CPU::TSX() {
        delay(1);
        X = SP;
        check_nz(X);
    }

    void CPU::TYA() {
        delay(1);
        A = Y;
        check_nz(A);
    }

    void CPU::TXA() {
        delay(1);
        A = X;
        check_nz(A);
    }

    void CPU::TXS() {
        delay(1);
        SP = X;
    }

    void CPU::BCS() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(C)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BCC() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(C)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BEQ() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(Z)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BNE() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(Z)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BVS() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(V)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BVC() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(V)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BPL() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (!P.test(N)) {
            PC += int8_t(b1);
            delay(1);
            if ((old_pc & 0xFF00) != (PC & 0xFF00))
                delay(1); // page cross penalty
        }
    }

    void CPU::BMI() {
        auto b1 = read(PC++);
        auto old_pc = PC;
        if (P.test(N)) {
            PC += int8_t(b1);
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

    void CPU::DEY() {
        Y--;
        check_nz(Y);
    }

    void CPU::DEX() {
        X--;
        check_nz(X);
    }
    
    void CPU::NMI() {
        uint8_t p = P.to_ulong() | 0b0011'0000;
        push(p);
        push(PC >> 8);
        push(PC & 0xFF);
        uint16_t b1 = read(0xFFFB);
        uint16_t b2 = read(0xFFFA);
        uint16_t addr = b1 | (b2 << 8);
        PC = addr;
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
        for (int j = 0; j < i; j++)
            bus_.ppu_.Tick();
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
        #define ins(num, instr) case num: instr(); break
        switch (fetched_) {
            ins(0x01, ORA<IndirectX>);
            ins(0x03, SLO<IndirectX>);
            ins(0x04, NOP<ZeroPage>);
            ins(0x05, ORA<ZeroPage>);
            ins(0x06, ASL<ZeroPage>);
            ins(0x07, SLO<ZeroPage>);
            ins(0x08, PHP);
            ins(0x09, ORA<Immediate>);
            ins(0x0A, ASL<Accumulator>);
            ins(0x0C, NOP<Absolute>);
            ins(0x0D, ORA<Absolute>);
            ins(0x0E, ASL<Absolute>);
            ins(0x0F, SLO<Absolute>);
            ins(0x10, BPL);
            ins(0x11, ORA<IndirectY>);
            ins(0x13, SLO<IndirectY>);
            ins(0x14, NOP<ZeroPageX>);
            ins(0x15, ORA<ZeroPageX>);
            ins(0x16, ASL<ZeroPageX>);
            ins(0x17, SLO<ZeroPageX>);
            ins(0x18, CLC);
            ins(0x19, ORA<AbsoluteY>);
            ins(0x1A, NOP<Implied>);
            ins(0x1B, SLO<AbsoluteY>);
            ins(0x1C, NOP<AbsoluteX>);
            ins(0x1D, ORA<AbsoluteX>);
            ins(0x1E, ASL<AbsoluteX>);
            ins(0x1F, SLO<AbsoluteX>);
            ins(0x20, JSR);
            ins(0x21, AND<IndirectX>);
            ins(0x23, RLA<IndirectX>);
            ins(0x24, BIT<ZeroPage>);
            ins(0x25, AND<ZeroPage>);
            ins(0x26, ROL<ZeroPage>);
            ins(0x27, RLA<ZeroPage>);
            ins(0x28, PLP);
            ins(0x29, AND<Immediate>);
            ins(0x2A, ROL<Accumulator>);
            ins(0x2C, BIT<Absolute>);
            ins(0x2D, AND<Absolute>);
            ins(0x2E, ROL<Absolute>);
            ins(0x2F, RLA<Absolute>);
            ins(0x30, BMI);
            ins(0x31, AND<IndirectY>);
            ins(0x33, RLA<IndirectY>);
            ins(0x34, NOP<ZeroPageX>);
            ins(0x35, AND<ZeroPageX>);
            ins(0x36, ROL<ZeroPageX>);
            ins(0x37, RLA<ZeroPageX>);
            ins(0x38, SEC);
            ins(0x39, AND<AbsoluteY>);
            ins(0x3A, NOP<Implied>);
            ins(0x3B, RLA<AbsoluteY>);
            ins(0x3C, NOP<AbsoluteX>);
            ins(0x3D, AND<AbsoluteX>);
            ins(0x3E, ROL<AbsoluteX>);
            ins(0x3F, RLA<AbsoluteX>);
            ins(0x40, RTI);
            ins(0x41, EOR<IndirectX>);
            ins(0x43, SRE<IndirectX>);
            ins(0x44, NOP<ZeroPage>);
            ins(0x45, EOR<ZeroPage>);
            ins(0x46, LSR<ZeroPage>);
            ins(0x47, SRE<ZeroPage>);
            ins(0x48, PHA);
            ins(0x49, EOR<Immediate>);
            ins(0x4A, LSR<Accumulator>);
            ins(0x4C, JMP<Absolute>);
            ins(0x4D, EOR<Absolute>);
            ins(0x4E, LSR<Absolute>);
            ins(0x4F, SRE<Absolute>);
            ins(0x50, BVC);
            ins(0x51, EOR<IndirectY>);
            ins(0x53, SRE<IndirectY>);
            ins(0x54, NOP<ZeroPageX>);
            ins(0x55, EOR<ZeroPageX>);
            ins(0x56, LSR<ZeroPageX>);
            ins(0x57, SRE<ZeroPageX>);
            ins(0x59, EOR<AbsoluteY>);
            ins(0x5A, NOP<Implied>);
            ins(0x5B, SRE<AbsoluteY>);
            ins(0x5C, NOP<AbsoluteX>);
            ins(0x5D, EOR<AbsoluteX>);
            ins(0x5E, LSR<AbsoluteX>);
            ins(0x5F, SRE<AbsoluteX>);
            ins(0x60, RTS);
            ins(0x61, ADC<IndirectX>);
            ins(0x63, RRA<IndirectX>);
            ins(0x64, NOP<ZeroPage>);
            ins(0x65, ADC<ZeroPage>);
            ins(0x66, ROR<ZeroPage>);
            ins(0x67, RRA<ZeroPage>);
            ins(0x68, PLA);
            ins(0x69, ADC<Immediate>);
            ins(0x6A, ROR<Accumulator>);
            ins(0x6C, JMP<Indirect>);
            ins(0x6D, ADC<Absolute>);
            ins(0x6E, ROR<Absolute>);
            ins(0x6F, RRA<Absolute>);
            ins(0x70, BVS);
            ins(0x71, ADC<IndirectY>);
            ins(0x73, RRA<IndirectY>);
            ins(0x74, NOP<ZeroPageX>);
            ins(0x75, ADC<ZeroPageX>);
            ins(0x76, ROR<ZeroPageX>);
            ins(0x77, RRA<ZeroPageX>);
            ins(0x78, SEI);
            ins(0x79, ADC<AbsoluteY>);
            ins(0x7A, NOP<Implied>);
            ins(0x7B, RRA<AbsoluteY>);
            ins(0x7C, NOP<AbsoluteX>);
            ins(0x7D, ADC<AbsoluteX>);
            ins(0x7E, ROR<AbsoluteX>);
            ins(0x7F, RRA<AbsoluteX>);
            ins(0x80, NOP<Immediate>);
            ins(0x81, STA<IndirectX>);
            ins(0x82, NOP<Immediate>);
            ins(0x83, SAX<IndirectX>);
            ins(0x84, STY<ZeroPage>);
            ins(0x85, STA<ZeroPage>);
            ins(0x86, STX<ZeroPage>);
            ins(0x87, SAX<ZeroPage>);
            ins(0x88, DEY);
            ins(0x89, NOP<Immediate>);
            ins(0x8A, TXA);
            ins(0x8C, STY<Absolute>);
            ins(0x8D, STA<Absolute>);
            ins(0x8E, STX<Absolute>);
            ins(0x8F, SAX<Absolute>);
            ins(0x90, BCC);
            ins(0x91, STA<IndirectY>);
            ins(0x94, STY<ZeroPageX>);
            ins(0x95, STA<ZeroPageX>);
            ins(0x96, STX<ZeroPageY>);
            ins(0x97, SAX<ZeroPageY>);
            ins(0x98, TYA);
            ins(0x99, STA<AbsoluteY>);
            ins(0x9A, TXS);
            ins(0x9D, STA<AbsoluteX>);
            ins(0xA0, LDY<Immediate>);
            ins(0xA1, LDA<IndirectX>);
            ins(0xA2, LDX<Immediate>);
            ins(0xA3, LAX<IndirectX>);
            ins(0xA4, LDY<ZeroPage>);
            ins(0xA5, LDA<ZeroPage>);
            ins(0xA6, LDX<ZeroPage>);
            ins(0xA7, LAX<ZeroPage>);
            ins(0xA8, TAY);
            ins(0xA9, LDA<Immediate>);
            ins(0xAA, TAX);
            ins(0xAC, LDY<Absolute>);
            ins(0xAD, LDA<Absolute>);
            ins(0xAE, LDX<Absolute>);
            ins(0xAF, LAX<Absolute>);
            ins(0xB0, BCS);
            ins(0xB1, LDA<IndirectY>);
            ins(0xB3, LAX<IndirectY>);
            ins(0xB4, LDY<ZeroPageX>);
            ins(0xB5, LDA<ZeroPageX>);
            ins(0xB6, LDX<ZeroPageY>);
            ins(0xB7, LAX<ZeroPageY>);
            ins(0xB8, CLV);
            ins(0xB9, LDA<AbsoluteY>);
            ins(0xBA, TSX);
            ins(0xBC, LDY<AbsoluteX>);
            ins(0xBD, LDA<AbsoluteX>);
            ins(0xBE, LDX<AbsoluteY>);
            ins(0xBF, LAX<AbsoluteY>);
            ins(0xC0, CPY<Immediate>);
            ins(0xC1, CMP<IndirectX>);
            ins(0xC2, NOP<Immediate>);
            ins(0xC3, DCP<IndirectX>);
            ins(0xC4, CPY<ZeroPage>);
            ins(0xC5, CMP<ZeroPage>);
            ins(0xC6, DEC<ZeroPage>);
            ins(0xC7, DCP<ZeroPage>);
            ins(0xC8, INY);
            ins(0xC9, CMP<Immediate>);
            ins(0xCA, DEX);
            ins(0xCC, CPY<Absolute>);
            ins(0xCD, CMP<Absolute>);
            ins(0xCE, DEC<Absolute>);
            ins(0xCF, DCP<Absolute>);
            ins(0xD0, BNE);
            ins(0xD1, CMP<IndirectY>);
            ins(0xD3, DCP<IndirectY>);
            ins(0xD4, NOP<ZeroPageX>);
            ins(0xD5, CMP<ZeroPageX>);
            ins(0xD6, DEC<ZeroPageX>);
            ins(0xD7, DCP<ZeroPageX>);
            ins(0xD8, CLD);
            ins(0xD9, CMP<AbsoluteY>);
            ins(0xDA, NOP<Implied>);
            ins(0xDB, DCP<AbsoluteY>);
            ins(0xDC, NOP<AbsoluteX>);
            ins(0xDD, CMP<AbsoluteX>);
            ins(0xDE, DEC<AbsoluteX>);
            ins(0xDF, DCP<AbsoluteX>);
            ins(0xE0, CPX<Immediate>);
            ins(0xE1, SBC<IndirectX>);
            ins(0xE2, NOP<Immediate>);
            ins(0xE3, ISC<IndirectX>);
            ins(0xE4, CPX<ZeroPage>);
            ins(0xE5, SBC<ZeroPage>);
            ins(0xE6, INC<ZeroPage>);
            ins(0xE7, ISC<ZeroPage>);
            ins(0xE8, INX);
            ins(0xE9, SBC<Immediate>);
            ins(0xEA, NOP<Implied>);
            ins(0xEB, SBC<Immediate>);
            ins(0xEC, CPX<Absolute>);
            ins(0xED, SBC<Absolute>);
            ins(0xEE, INC<Absolute>);
            ins(0xEF, ISC<Absolute>);
            ins(0xF0, BEQ);
            ins(0xF1, SBC<IndirectY>);
            ins(0xF3, ISC<IndirectY>);
            ins(0xF4, NOP<ZeroPageX>);
            ins(0xF5, SBC<ZeroPageX>);
            ins(0xF6, INC<ZeroPageX>);
            ins(0xF7, ISC<ZeroPageX>);
            ins(0xF8, SED);
            ins(0xF9, SBC<AbsoluteY>);
            ins(0xFA, NOP<Implied>);
            ins(0xFB, ISC<AbsoluteY>);
            ins(0xFC, NOP<AbsoluteX>);
            ins(0xFD, SBC<AbsoluteX>);
            ins(0xFE, INC<AbsoluteX>);
            ins(0xFF, ISC<AbsoluteX>);
            default: throw std::runtime_error(std::string("Unimplemented instr:") + std::to_string(fetched_));
        }
        #undef ins
    }

    void CPU::Tick() {
        fetch();
        execute();
    }
    
    void CPU::Reset() {
        PC = (read_no_d(0xFFFD) << 8) | read_no_d(0xFFFC);
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