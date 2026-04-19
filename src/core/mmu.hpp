#pragma once
#include "common/types.hpp"
#include <array>
#include <functional>

// Forward declarations
class Cartridge;
class PPU;
class APU;
class Timer;
class Joypad;
class Serial;

class MMU {
public:
    MMU() = default;

    void Init(Cartridge* cart, PPU* ppu, APU* apu, Timer* timer,
              Joypad* joypad, Serial* serial);

    u8   Read(u16 addr);
    void Write(u16 addr, u8 val);

    // Direct access helpers (bypass bus timings for DMA/HDMA)
    u8   DirectRead(u16 addr) {
        bool prev = dmaBusy;
        dmaBusy = false;
        u8 val = Read(addr);
        dmaBusy = prev;
        return val;
    }
    void DirectWrite(u16 addr, u8 val) {
        bool prev = dmaBusy;
        dmaBusy = false;
        Write(addr, val);
        dmaBusy = prev;
    }

    // Memory regions (public for save states / DMA access)
    std::array<u8, 0x8000> wram{};     // 32KB (banks 0-7, CGB)
    std::array<u8, 0x0200> oam{};      // 256 bytes (FE00-FEFF)
    std::array<u8, 0x007F> hram{};     // FF80-FFFE
    u8  ie{0x00};                       // FFFF
    u8  intf{0x00};                     // FF0F  (IF)

    // WRAM bank (CGB – bank 1-7 in 0xD000-0xDFFF)
    u8   wramBank{1};

    // DMA active flag – blocks CPU from accessing memory except HRAM
    bool dmaBusy{false};
    // current OAM DMA source
    u16  dmaSource{0};

private:
    Cartridge* m_cart{nullptr};
    PPU*       m_ppu{nullptr};
    APU*       m_apu{nullptr};
    Timer*     m_timer{nullptr};
    Joypad*    m_joypad{nullptr};
    Serial*    m_serial{nullptr};

    u8   ReadIO(u8 reg);
    void WriteIO(u8 reg, u8 val);
};
