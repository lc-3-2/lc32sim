#pragma once
#include <bit>
#include <cstdint>
#include <memory>
#include <tuple>

#include "elf_file.hpp"
#include "utils.hpp"

namespace lc32sim {
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "mixed-endian architectures are not supported");

    // GBA I/O devices
    const uint32_t REG_VCOUNT_ADDR = 0xF0000000;
    const uint32_t REG_KEYINPUT_ADDR = 0xF0000002;
    const uint32_t DMA_CONTROLLER_ADDR = 0xF000000C;
    const uint32_t VIDEO_BUFFER_ADDR = 0xFC000000;

    // LC-3 I/O Devices (not yet implemented, use TRAPs)
    const uint32_t KBSR_ADDR = 0xFFFFFE00;
    const uint32_t KBDR_ADDR = 0xFFFFFE02;
    const uint32_t DSR_ADDR = 0xFFFFFE04;
    const uint32_t DDR_ADDR = 0xFFFFFE06;
    const uint32_t TMR_ADDR = 0xFFFFFE08;
    const uint32_t TMI_ADDR = 0xFFFFFE0A;
    const uint32_t MPR_ADDR = 0xFFFFFE12;
    const uint32_t PSR_ADDR = 0xFFFFFFFC;
    const uint32_t MCR_ADDR = 0xFFFFFFFE;

    // DMA details can be opaque to most code
    // Therefore, they are hidden away in a namespace
    namespace dma {
        struct DMAController {
            uint32_t source;
            uint32_t destination;
            uint32_t control;
            
            DMAController() : source(0), destination(0), control(0) {}
            void handle();
        };

        static const uint32_t DMA_DESTINATION = 3_u32 << 21;
        static const uint32_t DMA_DESTINATION_INCREMENT = 0_u32 << 21;
        static const uint32_t DMA_DESTINATION_DECREMENT = 1_u32 << 21;
        static const uint32_t DMA_DESTINATION_FIXED = 2_u32 << 21;
        static const uint32_t DMA_DESTINATION_RESET = 3_u32 << 21;

        static const uint32_t DMA_SOURCE = 3_u32 << 23;
        static const uint32_t DMA_SOURCE_INCREMENT = 0_u32 << 23;
        static const uint32_t DMA_SOURCE_DECREMENT = 1_u32 << 23;
        static const uint32_t DMA_SOURCE_FIXED = 2_u32 << 23;

        static const uint32_t DMA_WIDTH = 1_u32 << 26;
        static const uint32_t DMA_16 = 0_u32 << 26;
        static const uint32_t DMA_32 = 1_u32 << 26;

        static const uint32_t DMA_TIMING = 3_u32 << 28;
        static const uint32_t DMA_NOW = 0_u32 << 28;
        static const uint32_t DMA_AT_VBLANK = 1_u32 << 28;
        static const uint32_t DMA_AT_HBLANK = 2_u32 << 28;
        static const uint32_t DMA_AT_REFRESH = 3_u32 << 28;

        // 1-bit boolean flags
        static const uint32_t DMA_REPEAT = 1_u32 << 25; // used for DMA_AT_(V|H)BLANK
        static const uint32_t DMA_IRQ = 1_u32 << 30; // raise an interrupt when DMA is done
        static const uint32_t DMA_ON = 1_u32 << 31;

        static const uint32_t DMA_NUM_TRANSFERS = 0xFFFF_u32;
    }
    
    class Memory {
        private:
            std::unique_ptr<bool[]> page_initialized;
            unsigned int seed;
            std::unique_ptr<uint8_t[]> data;
            void init_page(uint32_t page_num);
            void init_io_devices();

        public:
            Memory(unsigned int seed);
            Memory();
            ~Memory();

            void set_seed(unsigned int seed);
            void load_elf(ELFFile& elf);
            template<typename T> T read(uint32_t addr);
            template<typename T> void write(uint32_t addr, T val);

            void set_vcount(uint16_t scanline);
            uint16_t *get_reg_keyinput();
            void handle_dma();
            uint16_t *get_video_buffer();
    };
}