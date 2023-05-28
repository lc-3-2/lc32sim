#include <chrono>
#include <cmath>
#include <cstdlib>

#include "exceptions.hpp"
#include "instruction.hpp"
#include "sim.hpp"
#include "utils.hpp"

namespace lc32sim {
    Simulator::Simulator(unsigned int seed) : running(false), pc(0x3000), mem() {
        std::srand(seed);
        this->running = false;
        pc = 0x3000;
        for (size_t i = 0; i < sizeof(this->regs)/sizeof(this->regs[0]); i++) {
            this->regs[i] = std::rand();
        }
        mem.set_seed(std::rand());
    }
    Simulator::~Simulator() {
        if (this->running) {
            this->stop_sim();
        } else {
            this->sim_thread.join();
        }
    }

    void Simulator::setcc(uint32_t val) {
        if (val == 0) {
            cond = 0b001;
        } else if (val & 0x80000000) {
            cond = 0b100;
        } else {
            cond = 0b010;
        }
    }

    void Simulator::simulate() {
        Instruction i;
        while (this->running) {
            // FETCH/DECODE
            i = Instruction(mem.read<uint16_t>(pc));
            pc += 2;

            // EXECUTE
            uint32_t val2; // represents the second value in arithmetic instructions
            switch (i.type) {
                case InstructionType::ADD:
                    val2 = i.data.arithmetic.imm ? i.data.arithmetic.imm5 : regs[i.data.arithmetic.sr2];
                    regs[i.data.arithmetic.dr] = regs[i.data.arithmetic.sr1] + val2;
                    setcc(regs[i.data.arithmetic.dr]);
                    break;
                case InstructionType::AND:
                    val2 = i.data.arithmetic.imm ? i.data.arithmetic.imm5 : regs[i.data.arithmetic.sr2];
                    regs[i.data.arithmetic.dr] = regs[i.data.arithmetic.sr1] & val2;
                    setcc(regs[i.data.arithmetic.dr]);
                    break;
                case InstructionType::BR:
                    if (cond & i.data.br.cond) {
                        pc += i.data.br.pcoffset9;
                    }
                    break;
                case InstructionType::JMP:
                    pc = regs[i.data.jmp.baseR];
                    break;
                case InstructionType::JSR:
                    regs[7] = pc;
                    pc += i.data.jsr.pcoffset11;
                    break;
                case InstructionType::JSRR:
                    regs[7] = pc;
                    pc = regs[i.data.jsrr.baseR];
                    break;
                case InstructionType::LDB:
                    regs[i.data.load.dr] = sext<8, 32>(mem.read<uint8_t>(regs[i.data.load.baseR] + i.data.load.offset6));
                    setcc(regs[i.data.load.dr]);
                    break;
                case InstructionType::LDH:
                    regs[i.data.load.dr] = sext<16, 32>(mem.read<uint16_t>(regs[i.data.load.baseR] + (i.data.load.offset6 * 2)));
                    setcc(regs[i.data.load.dr]);
                    break;
                case InstructionType::LDW:
                    regs[i.data.load.dr] = mem.read<uint32_t>(regs[i.data.load.baseR] + (i.data.load.offset6 * 4));
                    setcc(regs[i.data.load.dr]);
                    break;
                case InstructionType::LEA:
                    regs[i.data.lea.dr] = pc + i.data.lea.pcoffset9;
                    setcc(regs[i.data.load.dr]);
                    break;
                case InstructionType::RTI:
                    throw SimulatorException("simulate(): RTI not implemented");
                    break;
                case InstructionType::LSHF:
                    regs[i.data.shift.dr] = regs[i.data.shift.sr1] << regs[i.data.shift.sr2];
                    setcc(regs[i.data.shift.dr]);
                        break;
                case InstructionType::RSHFL:
                    regs[i.data.shift.dr] = regs[i.data.shift.sr1] >> regs[i.data.shift.sr2];
                    setcc(regs[i.data.shift.dr]);
                    break;
                case InstructionType::RSHFA:
                    if (i.data.shift.imm) {
                        regs[i.data.shift.dr] = static_cast<int32_t>(regs[i.data.shift.sr1]) >> i.data.shift.amount3;
                    } else {
                        regs[i.data.shift.dr] = static_cast<int32_t>(regs[i.data.shift.sr1]) >> regs[i.data.shift.sr2];
                    }
                    setcc(regs[i.data.shift.dr]);
                    break;
                case InstructionType::STB:
                    mem.write<uint8_t>(regs[i.data.store.baseR] + i.data.store.offset6, static_cast<uint8_t>(regs[i.data.store.sr]));
                    break;
                case InstructionType::STH:
                    mem.write<uint16_t>(regs[i.data.store.baseR] + (i.data.store.offset6 * 2), static_cast<uint16_t>(regs[i.data.store.sr]));
                    break;
                case InstructionType::STW:
                    mem.write<uint32_t>(regs[i.data.store.baseR] + (i.data.store.offset6 * 4), static_cast<uint32_t>(regs[i.data.store.sr]));
                    break;
                case InstructionType::TRAP:
                    if (i.data.trap.trapvect8 == 0x25) {
                        this->running = false;
                    } else {
                        throw SimulatorException("simulate(): unknown TRAP vector " + std::to_string(i.data.trap.trapvect8));
                    }
                    break;
                case InstructionType::XOR:
                    val2 = i.data.arithmetic.imm ? i.data.arithmetic.imm5 : regs[i.data.arithmetic.sr2];
                    regs[i.data.arithmetic.dr] = regs[i.data.arithmetic.sr1] ^ val2;
                    setcc(regs[i.data.arithmetic.dr]);
                    break;
                default:
                    throw SimulatorException("simulate(): unknown instruction type " + std::to_string(static_cast<std::underlying_type<InstructionType>::type>(i.type)));
                    break;                
            }
        }
    }
    void Simulator::launch_sim_thread() {
        if (this->running) {
            throw SimulatorException("launch_sim_thread(): simulator already running");
        }
        this->sim_thread = std::thread(&Simulator::simulate, this);
        this->running = true;
    }
    void Simulator::stop_sim() {
        if (!this->running) {
            throw SimulatorException("stop_sim(): simulator not running");
        }
        this->running = false;
        sim_thread.join();
    }
}