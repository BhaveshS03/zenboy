#include "cpu.hpp"
#include "mmu.hpp"
#include "common/log.hpp"
#include <stdexcept>

void CPU::Init(MMU* mmu) {
    m_mmu = mmu;
    SkipBootROM(false);
}

void CPU::SkipBootROM(bool isCGB) {
    regs.A = isCGB ? 0x11 : 0x01;
    regs.F = 0xB0;
    regs.B = 0x00; regs.C = 0x13;
    regs.D = 0x00; regs.E = 0xD8;
    regs.H = 0x01; regs.L = 0x4D;
    regs.SP = 0xFFFE;
    regs.PC = 0x0100;
    m_mmu->Write(0xFF50, 0x01); // Unmap boot rom
}

void CPU::Tick4() {
    m_cycles += 4;
}

u8 CPU::Read8(u16 addr) {
    Tick4();
    return m_mmu->Read(addr);
}

void CPU::Write8(u16 addr, u8 val) {
    Tick4();
    m_mmu->Write(addr, val);
}

u8 CPU::Fetch8() {
    return Read8(regs.PC++);
}

u16 CPU::Fetch16() {
    u8 lo = Fetch8();
    u8 hi = Fetch8();
    return (hi << 8) | lo;
}

u16 CPU::Read16(u16 addr) {
    u8 lo = Read8(addr);
    u8 hi = Read8(addr + 1);
    return (hi << 8) | lo;
}

void CPU::Write16(u16 addr, u16 val) {
    Write8(addr, val & 0xFF);
    Write8(addr + 1, val >> 8);
}

void CPU::Push16(u16 val) {
    Tick4(); // internal delay
    Write8(--regs.SP, val >> 8);
    Write8(--regs.SP, val & 0xFF);
}

u16 CPU::Pop16() {
    u8 lo = Read8(regs.SP++);
    u8 hi = Read8(regs.SP++);
    return (hi << 8) | lo;
}

void CPU::RequestInterrupt(u8 bit) {
    m_mmu->intf |= (1 << bit);
}

u32 CPU::HandleInterrupts() {
    if (!IME && !halted) return 0;
    u8 req = m_mmu->ie & m_mmu->intf & 0x1F;
    if (req == 0) return 0;

    // Wake up
    halted = false;
    if (!IME) return 0;

    // Service interrupt
    IME = false;
    Tick4(); // 2 wait states
    Tick4();

    for (int i = 0; i < 5; ++i) {
        if (req & (1 << i)) {
            m_mmu->intf &= ~(1 << i);
            Push16(regs.PC);
            regs.PC = 0x0040 + (i * 0x08);
            return m_cycles;
        }
    }
    return 0;
}

u32 CPU::Step() {
    m_cycles = 0;

    u32 intCycles = HandleInterrupts();
    if (intCycles > 0) return intCycles;

    if (halted) {
        Tick4();
        return m_cycles;
    }

    if (IMEPending) {
        IME = true;
        IMEPending = false;
    }

    // u16 oldPC = regs.PC;
    u8 opcode = Fetch8();
    if (haltBug) {
        regs.PC--; // executes next byte twice
        haltBug = false;
    }

    Execute(opcode);

    return m_cycles;
}

