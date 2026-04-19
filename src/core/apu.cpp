#include "apu.hpp"
#include <cstring>

void APU::Init() {
    m_cycles = 0;
    m_seqTimer = 0;
    m_seqStep = 0;
    std::memset(m_regs, 0, sizeof(m_regs));
    std::memset(m_waveRam, 0, sizeof(m_waveRam));
}

void APU::Step(u32 tcycles) {
    m_cycles += tcycles;

    // Frame sequencer clocks at 512Hz (every 8192 T-cycles assuming 4MHz clock)
    m_seqTimer += tcycles;
    if (m_seqTimer >= 8192) {
        m_seqTimer -= 8192;
        FrameSequencerStep();
    }
}

void APU::FrameSequencerStep() {
    m_seqStep = (m_seqStep + 1) & 7;
}

u8 APU::ReadReg(u8 reg) {
    if (reg >= 0x10 && reg < 0x27) return m_regs[reg - 0x10];
    if (reg >= 0x30 && reg <= 0x3F) return m_waveRam[reg - 0x30];
    return 0xFF;
}

void APU::WriteReg(u8 reg, u8 val) {
    if (reg >= 0x10 && reg < 0x27) {
        if (reg == 0x26) {
            // NR52 master control
            if ((val & 0x80) == 0) {
                // power off: clear all registers
                std::memset(m_regs, 0, sizeof(m_regs));
            }
            m_regs[reg - 0x10] = val & 0x8F; // only power bit and channel statuses are kept
        } else {
            // writes ignored if powered off
            if (m_regs[0x26 - 0x10] & 0x80) {
                m_regs[reg - 0x10] = val;
            }
        }
    } else if (reg >= 0x30 && reg <= 0x3F) {
        m_waveRam[reg - 0x30] = val;
    }
}
