#include "timer.hpp"
#include "cpu.hpp"

// TAC clock sources mapped to DIV bits
static const u8 tacBits[4] = {9, 3, 5, 7}; // 4096 (10), 262144 (4), 65536 (6), 16384 (8) in DIV

void Timer::Init(CPU* cpu) {
    m_cpu = cpu;
    m_div = 0xABCC; // post-boot DIV state
    m_tima = 0;
    m_tma = 0;
    m_tac = 0;
    m_lastEdge = false;
}

void Timer::Step(u32 tcycles) {
    for (u32 i = 0; i < tcycles; i += 4) {
        // DIV increments every T-cycle, but we step 4 at a time (M-cycle sync)
        m_div += 4;
        CheckFallingEdge();
    }
}

void Timer::CheckFallingEdge() {
    bool enable = (m_tac & (1 << 2)) != 0;
    u8 clockSrc = m_tac & 0x03;
    bool bit = (m_div >> tacBits[clockSrc]) & 1;
    bool currentEdge = enable && bit;

    if (m_lastEdge && !currentEdge) {
        // Falling edge
        m_tima++;
        if (m_tima == 0) { // overflow
            m_tima = m_tma;
            if (m_cpu) m_cpu->RequestInterrupt(2); // Timer interrupt
        }
    }
    m_lastEdge = currentEdge;
}
