#include <cstddef>
#include <iostream>
#include <ostream>
#include <iomanip>


#include "../headers/common.hpp"
#include "../headers/cpu.hpp"
#include "../headers/bus.hpp"
#include "../headers/instructions.hpp"
using namespace std;


gbCpu::gbCpu(Bus& bus, Instructions& instr, Timer& timer) 
    : bus(bus), instr(instr), timer(timer) {
    regs.pc = 0x100;
    regs.a = regs.b = regs.c = regs.d = regs.e = regs.f = regs.h = regs.l = 0;
}
void gbCpu::debug(){
    curr_ins = instr.Instruction_by_opcode(opcode);
    cout<<"'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''"<<endl;
    cout << "Current opcode: 0x" << uppercase << hex << static_cast<int>(opcode)<<" ("<<hex<<regs.pc-1<<" "<<hex<<regs.pc<<" "<<hex<<regs.pc+1<<")"<< endl;
    cout<<inst_name(curr_ins->type) << endl;
    cout << "flags: Z"<<regs.read_flag('Z')<<" C"<<regs.read_flag('C')<<" N"<<regs.read_flag('N')<<" H"<<regs.read_flag('H')<<endl;
    cout<<"PC: "<<regs.pc<<" AF: 0x"<<hex<<regs.read_reg(RT::AF)<<" BC: 0x"<<hex<<regs.read_reg(RT::BC)<<" DE: 0x"<<hex<<regs.read_reg(RT::DE)<<" HL: 0x"<<hex<<regs.read_reg(RT::HL)<<" SP: 0x"<<hex<<regs.read_reg(RT::SP)<<endl;
}

bool gbCpu::check_cond(){
    bool flag_z = regs.read_flag('Z');
    bool flag_c = regs.read_flag('C');
    
    switch(curr_ins->cond){
        case(CT::NONE): return true;
        case(CT::C): return flag_c;
        case(CT::NC): return !flag_c;
        case(CT::Z): return flag_z;
        case(CT::NZ): return !flag_z;
        default: return false;
    }
    return true;
}; 

void gbCpu::goto_addr(u16 addr, bool pushpc){
    if (check_cond()){
        if(pushpc){
            //stack implemnts
         }
        regs.pc = addr;
        timer.emu_cycles(1);
        return;
    }
    return;
}

void gbCpu::run(){
    int i=0;
    while (!halted) {
        mem_dest = 0;
        is_mem_dest = false;
        debug();
        fetch();
        decode();
        execute();

        i+=1;
    }
}

void gbCpu::fetch(){
    // cout<<"Reading: "<<regs.pc<<endl;
    opcode = bus.read(regs.pc);
    regs.pc++;
}

