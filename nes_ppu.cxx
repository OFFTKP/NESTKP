#include "nes_ppu.hxx"
#include <iostream>

enum ScanlineState {
    NT_BYTE_LOW = 0,
    NT_BYTE_HIGH = 1,
    AT_BYTE_LOW = 2,
    AT_BYTE_HIGH = 3,
    LOW_BG_BYTE_LOW = 4,
    LOW_BG_BYTE_HIGH = 5,
    HIGH_BG_BYTE_LOW = 6,
    HIGH_BG_BYTE_HIGH = 7,
};
namespace TKPEmu::NES::Devices {
    void PPU::SetNMI(std::function<void(void)> func) {
        fire_nmi = std::move(func);
    }
    void PPU::Tick() {
        if (scanline_ <= 239) {
            handle_normal_scanline();
            scanline_cycle_++;
        } else if (scanline_ == 240) {
            handle_empty_scanline();
            scanline_cycle_++;
        } else if (scanline_ <= 260) {
            ppu_status_ |= 0x80;
            if (nmi_enabled_) {
                nmi_enabled_ = false;
                fire_nmi();
            }
            handle_empty_scanline();
            scanline_cycle_++;
        } else if (scanline_ == 261) {
            ppu_status_ &= ~0x80;
            scanline_ = 0;
            cur_y_ = 0;
        }
    }

    void PPU::handle_normal_scanline() {
        if (scanline_cycle_ == 0) {
            // NOP
        } else if (scanline_cycle_ <= 256) {
            execute_pipeline();
            draw_pixel();
            pixel_cycle_++;
            if (pixel_cycle_ == 8)
                pixel_cycle_ = 0;
        } else if (scanline_cycle_ < 321) {

        } else if (scanline_cycle_ < 336) {
            execute_pipeline();
        } else if (scanline_cycle_ < 340) {

        } else {
            scanline_++;
            scanline_cycle_ = 0;
            cur_y_++;
            cur_x_ = 0;
        }
    }

    void PPU::handle_empty_scanline() {
        if (scanline_cycle_ == 340) {
            scanline_++;
            scanline_cycle_ = 0;
            cur_y_++;
            cur_x_ = 0;
        }
    }

    uint8_t* PPU::GetScreenData() {
        return screen_color_data_.data();
    }

    bool& PPU::IsReadyToDraw() {
        return should_draw_;
    }

    void PPU::execute_pipeline() {
        switch (pixel_cycle_) {
            case NT_BYTE_HIGH:
            nt_latch_ = fetch_nt();
            break;
            case AT_BYTE_HIGH:
            at_latch_ = fetch_at();
            break;
            case LOW_BG_BYTE_HIGH:
            pt_low_latch_ = fetch_pt_low();
            break;
            case HIGH_BG_BYTE_HIGH:
            pt_high_latch_ = fetch_pt_high();
            piso_bg_high_ = pt_high_latch_;
            piso_bg_low_ = pt_low_latch_;
            break;
            default:
            break;
        }
    }

    void PPU::invalidate(uint8_t addr, uint8_t data) {
        switch (addr & 0b111) {
            case 0b000: {
                ppu_ctrl_ = data;
                nt_addr_ = 0x2000 + (ppu_ctrl_ & 0b11) * 0x400;
                vram_incr_vertical_ = ppu_ctrl_ & 0b100;
                sprite_pattern_address_ = !!(ppu_ctrl_ & 0b1000) * 0x1000;
                background_pattern_address_ = !!(ppu_ctrl_ & 0b1'0000) * 0x1000;
                sprite_size_ = ppu_ctrl_ & 0b10'0000;
                ppu_master_ = ppu_ctrl_ & 0b100'0000;
                nmi_enabled_ = ppu_ctrl_ & 0b1000'0000;
                std::cout << "ppu ctrl:" << std::hex << (int)ppu_ctrl_ << std::endl;
                break;
            }
            case 0b001: {
                ppu_mask_ = data;
                break;
            }
            case 0b010: {
                ppu_status_ = data;
                break;
            }
            case 0b011: {
                oam_addr_ = data;
                break;
            }
            case 0b100: {
                oam_data_ = data;
                break;
            }
            case 0b101: {
                ppu_scroll_ = data;
                break;
            }
            case 0b110: {
                vram_addr_ <<= 8;
                vram_addr_ |= data;
                // std::cout << "new addr: " << std::hex << vram_addr_ << std::endl;
                break;
            }
            case 0b111: {
                vram_[vram_addr_ & 0x7FF] = data;
                vram_addr_ += 1 + 31 * vram_incr_vertical_;
                break;
            }
        }
    }

    uint8_t PPU::fetch_nt() {
        auto tile_x = (scanline_cycle_ - 1) / 8;
        auto tile_y = cur_y_ / 8;
        return vram_[(nt_addr_ + tile_x + tile_y * 32) & 0x7FF];
    }
    
    uint8_t PPU::fetch_pt_low() {
        auto addr = nt_latch_;
        return read(addr * 16 + (scanline_ & 0b111));//test_tile_.at(0 + (cur_y_ & 0b111));
    }

    uint8_t PPU::fetch_pt_high() {
        uint16_t addr = nt_latch_;
        return read(addr * 16 + (scanline_ & 0b111));//test_tile_.at(8 + (cur_y_ & 0b111));
    }

    void PPU::draw_pixel() {
        uint8_t bg_cur = piso_bg_high_ | piso_bg_low_;
        auto pixel = scanline_cycle_ - 1;
        screen_color_data_.at(pixel * 4 + cur_y_ * 256 * 4) = !!(bg_cur & 0x80) * 255;
        screen_color_data_.at(pixel * 4 + cur_y_ * 256 * 4 + 1) = !!(bg_cur & 0x80) * 255;
        screen_color_data_.at(pixel * 4 + cur_y_ * 256 * 4 + 2) = !!(bg_cur & 0x80) * 255;
        screen_color_data_.at(pixel * 4 + cur_y_ * 256 * 4 + 3) = 255;
        piso_bg_low_ <<= 1;
        piso_bg_high_ <<= 1;
        should_draw_ = true;
    }

    uint8_t PPU::read(uint16_t addr) {
        if (addr < 0x2000)
            return chr_rom_[addr];
        else if (addr < 0x3000)
            return vram_[addr & 0x7FF];
        return 0;
    }
    
    void PPU::Reset() {
        cur_x_ = 0;
        cur_y_ = 0;
        scanline_ = 0;
        scanline_cycle_ = 0;
        for (int i = 0; i < 256 * 240 * 4; i += 4) {
            screen_color_data_ [i] = 255;
            screen_color_data_ [i + 1] = 0;
            screen_color_data_ [i + 2] = 0;
            screen_color_data_ [i + 3] = 255;
        }
        should_draw_ = true;
        vram_.fill(0);
    }
}