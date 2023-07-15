#include <iostream>
#include <set>

#include "display.hpp"
#include "exceptions.hpp"
#include "log.hpp"
#include "memory.hpp"

namespace lc32sim {
    Display::Display() {
        this->ticks_per_frame = 1000.0 / Config.display.frames_per_second;

        int renderer_flags = Config.display.accelerated_rendering ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE;
        const int window_flags = SDL_WINDOW_ALLOW_HIGHDPI;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw DisplayException("SDL could not be initialized", SDL_GetError());
        }

        this->window = SDL_CreateWindow("LC3.2 Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Config.display.width, Config.display.height, window_flags);
        if (!this->window) {
            throw DisplayException("Window could not be created", SDL_GetError());
        }

        this->renderer = SDL_CreateRenderer(this->window, -1, renderer_flags);
        if (!this->renderer) {
            throw DisplayException("Renderer could not be created", SDL_GetError());
        }

        this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_BGR565, SDL_TEXTUREACCESS_STREAMING, Config.display.width, Config.display.height);
        if (!this->texture) {
            throw DisplayException("Texture could not be created", SDL_GetError());
        }

        int render_width, render_height;
        SDL_GetRendererOutputSize(this->renderer, &render_width, &render_height);
        SDL_RenderSetScale(renderer, render_width / Config.display.width, render_height / Config.display.height);

        initialize_key(keys[0], Config.keybinds.a, 0);
        initialize_key(keys[1], Config.keybinds.b, 1);
        initialize_key(keys[2], Config.keybinds.select, 2);
        initialize_key(keys[3], Config.keybinds.start, 3);
        initialize_key(keys[4], Config.keybinds.right, 4);
        initialize_key(keys[5], Config.keybinds.left, 5);
        initialize_key(keys[6], Config.keybinds.up, 6);
        initialize_key(keys[7], Config.keybinds.down, 7);
        initialize_key(keys[8], Config.keybinds.r, 8);
        initialize_key(keys[9], Config.keybinds.l, 9);
    }

    void Display::initialize_key(Keybind &key, std::string key_name, int map_location) {
        static std::set<SDL_Keycode> used_keys;
        key.code = SDL_GetKeyFromName(key_name.c_str());
        if (key.code == SDLK_UNKNOWN) {
            throw SimulatorException("Invalid keybind \"" + key_name + "\"");
        } else {
            if (used_keys.find(key.code) != used_keys.end()) {
                logger.warn << "Keybind \"" << key_name << "\" is bound to multiple keys";
            } else {
                used_keys.insert(key.code);
            }
        }
        key.map_location = map_location;
    }

    bool Display::update(unsigned int scanline, Simulator &sim) {
        uint16_t *video_buffer = sim.mem.get_video_buffer();
        uint16_t *reg_keyinput = sim.mem.get_reg_keyinput();
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                SDL_DestroyRenderer(this->renderer);
                SDL_DestroyWindow(this->window);
                SDL_Quit();
                return false;
            } else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                for (Keybind &key : this->keys) {
                    if (key.code == e.key.keysym.sym) {
                        *reg_keyinput |= 1 << key.map_location;
                        break;
                    }
                }
            } else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
                for (Keybind &key : this->keys) {
                    if (key.code == e.key.keysym.sym) {
                        *reg_keyinput &= ~(1 << key.map_location);
                        break;
                    }
                }
            }
        }

        int num_keys, mouse_x, mouse_y;
        const uint8_t *key_state = SDL_GetKeyboardState(&num_keys);
        uint32_t mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
        sim.mem.write<uint32_t>(MOUSE_X_ADDR, static_cast<uint32_t>(mouse_x));
        sim.mem.write<uint32_t>(MOUSE_Y_ADDR, static_cast<uint32_t>(mouse_y));
        sim.mem.write<uint32_t>(MOUSE_BUTTONS_ADDR, mouse_buttons);
        sim.mem.write<uint32_t>(KEYBOARD_NUM_KEYS_ADDR, static_cast<uint32_t>(num_keys));
        sim.mem.write_array<uint8_t>(KEYBOARD_KEYS_ADDR, key_state, static_cast<uint32_t>(num_keys));

        if (scanline < Config.display.height) {
            SDL_Rect line = {0, static_cast<int>(scanline), static_cast<int>(Config.display.width), 1};
            SDL_UpdateTexture(this->texture, &line, video_buffer + scanline * Config.display.width, Config.display.width * sizeof(uint16_t));
        }

        if (scanline == Config.display.height - 1) {
            SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr);
            if (this->target_time == 0) {
                this->target_time = static_cast<double>(SDL_GetTicks());
                SDL_RenderPresent(this->renderer);
            } else {
                uint64_t target_time_int = static_cast<uint64_t>(this->target_time);
                while (target_time_int > SDL_GetTicks()) {
                    SDL_Delay(1);
                }
                SDL_RenderPresent(this->renderer);
                this->target_time += this->ticks_per_frame;
            }
        }
        return true;
    }
}
