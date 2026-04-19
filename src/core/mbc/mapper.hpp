#pragma once
#include "common/types.hpp"
#include <vector>

// Abstract base for all MBC / mapper implementations
class Mapper {
public:
    virtual ~Mapper() = default;

    // ROM bank 0 (0x0000-0x3FFF) - often fixed
    virtual u8   ReadROM0(u16 addr) = 0;
    // ROM bank N (0x4000-0x7FFF)
    virtual u8   ReadROMN(u16 addr) = 0;
    // External RAM (0xA000-0xBFFF)
    virtual u8   ReadSRAM(u16 addr) = 0;
    virtual void WriteROM(u16 addr, u8 val) = 0;
    virtual void WriteSRAM(u16 addr, u8 val) = 0;

    // Access raw SRAM for persistence
    virtual std::vector<u8>& SRAM() = 0;

protected:
    std::vector<u8>* m_rom{nullptr};
    std::vector<u8>* m_sram{nullptr};
};
