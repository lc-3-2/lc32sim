#pragma once
#include <chrono>

#include "iodevice.hpp"

namespace lc32sim {
    class Clock : public IODevice {
        public:
            std::string get_name() override { return "Clock"; };
            read_handlers get_read_handlers() override {
                return {
                    { REG_CURRTIME_ADDR, [](uint32_t addr) -> uint32_t {
                        const auto now =
                            std::chrono::system_clock::now().time_since_epoch();
                        const auto now_sec =
                            std::chrono::duration_cast<std::chrono::seconds>(now);
                        return now_sec.count();
                    }},
                    { REG_HIGHRESTIME_ADDR, [](uint32_t addr) -> uint32_t {
                        const auto now =
                            std::chrono::system_clock::now().time_since_epoch();
                        const auto now_milli =
                            std::chrono::duration_cast<std::chrono::milliseconds>(now);
                        const auto now_milli_rem =
                            now_milli % std::chrono::milliseconds(std::chrono::seconds(1));
                        return now_milli_rem.count();
                    }},
                };
            };
    };
}
