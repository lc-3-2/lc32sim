#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <unordered_map>

#include "display.hpp"
#include "exceptions.hpp"
#include "memory.hpp"
#include "utils.hpp"

#define NUM_PAGES (((Config.memory.size - 1) / Config.memory.simulator_page_size) + 1)

namespace lc32sim {
    Memory::Memory(unsigned int seed) : seed(seed), pre_read_hooks(), pre_write_hooks() {
        if (Config.memory.size > (1_u64 << 32)) {
            throw SimulatorException("memory size must be <= 4 GiB");
        }
        if (Config.memory.simulator_page_size > Config.memory.size) {
            throw SimulatorException("simulator page size must be <= memory size");
        }
        if (Config.memory.simulator_page_size % 4 != 0) {
            throw SimulatorException("simulator page size must be a multiple of 4");
        }
        if (Config.memory.user_space_max > Config.memory.size) {
            throw SimulatorException("user space must be <= memory size");
        }
        if (Config.memory.user_space_min > Config.memory.user_space_max) {
            throw SimulatorException("user space min must be <= user space max");
        }
        if (Config.memory.io_space_min > Config.memory.size) {
            throw SimulatorException("I/O space must be <= memory size");
        }

        data = std::make_unique_for_overwrite<uint8_t[]>(Config.memory.size);
        page_initialized = std::make_unique<bool[]>(NUM_PAGES);
    };
    Memory::Memory() : Memory(0) {}
    Memory::~Memory() {}

    void Memory::set_seed(unsigned int seed) {
        this->seed = seed;
    }

    void Memory::init_page(uint32_t page_num) {
        assert(page_num < NUM_PAGES);
        assert(!page_initialized[page_num]);
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

                // Not all the data may be provided by the file. The remainder
                // should be zeros. Therefore, compute how much will come from
                // the file and how much will be zeros.
                uint32_t file_amt = std::min(ph.filesz, ph.memsz);
                uint32_t zero_amt = ph.memsz <= ph.filesz ? UINT32_C(0) : ph.memsz - ph.filesz;
                // Populate from the file
                elf.read_chunk(&this->data[ph.vaddr], ph.offset, file_amt);
                // Set the rest to zero
                std::memset(&this->data[ph.vaddr + file_amt], 0, zero_amt);
            }
        }
    }

    void Memory::add_pre_read_hook(uint32_t addr, std::function<uint32_t(uint32_t)> hook) {
        if (this->pre_read_hooks.find(addr) != this->pre_read_hooks.end()) {
            throw SimulatorException("pre-read hook already exists for address " + std::to_string(addr));
        }
        this->pre_read_hooks[addr] = hook;
    }

    void Memory::add_pre_write_hook(uint32_t addr, std::function<void(uint32_t, uint32_t)> hook) {
        if (this->pre_write_hooks.find(addr) != this->pre_write_hooks.end()) {
            throw SimulatorException("pre-write hook already exists for address " + std::to_string(addr));
        }
        this->pre_write_hooks[addr] = hook;
    }
}