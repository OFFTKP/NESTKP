#include "nes_cpubus.hxx"
#include <include/error_factory.hxx>
#include <fstream>
#define kb16 16384

namespace TKPEmu::NES::Devices {
    bool Bus::LoadCartridge(std::string path) {
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
            ifs.read(reinterpret_cast<char*>(prg_rom_.data()), prg_rom_size * kb16);
            auto chr_rom_size = ((header_.prg_chr_msb >> 4) << 8) | header_.chr_lsb;
            chr_rom_.resize(chr_rom_size * kb16);
            ifs.read(reinterpret_cast<char*>(chr_rom_.data()), chr_rom_size * kb16);
            refill_prg_map();
        } else {
            return false;
        }
        return true;
    }

    uint8_t Bus::read(uint16_t addr) {
        uint8_t* page = fast_map_[addr >> 8];
        if (page)
            return *(page + (addr & 0xFF));
        else
            return 0xFF;
    }

    void Bus::write(uint16_t addr, uint8_t data) {

    }

    void Bus::refill_prg_map() {
        switch (mapper_) {
            case MAPPER_NROM: {
                for (uint16_t i = 0x80; i <= 0xBF; i++)
                    fast_map_[i] = &prg_rom_[(i << 8) - 0x8000];
                for (uint16_t i = 0xC0; i <= 0xFF; i++)
                    fast_map_[i] = &prg_rom_[(i << 8) - 0xC000];
                break;
            }
        }
    }

    void Bus::Reset() {
        
    }
}