#include <cstring>
#include "filesystem.hpp"

// SANDBOX PATHS

namespace lc32sim {
    sim_fd Filesystem::sim_open(const char *filename, sim_int flags, sim_int mode, sim_int *error) {
        if (logger.info.enabled()) {
            logger.info << "Called sim_open function with filename : " << filename << ", and flags : " << std::hex << flags << ", and mode: " << std::oct << mode;
        }
        if (filename == NULL) {
            *error = LC32_EFAULT;
            return 0;
        }
        
        int flags_convert = convert_flags(flags);
        if (logger.info.enabled()) {
            logger.info << "Converted flags: " << std::hex << flags_convert;

        }

        if (flags_convert == -1) {
            *error = LC32_ENOSYS;
            return 0;
        }


        int fd = open(filename, flags_convert, mode);
        if (fd == -1) {
            *error = convert_errno(errno);
            return 0;
        }

        file_table.push_back(File(fd, true));

        return file_table.size() - 1;
    }

    sim_size_t Filesystem::sim_read(sim_fd fd, void *ptr, sim_size_t cnt, sim_int *error) {
        if (logger.info.enabled()) {
            logger.info << "sim_read called with fd: " << fd << ", ptr: "  << std::hex << ptr << ", cnt: " << cnt;
        }
        if (fd >= file_table.size()) {
            *error = LC32_EBADF;
            return 0;
        }

        File &f = file_table[fd];
        if (!f.open) {
            *error = LC32_EBADF;
            return 0;
        }

        int num_read = read(f.fd, ptr, cnt);

        // Check for error
        if (num_read == -1) {
            *error = convert_errno(errno);
            return 0;
        }

        return num_read;
    }

    sim_size_t Filesystem::sim_write(sim_fd fd, const void *ptr, sim_size_t cnt, sim_int *error) {
        if (logger.info.enabled()) {
            logger.info << "In sim_write with fd: " << fd << "ptr: " << std::hex << ptr << ", and cnt: " << cnt;
        }
        if (fd >= file_table.size()) {
            *error = LC32_EBADF;
            return 0;
        }

        File &f = file_table[fd];
        if (!f.open) {
            *error = LC32_EBADF;
            return 0;
        }
        
        // I have some worries about ptr causing a segfault
        int num_write = write(f.fd, ptr, cnt);
        // check for error
        if (num_write == -1) {
            *error = convert_errno(errno);
            return 0;
        }

        return num_write;
    }

    sim_size_t Filesystem::sim_seek(sim_fd fd, sim_int offset, sim_int whence, sim_int *error) {
        if(logger.info.enabled()) {
            logger.info << "sim_seek called with fd: " << fd << ", offset: " << offset << ", whence: " << whence;
        }

        if (fd >= file_table.size()) {
            *error = LC32_EBADF;
            return 0;
        }

        int whence_convert = convert_whence(whence);
        if (whence_convert == -1) {
            *error = LC32_EINVAL;
            return 0;
        }

        File &f = file_table[fd];
        if (!f.open) {
            *error = LC32_EBADF;
            return 0;
        }

        off_t old_pos = lseek(f.fd, 0, SEEK_CUR);

        if (old_pos == -1) {
            *error = convert_errno(errno);
            return 0;
        }

        off_t new_pos = lseek(f.fd, offset, whence_convert);

        if (new_pos == -1) {
            *error = convert_errno(errno);
            return 0;
        }

        if (will_overflow(32, new_pos)) {
            // new_pos cannot be represented in 32-bit, reverse the seek
            lseek(f.fd, old_pos, SEEK_SET);
            *error = LC32_EOVERFLOW;
            return 0;
        }

        if(logger.info.enabled()) {
            logger.info << "sim_seek returning: " << new_pos;
        }

        return new_pos;  
    }

    sim_int Filesystem::sim_close(sim_fd fd, sim_int *error) {
        if(logger.info.enabled()) {
            logger.info << "sim_close called with fd: " << fd;
        }
        
        if (fd >= file_table.size()) {
            *error = LC32_EBADF;
            return EOF;
        }

        File &f = file_table[fd];
        if (!f.open) {
            *error = LC32_EBADF;
            return EOF;
        }

        // do not want to actually close stdin, stdout, or stderr
        if (f.fd != STDIN_FILENO && f.fd != STDOUT_FILENO && f.fd != STDERR_FILENO) {
            close(f.fd);
        }

        f.open = false;

        return 0;
    }

