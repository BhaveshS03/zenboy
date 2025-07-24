#include <cstdint>
#include <vector>
#include <iostream>
#include "../headers/bus.hpp"
#include "../headers/cart.hpp"

// Game Boy Memory class
Bus::Bus(Cart cart) {
    memory = cart.get_rom_data();
    if (memory.size()>0){
    memory.resize(GB_MEMORY_SIZE);
    } else {
    std::cerr<<"Unable To Read Rom"<<std::endl;
    exit(-1);
    }
}

u8 Bus::read(uint16_t address) {
    if (address < GB_MEMORY_SIZE) {
        return memory[address];
    }
    return 0xFF;
}

void Bus::write16(uint16_t address, u16 value) {
    if (address < GB_MEMORY_SIZE) {
        memory[address] = static_cast<u8>(value & 0xFF);
        if (address + 1 < GB_MEMORY_SIZE) {
            memory[address + 1] = static_cast<u8>((value >> 8) & 0xFF);
        }
    }
}

void Bus::write(uint16_t address, u8 value) {
    if (address < GB_MEMORY_SIZE) {
        value = static_cast<u8>(value);
        memory[address]= value;
    }
}


