#include "cartridge.hpp"
#include "mbc/mapper.hpp"
#include "mbc/all_mappers.hpp"
#include "common/log.hpp"
#include <fstream>
#include <cstring>

// ROM size table: code -> number of 16KB banks
static const u32 kRomBankTable[] = {
    2, 4, 8, 16, 32, 64, 128, 256, 512
};

// RAM size table: code -> bytes
static const u32 kRamSizeTable[] = {
    0, 0, 8*1024, 32*1024, 128*1024, 64*1024
};

Cartridge::Cartridge() = default;
Cartridge::~Cartridge() = default;

bool Cartridge::Load(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) {
        Log::Error("Cannot open ROM: %s", path.c_str());
        return false;
    }
    auto sz = f.tellg();
    f.seekg(0);
    m_rom.resize(static_cast<size_t>(sz));
    f.read(reinterpret_cast<char*>(m_rom.data()), sz);

    if (!ParseHeader()) return false;
    if (!ValidateHeaderChecksum()) {
        Log::Warn("Header checksum mismatch – continuing anyway.");
    }

    // Determine RAM size
    u32 ramBytes = 0;
    if (m_header.ramSizeCode < 6) ramBytes = kRamSizeTable[m_header.ramSizeCode];
    // MBC2 has its own 512-byte RAM allocated by the mapper
    m_sram.assign(ramBytes, 0xFF);

    u32 numRomBanks = 2;
    if (m_header.romSizeCode < 9) numRomBanks = kRomBankTable[m_header.romSizeCode];
    u32 numRamBanks = ramBytes / 0x2000;

    Log::Info("Loaded: %s  Cart=$%02X  ROM=%u banks  RAM=%u bytes",
              m_header.title, m_header.cartType, numRomBanks, ramBytes);

    switch (m_header.cartType) {
        case 0x00:
            m_mapper = std::make_unique<MapperROM>(m_rom, m_sram);
            break;
        case 0x01: case 0x02: case 0x03:
            m_mapper = std::make_unique<MapperMBC1>(m_rom, m_sram, numRomBanks, numRamBanks);
            break;
        case 0x05: case 0x06:
            m_mapper = std::make_unique<MapperMBC2>(m_rom, m_sram, numRomBanks);
            break;
        case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
            m_mapper = std::make_unique<MapperMBC3>(m_rom, m_sram, numRomBanks, numRamBanks);
            break;
        case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
            m_mapper = std::make_unique<MapperMBC5>(m_rom, m_sram, numRomBanks, numRamBanks);
            break;
        default:
            Log::Warn("Unknown cart type $%02X – using ROM only mapper", m_header.cartType);
            m_mapper = std::make_unique<MapperROM>(m_rom, m_sram);
            break;
    }

    m_loaded = true;
    return true;
}

bool Cartridge::ParseHeader() {
    if (m_rom.size() < 0x0150) {
        Log::Error("ROM too small to contain header");
        return false;
    }
    size_t titleLen = 0;
    for (int i = 0; i < 16; ++i) {
        u8 c = m_rom[0x0134 + i];
        if (c == 0) break;
        m_header.title[titleLen++] = c;
    }
    m_header.title[titleLen] = '\0';
    m_header.cartType    = m_rom[0x0147];
    m_header.romSizeCode = m_rom[0x0148];
    m_header.ramSizeCode = m_rom[0x0149];
    m_header.isCGB       = (m_rom[0x0143] == 0x80 || m_rom[0x0143] == 0xC0);
    m_header.isSGB       = (m_rom[0x0146] == 0x03);
    m_header.headerChecksum = m_rom[0x014D];
    return true;
}

bool Cartridge::ValidateHeaderChecksum() {
    u8 checksum = 0;
    for (u16 i = 0x0134; i <= 0x014C; ++i)
        checksum = checksum - m_rom[i] - 1;
    return checksum == m_header.headerChecksum;
}

u8 Cartridge::Read(u16 addr) {
    if (!m_mapper) return 0xFF;
    if (addr < 0x4000) return m_mapper->ReadROM0(addr);
    if (addr < 0x8000) return m_mapper->ReadROMN(addr);
    if (addr >= 0xA000 && addr < 0xC000) return m_mapper->ReadSRAM(addr);
    return 0xFF;
}

void Cartridge::Write(u16 addr, u8 val) {
    if (!m_mapper) return;
    if (addr < 0x8000) { m_mapper->WriteROM(addr, val); return; }
    if (addr >= 0xA000 && addr < 0xC000) m_mapper->WriteSRAM(addr, val);
}

void Cartridge::SaveSRAM(const std::string& savePath) {
    if (!m_mapper) return;
    auto& sram = m_mapper->SRAM();
    if (sram.empty()) return;
    std::ofstream f(savePath, std::ios::binary);
    if (f) {
        f.write(reinterpret_cast<const char*>(sram.data()), static_cast<std::streamsize>(sram.size()));
        Log::Info("SRAM saved to %s", savePath.c_str());
    }
}

void Cartridge::LoadSRAM(const std::string& savePath) {
    if (!m_mapper) return;
    auto& sram = m_mapper->SRAM();
    if (sram.empty()) return;
    std::ifstream f(savePath, std::ios::binary);
    if (f) {
        f.read(reinterpret_cast<char*>(sram.data()), static_cast<std::streamsize>(sram.size()));
        Log::Info("SRAM loaded from %s", savePath.c_str());
    }
}
