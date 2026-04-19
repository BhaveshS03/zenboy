#pragma once
#include "common/types.hpp"

class APU {
public:
    APU() = default;

    void Init();
    void Step(u32 tcycles);

    u8   ReadReg(u8 reg);
    void WriteReg(u8 reg, u8 val);

private:
    u32 m_cycles{0};
    u32 m_seqTimer{0};
    u8  m_seqStep{0};

    // simplified internal state for now — a stub sufficient to boot ROMs
    u8 m_regs[0x30]{};
    u8 m_waveRam[0x10]{};

    void FrameSequencerStep();
};
