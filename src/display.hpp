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
            bool initialized = false;
            SDL_Renderer *renderer = nullptr;
            SDL_Window *window = nullptr;
            SDL_Texture *texture = nullptr;


            void initialize_key(Keybind &key, std::string key_name, int map_location);
        public:
            Keybind *changed_key = nullptr;
            Keybind keys[10];

            Display();
            void initialize();
            bool draw(unsigned int scanline, uint16_t *video_buffer);
    };
}