    sim_int Filesystem::sim_fstat(sim_fd fd, sim_int pstat, sim_int *error) {
        if(logger.info.enabled()) {
            logger.info << "sim_fstat called with fd: " << fd << ", and pstat: " << std::hex << pstat;
        }

        if (fd >= file_table.size()) {
            *error = LC32_EFAULT;
            return -1;
        }

        File &f = file_table[fd];

        if (!f.open) {
            *error = LC32_EBADF;
            return -1;
        }

        struct stat stat_struct;
        int success = fstat(f.fd, &stat_struct);

        if (success == -1) {
            *error = convert_errno(errno);
            return -1;
        }

        // printf("stat->st_dev): %lu\n", stat_struct.st_dev);
        // printf("stat->st_ino): %lu\n", stat_struct.st_ino);
        // printf("stat->st_mode): %u\n", (stat_struct.st_mode));
        // printf("stat->st_nlink): %lu\n", (stat_struct.st_nlink));
        // printf("stat->st_uid): %u\n", (stat_struct.st_uid));
        // printf("stat->st_gid): %u\n", (stat_struct.st_gid));
        // printf("stat->st_rdev): %lu\n", (stat_struct.st_rdev));
        // printf("stat->st_size): %lu\n", (stat_struct.st_size));
        // printf("stat->st_atim): %lu\n", (stat_struct.st_atim.tv_sec));
        // printf("stat->st_mtim): %lu\n", (stat_struct.st_mtim.tv_sec));
        // printf("stat->st_ctim): %lu\n", (stat_struct.st_ctim.tv_sec));
        // printf("stat->st_blksize): %lu\n", (stat_struct.st_blksize));
        // printf("stat->st_blocks): %lu\n", (stat_struct.st_blocks));

        // copy over stat info into lc32 memory
        bool copy_success = copy_stat(pstat, stat_struct);

        if (!copy_success) {
            if (logger.warn.enabled()) {
                logger.warn << "copy on stat failed";
            }
            *error = LC32_EOVERFLOW;
            return -1;
        }
        
        this->mem.write<uint32_t, true>(FS_CONTROLLER_ADDR + 8, 0);
        return 0;
    }

    sim_int Filesystem::sim_isatty(sim_fd fd, sim_int *error) {
        if (logger.info.enabled()) {
            logger.info << "sim_isatty called with fd: " << fd;
        }

        if (fd >= file_table.size() ) {
            *error = LC32_EBADF;
            return 0;
        }

        File &f = file_table[fd];
        if (!f.open) {
            *error = LC32_EBADF;
            return 0;
        }

        int is_terminal = isatty(f.fd);

        if (!is_terminal) {
            *error = convert_errno(errno);
            return 0;
        }

        return 1;
    }

    sim_int Filesystem::sim_link(const char *oldpath, const char *newpath, sim_int *error) {
        if (logger.info.enabled()) {
            logger.info << "sim_link called with oldpath: " << oldpath << ", and newpath: " << newpath;
        }
        if (oldpath == NULL || newpath == NULL) {
            *error = LC32_EFAULT;
            return -1;
        }

        int res = link(oldpath, newpath);

        if (res == -1) {
            *error = convert_errno(errno);
            return -1;
        }

        return 0;
    }

    sim_int Filesystem::sim_unlink(const char *pathname, sim_int *error) {
        if (logger.info.enabled()) {
            logger.info << "sim_unlink called with pathname: " << pathname;
        }

        if (pathname == NULL) {
            *error = LC32_EFAULT;
            return -1;
        }

        int res = unlink(pathname);

        if (res == -1) {
            *error = convert_errno(errno);
            return -1;
        }

        return 0;
    }

    sim_int Filesystem::sim_mkdir(const char *pathname, sim_int mode, sim_int *error) {
        if (logger.info.enabled()) {
            logger.info << "sim_mkdir called with pathname: " << pathname << ", and mode: " << std::oct << mode;
        }
        if (pathname == NULL) {
            *error = LC32_EFAULT;
            return -1;
        }

        int res = mkdir(pathname, mode);

        if (res == -1) {
            *error = convert_errno(errno);
            return -1;
        }

        return 0;
    }

