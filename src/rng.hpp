#pragma once
#include <random>

#include "iodevice.hpp"

namespace lc32sim {
    class RNG : public IODevice {
        private:
            std::random_device rd;
            std::uniform_int_distribution<uint32_t> dist;
        public:
            RNG() : rd("/dev/urandom"), dist() {}
            std::string get_name() override { return "RNG"; };
            read_handlers get_read_handlers() override {
                return {
                    { RNG_ADDR, [this](uint32_t val) -> uint32_t {
                        return this->dist(this->rd);
                    }},
                };
            };
    };
}
