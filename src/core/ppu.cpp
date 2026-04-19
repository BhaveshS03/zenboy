#include "ppu.hpp"
#include "cpu.hpp"
#include "mmu.hpp"
#include <algorithm>

static const u32 DMG_PALETTE[4] = {
    0xFFE0F8D0, 0xFF88C070, 0xFF346856, 0xFF081820
};

void PPU::Init(CPU* cpu, MMU* mmu) {
    m_cpu = cpu;
    m_mmu = mmu;
    m_vram.fill(0);
    m_frameBuffer.fill(0xFFE0F8D0);
    m_frameReady = false;
    m_dots = 0;
    m_windowLine = 0;
    m_vramBank = 0;
}

u8 PPU::ReadVRAM(u16 addr) {
    if ((m_lcdc & 0x80) && (m_stat & 0x03) == 3) return 0xFF; // Blocked during mode 3 (pixel transfer)
    u16 offset = addr - 0x8000;
    return m_vram[offset + (m_vramBank * 0x2000)];
}

void PPU::WriteVRAM(u16 addr, u8 val) {
    if ((m_lcdc & 0x80) && (m_stat & 0x03) == 3) return;
    u16 offset = addr - 0x8000;
    m_vram[offset + (m_vramBank * 0x2000)] = val;
}

u8 PPU::ReadReg(u8 reg) {
    switch (reg) {
        case 0x40: return m_lcdc;
        case 0x41: return m_stat | 0x80;
        case 0x42: return m_scy;
        case 0x43: return m_scx;
        case 0x44: return m_ly;
        case 0x45: return m_lyc;
        case 0x47: return m_bgp;
        case 0x48: return m_obp0;
        case 0x49: return m_obp1;
        case 0x4A: return m_wy;
        case 0x4B: return m_wx;
        case 0x4F: return m_vramBank | 0xFE;
        case 0x68: return m_bgpi;
        case 0x69: return m_bgCram[m_bgpi & 0x3F];
        case 0x6A: return m_obpi;
        case 0x6B: return m_obCram[m_obpi & 0x3F];
    }
    return 0xFF;
}

void PPU::WriteReg(u8 reg, u8 val) {
    switch (reg) {
        case 0x40: {
            bool wasOn = (m_lcdc & 0x80) != 0;
            m_lcdc = val;
            bool isOn = (m_lcdc & 0x80) != 0;
            if (wasOn && !isOn) {
                m_ly = 0;
                m_dots = 0;
                ChangeMode(0);
            }
            break;
        }
        case 0x41: m_stat = (val & 0x78) | (m_stat & 0x07); break; // lower 3 bits read-only
        case 0x42: m_scy = val; break;
        case 0x43: m_scx = val; break;
        case 0x45: m_lyc = val; break;
        case 0x47: m_bgp = val; break;
        case 0x48: m_obp0 = val; break;
        case 0x49: m_obp1 = val; break;
        case 0x4A: m_wy = val; break;
        case 0x4B: m_wx = val; break;
        case 0x4F: m_vramBank = val & 1; break;
        case 0x68: m_bgpi = val; break;
        case 0x69: {
            m_bgCram[m_bgpi & 0x3F] = val;
            if (m_bgpi & 0x80) m_bgpi = (m_bgpi & 0x80) | ((m_bgpi + 1) & 0x3F);
            break;
        }
        case 0x6A: m_obpi = val; break;
        case 0x6B: {
            m_obCram[m_obpi & 0x3F] = val;
            if (m_obpi & 0x80) m_obpi = (m_obpi & 0x80) | ((m_obpi + 1) & 0x3F);
            break;
        }
    }
}

void PPU::ChangeMode(u8 mode) {
    m_stat = (m_stat & ~0x03) | (mode & 0x03);
}

void PPU::CheckSTATInterrupt() {
    bool interrupt = false;
    u8 mode = m_stat & 0x03;
    if ((m_stat & 0x40) && m_ly == m_lyc) interrupt = true;
    if ((m_stat & 0x20) && mode == 2) interrupt = true;
    if ((m_stat & 0x10) && mode == 1) interrupt = true;
    if ((m_stat & 0x08) && mode == 0) interrupt = true;

    if (interrupt) m_cpu->RequestInterrupt(1); // STAT Interrupt
}

void PPU::Step(u32 tcycles) {
    if (!(m_lcdc & 0x80)) return;

    m_dots += tcycles;

    u8 currentMode = m_stat & 0x03;
    bool lycUpdate = false;

    if (m_ly < 144) {
        if (m_dots < 80) {
            if (currentMode != 2) { ChangeMode(2); CheckSTATInterrupt(); }
        } else if (m_dots < 80 + 172) {
            if (currentMode != 3) { ChangeMode(3); }
        } else if (m_dots < 456) {
            if (currentMode != 0) {
                ChangeMode(0);
                CheckSTATInterrupt();
                RenderScanline();
            }
        } else {
            m_dots -= 456;
            m_ly++;
            lycUpdate = true;
            if (m_ly == 144) {
                ChangeMode(1);
                m_cpu->RequestInterrupt(0); // VBLANK
                CheckSTATInterrupt();
                m_frameReady = true;
                m_windowLine = 0;
            } else {
                ChangeMode(2);
                CheckSTATInterrupt();
            }
        }
    } else { // VBLANK lines (144-153)
        if (m_dots >= 456) {
            m_dots -= 456;
            m_ly++;
            lycUpdate = true;
            if (m_ly > 153) {
                m_ly = 0;
                ChangeMode(2);
                CheckSTATInterrupt();
            }
        }
    }

    if (lycUpdate) {
        if (m_ly == m_lyc) {
            m_stat |= 0x04;
            if (m_stat & 0x40) m_cpu->RequestInterrupt(1);
        } else {
            m_stat &= ~0x04;
        }
    }
}

