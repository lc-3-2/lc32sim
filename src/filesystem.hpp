#pragma once

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "iodevice.hpp"
#include "memory.hpp"
#include "utils.hpp"

namespace lc32sim { 
    using sim_fd = uint16_t;
    using sim_size_t = uint32_t;
    using sim_long = int64_t;
    using sim_int = int32_t;

    // this guarantee is needed for safe conversions
    static_assert(sizeof(sim_size_t) <= sizeof(size_t));

    class Filesystem : public IODevice {
        private:
            struct File {
                FILE *f = nullptr;
                bool open = false;
                File(FILE *f, bool open) : f(f), open(open) {}
            };

            std::vector<File> file_table;
            Memory &mem;

            static const uint16_t MODE_OFF = 0;
            static const uint16_t MODE_OPEN = 1;
            static const uint16_t MODE_CLOSE = 2;
            static const uint16_t MODE_READ = 3;
            static const uint16_t MODE_WRITE = 4;
            static const uint16_t MODE_SEEK = 5;

        public:
            Filesystem(Memory &mem) : file_table(), mem(mem) {}

            sim_fd open(const char *filename, const char* mode);
            sim_size_t read(sim_fd fd, void *ptr, sim_size_t size, sim_size_t nmemb);
            sim_size_t write(sim_fd fd, const void *ptr, sim_size_t size, sim_size_t nmemb);
            sim_size_t seek(sim_fd fd, sim_long offset, uint32_t whence);
            sim_int close(sim_fd fd);

            std::string get_name() override { return "Filesystem"; };
            write_handlers get_write_handlers() override {
                return {
                    { FS_CONTROLLER_ADDR, [this](uint32_t old_value, uint32_t value) {
                        static_assert(sizeof(sim_fd) == sizeof(uint16_t));
                        uint16_t mode = first16(value);
                        sim_fd fd = second16(value);

                        if (mode == MODE_OFF) {
                            return value;
                        }

                        uint32_t data1 = this->mem.read<uint32_t, true>(FS_CONTROLLER_ADDR + 4);
                        uint32_t data2 = this->mem.read<uint32_t, true>(FS_CONTROLLER_ADDR + 8);
                        uint32_t data3 = this->mem.read<uint32_t, true>(FS_CONTROLLER_ADDR + 12);
                        uint64_t data12;

                        uint32_t ret;
                        switch (mode) {
                            case MODE_OPEN:
                                ret = this->open(this->mem.ptr_to<const char>(data1), this->mem.ptr_to<const char>(data2));
                                return from16(MODE_OFF, ret);
                            case MODE_CLOSE:
                                ret = this->close(fd);
                                goto write_ret;
                            case MODE_READ:
                                ret = this->read(fd, this->mem.ptr_to<void>(data1), data2, data3);
                                goto write_ret;
                            case MODE_WRITE:
                                ret = this->write(fd, this->mem.ptr_to<const void>(data1), data2, data3);
                                goto write_ret;
                            case MODE_SEEK:
                                data12 = ((static_cast<uint64_t>(data2) << 32) | data1);
                                ret = this->seek(fd, data12, data3);
                                goto write_ret;
                            write_ret:
                                this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 12, ret);
                                break;
                        }

                        return from16(MODE_OFF, second16(value));
                    }
                }};
            }
    };
}