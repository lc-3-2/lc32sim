#include <iostream>

#include "display.hpp"
#include "exceptions.hpp"

namespace lc32sim {
    Display::Display() = default;
    uint32_t Display::LC32_EVENT_DRAWPIXELS = static_cast<uint32_t>(-1);
    uint32_t Display::LC32_EVENT_PRESENT = static_cast<uint32_t>(-1);

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

        if (LC32_EVENT_DRAWPIXELS == static_cast<uint32_t>(-1)) {
            LC32_EVENT_DRAWPIXELS = SDL_RegisterEvents(2);
            if (LC32_EVENT_DRAWPIXELS == static_cast<uint32_t>(-1)) {
                throw DisplayException("Could not register custom events");
            }
            LC32_EVENT_PRESENT = LC32_EVENT_DRAWPIXELS + 1;
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

        SDL_SetRenderDrawColor(this->renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(this->renderer);
        SDL_RenderPresent(this->renderer);
    }

    void Display::loop() {
        if (!this->initialized) {
            throw DisplayException("Display not initialized");
        }
        while (true) {
            SDL_Event e;
            SDL_PollEvent(&e);
            if (e.type == SDL_QUIT) {
                this->initialized = false;
                SDL_DestroyRenderer(this->renderer);
                SDL_DestroyWindow(this->window);
                SDL_Quit();
                break;
            } else if (e.type == Display::LC32_EVENT_DRAWPIXELS) {
                uint32_t start_pos = static_cast<uint32_t>(e.user.code);
                uintptr_t num_pixels = reinterpret_cast<uintptr_t>(e.user.data1);
                uint16_t *buffer = static_cast<uint16_t*>(e.user.data2);

                for (uintptr_t i = 0; i < num_pixels; i++) {
                    int row = (start_pos + i) / SCREEN_WIDTH;
                    int col = (start_pos + i) % SCREEN_WIDTH;
                    uint16_t pixel = buffer[i];
                    uint8_t r = (pixel & 0x7C00) >> 7;
                    uint8_t g = (pixel & 0x03E0) >> 2;
                    uint8_t b = (pixel & 0x001F) << 3;
                    SDL_SetRenderDrawColor(this->renderer, r, g, b, 0xFF);
                    SDL_RenderDrawPoint(this->renderer, col, row);
                }

            } else if (e.type == LC32_EVENT_PRESENT) {
                SDL_RenderPresent(this->renderer);
            }
        }
    }
    void Display::draw(uint16_t *video_buffer) {
        #ifdef DEBUG_CHECKS
        if (!this->initialized) {
            throw DisplayException("Display not initialized");
        }
        #endif

        time_point now = std::chrono::steady_clock::now();
        duration elapsed = now - this->last_frame;

        if (elapsed > FRAME_TIME) {
            // We need to draw the previous frame
            uintptr_t num_pixels = (SCREEN_WIDTH * SCREEN_HEIGHT) - this->next_to_draw;
            if (num_pixels > 0) {
                std::cout << "Drawing " << std::dec << num_pixels << " pixels #1" << std::endl;
                push_draw_event(this->next_to_draw, num_pixels, video_buffer);
            }
            std::cout << "Presenting frame" << std::endl;
            push_present_event();
            this->next_to_draw = 0;
            // Loop to skip frames if we're too far behind
            while ((now - this->last_frame) > FRAME_TIME) {
                this->last_frame += FRAME_TIME;
            }
        } else {
            int elapsed_pixels = (elapsed / PIXEL_TIME);
            int current_row = elapsed_pixels / (SCREEN_WIDTH + HBLANK_LENGTH);
            int current_col = elapsed_pixels % (SCREEN_WIDTH + HBLANK_LENGTH);
            int end_pos = (current_row * SCREEN_WIDTH) + current_col;
            if (end_pos >= this->next_to_draw) {
                uintptr_t num_pixels = end_pos - this->next_to_draw + 1;
                std::cout << "Drawing " << std::dec << num_pixels << " pixels #2" << std::endl;
                push_draw_event(this->next_to_draw, num_pixels, video_buffer);
                this->next_to_draw = end_pos + 1;
            }
        }
    }

    void Display::push_draw_event(uint32_t start_pos, uintptr_t num_pixels, uint16_t *buffer) {
        SDL_Event e;
        e.type = LC32_EVENT_DRAWPIXELS;
        e.user.code = static_cast<int32_t>(start_pos);
        e.user.data1 = reinterpret_cast<void*>(num_pixels);
        e.user.data2 = static_cast<void*>(buffer);
        SDL_PushEvent(&e);
    }
    void Display::push_present_event() {
        SDL_Event e;
        e.type = LC32_EVENT_PRESENT;
        SDL_PushEvent(&e);
    }
}