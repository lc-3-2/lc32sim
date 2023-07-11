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
        
        this->init_io_devices();
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

    template <typename T> void Memory::write_array(uint32_t addr, const T *arr, uint32_t len) {
        uint32_t num_bytes = sizeof(T) * len;
        if (Config.memory.size < num_bytes) {
            throw SimulatorException("cannot write " + std::to_string(num_bytes) + " bytes to memory: memory size is " + std::to_string(Config.memory.size) + " bytes");
        }
        if ((Config.memory.size - num_bytes) < addr) {
            throw SimulatorException("cannot write " + std::to_string(num_bytes) + " bytes to memory: address " + std::to_string(addr) + " is out of range");
        }

        if constexpr (sizeof(T) > 1) {
            if (addr % sizeof(T) != 0) {
                throw UnalignedMemoryAccessException(addr, sizeof(T));
            }
        }

        uint32_t page_num = addr / Config.memory.simulator_page_size;
        uint32_t end_page = (addr + num_bytes) / Config.memory.simulator_page_size;
        for (uint32_t i = page_num; i <= end_page; i++) {
            if (!this->page_initialized[i]) {
                this->init_page(i);
            }
        }
        if constexpr (endian::native == endian::big) {
            for (uint32_t i = 0; i < len; i++) {
                arr[i] = std::byteswap(arr[i]);
            }
        }
        memcpy(&this->data[addr], arr, num_bytes);
    }

    // Templates for read/write functions needed by other files
    template char Memory::read<char>(uint32_t addr);
    template uint8_t Memory::read<uint8_t>(uint32_t addr);
    template uint16_t Memory::read<uint16_t>(uint32_t addr);
    template uint32_t Memory::read<uint32_t>(uint32_t addr);
    template void Memory::write<uint8_t>(uint32_t addr, uint8_t val);
    template void Memory::write<uint16_t>(uint32_t addr, uint16_t val);
    template void Memory::write<uint32_t>(uint32_t addr, uint32_t val);
    template void Memory::write_array<uint8_t>(uint32_t addr, const uint8_t *arr, uint32_t len);

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

    // Code for handling I/O devices
    void Memory::init_io_devices() {
        this->write<uint16_t>(REG_VCOUNT_ADDR, 0);
        this->write<uint16_t>(REG_KEYINPUT_ADDR, 0xFFFF);
        this->write<dma::DMAController>(DMA_CONTROLLER_ADDR, dma::DMAController());
        for (uint32_t pixel = 0; pixel < (Config.display.width * Config.display.height); pixel++) {
            this->write<uint16_t>(VIDEO_BUFFER_ADDR + (pixel * 2), 0);
        }
    }

    void Memory::set_vcount(uint16_t scanline) {
        this->write<uint16_t>(REG_VCOUNT_ADDR, scanline);
    }

    uint16_t *Memory::get_reg_keyinput() {
        return reinterpret_cast<uint16_t*>(&this->data[REG_KEYINPUT_ADDR]);
    }

    void Memory::handle_dma() {
        using namespace lc32sim::dma;
        DMAController dma = this->read<DMAController>(DMA_CONTROLLER_ADDR);
        uint32_t &src = dma.source;
        uint32_t &dst = dma.destination;
        uint32_t &cnt = dma.control;

        if (cnt & DMA_ON) {
            if ((cnt & DMA_TIMING) != DMA_NOW) {
                throw SimulatorException("DMA timing besides DMA_NOW not implemented");
            }
            if (cnt & DMA_IRQ) {
                throw SimulatorException("DMA_IRQ not implemented");
            }
            if ((cnt & DMA_DESTINATION) == DMA_DESTINATION_RESET) {
                // Since DMA_TIMING_(H|V)BLANK are not implemented,
                // these are equivalent
                cnt &= ~DMA_DESTINATION;
                cnt |= DMA_DESTINATION_INCREMENT;
            }

            uint32_t num_transfers = cnt & DMA_NUM_TRANSFERS;
            uint32_t transfer_size = ((cnt & DMA_WIDTH) == DMA_32 ? 4 : 2);
            uint32_t total_size = num_transfers * transfer_size;

            int source_increment = 0;
            int destination_increment = 0;

            // Ensure all source pages are initialized
            uint32_t start_page = src / Config.memory.simulator_page_size;
            if ((cnt & DMA_SOURCE) == DMA_SOURCE_INCREMENT) {
                if ((Config.memory.size - src) < total_size) {
                    throw SimulatorException("DMA_SOURCE_INCREMENT hits end of memory");
                }
                uint32_t end_page = (src + total_size) / Config.memory.simulator_page_size;
                for (uint32_t page = start_page; page <= end_page; page++) {
                    if (!this->page_initialized[page]) {
                        this->init_page(page);
                    }
                }
                source_increment = transfer_size;
            } else if ((cnt & DMA_SOURCE) == DMA_SOURCE_DECREMENT) {
                if (src < total_size) {
                    throw SimulatorException("DMA_SOURCE_DECREMENT hits start of memory");
                }
                uint32_t end_page = (src - total_size) / Config.memory.simulator_page_size;
                for (uint32_t page = start_page; page >= end_page; page--) {
                    if (!this->page_initialized[page]) {
                        this->init_page(page);
                    }
                }
                source_increment = -transfer_size;
            } else if ((cnt & DMA_SOURCE) == DMA_SOURCE_FIXED) {
                if (!this->page_initialized[start_page]) {
                    this->init_page(start_page);
                }   
            } else {
                throw SimulatorException("DMA_SOURCE invalid");
            }

            // Ensure all destination pages are initialized
            start_page = dst / Config.memory.simulator_page_size;
            if ((cnt & DMA_DESTINATION) == DMA_DESTINATION_INCREMENT) {
                if ((Config.memory.size - dst) < total_size) {
                    throw SimulatorException("DMA_DESTINATION_INCREMENT hits end of memory");
                }
                uint32_t end_page = (dst + total_size) / Config.memory.simulator_page_size;
                for (uint32_t page = start_page; page <= end_page; page++) {
                    if (!this->page_initialized[page]) {
                        this->init_page(page);
                    }
                }
                destination_increment = transfer_size;
            } else if ((cnt & DMA_DESTINATION) == DMA_DESTINATION_DECREMENT) {
                if (dst < total_size) {
                    throw SimulatorException("DMA_DESTINATION_DECREMENT hits start of memory");
                }
                uint32_t end_page = (dst - total_size) / Config.memory.simulator_page_size;
                for (uint32_t page = start_page; page >= end_page; page--) {
                    if (!this->page_initialized[page]) {
                        this->init_page(page);
                    }
                }
                destination_increment = -transfer_size;
            } else if ((cnt & DMA_DESTINATION) == DMA_DESTINATION_FIXED) {
                if (!this->page_initialized[start_page]) {
                    this->init_page(start_page);
                }   
            } else {
                throw SimulatorException("DMA_DESTINATION invalid");
            }

            if ((cnt & DMA_WIDTH) == DMA_16) {
                for (uint32_t i = 0; i < num_transfers; i++) {
                    uint16_t &data = *reinterpret_cast<uint16_t*>(this->data[src]);
                    *reinterpret_cast<uint16_t*>(this->data[dst]) = data;

                    src += source_increment;
                    dst += destination_increment;
                }
            } else if ((cnt & DMA_WIDTH) == DMA_32) {
                for (uint32_t i = 0; i < num_transfers; i++) {
                    uint32_t &data = *reinterpret_cast<uint32_t*>(this->data[src]);
                    *reinterpret_cast<uint32_t*>(this->data[dst]) = data;

                    src += source_increment;
                    dst += destination_increment;
                }
            } else {
                throw SimulatorException("DMA_WIDTH invalid");
            }
        }
    }

    uint16_t *Memory::get_video_buffer() {
        return reinterpret_cast<uint16_t*>(&this->data[VIDEO_BUFFER_ADDR]);
    }
}