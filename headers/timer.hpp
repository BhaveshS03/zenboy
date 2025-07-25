#pragma once
#include "common.hpp"
#include "cpu.hpp"
class gbCpu;

class Timer{
    private:
        gbCpu* cpu;
        u8 tma;
        u8 tima;
        u8 tac; 
    public:
        u16 div;
        Timer(gbCpu* cpu = nullptr);
        void emu_cycles(int cycle);
        u8 timer_read(u16 address);
        void timer_init();
        void timer_tick();
        void timer_write(u16 address, u8 value);
};