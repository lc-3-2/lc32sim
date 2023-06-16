#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include "display.hpp"
#include "exceptions.hpp"
#include "memory.hpp"
#include "utils.hpp"

using endian = std::endian;

namespace lc32sim {
    Memory::Memory(unsigned int seed) : page_initialized(), seed(seed) {
        data = std::make_unique_for_overwrite<uint8_t[]>(MEM_SIZE);
        
        // Ensure that the video buffer pages are iniitalized so the display doesn't read random garbage
        constexpr uint32_t vbuf_start_page = IO_SPACE_ADDR / PAGE_SIZE;
        uint32_t vbuf_bytes = Config.display.width * Config.display.height * sizeof(uint16_t);
        uint32_t vbuf_pages = static_cast<uint32_t>((vbuf_bytes / static_cast<double>(PAGE_SIZE)) + 0.5);
        for (uint32_t page = vbuf_start_page; page < vbuf_start_page + vbuf_pages; page++) {
            if (!this->page_initialized[page]) {
                this->init_page(page);
            }
        }
    };
    Memory::Memory() : Memory(0) {}
    Memory::~Memory() {}

    void Memory::set_seed(unsigned int seed) {
        this->seed = seed;
    }

    void Memory::init_page(uint32_t page_num) {
        #ifdef DEBUG_CHECKS
        if (page_num >= NUM_PAGES) {
            throw SimulatorException("cannot initialize page " + std::to_string(page_num) + ": page_num out of range");
        }
        if (page_initialized[page_num]) {
            throw SimulatorException("cannot initialize page " + std::to_string(page_num) + ": page already initialized");
        }
        #endif

        srand(seed ^ page_num);
        for (uint64_t i = 0; i < PAGE_SIZE; i++) {
            data[(page_num * PAGE_SIZE) + i] = static_cast<uint8_t>(rand());
        }
        page_initialized[page_num] = true;
    }
    template<typename T> T Memory::read(uint32_t addr) {
        uint32_t page_num = addr / PAGE_SIZE;
        if constexpr (sizeof(T) > 1) {
            if (Config.allow_unaligned_access) {
                uint32_t end_page_num = (addr + sizeof(T) - 1) / PAGE_SIZE;
                for (uint32_t page = page_num + 1; page <= end_page_num; page++) {
                    if (!this->page_initialized[page]) {
                        this->init_page(page);
                    }
                }
            } else {
                if (addr % sizeof(T) != 0) {
                    throw UnalignedMemoryAccessException(addr, sizeof(T));
                }
            }
        }
        if (!this->page_initialized[page_num]) {
            this->init_page(page_num);
        }
        T ret = *reinterpret_cast<T*>(&this->data[addr]);
        if constexpr(sizeof(T) > 1 && endian::native == endian::big) {
            return std::byteswap(ret);
        } else {
            return ret;
        }
    }
    template <typename T> void Memory::write(uint32_t addr, T val) {
        uint32_t page_num = addr / PAGE_SIZE;
        if constexpr (sizeof(T) > 1) {
            if (Config.allow_unaligned_access) {
                uint32_t end_page_num = (addr + sizeof(T) - 1) / PAGE_SIZE;
                for (uint32_t page = page_num + 1; page <= end_page_num; page++) {
                    if (!this->page_initialized[page]) {
                        this->init_page(page);
                    }
                }
            } else {
                if (addr % sizeof(T) != 0) {
                    throw UnalignedMemoryAccessException(addr, sizeof(T));
                }
            }
        }
        if (!this->page_initialized[page_num]) {
            this->init_page(page_num);
        }
        if constexpr (endian::native == endian::big) {
            val = std::byteswap(val);
        }
        *reinterpret_cast<T*>(&this->data[addr]) = val;
    }

    template char Memory::read<char>(uint32_t addr);
    template uint8_t Memory::read<uint8_t>(uint32_t addr);
    template uint16_t Memory::read<uint16_t>(uint32_t addr);
    template uint32_t Memory::read<uint32_t>(uint32_t addr);
    template void Memory::write<uint8_t>(uint32_t addr, uint8_t val);
    template void Memory::write<uint16_t>(uint32_t addr, uint16_t val);
    template void Memory::write<uint32_t>(uint32_t addr, uint32_t val);

    void Memory::load_elf(ELFFile& elf) {
        for (uint16_t i = 0; i < elf.get_header().phnum; i++) {
            auto ph = elf.get_program_header(i);
            if (ph.type == segment_type::LOADABLE) {
                // Ensure that all the pages needed are loaded
                uint32_t start_page = ph.vaddr / PAGE_SIZE;
                uint32_t end_page = (ph.vaddr + ph.memsz - 1) / PAGE_SIZE;
                for (uint32_t page = start_page; page <= end_page; page++) {
                    if (!this->page_initialized[page]) {
                        this->init_page(page);
                    }
                }
                elf.read_chunk(&this->data[ph.vaddr], ph.offset, ph.filesz);
            }
        }
    }

    uint16_t *Memory::get_video_buffer() {
        return reinterpret_cast<uint16_t*>(&this->data[IO_SPACE_ADDR]);
    }
}