#pragma once
#include <bit>
#include <cstdint>
#include <memory>
#include <tuple>
#include <unordered_map>

#include "config.hpp"
#include "elf_file.hpp"
#include "exceptions.hpp"
#include "iodevice.hpp"
#include "log.hpp"
#include "utils.hpp"

namespace lc32sim {
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "mixed-endian architectures are not supported");

    class Memory {
        private:
            std::unique_ptr<bool[]> page_initialized;
            unsigned int seed;
            std::unique_ptr<uint8_t[]> data;
            void init_page(uint32_t page_num);
            std::unordered_map<uint32_t, read_handler> read_hooks;
            std::unordered_map<uint32_t, write_handler> write_hooks;

        public:
            Memory(unsigned int seed);
            Memory();
            ~Memory();

            void set_seed(unsigned int seed);
            void load_elf(ELFFile& elf);

            // Unsafe skips checks for segmentation faults, unaligned accesses, and unloaded pages
            // It is the caller's responsibility to check these manually
            template<typename T, bool unsafe = false>
            T read(uint32_t addr) {
                static_assert(sizeof(T) <= 4);
                uint32_t page_num = addr / Config.memory.simulator_page_size;

                if constexpr (!unsafe) {
                    if (addr < Config.memory.user_space_min || addr > Config.memory.user_space_max) {
                        throw SegmentationFaultException(addr);
                    }

                    if constexpr (sizeof(T) > 1) {
                        if (addr % sizeof(T) != 0) {
                            throw UnalignedMemoryAccessException(addr, sizeof(T));
                        }
                    }

                    if (!this->page_initialized[page_num]) {
                        this->init_page(page_num);
                    }
                }

                uint32_t aligned_addr = addr & ~0x3;
                uint32_t offset = addr & 0x3;
                uint32_t ret = *reinterpret_cast<uint32_t*>(&this->data[aligned_addr]);
                if constexpr(sizeof(T) > 1 && std::endian::native == std::endian::big) {
                    ret = std::byteswap(ret);
                }

                if (addr >= Config.memory.io_space_min) {
                    auto hook = this->read_hooks.find(aligned_addr);
                    if (hook != this->read_hooks.end()) {
                        ret = (hook->second)(ret);
                    }
                }

                if (std::endian::native == std::endian::little) {
                    return ret >> (offset * 8);
                } else {
                    return ret << (offset * 8);
                }
            }
            
            template <typename T, bool unsafe = false>
            void write(uint32_t addr, T val) {
                static_assert(sizeof(T) <= 4);
                uint32_t page_num = addr / Config.memory.simulator_page_size;

                if constexpr (!unsafe) {
                    if (addr < Config.memory.user_space_min || addr > Config.memory.user_space_max) {
                        throw SegmentationFaultException(addr);
                    }

                    if constexpr (sizeof(T) > 1) {
                        if (addr % sizeof(T) != 0) {
                            throw UnalignedMemoryAccessException(addr, sizeof(T));
                        }
                    }

                    if (!this->page_initialized[page_num]) {
                        this->init_page(page_num);
                    }
                }

                if constexpr (sizeof(T) > 1 && std::endian::native == std::endian::big) {
                    val = std::byteswap(val);
                }
                if (addr >= Config.memory.io_space_min) {
                    auto hook = this->write_hooks.find(addr);
                    if (hook != this->write_hooks.end()) {
                        uint32_t aligned_addr = addr & ~0x3;

                        // old_data: the data that was at the address before the write
                        // new_data: the data that would be written if the hook didn't exist
                        // final_data: the data that will be actually written
                        // volatile is required to prevent GCC from reordering these accesses
                        uint32_t old_data = *reinterpret_cast<volatile uint32_t*>(&this->data[aligned_addr]);
                        *reinterpret_cast<volatile T*>(&this->data[addr]) = val;
                        uint32_t new_data = *reinterpret_cast<volatile uint32_t*>(&this->data[aligned_addr]);

                        if constexpr (std::endian::native == std::endian::big) {
                            old_data = std::byteswap(old_data);
                            new_data = std::byteswap(new_data);
                        }

                        uint32_t final_data = (hook->second)(old_data, new_data);
                        *reinterpret_cast<uint32_t*>(&this->data[aligned_addr]) = final_data;
                        return;
                    }
                } 
                *reinterpret_cast<T*>(&this->data[addr]) = val;
            }

            // Functions to allow I/O devices to "hook" into certain memory addresses, mimicing MMIO
            void add_read_hook(uint32_t addr, read_handler hook);
            void add_write_hook(uint32_t addr, write_handler hook);

            template<typename T>
            inline T *ptr_to(uint32_t addr) {
                return reinterpret_cast<T*>(&this->data[addr]);
            }


            inline uint16_t *get_video_buffer() {
                return reinterpret_cast<uint16_t*>(&this->data[VIDEO_BUFFER_ADDR]);
            }

        friend class DMAController;
    };
}