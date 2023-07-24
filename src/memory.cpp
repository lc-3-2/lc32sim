#include <cassert>
#include <cstdlib>
#include <ctime>
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
        page_initialized = std::make_unique<bool[]>(NUM_PAGES);
        
        this->init_io_devices();
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

    template<typename T, bool internal>
    T Memory::read(uint32_t addr) {
        uint32_t page_num = addr / Config.memory.simulator_page_size;
        if constexpr (!internal && (sizeof(T) > 1)) {
            if (addr % sizeof(T) != 0) {
                throw UnalignedMemoryAccessException(addr, sizeof(T));
            }
        }
        if (!this->page_initialized[page_num]) {
            this->init_page(page_num);
        }

        if constexpr (!internal) {
            if (addr >= IO_SPACE_START) {
                using currtime_t = uint32_t;
                if (REG_CURRTIME_ADDR <= addr && addr < REG_CURRTIME_ADDR + sizeof(currtime_t)) {
                    this->write<currtime_t>(REG_CURRTIME_ADDR, static_cast<currtime_t>(std::time(0)));
                }
            }
        }

        T ret = *reinterpret_cast<T*>(&this->data[addr]);
        if constexpr(sizeof(T) > 1 && endian::native == endian::big) {
            return std::byteswap(ret);
        } else {
            return ret;
        }
    }
    
    template <typename T, bool internal>
    void Memory::write(uint32_t addr, T val) {
        uint32_t page_num = addr / Config.memory.simulator_page_size;
        if constexpr (!internal && (sizeof(T) > 1)) {
            if (addr % sizeof(T) != 0) {
                throw UnalignedMemoryAccessException(addr, sizeof(T));
            }
        }
        if (!this->page_initialized[page_num]) {
            this->init_page(page_num);
        }

        if constexpr (endian::native == endian::big) {
            val = std::byteswap(val);
        }
        *reinterpret_cast<T*>(&this->data[addr]) = val;

        if constexpr (!internal) {
            if (addr >= IO_SPACE_START) {
                if (DMA_CONTROLLER_ADDR <= addr && addr < DMA_CONTROLLER_ADDR + sizeof(dma::DMAController)) {
                    this->handle_dma();
                }
                if (FS_CONTROLLER_ADDR <= addr && addr < FS_CONTROLLER_ADDR + sizeof(fs::FSController)) {
                    this->handle_fs();
                }
            }
        }
    }

    template <typename T, bool internal>
    void Memory::write_array(uint32_t addr, const T *arr, uint32_t len) {
        uint32_t num_bytes = sizeof(T) * len;
        if (Config.memory.size < num_bytes) {
            throw SimulatorException("cannot write " + std::to_string(num_bytes) + " bytes to memory: memory size is " + std::to_string(Config.memory.size) + " bytes");
        }
        if ((Config.memory.size - num_bytes) < addr) {
            throw SimulatorException("cannot write " + std::to_string(num_bytes) + " bytes to memory: address " + std::to_string(addr) + " is out of range");
        }

        if constexpr (!internal && (sizeof(T) > 1)) {
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
        this->write<uint16_t, true>(REG_VCOUNT_ADDR, 0);
        this->write<uint16_t, true>(REG_KEYINPUT_ADDR, 0xFFFF);
        this->write<dma::DMAController, true>(DMA_CONTROLLER_ADDR, dma::DMAController());
        this->write<fs::FSController, true>(FS_CONTROLLER_ADDR, fs::FSController());
        for (uint32_t pixel = 0; pixel < (Config.display.width * Config.display.height); pixel++) {
            this->write<uint16_t, true>(VIDEO_BUFFER_ADDR + (pixel * 2), 0);
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
        DMAController dma = this->read<DMAController, true>(DMA_CONTROLLER_ADDR);
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
                    uint16_t &tmp = *reinterpret_cast<uint16_t*>(&this->data[src]);
                    *reinterpret_cast<uint16_t*>(&this->data[dst]) = tmp;

                    src += source_increment;
                    dst += destination_increment;
                }
            } else if ((cnt & DMA_WIDTH) == DMA_32) {
                for (uint32_t i = 0; i < num_transfers; i++) {
                    uint32_t &tmp = *reinterpret_cast<uint32_t*>(&this->data[src]);
                    *reinterpret_cast<uint32_t*>(&this->data[dst]) = tmp;

                    src += source_increment;
                    dst += destination_increment;
                }
            } else {
                throw SimulatorException("DMA_WIDTH invalid");
            }

            this->write<DMAController, true>(DMA_CONTROLLER_ADDR, DMAController());
        }
    }

    void Memory::handle_fs() {
        using namespace lc32sim::fs;
        FSController con = this->read<FSController, true>(FS_CONTROLLER_ADDR);
        if (con.mode != FS_OFF) {
            const char *filename = reinterpret_cast<const char*>(&this->data[con.data1]);
            const char *mode = reinterpret_cast<const char*>(&this->data[con.data2]);
            void *ptr = reinterpret_cast<void*>(&this->data[con.data1]);
            uint64_t offset = con.data2 | (static_cast<uint64_t>(con.data1) << 32);

            switch (con.mode) {
                case FS_OPEN:
                    con.fd = fs.open(filename, mode);
                    break;
                case FS_CLOSE:
                    con.data3 = fs.close(con.fd);
                    break;
                case FS_READ:
                    con.data3 = fs.read(con.fd, ptr, con.data2, con.data3);
                    break;
                case FS_WRITE:
                    con.data3 = fs.write(con.fd, ptr, con.data2, con.data3);
                    break;
                case FS_SEEK:
                    con.data3 = fs.seek(con.fd, offset, con.data3);
                    break;
            }

            con.mode = FS_OFF;
            this->write<FSController, true>(FS_CONTROLLER_ADDR, con);
        }

    }

    uint16_t *Memory::get_video_buffer() {
        return reinterpret_cast<uint16_t*>(&this->data[VIDEO_BUFFER_ADDR]);
    }
}