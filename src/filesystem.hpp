#pragma once

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "iodevice.hpp"
#include "memory.hpp"
#include "utils.hpp"
#include "lc32_flags.hpp"
#include "log.hpp"

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
                int fd = 0;
                bool open = false;
                File(int fd, bool open) : fd(fd), open(open) {}
            };

            std::vector<File> file_table;
            Memory &mem;

            std::unordered_map<int, int> flags_map;
            std::unordered_map<int, int> mode_map;
            std::unordered_map<int, int> errno_map;

            static const uint16_t MODE_OFF = 0;
            static const uint16_t MODE_OPEN = 1;
            static const uint16_t MODE_CLOSE = 2;
            static const uint16_t MODE_READ = 3;
            static const uint16_t MODE_WRITE = 4;
            static const uint16_t MODE_SEEK = 5;
            static const uint16_t MODE_STAT = 6;
            static const uint16_t MODE_FSTAT = 7;
            static const uint16_t MODE_ISATTY = 8;
            static const uint16_t MODE_LINK = 9;
            static const uint16_t MODE_UNLINK = 10;
            static const uint16_t MODE_MKDIR = 11;


        public:
            Filesystem(Memory &mem) : file_table(), mem(mem), flags_map(), mode_map(), errno_map() {
                flags_map[LC32_O_RDONLY] = O_RDONLY;
                flags_map[LC32_O_WRONLY] = O_WRONLY;
                flags_map[LC32_O_RDWR] = O_RDWR;
                flags_map[LC32_O_APPEND] = O_APPEND;
                flags_map[LC32_O_CREAT] = O_CREAT;
                flags_map[LC32_O_TRUNC] = O_TRUNC;

                mode_map[LC32_S_IXOTH] = S_IXOTH;
                mode_map[LC32_S_IWOTH] = S_IWOTH;
                mode_map[LC32_S_IROTH] = S_IROTH;
                mode_map[LC32_S_IXGRP] = S_IXGRP;
                mode_map[LC32_S_IWGRP] = S_IWGRP;
                mode_map[LC32_S_IRGRP] = S_IRGRP;
                mode_map[LC32_S_IXUSR] = S_IXUSR;
                mode_map[LC32_S_IWUSR] = S_IWUSR;
                mode_map[LC32_S_IRUSR] = S_IRUSR;

                // Add stdout, stderr, and stdin to table
                file_table.insert(file_table.begin() + LC32_STDIN_FILENO, File(STDIN_FILENO, true));
                file_table.insert(file_table.begin() + LC32_STDOUT_FILENO, File(STDOUT_FILENO, true));
                file_table.insert(file_table.begin() + LC32_STDERR_FILENO, File(STDERR_FILENO, true));
            }

            sim_fd sim_open(const char *filename, int flags, int mode);
            sim_size_t sim_read(sim_fd fd, void *ptr, sim_size_t cnt);
            sim_size_t sim_write(sim_fd fd, const void *ptr, sim_size_t cnt);
            sim_size_t sim_seek(sim_fd fd, sim_int offset, sim_int whence);
            sim_int sim_close(sim_fd fd);
            sim_int sim_stat(const char *filename, struct stat *pstat);
            int convert_flags(int flags);
            int convert_mode(int mode);
            sim_int convert_errno(int sim_errno);

            std::string get_name() override { return "Filesystem"; };
            write_handlers get_write_handlers() override {
                return {
                    { FS_CONTROLLER_ADDR, [this](uint32_t old_value, uint32_t value) {
                        static_assert(sizeof(sim_fd) == sizeof(uint16_t));
                        uint16_t mode = first16(value);
                        sim_fd fd = second16(value);
                        if (logger.info.enabled()) {
                            logger.info << "Filesystem peripheral handler set off with mode: " << mode;
                        }

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
                                ret = this->sim_open(this->mem.ptr_to<const char>(data1), data2, data3);
                                return from16(MODE_OFF, ret);
                            case MODE_CLOSE:
                                ret = this->sim_close(fd);
                                goto write_ret;
                            case MODE_READ:
                                ret = this->sim_read(fd, this->mem.ptr_to<void>(data1), data2);
                                goto write_ret;
                            case MODE_WRITE:
                                ret = this->sim_write(fd, this->mem.ptr_to<const void>(data1), data2);
                                goto write_ret;
                            case MODE_SEEK:
                                data12 = ((static_cast<uint64_t>(data2) << 32) | data1);
                                ret = this->sim_seek(fd, data1, data2);
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