#pragma once
#include <chrono>
#include <cstdint>
#include <ctime>

#include "iodevice.hpp"

namespace lc32sim {
    class Clock : public IODevice {
        using namespace std::chrono;
        static 
        public:
            std::string get_name() override { return "Clock"; };
            read_handlers get_read_handlers() override {
                return read_handlers {
                    { REG_CURRTIME_ADDR, [](uint32_t addr) -> uint32_t {
                        return time(NULL);
                    } }
                }, 
                { HIGH_RES_CLOCK_ADDR, [](uint32_t addr) -> uint32_t {
                    return 0;
                } }
            }
            write_handlers get_write_handlers() override {};
    };
}