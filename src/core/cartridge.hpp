#pragma once
#include "common/types.hpp"
#include <vector>
#include <string>
#include <memory>

// Forward declare mapper base
class Mapper;

struct CartridgeHeader {
    u8  title[17]{};         // 0134-0143 + null terminator
    u8  cartType{};          // 0147
    u8  romSizeCode{};       // 0148
    u8  ramSizeCode{};       // 0149
    u8  headerChecksum{};    // 014D
    bool isCGB{};
    bool isSGB{};
};

class Cartridge {
public:
    Cartridge();
    ~Cartridge();
    bool Load(const std::string& path);

    // ROM/RAM access routed by mapper
    u8   Read(u16 addr);
    void Write(u16 addr, u8 val);

    // Save/load external RAM
    void SaveSRAM(const std::string& savePath);
    void LoadSRAM(const std::string& savePath);

    const CartridgeHeader& Header() const { return m_header; }
    bool IsLoaded() const { return m_loaded; }

    // Raw ROM for validation
    const std::vector<u8>& ROM() const { return m_rom; }

private:
    bool ParseHeader();
    bool ValidateHeaderChecksum();

    std::vector<u8>           m_rom;
    std::vector<u8>           m_sram;
    CartridgeHeader           m_header{};
    std::unique_ptr<Mapper>   m_mapper;
    bool                      m_loaded{false};
};
