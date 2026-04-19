#pragma once
#include "common/types.hpp"
#include "core/cpu.hpp"
#include "core/mmu.hpp"
#include "core/ppu.hpp"
#include "core/apu.hpp"
#include "core/timer.hpp"
#include "core/joypad.hpp"
#include "core/serial.hpp"
#include "core/cartridge.hpp"

#include <string>
#include <memory>

class Emulator {
public:
    Emulator() = default;

    bool LoadROM(const std::string& path);

    // Executes emulation until a frame is completely rendered
    void StepFrame();

    // Input
    void SetInput(u8 buttons, u8 dpad);

    // Frontend access
    const u32* GetFrameBuffer() const { return ppu.GetFrameBuffer(); }
    const Cartridge& GetCartridge() const { return cart; }

    // On close
    void Shutdown();

private:
    CPU       cpu;
    MMU       mmu;
    PPU       ppu;
    APU       apu;
    Timer     timer;
    Joypad    joypad;
    Serial    serial;
    Cartridge cart;

    std::string m_savePath;

    // Handles the logic for OAM DMA
    void ProcessDMA();
};
