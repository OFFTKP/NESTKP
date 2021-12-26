#include "nes_cpu.h"
namespace TKPEmu::NES::Devices {
    TickRet NES::tick() {
        // Instruction queue splits the instruction to necessary parts
        // so that later NES update function will run PPU, APU and other updates
        // in the middle of the instruction
        instruction_queue_.push([](){
            // Instruction fetching time
            return static_cast<FuncRet>(1);
        });
        uint8_t opcode = read(PC);
        const Instruction& instr = instructions_[opcode];
        // Handle addressing mode
        instruction_queue_.push(get_addr_mode(instr.addr));
        return 0;
    }
    FuncRetPtr NES::get_addr_mode(AddressingMode addr) {
        switch (addr) {

        }
        return [](){ return static_cast<FuncRet>(0); };
    }
    FuncRet NES::read(uint16_t addr) {
        return 0;
    }
}