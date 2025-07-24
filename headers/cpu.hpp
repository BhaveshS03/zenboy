#pragma  once

#include "instructions.hpp"
#include "bus.hpp"
#include "timer.hpp"
#include "common.hpp"

class Bus;

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

    void set_flag(char flag_char, bool value);
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
        void debug();
        bool check_cond();
        void goto_addr(u16 addr, bool pushpc);
        void run();
        void fetch();
        void decode();
        void execute();
        void set_ie_register(uint8_t value);
        u8 get_ie_register();

    private:
        gbRegisters regs;
        const InstructionData* curr_ins;
        u16 fetched_data;
        
        Bus& bus;         // Reference to the memory interface
        Instructions& instr; // Reference to the instructions handler
        Timer& timer;        // Reference to the timer interface

        u8 opcode;
        u16 mem_dest;
        u8 ie_register = 0;
        u8 int_flags = 0;
        bool interupt_en = true;
        bool enabling_ime = false;
        bool is_mem_dest = false;
        bool halted = false;

        u8 stack_pop();
        u16 stack_pop16();
        void stack_push(u8 data);
        void stack_push16(u16 data);
        void cpu_set_flags(int8_t z, int8_t n, int8_t h, int8_t c);
        bool is_16_bit(RT reg_type);

        void proc_none();
        void proc_nop();
        void proc_cb();
        void proc_rlca();
        void proc_rrca();
        void proc_rla();
        void proc_stop();
        void proc_daa();
        void proc_cpl();
        void proc_scf();
        void proc_ccf();
        void proc_halt();
        void proc_rra();
        void proc_and();
        void proc_xor();
        void proc_or();
        void proc_cp();
        void proc_di();
        void proc_ei();
        void proc_ld();
        void proc_ldh();
        void proc_jp();
        void proc_jr();
        void proc_call();
        void proc_rst();
        void proc_ret();
        void proc_reti();
        void proc_pop();
        void proc_push();
        void proc_inc();
        void proc_dec();
        void proc_sub();
        void proc_sbc();
        void proc_adc();
        void proc_add();
    };