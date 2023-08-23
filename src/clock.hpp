#pragma once
#include <chrono>
#include <cstdint>
#include <ctime>

#include "iodevice.hpp"

namespace lc32sim {
    class Clock : public IODevice {
        public:
            std::string get_name() override { return "Clock"; };
            read_handlers get_read_handlers() override {
                return {
                    { REG_CURRTIME_ADDR, [](uint32_t addr) -> uint32_t {
                        return time(NULL);
                    }}, 
                };
            };
    };
}