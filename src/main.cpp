#include <argparse/argparse.hpp>
#include <bitset>
#include <chrono>
#include <csignal>
#include <exception>
#include <functional>
#include <iostream>

#include "display.hpp"
#include "dma_controller.hpp"
#include "config.hpp"
#include "elf_file.hpp"
#include "instruction.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "sim.hpp"

using lc32sim::logger;
using lc32sim::Config;

const std::string VERSION = "0.0.1";

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("lc32sim", VERSION);
    program.add_argument("file").help("LC-3.2 ELF executable to simulate");
    program.add_argument("-c", "--config-file").help("path to a JSON-formatted config file").default_value(std::string("./lc32sim.json"));
    program.add_argument("-s", "--software-rendering").help("disable hardware-accelerated rendering, even if enabled in config").default_value(false).implicit_value(true);
    program.add_argument("-l", "--log-level").help("set minimum log level to be displayed; lower levels are suppressed").default_value(std::string("use-config"));
    program.add_argument("-H", "--headless").help("run simulator without a display").default_value(false).implicit_value(true);

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        logger.error << err.what();
        logger.error << program;
        exit(1);
    }

    if (program["--version"] == true) {
        std::cout << "lc32sim " << VERSION << std::endl;
        exit(0);
    }

    if (program["--help"] == true) {
        std::cout << program;
        exit(0);
    }
    bool headless = program.get<bool>("--headless");

    lc32sim::config_instance.load_config(program);

    lc32sim::ELFFile elf(program.get<std::string>("file"));
    std::unique_ptr<lc32sim::Simulator> simptr = std::make_unique<lc32sim::Simulator>(42);
    lc32sim::Simulator &sim = *simptr;
    sim.mem.load_elf(elf);
    sim.pc = elf.get_header().entry;

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    uint64_t instructions_executed = 0;

    uint64_t vsyncs = 0;

    sim.register_io_device(new lc32sim::DMAController(sim.mem));

    if (headless) {
        while (sim.step()) {
            instructions_executed++;
        }
        instructions_executed++;
    } else {
        unsigned int scanline_max = Config.display.height + Config.display.vblank_length;
        if (scanline_max > std::numeric_limits<uint16_t>().max()) {
            logger.error << "Display height + vblank length exceeds range of uint16_t";
            exit(1);
        }
        uint16_t scanline = 0;
        lc32sim::Display display = lc32sim::Display(scanline);

        sim.register_io_device(display);
        
        while (true) {
            for (scanline = 0; scanline < scanline_max; scanline++) {
                for (unsigned int instruction = 0; instruction < Config.display.instructions_per_scanline; instruction++) {
                    instructions_executed++;
                    if (!sim.step()) {
                        goto done;
                    }
                }
                
                if (!display.update(sim)) {
                    goto done;
                }
            }
            vsyncs++;
        }
    }

    done:
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    logger.info << "Executed " << instructions_executed << " instructions in " << elapsed.count() << " seconds (" << instructions_executed / elapsed.count() << " Hz)";
    logger.info << "Vsyncs: " << vsyncs << ", Vsyncs/second " << vsyncs / elapsed.count();
    return 0;
}