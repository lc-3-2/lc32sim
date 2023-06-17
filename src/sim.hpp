#pragma once
#include <chrono>
#include <memory>
#include <thread>

#include "config.hpp"
#include "display.hpp"
#include "memory.hpp"
#include "log.hpp"

namespace lc32sim {
    class Simulator {
        private:
            /*!
             * \brief Dumps the state of the machine to `cout`
             *
             * This method is used by the `BREAK` TRAP to write out the state of
             * the machine. It prints with four spaces in front of each entry.
             *
             * @param[in] log The log to dump to, like `logger.info`
             */
            void dump_state(Log log);

        // TODO: figure out exactly what the public interface should be
        // private:
        public:
            volatile bool running;
            uint32_t pc;
            uint32_t regs[8];
            Memory mem;
            std::unique_ptr<uint16_t[]> video_buffer;
            uint8_t cond;
            std::thread sim_thread;
            std::exception_ptr sim_thread_exception;

            void setcc(uint32_t val);
            void simulate(Display *display);
        public:
            Simulator(unsigned int seed);
            ~Simulator();
            void launch_sim_thread();
            void launch_sim_thread_with_display(Display &display);
            void stop_sim();
            void join_sim();
    };
}
