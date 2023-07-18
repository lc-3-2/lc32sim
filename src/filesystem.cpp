#include <cstring>

#include "filesystem.hpp"

namespace lc32sim {
    sim_fd Filesystem::open(const char *filename, const char *mode) {
        FILE *f = fopen(filename, mode);
        if (f == nullptr) {
            return 0;
        }

        file_table.push_back(File(f, true));
        return file_table.size();
    }

    sim_size_t Filesystem::read(sim_fd fd, void *ptr, sim_size_t size, sim_size_t nmemb) {
        if (fd == 0 || fd > file_table.size()) {
            return 0;
        }

        File &f = file_table[fd - 1];
        if (!f.open) {
            return 0;
        }

        return fread(ptr, size, nmemb, f.f);
    }

    sim_size_t Filesystem::write(sim_fd fd, const void *ptr, sim_size_t size, sim_size_t nmemb) {
        if (fd == 0 || fd > file_table.size()) {
            return 0;
        }

        File &f = file_table[fd - 1];
        if (!f.open) {
            return 0;
        }

        return fwrite(ptr, size, nmemb, f.f);
    }

    sim_size_t Filesystem::seek(sim_fd fd, sim_long offset, uint32_t whence) {
        if (fd == 0 || fd > file_table.size()) {
            return 2110;
        }

        File &f = file_table[fd - 1];
        if (!f.open) {
            return 2111;
        }

        return fseek(f.f, offset, whence);
    }

    sim_int Filesystem::close(sim_fd fd) {
        if (fd == 0 || fd > file_table.size()) {
            return EOF;
        }

        File &f = file_table[fd - 1];
        if (!f.open) {
            return EOF;
        }

        fclose(f.f);
        f.open = false;
        return 0;
    }
}