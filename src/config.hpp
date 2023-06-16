#pragma once

namespace lc32sim {
    class Config
    {
        private:
            const std::string CONFIG_FILE_NAME = "lc32sim.json";
            void set_defaults();

        public:
            Config();
            Config(Config const&) = delete;
            void operator=(Config const&) = delete;

            // Configurable options:
            struct {
                int width = 640;
                int height = 480;
                int hblank_length = 68;
                int vblank_length = 68;
                double frames_per_second = 60.0;
                bool accelerated_rendering = true;
            } display;
            bool allow_unaligned_access = false;
    };
    extern const class Config &Config;
}