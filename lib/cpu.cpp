#include <cstddef>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <cmath>

#include "../headers/common.hpp"
#include "../headers/cpu.hpp"
#include "../headers/bus.hpp"
#include "../headers/instructions.hpp"

#define CPU_FLAG_Z regs.read_flag('Z')
#define CPU_FLAG_N regs.read_flag('N')
#define CPU_FLAG_H regs.read_flag('H')
#define CPU_FLAG_C regs.read_flag('C')


using namespace std;

gbCpu::gbCpu(Bus& bus, Instructions& instr, Timer& timer)
    : bus(bus), instr(instr), timer(timer), halted(false), interupt_en(false), enabling_ime(false) {
    
    // Initialize registers with the provided values
    regs.a = 0x01;
    regs.f = 0xB0;
    regs.b = 0x00;
    regs.c = 0x13;
    regs.d = 0x00;
    regs.e = 0xD8;
    regs.h = 0x01;
    regs.l = 0x4D;
    regs.sp = 0xFFFE;
    regs.pc = 0x0100;

}

// void gbCpu::debug() {
//     curr_ins = instr.Instruction_by_opcode(opcode); // Re-fetch to ensure curr_ins is valid for debug
//     cout << "'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''" << endl;
//     cout << "Current opcode: 0x" << uppercase << hex << static_cast<int>(opcode) << " (" << hex << regs.pc - 1 << " " << hex << regs.pc << " " << hex << regs.pc + 1 << ")" << endl;
//     if (curr_ins) {
//         cout << inst_name(curr_ins->type) << endl;
//     } else {
//         cout << "UNKNOWN INSTRUCTION" << endl;
//     }
//     cout << "flags: Z" << regs.read_flag('Z') << " C" << regs.read_flag('C') << " N" << regs.read_flag('N') << " H" << regs.read_flag('H') << endl;
//     cout << "PC: " << regs.pc << " AF: 0x" << hex << regs.read_reg(RT::AF) << " BC: 0x" << hex << regs.read_reg(RT::BC) << " DE: 0x" << hex << regs.read_reg(RT::DE) << " HL: 0x" << hex << regs.read_reg(RT::HL) << " SP: 0x" << hex << regs.read_reg(RT::SP) << endl;
// }

void gbCpu::debug(){
    std::cout << std::hex << std::uppercase << std::setfill('0'); // Set formatting for hex output

    std::cout << "A:" << std::setw(2) << static_cast<int>(regs.a)
              << " F:" << std::setw(2) << static_cast<int>(regs.f)
              << " B:" << std::setw(2) << static_cast<int>(regs.b)
              << " C:" << std::setw(2) << static_cast<int>(regs.c)
              << " D:" << std::setw(2) << static_cast<int>(regs.d)
              << " E:" << std::setw(2) << static_cast<int>(regs.e)
              << " H:" << std::setw(2) << static_cast<int>(regs.h)
              << " L:" << std::setw(2) << static_cast<int>(regs.l);

    std::cout << " SP:" << std::setw(4) << regs.sp
              << " PC:" << std::setw(4) << regs.pc;

    // For PCMEM, we might want to reset setfill to default space if subsequent output
    // isn't intended to be zero-padded, or just ensure consistency for bytes.
    std::cout << " PCMEM:";
    std::cout << std::setw(2) << static_cast<int>(bus.read(regs.pc)) << ","
              << std::setw(2) << static_cast<int>(bus.read(regs.pc+1)) << ","
              << std::setw(2) << static_cast<int>(bus.read(regs.pc+2)) << ","
              << std::setw(2) << static_cast<int>(bus.read(regs.pc+3)) << std::endl;

    // Optional: Reset stream format to default if this is not the only place hex is used
    std::cout << std::dec << std::noshowbase << std::setfill(' ');
}

bool gbCpu::check_cond() {
    bool flag_z = regs.read_flag('Z');
    bool flag_c = regs.read_flag('C');

    switch (curr_ins->cond) {
        case (CT::NONE): return true;
        case (CT::C): return flag_c;
        case (CT::NC): return !flag_c;
        case (CT::Z): return flag_z;
        case (CT::NZ): return !flag_z;
        default: return false; // Should not happen
    }
}

