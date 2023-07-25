#pragma once
#include <bit>
#include <cstdint>
#include <memory>
#include <tuple>
#include <unordered_map>

#include "elf_file.hpp"
#include "exceptions.hpp"
#include "filesystem.hpp"
#include "iodevice.hpp"
#include "utils.hpp"

namespace lc32sim {
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "mixed-endian architectures are not supported");

    class Memory {
        private:
            std::unique_ptr<bool[]> page_initialized;
            unsigned int seed;
            std::unique_ptr<uint8_t[]> data;
            void init_page(uint32_t page_num);
            std::unordered_map<uint32_t, read_handler> pre_read_hooks;
            std::unordered_map<uint32_t, write_handler> pre_write_hooks;

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

                if (addr >= Config.memory.io_space_min) {
                    if (this->pre_read_hooks.find(addr) != this->pre_read_hooks.end()) {
                        return this->pre_read_hooks[addr](addr);
                    }
                }

                T ret = *reinterpret_cast<T*>(&this->data[addr]);
                if constexpr(sizeof(T) > 1 && std::endian::native == std::endian::big) {
                    return std::byteswap(ret);
                } else {
                    return ret;
                }
            }
            
            template <typename T, bool unsafe = false>
            void write(uint32_t addr, T val) {
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

                if (addr >= Config.memory.io_space_min) {
                    if (this->pre_write_hooks.find(addr) != this->pre_write_hooks.end()) {
                        this->pre_write_hooks[addr](addr, val);
                        return;
                    }
                }

                if constexpr (sizeof(T) > 1 && std::endian::native == std::endian::big) {
                    val = std::byteswap(val);
                }
                *reinterpret_cast<T*>(&this->data[addr]) = val;
            }

            // Functions to allow I/O devices to "hook" into certain memory addresses, mimicing MMIO
            void add_pre_read_hook(uint32_t addr, read_handler hook);
            void add_pre_write_hook(uint32_t addr, write_handler hook);


            inline uint16_t *get_video_buffer() {
                return reinterpret_cast<uint16_t*>(&this->data[VIDEO_BUFFER_ADDR]);
            }

        friend class DMAController;
    };
}