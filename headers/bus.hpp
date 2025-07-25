#pragma once
#include "cart.hpp"
#include "cpu.hpp"
#include "timer.hpp"
#include "common.hpp"
#include <cstdint>

class gbCpu;
class Timer;

const size_t GB_MEMORY_SIZE = 65536; // Example size, adjust as needed

class Bus {
    private:
        gbCpu* cpu;
        Timer* tmr;
        std::vector<uint8_t> memory;
        uint8_t wram[0x2000];
        uint8_t hram[0x80];
        uint8_t hram_read(uint16_t address);
        uint8_t wram_read(uint16_t address);
        void hram_write(uint16_t address, uint8_t value);
        void wram_write(uint16_t address, uint8_t value);
        void io_write(u16 address, u8 value);
        u8 io_read(u16 address);
    
    public:
        Bus(Cart& cart_in, Timer* tmr_ptr = nullptr, gbCpu* cpu_ptr=nullptr);
        void set_cpu(gbCpu* cpu_ptr);
        uint8_t get_ie_register();
        void set_ie_register(uint8_t value);
        uint8_t read(uint16_t address);
        uint16_t read16(uint16_t address);
        void write(uint16_t address, uint8_t value);
        void write16(uint16_t address, uint16_t value);
    };

// Memory map
enum MemoryMap {
    ROM_BANK_00       = 0x0000,
    VRAM             = 0x8000,
    OAM              = 0xFE00,
    IO_REGISTERS      = 0xFF00,
    HRAM             = 0xFF80,
    INTERRUPT_ENABLE = 0xFFFF
};

// 0x0000 - 0x3FFF : ROM Bank 0
// 0x4000 - 0x7FFF : ROM Bank 1 - Switchable
// 0x8000 - 0x97FF : CHR RAM
// 0x9800 - 0x9BFF : BG Map 1
// 0x9C00 - 0x9FFF : BG Map 2
// 0xA000 - 0xBFFF : Cartridge RAM
// 0xC000 - 0xCFFF : RAM Bank 0
// 0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only
// 0xE000 - 0xFDFF : Reserved - Echo RAM
// 0xFE00 - 0xFE9F : Object Attribute Memory
// 0xFEA0 - 0xFEFF : Reserved - Unusable
// 0xFF00 - 0xFF7F : I/O Registers
// 0xFF80 - 0xFFFE : Zero Page