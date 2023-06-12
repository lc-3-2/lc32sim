#pragma once

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace lc32sim {
    class Config
    {
        private:
            const std::string CONFIG_FILE_NAME = "lc32sim.json";
            json data;
            void set_defaults();

        public:
            Config();
            Config(Config const&) = delete;
            void operator=(Config const&) = delete;
            int display_width;
            int display_height;
    };
    extern const Config &config;
}