#include "nes_cpubus.hxx"
#include <include/error_factory.hxx>
#include <fstream>
#include <iostream>
#define kb16 0x4000
#define kb8 0x2000

namespace TKPEmu::NES::Devices {
    CPUBus::CPUBus(PPU& ppu, APU& apu) : ppu_(ppu), apu_(apu) {}
    bool CPUBus::LoadCartridge(std::string path) {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (ifs.is_open()) {
            ifs.unsetf(std::ios::skipws);
            ifs.seekg(0, std::ios::end);
            std::streampos size = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            ifs.read(reinterpret_cast<char*>(&header_), sizeof(Header));
            if (!(header_.id[0] == 'N' && header_.id[1] == 'E' && header_.id[2] == 'S' && header_.id[3] == '\032'))
                return false;
            bool has_trainer = header_.flags_6 & 0b100;
            if (has_trainer)
                ifs.read(reinterpret_cast<char*>(trainer_.data()), sizeof(trainer_));
            // TODO: exponent size
            auto prg_rom_size = ((header_.prg_chr_msb & 0b1111) << 8) | header_.prg_lsb;
            prg_rom_.resize(prg_rom_size * kb16);
            // Fill with illegal opcodes so that we can't run from a bad area without knowing
            std::fill(prg_rom_.begin(), prg_rom_.end(), 2);
            ifs.read(reinterpret_cast<char*>(prg_rom_.data()), prg_rom_size * kb16);
            auto chr_rom_size = ((header_.prg_chr_msb >> 4) << 8) | header_.chr_lsb;
            if (chr_rom_size == 0) {
                ppu_.chr_rom_.resize(kb8);
            } else {
                ppu_.chr_rom_.resize(chr_rom_size * kb8);
                ifs.read(reinterpret_cast<char*>(ppu_.chr_rom_.data()), chr_rom_size * kb8);
            }
            refill_prg_map();
            ram_.fill(0);
        } else {
            return false;
        }
        return true;
    }

    uint8_t CPUBus::read(uint16_t addr) {
        uint8_t* page = fast_map_.at(addr >> 8);
        uint8_t ret;
        if (page)
            ret = *(page + (addr & 0xFF));
        else
            ret = redirect_address_r(addr);
        last_read_ = ret;
        return ret;
    }

    uint8_t CPUBus::redirect_address_r(uint16_t addr) {
        if (addr >= 0x2000 && addr <= 0x3FFF) {
            switch (addr & 0b111) {
                case 0b000: return ppu_.ppu_ctrl_;
                case 0b001: return ppu_.ppu_mask_;
                case 0b010: {
                    auto temp = ppu_.ppu_status_;
                    ppu_.ppu_status_ &= ~0x80;
                    return temp;
                }
                case 0b011: return ppu_.oam_addr_;
                case 0b100: return ppu_.oam_data_;
                case 0b101: return ppu_.ppu_scroll_;
                case 0b110: return ppu_.vram_addr_;
                case 0b111: return ppu_.ppu_data_;
            }
        }
        return last_read_;
    }

    void CPUBus::redirect_address_w(uint16_t addr, uint8_t data) {
        if (addr >= 0x2000 && addr <= 0x3FFF) {
            return ppu_.invalidate(addr & 0b111, data);
        } else if (addr >= 0x4000 && addr <= 0x4017) {
            return apu_.invalidate(addr & 0b11111, data);
        }
    }

    void CPUBus::write(uint16_t addr, uint8_t data) {
        uint8_t* page = fast_map_.at(addr >> 8);
        if (page) {
            *(page + (addr & 0xFF)) = data;
        } else {
            redirect_address_w(addr, data);
        }
    }

    void CPUBus::refill_prg_map() {
        fast_map_.fill(0);
        for (uint16_t i = 0x00; i < 0x08; i++) {
            fast_map_.at(i) = &ram_.at(i << 8);
        }
        switch (mapper_) {
            case MAPPER_NROM: {
                for (uint16_t i = 0x80; i <= 0xBF; i++)
                    fast_map_.at(i) = &prg_rom_.at((i << 8) - 0x8000);
                if (prg_rom_.size() == kb16 * 2) {
                    for (uint16_t i = 0xC0; i <= 0xFF; i++)
                        fast_map_.at(i) = &prg_rom_.at((i << 8) - 0x8000);
                } else {
                    for (uint16_t i = 0xC0; i <= 0xFF; i++)
                        fast_map_.at(i) = &prg_rom_.at((i << 8) - 0xC000);
                }
                break;
            }
            default: throw ErrorFactory::generate_exception(__func__, __LINE__, "Unknown mapper");
        }
    }

    void CPUBus::Reset() {
        
    }
}