#include <cstring>
#include "filesystem.hpp"

namespace lc32sim {
    sim_fd Filesystem::sim_open(const char *filename, sim_int flags, sim_int mode) {
        if (logger.info.enabled()) {
            logger.info << "Called sim_open function with filename : " << filename << ", and flags : " << std::hex << flags << ", and mode: " << std::oct << mode;
        }
        if (filename == NULL) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EFAULT);
            return 0;
        }
        
        int flags_convert = convert_flags(flags);
        int mode_convert = convert_mode(mode);
        if (logger.info.enabled()) {
            logger.info << "Converted flags: " << std::hex << flags_convert;
            logger.info << "Converted mode: " << std::oct << mode_convert;

        }

        if (flags_convert == -1) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, ENOSYS);
            return 0;
        }


        int fd = open(filename, flags_convert, mode_convert);
        if (fd == -1) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, errno);
            return 0;
        }


        this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, 0);
        file_table.push_back(File(fd, true));
        return file_table.size() - 1;
    }

    sim_size_t Filesystem::sim_read(sim_fd fd, void *ptr, sim_size_t cnt) {
        if (logger.info.enabled()) {
            logger.info << "sim_read called with fd: " << fd << ", ptr: "  << std::hex << ptr << ", cnt: " << cnt;
        }
        if (fd > file_table.size()) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return 0;
        }

        File &f = file_table[fd];
        if (!f.open) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return 0;
        }

        int num_read = read(f.fd, ptr, cnt);

        // Check for error
        if (num_read == -1) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, errno);
            return 0;
        }

        // mark as success and return num_read
        this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, 0);
        return num_read;
    }

    sim_size_t Filesystem::sim_write(sim_fd fd, const void *ptr, sim_size_t cnt) {
        if (logger.info.enabled()) {
            logger.info << "In sim_write with fd: " << fd << "ptr: " << std::hex << ptr << ", and cnt: " << cnt;
        }
        if (fd > file_table.size()) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return 0;
        }

        File &f = file_table[fd];
        if (!f.open) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return 0;
        }

        int num_write = write(f.fd, ptr, cnt);
        // check for error
        if (num_write == -1) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, errno);
            return 0;
        }

        // mark as success and return num_write
        this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, 0);
        return num_write;
    }

    sim_size_t Filesystem::sim_seek(sim_fd fd, sim_int offset, sim_int whence) {
        // MIGHT NEED TO MAP WHENCE VALUES
        if(logger.info.enabled()) {
            logger.info << "sim_seek called with fd: " << fd << ", offset: " << offset << ", whence: " << whence;
        }

        if (fd > file_table.size()) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return 0;
        }

        File &f = file_table[fd];
        if (!f.open) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return 0;
        }

        off_t old_pos = lseek(f.fd, 0, SEEK_CUR);

        off_t new_pos = lseek(f.fd, offset, whence);

        if (new_pos > INT_MAX || new_pos < INT_MIN) {
            // reverse the seek
            lseek(f.fd, old_pos, SEEK_SET);
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EOVERFLOW);
            return 0;
        }

        if (new_pos == -1) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, errno);
            return 0;
        }

        if(logger.info.enabled()) {
            logger.info << "sim_seek returning: " << new_pos;
        }

        this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, 0);
        return new_pos;  
    }

    sim_int Filesystem::sim_close(sim_fd fd) {
        if(logger.info.enabled()) {
            logger.info << "sim_close called with fd: " << fd;
        }
        
        if (fd > file_table.size()) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return EOF;
        }

        File &f = file_table[fd];
        if (!f.open) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EBADF);
            return EOF;
        }
        if (f.fd != STDIN_FILENO && f.fd != STDOUT_FILENO && f.fd != STDERR_FILENO) {
            close(f.fd);
        }
        f.open = false;
        this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, 0);
        // remove file from file table?

        return 0;
    }

    sim_int Filesystem::sim_stat(const char *filename, struct stat *pstat) {
        if (filename == NULL || pstat == NULL) {
            this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, EFAULT);
            return -1;
        }

        
        return 0;
    }


    int Filesystem::convert_flags(sim_int flags) {
        int flags_convert = 0;
        for (size_t i = 0; i < sizeof(flags) * 8; ++i) {
            int f = flags & (1 << i);
            if (this->flags_map.count(f)) {
                flags_convert |= this->flags_map[f];
            } else {
                return -1;
            }
        }
        return flags_convert;
    }

    int Filesystem::convert_mode(sim_int mode) {
        int mode_convert = 0;
        for (size_t i = 0; i < sizeof(mode) * 8; ++i) {
            int m = mode & (1 << i);
            if (this->mode_map.count(m)) {
                mode_convert |= this->mode_map[m];
            }

        }
        return mode_convert;
    }

    sim_int Filesystem::convert_errno(int sim_errno) {
        return 0;
    }
}