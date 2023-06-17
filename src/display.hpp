#pragma once

#include <chrono>
#include <cstdint>

#include "config.hpp"
#include "SDL2/SDL.h"

namespace lc32sim {
    using duration = std::chrono::duration<double, std::nano>;
    using time_point = std::chrono::time_point<std::chrono::steady_clock, duration>;
    
    class Display {
        private:
            static duration FRAME_TIME;
            static duration LINE_TIME;
            static duration PIXEL_TIME;

            static uint32_t LC32_EVENT_DRAWPIXELS;
            static uint32_t LC32_EVENT_PRESENT;

            bool initialized = false;
            SDL_Renderer* renderer = nullptr;
            SDL_Window* window = nullptr;

            time_point last_frame;
            bool last_frame_initialized = false;
            int32_t next_to_draw = 0;

            void push_draw_event(uint32_t start_pos, uintptr_t num_pixels, uint16_t *buffer);
            void push_present_event();
        public:
            Display();
            void initialize();
            bool iterate();
            void draw(uint16_t *video_buffer);
    };
}