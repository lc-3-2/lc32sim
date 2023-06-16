#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <termios.h>
#include <unistd.h>

#include "exceptions.hpp"
#include "instruction.hpp"
#include "sim.hpp"
#include "utils.hpp"

namespace lc32sim {
    Simulator::Simulator(unsigned int seed) : running(false), pc(0x30000000), display(), mem() {
        std::srand(seed);
        this->video_buffer = std::make_unique<uint16_t[]>(Config.display.width * Config.display.height);
        this->running = false;
        for (size_t i = 0; i < sizeof(this->regs)/sizeof(this->regs[0]); i++) {
            this->regs[i] = std::rand();
        }
        mem.set_seed(std::rand());
        display.initialize();

        // Need to turn of ECHO and ICANON on the terminal
        // GETC and IN assumes that characters are not echoed and that input is
        // not line buffered.
        {
            // Get the terminal information
            struct termios ti;
            if (tcgetattr(STDIN_FILENO, &ti))
                throw TerminalConfigurationException("Could not retrieve terminal configuration");
            // Disable
            ti.c_lflag &= ~(ECHO | ICANON);
            if (tcsetattr(STDIN_FILENO, TCSANOW, &ti))
                throw TerminalConfigurationException("Could not disable ECHO");
        }
    }
    Simulator::~Simulator() {
        if (this->running) {
            this->stop_sim();
        } else {
            if (this->sim_thread.joinable()) {
                this->sim_thread.join();
            }
        }

        // We turned ECHO and ICANON off in the constructor, so turn it back on
        // Don't fail if this doesn't work - we're dead anyway
        {
            // Get the terminal information
            struct termios ti;
            tcgetattr(STDIN_FILENO, &ti);
            // Enable
            ti.c_lflag |= ECHO | ICANON;
            tcsetattr(STDIN_FILENO, TCSANOW, &ti);
        }
    }

    void Simulator::setcc(uint32_t val) {
        int32_t sval = static_cast<int32_t>(val);
        cond = (sval < 0) ? 0b100 : (sval == 0) ? 0b010 : 0b001;
    }

    void Simulator::simulate() {
        Instruction i;
        while (this->running) {
            // FETCH/DECODE
            i = Instruction(mem.read<uint16_t>(pc));
            std::cout << "Executing instruction " << i << std::endl;
            pc += 2;
            std::cout << "(1) PC is now " << pc << std::endl;

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
                        pc += i.data.br.pcoffset9 * 2;
                        std::cout << "(2) PC is now " << pc << std::endl;
                    }
                    break;
                case InstructionType::JMP:
                    pc = regs[i.data.jmp.baseR];
                    std::cout << "(3) PC is now " << pc << std::endl;
                    break;
                case InstructionType::JSR:
                    regs[7] = pc;
                    pc += i.data.jsr.pcoffset11 * 2;
                    std::cout << "(4) PC is now " << pc << std::endl;
                    break;
                case InstructionType::JSRR:
                    regs[7] = pc;
                    pc = regs[i.data.jsrr.baseR];
                    std::cout << "(5) PC is now " << pc << std::endl;
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
                    switch (i.data.trap.trapvect8) {
                        case TrapVector::GETC: {
                            char received;
                            std::cin >> received;
                            this->regs[0] = static_cast<uint32_t>(received & 0xff);
                            break;
                        }
                        case TrapVector::OUT:
                            std::cout << static_cast<char>(this->regs[0] & 0xff);
                            break;
                        case TrapVector::PUTS: {
                            char c;
                            for (uint32_t i = regs[0]; (c = mem.read<char>(i)) != '\0'; i++)
                                std::cout << c;
                            break;
                        }
                        case TrapVector::IN: {
                            char received;
                            std::cout << "> ";
                            std::cin >> received;
                            std::cout << received;
                            this->regs[0] = static_cast<uint32_t>(received & 0xff);
                            break;
                        }
                        case TrapVector::HALT:
                            this->running = false;
                            break;
                        case TrapVector::BREAK:
                            // If the user tries to give control to the
                            // debugger, print a message and dump the state. It
                            // is *not* an error to execute this instruction.
                            std::cout << "Encountered BREAK:\n";
                            this->dump_state();
                            std::cout << "    Continuing execution...\n";
                            break;
                        case TrapVector::CRASH:
                            // This should never happen. If it does, die
                            throw SimulatorException("simulate(): encountered CRASH");
                        default:
                            throw SimulatorException("simulate(): unknown TRAP vector " + std::to_string(static_cast<uint8_t>(i.data.trap.trapvect8)));
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

            this->display.draw(mem.get_video_buffer());
        }
    }
    void Simulator::launch_sim_thread() {
        if (this->running) {
            throw SimulatorException("launch_sim_thread(): simulator already running");
        }
        this->sim_thread = std::thread(&Simulator::simulate, this);
        this->running = true;
    }
    void Simulator::run_sim_with_display() {
        this->launch_sim_thread();
        this->display.loop();
    }
    void Simulator::stop_sim() {
        if (!this->running) {
            throw SimulatorException("stop_sim(): simulator not running");
        }
        this->running = false;
        if (this->sim_thread.joinable()) {
            sim_thread.join();
        }
    }

    void Simulator::dump_state() {
        std::cout << "    PC: "
            << std::hex << std::setfill('0') << std::setw(8)
            << this->pc << '\n';
        std::cout << "    CC: "
            << (this->cond & 0b100 ? "n" : "")
            << (this->cond & 0b010 ? "z" : "")
            << (this->cond & 0b001 ? "p" : "")
            << '\n';

        for (size_t i = 0; i < 8; i++)
            std::cout << "    R" << i << ": "
                << std::hex << std::setfill('0') << std::setw(8)
                << this->regs[i] << '\n';
    }
}
