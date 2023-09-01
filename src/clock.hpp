#pragma once
#include <chrono>

#include "iodevice.hpp"

namespace lc32sim {
    class Clock : public IODevice {
        private:
            uint32_t time_mil = 0;
            uint32_t time_sec = 0;

        public:
            std::string get_name() override { return "Clock"; };
            read_handlers get_read_handlers() override {
                return {
                    { CLOCK_MIL_ADDR, [this](uint32_t val) -> uint32_t {
                        return this->time_mil;
                    }},
                    { CLOCK_SEC_ADDR, [this](uint32_t val) -> uint32_t {
                        return this->time_sec;
                    }},
                };
            };
            write_handlers get_write_handlers() override {
                return {
                    {CLOCK_STATUS_ADDR, [this](uint32_t oldval, uint32_t val) -> uint32_t {
                        // Get the time since epoch as a duration
                        const auto now =
                            std::chrono::system_clock::now().time_since_epoch();
                        // Count the number of seconds and set that
                        const auto now_sec =
                            std::chrono::duration_cast<std::chrono::seconds>(now);
                        this->time_sec = now_sec.count();
                        // Count the number of milliseconds and set that
                        const auto now_mil =
                            std::chrono::duration_cast<std::chrono::milliseconds>(now);
                        this->time_mil = (now_mil % std::chrono::milliseconds(1000)).count();
                        // Return zero for read
                        return 0;
                    }},
                };
            };
    };
}