void PPU::RenderScanline() {
    u8 scanlineColorIds[160] = {0}; // Track BG color id for object priority (0 means transparent BG)
    RenderBackgroundWindow(scanlineColorIds);
    RenderSprites(scanlineColorIds);
}

void PPU::RenderBackgroundWindow(u8* scanlineInfo) {
    if (!(m_lcdc & 0x01)) { // BG enable
        for (int i = 0; i < 160; i++) {
            m_frameBuffer[m_ly * 160 + i] = DMG_PALETTE[0];
            scanlineInfo[i] = 0;
        }
        return;
    }

    bool drawWindow = (m_lcdc & 0x20) && m_wy <= m_ly && m_wx <= 166;
    u16 bgMapSpace = (m_lcdc & 0x08) ? 0x9C00 : 0x9800;
    u16 winMapSpace = (m_lcdc & 0x40) ? 0x9C00 : 0x9800;
    u16 tileDataSpace = (m_lcdc & 0x10) ? 0x8000 : 0x8800; // 0x8800 uses signed relative addressing

    u8 yPos = m_scy + m_ly;
    u16 mapRowOffs = (yPos / 8) * 32;

    u8 winY = drawWindow ? m_windowLine : 0;
    u16 winRowOffs = (winY / 8) * 32;

    for (int p = 0; p < 160; p++) {
        bool inWindow = drawWindow && p >= (m_wx - 7);
        u8 mapX = inWindow ? (p - (m_wx - 7)) : (m_scx + p);
        u8 mapY = inWindow ? winY : yPos;
        u16 mapAddr = (inWindow ? winMapSpace : bgMapSpace) + (inWindow ? winRowOffs : mapRowOffs) + (mapX / 8);

        u8 tileNum = m_vram[mapAddr - 0x8000];
        u16 tileAddr = 0;
        if (m_lcdc & 0x10) tileAddr = tileDataSpace + (tileNum * 16);
        else tileAddr = tileDataSpace + ((s8)tileNum + 128) * 16;

        u8 line = (mapY % 8) * 2;
        u8 data1 = m_vram[tileAddr - 0x8000 + line];
        u8 data2 = m_vram[tileAddr - 0x8000 + line + 1];

        int colorBit = 7 - (mapX % 8);
        u8 colorId = ((data2 >> colorBit) & 1) << 1 | ((data1 >> colorBit) & 1);
        scanlineInfo[p] = colorId;

        u8 color = (m_bgp >> (colorId * 2)) & 3;
        m_frameBuffer[m_ly * 160 + p] = DMG_PALETTE[color];
    }

    if (drawWindow) m_windowLine++;
}

void PPU::RenderSprites(u8* scanlineInfo) {
    if (!(m_lcdc & 0x02)) return;

    bool objSize16 = (m_lcdc & 0x04) != 0;
    u8 spriteHeight = objSize16 ? 16 : 8;

    int spritesDrawn = 0;
    for (int i = 0; i < 40; i++) {
        u8 oamY = m_mmu->oam[i * 4];
        u8 oamX = m_mmu->oam[i * 4 + 1];
        u8 tile = m_mmu->oam[i * 4 + 2];
        u8 attr = m_mmu->oam[i * 4 + 3];

        int yPos = oamY - 16;
        int xPos = oamX - 8;

        if (m_ly >= yPos && m_ly < yPos + spriteHeight) {
            if (spritesDrawn++ >= 10) break; // At most 10 sprites per line

            int line = m_ly - yPos;
            if (attr & 0x40) line = spriteHeight - 1 - line; // Y-flip

            if (objSize16) tile &= 0xFE;

            u16 tileAddr = 0x8000 + (tile * 16) + (line * 2);
            u8 data1 = m_vram[tileAddr - 0x8000];
            u8 data2 = m_vram[tileAddr - 0x8000 + 1];

            for (int objP = 0; objP < 8; objP++) {
                int px = xPos + objP;
                if (px < 0 || px >= 160) continue;

                int colorBit = (attr & 0x20) ? objP : 7 - objP; // X-flip
                u8 colorId = ((data2 >> colorBit) & 1) << 1 | ((data1 >> colorBit) & 1);
                if (colorId == 0) continue; // transparent

                // Priority: if attr bit 7 is set and BG is not 0 (transparent), BG draws over sprite.
                if ((attr & 0x80) && scanlineInfo[px] != 0) continue;

                u8 pal = (attr & 0x10) ? m_obp1 : m_obp0;
                u8 color = (pal >> (colorId * 2)) & 3;

                m_frameBuffer[m_ly * 160 + px] = DMG_PALETTE[color];
            }
        }
    }
}