void gbCpu::goto_addr(u16 addr, bool pushpc) {
    if (check_cond()) {
        if (pushpc) {
            timer.emu_cycles(2); 
            stack_push16(regs.pc);
        }
        regs.pc = addr;
        timer.emu_cycles(1);
        return;
    }
    return;
}

void gbCpu::run() {
    int i = 0;
    while (!halted) {
        mem_dest = 0;
        is_mem_dest = false;
        debug();
        fetch();
        decode();
        execute();
        i += 1;

        if (enabling_ime) {
            interupt_en = true;
            enabling_ime = false; // Reset after one instruction
        }
    }
}

void gbCpu::fetch() {
    opcode = bus.read(regs.pc);
    regs.pc++;
}

void gbCpu::decode() {
    curr_ins = instr.Instruction_by_opcode(opcode);
    if (curr_ins == NULL) {
        cerr << "Instruction not implemented, opcode: 0x" << hex << static_cast<int>(opcode) << " PC: " << regs.pc << endl;
        exit(-2);
    }
    switch (curr_ins->mode) {
        case (AM::IMP): {
            break;
        }
        case (AM::R): {
            fetched_data = regs.read_reg(curr_ins->reg_1);
            break;
        }
        case (AM::R_D16):
        case (AM::D16): {
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            u8 hi = bus.read(regs.pc + 1);
            timer.emu_cycles(1);
            fetched_data = lo | static_cast<uint16_t>(hi << 8);
            regs.pc += 2;
            break;
        }
        case AM::R_R: {
            fetched_data = regs.read_reg(curr_ins->reg_2);
            break;
        }
        case AM::A16_R: {
            // from Reg to memory 16b addr
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            u8 hi = bus.read(regs.pc + 1);
            timer.emu_cycles(1);
            is_mem_dest = true;
            mem_dest = (hi << 8 | lo);
            regs.pc += 2;
            fetched_data = regs.read_reg(curr_ins->reg_2);
            break;
        }
        case AM::A8_R: {
            // from Reg to memory 8b addr
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            is_mem_dest = true;
            mem_dest = 0xFF00 | lo; // A8_R implies high RAM (0xFF00-0xFFFF)
            regs.pc++; // Only 1 byte offset
            fetched_data = regs.read_reg(curr_ins->reg_2);
            break;
        }
        case AM::R_A8:
        case AM::D8:
        case AM::HL_SPR:
        case AM::R_D8: {
            fetched_data = bus.read(regs.pc);
            timer.emu_cycles(1);
            regs.pc++;
            break;
        }
        case AM::R_MR: {
            u16 addr = regs.read_reg(curr_ins->reg_2);
            if (curr_ins->reg_2 == RT::C) addr |= 0xff00; // For (C) addressing
            fetched_data = bus.read(addr);
            timer.emu_cycles(1);
            break;
        }
        case AM::MR_R: {
            fetched_data = regs.read_reg(curr_ins->reg_2);
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest = true;
            if (curr_ins->reg_1 == RT::C) {
                mem_dest |= 0xFF00; // For (C) addressing
            }
            break;
        }
        case AM::MR_D8:
            fetched_data = bus.read(regs.pc);
            timer.emu_cycles(1);
            regs.pc++;
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest = true;
            break;
        case AM::MR: { 
            // This seems to be a destination for some operations, but also fetching source.
           // Review specific opcodes using AM::MR
            // If it's used as a source (e.g., INC (HL)), then fetched_data should be the value at mem_dest
            // If it's just a destination, fetched_data might not be relevant here or set elsewhere.
            // For now, mirroring existing behavior:
            fetched_data = bus.read(regs.read_reg(curr_ins->reg_1));
            timer.emu_cycles(1);
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest = true;
            break;
        }
        case (AM::R_A16): {
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            u8 hi = bus.read(regs.pc + 1);
            timer.emu_cycles(1);
            u16 addr = lo | (hi << 8);
            regs.pc += 2;
            fetched_data = bus.read(addr); // For (A16) as source
            break;
        }
        case AM::HLI_R: { // (HLI), R -> LD (HL+), R
            fetched_data = regs.read_reg(curr_ins->reg_2);
            mem_dest = regs.read_reg(curr_ins->reg_1); // HL
            is_mem_dest = true;
            regs.set_reg(RT::HL, regs.read_reg(RT::HL) + 1);
            break;
        }
        case AM::HLD_R: { // (HLD), R -> LD (HL-), R
            fetched_data = regs.read_reg(curr_ins->reg_2);
            mem_dest = regs.read_reg(curr_ins->reg_1); // HL
            is_mem_dest = true;
            regs.set_reg(RT::HL, regs.read_reg(RT::HL) - 1);
            break;
        }
        case AM::R_HLI: { // R, (HLI) -> LD R, (HL+)
            fetched_data = bus.read(regs.read_reg(curr_ins->reg_2));
            timer.emu_cycles(1);
            regs.set_reg(RT::HL, regs.read_reg(RT::HL) + 1);
            break;
        }
        case AM::R_HLD: { // R, (HLD) -> LD R, (HL-)
            fetched_data = bus.read(regs.read_reg(curr_ins->reg_2));
            timer.emu_cycles(1);
            regs.set_reg(RT::HL, regs.read_reg(RT::HL) - 1);
            break;
        }
        default:
            cerr << "Instruction not implemented (decode): " << inst_name(curr_ins->type) << " " << hex << regs.pc << " " << hex << static_cast<int>(opcode) << endl;
            exit(-2);
    }
}

