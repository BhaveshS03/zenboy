#pragma once
#include "mapper.hpp"
#include "common/types.hpp"

// ROM Only: 32KB, no banking
class MapperROM : public Mapper {
public:
    MapperROM(std::vector<u8>& rom, std::vector<u8>& sram) {
        m_rom = &rom;
        m_sram = &sram;
    }

    u8 ReadROM0(u16 addr) override {
        return (*m_rom)[addr & 0x7FFF];
    }
    u8 ReadROMN(u16 addr) override {
        return (*m_rom)[addr & 0x7FFF];
    }
    u8 ReadSRAM(u16) override { return 0xFF; }
    void WriteROM(u16, u8) override {}
    void WriteSRAM(u16, u8) override {}
    std::vector<u8>& SRAM() override { return *m_sram; }
};

// MBC1: up to 2MB ROM / 32KB RAM
class MapperMBC1 : public Mapper {
public:
    MapperMBC1(std::vector<u8>& rom, std::vector<u8>& sram, u32 numRomBanks, u32 numRamBanks)
        : m_numRomBanks(numRomBanks), m_numRamBanks(numRamBanks)
    {
        m_rom  = &rom;
        m_sram = &sram;
    }

    u8 ReadROM0(u16 addr) override {
        // In Mode 1, bank 0 can map to $20, $40, $60
        u32 bank = (m_mode == 1) ? ((m_ramRomUpper & 0x03) << 5) : 0;
        bank %= m_numRomBanks;
        return (*m_rom)[(static_cast<u32>(bank) * 0x4000) + addr];
    }

    u8 ReadROMN(u16 addr) override {
        u32 bank = (m_ramRomUpper << 5) | m_romBankLow;
        if ((bank & 0x1F) == 0) bank++;          // $00 -> $01 emulation
        bank %= m_numRomBanks;
        return (*m_rom)[(static_cast<u32>(bank) * 0x4000) + (addr - 0x4000)];
    }

    u8 ReadSRAM(u16 addr) override {
        if (!m_ramEnabled || m_numRamBanks == 0) return 0xFF;
        u32 bank = (m_mode == 1) ? (m_ramRomUpper & 0x03) : 0;
        bank %= m_numRamBanks;
        u32 offset = bank * 0x2000 + (addr - 0xA000);
        if (offset < m_sram->size()) return (*m_sram)[offset];
        return 0xFF;
    }

    void WriteROM(u16 addr, u8 val) override {
        if (addr < 0x2000)      m_ramEnabled   = (val & 0x0F) == 0x0A;
        else if (addr < 0x4000) m_romBankLow   = val & 0x1F;
        else if (addr < 0x6000) m_ramRomUpper  = val & 0x03;
        else                    m_mode          = val & 0x01;
    }

    void WriteSRAM(u16 addr, u8 val) override {
        if (!m_ramEnabled || m_numRamBanks == 0) return;
        u32 bank = (m_mode == 1) ? (m_ramRomUpper & 0x03) : 0;
        bank %= m_numRamBanks;
        u32 offset = bank * 0x2000 + (addr - 0xA000);
        if (offset < m_sram->size()) (*m_sram)[offset] = val;
    }

    std::vector<u8>& SRAM() override { return *m_sram; }

private:
    u32 m_numRomBanks{2};
    u32 m_numRamBanks{0};
    u32 m_romBankLow{1};
    u32 m_ramRomUpper{0};
    u32 m_mode{0};
    bool m_ramEnabled{false};
};

// MBC2: 256KB ROM / 512x4bit internal RAM
class MapperMBC2 : public Mapper {
public:
    MapperMBC2(std::vector<u8>& rom, std::vector<u8>& sram, u32 numRomBanks)
        : m_numRomBanks(numRomBanks)
    {
        m_rom  = &rom;
        m_sram = &sram;
        if (m_sram->size() < 512) m_sram->resize(512, 0xFF);
    }

    u8 ReadROM0(u16 addr) override { return (*m_rom)[addr]; }

    u8 ReadROMN(u16 addr) override {
        u32 bank = m_romBank % m_numRomBanks;
        if (bank == 0) bank = 1;
        return (*m_rom)[bank * 0x4000 + (addr - 0x4000)];
    }

    u8 ReadSRAM(u16 addr) override {
        if (!m_ramEnabled) return 0xFF;
        u16 offset = (addr - 0xA000) & 0x01FF;
        return (*m_sram)[offset] | 0xF0; // upper nibble always 1
    }

    void WriteROM(u16 addr, u8 val) override {
        if (addr > 0x3FFF) return;
        bool bit8 = (addr >> 8) & 1;
        if (!bit8) m_ramEnabled = (val & 0x0F) == 0x0A;
        else       m_romBank    = val & 0x0F;
    }

    void WriteSRAM(u16 addr, u8 val) override {
        if (!m_ramEnabled) return;
        u16 offset = (addr - 0xA000) & 0x01FF;
        (*m_sram)[offset] = val & 0x0F;
    }

    std::vector<u8>& SRAM() override { return *m_sram; }

private:
    u32  m_numRomBanks{2};
    u32  m_romBank{1};
    bool m_ramEnabled{false};
};

