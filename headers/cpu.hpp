#pragma  once

#include "instructions.hpp"
#include "bus.hpp"
#include "timer.hpp"
#include "common.hpp"

class gbRegisters{
public:
    u8 a;
    u8 f;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 h;
    u8 l;
    u16 pc;
    u16 sp;

    void set_flag(bool Z, bool N, bool H, bool C){
        if (Z==true) f |= 1<<7;
        if (N==true) f |= 1<<6;
        if (H==true) f |= 1<<5;
        if (C==true) f |= 1<<4;
    }
    void toggle_flag(bool Z, bool N, bool H, bool C){
        if (Z==true) f ^= 1<<7;
        if (N==true) f ^= 1<<6;
        if (H==true) f ^= 1<<5;
        if (C==true) f ^= 1<<4;
    }
    void clear_flag(bool Z,bool N,bool H,bool C){
        if (Z==true) f &= ~(1<<7);
        if (N==true) f &= ~(1<<6);
        if (H==true) f &= ~(1<<5);
        if (C==true) f &= ~(1<<4);
    }
    bool read_flag(char flag){
        switch(flag){
            case 'Z': return (f<<7)&1;
            case 'N': return (f<<6)&1;
            case 'H': return (f<<5)&1;
            case 'C': return (f<<4)&1;
        }
        return 0;
    }
    u16 read_reg(RT reg){
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
    void set_reg(RT reg, uint16_t value) {
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
            default:
                // Same error handling as in read_reg
                std::invalid_argument("Invalid register");
                break;
        }
    }

    std::string Register_by_Name(RT reg){
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
};


class gbCpu{
    public:
        gbCpu(Bus& bus, Instructions& instr, Timer& timer);
        void run();
    private:
        gbRegisters regs;
        const InstructionData* curr_ins;
        u16 fetched_data;
        
        Bus& bus;         // Reference to the memory interface
        Instructions& instr; // Reference to the instructions handler
        Timer& timer;        // Reference to the timer interface

        void fetch();
        void decode();
        void execute();
        void debug();

        u8 opcode;
        u16 mem_dest;
        bool is_mem_dest = false;
        bool check_cond();
        void goto_addr(u16 addr, bool pushpc);
        bool interupt_en = true;

        bool halted = false;
};