#pragma once

#include <string>
#include <array>
#include <optional>
#include "common.hpp"

enum class AM{
    IMP,
    R_D16,
    R_R,
    MR_R,
    R,
    R_D8,
    R_MR,
    R_HLI,
    R_HLD,
    HLI_R,
    HLD_R,
    R_A8,
    A8_R,
    HL_SPR,
    D16,
    D8,
    D16_R,
    MR_D8,
    MR,
    A16_R,
    R_A16
};

enum class RT{
    NONE,
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    AF,
    BC,
    DE,
    HL,
    SP,
    PC
} ;

enum class IN {
    NONE,
    NOP,
    LD,
    INC,
    DEC,
    RLCA,
    ADD,
    RRCA,
    STOP,
    RLA,
    JR,
    RRA,
    DAA,
    CPL,
    SCF,
    CCF,
    HALT,
    ADC,
    SUB,
    SBC,
    AND,
    XOR,
    OR,
    CP,
    POP,
    JP,
    PUSH,
    RET,
    CB,
    CALL,
    RETI,
    LDH,
    JPHL,
    DI,
    EI,
    RST,
    ERR,
    //CB Instructions...
    RLC, 
    RRC,
    RL, 
    RR,
    SLA, 
    SRA,
    SWAP, 
    SRL,
    BIT, 
    RES, 
    SET
};

enum class CT{
    NONE, NZ, Z, NC, C
};

typedef struct {
    IN type;
    AM mode;
    RT reg_1;
    RT reg_2;
    CT cond;
    u8 param;
} InstructionData;


class Instructions{
    public:
        Instructions();
        const InstructionData* Instruction_by_opcode(std::uint8_t opcode);
    private:
        std::array<std::optional<InstructionData>, 256> table;
};

std::string inst_name(IN t);

// table[0x00] = InstructionData{IN::NOP, AM::IMP, RT::NONE};
// table[0x05] = InstructionData{IN::DEC, AM::R, RT::B};
// table[0x0E] = InstructionData{IN::LD, AM::R_D8, RT::C};
// table[0xAF] = InstructionData{IN::XOR, AM::R, RT::A};
// table[0xC3] = InstructionData{IN::JP, AM::D16, RT::NONE};
// table[0xF3] = InstructionData{IN::DI, AM::IMP, RT::NONE};
// table[0xF8] = InstructionData{IN::LD, AM::HL_SPR, RT::SP};
// table[0xFE] = InstructionData{IN::CP, AM::D8, RT::NONE};
// table[0xCB] = InstructionData{IN::CB, AM::IMP, RT::NONE};   
// table[0xE0] = InstructionData{IN::LDH, AM::A8_R, RT::A};
// table[0x21] = InstructionData{IN::LD, AM::R_A16, RT::HL};
// table[0x06] = InstructionData{IN::LD, AM::R_D8, RT::B};
// table[0x32] = InstructionData{IN::LD, AM::HLD_R, RT::HL, RT::A};
// table[0x31] = InstructionData{IN::LD, AM::R_D16, RT::SP}; 
// table[0xEA] = InstructionData{IN::LD, AM::A16_R, RT::NONE, RT::A};
// table[0X3E] = InstructionData{IN::LD, AM::R_D8, RT::A};