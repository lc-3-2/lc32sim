#include <bit>
#include <fstream>
#include <string>

namespace lc32sim {
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "mixed-endian architectures are not supported");
    class ELFParser {
        private:
            struct elf32_header {
                uint16_t machine;
                uint32_t version;
                uint32_t entry;
                uint32_t phoff;
                uint32_t shoff;
                uint32_t flags;
                uint16_t ehsize;
                uint16_t phentsize;
                uint16_t phnum;
                uint16_t shentsize;
                uint16_t shnum;
                uint16_t shstrndx;
            };
            bool reverse;
            elf32_header eh;
            std::ifstream file;
            template <typename T, bool reverse> T read();

        public:
            ELFParser(const std::string& filename);
            ~ELFParser();
            inline uint16_t get_num_segments() const { return eh.phnum; }
    };
}