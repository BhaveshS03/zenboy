#pragma once
#include "common/types.hpp"

class CPU;

class Serial {
public:
    Serial() = default;

    void Init(CPU* cpu);

    void Step(u32 tcycles);

    u8   ReadSB() const { return m_sb; }
    void WriteSB(u8 v)  { m_sb = v; }

    u8   ReadSC() const { return m_sc | 0x7E; }
    void WriteSC(u8 v)  { m_sc = v; }

private:
    CPU* m_cpu{nullptr};
    u8 m_sb{0};
    u8 m_sc{0};
    u32 m_transferClock{0};
};
