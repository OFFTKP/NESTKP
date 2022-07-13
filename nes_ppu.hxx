#ifndef TKP_NES_PPU_H
#define TKP_NES_PPU_H
#include <cstdint>
#include <array>
#include <functional>

namespace TKPEmu::NES::Devices {
    class CPUBus;
    class PPU {
    public:
        void Tick();
        void Reset();
        uint8_t* GetScreenData();
        bool& IsReadyToDraw();
        void SetNMI(std::function<void()> func);
    private:
        bool should_draw_ = false;
        uint8_t ppu_ctrl_, ppu_mask_, ppu_status_, oam_addr_,
            oam_data_, ppu_scroll_, ppu_data_, oam_dma_;
        uint16_t vram_addr_ = 0;
        int scanline_ = 0;
        int scanline_cycle_ = 0;
        int pixel_cycle_ = 0;
        uint8_t nt_latch_ = 0;
        uint8_t at_latch_ = 0;
        uint8_t pt_low_latch_ = 0;
        uint8_t pt_high_latch_ = 0;
        uint16_t piso_bg_low_ = 0;
        uint16_t piso_bg_high_ = 0;
        uint16_t nt_addr_ = 0;
        uint16_t cur_x_ = 0;
        uint16_t cur_y_ = 0;
        bool vram_incr_vertical_ = false;
        uint16_t sprite_pattern_address_ = 0;
        uint16_t background_pattern_address_ = 0;
        bool sprite_size_ = false;
        bool ppu_master_ = false;
        bool nmi_enabled_ = false;
        std::array<uint8_t, 0x800> vram_;
        std::array<uint8_t, 256 * 240 * 4> screen_color_data_;
        __always_inline void handle_normal_scanline();
        __always_inline void handle_empty_scanline();
        __always_inline uint8_t fetch_nt();
        __always_inline uint8_t fetch_at(){ return 0;};
        __always_inline uint8_t fetch_pt_low();
        __always_inline uint8_t fetch_pt_high();
        __always_inline void execute_pipeline();
        __always_inline void draw_pixel();
        __always_inline uint8_t read(uint16_t addr);
        void invalidate(uint8_t addr, uint8_t data);
        std::vector<uint8_t> chr_rom_;
        std::function<void()> fire_nmi;
        friend class CPUBus;
    };
}
#endif