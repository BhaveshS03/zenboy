#include "joypad.hpp"
#include "cpu.hpp"

void Joypad::Init(CPU* cpu) {
    m_cpu = cpu;
    m_buttons = 0xFF;
    m_dpad = 0xFF;
    m_select = 0x30;
}

u8 Joypad::Read() const {
    u8 val = m_select | 0xCF; // bits 4, 5 are the selectors, lower 4 bits are inputs

    if (!(m_select & 0x10)) {
        // bit 4 is 0 -> select dpad
        val &= m_dpad;
    }
    if (!(m_select & 0x20)) {
        // bit 5 is 0 -> select buttons
        val &= m_buttons;
    }
    return val;
}

void Joypad::Write(u8 val) {
    m_select = val & 0x30;
}

void Joypad::SetState(u8 buttons, u8 dpad) {
    bool previouslyHigh = (Read() & 0x0F) != 0x0F;

    m_buttons = buttons;
    m_dpad = dpad;

    bool currentlyLow = (Read() & 0x0F) != 0x0F;
    if (!previouslyHigh && currentlyLow) {
        if (m_cpu) m_cpu->RequestInterrupt(4); // Joypad interrupt
    }
}
