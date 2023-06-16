#pragma once
#include <bitset>
#include <cstdint>
#include <ios>
#include <sstream>
#include <stacktrace>
#include <stdexcept>
#include <string>
#include <cstring>

namespace lc32sim {
    inline std::string int_to_hex(uint64_t val) {
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
    class ELFParsingException : public std::runtime_error {
        public:
            ELFParsingException(std::string msg) : std::runtime_error(msg) {}
    };
    class DisplayException : public SimulatorException {
        public:
            DisplayException(std::string msg) : SimulatorException(msg) {}
            DisplayException(std::string msg, std::string msg2) : SimulatorException(msg + ": " + msg2) {}
    };
    // class MalformedInstruction : public SimulatorException {
    //     public:
    //         MalformedInstruction(uint16_t instruction_bits, std::string msg) : SimulatorException(
    //             "Malformed instruction (" + std::bitset<16>(instruction_bits).to_string() + "): " + msg) {}
    // };

    /*!
     * \brief Thrown if we can't configure the terminal
     *
     * The simulator needs to configure the terminal while running the
     * simulator. Specifically, it needs to disable echoing. If it can't this
     * exception is thrown.
     */
    class TerminalConfigurationException : public SimulatorException {
        public:
            TerminalConfigurationException() : SimulatorException(
                std::string("Could not configure terminal") + std::strerror(errno)
            ) {}
            TerminalConfigurationException(std::string msg) : SimulatorException(
                msg + std::strerror(errno)
            ) {}
    };
}
