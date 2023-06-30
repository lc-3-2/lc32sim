#include <iostream>
#include <set>

#include "display.hpp"
#include "exceptions.hpp"
#include "log.hpp"

namespace lc32sim {
    Display::Display() = default;

    void Display::initialize() {
        if (this->initialized) {
            throw DisplayException("Display already initialized");
        }
        this->initialized = true;

        int renderer_flags = Config.display.accelerated_rendering ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE;
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
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

        this->texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, Config.display.width, Config.display.height);
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
        key.pressed = false;
        key.map_location = map_location;
    }

    bool Display::draw(unsigned int scanline, uint16_t *video_buffer) {
        if (!this->initialized) {
            throw DisplayException("Display not initialized");
        }
        this->changed_key = nullptr;
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                this->initialized = false;
                SDL_DestroyRenderer(this->renderer);
                SDL_DestroyWindow(this->window);
                SDL_Quit();
                return false;
            } else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                for (Keybind &key : this->keys) {
                    if (key.code == e.key.keysym.sym) {
                        key.pressed = true;
                        this->changed_key = &key;
                        break;
                    }
                }
            } else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
                for (Keybind &key : this->keys) {
                    if (key.code == e.key.keysym.sym) {
                        key.pressed = false;
                        this->changed_key = &key;
                        break;
                    }
                }
            }
        }
        if (scanline < Config.display.height) {
            SDL_Rect line = {0, static_cast<int>(scanline), static_cast<int>(Config.display.width), 1};
            SDL_UpdateTexture(this->texture, &line, video_buffer + scanline * Config.display.width, Config.display.width * sizeof(uint16_t));
        }

        if (scanline == Config.display.height - 1) {
            SDL_RenderCopy(this->renderer, this->texture, nullptr, nullptr);
            // Note: this call blocks until the next vsync
            SDL_RenderPresent(this->renderer);
        }
        return true;
    }
}