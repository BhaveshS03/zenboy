#include <iostream>

#include "../headers/cart.hpp"
#include "../headers/emu.hpp"
#include "../headers/bus.hpp"
#include "../headers/timer.hpp"
#include "../headers/cpu.hpp"
#include "../headers/instructions.hpp"

int Emulator::run_emu(bool debug){
    Timer timer;
    Cart cart;
    cart.read_rom("../../roms/02-interrupts.gb");

    Bus bus = Bus(cart, &timer, nullptr);    // Create Bus
    Instructions instr = Instructions();     // Create Instructions

    gbCpu cpu(bus, instr, timer); // Pass pointers
    timer.set_cpu(&cpu);
    bus.set_cpu(&cpu);

    int i=0;
    while(true){
        if (!cpu.step()) {
            std::cout<<("CPU Stopped\n");
            return 0;
        }
        timer.timer_tick();
        i+=1;
    }
    
    return 0;
}