// --- Helper Functions for Execution (Private members of gbCpu) ---

void gbCpu::cpu_set_flags(int8_t z, int8_t n, int8_t h, int8_t c) {
    if (z != -1) {
        regs.set_flag('Z', z);
    }
    if (n != -1) {
        regs.set_flag('N', n);
    }
    if (h != -1) {
        regs.set_flag('H', h);
    }
    if (c != -1) {
        regs.set_flag('C', c);
    }
}

bool gbCpu::is_16_bit(RT reg_type) {
    return reg_type == RT::AF || reg_type == RT::BC || reg_type == RT::DE || reg_type == RT::HL || reg_type == RT::SP;
}

void gbCpu::proc_none() {
    cerr << "INVALID INSTRUCTION!" << endl;
    exit(-7);
}

void gbCpu::proc_nop() {
    // No operation, do nothing
}

void gbCpu::proc_cb() {
    u8 op = fetched_data;
    RT reg = RT::NONE;

    // Decode register based on lower 3 bits (0b111)
    u8 reg_code = op & 0b111;
    switch (reg_code) {
        case 0b000: reg = RT::B; break;
        case 0b001: reg = RT::C; break;
        case 0b010: reg = RT::D; break;
        case 0b011: reg = RT::E; break;
        case 0b100: reg = RT::H; break;
        case 0b101: reg = RT::L; break;
        case 0b110: reg = RT::HL; break; // (HL)
        case 0b111: reg = RT::A; break;
    }

    u8 bit = (op >> 3) & 0b111;
    u8 bit_op = (op >> 6) & 0b11;

    u8 reg_val = 0;
    if (reg == RT::HL) {
        reg_val = bus.read(regs.read_reg(RT::HL));
        timer.emu_cycles(1); // Additional cycle for (HL) access
    } else {
        reg_val = regs.read_reg(reg);
    }
    timer.emu_cycles(1); // General cycle for CB instruction

    switch (bit_op) {
        case 1: { // BIT
            cpu_set_flags(!(reg_val & (1 << bit)), 0, 1, -1);
            break;
        }
        case 2: { // RST (RESET BIT)
            reg_val &= ~(1 << bit);
            if (reg == RT::HL) {
                bus.write(regs.read_reg(RT::HL), reg_val);
                timer.emu_cycles(1); // Additional cycle for write to (HL)
            } else {
                regs.set_reg(reg, reg_val);
            }
            break;
        }
        case 3: { // SET (SET BIT)
            reg_val |= (1 << bit);
            if (reg == RT::HL) {
                bus.write(regs.read_reg(RT::HL), reg_val);
                timer.emu_cycles(1); // Additional cycle for write to (HL)
            } else {
                regs.set_reg(reg, reg_val);
            }
            break;
        }
        case 0: { // Rotations/Shifts
            switch (bit) {
                case 0: { // RLC
                    bool setC = false;
                    u8 result = (reg_val << 1) & 0xFF;
                    if ((reg_val & (1 << 7)) != 0) {
                        result |= 1;
                        setC = true;
                    }
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), result); timer.emu_cycles(1); } else { regs.set_reg(reg, result); }
                    cpu_set_flags(result == 0, 0, 0, setC);
                    break;
                }
                case 1: { // RRC
                    u8 old = reg_val;
                    reg_val >>= 1;
                    reg_val |= (old << 7);
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), reg_val); timer.emu_cycles(1); } else { regs.set_reg(reg, reg_val); }
                    cpu_set_flags(!reg_val, 0, 0, old & 1);
                    break;
                }
                case 2: { // RL
                    u8 old = reg_val;
                    u8 cf = CPU_FLAG_C;
                    reg_val <<= 1;
                    reg_val |= cf;
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), reg_val); timer.emu_cycles(1); } else { regs.set_reg(reg, reg_val); }
                    cpu_set_flags(!reg_val, 0, 0, !!(old & 0x80));
                    break;
                }
                case 3: { // RR
                    u8 old = reg_val;
                    reg_val >>= 1;
                    reg_val |= (CPU_FLAG_C << 7);
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), reg_val); timer.emu_cycles(1); } else { regs.set_reg(reg, reg_val); }
                    cpu_set_flags(!reg_val, 0, 0, old & 1);
                    break;
                }
                case 4: { // SLA (Shift Left Arithmetic)
                    u8 old = reg_val;
                    reg_val <<= 1;
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), reg_val); timer.emu_cycles(1); } else { regs.set_reg(reg, reg_val); }
                    cpu_set_flags(!reg_val, 0, 0, !!(old & 0x80));
                    break;
                }
                case 5: { // SRA (Shift Right Arithmetic)
                    u8 u = (int8_t)reg_val >> 1; // Signed shift to preserve bit 7
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), u); timer.emu_cycles(1); } else { regs.set_reg(reg, u); }
                    cpu_set_flags(!u, 0, 0, reg_val & 1);
                    break;
                }
                case 6: { // SWAP
                    reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), reg_val); timer.emu_cycles(1); } else { regs.set_reg(reg, reg_val); }
                    cpu_set_flags(reg_val == 0, 0, 0, 0);
                    break;
                }
                case 7: { // SRL (Shift Right Logical)
                    u8 u = reg_val >> 1; // Unsigned shift
                    if (reg == RT::HL) { bus.write(regs.read_reg(RT::HL), u); timer.emu_cycles(1); } else { regs.set_reg(reg, u); }
                    cpu_set_flags(!u, 0, 0, reg_val & 1);
                    break;
                }
            }
            break;
        }
    }
}

