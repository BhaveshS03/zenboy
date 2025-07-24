#include <cstdint>
#include <vector>
#include <iostream>
#include <algorithm>

#include "../headers/bus.hpp"
#include "../headers/cart.hpp"

#ifndef GB_MEMORY_SIZE
#define GB_MEMORY_SIZE 0x10000
#endif



Bus::Bus(Cart cart) {
    memory = cart.get_rom_data();

    if (memory.empty()) {
        std::cerr << "Error: Unable to read ROM data or ROM data is empty." << std::endl;
        exit(-1);
    }

    if (memory.size() < GB_MEMORY_SIZE) {
        memory.resize(GB_MEMORY_SIZE, 0x00);
    }

    std::fill(std::begin(wram), std::end(wram), 0x00);
    std::fill(std::begin(hram), std::end(hram), 0x00);
}

uint8_t Bus::read(uint16_t address) {
    if (address >= 0xC000 && address <= 0xDFFF) {
        uint16_t offset = address - 0xC000;
        if (offset >= 0x2000) {
            std::cerr << "Error: Invalid WRAM read address 0x" << std::hex << address << std::dec << std::endl;
            exit(-1);
        }
        return wram[offset];
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        uint16_t offset = address - 0xFF80;
        if (offset >= 0x80) {
            std::cerr << "Error: Invalid HRAM read address 0x" << std::hex << address << std::dec << std::endl;
            exit(-1);
        }
        return hram[offset];
    }
    else if (address < GB_MEMORY_SIZE) {
        return memory[address];
    }
    return 0xFF;
}

void Bus::write(uint16_t address, uint8_t value) {
    if (address >= 0xC000 && address <= 0xDFFF) {
        uint16_t offset = address - 0xC000;
        if (offset >= 0x2000) {
            std::cerr << "Error: Invalid WRAM write address 0x" << std::hex << address << std::dec << std::endl;
            exit(-1);
        }
        wram[offset] = value;
    }
    else if (address >= 0xFF80 && address <= 0xFFFE) {
        uint16_t offset = address - 0xFF80;
        if (offset >= 0x80) {
            std::cerr << "Error: Invalid HRAM write address 0x" << std::hex << address << std::dec << std::endl;
            exit(-1);
        }
        hram[offset] = value;
    }
    else if (address < GB_MEMORY_SIZE) {
        memory[address] = value;
    }
}

void Bus::write16(uint16_t address, uint16_t value) {
    write(address, static_cast<uint8_t>(value & 0xFF));
    write(address + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
}