// ---------------------------------------------------------
// ALU
// ---------------------------------------------------------
void CPU::ADD_A(u8 val) {
    u16 res = regs.A + val;
    regs.SetZF((res & 0xFF) == 0);
    regs.SetNF(false);
    regs.SetHF((regs.A & 0x0F) + (val & 0x0F) > 0x0F);
    regs.SetCF(res > 0xFF);
    regs.A = res & 0xFF;
}
void CPU::ADC_A(u8 val) {
    u8 c = regs.CF() ? 1 : 0;
    u16 res = regs.A + val + c;
    regs.SetZF((res & 0xFF) == 0);
    regs.SetNF(false);
    regs.SetHF((regs.A & 0x0F) + (val & 0x0F) + c > 0x0F);
    regs.SetCF(res > 0xFF);
    regs.A = res & 0xFF;
}
void CPU::SUB_A(u8 val) {
    u16 res = regs.A - val;
    regs.SetZF((res & 0xFF) == 0);
    regs.SetNF(true);
    regs.SetHF((regs.A & 0x0F) < (val & 0x0F));
    regs.SetCF(regs.A < val);
    regs.A = res & 0xFF;
}
void CPU::SBC_A(u8 val) {
    u8 c = regs.CF() ? 1 : 0;
    u16 res = regs.A - val - c;
    regs.SetZF((res & 0xFF) == 0);
    regs.SetNF(true);
    regs.SetHF((regs.A & 0x0F) < (val & 0x0F) + c);
    regs.SetCF(regs.A < val + c);
    regs.A = res & 0xFF;
}
void CPU::AND_A(u8 val) {
    regs.A &= val;
    regs.SetFlags(regs.A == 0, false, true, false);
}
void CPU::XOR_A(u8 val) {
    regs.A ^= val;
    regs.SetFlags(regs.A == 0, false, false, false);
}
void CPU::OR_A(u8 val) {
    regs.A |= val;
    regs.SetFlags(regs.A == 0, false, false, false);
}
void CPU::CP_A(u8 val) {
    u16 res = regs.A - val;
    regs.SetZF((res & 0xFF) == 0);
    regs.SetNF(true);
    regs.SetHF((regs.A & 0x0F) < (val & 0x0F));
    regs.SetCF(regs.A < val);
}
u8 CPU::INC8(u8 val) {
    u8 res = val + 1;
    regs.SetZF(res == 0);
    regs.SetNF(false);
    regs.SetHF((val & 0x0F) == 0x0F);
    return res;
}
u8 CPU::DEC8(u8 val) {
    u8 res = val - 1;
    regs.SetZF(res == 0);
    regs.SetNF(true);
    regs.SetHF((val & 0x0F) == 0x00);
    return res;
}
u16 CPU::ADD_HL(u16 val) {
    Tick4(); // 16-bit add takes 1 internal cycle
    u32 res = regs.HL + val;
    regs.SetNF(false);
    regs.SetHF((regs.HL & 0x0FFF) + (val & 0x0FFF) > 0x0FFF);
    regs.SetCF(res > 0xFFFF);
    regs.HL = res & 0xFFFF;
    return regs.HL;
}
u16 CPU::ADD_SP_r8() {
    s8 offset = static_cast<s8>(Fetch8());
    Tick4();
    Tick4();
    u32 res = regs.SP + offset;
    regs.SetZF(false);
    regs.SetNF(false);
    regs.SetHF((regs.SP & 0x0F) + (offset & 0x0F) > 0x0F);
    regs.SetCF((regs.SP & 0xFF) + (u8)offset > 0xFF);
    return res & 0xFFFF;
}

