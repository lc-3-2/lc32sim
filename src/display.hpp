#include <cstdint>

#include "SDL2/SDL.h"

namespace lc32sim {
    const int SCREEN_WIDTH = 640;
    const int SCREEN_HEIGHT = 480;
    
    class Display {
        private:
            bool initialized;
            SDL_Renderer* renderer;
            SDL_Window* window;
        public:
            Display();
            void initialize();
            void loop(volatile uint16_t *video_buffer);
    };
}