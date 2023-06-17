#include <argparse/argparse.hpp>
#include <bitset>
#include <csignal>
#include <exception>
#include <functional>
#include <iostream>

#include "config.hpp"
#include "elf_file.hpp"
#include "instruction.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "sim.hpp"

using lc32sim::logger;

const std::string VERSION = "0.0.1";

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("lc32sim", VERSION);
    program.add_argument("file").help("LC-3.2 ELF executable to simulate");
    program.add_argument("-c", "--config-file").help("path to a JSON-formatted config file").default_value(std::string("./lc32sim.json"));
    program.add_argument("-s", "--software-rendering").help("disable hardware-accelerated rendering if enabled in config").default_value(false).implicit_value(true);
    program.add_argument("-l", "--log-level").help("set log level").default_value(std::string("use-config"));
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

    if (headless) {
        sim.launch_sim_thread();
        sim.join_sim();
    } else {
        lc32sim::Display display;
        display.initialize();
        sim.launch_sim_thread_with_display(display);

        while (sim.running) {
            // iterate() returns false when the window is closed
            if (!display.iterate()) {
                sim.stop_sim();
                break;
            }
        }
    }

    return 0;
}