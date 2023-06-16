#pragma once
#include <chrono>
#include <memory>
#include <thread>

#include "config.hpp"
#include "display.hpp"
#include "memory.hpp"

namespace lc32sim {
    class Simulator {
        private:
            /*!
             * \brief Dumps the state of the machine to `cout`
             *
             * This method is used by the `BREAK` TRAP to write out the state of
             * the machine. It prints with four spaces in front of each entry.
             */
            void dump_state();

        // TODO: figure out exactly what the public interface should be
        // private:
        public:
            volatile bool running;
            uint32_t pc;
            uint32_t regs[8];
            Display display;
            Memory mem;
            std::unique_ptr<uint16_t[]> video_buffer;
            uint8_t cond;
            std::thread sim_thread;

            void setcc(uint32_t val);
            void simulate();
        public:
            Simulator(unsigned int seed);
            ~Simulator();
            void launch_sim_thread();
            void run_sim_with_display();
            void stop_sim();
    };
}
