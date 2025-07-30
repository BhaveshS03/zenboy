#include "cpu.hpp"
#include "bus.hpp"

#include <iostream>

void gbRegisters::set_flag(char flag_char, bool value) {
    switch (flag_char) {
        case 'Z':
        case 'z':
            if (value) f |= (1 << 7); else f &= ~(1 << 7);
            break;
        case 'N':
        case 'n':
            if (value) f |= (1 << 6); else f &= ~(1 << 6);
            break;
        case 'H':
        case 'h':
            if (value) f |= (1 << 5); else f &= ~(1 << 5);
            break;
        case 'C':
        case 'c':
            if (value) f |= (1 << 4); else f &= ~(1 << 4);
            break;
        default:
            // Handle invalid flag character, e.g., throw an exception or log an error
            break;
    }
}
void gbRegisters::toggle_flag(bool Z, bool N, bool H, bool C){
    if (Z==true) f ^= 1<<7;
    if (N==true) f ^= 1<<6;
    if (H==true) f ^= 1<<5;
    if (C==true) f ^= 1<<4;
}
void gbRegisters::clear_flag(bool Z,bool N,bool H,bool C){
    if (Z==true) f &= ~(1<<7);
    if (N==true) f &= ~(1<<6);
    if (H==true) f &= ~(1<<5);
    if (C==true) f &= ~(1<<4);
}
bool gbRegisters::read_flag(char flag){
    switch(flag){
        case 'Z': return (f>>7)&1;
        case 'N': return (f>>6)&1;
        case 'H': return (f>>5)&1;
        case 'C': return (f>>4)&1;
    }
    return 0;
}
u16 gbRegisters::read_reg(RT reg){
    switch(reg){
        case RT::A: return a;        
        case RT::B: return b;
        case RT::C: return c;
        case RT::D: return d;
        case RT::E: return e;
        case RT::H: return h;
        case RT::L: return l;
        case RT::AF: return (a << 8)|f;
        case RT::BC: return (b << 8)|c;
        case RT::DE: return (d << 8)|e;
        case RT::HL: return (h << 8)|l;
        case RT::SP: return sp;
        default: 
            std::invalid_argument("Invalid register");
            exit(-4);
            return 0;
    }
}

void gbRegisters::set_reg(RT reg, uint16_t value) {
    switch (reg) {
        case RT::A:  a = static_cast<uint8_t>(value); break; // Cast to 8-bit
        case RT::B:  b = static_cast<uint8_t>(value); break;
        case RT::C:  c = static_cast<uint8_t>(value); break;
        case RT::D:  d = static_cast<uint8_t>(value); break;
        case RT::E:  e = static_cast<uint8_t>(value); break;
        case RT::H:  h = static_cast<uint8_t>(value); break;
        case RT::L:  l = static_cast<uint8_t>(value); break;
        case RT::AF:
            a = static_cast<uint8_t>(value >> 8); // High byte
            f = static_cast<uint8_t>(value & 0xFF); // Low byte
            break;
        case RT::BC:
            b = static_cast<uint8_t>(value >> 8);
            c = static_cast<uint8_t>(value & 0xFF);
            break;
        case RT::DE:
            d = static_cast<uint8_t>(value >> 8);
            e = static_cast<uint8_t>(value & 0xFF);
            break;
        case RT::HL:
            h = static_cast<uint8_t>(value >> 8);
            l = static_cast<uint8_t>(value & 0xFF);
            break;
        case RT::SP: sp = value; break;
        case RT::PC: pc = value; break;
        default:
            // Same error handling as in read_reg
            std::invalid_argument("Invalid register");
            break;
    }
}

std::string gbRegisters::Register_by_Name(RT reg){
    switch (reg) {
        case RT::A: return "A";
        case RT::B: return "B";
        case RT::C: return "C";
        case RT::D: return "D";
        case RT::E: return "E";
        case RT::H: return "H";
        case RT::L: return "L";
        case RT::HL: return "HL";
        case RT::BC: return "BC";
        case RT::DE: return "DE";
        case RT::SP: return "SP";
        case RT::PC: return "PC";
        case RT::AF: return "AF";
        default: return "Unknown Register";
    }
}

u8 gbCpu::stack_pop(){
    return bus.read(regs.sp++);
};
u16 gbCpu::stack_pop16(){
    u8 hi = stack_pop();
    u8 lo = stack_pop();
    return (hi<<8)|lo; 
};
void gbCpu::stack_push(u8 data){
    regs.sp--;
    bus.write(regs.sp,data);
};
void gbCpu::stack_push16(u16 data){
    stack_push((data>>8) & 0xff);
    stack_push(data & 0xff);
};

u8 gbCpu::get_ie_register() {
    return ie_register;
}

void gbCpu::set_ie_register(uint8_t value) {
    ie_register = value;
}

void gbCpu::int_handle( u16 address) {
    stack_push16(regs.pc);
    regs.pc = address;
}

bool gbCpu::int_check(u16 address, interrupt_type it) {
    if (int_flags & it && ie_register & it) {
        int_handle(address);
        int_flags &= ~it;
        halted = false;
        interupt_en = false;
        return true;
    }
    return false;
}

void  gbCpu::cpu_handle_interrupts() {
    if (int_check(0x40, IT_VBLANK)) {

    } else if (int_check(0x48, IT_LCD_STAT)) {

    } else if (int_check(0x50, IT_TIMER)) {

    } else if (int_check(0x58, IT_SERIAL)) {

    } else if (int_check(0x60, IT_JOYPAD)) {

    } 
}
void gbCpu::request_interrupt(interrupt_type t) {
    int_flags |= t;
}

u8 gbCpu::get_int_flags(){
    return int_flags;
}

void gbCpu::set_int_flags(u8 value){
    int_flags = value;
}

void gbCpu::dbg_update(){
    if (bus.read(0xFF02) == 0x81) {
        char c = bus.read(0xFF01);
        dbg_msg[msg_size++] = c;
        bus.write(0xFF02, 0);
    }
};
void gbCpu::dbg_print(){   
    if (dbg_msg[0]) {
    // std::cerr<<"DBG:"<<dbg_msg<<std::endl;
}};