void gbCpu::proc_rlca() {
    u8 u = regs.a;
    bool c = (u >> 7) & 1;
    u = (u << 1) | c;
    regs.a = u;
    cpu_set_flags(0, 0, 0, c);
}

void gbCpu::proc_rrca() {
    u8 b = regs.a & 1;
    regs.a >>= 1;
    regs.a |= (b << 7);
    cpu_set_flags(0, 0, 0, b);
}

void gbCpu::proc_rla() {
    u8 u = regs.a;
    u8 cf = CPU_FLAG_C;
    u8 c = (u >> 7) & 1;
    regs.a = (u << 1) | cf;
    cpu_set_flags(0, 0, 0, c);
}

void gbCpu::proc_rra() {
    u8 carry = CPU_FLAG_C;
    u8 new_c = regs.a & 1;
    regs.a >>= 1;
    regs.a |= (carry << 7);
    cpu_set_flags(0, 0, 0, new_c);
}

void gbCpu::proc_stop() {
    cerr << "STOPPING! (Instruction not fully implemented)" << endl;
    // In a real emulator, this would handle STOP mode
}

void gbCpu::proc_daa() {
    u8 u = 0;
    int fc = 0;
    if (CPU_FLAG_H || (!CPU_FLAG_N && (regs.a & 0xF) > 9)) {
        u = 6;
    }
    if (CPU_FLAG_C || (!CPU_FLAG_N && regs.a > 0x99)) {
        u |= 0x60;
        fc = 1;
    }
    regs.a += CPU_FLAG_N ? -u : u;
    cpu_set_flags(regs.a == 0, -1, 0, fc);
}

void gbCpu::proc_cpl() {
    regs.a = ~regs.a;
    cpu_set_flags(-1, 1, 1, -1);
}

