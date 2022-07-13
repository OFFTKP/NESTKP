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
    PPU::PPU(std::mutex& draw_mutex) : draw_mutex_(draw_mutex) {}
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
                // std::cout << "set vbl: " << std::hex << master_clock_dbg_ << std::endl;
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
                ppu_status_ &= ~0x80;
            }
            handle_empty_scanline();
            scanline_cycle_++;
        } else if (scanline_ == 262) {
            scanline_ = 0;
            cur_y_ = 0;
        }
        master_clock_dbg_++;
    }

    void PPU::handle_normal_scanline() {
        if (scanline_cycle_ == 0) {
            // NOP
        } else if (scanline_cycle_ <= 256) {
            fetch_x_ = (scanline_cycle_ + 8 - 1) / 8;
            fetch_y_ = cur_y_ / 8;
            execute_pipeline();
            draw_pixel();
            pixel_cycle_++;
            if (pixel_cycle_ == 8)
                pixel_cycle_ = 0;
        } else if (scanline_cycle_ < 321) {

        } else if (scanline_cycle_ <= 336) {
            fetch_x_ = 0;
            fetch_y_ = (cur_y_ + 1) / 8;
            execute_pipeline();
            pixel_cycle_++;
            if (pixel_cycle_ == 8)
                pixel_cycle_ = 0;
        } else if (scanline_cycle_ <= 340) {

        } else {
            scanline_++;
            scanline_cycle_ = 0;
            cur_y_++;
        }
    }

    void PPU::handle_empty_scanline() {
        if (scanline_cycle_ == 340) {
            scanline_++;
            scanline_cycle_ = 0;
            cur_y_++;
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
                nmi_output_ = ppu_ctrl_ & 0b1000'0000;
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
    
    uint8_t PPU::fetch_pt_low() {
        uint16_t addr = nt_latch_ * 16 + (scanline_ & 0b111);
        return read(background_pattern_address_ + addr);
    }

    uint8_t PPU::fetch_pt_high() {
        uint16_t addr = nt_latch_ * 16 + (scanline_ & 0b111);
        return read(background_pattern_address_ + 8 + addr);
    }

    void PPU::draw_pixel() {
        uint8_t bg_cur = piso_bg_high_ | piso_bg_low_;
        auto pixel = cur_x_ * 4 + cur_y_ * 256 * 4;//scanline_cycle_ - 1;
        screen_color_data_second_.at(pixel) = !!(bg_cur & 0x80) * 255;
        screen_color_data_second_.at(pixel + 1) = !!(bg_cur & 0x80) * 255;
        screen_color_data_second_.at(pixel + 2) = !!(bg_cur & 0x80) * 255;
        screen_color_data_second_.at(pixel + 3) = 255;
        cur_x_++;
        if (cur_x_ == 256)
            cur_x_ = 0;
        piso_bg_low_ <<= 1;
        piso_bg_high_ <<= 1;
    }

    uint8_t PPU::read(uint16_t addr) {
        if (addr < 0x2000)
            return chr_rom_[addr & 0x1FFF];
        else if (addr < 0x3000)
            return vram_[addr & 0x7FF];
        return 0;
    }

    void PPU::write(uint16_t addr, uint8_t data) {
        if (addr < 0x2000) {
            chr_rom_.at(addr) = data;
        } else if (addr < 0x3000)
            vram_.at(addr & 0x7FF) = data;
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