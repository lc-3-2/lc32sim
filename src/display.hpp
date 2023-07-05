#pragma once

#include "config.hpp"
#include "SDL2/SDL.h"

namespace lc32sim {
    struct Keybind {
        SDL_Keycode code;
        bool pressed;
        int map_location;
    };
    
    class Display {
        private:
            double target_time = 0;
            double ticks_per_frame;

            SDL_Renderer *renderer = nullptr;
            SDL_Window *window = nullptr;
            SDL_Texture *texture = nullptr;


            void initialize_key(Keybind &key, std::string key_name, int map_location);
        public:
            Keybind *changed_key = nullptr;
            Keybind keys[10];

            Display();
            bool draw(unsigned int scanline, uint16_t *video_buffer);
    };
}