void gbCpu::proc_scf() {
    cpu_set_flags(-1, 0, 0, 1);
}

void gbCpu::proc_ccf() {
    cpu_set_flags(-1, 0, 0, CPU_FLAG_C ^ 1);
}

void gbCpu::proc_halt() {
    halted = true;
}

void gbCpu::proc_and() {
    regs.a &= fetched_data;
    cpu_set_flags(regs.a == 0, 0, 1, 0);
}

void gbCpu::proc_xor() {
    regs.a ^= fetched_data & 0xFF;
    cpu_set_flags(regs.a == 0, 0, 0, 0);
}

void gbCpu::proc_or() {
    regs.a |= fetched_data & 0xFF;
    cpu_set_flags(regs.a == 0, 0, 0, 0);
}

void gbCpu::proc_cp() {
    int n = (int)regs.a - (int)fetched_data;
    cpu_set_flags(n == 0, 1,
                  ((int)regs.a & 0x0F) - ((int)fetched_data & 0x0F) < 0, n < 0);
}

void gbCpu::proc_di() {
    interupt_en = false;
}

void gbCpu::proc_ei() {
    enabling_ime = true; // Set a flag to enable interrupts after the next instruction
}

void gbCpu::proc_ld() {
    if (is_mem_dest) {
        if (is_16_bit(curr_ins->reg_2) || is_16_bit(curr_ins->reg_1)) { // Check if source or dest is 16-bit to determine write size
            // If reg_2 is 16-bit (like LD (A16), SP), fetched_data is 16-bit
            // If reg_1 is 16-bit (like LD SP, (HL)), fetched_data is also 16-bit after being read.
            bus.write16(mem_dest, fetched_data);
            timer.emu_cycles(1);
        } else {
            bus.write(mem_dest, fetched_data & 0xFF); // Ensure 8-bit write
        }
        timer.emu_cycles(1); // Additional cycle for memory write
        return;
    }

    if (curr_ins->mode == AM::HL_SPR) {
        // LD HL, SP+r8
        s8 signed_offset = static_cast<s8>(fetched_data);
        u16 sp_val = regs.read_reg(RT::SP);

        u8 hflag = ((sp_val & 0xF) + (signed_offset & 0xF)) >= 0x10;
        u8 cflag = ((sp_val & 0xFF) + (signed_offset & 0xFF)) >= 0x100;

        cpu_set_flags(0, 0, hflag, cflag);
        regs.set_reg(curr_ins->reg_1, sp_val + signed_offset);
        timer.emu_cycles(1); 
        return;
    }

    // Standard LD R, N or LD R1, R2
    regs.set_reg(curr_ins->reg_1, fetched_data);
}

void gbCpu::proc_ldh() {
    if (curr_ins->reg_1 == RT::A) { // LDH A, (a8)
        regs.set_reg(curr_ins->reg_1, bus.read(0xFF00 | fetched_data));
    } else { // LDH (a8), A
        bus.write(mem_dest, regs.a); // mem_dest would be 0xFF00 | offset
    }
    timer.emu_cycles(1); 
}

void gbCpu::proc_jp() {
    goto_addr(fetched_data, false);
}

void gbCpu::proc_jr() {
    s8 rel = static_cast<s8>(fetched_data); // fetched_data is u8, cast to s8 for signed relative jump
    u16 addr = regs.pc + rel;
    goto_addr(addr, false);
}

void gbCpu::proc_call() {
    goto_addr(fetched_data, true);
}

void gbCpu::proc_rst() {
    // curr_ins->param holds the RST address (0x00, 0x08, etc.)
    goto_addr(curr_ins->param, true);
}

void gbCpu::proc_ret() {
    if (curr_ins->cond != CT::NONE) {
        timer.emu_cycles(1); // Additional cycle if condition is present
    }
    if (check_cond()) {
        u16 lo = bus.read(regs.sp);
        timer.emu_cycles(1);
        u16 hi = bus.read(regs.sp + 1);
        timer.emu_cycles(1);
        u16 n = (hi << 8) | lo;
        regs.pc = n;
        regs.sp += 2; // Increment SP after pop
        timer.emu_cycles(1); 
    }
}

void gbCpu::proc_reti() {
    interupt_en = true;
    proc_ret(); // RETI is essentially RET + EI
}