// ---------------------------------------------------------
// CPU Opcodes - Simplified generator macros
// ---------------------------------------------------------
void CPU::Execute(u8 op) {
    switch(op) {
        case 0x00: break; // NOP
        case 0x01: regs.BC = Fetch16(); break; // LD BC, d16
        case 0x02: Write8(regs.BC, regs.A); break; // LD (BC), A
        case 0x03: Tick4(); regs.BC++; break; // INC BC
        case 0x04: regs.B = INC8(regs.B); break;
        case 0x05: regs.B = DEC8(regs.B); break;
        case 0x06: regs.B = Fetch8(); break;
        case 0x07: { // RLCA
            u8 c = (regs.A >> 7) & 1;
            regs.A = (regs.A << 1) | c;
            regs.SetFlags(false, false, false, c);
        } break;
        case 0x08: Write16(Fetch16(), regs.SP); break; // LD (a16), SP
        case 0x09: ADD_HL(regs.BC); break;
        case 0x0A: regs.A = Read8(regs.BC); break; // LD A, (BC)
        case 0x0B: Tick4(); regs.BC--; break;
        case 0x0C: regs.C = INC8(regs.C); break;
        case 0x0D: regs.C = DEC8(regs.C); break;
        case 0x0E: regs.C = Fetch8(); break;
        case 0x0F: { // RRCA
            u8 c = regs.A & 1;
            regs.A = (regs.A >> 1) | (c << 7);
            regs.SetFlags(false, false, false, c);
        } break;
        case 0x10: Fetch8(); stopped = true; break; // STOP
        case 0x11: regs.DE = Fetch16(); break;
        case 0x12: Write8(regs.DE, regs.A); break;
        case 0x13: Tick4(); regs.DE++; break;
        case 0x14: regs.D = INC8(regs.D); break;
        case 0x15: regs.D = DEC8(regs.D); break;
        case 0x16: regs.D = Fetch8(); break;
        case 0x17: { // RLA
            u8 c = regs.CF() ? 1 : 0;
            u8 nextC = (regs.A >> 7) & 1;
            regs.A = (regs.A << 1) | c;
            regs.SetFlags(false, false, false, nextC);
        } break;
        case 0x18: { s8 d = Fetch8(); Tick4(); regs.PC += d; } break; // JR
        case 0x19: ADD_HL(regs.DE); break;
        case 0x1A: regs.A = Read8(regs.DE); break;
        case 0x1B: Tick4(); regs.DE--; break;
        case 0x1C: regs.E = INC8(regs.E); break;
        case 0x1D: regs.E = DEC8(regs.E); break;
        case 0x1E: regs.E = Fetch8(); break;
        case 0x1F: { // RRA
            u8 c = regs.CF() ? 1 : 0;
            u8 nextC = regs.A & 1;
            regs.A = (regs.A >> 1) | (c << 7);
            regs.SetFlags(false, false, false, nextC);
        } break;
        case 0x20: { s8 d = Fetch8(); if(!regs.ZF()) { Tick4(); regs.PC += d; } } break; // JR NZ
        case 0x21: regs.HL = Fetch16(); break;
        case 0x22: Write8(regs.HL++, regs.A); break; // LDI (HL), A
        case 0x23: Tick4(); regs.HL++; break;
        case 0x24: regs.H = INC8(regs.H); break;
        case 0x25: regs.H = DEC8(regs.H); break;
        case 0x26: regs.H = Fetch8(); break;
        case 0x27: { // DAA
            u8 a = regs.A; u16 res = a;
            if(!regs.NF()) {
                if(regs.HF() || (res & 0x0F) > 0x09) res += 0x06;
                if(regs.CF() || res > 0x9F) res += 0x60;
            } else {
                if(regs.HF()) res = (res - 0x06) & 0xFF;
                if(regs.CF()) res -= 0x60;
            }
            regs.SetZF((res & 0xFF) == 0);
            regs.SetHF(false);
            if(res & 0x100) regs.SetCF(true);
            regs.A = res & 0xFF;
        } break;
        case 0x28: { s8 d = Fetch8(); if(regs.ZF()) { Tick4(); regs.PC += d; } } break;
        case 0x29: ADD_HL(regs.HL); break;
        case 0x2A: regs.A = Read8(regs.HL++); break; // LDI A, (HL)
        case 0x2B: Tick4(); regs.HL--; break;
        case 0x2C: regs.L = INC8(regs.L); break;
        case 0x2D: regs.L = DEC8(regs.L); break;
        case 0x2E: regs.L = Fetch8(); break;
        case 0x2F: regs.A = ~regs.A; regs.SetNF(true); regs.SetHF(true); break; // CPL
        case 0x30: { s8 d = Fetch8(); if(!regs.CF()) { Tick4(); regs.PC += d; } } break;
        case 0x31: regs.SP = Fetch16(); break;
        case 0x32: Write8(regs.HL--, regs.A); break; // LDD (HL), A
        case 0x33: Tick4(); regs.SP++; break;
        case 0x34: Write8(regs.HL, INC8(Read8(regs.HL))); break;
        case 0x35: Write8(regs.HL, DEC8(Read8(regs.HL))); break;
        case 0x36: Write8(regs.HL, Fetch8()); break;
        case 0x37: regs.SetNF(false); regs.SetHF(false); regs.SetCF(true); break; // SCF
        case 0x38: { s8 d = Fetch8(); if(regs.CF()) { Tick4(); regs.PC += d; } } break;
        case 0x39: ADD_HL(regs.SP); break;
        case 0x3A: regs.A = Read8(regs.HL--); break; // LDD A, (HL)
        case 0x3B: Tick4(); regs.SP--; break;
        case 0x3C: regs.A = INC8(regs.A); break;
        case 0x3D: regs.A = DEC8(regs.A); break;
        case 0x3E: regs.A = Fetch8(); break;
        case 0x3F: regs.SetNF(false); regs.SetHF(false); regs.SetCF(!regs.CF()); break; // CCF

        // LD r1, r2
        case 0x40: regs.B = regs.B; break;
        case 0x41: regs.B = regs.C; break;
        case 0x42: regs.B = regs.D; break;
        case 0x43: regs.B = regs.E; break;
        case 0x44: regs.B = regs.H; break;
        case 0x45: regs.B = regs.L; break;
        case 0x46: regs.B = Read8(regs.HL); break;
        case 0x47: regs.B = regs.A; break;
        case 0x48: regs.C = regs.B; break;
        case 0x49: regs.C = regs.C; break;
        case 0x4A: regs.C = regs.D; break;
        case 0x4B: regs.C = regs.E; break;
        case 0x4C: regs.C = regs.H; break;
        case 0x4D: regs.C = regs.L; break;
        case 0x4E: regs.C = Read8(regs.HL); break;
        case 0x4F: regs.C = regs.A; break;
        case 0x50: regs.D = regs.B; break;
        case 0x51: regs.D = regs.C; break;
        case 0x52: regs.D = regs.D; break;
        case 0x53: regs.D = regs.E; break;
        case 0x54: regs.D = regs.H; break;
        case 0x55: regs.D = regs.L; break;
        case 0x56: regs.D = Read8(regs.HL); break;
        case 0x57: regs.D = regs.A; break;
        case 0x58: regs.E = regs.B; break;
        case 0x59: regs.E = regs.C; break;
        case 0x5A: regs.E = regs.D; break;
        case 0x5B: regs.E = regs.E; break;
        case 0x5C: regs.E = regs.H; break;
        case 0x5D: regs.E = regs.L; break;
        case 0x5E: regs.E = Read8(regs.HL); break;
        case 0x5F: regs.E = regs.A; break;
        case 0x60: regs.H = regs.B; break;
        case 0x61: regs.H = regs.C; break;
        case 0x62: regs.H = regs.D; break;
        case 0x63: regs.H = regs.E; break;
        case 0x64: regs.H = regs.H; break;
        case 0x65: regs.H = regs.L; break;
        case 0x66: regs.H = Read8(regs.HL); break;
        case 0x67: regs.H = regs.A; break;
        case 0x68: regs.L = regs.B; break;
        case 0x69: regs.L = regs.C; break;
        case 0x6A: regs.L = regs.D; break;
        case 0x6B: regs.L = regs.E; break;
        case 0x6C: regs.L = regs.H; break;
        case 0x6D: regs.L = regs.L; break;
        case 0x6E: regs.L = Read8(regs.HL); break;
        case 0x6F: regs.L = regs.A; break;
        case 0x70: Write8(regs.HL, regs.B); break;
        case 0x71: Write8(regs.HL, regs.C); break;
        case 0x72: Write8(regs.HL, regs.D); break;
        case 0x73: Write8(regs.HL, regs.E); break;
        case 0x74: Write8(regs.HL, regs.H); break;
        case 0x75: Write8(regs.HL, regs.L); break;
        case 0x76: halted = true; break; // HALT. Halt bug check
            // (HALT Bug logic is simplified: if IME=0 and IF&IE!=0, haltbug = true handled elsewhere ideally but we put it in Step)
        case 0x77: Write8(regs.HL, regs.A); break;
        case 0x78: regs.A = regs.B; break;
        case 0x79: regs.A = regs.C; break;
        case 0x7A: regs.A = regs.D; break;
        case 0x7B: regs.A = regs.E; break;
        case 0x7C: regs.A = regs.H; break;
        case 0x7D: regs.A = regs.L; break;
        case 0x7E: regs.A = Read8(regs.HL); break;
        case 0x7F: regs.A = regs.A; break;

        // ALU
        case 0x80: ADD_A(regs.B); break;
        case 0x81: ADD_A(regs.C); break;
        case 0x82: ADD_A(regs.D); break;
        case 0x83: ADD_A(regs.E); break;
        case 0x84: ADD_A(regs.H); break;
        case 0x85: ADD_A(regs.L); break;
        case 0x86: ADD_A(Read8(regs.HL)); break;
        case 0x87: ADD_A(regs.A); break;
        case 0x88: ADC_A(regs.B); break;
        case 0x89: ADC_A(regs.C); break;
        case 0x8A: ADC_A(regs.D); break;
        case 0x8B: ADC_A(regs.E); break;
        case 0x8C: ADC_A(regs.H); break;
        case 0x8D: ADC_A(regs.L); break;
        case 0x8E: ADC_A(Read8(regs.HL)); break;
        case 0x8F: ADC_A(regs.A); break;
        case 0x90: SUB_A(regs.B); break;
        case 0x91: SUB_A(regs.C); break;
        case 0x92: SUB_A(regs.D); break;
        case 0x93: SUB_A(regs.E); break;
        case 0x94: SUB_A(regs.H); break;
        case 0x95: SUB_A(regs.L); break;
        case 0x96: SUB_A(Read8(regs.HL)); break;
        case 0x97: SUB_A(regs.A); break;
        case 0x98: SBC_A(regs.B); break;
        case 0x99: SBC_A(regs.C); break;
        case 0x9A: SBC_A(regs.D); break;
        case 0x9B: SBC_A(regs.E); break;
        case 0x9C: SBC_A(regs.H); break;
        case 0x9D: SBC_A(regs.L); break;
        case 0x9E: SBC_A(Read8(regs.HL)); break;
        case 0x9F: SBC_A(regs.A); break;
        case 0xA0: AND_A(regs.B); break;
        case 0xA1: AND_A(regs.C); break;
        case 0xA2: AND_A(regs.D); break;
        case 0xA3: AND_A(regs.E); break;
        case 0xA4: AND_A(regs.H); break;
        case 0xA5: AND_A(regs.L); break;
        case 0xA6: AND_A(Read8(regs.HL)); break;
        case 0xA7: AND_A(regs.A); break;
        case 0xA8: XOR_A(regs.B); break;
        case 0xA9: XOR_A(regs.C); break;
        case 0xAA: XOR_A(regs.D); break;
        case 0xAB: XOR_A(regs.E); break;
        case 0xAC: XOR_A(regs.H); break;
        case 0xAD: XOR_A(regs.L); break;
        case 0xAE: XOR_A(Read8(regs.HL)); break;
        case 0xAF: XOR_A(regs.A); break;
        case 0xB0: OR_A(regs.B); break;
        case 0xB1: OR_A(regs.C); break;
        case 0xB2: OR_A(regs.D); break;
        case 0xB3: OR_A(regs.E); break;
        case 0xB4: OR_A(regs.H); break;
        case 0xB5: OR_A(regs.L); break;
        case 0xB6: OR_A(Read8(regs.HL)); break;
        case 0xB7: OR_A(regs.A); break;
        case 0xB8: CP_A(regs.B); break;
        case 0xB9: CP_A(regs.C); break;
        case 0xBA: CP_A(regs.D); break;
        case 0xBB: CP_A(regs.E); break;
        case 0xBC: CP_A(regs.H); break;
        case 0xBD: CP_A(regs.L); break;
        case 0xBE: CP_A(Read8(regs.HL)); break;
        case 0xBF: CP_A(regs.A); break;

        // Control
        case 0xC0: if(!regs.ZF()) { Tick4(); regs.PC = Pop16(); } break; // RET NZ
        case 0xC1: regs.BC = Pop16(); break;
        case 0xC2: { u16 a = Fetch16(); if(!regs.ZF()) { Tick4(); regs.PC = a; } } break;
        case 0xC3: { u16 a = Fetch16(); Tick4(); regs.PC = a; } break; // JP
        case 0xC4: { u16 a = Fetch16(); if(!regs.ZF()) { Tick4(); Push16(regs.PC); regs.PC = a; } } break;
        case 0xC5: Push16(regs.BC); break;
        case 0xC6: ADD_A(Fetch8()); break;
        case 0xC7: Tick4(); Push16(regs.PC); regs.PC = 0x00; break;
        case 0xC8: if(regs.ZF()) { Tick4(); regs.PC = Pop16(); } break;
        case 0xC9: regs.PC = Pop16(); Tick4(); break; // RET
        case 0xCA: { u16 a = Fetch16(); if(regs.ZF()) { Tick4(); regs.PC = a; } } break;
        case 0xCB: ExecuteCB(Fetch8()); break;
        case 0xCC: { u16 a = Fetch16(); if(regs.ZF()) { Tick4(); Push16(regs.PC); regs.PC = a; } } break;
        case 0xCD: { u16 a = Fetch16(); Tick4(); Push16(regs.PC); regs.PC = a; } break; // CALL
        case 0xCE: ADC_A(Fetch8()); break;
        case 0xCF: Tick4(); Push16(regs.PC); regs.PC = 0x08; break;
        case 0xD0: if(!regs.CF()) { Tick4(); regs.PC = Pop16(); } break;
        case 0xD1: regs.DE = Pop16(); break;
        case 0xD2: { u16 a = Fetch16(); if(!regs.CF()) { Tick4(); regs.PC = a; } } break;
        // D3 unmapped
        case 0xD4: { u16 a = Fetch16(); if(!regs.CF()) { Tick4(); Push16(regs.PC); regs.PC = a; } } break;
        case 0xD5: Push16(regs.DE); break;
        case 0xD6: SUB_A(Fetch8()); break;
        case 0xD7: Tick4(); Push16(regs.PC); regs.PC = 0x10; break;
        case 0xD8: if(regs.CF()) { Tick4(); regs.PC = Pop16(); } break;
        case 0xD9: regs.PC = Pop16(); IME = true; Tick4(); break; // RETI
        case 0xDA: { u16 a = Fetch16(); if(regs.CF()) { Tick4(); regs.PC = a; } } break;
        // DB unmapped
        case 0xDC: { u16 a = Fetch16(); if(regs.CF()) { Tick4(); Push16(regs.PC); regs.PC = a; } } break;
        // DD unmapped
        case 0xDE: SBC_A(Fetch8()); break;
        case 0xDF: Tick4(); Push16(regs.PC); regs.PC = 0x18; break;
        case 0xE0: Write8(0xFF00 + Fetch8(), regs.A); break; // LDH (a8), A
        case 0xE1: regs.HL = Pop16(); break;
        case 0xE2: Write8(0xFF00 + regs.C, regs.A); break; // LD (C), A
        // E3, E4 unmapped
        case 0xE5: Push16(regs.HL); break;
        case 0xE6: AND_A(Fetch8()); break;
        case 0xE7: Tick4(); Push16(regs.PC); regs.PC = 0x20; break;
        case 0xE8: regs.SP = ADD_SP_r8(); break;
        case 0xE9: regs.PC = regs.HL; break; // JP (HL)
        case 0xEA: Write8(Fetch16(), regs.A); break;
        // EB, EC unmapped
        // ED unmapped
        case 0xEE: XOR_A(Fetch8()); break;
        case 0xEF: Tick4(); Push16(regs.PC); regs.PC = 0x28; break;
        case 0xF0: regs.A = Read8(0xFF00 + Fetch8()); break; // LDH A, (a8)
        case 0xF1: regs.AF = Pop16() & 0xFFF0; break; // bit 0..3 of F are 0
        case 0xF2: regs.A = Read8(0xFF00 + regs.C); break;
        case 0xF3: IME = false; break; // DI
        // F4 unmapped
        case 0xF5: Push16(regs.AF); break;
        case 0xF6: OR_A(Fetch8()); break;
        case 0xF7: Tick4(); Push16(regs.PC); regs.PC = 0x30; break;
        case 0xF8: { u16 v = ADD_SP_r8(); regs.HL = v; } break; // LD HL, SP+r8
        case 0xF9: Tick4(); regs.SP = regs.HL; break; // LD SP, HL
        case 0xFA: regs.A = Read8(Fetch16()); break;
        case 0xFB: IMEPending = true; break; // EI
        // FC, FD unmapped
        case 0xFE: CP_A(Fetch8()); break;
        case 0xFF: Tick4(); Push16(regs.PC); regs.PC = 0x38; break;

        default:
            // Illegal opcode
            break;
    }
}

