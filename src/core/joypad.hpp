#pragma once
#include "common/types.hpp"

class CPU;

class Joypad {
public:
    Joypad() = default;

    void Init(CPU* cpu);

    u8   Read() const;
    void Write(u8 val);

    void SetState(u8 buttons, u8 dpad);

private:
    CPU* m_cpu{nullptr};
    u8 m_buttons{0xFF}; // 0 = pressed
    u8 m_dpad{0xFF};    // 0 = pressed
    u8 m_select{0x30};  // Bits 4, 5 control reading
};
