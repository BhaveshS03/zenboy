#include <iostream>

#include "../headers/cart.hpp"
#include "../headers/emu.hpp"
#include "../headers/bus.hpp"
#include "../headers/timer.hpp"
#include "../headers/cpu.hpp"
#include "../headers/instructions.hpp"

int Emulator::run_emu(bool debug){
    Timer timer = Timer();        // Create Timer
    Cart cart;
    cart.read_rom("./roms/7cpu.gb");

    Bus bus = Bus(cart, &timer, nullptr);    // Create Bus
    Instructions instr = Instructions();     // Create Instructions

    gbCpu cpu(bus, instr, timer); // Pass pointers
    bus.set_cpu(&cpu);
    while(true){
        if (!cpu.step()) {
            printf("CPU Stopped\n");
            return 0;
        }
        timer.timer_tick();
    }
    
    return 0;
}