void gbCpu::proc_pop() {
    u16 lo = bus.read(regs.sp);
    timer.emu_cycles(1);
    u16 hi = bus.read(regs.sp + 1);
    timer.emu_cycles(1);
    u16 n = (hi << 8) | lo;
    regs.sp += 2; // Increment SP after pop

    regs.set_reg(curr_ins->reg_1, n);

    if (curr_ins->reg_1 == RT::AF) {
        // When popping to AF, the lower 4 bits of F (flags) are always 0
        regs.set_reg(curr_ins->reg_1, n & 0xFFF0);
    }
}

void gbCpu::proc_push() {
    u16 val = regs.read_reg(curr_ins->reg_1);
    u8 hi = (val >> 8) & 0xFF;
    u8 lo = val & 0xFF;

    regs.sp -= 1;
    bus.write(regs.sp, hi);
    timer.emu_cycles(1);

    regs.sp -= 1;
    bus.write(regs.sp, lo);
    timer.emu_cycles(1);

    timer.emu_cycles(1);
}

void gbCpu::proc_inc() {
    u16 val;
    bool sixteen_bit = is_16_bit(curr_ins->reg_1);

    if (curr_ins->reg_1 == RT::HL && curr_ins->mode == AM::MR) { // INC (HL)
        val = bus.read(regs.read_reg(RT::HL)) + 1;
        val &= 0xFF; // Ensure 8-bit result
        bus.write(regs.read_reg(RT::HL), val);
        timer.emu_cycles(1); // For memory write
    } else {
        val = regs.read_reg(curr_ins->reg_1) + 1;
        regs.set_reg(curr_ins->reg_1, val);
    }

    if (sixteen_bit) {
        timer.emu_cycles(1); // For 16-bit register increment
        return; // 16-bit INC does not affect Z, N, H flags
    }

    // For 8-bit INC
    cpu_set_flags((val & 0xFF) == 0, 0, ((val & 0x0F) == 0x00), -1);
}

void gbCpu::proc_dec() {
    u16 val;
    bool sixteen_bit = is_16_bit(curr_ins->reg_1);

    if (curr_ins->reg_1 == RT::HL && curr_ins->mode == AM::MR) { // DEC (HL)
        val = bus.read(regs.read_reg(RT::HL)) - 1;
        val &= 0xFF; // Ensure 8-bit result
        bus.write(regs.read_reg(RT::HL), val);
        timer.emu_cycles(1); // For memory write
    } else {
        val = regs.read_reg(curr_ins->reg_1) - 1;
        regs.set_reg(curr_ins->reg_1, val);
    }

    if (sixteen_bit) {
        timer.emu_cycles(1); // For 16-bit register decrement
        return; // 16-bit DEC does not affect Z, N, H flags
    }

    // For 8-bit DEC
    cpu_set_flags((val & 0xFF) == 0, 1, ((val & 0x0F) == 0x0F), -1);
}

void gbCpu::proc_sub() {
    u16 reg1_val = regs.read_reg(curr_ins->reg_1); // Assuming reg_1 is A
    u16 val = reg1_val - fetched_data;

    int z = (val & 0xFF) == 0;
    int h = ((int)reg1_val & 0x0F) - ((int)fetched_data & 0x0F) < 0;
    int c = ((int)reg1_val) - ((int)fetched_data) < 0;

    regs.set_reg(curr_ins->reg_1, val & 0xFF);
    cpu_set_flags(z, 1, h, c);
}

void gbCpu::proc_sbc() {
    u16 reg1_val = regs.read_reg(curr_ins->reg_1); // Assuming reg_1 is A
    u16 val = fetched_data + CPU_FLAG_C; // Value to subtract + carry

    int z = (reg1_val - val) == 0;
    int h = ((int)reg1_val & 0xF) - ((int)fetched_data & 0xF) - ((int)CPU_FLAG_C) < 0;
    int c = ((int)reg1_val) - ((int)fetched_data) - ((int)CPU_FLAG_C) < 0;

    regs.set_reg(curr_ins->reg_1, (reg1_val - val) & 0xFF);
    cpu_set_flags(z, 1, h, c);
}

