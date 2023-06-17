#include <bitset>
#include <csignal>
#include <exception>
#include <functional>
#include <iostream>

#include "config.hpp"
#include "elf_file.hpp"
#include "instruction.hpp"
#include "memory.hpp"
#include "sim.hpp"

using namespace lc32sim;
using namespace std;

[[maybe_unused]] uint32_t program[] = {
    /* 0x3000: AND R0, R0, 0  */  0x5020, // R0 = 0
    /* 0x3002: ADD R1, R0, 1  */  0x1221, // R1 = 1
    /* 0x3004: ADD R0, R0, 13 */  0x102D, // R0 = 13
    /* 0x3006: XOR R0, R0, -1 */  0x903F, // R0 = -14
    /* 0x3008: LEA R2, 6      */  0xE406, // R2 = 0x3010
    /* 0x300A: LDW R2, R2, 0  */  0xA480, // R2 = 0x12345678
    /* 0x300C: TRAP 0x25      */  0xF025,
    /* 0x300E: (padding) */       0x0000,
    /* 0x3010: .fill 0x5678   */  0x5678,
    /* 0x3012: .fill 0x1234   */  0x1234,

};
[[maybe_unused]] void initialize_memory(Memory &mem) {
    for (unsigned i = 0; i < sizeof(program)/sizeof(program[0]); i++) {
        mem.write<uint16_t>(0x3000 + (i * 2), program[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }

    // const_cast is a necessary evil for now. TODO: figure out a better approach.
    const_cast<class Config&>(Config).load_config_file("lc32sim.json");

    ELFFile elf(argv[1]);
    unique_ptr<Simulator> simptr = make_unique<Simulator>(42);
    Simulator &sim = *simptr;
    sim.mem.load_elf(elf);

    // sim.mem.write<uint16_t>(0x30000000, 0xe006); // patch to fix initial version of hello_world executable.

    // cout << ".text section dump: " << endl;
    // Instruction inst;
    // for (uint32_t i = 0x30000000; i < 0x30000000 + 0xb4; i += 2) {
    //     uint16_t inst_val = sim.mem.read<uint16_t>(i);
    //     inst = Instruction(inst_val);
    //     cout << "0x" << hex << i << ": 0x" << hex << inst_val << " (" << inst << ")" << endl;
    // }

    sim.pc = elf.get_header().entry;
    sim.launch_sim_thread();
    sim.join_sim();
    return 0;
}