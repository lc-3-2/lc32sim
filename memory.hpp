#pragma once
#include <cstdint>
#include <memory>

namespace lc32sim {
    const uint64_t MEM_SIZE = 1ULL << 32;
    const uint64_t PAGE_SIZE = 1ULL << 12;
    static_assert(PAGE_SIZE % 4 == 0, "PAGE_SIZE must be a multiple of 4");
    const uint32_t IO_SPACE_ADDR = 0xF0000000;
    static_assert(IO_SPACE_ADDR % PAGE_SIZE == 0, "IO_SPACE_ADDR must be page aligned");
    const uint32_t NUM_PAGES = MEM_SIZE / PAGE_SIZE;

    class Memory {
        private:
            struct Page {
                std::unique_ptr<uint8_t[]> data;
                Page();
                ~Page();
                void init(unsigned int seed);
                bool is_initialized() const;
            };
            Page pages[NUM_PAGES];
            unsigned int seed;
            void init_page(uint32_t page_num);
        public:
            Memory(unsigned int seed);
            Memory();
            ~Memory();

            void set_seed(unsigned int seed);

            uint8_t read_byte(uint32_t addr);
            uint16_t read_half(uint32_t addr);
            uint32_t read_word(uint32_t addr);
            void write_byte(uint32_t addr, uint8_t data);
            void write_half(uint32_t addr, uint16_t data);
            void write_word(uint32_t addr, uint32_t data);
    };
}