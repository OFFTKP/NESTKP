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
    PPU::PPU(std::mutex& draw_mutex) : draw_mutex_(draw_mutex), master_palette_{{
        { 84, 84, 84 }, { 0, 30, 116 }, { 8, 16, 144 }, { 48, 0, 136 }, { 68, 0, 100 }, { 92, 0, 48 }, { 84, 4, 0 }, { 60, 24, 0 }, { 32, 42, 0 }, { 8, 58, 0 }, { 0, 64, 0 }, { 0, 60, 0 }, { 0, 50, 60 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 152, 150, 152 }, { 8, 76, 196 }, { 48, 50, 236 }, { 92, 30, 228 }, { 136, 20, 176 }, { 160, 20, 100 }, { 152, 34, 32 }, { 120, 60, 0 }, { 84, 90, 0 }, { 40, 114, 0 }, { 8, 124, 0 }, { 0, 118, 40 }, { 0, 102, 120 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 236, 238, 236 }, { 76, 154, 236 }, { 120, 124, 236 }, { 176, 98, 236 }, { 228, 84, 236 }, { 236, 88, 180 }, { 236, 106, 100 }, { 212, 136, 32 }, { 160, 170, 0 }, { 116, 196, 0 }, { 76, 208, 32 }, { 56, 204, 108 }, { 56, 180, 204 }, { 60, 60, 60 }, { 0, 0, 0 }, { 0, 0, 0 },
        { 236, 238, 236 }, { 168, 204, 236 }, { 188, 188, 236 }, { 212, 178, 236 }, { 236, 174, 236 }, { 236, 174, 212 }, { 236, 180, 176 }, { 228, 196, 144 }, { 204, 210, 120 }, { 180, 222, 120 }, { 168, 226, 144 }, { 152, 226, 180 }, { 160, 214, 228 }, { 160, 162, 160 }, { 0, 0, 0 }, { 0, 0, 0 },
    }}{
    }
    void PPU::SetNMI(std::function<void(void)> func) {
        fire_nmi = std::move(func);
    }
    void PPU::Tick() {
        static bool vblank_first = false;
        if (scanline_ <= 239) {
            handle_normal_scanline();
            scanline_cycle_++;
        } else if (scanline_ == 240) {
            handle_empty_scanline();
            scanline_cycle_++;
        } else if (scanline_ <= 260) {
            if (scanline_ == 241 && scanline_cycle_ == 1) {
                ppu_status_ |= 0x80;
                std::lock_guard lg(draw_mutex_);
                std::swap(screen_color_data_, screen_color_data_second_);
                should_draw_ = true;
            }
            if (nmi_output_) {
                nmi_output_ = false;
                fire_nmi();
            }
            handle_empty_scanline();
            scanline_cycle_++;
        } else if (scanline_ == 261) {
            if (scanline_cycle_ == 1) {
                fetch_x_ = 0;
                ppu_status_ &= ~0x80;
            }
            handle_prerender_scanline();
            scanline_cycle_++;
        }
        master_clock_dbg_++;
    }

    void PPU::handle_normal_scanline() {
        if (scanline_cycle_ == 0) {
            cur_x_ = 0;
        } else if (scanline_cycle_ <= 256) {
            fetch_x_ = 2 + (scanline_cycle_ - 1) / 8;
            fetch_y_ = cur_y_ / 8;
            draw_pixel();
            execute_pipeline();
            pixel_cycle_++;
            if (pixel_cycle_ == 8)
                pixel_cycle_ = 0;
        } else if (scanline_cycle_ <= 320) {
            if (scanline_cycle_ == 257) {
                piso_bg_high_ = 0;
                piso_bg_low_ = 0;
                fetch_x_ = 0;
                scanline_++;
                cur_y_++;
            }
        } else if (scanline_cycle_ <= 336) {
            fetch_y_ = cur_y_ / 8;
            execute_pipeline();
            pixel_cycle_++;
            if (pixel_cycle_ == 8) {
                pixel_cycle_ = 0;
                if (scanline_cycle_ == 328) {
                    fetch_x_ += 1;
                    piso_bg_high_ <<= 8;
                    piso_bg_low_ <<= 8;
                }
            }
        } else if (scanline_cycle_ <= 340) {

        } else {
            scanline_cycle_ = -1;
        }
    }

    void PPU::handle_empty_scanline() {
        if (scanline_cycle_ == 340) {
            scanline_++;
            scanline_cycle_ = 0;
            cur_y_++;
        }
    }

    void PPU::handle_prerender_scanline() {
        if (scanline_cycle_ >= 321 && scanline_cycle_ <= 336) {
            fetch_y_ = 0;
            scanline_ = 0;
            execute_pipeline();
            scanline_ = 261;
            pixel_cycle_++;
            if (pixel_cycle_ == 8) {
                pixel_cycle_ = 0;
                if (scanline_cycle_ == 328) {
                    fetch_x_ += 1;
                    piso_bg_high_ <<= 8;
                    piso_bg_low_ <<= 8;
                }
            }
        } else if (scanline_cycle_ == 340) {
            scanline_ = 0;
            cur_y_ = 0;
            scanline_cycle_ = -1;
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
            piso_bg_high_ |= pt_high_latch_;
            piso_bg_low_ |= pt_low_latch_;
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
                nmi_output_ = ppu_ctrl_ & 0b1000'0000;
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
                break;
            }
            case 0b111: {
                write(vram_addr_, data);
                vram_addr_ += 1 + 31 * vram_incr_vertical_;
                break;
            }
        }
    }

    uint8_t PPU::fetch_nt() {
        auto addr = (nt_addr_ + fetch_x_ + fetch_y_ * 32) & 0x7FF;
        return vram_.at(addr);
    }

    uint8_t PPU::fetch_at() {
        auto addr = (nt_addr_ & 0x7FF) + (fetch_x_ / 4) + (fetch_y_ / 4) * 8 + 0x3C0;
        return vram_.at(addr);
    }
    
    uint8_t PPU::fetch_pt_low() {
        uint16_t addr = nt_latch_ * 16 + (scanline_ & 0b111);
        return read(background_pattern_address_ + addr);
    }

    uint8_t PPU::fetch_pt_high() {
        uint16_t addr = nt_latch_ * 16 + (scanline_ & 0b111);
        return read(background_pattern_address_ + 8 + addr);
    }

    void PPU::draw_pixel() {
        uint8_t bg_cur = ((piso_bg_high_ >> 15) << 1) | (piso_bg_low_ >> 15);
        auto pixel = cur_x_ * 4 + cur_y_ * 256 * 4;
        uint8_t left_shift = !!(fetch_x_ & 0b10) * 2;
        uint8_t top_shift = (fetch_y_ & 0b10000) * 4;
        auto pal_index = (at_latch_ >> left_shift) & 0b11;
        screen_color_data_second_.at(pixel) = background_palettes_[pal_index][bg_cur][0];
        screen_color_data_second_.at(pixel + 1) = background_palettes_[pal_index][bg_cur][1];
        screen_color_data_second_.at(pixel + 2) = background_palettes_[pal_index][bg_cur][2];
        screen_color_data_second_.at(pixel + 3) = 255;
        cur_x_++;
        piso_bg_low_ <<= 1;
        piso_bg_high_ <<= 1;
    }

    uint8_t PPU::read(uint16_t addr) {
        if (addr < 0x2000)
            return chr_rom_.at(addr & 0x1FFF);
        else if (addr < 0x3000)
            return vram_.at(addr & 0x7FF);
        return 0;
    }

    void PPU::write(uint16_t addr, uint8_t data) {
        if (addr < 0x2000) {
            chr_rom_.at(addr) = data;
        } else if (addr < 0x3000) {
            vram_.at(addr & 0x7FF) = data;
        } else if (addr > 0x3F00 && addr < 0x3F20) {
            data &= 0x3F;
            addr &= 0xFF;
            switch (addr) {
                case 0x00:
                case 0x10: {
                    universal_bg_[0] = master_palette_[data][0];
                    universal_bg_[1] = master_palette_[data][1];
                    universal_bg_[2] = master_palette_[data][2];
                    break;
                }
                case 0x1: case 0x2: case 0x3:
                case 0x5: case 0x6: case 0x7:
                case 0x9: case 0xA: case 0xB:
                case 0xD: case 0xE: case 0xF: {
                    auto cur = addr / 4;
                    auto col = (addr) & 0b11;
                    background_palettes_.at(cur).at(col).at(0) = master_palette_[data][0];
                    background_palettes_.at(cur).at(col).at(1) = master_palette_[data][1];
                    background_palettes_.at(cur).at(col).at(2) = master_palette_[data][2];
                    std::cout << "pal[" << cur << "]" << ", col " << col << ": #" << std::hex << (int)master_palette_[data][0] << "" << (int)master_palette_[data][1] << "" << (int)master_palette_[data][2] << std::endl;
                    break;
                }
                case 0x11: case 0x12: case 0x13:
                case 0x15: case 0x16: case 0x17:
                case 0x19: case 0x1A: case 0x1B:
                case 0x1D: case 0x1E: case 0x1F: {
                    auto cur = (addr & 0xF) / 4;
                    auto col = (addr) & 0b11;
                    sprite_palettes_.at(cur).at(col).at(0) = master_palette_[data][0];
                    sprite_palettes_.at(cur).at(col).at(1) = master_palette_[data][1];
                    sprite_palettes_.at(cur).at(col).at(2) = master_palette_[data][2];
                    break;
                }
            }
        }
    }
    
    void PPU::Reset() {
        cur_x_ = 0;
        cur_y_ = 0;
        scanline_ = 0;
        scanline_cycle_ = 0;
        pixel_cycle_ = 0;
        for (int i = 0; i < 256 * 240 * 4; i += 4) {
            screen_color_data_ [i] = 0;
            screen_color_data_ [i + 1] = 0;
            screen_color_data_ [i + 2] = 0;
            screen_color_data_ [i + 3] = 255;
        }
        should_draw_ = true;
        vram_.fill(0);
    }
}