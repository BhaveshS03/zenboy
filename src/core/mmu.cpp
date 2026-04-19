#include "mmu.hpp"
#include "cartridge.hpp"
#include "ppu.hpp"
#include "apu.hpp"
#include "timer.hpp"
#include "joypad.hpp"
#include "serial.hpp"
#include "common/log.hpp"
#include <cstring>

void MMU::Init(Cartridge* cart, PPU* ppu, APU* apu, Timer* timer,
               Joypad* joypad, Serial* serial)
{
    m_cart   = cart;
    m_ppu    = ppu;
    m_apu    = apu;
    m_timer  = timer;
    m_joypad = joypad;
    m_serial = serial;
    wram.fill(0x00);
    oam.fill(0x00);
    hram.fill(0x00);
    ie   = 0x00;
    intf = 0x00;
}

u8 MMU::Read(u16 addr) {
    // DMA active: only HRAM accessible from CPU
    if (dmaBusy && addr < 0xFF80) return 0xFF;

    if (addr < 0x8000) return m_cart ? m_cart->Read(addr) : 0xFF;  // ROM
    if (addr < 0xA000) return m_ppu  ? m_ppu->ReadVRAM(addr)  : 0xFF; // VRAM
    if (addr < 0xC000) return m_cart ? m_cart->Read(addr) : 0xFF;  // Ext RAM
    if (addr < 0xD000) return wram[addr - 0xC000];                  // WRAM bank 0
    if (addr < 0xE000) return wram[0x1000 * wramBank + (addr - 0xD000)]; // WRAM bank 1-7
    if (addr < 0xFE00) return wram[(addr - 0xE000) & 0x1FFF];      // Echo RAM
    if (addr < 0xFEA0) return oam[addr - 0xFE00];                  // OAM
    if (addr < 0xFF00) return 0xFF;                                  // Unusable
    if (addr < 0xFF80) return ReadIO(static_cast<u8>(addr & 0xFF)); // I/O
    if (addr < 0xFFFF) return hram[addr - 0xFF80];                  // HRAM
    return ie;                                                        // IE
}

void MMU::Write(u16 addr, u8 val) {
    if (dmaBusy && addr < 0xFF80) return;

    if (addr < 0x8000) { if (m_cart) m_cart->Write(addr, val); return; }
    if (addr < 0xA000) { if (m_ppu)  m_ppu->WriteVRAM(addr, val); return; }
    if (addr < 0xC000) { if (m_cart) m_cart->Write(addr, val); return; }
    if (addr < 0xD000) { wram[addr - 0xC000] = val; return; }
    if (addr < 0xE000) { wram[0x1000 * wramBank + (addr - 0xD000)] = val; return; }
    if (addr < 0xFE00) { wram[(addr - 0xE000) & 0x1FFF] = val; return; }
    if (addr < 0xFEA0) { oam[addr - 0xFE00] = val; return; }
    if (addr < 0xFF00) return; // Unusable, ignore
    if (addr < 0xFF80) { WriteIO(static_cast<u8>(addr & 0xFF), val); return; }
    if (addr < 0xFFFF) { hram[addr - 0xFF80] = val; return; }
    ie = val;
}

u8 MMU::ReadIO(u8 reg) {
    switch (reg) {
        case 0x00: return m_joypad ? m_joypad->Read() : 0xFF;
        case 0x01: return m_serial ? m_serial->ReadSB() : 0xFF;
        case 0x02: return m_serial ? m_serial->ReadSC() : 0xFF;
        case 0x04: return m_timer  ? m_timer->ReadDIV() : 0xFF;
        case 0x05: return m_timer  ? m_timer->ReadTIMA() : 0xFF;
        case 0x06: return m_timer  ? m_timer->ReadTMA() : 0xFF;
        case 0x07: return m_timer  ? m_timer->ReadTAC() : 0xFF;
        case 0x0F: return intf | 0xE0;  // upper 3 bits always 1
        // PPU registers
        case 0x40: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x41: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x42: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x43: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x44: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x45: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x46: return 0xFF; // DMA is write-only source reg
        case 0x47: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x48: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x49: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x4A: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x4B: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        // CGB PPU regs
        case 0x4F: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x68: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x69: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x6A: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        case 0x6B: return m_ppu ? m_ppu->ReadReg(reg) : 0xFF;
        // APU
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
        case 0x16: case 0x17: case 0x18: case 0x19:
        case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
        case 0x20: case 0x21: case 0x22: case 0x23:
        case 0x24: case 0x25: case 0x26:
            return m_apu ? m_apu->ReadReg(reg) : 0xFF;
        case 0x50: return 0xFF; // Boot ROM disable – write-only behavior
        default:   return 0xFF;
    }
}

void MMU::WriteIO(u8 reg, u8 val) {
    switch (reg) {
        case 0x00: if (m_joypad) m_joypad->Write(val); break;
        case 0x01: if (m_serial) m_serial->WriteSB(val); break;
        case 0x02: if (m_serial) m_serial->WriteSC(val); break;
        case 0x04: if (m_timer)  m_timer->WriteDIV(); break;
        case 0x05: if (m_timer)  m_timer->WriteTIMA(val); break;
        case 0x06: if (m_timer)  m_timer->WriteTMA(val); break;
        case 0x07: if (m_timer)  m_timer->WriteTAC(val); break;
        case 0x0F: intf = val & 0x1F; break;
        // PPU
        case 0x40: case 0x41: case 0x42: case 0x43:
        case 0x45: case 0x47: case 0x48: case 0x49:
        case 0x4A: case 0x4B: case 0x4F: case 0x68:
        case 0x69: case 0x6A: case 0x6B:
            if (m_ppu) m_ppu->WriteReg(reg, val);
            break;
        case 0x44: break; // LY is read-only
        case 0x46: {      // OAM DMA trigger
            dmaSource = static_cast<u16>(val) << 8;
            dmaBusy   = true;
            break;
        }
        // APU
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
        case 0x16: case 0x17: case 0x18: case 0x19:
        case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
        case 0x20: case 0x21: case 0x22: case 0x23:
        case 0x24: case 0x25: case 0x26:
            if (m_apu) m_apu->WriteReg(reg, val);
            break;
        // Wave RAM 0x30-0x3F
        default:
            if (reg >= 0x30 && reg <= 0x3F) {
                if (m_apu) m_apu->WriteReg(reg, val);
            }
            break;
    }
}
