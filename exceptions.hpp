#pragma once
#include <bitset>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>

namespace lc32sim {
    template<typename T> inline std::string int_to_hex(T val) {
        std::stringstream stream;
        stream << std::hex << val;
        return stream.str();
    }

    class SimulatorException : public std::runtime_error {
        public:
            SimulatorException(std::string msg) : std::runtime_error(msg) {}
    };
    class UnalignedMemoryAccessException : public SimulatorException {
        public:
            UnalignedMemoryAccessException(uint32_t addr, int alignment) : SimulatorException(
                "Address 0x" + int_to_hex(addr) + " is not " + std::to_string(alignment) + "-byte aligned"
            ) {}
    };
    // class MalformedInstruction : public SimulatorException {
    //     public:
    //         MalformedInstruction(uint16_t instruction_bits, std::string msg) : SimulatorException(
    //             "Malformed instruction (" + std::bitset<16>(instruction_bits).to_string() + "): " + msg) {}
    // };
}