void gbCpu::proc_adc() {
    u16 u = fetched_data;
    u16 a = regs.a;
    u16 c = CPU_FLAG_C;

    u16 result = (a + u + c) & 0xFF;
    regs.a = result;

    cpu_set_flags(result == 0, 0,
                  (a & 0xF) + (u & 0xF) + c > 0xF, // Half-carry
                  a + u + c > 0xFF); // Carry
}

void gbCpu::proc_add() {
    u32 val;
    bool is_16bit_reg1 = is_16_bit(curr_ins->reg_1);

    if (is_16bit_reg1) {
        val = regs.read_reg(curr_ins->reg_1) + fetched_data;
        timer.emu_cycles(1);
    } else { // 8-bit ADD
        val = regs.read_reg(curr_ins->reg_1) + fetched_data;
    }

    int z = -1;
    int h;
    int c;

    if (is_16bit_reg1) {
        if (curr_ins->reg_1 == RT::SP) { // ADD SP, r8
            z = 0; // Z flag is always 0 for ADD SP, r8
            h = ( (regs.read_reg(RT::SP) & 0xF) + (fetched_data & 0xF) ) >= 0x10;
            c = ( (regs.read_reg(RT::SP) & 0xFF) + (fetched_data & 0xFF) ) >= 0x100;
        } else { // ADD HL, BC/DE/HL/SP (16-bit adds)
            z = -1; // Z flag unaffected for HL adds
            h = ( (regs.read_reg(curr_ins->reg_1) & 0xFFF) + (fetched_data & 0xFFF) ) >= 0x1000;
            c = ( (u32)regs.read_reg(curr_ins->reg_1) + (u32)fetched_data ) >= 0x10000;
        }
    } else { // 8-bit ADD (ADD A, R / A, D8 / A, (HL))
        z = (val & 0xFF) == 0;
        h = ((regs.read_reg(curr_ins->reg_1) & 0xF) + (fetched_data & 0xF)) >= 0x10;
        c = ((regs.read_reg(curr_ins->reg_1) & 0xFF) + (fetched_data & 0xFF)) >= 0x100;
    }

    regs.set_reg(curr_ins->reg_1, val & 0xFFFF); // Store result (handle 16-bit correctly)
    cpu_set_flags(z, 0, h, c); // N flag is always 0 for ADD
}

// --- Main Execute Function ---
void gbCpu::execute() {
    switch (curr_ins->type) {
        case IN::NONE:    proc_none(); break;
        case IN::NOP:     proc_nop(); break;
        case IN::JP:      proc_jp(); break;
        case IN::XOR:     proc_xor(); break;
        case IN::LD:      proc_ld(); break;
        case IN::LDH:     proc_ldh(); break;
        case IN::ADD:     proc_add(); break;
        case IN::DEC:     proc_dec(); break;
        case IN::DI:      proc_di(); break;
        case IN::JR:      proc_jr(); break;
        case IN::RRA:     proc_rra(); break;
        case IN::RRCA:    proc_rrca(); break;
        case IN::RLA:     proc_rla(); break;
        case IN::RLCA:    proc_rlca(); break;
        case IN::OR:      proc_or(); break;
        case IN::INC:     proc_inc(); break;
        case IN::CALL:    proc_call(); break;
        case IN::RET:     proc_ret(); break;
        case IN::RST:     proc_rst(); break;
        case IN::POP:     proc_pop(); break;
        case IN::PUSH:    proc_push(); break;
        case IN::SUB:     proc_sub(); break;
        case IN::SBC:     proc_sbc(); break;
        case IN::ADC:     proc_adc(); break;
        case IN::AND:     proc_and(); break;
        case IN::CP:      proc_cp(); break;
        case IN::CB:      proc_cb(); break;
        case IN::STOP:    proc_stop(); break;
        case IN::HALT:    proc_halt(); break;
        case IN::DAA:     proc_daa(); break;
        case IN::CPL:     proc_cpl(); break;
        case IN::SCF:     proc_scf(); break;
        case IN::CCF:     proc_ccf(); break;
        case IN::EI:      proc_ei(); break;
        case IN::RETI:    proc_reti(); break;
        default:
            cerr << "Execute not implemented: " << inst_name(curr_ins->type) << " OP: 0X" << uppercase << hex << static_cast<int>(opcode) << " PC: " << regs.pc << endl;
            exit(-3);
            break;
    }
}