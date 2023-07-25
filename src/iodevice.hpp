#pragma once

#include <string>
#include <vector>
#include <utility>

namespace lc32sim {
    const uint32_t REG_VCOUNT_ADDR = 0xF0000000;
    const uint32_t REG_KEYINPUT_ADDR = 0xF0000002;
    const uint32_t DMA_CONTROLLER_ADDR = 0xF000000C;
    const uint32_t VIDEO_BUFFER_ADDR = 0xFC000000;

    using read_handler = std::function<uint32_t(uint32_t)>;
    using write_handler = std::function<void(uint32_t, uint32_t)>;
    using read_handlers = std::vector<std::pair<uint32_t, read_handler>>;
    using write_handlers = std::vector<std::pair<uint32_t, write_handler>>;

    class IODevice {
        public:
            virtual std::string get_name() = 0;
            virtual read_handlers get_read_handlers() = 0;
            virtual write_handlers get_write_handlers() = 0;
    };
}