#include <bitset>
#include <iostream>

#include "memory.hpp"
#include "sim.hpp"

using namespace lc32sim;
using namespace std;

uint32_t program[] = {
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
void initialize_memory(Memory &mem) {
    for (unsigned i = 0; i < sizeof(program)/sizeof(program[0]); i++) {
        mem.write<uint16_t>(0x3000 + (i * 2), program[i]);
    }
}

int main(int argc, char *argv[]) {
    unique_ptr<Simulator> simptr = make_unique<Simulator>(42);
    Simulator &sim = *simptr;
    initialize_memory(sim.mem);
    sim.launch_sim_thread();
    while (sim.running) {
        this_thread::sleep_for(100ms);
    }
    cout << "Final register values:" << endl;
    for (unsigned i = 0; i < sizeof(sim.regs)/sizeof(sim.regs[0]); i++) {
        cout << "R" << i << ": 0x" << hex << sim.regs[i] << " (" << dec << static_cast<int32_t>(sim.regs[i]) << ")" << endl;
    }
    cout << "PC: 0x" << hex << sim.pc << endl;
    cout << "cc: " << bitset<3>(sim.cond) << endl;
}