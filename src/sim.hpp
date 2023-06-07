#pragma once
#include <chrono>
#include <thread>

#include "display.hpp"
#include "memory.hpp"

namespace lc32sim {
    class Simulator {
        // private:
        public:
            volatile bool running;
            uint32_t pc;
            uint32_t regs[8];
            Display display;
            Memory mem;
            uint8_t cond;
            std::thread sim_thread;

            void setcc(uint32_t val);
            void simulate();
        public:
            Simulator(unsigned int seed);
            ~Simulator();
            void launch_sim_thread();
            void stop_sim();
    };
}