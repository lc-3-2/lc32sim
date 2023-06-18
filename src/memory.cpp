#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include "display.hpp"
#include "exceptions.hpp"
#include "memory.hpp"
#include "utils.hpp"

using endian = std::endian;

#define NUM_PAGES (((Config.memory.size - 1) / Config.memory.simulator_page_size) + 1)

namespace lc32sim {
    Memory::Memory(unsigned int seed) : seed(seed) {
        if (Config.memory.size > (static_cast<uint64_t>(1) << 32)) {
            throw SimulatorException("memory size must be <= 4 GiB");
        }
        if (Config.memory.simulator_page_size > Config.memory.size) {
            throw SimulatorException("simulator page size must be <= memory size");
        }
        if (Config.memory.simulator_page_size % 4 != 0) {
            throw SimulatorException("simulator page size must be a multiple of 4");
        }

        data = std::make_unique_for_overwrite<uint8_t[]>(Config.memory.size);
        page_initialized = std::make_unique_for_overwrite<bool[]>(NUM_PAGES);
        
        // Ensure that the video buffer pages are iniitalized so the display doesn't read random garbage
        uint32_t vbuf_start_page = VIDEO_BUFFER_ADDR / Config.memory.simulator_page_size;
        uint32_t vbuf_bytes = Config.display.width * Config.display.height * sizeof(uint16_t);
        uint32_t vbuf_pages = static_cast<uint32_t>((vbuf_bytes / static_cast<double>(Config.memory.simulator_page_size)) + 0.5);
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
        if (page_num >= ) {
            throw SimulatorException("cannot initialize page " + std::to_string(page_num) + ": page_num out of range");
        }
        if (page_initialized[page_num]) {
            throw SimulatorException("cannot initialize page " + std::to_string(page_num) + ": page already initialized");
        }
        #endif

        srand(seed ^ page_num);
        for (uint64_t i = 0; i < Config.memory.simulator_page_size; i++) {
            uint64_t addr = (page_num * Config.memory.simulator_page_size) + i;
            if (addr >= Config.memory.size) {
                break;
            }
            data[addr] = static_cast<uint8_t>(rand());
        }
        page_initialized[page_num] = true;
    }
    template<typename T> T Memory::read(uint32_t addr) {
        uint32_t page_num = addr / Config.memory.simulator_page_size;
        if constexpr (sizeof(T) > 1) {
            if (Config.memory.allow_unaligned_access) {
                uint32_t end_page_num = (addr + sizeof(T) - 1) / Config.memory.simulator_page_size;
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
        uint32_t page_num = addr / Config.memory.simulator_page_size;
        if constexpr (sizeof(T) > 1) {
            if (Config.memory.allow_unaligned_access) {
                uint32_t end_page_num = (addr + sizeof(T) - 1) / Config.memory.simulator_page_size;
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
                uint32_t start_page = ph.vaddr / Config.memory.simulator_page_size;
                uint32_t end_page = (ph.vaddr + ph.memsz - 1) / Config.memory.simulator_page_size;
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
        return reinterpret_cast<uint16_t*>(&this->data[VIDEO_BUFFER_ADDR]);
    }
}