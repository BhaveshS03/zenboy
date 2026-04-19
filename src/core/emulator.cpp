#include "emulator.hpp"
#include "common/log.hpp"
#include <filesystem>

bool Emulator::LoadROM(const std::string& path) {
    if (!cart.Load(path)) return false;

    // Prepare save path
    std::filesystem::path p(path);
    m_savePath = p.replace_extension(".sav").string();
    cart.LoadSRAM(m_savePath);

    mmu.Init(&cart, &ppu, &apu, &timer, &joypad, &serial);
    cpu.Init(&mmu);
    ppu.Init(&cpu, &mmu);
    apu.Init();
    timer.Init(&cpu);
    joypad.Init(&cpu);
    serial.Init(&cpu);

    return true;
}

void Emulator::Shutdown() {
    if (cart.IsLoaded()) {
        cart.SaveSRAM(m_savePath);
    }
}

void Emulator::StepFrame() {
    ppu.ClearFrameReady();

    // Emulate until PPU signals frame is over
    while (!ppu.FrameReady()) {
        // Handle OAM DMA if active
        if (mmu.dmaBusy) {
            ProcessDMA();
            continue;
        }

        u32 tcycles = cpu.Step();
        if (tcycles == 0) tcycles = 4; // Failsafe if halt state with no interrupt cycles

        timer.Step(tcycles);
        ppu.Step(tcycles);
        apu.Step(tcycles);
        serial.Step(tcycles);
    }
}

void Emulator::ProcessDMA() {
    // OAM DMA transfers 160 bytes ($XX00 to $XX9F) to OAM ($FE00-$FE9F).
    // Takes 640 T-Cycles.
    for (u16 i = 0; i < 160; i++) {
        // Direct reads bypass the bus to avoid blocking
        u8 val = mmu.DirectRead(mmu.dmaSource + i);
        mmu.oam[i] = val;

        // Step components while bus is blocked
        timer.Step(4);
        ppu.Step(4);
        apu.Step(4);
        serial.Step(4);
    }
    mmu.dmaBusy = false;
}

void Emulator::SetInput(u8 buttons, u8 dpad) {
    joypad.SetState(buttons, dpad);
}
