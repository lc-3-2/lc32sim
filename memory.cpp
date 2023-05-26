#include <cstdlib>

#include "exceptions.hpp"
#include "memory.hpp"

namespace lc32sim {
    Memory::Page::Page() : data(nullptr) {}
    Memory::Page::~Page() {}

    void Memory::Page::init(unsigned int seed) {
        this->data = std::make_unique_for_overwrite<uint8_t[]>(PAGE_SIZE);
        srand(seed);
        for (uint64_t i = 0; i < PAGE_SIZE; i++) {
            data[i] = static_cast<uint8_t>(rand());
        }
    }
    bool Memory::Page::is_initialized() const {
        return this->data != nullptr;
    }

    Memory::Memory(unsigned int seed) : pages(), seed(seed) {};
    Memory::Memory() : Memory(0) {}
    Memory::~Memory() {}

    void Memory::set_seed(unsigned int seed) {
        this->seed = seed;
    }

    void Memory::init_page(uint32_t page_num) {
        this->pages[page_num].init(this->seed ^ page_num);
    }
    uint8_t Memory::read_byte(uint32_t addr) {
        uint32_t page_num = addr / PAGE_SIZE;
        uint32_t page_offset = addr % PAGE_SIZE;
        if (!this->pages[page_num].is_initialized()) {
            this->init_page(page_num);
        }
        return this->pages[page_num].data[page_offset];
    }
    uint16_t Memory::read_half(uint32_t addr) {
        uint32_t page_num = addr / PAGE_SIZE;
        uint32_t page_offset = addr % PAGE_SIZE;
        if (addr % 2 != 0) {
            throw UnalignedMemoryAccessException(addr, 2);
        }
        if (!this->pages[page_num].is_initialized()) {
            this->init_page(page_num);
        }
        return *reinterpret_cast<uint16_t*>(&this->pages[page_num].data[page_offset]);
    }
    uint32_t Memory::read_word(uint32_t addr) {
        uint32_t page_num = addr / PAGE_SIZE;
        uint32_t page_offset = addr % PAGE_SIZE;
        if (addr % 4 != 0) {
            throw UnalignedMemoryAccessException(addr, 4);
        }
        if (!this->pages[page_num].is_initialized()) {
            this->init_page(page_num);
        }
        return *reinterpret_cast<uint32_t*>(&this->pages[page_num].data[page_offset]);
    }
    void Memory::write_byte(uint32_t addr, uint8_t data) {
        uint32_t page_num = addr / PAGE_SIZE;
        uint32_t page_offset = addr % PAGE_SIZE;
        if (!this->pages[page_num].is_initialized()) {
            this->init_page(page_num);
        }
        if (addr < IO_SPACE_ADDR) {
            this->pages[page_num].data[page_offset] = data;
        } else {
            *reinterpret_cast<volatile uint8_t*>(&this->pages[page_num].data[page_offset]) = data;
        }
    }
    void Memory::write_half(uint32_t addr, uint16_t data) {
        uint32_t page_num = addr / PAGE_SIZE;
        uint32_t page_offset = addr % PAGE_SIZE;
        if (addr % 2 != 0) {
            throw UnalignedMemoryAccessException(addr, 2);
        }
        if (!this->pages[page_num].is_initialized()) {
            this->init_page(page_num);
        }
        if (addr < IO_SPACE_ADDR) {
            *reinterpret_cast<uint16_t*>(&this->pages[page_num].data[page_offset]) = data;
        } else {
            *reinterpret_cast<volatile uint16_t*>(&this->pages[page_num].data[page_offset]) = data;
        }
    }
    void Memory::write_word(uint32_t addr, uint32_t data) {
        uint32_t page_num = addr / PAGE_SIZE;
        uint32_t page_offset = addr % PAGE_SIZE;
        if (addr % 4 != 0) {
            throw UnalignedMemoryAccessException(addr, 4);
        }
        if (!this->pages[page_num].is_initialized()) {
            this->init_page(page_num);
        }
        if (addr < IO_SPACE_ADDR) {
            *reinterpret_cast<uint32_t*>(&this->pages[page_num].data[page_offset]) = data;
        } else {
            *reinterpret_cast<volatile uint32_t*>(&this->pages[page_num].data[page_offset]) = data;
        }
    }
}