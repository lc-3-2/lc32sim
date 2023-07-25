#pragma once

#include "config.hpp"
#include "exceptions.hpp"
#include "iodevice.hpp"
#include "memory.hpp"

namespace lc32sim {
    class DMAController : public IODevice {
        private:
            Memory &mem;
            bool dma_on = false;

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

            forceinline void handle_dma(uint32_t source, uint32_t dest, uint32_t control) {
                if ((control & DMA_TIMING) != DMA_NOW) {
                    throw SimulatorException("DMA timing besides DMA_NOW not implemented");
                }
                if (control & DMA_IRQ) {
                    throw SimulatorException("DMA_IRQ not implemented");
                }
                if ((control & DMA_DESTINATION) == DMA_DESTINATION_RESET) {
                    // Since DMA_TIMING_(H|V)BLANK are not implemented,
                    // these are equivalent
                    control &= ~DMA_DESTINATION;
                    control |= DMA_DESTINATION_INCREMENT;
                }

                uint32_t num_transfers = control & DMA_NUM_TRANSFERS;
                uint32_t transfer_size = ((control & DMA_WIDTH) == DMA_32 ? 4 : 2);
                uint32_t total_size = num_transfers * transfer_size;

                int source_increment = 0;
                int destination_increment = 0;

                // Ensure all source pages are initialized
                uint32_t start_page = source / Config.memory.simulator_page_size;
                if ((control & DMA_SOURCE) == DMA_SOURCE_INCREMENT) {
                    if ((Config.memory.size - source) < total_size) {
                        throw SimulatorException("DMA_SOURCE_INCREMENT hits end of memory");
                    }
                    uint32_t end_page = (source + total_size) / Config.memory.simulator_page_size;
                    for (uint32_t page = start_page; page <= end_page; page++) {
                        if (!this->mem.page_initialized[page]) {
                            this->mem.init_page(page);
                        }
                    }
                    source_increment = transfer_size;
                } else if ((control & DMA_SOURCE) == DMA_SOURCE_DECREMENT) {
                    if (source < total_size) {
                        throw SimulatorException("DMA_SOURCE_DECREMENT hits start of memory");
                    }
                    uint32_t end_page = (source - total_size) / Config.memory.simulator_page_size;
                    for (uint32_t page = start_page; page >= end_page; page--) {
                        if (!this->mem.page_initialized[page]) {
                            this->mem.init_page(page);
                        }
                    }
                    source_increment = -transfer_size;
                } else if ((control & DMA_SOURCE) == DMA_SOURCE_FIXED) {
                    if (!this->mem.page_initialized[start_page]) {
                        this->mem.init_page(start_page);
                    }   
                } else {
                    throw SimulatorException("DMA_SOURCE invalid");
                }

                // Ensure all destination pages are initialized
                start_page = dest / Config.memory.simulator_page_size;
                if ((control & DMA_DESTINATION) == DMA_DESTINATION_INCREMENT) {
                    if ((Config.memory.size - dest) < total_size) {
                        throw SimulatorException("DMA_DESTINATION_INCREMENT hits end of memory");
                    }
                    uint32_t end_page = (dest + total_size) / Config.memory.simulator_page_size;
                    for (uint32_t page = start_page; page <= end_page; page++) {
                        if (!this->mem.page_initialized[page]) {
                            this->mem.init_page(page);
                        }
                    }
                    destination_increment = transfer_size;
                } else if ((control & DMA_DESTINATION) == DMA_DESTINATION_DECREMENT) {
                    if (dest < total_size) {
                        throw SimulatorException("DMA_DESTINATION_DECREMENT hits start of memory");
                    }
                    uint32_t end_page = (dest - total_size) / Config.memory.simulator_page_size;
                    for (uint32_t page = start_page; page >= end_page; page--) {
                        if (!this->mem.page_initialized[page]) {
                            this->mem.init_page(page);
                        }
                    }
                    destination_increment = -transfer_size;
                } else if ((control & DMA_DESTINATION) == DMA_DESTINATION_FIXED) {
                    if (!this->mem.page_initialized[start_page]) {
                        this->mem.init_page(start_page);
                    }   
                } else {
                    throw SimulatorException("DMA_DESTINATION invalid");
                }

                if ((control & DMA_WIDTH) == DMA_16) {
                    for (uint32_t i = 0; i < num_transfers; i++) {
                        uint16_t tmp = this->mem.read<uint16_t, true>(source);
                        this->mem.write<uint16_t, true>(dest, tmp);

                        source += source_increment;
                        dest += destination_increment;
                    }
                } else if ((control & DMA_WIDTH) == DMA_32) {
                    for (uint32_t i = 0; i < num_transfers; i++) {
                        uint32_t tmp = this->mem.read<uint32_t, true>(source);
                        this->mem.write<uint32_t, true>(dest, tmp);

                        source += source_increment;
                        dest += destination_increment;
                    }
                } else {
                    throw SimulatorException("DMA_WIDTH invalid");
                }
            }

        public:
            DMAController(Memory &mem) : mem(mem) {}
            
            std::string get_name() override {
                return "DMA Controller";
            }

            write_handlers get_write_handlers() override {
                return {
                    {DMA_CONTROLLER_ADDR + 8, [this](uint32_t old_value, uint32_t value) {
                        if ((value & DMA_ON) && !dma_on) {
                            dma_on = true;
                            uint32_t source = this->mem.read<uint32_t, true>(DMA_CONTROLLER_ADDR);
                            uint32_t dest = this->mem.read<uint32_t, true>(DMA_CONTROLLER_ADDR + 4);

                            handle_dma(source, dest, value);

                            dma_on = false;
                        }
                        return 0;
                    }}
                };
            }
    };
}