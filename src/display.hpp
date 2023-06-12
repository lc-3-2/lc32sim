 #include <chrono>
#include <cstdint>

#include "SDL2/SDL.h"
#include "utils.hpp"

namespace lc32sim {
    using namespace std::chrono_literals;

    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    const int HBLANK_LENGTH = 68; // in pixels
    const int VBLANK_LENGTH = 68; // in pixels

    const double FRAMES_PER_SECOND = 60;
    const double FRAME_TIME_NS = 1000000000.0 / FRAMES_PER_SECOND;
    const double LINE_TIME_NS = FRAME_TIME_NS / (SCREEN_HEIGHT + VBLANK_LENGTH);
    const double PIXEL_TIME_NS = LINE_TIME_NS / (SCREEN_WIDTH + HBLANK_LENGTH);

    using duration = std::chrono::duration<double, std::nano>;
    using time_point = std::chrono::time_point<std::chrono::steady_clock, duration>;
    const auto FRAME_TIME = duration(FRAME_TIME_NS);
    const auto LINE_TIME = duration(LINE_TIME_NS);
    const auto PIXEL_TIME = duration(PIXEL_TIME_NS);
    
    class Display {
        private:
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
            void loop();
            void draw(uint16_t *video_buffer);
    };
}