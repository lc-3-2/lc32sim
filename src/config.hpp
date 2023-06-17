#pragma once

#include <argparse/argparse.hpp>
#include <string>

namespace lc32sim {
    // log level used if not specified in config file & prior to config file being loaded
    const std::string DEFAULT_LOG_LEVEL = "INFO";
    class Config
    {
        private:
            const std::string CONFIG_FILE_NAME = "lc32sim.json";
            void set_defaults();

        public:
            Config();
            Config(Config const&) = delete;
            void operator=(Config const&) = delete;
            void load_config(argparse::ArgumentParser program);

            // Default values for config options
            struct {
                int width = 640;
                int height = 480;
                int hblank_length = 68;
                int vblank_length = 68;
                double frames_per_second = 60.0;
                bool accelerated_rendering = true;
            } display;
            bool allow_unaligned_access = false;
            std::string log_level = DEFAULT_LOG_LEVEL;

    };
    // This is where you "register" a new option
    #define FOR_EACH_CONFIG_OPTION(X) \
        X(display.width) \
        X(display.height) \
        X(display.hblank_length) \
        X(display.vblank_length) \
        X(display.frames_per_second) \
        X(display.accelerated_rendering) \
        X(allow_unaligned_access) \
        X(log_level) \

    extern class Config config_instance;
    extern const class Config &Config;
}