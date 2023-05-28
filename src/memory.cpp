#include <cstdlib>

#include "exceptions.hpp"
#include "memory.hpp"

using endian = std::endian;

namespace lc32sim {
    Memory::Memory(unsigned int seed) : page_initialized(), seed(seed) {
        data = std::make_unique_for_overwrite<uint8_t[]>(MEM_SIZE);
    };
    Memory::Memory() : Memory(0) {}
    Memory::~Memory() {}

    void Memory::set_seed(unsigned int seed) {
        this->seed = seed;
    }

    void Memory::init_page(uint32_t page_num) {
        srand(seed ^ page_num);
        for (uint64_t i = 0; i < PAGE_SIZE; i++) {
            data[i] = static_cast<uint8_t>(rand());
        }
        page_initialized[page_num] = true;
    }
    template<typename T> T Memory::read(uint32_t addr) {
        uint32_t page_num = addr / PAGE_SIZE;
        if constexpr (sizeof(T) > 1) {
            if (addr % sizeof(T) != 0) {
                throw UnalignedMemoryAccessException(addr, sizeof(T));
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
            if (addr % sizeof(T) != 0) {
                throw UnalignedMemoryAccessException(addr, sizeof(T));
            }
        }
        if (this->page_initialized[page_num]) {
            this->init_page(page_num);
        }
        if constexpr (endian::native == endian::big) {
            val = std::byteswap(val);
        }
        *reinterpret_cast<T*>(&this->data[addr]) = val;
    }

    template uint8_t Memory::read<uint8_t>(uint32_t addr);
    template uint16_t Memory::read<uint16_t>(uint32_t addr);
    template uint32_t Memory::read<uint32_t>(uint32_t addr);
    template void Memory::write<uint8_t>(uint32_t addr, uint8_t val);
    template void Memory::write<uint16_t>(uint32_t addr, uint16_t val);
    template void Memory::write<uint32_t>(uint32_t addr, uint32_t val);
}