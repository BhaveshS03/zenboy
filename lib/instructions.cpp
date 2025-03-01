#include <array>
#include <instructions.hpp>
#include <string>


Instructions::Instructions(){

    table[0x00] = InstructionData{IN::NOP, AM::IMP};
    table[0x01] = InstructionData{IN::LD, AM::R_D16, RT::BC};
    table[0x02] = InstructionData{IN::LD, AM::MR_R, RT::BC, RT::A};
    table[0x05] = InstructionData{IN::DEC, AM::R, RT::B};
    table[0x06] = InstructionData{IN::LD, AM::R_D8, RT::B};
    table[0x08] = InstructionData{IN::LD, AM::A16_R, RT::NONE, RT::SP};
    table[0x0A] = InstructionData{IN::LD, AM::R_MR, RT::A, RT::BC};
    table[0x0F] = InstructionData{IN::RRCA, AM::R, RT::A};
    table[0x0E] = InstructionData{IN::LD, AM::R_D8, RT::C};

    table[0x11] = InstructionData{IN::LD, AM::R_D16, RT::DE};
    table[0x12] = InstructionData{IN::LD, AM::MR_R, RT::DE, RT::A};
    table[0x15] = InstructionData{IN::DEC, AM::R, RT::D};
    table[0x16] = InstructionData{IN::LD, AM::R_D8, RT::D};
    table[0x18] = InstructionData{IN::JR, AM::D8};
    table[0x1A] = InstructionData{IN::LD, AM::R_MR, RT::A, RT::DE};
    table[0x1F] = InstructionData{IN::RRA, AM::R, RT::A};
    table[0x1E] = InstructionData{IN::LD, AM::R_D8, RT::E};

    table[0x20] = InstructionData{IN::JR, AM::D8, .cond = CT::NZ};  
    table[0x21] = InstructionData{IN::LD, AM::R_D16, RT::HL};
    table[0x22] = InstructionData{IN::LD, AM::HLI_R, RT::HL, RT::A};
    table[0x25] = InstructionData{IN::DEC, AM::R, RT::H};
    table[0x26] = InstructionData{IN::LD, AM::R_D8, RT::H};
    table[0x28] = InstructionData{IN::JR, AM::D8, .cond = CT::Z}; 
    table[0x2A] = InstructionData{IN::LD, AM::R_HLI, RT::A, RT::HL};
    table[0x2E] = InstructionData{IN::LD, AM::R_D8, RT::L};

    table[0x30] = InstructionData{IN::JR, AM::D8, .cond = CT::NC}; 
    table[0x31] = InstructionData{IN::LD, AM::R_D16, RT::SP};
    table[0x32] = InstructionData{IN::LD, AM::HLD_R, RT::HL, RT::A};
    table[0x35] = InstructionData{IN::DEC, AM::R, RT::HL};
    table[0x36] = InstructionData{IN::LD, AM::MR_D8, RT::HL};
    table[0x38] = InstructionData{IN::JR, AM::D8, .cond = CT::C}; 
    table[0x3A] = InstructionData{IN::LD, AM::R_HLD, RT::A, RT::HL};
    table[0x3E] = InstructionData{IN::LD, AM::R_D8, RT::A};

    table[0x40] = InstructionData{IN::LD, AM::R_R, RT::B, RT::B};
    table[0x41] = InstructionData{IN::LD, AM::R_R, RT::B, RT::C};
    table[0x42] = InstructionData{IN::LD, AM::R_R, RT::B, RT::D};
    table[0x43] = InstructionData{IN::LD, AM::R_R, RT::B, RT::E};
    table[0x44] = InstructionData{IN::LD, AM::R_R, RT::B, RT::H};
    table[0x45] = InstructionData{IN::LD, AM::R_R, RT::B, RT::L};
    table[0x46] = InstructionData{IN::LD, AM::R_MR, RT::B, RT::HL};
    table[0x47] = InstructionData{IN::LD, AM::R_R, RT::B, RT::A};
    table[0x48] = InstructionData{IN::LD, AM::R_R, RT::C, RT::B};
    table[0x49] = InstructionData{IN::LD, AM::R_R, RT::C, RT::C};
    table[0x4A] = InstructionData{IN::LD, AM::R_R, RT::C, RT::D};
    table[0x4B] = InstructionData{IN::LD, AM::R_R, RT::C, RT::E};
    table[0x4C] = InstructionData{IN::LD, AM::R_R, RT::C, RT::H};
    table[0x4D] = InstructionData{IN::LD, AM::R_R, RT::C, RT::L};
    table[0x4E] = InstructionData{IN::LD, AM::R_MR, RT::C, RT::HL};
    table[0x4F] = InstructionData{IN::LD, AM::R_R, RT::C, RT::A};

    table[0x50] = InstructionData{IN::LD, AM::R_R, RT::D, RT::B};
    table[0x51] = InstructionData{IN::LD, AM::R_R, RT::D, RT::C};
    table[0x52] = InstructionData{IN::LD, AM::R_R, RT::D, RT::D};
    table[0x53] = InstructionData{IN::LD, AM::R_R, RT::D, RT::E};
    table[0x54] = InstructionData{IN::LD, AM::R_R, RT::D, RT::H};
    table[0x55] = InstructionData{IN::LD, AM::R_R, RT::D, RT::L};
    table[0x56] = InstructionData{IN::LD, AM::R_MR, RT::D, RT::HL};
    table[0x57] = InstructionData{IN::LD, AM::R_R, RT::D, RT::A};
    table[0x58] = InstructionData{IN::LD, AM::R_R, RT::E, RT::B};
    table[0x59] = InstructionData{IN::LD, AM::R_R, RT::E, RT::C};
    table[0x5A] = InstructionData{IN::LD, AM::R_R, RT::E, RT::D};
    table[0x5B] = InstructionData{IN::LD, AM::R_R, RT::E, RT::E};
    table[0x5C] = InstructionData{IN::LD, AM::R_R, RT::E, RT::H};
    table[0x5D] = InstructionData{IN::LD, AM::R_R, RT::E, RT::L};
    table[0x5E] = InstructionData{IN::LD, AM::R_MR, RT::E, RT::HL};
    table[0x5F] = InstructionData{IN::LD, AM::R_R, RT::E, RT::A};

    table[0x60] = InstructionData{IN::LD, AM::R_R, RT::H, RT::B};
    table[0x61] = InstructionData{IN::LD, AM::R_R, RT::H, RT::C};
    table[0x62] = InstructionData{IN::LD, AM::R_R, RT::H, RT::D};
    table[0x63] = InstructionData{IN::LD, AM::R_R, RT::H, RT::E};
    table[0x64] = InstructionData{IN::LD, AM::R_R, RT::H, RT::H};
    table[0x65] = InstructionData{IN::LD, AM::R_R, RT::H, RT::L};
    table[0x66] = InstructionData{IN::LD, AM::R_MR, RT::H, RT::HL};
    table[0x67] = InstructionData{IN::LD, AM::R_R, RT::H, RT::A};
    table[0x68] = InstructionData{IN::LD, AM::R_R, RT::L, RT::B};
    table[0x69] = InstructionData{IN::LD, AM::R_R, RT::L, RT::C};
    table[0x6A] = InstructionData{IN::LD, AM::R_R, RT::L, RT::D};
    table[0x6B] = InstructionData{IN::LD, AM::R_R, RT::L, RT::E};
    table[0x6C] = InstructionData{IN::LD, AM::R_R, RT::L, RT::H};
    table[0x6D] = InstructionData{IN::LD, AM::R_R, RT::L, RT::L};
    table[0x6E] = InstructionData{IN::LD, AM::R_MR, RT::L, RT::HL};
    table[0x6F] = InstructionData{IN::LD, AM::R_R, RT::L, RT::A};

    table[0x70] = InstructionData{IN::LD, AM::MR_R, RT::HL, RT::B};
    table[0x71] = InstructionData{IN::LD, AM::MR_R, RT::HL, RT::C};
    table[0x72] = InstructionData{IN::LD, AM::MR_R, RT::HL, RT::D};
    table[0x73] = InstructionData{IN::LD, AM::MR_R, RT::HL, RT::E};
    table[0x74] = InstructionData{IN::LD, AM::MR_R, RT::HL, RT::H};
    table[0x75] = InstructionData{IN::LD, AM::MR_R, RT::HL, RT::L};
    table[0x76] = InstructionData{IN::HALT, AM::IMP};
    table[0x77] = InstructionData{IN::LD, AM::MR_R, RT::HL, RT::A};
    table[0x78] = InstructionData{IN::LD, AM::R_R, RT::A, RT::B};
    table[0x79] = InstructionData{IN::LD, AM::R_R, RT::A, RT::C};
    table[0x7A] = InstructionData{IN::LD, AM::R_R, RT::A, RT::D};
    table[0x7B] = InstructionData{IN::LD, AM::R_R, RT::A, RT::E};
    table[0x7C] = InstructionData{IN::LD, AM::R_R, RT::A, RT::H};
    table[0x7D] = InstructionData{IN::LD, AM::R_R, RT::A, RT::L};
    table[0x7E] = InstructionData{IN::LD, AM::R_MR, RT::A, RT::HL};
    table[0x7F] = InstructionData{IN::LD, AM::R_R, RT::A, RT::A};

    table[0x80] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::B};
    table[0x81] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::C};
    table[0x82] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::D};
    table[0x83] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::E};
    table[0x84] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::H};
    table[0x85] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::L};
    table[0x86] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::HL};
    table[0x87] = InstructionData{IN::ADD, AM::R_R, RT::A, RT::A};

    table[0xAF] = InstructionData{IN::XOR, AM::R, RT::A};



    table[0xC0] = InstructionData{IN::RET, AM::IMP, .cond = CT::NZ}; 
    table[0xC1] = InstructionData{IN::POP, AM::R, RT::BC};
    table[0xC2] = InstructionData{IN::JP, AM::D16, .cond = CT::NZ}; 
    table[0xC3] = InstructionData{IN::JP, AM::D16};
    table[0xC4] = InstructionData{IN::CALL, AM::D16, .cond = CT::NZ};  
    table[0xC5] = InstructionData{IN::PUSH, AM::R, RT::BC};
    table[0xC7] = InstructionData{IN::RST, AM::IMP, .param = 0x00}; 
    table[0xC8] = InstructionData{IN::RET, AM::IMP, .cond = CT::Z};  
    table[0xC9] = InstructionData{IN::RET, AM::IMP};
    table[0xCA] = InstructionData{IN::JP, AM::D16, .cond = CT::Z};  
    table[0xCC] = InstructionData{IN::CALL, AM::D16, .cond = CT::Z};  
    table[0xCD] = InstructionData{IN::CALL, AM::D16};
    table[0xCF] = InstructionData{IN::RST, AM::IMP, .param = 0x08}; 

    table[0xD0] = InstructionData{IN::RET, AM::IMP, .cond = CT::NC}; 
    table[0xD1] = InstructionData{IN::POP, AM::R, RT::DE};
    table[0xD2] = InstructionData{IN::JP, AM::D16, .cond = CT::NC}; 
    table[0xD4] = InstructionData{IN::CALL, AM::D16, .cond = CT::NC};
    table[0xD5] = InstructionData{IN::PUSH, AM::R, RT::DE};
    table[0xD7] = InstructionData{IN::RST, AM::IMP, .param = 0x10}; 
    table[0xD8] = InstructionData{IN::RET, AM::IMP, .cond = CT::C};  
    table[0xD9] = InstructionData{IN::RETI, AM::IMP};
    table[0xDA] = InstructionData{IN::JP, AM::D16, .cond = CT::C};  
    table[0xDC] = InstructionData{IN::CALL, AM::D16, .cond = CT::C};  
    table[0xDF] = InstructionData{IN::RST, AM::IMP, .param = 0x18}; 

    table[0xE0] = InstructionData{IN::LDH, AM::A8_R, RT::NONE, RT::A};
    table[0xE1] = InstructionData{IN::POP, AM::R, RT::HL};
    table[0xE2] = InstructionData{IN::LD, AM::MR_R, RT::C, RT::A};
    table[0xE5] = InstructionData{IN::PUSH, AM::R, RT::HL};
    table[0xE7] = InstructionData{IN::RST, AM::IMP, .param = 0x20}; 
    table[0xE9] = InstructionData{IN::JP, AM::MR, RT::HL};
    table[0xEA] = InstructionData{IN::LD, AM::A16_R, RT::NONE, RT::A};
    table[0xEF] = InstructionData{IN::RST, AM::IMP, .param = 0x28}; 

    table[0xF0] = InstructionData{IN::LDH, AM::R_A8, RT::A};
    table[0xF1] = InstructionData{IN::POP, AM::R, RT::AF};
    table[0xF2] = InstructionData{IN::LD, AM::R_MR, RT::A, RT::C};
    table[0xF3] = InstructionData{IN::DI, AM::IMP};
    table[0xF5] = InstructionData{IN::PUSH, AM::R, RT::AF};
    table[0xF7] = InstructionData{IN::RST, AM::IMP, .param = 0x30}; 
    table[0xFA] = InstructionData{IN::LD, AM::R_A16, RT::A};
    table[0xFF] = InstructionData{IN::RST, AM::IMP, .param = 0x38};
}

const InstructionData* Instructions::Instruction_by_opcode(std::uint8_t opcode) {
    return table[opcode].has_value() ? &table[opcode].value() : nullptr;
}

std::array<std::string,48> inst_lookup[] = {
    "<NONE>",
    "NOP",
    "LD",
    "INC",
    "DEC",
    "RLCA",
    "ADD",
    "RRCA",
    "STOP",
    "RLA",
    "JR",
    "RRA",
    "DAA",
    "CPL",
    "SCF",
    "CCF",
    "HALT",
    "ADC",
    "SUB",
    "SBC",
    "AND",
    "XOR",
    "OR",
    "CP",
    "POP",
    "JP",
    "PUSH",
    "RET",
    "CB",
    "CALL",
    "RETI",
    "LDH",
    "JPHL",
    "DI",
    "EI",
    "RST",
    "IN_ERR",
    "IN_RLC", 
    "IN_RRC",
    "IN_RL", 
    "IN_RR",
    "IN_SLA", 
    "IN_SRA",
    "IN_SWAP", 
    "IN_SRL",
    "IN_BIT", 
    "IN_RES", 
    "IN_SET"
};

std::string inst_name(IN t) {
    return inst_lookup[0][static_cast<std::size_t>(t)];
}