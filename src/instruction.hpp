#pragma once
#include <cstdint>
#include <fstream>

namespace lc32sim {
    // mostly equivalent to opcodes, but some opcodes can encode multiple instructions
    enum class InstructionType {
        ADD, AND, BR, JMP, JSR,
        JSRR, LDB, LDH, LDW, LEA,
        RTI, LSHF, RSHFL, RSHFA,
        STB, STH, STW, TRAP, XOR
    };
    enum class TrapVector : uint8_t {
        // Note that PUTSP is not defined. This is because we're on a
        // byte-addressible architecture, so packing bytes into memory words
        // just isn't a thing.
        GETC = 0x20,
        OUT = 0x21,
        PUTS = 0x22,
        IN = 0x23,
        HALT = 0x25,

        /*!
         * \brief TRAP for debug breakpoints
         *
         * These can be emitted if the user calls __builtin_debugtrap(). It is
         * supposed to give control to the debugger, but we don't have one.
         */
        BREAK = 0xFE,

        /*!
         * \brief TRAP for bad execution
         *
         * This is used for padding by the compiler. It denotes an instruction
         * that should never be executed.
         */
        CRASH = 0xFF,
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
            uint32_t amount5;
        } shift;
        struct {
            uint8_t sr, baseR;
            uint32_t offset6;
        } store;
        struct {
            TrapVector trapvect8;
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

    inline std::ostream &operator<<(std::ostream &stream, Instruction const &i) {
        std::ios_base::fmtflags flags(stream.flags());

        stream << std::dec;
        switch(i.type) {
            case InstructionType::ADD: stream << "ADD "; goto arithmetic;
            case InstructionType::AND: stream << "AND "; goto arithmetic;
            case InstructionType::BR:
                stream << "BR";
                if (i.data.br.cond & 0b100) stream << "n";
                if (i.data.br.cond & 0b010) stream << "z";
                if (i.data.br.cond & 0b001) stream << "p";
                stream << " " << +i.data.br.pcoffset9;
                break;
            case InstructionType::JMP: stream << "JMP R" << +i.data.jmp.baseR; break;
            case InstructionType::JSR: stream << "JSR " << +i.data.jsr.pcoffset11; break;
            case InstructionType::JSRR: stream << "JSRR R" << +i.data.jsrr.baseR; break;
            case InstructionType::LDB: stream << "LDB "; goto load;
            case InstructionType::LDH: stream << "LDH "; goto load;
            case InstructionType::LDW: stream << "LDW "; goto load;
            case InstructionType::LEA: stream << "LEA " << +i.data.lea.dr << ", #" << +i.data.lea.pcoffset9; break;
            case InstructionType::RTI: stream << "RTI"; break;
            case InstructionType::LSHF: stream << "LSHF "; goto shift;
            case InstructionType::RSHFL: stream << "RSHFL "; goto shift;
            case InstructionType::RSHFA: stream << "RSHFA "; goto shift;
            case InstructionType::STB: stream << "STB "; goto store;
            case InstructionType::STH: stream << "STH "; goto store;
            case InstructionType::STW: stream << "STW "; goto store;
            case InstructionType::TRAP: stream << "TRAP x" << std::hex << +static_cast<uint8_t>(i.data.trap.trapvect8); break;
            case InstructionType::XOR: stream << "XOR"; goto arithmetic;
            arithmetic:
                stream << "R" << +i.data.arithmetic.dr << ", R" << +i.data.arithmetic.sr1 << ", ";
                if (i.data.arithmetic.imm) stream << "#" << +i.data.arithmetic.imm5;
                else stream << "R" << +i.data.arithmetic.sr2;
                break;
            shift:
                stream << "R" << +i.data.shift.dr << ", R" << +i.data.shift.sr1 << ", ";
                if (i.data.shift.imm) stream << "#" << i.data.shift.amount5;
                else stream << "R" << +i.data.shift.sr2;
                break;
            load:
                stream << "R" << +i.data.load.dr << ", R" << +i.data.load.baseR << ", #" << +i.data.load.offset6;
                break;
            store:
                stream << "R" << +i.data.store.sr << ", R" << +i.data.store.baseR << ", #" << +i.data.store.offset6;
                break;
            default: stream << "Unrecognized instruction"; break;
        }

        stream.flags(flags);
        return stream;
    }
}
