#include <bitset>
#include <iostream>

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

    ELFFile elf(argv[1]);
    unique_ptr<Simulator> simptr = make_unique<Simulator>(42);
    Simulator &sim = *simptr;
    sim.mem.load_elf(elf);
    // sim.mem.write<uint16_t>(0x30000000, 0xe006);
    // cout << ".text section dump: " << endl;
    // Instruction inst;
    // for (uint32_t i = 0x30000000; i < 0x30000000 + 0xb4; i += 2) {
    //     uint16_t inst_val = sim.mem.read<uint16_t>(i);
    //     inst = Instruction(inst_val);
    //     cout << "0x" << hex << i << ": 0x" << hex << inst_val << " (" << inst << ")" << endl;
    // }
    sim.pc = elf.get_header().entry;    
    // cout << "Final register values:" << endl;
    // for (unsigned i = 0; i < sizeof(sim.regs)/sizeof(sim.regs[0]); i++) {
    //     cout << "R" << dec << i << ": 0x" << hex << sim.regs[i] << " (" << dec << static_cast<int32_t>(sim.regs[i]) << ")" << endl;
    // }
    // cout << "PC: 0x" << hex << sim.pc << endl;
    // cout << "cc: " << bitset<3>(sim.cond) << endl;

    // uint16_t *video_buffer = sim.mem.get_video_buffer();
    // // Auto generate a rainbow pattern
    // for (unsigned int row = 0; row < 480; row++) {
    //     for (unsigned int col = 0; col < 640; col++) {
    //         uint8_t r = (row * 31) / 480;
    //         uint8_t g = (col * 31) / 640;
    //         uint8_t b = ((row + col) * 31) / (480 + 640);
    //         video_buffer[row * 640 + col] = (r << 10) | (g << 5) | b;
    //     }
    // }

    sim.run_sim_with_display();

    // Dumping video buffer
    // cout << "Video buffer dump:" << endl;
    // for (unsigned int row = 0; row < 480; row++) {
    //     for (unsigned int col = 0; col < 640; col++) {
    //         cout << hex << sim.mem.read<uint16_t>(IO_SPACE_ADDR + (row * 640 + col) * 2) << " ";
    //     }
    //     cout << endl;
    // }

    return 0;
}