// MBC3: up to 2MB ROM / 32KB RAM + optional RTC
class MapperMBC3 : public Mapper {
public:
    MapperMBC3(std::vector<u8>& rom, std::vector<u8>& sram, u32 numRomBanks, u32 numRamBanks)
        : m_numRomBanks(numRomBanks), m_numRamBanks(numRamBanks)
    {
        m_rom  = &rom;
        m_sram = &sram;
    }

    u8 ReadROM0(u16 addr) override { return (*m_rom)[addr]; }

    u8 ReadROMN(u16 addr) override {
        u32 bank = m_romBank % m_numRomBanks;
        if (bank == 0) bank = 1;
        return (*m_rom)[bank * 0x4000 + (addr - 0x4000)];
    }

    u8 ReadSRAM(u16 addr) override {
        if (!m_ramTimerEnabled) return 0xFF;
        if (m_ramBank <= 0x03 && m_numRamBanks > 0) {
            u32 bank   = m_ramBank % m_numRamBanks;
            u32 offset = bank * 0x2000 + (addr - 0xA000);
            if (offset < m_sram->size()) return (*m_sram)[offset];
        } else if (m_ramBank >= 0x08 && m_ramBank <= 0x0C) {
            // RTC registers (stubbed)
            return m_rtcRegs[m_ramBank - 0x08];
        }
        return 0xFF;
    }

    void WriteROM(u16 addr, u8 val) override {
        if (addr < 0x2000)      m_ramTimerEnabled = (val & 0x0F) == 0x0A;
        else if (addr < 0x4000) { m_romBank = val & 0x7F; }
        else if (addr < 0x6000) m_ramBank   = val;
        else                    m_latch      = (m_lastLatch == 0x00 && val == 0x01);
        if (addr >= 0x4000 && addr < 0x6000) m_lastLatch = val;
    }

    void WriteSRAM(u16 addr, u8 val) override {
        if (!m_ramTimerEnabled) return;
        if (m_ramBank <= 0x03 && m_numRamBanks > 0) {
            u32 bank   = m_ramBank % m_numRamBanks;
            u32 offset = bank * 0x2000 + (addr - 0xA000);
            if (offset < m_sram->size()) (*m_sram)[offset] = val;
        } else if (m_ramBank >= 0x08 && m_ramBank <= 0x0C) {
            m_rtcRegs[m_ramBank - 0x08] = val;
        }
    }

    std::vector<u8>& SRAM() override { return *m_sram; }

private:
    u32  m_numRomBanks{2};
    u32  m_numRamBanks{0};
    u32  m_romBank{1};
    u32  m_ramBank{0};
    bool m_ramTimerEnabled{false};
    bool m_latch{false};
    u8   m_lastLatch{0xFF};
    u8   m_rtcRegs[5]{};   // S, M, H, DL, DH
};

// MBC5: up to 8MB ROM / 128KB RAM
class MapperMBC5 : public Mapper {
public:
    MapperMBC5(std::vector<u8>& rom, std::vector<u8>& sram, u32 numRomBanks, u32 numRamBanks)
        : m_numRomBanks(numRomBanks), m_numRamBanks(numRamBanks)
    {
        m_rom  = &rom;
        m_sram = &sram;
    }

    u8 ReadROM0(u16 addr) override { return (*m_rom)[addr]; }

    u8 ReadROMN(u16 addr) override {
        u32 bank = m_romBank % m_numRomBanks;
        return (*m_rom)[bank * 0x4000 + (addr - 0x4000)];
    }

    u8 ReadSRAM(u16 addr) override {
        if (!m_ramEnabled || m_numRamBanks == 0) return 0xFF;
        u32 bank   = m_ramBank % m_numRamBanks;
        u32 offset = bank * 0x2000 + (addr - 0xA000);
        if (offset < m_sram->size()) return (*m_sram)[offset];
        return 0xFF;
    }

    void WriteROM(u16 addr, u8 val) override {
        if (addr < 0x2000)      m_ramEnabled      = (val & 0x0F) == 0x0A;
        else if (addr < 0x3000) m_romBank          = (m_romBank & 0x100) | val;
        else if (addr < 0x4000) m_romBank          = (m_romBank & 0x0FF) | ((val & 0x01) << 8);
        else if (addr < 0x6000) m_ramBank          = val & 0x0F;
    }

    void WriteSRAM(u16 addr, u8 val) override {
        if (!m_ramEnabled || m_numRamBanks == 0) return;
        u32 bank   = m_ramBank % m_numRamBanks;
        u32 offset = bank * 0x2000 + (addr - 0xA000);
        if (offset < m_sram->size()) (*m_sram)[offset] = val;
    }

    std::vector<u8>& SRAM() override { return *m_sram; }

private:
    u32  m_numRomBanks{2};
    u32  m_numRamBanks{0};
    u32  m_romBank{0};   // MBC5 starts at bank 0, no $00->$01 quirk
    u32  m_ramBank{0};
    bool m_ramEnabled{false};
};