void gbCpu::decode(){
    curr_ins = instr.Instruction_by_opcode(opcode);
    if(curr_ins == NULL){
        cerr<<"Instruction not implented, opcode: 0x"<<hex<<static_cast<int>(opcode)<<" PC: "<<regs.pc<<endl;
        exit(-2);
    }
    switch(curr_ins->mode){
        
        case(AM::IMP):{
            break;
        }

        case(AM::R):{
            fetched_data = regs.read_reg(curr_ins->reg_1);
            break;
        }

        case(AM::R_D16):
        case(AM::D16):{
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            u8 hi = bus.read(regs.pc+1);
            timer.emu_cycles(1);
            fetched_data = lo | static_cast<uint16_t>(hi << 8);
            regs.pc+=2;
            break;
        }

        case AM::R_R:{
            fetched_data = regs.read_reg(curr_ins->reg_2);
            break;
        }

        case AM::A16_R:{
            // from Reg to memory 16b addr            
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            u8 hi = bus.read(regs.pc+1);
            timer.emu_cycles(1);
            
            is_mem_dest= true;
            mem_dest = (hi<<8|lo);
            regs.pc+=2;

            fetched_data = regs.read_reg(curr_ins->reg_2);
            break;
        }
        case AM::A8_R:{
            // from Reg to memory 16b addr            
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            is_mem_dest= true;
            mem_dest = lo;
            regs.pc+=2;

            fetched_data = regs.read_reg(curr_ins->reg_2);
            break;
        }

        case AM::R_A8:
        case AM::D8:
        case AM::HL_SPR:        //doubt
        case AM::R_D8:{
            fetched_data = bus.read(regs.pc);
            timer.emu_cycles(1);
            regs.pc++;
            break;
        }

        case AM::R_MR:{
            u16 addr = regs.read_reg(curr_ins->reg_2);
            if(curr_ins->reg_2 ==  RT::C) addr |= 0xff00;
            fetched_data = bus.read(addr);
            timer.emu_cycles(1);
            break;
        }

        case AM::MR_R:{
            fetched_data = regs.read_reg(curr_ins->reg_2);
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest=true;
            if (curr_ins->reg_1 == RT::C) {
                mem_dest |= 0xFF00;
            }
            break;
        }

        case AM::MR_D8:
            fetched_data = bus.read(regs.pc);
            timer.emu_cycles(1);
            regs.pc++;
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest= true;
            break;

        case AM::MR:{
            fetched_data = bus.read(regs.read_reg(curr_ins->reg_1));
            timer.emu_cycles(1);
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest= true;
            break;
        }

        case(AM::R_A16):{
            u8 lo = bus.read(regs.pc);
            timer.emu_cycles(1);
            u8 hi = bus.read(regs.pc+1);
            timer.emu_cycles(1);
            u16 addr = lo | (hi << 8);
            regs.pc+=2;
            fetched_data = bus.read(addr);
            break;
        }

        case AM::R_HLI:{
            fetched_data = bus.read(regs.read_reg(curr_ins->reg_2));
            timer.emu_cycles(1);
            regs.set_reg(RT::HL, regs.read_reg(RT::HL)+1);
            break;
        }
        case AM::R_HLD:{
            fetched_data = bus.read(regs.read_reg(curr_ins->reg_2));
            timer.emu_cycles(1);
            regs.set_reg(RT::HL, regs.read_reg(RT::HL)-1);
            break;
        }
        case AM::HLI_R:{
            fetched_data = regs.read_reg(curr_ins->reg_2);
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest = true;
            regs.set_reg(RT::HL, regs.read_reg(RT::HL)+1);
            break;
        }
        case AM::HLD_R:{
            fetched_data = regs.read_reg(curr_ins->reg_2);
            mem_dest = regs.read_reg(curr_ins->reg_1);
            is_mem_dest = true;
            regs.set_reg(RT::HL, regs.read_reg(RT::HL)-1);
            break;
        }
        default:
            cerr<<"Instruction not implented: "<<inst_name(curr_ins->type)<<" "<<hex<<regs.pc<<" "<<hex<<static_cast<int>(opcode)<<endl;
            exit(-2);
    }
}

void gbCpu::execute(){
    switch (curr_ins->type)
    {
    case IN::NONE:
        cerr<<"<None>"<<endl;
        break;

    case IN::NOP:
        break;

    case IN::JP:{
        cout<<"jump to: 0x"<<hex<<fetched_data<<endl;
        goto_addr(fetched_data, false);
        break;
    }
    case IN::XOR:
        //XOR A,A 0XAF
        regs.a^=static_cast<u8>(fetched_data);
        regs.set_flag(regs.a==0,0,0,0);
        break;

    case IN::LD:
        if(is_mem_dest){
            if (curr_ins->reg_2 >= RT::AF) bus.write16(mem_dest,fetched_data);
            else bus.write(mem_dest, fetched_data);
            timer.emu_cycles(1);
            break;
        }
        // LD HL,d16 0x21 | LD SP,d16 0x31 | LD C,d8 0x0E | LD B,d8 0x06 | LD HLD,A 0x32
        regs.set_reg(curr_ins->reg_1, fetched_data);
        break;
    
    case IN::LDH:
        if(curr_ins->reg_2 ==  RT::A) regs.set_reg(curr_ins->reg_1, bus.read(0xFF | fetched_data));
        else bus.write(mem_dest, regs.a);
        break;
        
    case IN::ADD:
        regs.set_reg(curr_ins->reg_1, regs.read_reg(curr_ins->reg_1)+fetched_data);
        break;

    case IN::DEC:
        regs.set_reg(curr_ins->reg_1, regs.read_reg(curr_ins->reg_1)-1);
        break;

    case IN::DI:
        interupt_en=false;
        break;

    case IN::JR:{
        u16 addr = regs.pc + abs(fetched_data & 0xFF);
        goto_addr(addr, false);
        break;
    }
    default:
        cerr<<"Execute not implmented: "<<inst_name(curr_ins->type)<<" OP: 0X"<<uppercase<<hex<<static_cast<int>(opcode)<<" PC: "<<regs.pc<<endl;
        exit(-3);
        break;
    }
    
}