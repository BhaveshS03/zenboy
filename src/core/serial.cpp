#include "serial.hpp"
#include "cpu.hpp"
#include <cstdio>

void Serial::Init(CPU* cpu) {
    m_cpu = cpu;
    m_sb = 0;
    m_sc = 0;
    m_transferClock = 0;
}

void Serial::Step(u32 tcycles) {
    if ((m_sc & 0x81) == 0x81) { // Transfer requested, internal clock
        m_transferClock += tcycles;

        // Roughly 512 T-cycles per bit (1 byte = 4096 cycles for standard DMG clock)
        if (m_transferClock >= 4096) {
            m_transferClock -= 4096;
            m_sc &= 0x7F; // clear transfer flag
            m_cpu->RequestInterrupt(3); // Serial interrupt

            // For tests like Blargg's, output the serial data
            // printf("%c", m_sb);
            m_sb = 0xFF; // Simulate no link cable connection receiving side
        }
    }
}
