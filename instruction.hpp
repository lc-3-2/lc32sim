#pragma once
#include <cstdint>

namespace lc32sim {
    // mostly equivalent to opcodes, but some opcodes can encode multiple instructions
    enum class InstructionType {
        ADD, AND, BR, JMP, JSR,
        JSRR, LDB, LDH, LDW, LEA,
        RTI, LSHF, RSHFL, RSHFA,
        STB, STH, STW, TRAP, XOR
    };
    union InstructionData {
        struct {
            uint8_t dr, sr1, sr2;
            bool imm;
            uint32_t imm5;
        } arithmetic;
        struct {
            uint8_t cond;
            uint32_t pcoffset9;
        } br;
        struct {
            uint16_t baseR;
        } jmp;
        struct {
            uint8_t baseR;
        } jsrr;
        struct {
            uint32_t pcoffset11;
        } jsr;
        struct {
            uint8_t dr, baseR;
            uint32_t offset6;
        } load;
        struct {
            uint8_t dr;
            uint32_t pcoffset9;
        } lea;
        struct {
            uint8_t dr, sr1, sr2;
            bool imm;
            uint32_t amount3;
        } shift;
        struct {
            uint8_t sr, baseR;
            uint32_t offset6;
        } store;
        struct {
            uint8_t trapvect8;
        } trap;
    };
    class Instruction {
        private:
            void parse_arithmetic_instruction(uint16_t instruction_bits);
            void parse_load_instruction(uint16_t instruction_bits);
            void parse_store_instruction(uint16_t instruction_bits);
        public:
            InstructionType type;
            InstructionData data;
            Instruction();
            Instruction(uint16_t instruction_bits);
            ~Instruction();
    };
}