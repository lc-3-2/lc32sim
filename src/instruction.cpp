#include "instruction.hpp"
#include "utils.hpp"

namespace lc32sim {
    Instruction::Instruction() {}
    Instruction::~Instruction() {}

    Instruction::Instruction(uint16_t instruction_bits) {
        uint16_t opcode = instruction_bits >> 12;
        switch(opcode) {
            case 0b0001:
                this->type = InstructionType::ADD;
                this->parse_arithmetic_instruction(instruction_bits);
                break;
            case 0b0101:
                this->type = InstructionType::AND;
                this->parse_arithmetic_instruction(instruction_bits);
                break;
            case 0b0000:
                this->type = InstructionType::BR;
                this->data.br.cond = (instruction_bits & 0x0E00) >> 9;
                this->data.br.pcoffset9 = sext<9, 32>(instruction_bits & 0x01FF);
                break;
            case 0b1100:
                this->type = InstructionType::JMP;
                this->data.jmp.baseR = (instruction_bits & 0x01C0) >> 6;
                break;
            case 0b0100: {
                bool bit11 = instruction_bits & 0x0800;
                if (bit11) {
                    this->type = InstructionType::JSR;
                    this->data.jsr.pcoffset11 = sext<11, 32>(instruction_bits & 0x07FF);
                } else {
                    this->type = InstructionType::JSRR;
                    this->data.jsrr.baseR = (instruction_bits & 0x01C0) >> 6;
                }
                break;
            }
            case 0b0010:
                this->type = InstructionType::LDB;
                this->parse_load_instruction(instruction_bits);
                break;
            case 0b0110:
                this->type = InstructionType::LDH;
                this->parse_load_instruction(instruction_bits);
                break;
            case 0b1010:
                this->type = InstructionType::LDW;
                this->parse_load_instruction(instruction_bits);
                break;
            case 0b1110:
                this->type = InstructionType::LEA;
                this->data.lea.dr = (instruction_bits & 0x0E00) >> 9;
                this->data.lea.pcoffset9 = sext<9, 32>(instruction_bits & 0x01FF);
                break;
            case 0b1000:
                this->type = InstructionType::RTI;
                break;
            case 0b1101: {
                // Pull out the relevant bits
                // These are non-zero if set, zero if not
                bool d_bit = instruction_bits & 0x0008;
                bool a_bit = instruction_bits & 0x0010;
                bool i_bit = instruction_bits & 0x0020;
                // Decode the instruction type depending on bits 4 and 3
                // The case of A=1 and D=0 is handled as LSHF
                if (d_bit) {
                    // D=1 - right shift
                    // Here, arithmetic and logical are different
                    if (a_bit)
                        this->type = InstructionType::RSHFA;
                    else
                        this->type = InstructionType::RSHFL;
                } else {
                    // D=0 - left shift
                    // Doesn't matter if arithmetic or logical - it's the same
                    // operation
                    this->type = InstructionType::LSHF;
                }
                // Set the DR and SR1; they are common for all shifts
                this->data.shift.dr = (instruction_bits & 0x0E00) >> 9;
                this->data.shift.sr1 = (instruction_bits & 0x01C0) >> 6;
                // Either set SR2 or amount3 depending on bit 5
                // The incrementing of amount3 should be handled in `sim.cpp`
                if (i_bit) {
                    this->data.shift.imm = true;
                    this->data.shift.amount3 = instruction_bits & 0x0007;
                } else {
                    this->data.shift.imm = false;
                    this->data.shift.sr2 = instruction_bits & 0x0007;
                }
                // Done
                break;
            }
            case 0b0011:
                this->type = InstructionType::STB;
                this->parse_store_instruction(instruction_bits);
                break;
            case 0b0111:
                this->type = InstructionType::STH;
                this->parse_store_instruction(instruction_bits);
                break;
            case 0b1011:
                this->type = InstructionType::STW;
                this->parse_store_instruction(instruction_bits);
                break;
            case 0b1111:
                this->type = InstructionType::TRAP;
                this->data.trap.trapvect8 = static_cast<TrapVector>(instruction_bits & 0x00FF);
                break;
            case 0b1001:
                this->type = InstructionType::XOR;
                this->parse_arithmetic_instruction(instruction_bits);
                break;
        }
    }

    // Helper methods
    forceinline void Instruction::parse_arithmetic_instruction(uint16_t instruction_bits) {
        this->data.arithmetic.dr =  (instruction_bits & 0x0E00) >> 9;
        this->data.arithmetic.sr1 = (instruction_bits & 0x01C0) >> 6;
        this->data.arithmetic.imm = instruction_bits & 0x0020;
        if (this->data.arithmetic.imm) {
            this->data.arithmetic.imm5 = sext<5, 32>(instruction_bits & 0x001F);
        } else {
            this->data.arithmetic.sr2 = instruction_bits & 0x0007;
        }
    }
    forceinline void Instruction::parse_load_instruction(uint16_t instruction_bits) {
        this->data.load.dr = (instruction_bits & 0x0E00) >> 9;
        this->data.load.baseR = (instruction_bits & 0x01C0) >> 6;
        this->data.load.offset6 = sext<6, 32>(instruction_bits & 0x003F);
    }
    forceinline void Instruction::parse_store_instruction(uint16_t instruction_bits) {
        this->data.store.sr = (instruction_bits & 0x0E00) >> 9;
        this->data.store.baseR = (instruction_bits & 0x01C0) >> 6;
        this->data.store.offset6 = sext<6, 32>(instruction_bits & 0x003F);
    }
}
