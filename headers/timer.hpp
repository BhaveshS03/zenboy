// #pragma once
// #include "common.hpp"
// #include "cpu.hpp"
// class gbCpu;

// class Timer{
//     private:
//         gbCpu* cpu;
//         u8 tma;
//         u8 tima;
//         u8 tac; 
//     public:
//         u16 div;
//         void set_cpu(gbCpu* cpu_ptr);
//         void emu_cycles(int cycle);
//         u8 timer_read(u16 address);
//         void timer_init();
//         void timer_tick();
//         void timer_write(u16 address, u8 value);
// };

#pragma once
#include <cstdint>

using u8  = uint8_t;
using u16 = uint16_t;

class gbCpu; // forward declaration

class Timer {
public:
    Timer(gbCpu* cpu_ptr = nullptr) : cpu(cpu_ptr) {}

    void set_cpu(gbCpu* cpu_ptr);
    void emu_cycles(int cycles);
    void timer_init();

    void timer_write(u16 address, u8 value);
    u8   timer_read(u16 address);

    gbCpu* cpu = nullptr;

    u16 div = 0;
    u8  tima = 0;
    u8  tma  = 0;
    u8  tac  = 0;

    void timer_tick();
    void timer_step();      // TIMA increment + overflow handling
    int  timer_bit() const; // which DIV bit is used based on TAC
};
