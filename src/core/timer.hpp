#pragma once
#include "common/types.hpp"

class CPU;

class Timer {
public:
    Timer() = default;

    void Init(CPU* cpu);

    void Step(u32 tcycles);

    u8   ReadDIV() const  { return m_div >> 8; }
    void WriteDIV()       { m_div = 0; CheckFallingEdge(); }

    u8   ReadTIMA() const { return m_tima; }
    void WriteTIMA(u8 v)  { m_tima = v; }

    u8   ReadTMA() const  { return m_tma; }
    void WriteTMA(u8 v)   { m_tma = v; }

    u8   ReadTAC() const  { return m_tac | 0xF8; }
    void WriteTAC(u8 v)   { m_tac = v & 0x07; CheckFallingEdge(); }

private:
    CPU* m_cpu{nullptr};

    u16 m_div{0};
    u8  m_tima{0};
    u8  m_tma{0};
    u8  m_tac{0};

    bool m_lastEdge{false};
    void CheckFallingEdge();
};
