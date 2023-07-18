#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

namespace lc32sim { 
    using sim_fd = uint16_t;
    using sim_size_t = uint32_t;
    using sim_long = int64_t;
    using sim_int = int32_t;

    // this guarantee is needed for safe conversions
    static_assert(sizeof(sim_size_t) <= sizeof(size_t));


    class Filesystem {
        private:
            struct File {
                FILE *f = nullptr;
                bool open = false;
                File(FILE *f, bool open) : f(f), open(open) {}
            };

            std::vector<File> file_table;

        public:
            Filesystem() : file_table() {}

            sim_fd open(const char *filename, const char* mode);
            sim_size_t read(sim_fd fd, void *ptr, sim_size_t size, sim_size_t nmemb);
            sim_size_t write(sim_fd fd, const void *ptr, sim_size_t size, sim_size_t nmemb);
            sim_size_t seek(sim_fd fd, sim_long offset, uint32_t whence);
            sim_int close(sim_fd fd);
    };
}