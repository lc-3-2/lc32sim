#pragma once

#include "config.hpp"
#include "SDL2/SDL.h"
#include "sim.hpp"

namespace lc32sim {
    struct Keybind {
        SDL_Keycode code;
        int map_location;
    };
    
    class Display {
        private:
            double target_time = 0;
            double ticks_per_frame;
            Keybind keys[10];

            SDL_Renderer *renderer = nullptr;
            SDL_Window *window = nullptr;
            SDL_Texture *texture = nullptr;


            void initialize_key(Keybind &key, std::string key_name, int map_location);
        public:
            Display();
            bool update(unsigned int scanline, Simulator &sim);
    };
}