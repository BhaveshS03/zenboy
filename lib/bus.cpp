#include <cstdint>
#include <vector>
#include <iostream>
#include <algorithm>
#include "../headers/bus.hpp"
#include "../headers/cart.hpp"
#include "../headers/timer.hpp"

#ifndef GB_MEMORY_SIZE
#define GB_MEMORY_SIZE 0x10000
#endif

static char serial_data[2];

u8 Bus::io_read(u16 address) {
    if (address == 0xFF01) {
        return serial_data[0];
    }

    if (address == 0xFF02) {
        return serial_data[1];
    }

    if (address >= 0xFF04 && address <= 0xFF07) {
        return tmr->timer_read(address);
    }

    if (address == 0xFF0F) {
        return cpu->get_int_flags();
    }

    // printf("UNSUPPORTED bus_read(%04X)\n", address);
    return 0;
}

void Bus::io_write(u16 address, u8 value) {
    if (address == 0xFF01) {
        serial_data[0] = value;
        return;
    }

    if (address == 0xFF02) {
        serial_data[1] = value;
        return;
    }

    if (0xFF04 >= address && address <= 0xFF07) {
        tmr->timer_write(address, value);
        return;
    }
    
    if (address == 0xFF0F) {
        cpu->set_int_flags(value);
        return;
    }

    // std::cerr<<"UNSUPPORTED bus_write"<<hex<<(int)address;
}

void Bus::set_cpu(gbCpu* cpu_ptr) {
    cpu = cpu_ptr;
}
// Bus::Bus(Cart cart_in)
Bus::Bus(Cart& cart_in, Timer* tmr_ptr, gbCpu* cpu_ptr){ // Initialize cart member
    tmr = tmr_ptr;
    memory = cart_in.get_rom_data();
    if (memory.empty()) {
        std::cerr << "Error: Unable to read ROM data or ROM data is empty." << std::endl;
        exit(-1);
    }
    if (memory.size() < GB_MEMORY_SIZE) {
        memory.resize(GB_MEMORY_SIZE, 0x00);
    }
    std::fill(std::begin(wram), std::end(wram), 0x00);
    std::fill(std::begin(hram), std::end(hram), 0x00);
    // ie_register = 0x00; // Initialize IE register
}

uint8_t Bus::wram_read(uint16_t address) {
    if (address == 0xD81B) {
        std::cerr << "WRAM_READ 0xD81B: address=0x" << std::hex << address << std::endl;
    }
    
    uint16_t offset = address - 0xC000;
    
    if (address == 0xD81B) {
        std::cerr << "WRAM_READ 0xD81B: offset=0x" << std::hex << offset << std::endl;
    }
    
    if (offset >= 0x2000) {
        std::cerr << "Error: Invalid WRAM read address 0x" << std::hex << address << std::dec << std::endl;
        exit(-1);
    }
    
    uint8_t result = wram[offset];
    
    if (address == 0xD81B) {
        std::cerr << "WRAM_READ 0xD81B: wram[0x" << std::hex << offset << "]=0x" << (int)result << std::endl;
    }
    
    return result;
}

void Bus::wram_write(uint16_t address, uint8_t value) {
    uint16_t offset = address - 0xC000;
    
    if (offset >= 0x2000) {
        std::cerr << "Error: Invalid WRAM write address 0x" << std::hex << address << std::dec << std::endl;
        exit(-1);
    }
    wram[offset] = value;
}

uint8_t Bus::hram_read(uint16_t address) {
    uint16_t offset = address - 0xFF80;
    
    if (offset >= 0x80) {
        std::cerr << "Error: Invalid HRAM read address 0x" << std::hex << address << std::dec << std::endl;
        exit(-1);
    }
    return hram[offset];
}

void Bus::hram_write(uint16_t address, uint8_t value) {
    uint16_t offset = address - 0xFF80;
    
    if (offset >= 0x80) {
        std::cerr << "Error: Invalid HRAM write address 0x" << std::hex << address << std::dec << std::endl;
        exit(-1);
    }
    hram[offset] = value;
}


uint8_t Bus::read(uint16_t address) {
    if (address == 0xFF44) {
        return 0X90; // or however you track LY
    }
    if (address < 0x8000) {
        // ROM Bank 0 and 1 (Switchable)
        return memory[address];
    } else if (address < 0xA000) {
        // CHR RAM / BG Map Data (VRAM)
        // std::cerr << "Error: UNSUPPORTED bus_read(VRAM) 0x" << std::hex << address << std::dec << std::endl;
        return 0x00;
    } else if (address < 0xC000) {
        // Cartridge RAM (External RAM)
        return memory[address];
    } else if (address < 0xE000) {
        // WRAM (Working RAM) - Bank 0 and 1-7 (switchable)
        return wram_read(address);
    } else if (address < 0xFE00) {
        return wram_read(address  - 0x2000);
    } else if (address < 0xFEA0) {
        // Object Attribute Memory (OAM)
        // std::cerr << "Error: UNSUPPORTED bus_read(OAM) 0x" << std::hex << address << std::dec << std::endl;
        return 0x00;
    } else if (address < 0xFF00) {
        // Reserved - Unusable
        return 0x00;
    } else if (address < 0xFF80) {
        return io_read(address);
    } else if (address == 0xFFFF) {
        // CPU Interrupt Enable Register (IE)
        return cpu->get_ie_register();
    } else { // 0xFF80 - 0xFFFE
        // Zero Page / HRAM (High RAM)
        return hram_read(address);
    }
}

void Bus::write(uint16_t address, uint8_t value) {
    if (address < 0x8000) {
        // ROM Data (writes are usually ignored or control MBC)
        memory[address] = value;
    } else if (address < 0xA000) {
        // CHR RAM / BG Map Data (VRAM)
        // std::cerr << "Error: UNSUPPORTED bus_write(VRAM) 0x" << std::hex << address << std::dec << std::endl;
    } else if (address < 0xC000) {
        // Cartridge RAM (External RAM)
        memory[address] = value;
    } else if (address < 0xE000) {
        // WRAM (Working RAM)
        wram_write(address, value);
    } else if (address < 0xFE00) {
        wram_write(address - 0x2000, value);
    } else if (address < 0xFEA0) {
        // Object Attribute Memory (OAM)
        // std::cerr << "Error: UNSUPPORTED bus_write(OAM) 0x" << std::hex << address << std::dec << std::endl;
    } else if (address < 0xFF00) {
        // Reserved - Unusable (writes are ignored)
    } else if (address < 0xFF80) {
        // I/O Registers
        io_write(address,value);
        // std::cerr << "Error: UNSUPPORTED bus_write(IO Registers) 0x" << std::hex << address << std::dec << std::endl;
    } else if (address == 0xFFFF) {
        // CPU Interrupt Enable Register (IE)
        cpu->set_ie_register(value);
    } else { // 0xFF80 - 0xFFFE
        // Zero Page / HRAM (High RAM)
        hram_write(address, value);
    }
}

uint16_t Bus::read16(uint16_t address) {
    uint16_t lo = read(address);
    uint16_t hi = read(address + 1);
    return lo | (hi << 8);
}

void Bus::write16(uint16_t address, uint16_t value) {
    write(address, static_cast<uint8_t>(value & 0xFF));
    write(address + 1, static_cast<uint8_t>((value >> 8) & 0xFF));
}