// ---------------------------------------------------------
// CB Opcodes
// ---------------------------------------------------------
u8 CPU::RLC(u8 val) { u8 c = val >> 7; u8 r = (val << 1) | c; regs.SetFlags(r==0,0,0,c); return r; }
u8 CPU::RRC(u8 val) { u8 c = val & 1; u8 r = (val >> 1) | (c << 7); regs.SetFlags(r==0,0,0,c); return r; }
u8 CPU::RL(u8 val) { u8 c = regs.CF()?1:0; u8 nc = val >> 7; u8 r = (val << 1) | c; regs.SetFlags(r==0,0,0,nc); return r; }
u8 CPU::RR(u8 val) { u8 c = regs.CF()?1:0; u8 nc = val & 1; u8 r = (val >> 1) | (c << 7); regs.SetFlags(r==0,0,0,nc); return r; }
u8 CPU::SLA(u8 val) { u8 c = val >> 7; u8 r = val << 1; regs.SetFlags(r==0,0,0,c); return r; }
u8 CPU::SRA(u8 val) { u8 c = val & 1; u8 r = (val >> 1) | (val & 0x80); regs.SetFlags(r==0,0,0,c); return r; }
u8 CPU::SWAP(u8 val) { u8 r = (val >> 4) | (val << 4); regs.SetFlags(r==0,0,0,0); return r; }
u8 CPU::SRL(u8 val) { u8 c = val & 1; u8 r = val >> 1; regs.SetFlags(r==0,0,0,c); return r; }
void CPU::BIT(u8 bit, u8 val) { regs.SetZF(!(val & (1 << bit))); regs.SetNF(false); regs.SetHF(true); }