    bool Filesystem::copy_stat(sim_int plc32_stat, struct stat local_stat) {
        /// Check to see if values will fit in lc32 counterparts
        if (will_overflow(16, local_stat.st_dev) ||
            will_overflow(32, local_stat.st_mode) ||
            will_overflow(16, local_stat.st_nlink) ||
            will_overflow(16, local_stat.st_uid) ||
            will_overflow(16, local_stat.st_gid) ||
            will_overflow(16, local_stat.st_rdev) ||
            will_overflow(32, local_stat.st_size) ||
            will_overflow(32, local_stat.st_blksize) ||
            will_overflow(32, local_stat.st_blocks)) {
                return false;
        }

        // copy over values to lc32
        this->mem.write<uint16_t, true>(plc32_stat, static_cast<uint16_t>(local_stat.st_dev)); 
        // st_ino is likely to overflow, if it does, just ignore it
        if (will_overflow(16, local_stat.st_ino)) {
            this->mem.write<uint16_t, true>(plc32_stat += 2, 0); 
        } else {
            this->mem.write<uint16_t, true>(plc32_stat += 2, static_cast<uint16_t>(local_stat.st_ino));
        }
        this->mem.write<uint32_t, true>(plc32_stat += 2, static_cast<uint32_t>(local_stat.st_mode)); // This is POSIX defined so no need to convert
        this->mem.write<uint16_t, true>(plc32_stat += 4, static_cast<uint16_t>(local_stat.st_nlink));
        this->mem.write<uint16_t, true>(plc32_stat += 2, local_stat.st_uid);
        this->mem.write<uint16_t, true>(plc32_stat += 2, local_stat.st_gid);
        this->mem.write<uint16_t, true>(plc32_stat += 2, local_stat.st_rdev);
        this->mem.write<uint32_t, true>(plc32_stat += 2, local_stat.st_size);
        this->mem.write<uint32_t, true>(plc32_stat += 4, (local_stat.st_atim.tv_sec) & ((1l << 32) - 1)); // will not overflow, 64-bit on both systems
        this->mem.write<uint32_t, true>(plc32_stat += 4, (local_stat.st_atim.tv_sec >> 32) & ((1l << 32) - 1));
        this->mem.write<uint32_t, true>(plc32_stat += 4, local_stat.st_atim.tv_nsec); // guaranteed by POSIX to be <= 1,000,000,000, no overflow
        this->mem.write<uint32_t, true>(plc32_stat += 4, (local_stat.st_mtim.tv_sec) & ((1l << 32) - 1)); // will not overflow, 64-bit on both systems
        this->mem.write<uint32_t, true>(plc32_stat += 4, (local_stat.st_mtim.tv_sec >> 32) & ((1l << 32) - 1)); 
        this->mem.write<uint32_t, true>(plc32_stat += 4, local_stat.st_mtim.tv_nsec); // guaranteed by POSIX to be <= 1,000,000,000, no overflow
        this->mem.write<uint32_t, true>(plc32_stat += 4, (local_stat.st_ctim.tv_sec) & ((1l << 32) - 1)); // will not overflow, 64-bit on both systems
        this->mem.write<uint32_t, true>(plc32_stat += 4, (local_stat.st_ctim.tv_sec >> 32) & ((1l << 32) - 1)); 
        this->mem.write<uint32_t, true>(plc32_stat += 4, local_stat.st_ctim.tv_nsec); // guaranteed by POSIX to be <= 1,000,000,000, no overflow
        this->mem.write<uint32_t, true>(plc32_stat += 4, local_stat.st_blksize);
        this->mem.write<uint32_t, true>(plc32_stat += 4, local_stat.st_blocks);

        return true;
    }

    int Filesystem::convert_flags(sim_int flags) {
        int flags_convert = 0;
        for (size_t i = 0; i < sizeof(flags) * 8; ++i) {
            int f = flags & (1 << i);
            if (this->flags_map.count(f)) {
                flags_convert |= this->flags_map[f];
            } else if (f != 0) {
                // we have a flag on that we do not expect
                return -1;
            }
        }
        return flags_convert;
    }

    int Filesystem::convert_whence(sim_int whence) {
        if (this->whence_map.count(whence)) {
            return this->whence_map[whence];
        }

        return -1;
    }

    sim_int Filesystem::convert_errno(int sim_errno) {
        if (this->errno_map.count(sim_errno)) {
            return this->errno_map[sim_errno];
        }
        if (logger.warn.enabled()) {
            logger.warn << "corresponding lc32 errno unable to be found";
        }
        return ENOSYS;
    }

    bool Filesystem::will_overflow(int num_bits, long num) {
        long max = (1l << (num_bits - 1)) - 1;
        long min = (1l << (num_bits - 1)) * -1;
        if (num > max || num < min) {
            return true;
        }
        return false;
    }
}