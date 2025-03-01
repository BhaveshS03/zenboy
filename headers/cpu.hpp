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

    void set_flag(bool Z, bool N, bool H, bool C);
    void toggle_flag(bool Z, bool N, bool H, bool C);
    void clear_flag(bool Z,bool N,bool H,bool C);
    bool read_flag(char flag);
    u16 read_reg(RT reg);
    void set_reg(RT reg, uint16_t value);
    std::string Register_by_Name(RT reg);
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

        u8 stack_pop();
        u16 stack_pop16();
        void stack_push(u8 data);
        void stack_push16(u16 data);

    };