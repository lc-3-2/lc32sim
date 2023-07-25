#pragma once
#include <string>

#include "config.hpp"
#include "iodevice.hpp"
#include "SDL2/SDL.h"
#include "sim.hpp"
#include "utils.hpp"

namespace lc32sim {
    struct Keybind {
        SDL_Keycode code;
        int map_location;
    };
    
    class Display : public IODevice {
        private:
            static const size_t NUM_KEYS = 10;
            uint16_t &scanline;
            double target_time = 0;
            double ticks_per_frame;
            Keybind keys[NUM_KEYS];

            SDL_Renderer *renderer = nullptr;
            SDL_Window *window = nullptr;
            SDL_Texture *texture = nullptr;

            void initialize_key(std::string key_name, size_t map_location);
        public:
            Display(uint16_t &scanline);
            bool update(Simulator &sim);

            // IODevice methods
            std::string get_name() override { return "SDL2 Display"; };
            read_handlers get_read_handlers() override { return {
                { REG_VCOUNT_ADDR, [this](uint32_t addr) -> uint32_t {
                    return this->scanline;
                }},
                { REG_KEYINPUT_ADDR, [this](uint32_t addr) -> uint32_t {
                    uint32_t keyinput = 0;
                    const uint8_t *keystate = SDL_GetKeyboardState(nullptr);
                    for (size_t i = 0; i < NUM_KEYS; i++) {
                        if (keystate[keys[i].code]) {
                            keyinput |= 1_u32 << keys[i].map_location;
                        }
                    }
                    return ~keyinput;
                }}
            };}
            write_handlers get_write_handlers() override { return {}; };


    };
}