#pragma once

#include <functional>
#include <string>
#include <vector>
#include <utility>

namespace lc32sim {
    // GBA I/O registers
    const uint32_t REG_VCOUNT_ADDR = 0xF0000000;
    const uint32_t REG_KEYINPUT_ADDR = 0xF0000002;
    const uint32_t DMA_CONTROLLER_ADDR = 0xF000000C;
    const uint32_t VIDEO_BUFFER_ADDR = 0xFC000000;

    // Clock device
    //
    // Gets the current time on the host, in milliseconds, and exposes that to
    // the simulator. Unfortunately, this information cannot be packed into 32
    // bits, so we do the following.
    //
    // We have data registers for the number of seconds in the unix timestamp,
    // and a separate register for milliseconds. These do not update on their
    // own. Updates are triggered by writing to the status register.
    //
    // In the future, the status register may have flags. Thus, current
    // applications should always write zero to the status register to trigger
    // an update.
    const uint32_t CLOCK_STATUS_ADDR = 0xF0000010;
    const uint32_t CLOCK_MIL_ADDR = 0xF0000014;
    const uint32_t CLOCK_SEC_ADDR = 0xF0000018;

    // RNG Device
    // Returns a random number on read
    const uint32_t RNG_ADDR = 0xF000001C;

    // Others
    const uint32_t FS_CONTROLLER_ADDR = 0xF0000020;

    using read_handler = std::function<uint32_t(uint32_t)>;
    using write_handler = std::function<uint32_t(uint32_t, uint32_t)>;
    using read_handlers = std::vector<std::pair<uint32_t, read_handler>>;
    using write_handlers = std::vector<std::pair<uint32_t, write_handler>>;

    class IODevice {
        public:
            virtual std::string get_name() = 0;
            virtual read_handlers get_read_handlers() { return {}; };
            virtual write_handlers get_write_handlers() { return {}; };
    };
}
