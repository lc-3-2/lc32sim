#pragma once

#include <argparse/argparse.hpp>
#include <cstdint>
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
            std::string log_level = DEFAULT_LOG_LEVEL;
            struct {
                unsigned int width = 640;
                unsigned int height = 480;
                unsigned int vblank_length = 68;
                unsigned int instructions_per_scanline = 400;
                double frames_per_second = 60.0;
                bool accelerated_rendering = true;
            } display;

            struct {
                uint64_t size = (static_cast<uint64_t>(1) << 32);
                /*
                 * The simulator page size is the allocation granularity for the simulator's memory.
                 * It is not necessarily the same as the page size of the host system.
                 * This does not relate to any virtual memory systems in the simulated system.
                */
                uint64_t simulator_page_size = (static_cast<uint64_t>(1) << 12);
            } memory;

            struct {
                // https://wiki.libsdl.org/SDL2/SDL_Keycode
                std::string a = "a";
                std::string b = "b";
                std::string select = "Backspace";
                std::string start = "Return";
                std::string right = "Right";
                std::string left = "Left";
                std::string up = "Up";
                std::string down = "Down";
                std::string r = "r";
                std::string l = "l";
            } keybinds;
    };
    // This is where you "register" a new option
    #define FOR_EACH_CONFIG_OPTION(X) \
        X(log_level, "Log level") \
        X(display.width, "Display width") \
        X(display.height, "Display height") \
        X(display.vblank_length, "VBlank length") \
        X(display.instructions_per_scanline, "Instructions per scanline") \
        X(display.frames_per_second, "Framerate (FPS)") \
        X(display.accelerated_rendering, "Hardware-accelerated rendering") \
        X(memory.size, "Memory size") \
        X(memory.simulator_page_size, "Simulator page size") \
        X(keybinds.a, "\"A\" button keybind") \
        X(keybinds.b, "\"B\" button keybind") \
        X(keybinds.select, "\"Select\" button keybind") \
        X(keybinds.start, "\"Start\" button keybind") \
        X(keybinds.right, "\"Right\" button keybind") \
        X(keybinds.left, "\"Left\" button keybind") \
        X(keybinds.up, "\"Up\" button keybind") \
        X(keybinds.down, "\"Down\" button keybind") \
        X(keybinds.r, "\"R\" button keybind") \
        X(keybinds.l, "\"L\" button keybind")

    extern class Config config_instance;
    extern const class Config &Config;
}