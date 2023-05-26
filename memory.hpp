#pragma once
#include <bit>
#include <cstdint>
#include <memory>

namespace lc32sim {
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "mixed-endian architectures are not supported");
    const uint64_t MEM_SIZE = 1ULL << 32;
    const uint64_t PAGE_SIZE = 1ULL << 12;
    static_assert(PAGE_SIZE % 4 == 0, "PAGE_SIZE must be a multiple of 4");
    const uint32_t IO_SPACE_ADDR = 0xF0000000;
    static_assert(IO_SPACE_ADDR % PAGE_SIZE == 0, "IO_SPACE_ADDR must be page aligned");
    const uint32_t NUM_PAGES = MEM_SIZE / PAGE_SIZE;

    class Memory {
        private:
            bool page_initialized[NUM_PAGES];
            unsigned int seed;
            std::unique_ptr<uint8_t[]> data;
            void init_page(uint32_t page_num);
        public:
            Memory(unsigned int seed);
            Memory();
            ~Memory();

            void set_seed(unsigned int seed);

            template<typename T> T read(uint32_t addr);
            template<typename T> void write(uint32_t addr, T val);
    };
}