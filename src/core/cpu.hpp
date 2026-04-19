#pragma once
#include "common/types.hpp"

class MMU;

struct Registers {
    // 8-bit accessible pairs
    union { struct { u8 F, A; }; u16 AF; };
    union { struct { u8 C, B; }; u16 BC; };
    union { struct { u8 E, D; }; u16 DE; };
    union { struct { u8 L, H; }; u16 HL; };
    u16 SP{0xFFFE};
    u16 PC{0x0100};

    // Flag accessors (bits 7-4 of F)
    bool ZF() const { return (F >> 7) & 1; }
    bool NF() const { return (F >> 6) & 1; }
    bool HF() const { return (F >> 5) & 1; }
    bool CF() const { return (F >> 4) & 1; }

    void SetZF(bool v) { v ? (F |= 0x80) : (F &= ~0x80); }
    void SetNF(bool v) { v ? (F |= 0x40) : (F &= ~0x40); }
    void SetHF(bool v) { v ? (F |= 0x20) : (F &= ~0x20); }
    void SetCF(bool v) { v ? (F |= 0x10) : (F &= ~0x10); }

    void SetFlags(bool z, bool n, bool h, bool c) {
        F = (z ? 0x80 : 0) | (n ? 0x40 : 0) | (h ? 0x20 : 0) | (c ? 0x10 : 0);
    }
};

class CPU {
public:
    CPU() = default;

    void Init(MMU* mmu);

    // Boot without boot ROM – set post-boot register state
    void SkipBootROM(bool isCGB = false);

    // Execute one instruction. Returns T-cycles consumed.
    u32 Step();

    // Request interrupt (sets IF bit)
    void RequestInterrupt(u8 bit);

    Registers regs{};

    bool     IME{false};        // master interrupt enable
    bool     IMEPending{false}; // EI delay: enable IME next instruction
    bool     halted{false};
    bool     haltBug{false};
    bool     stopped{false};

private:
    MMU* m_mmu{nullptr};

    // Memory helpers
    u8   Fetch8();
    u16  Fetch16();
    u8   Read8(u16 addr);
    void Write8(u16 addr, u8 val);
    u16  Read16(u16 addr);
    void Write16(u16 addr, u16 val);
    void Push16(u16 val);
    u16  Pop16();

    // Timing accumulator within Step (T-cycles)
    u32 m_cycles{0};
    void Tick4();  // advance internal clock by 4 T-cycles

    // Interrupt handling – returns T-cycles if interrupt was serviced
    u32 HandleInterrupts();

    // Instruction execution
    void Execute(u8 opcode);
    void ExecuteCB(u8 opcode);

    // ALU helpers (result in A or dest)
    void ADD_A(u8 val);
    void ADC_A(u8 val);
    void SUB_A(u8 val);
    void SBC_A(u8 val);
    void AND_A(u8 val);
    void XOR_A(u8 val);
    void OR_A(u8 val);
    void CP_A(u8 val);

    u8  INC8(u8 val);
    u8  DEC8(u8 val);
    u16 ADD_HL(u16 val);
    u16 ADD_SP_r8();

    // CB ops
    u8  RLC(u8 val);
    u8  RRC(u8 val);
    u8  RL(u8 val);
    u8  RR(u8 val);
    u8  SLA(u8 val);
    u8  SRA(u8 val);
    u8  SWAP(u8 val);
    u8  SRL(u8 val);
    void BIT(u8 bit, u8 val);
};
