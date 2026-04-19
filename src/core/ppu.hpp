#pragma once
#include "common/types.hpp"
#include <array>
#include <functional>
#include <vector>

class CPU;
class MMU;

class PPU {
public:
    PPU() = default;

    void Init(CPU* cpu, MMU* mmu);

    void Step(u32 tcycles);

    u8   ReadVRAM(u16 addr);
    void WriteVRAM(u16 addr, u8 val);

    u8   ReadReg(u8 reg);
    void WriteReg(u8 reg, u8 val);

    // FrameBuffer RGBA8888 160x144
    const u32* GetFrameBuffer() const { return m_frameBuffer.data(); }
    bool       FrameReady() const { return m_frameReady; }
    void       ClearFrameReady() { m_frameReady = false; }

private:
    CPU* m_cpu{nullptr};
    MMU* m_mmu{nullptr};

    std::array<u8, 0x4000> m_vram{}; // 16KB (2 banks for CGB, 1 for DMG)
    std::array<u32, 160 * 144> m_frameBuffer{};
    bool m_frameReady{false};

    u8 m_vramBank{0};

    // Registers
    u8 m_lcdc{0x91}; // FF40
    u8 m_stat{0x85}; // FF41
    u8 m_scy{0};     // FF42
    u8 m_scx{0};     // FF43
    u8 m_ly{0};      // FF44
    u8 m_lyc{0};     // FF45
    u8 m_bgp{0xFC};  // FF47
    u8 m_obp0{0xFF}; // FF48
    u8 m_obp1{0xFF}; // FF49
    u8 m_wy{0};      // FF4A
    u8 m_wx{0};      // FF4B

    // CGB Registers
    u8 m_bgpi{0}; // FF68
    u8 m_obpi{0}; // FF6A
    std::array<u8, 64> m_bgCram{};
    std::array<u8, 64> m_obCram{};

    u32 m_dots{0};
    u16 m_windowLine{0};

    void ChangeMode(u8 mode);
    void CheckSTATInterrupt();

    void RenderScanline();
    void RenderBackgroundWindow(u8* scanlineInfo);
    void RenderSprites(u8* scanlineInfo);
};
