#include <iostream>

#include "../headers/cart.hpp"
#include "../headers/emu.hpp"
#include "../headers/bus.hpp"
#include "../headers/timer.hpp"
#include "../headers/cpu.hpp"
#include "../headers/instructions.hpp"

int Emulator::run_emu(bool debug){
    
    Cart cart;
    cart.read_rom("../roms/tetris.gb");

    Bus bus = Bus(cart);    // Create Bus
    Instructions instr = Instructions();     // Create Instructions
    Timer timer = Timer();        // Create Timer

    gbCpu cpu(bus, instr, timer); // Pass pointers
    bus.set_cpu(&cpu);
    while(true){
        if (!cpu.step()) {
            printf("CPU Stopped\n");
            return 0;
        }
    }
    
    return 0;
}