void CPU::ExecuteCB(u8 op) {
    u8 regIdx = op & 0x07;
    u8 oper = op >> 3;

    auto ReadReg = [&](int i) -> u8 {
        switch(i) {
            case 0: return regs.B; case 1: return regs.C; case 2: return regs.D; case 3: return regs.E;
            case 4: return regs.H; case 5: return regs.L; case 6: return Read8(regs.HL); case 7: return regs.A;
            default: return 0;
        }
    };
    auto WriteReg = [&](int i, u8 val) {
        switch(i) {
            case 0: regs.B = val; break; case 1: regs.C = val; break; case 2: regs.D = val; break; case 3: regs.E = val; break;
            case 4: regs.H = val; break; case 5: regs.L = val; break; case 6: Write8(regs.HL, val); break; case 7: regs.A = val; break;
        }
    };

    u8 val = ReadReg(regIdx);

    if(oper < 8) { // Rotates/Shifts
        u8 res = 0;
        switch(oper) {
            case 0x0: res = RLC(val); break; case 0x1: res = RRC(val); break;
            case 0x2: res = RL(val); break;  case 0x3: res = RR(val); break;
            case 0x4: res = SLA(val); break; case 0x5: res = SRA(val); break;
            case 0x6: res = SWAP(val); break;case 0x7: res = SRL(val); break;
        }
        WriteReg(regIdx, res);
    } else if(oper < 16) { // BIT
        BIT(oper & 0x07, val);
    } else if(oper < 24) { // RES
        WriteReg(regIdx, val & ~(1 << (oper & 0x07)));
    } else { // SET
        WriteReg(regIdx, val | (1 << (oper & 0x07)));
    }
}
