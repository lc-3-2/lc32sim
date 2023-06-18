#pragma once
#include <bit>
#include <cstdint>
#include <memory>

#include "elf_file.hpp"

namespace lc32sim {
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "mixed-endian architectures are not supported");
    const uint32_t VIDEO_BUFFER_ADDR = 0xF0000000;

    class Memory {
        private:
            std::unique_ptr<bool[]> page_initialized;
            unsigned int seed;
            std::unique_ptr<uint8_t[]> data;
            void init_page(uint32_t page_num);
        public:
            Memory(unsigned int seed);
            Memory();
            ~Memory();

            void set_seed(unsigned int seed);
            void load_elf(ELFFile& elf);
            uint16_t *get_video_buffer();

            template<typename T> T read(uint32_t addr);
            template<typename T> void write(uint32_t addr, T val);
    };
}