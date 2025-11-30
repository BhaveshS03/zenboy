#include "timer.hpp"
#include "cpu.hpp"

void Timer::set_cpu(gbCpu* cpu_ptr) {
    cpu = cpu_ptr;
}

void Timer::emu_cycles(int cycles) {
    for (int i = 0; i < cycles; i++) {
        timer_tick();
    }
}
void Timer::timer_init() {
    div  = 0xAC00;
    tac  = 0x00;
    tima = 0x00;
    tma  = 0x00;
}


int Timer::timer_bit() const {
    switch (tac & 0x03) {
        case 0b00: return 9; // 4096 Hz
        case 0b01: return 3; // 262144 Hz
        case 0b10: return 5; // 65536 Hz
        case 0b11: return 7; // 16384 Hz
    }
    return 9;
}

void Timer::timer_step() {
    if (tima == 0xFF) {
        tima = tma;
        if (cpu) {
            cpu->request_interrupt(IT_TIMER);
        }
    } else {
        tima++;
    }
}

void Timer::timer_tick() {
    int bit = timer_bit();

    bool prev = (div >> bit) & 1;
    div++;
    bool curr = (div >> bit) & 1;

    if (tac & 0x04) {
        if (prev && !curr) {
            timer_step();
        }
    }
}

void Timer::timer_write(u16 address, u8 value) {
    switch (address) {
        case 0xFF04: {
            int bit = timer_bit();
            bool prev = (div >> bit) & 1;

            div = 0;

            bool curr = 0;

            if ((tac & 0x04) && prev && !curr) {
                timer_step();
            }
            break;
        }

        case 0xFF05: // TIMA
            tima = value;
            break;

        case 0xFF06: // TMA
            tma = value;
            break;

        case 0xFF07: // TAC
            // Only low 3 bits are used
            tac = value & 0x07;
            break;
    }
}

u8 Timer::timer_read(u16 address) {
    switch (address) {
        case 0xFF04: // DIV
            return div >> 8;
        case 0xFF05: // TIMA
            return tima;
        case 0xFF06: // TMA
            return tma;
        case 0xFF07: // TAC
            return tac;
        default:
            return 0x00;
    }
}
