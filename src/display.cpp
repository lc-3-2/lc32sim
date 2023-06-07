#include <iostream>

#include "display.hpp"
#include "exceptions.hpp"

namespace lc32sim {
    Display::Display() : initialized(false), renderer(nullptr), window(nullptr) {}

    void Display::initialize() {
        if (this->initialized) {
            throw DisplayException("Display already initialized");
        }
        this->initialized = true;

        const int renderer_flags = SDL_RENDERER_ACCELERATED;
        const int window_flags = SDL_WINDOW_ALLOW_HIGHDPI;

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw DisplayException("SDL could not be initialized", SDL_GetError());
        }

        this->window = SDL_CreateWindow("LC32 Simulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, window_flags);
        if (!this->window) {
            throw DisplayException("Window could not be created", SDL_GetError());
        }

        this->renderer = SDL_CreateRenderer(this->window, -1, renderer_flags);
        if (!this->renderer) {
            throw DisplayException("Renderer could not be created", SDL_GetError());
        }

        int render_width, render_height;
        SDL_GetRendererOutputSize(this->renderer, &render_width, &render_height);
        std::cout << "Window size: " << render_width << "x" << render_height << std::endl;
        SDL_RenderSetScale(renderer, render_width / SCREEN_WIDTH, render_height / SCREEN_HEIGHT);
    }

    void Display::loop(volatile uint16_t *video_buffer) {
        if (!this->initialized) {
            throw DisplayException("Display not initialized");
        }
        SDL_SetRenderDrawColor(this->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(this->renderer);
        SDL_RenderPresent(this->renderer);
        while (true) {
            SDL_Event e;
            SDL_PollEvent(&e);
            if (e.type == SDL_QUIT) {
                break;
            }
            for (int row = 0; row < SCREEN_HEIGHT; row++) {
                for (int col = 0; col < SCREEN_WIDTH; col++) {
                    int index = row * SCREEN_WIDTH + col;
                    uint16_t pixel = video_buffer[index];
                    uint8_t r = (pixel & 0x7C00) >> 7;
                    uint8_t g = (pixel & 0x03E0) >> 2;
                    uint8_t b = (pixel & 0x001F) << 3;
                    SDL_SetRenderDrawColor(this->renderer, r, g, b, 0xFF);
                    SDL_RenderDrawPoint(this->renderer, col, row);
                }
            }
            SDL_RenderPresent(this->renderer);
        }
        SDL_DestroyRenderer(this->renderer);
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }
}