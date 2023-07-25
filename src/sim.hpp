#pragma once
#include <chrono>
#include <memory>
#include <thread>

#include "config.hpp"
#include "iodevice.hpp"
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
            inline void dump_state(Log &log);
            inline void setcc(uint32_t val);

            std::vector<std::unique_ptr<IODevice>> io_devices;
        public:
            bool halted;
            uint32_t pc;
            uint32_t regs[8];
            Memory mem;
            uint8_t cond;

            Simulator(unsigned int seed);
            ~Simulator();
            /*!
            * \brief Single-steps the program currently being executed
            * \return Whether or not the program is still running
            */
            bool step() noexcept;
            void register_io_device(IODevice &dev);
            void register_io_device(IODevice *dev);
    };
}
