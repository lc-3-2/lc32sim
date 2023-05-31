#pragma once
#include <bit>
#include <fstream>
#include <memory>
#include <string>

namespace lc32sim {
    static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "mixed-endian architectures are not supported");
    enum class segment_type : uint32_t {
        _NULL = 0x0,
        LOADABLE = 0x1,
        DYNAMIC = 0x2,
        INTERP = 0x3,
        NOTE = 0x4,
        SHLIB = 0x5,
        PHDR = 0x6,
        TLS = 0x7,
        LOOS = 0x60000000,
        HIOS = 0x6FFFFFFF,
        LOPROC = 0x70000000,
        HIPROC = 0x7FFFFFFF
    };
    inline std::ostream &operator<<(std::ostream &stream, segment_type const &s) {
        switch (s) {
            case segment_type::_NULL: stream << "_NULL"; break;
            case segment_type::LOADABLE: stream << "LOADABLE"; break;
            case segment_type::DYNAMIC: stream << "DYNAMIC"; break;
            case segment_type::INTERP: stream << "INTERP"; break;
            case segment_type::NOTE: stream << "NOTE"; break;
            case segment_type::SHLIB: stream << "SHLIB"; break;
            case segment_type::PHDR: stream << "PHDR"; break;
            case segment_type::TLS: stream << "TLS"; break;
            default: {
                uint32_t s_val = static_cast<uint32_t>(s);
                uint32_t loos_val = static_cast<uint32_t>(segment_type::LOOS);
                uint32_t hios_val = static_cast<uint32_t>(segment_type::HIPROC);
                uint32_t loproc_val = static_cast<uint32_t>(segment_type::LOPROC);
                uint32_t hiproc_val = static_cast<uint32_t>(segment_type::HIPROC);
                if (loos_val <= s_val && s_val <= hios_val) {
                    stream << "OS-specific segment type 0x" << std::hex << s_val;
                } else if (loproc_val <= s_val && s_val <= hiproc_val) {
                    stream << "processor-specific segment type 0x" << std::hex << s_val;
                } else {
                    stream << "unknown segment type 0x" << std::hex << static_cast<uint32_t>(s); break;
                }
            }
        };
        return stream;
    } 
    struct elf32_header {
        uint16_t type;
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
    }__attribute__((packed, aligned(4)));
    static_assert(sizeof(elf32_header) == 36, "elf32_header is not 36 bytes");
    struct elf32_program_header {
        segment_type type;
        uint32_t offset;
        uint32_t vaddr;
        uint32_t paddr;
        uint32_t filesz;
        uint32_t memsz;
        uint32_t flags;
        uint32_t align;
    }__attribute__((packed, aligned(4)));
    static_assert(sizeof(elf32_program_header) == 32, "elf32_program_header is not 32 bytes");
    class ELFFile {
        private:
            bool reverse;
            elf32_header eh;
            std::ifstream file;
            std::unique_ptr<elf32_program_header[]> ph;
            template <typename T, bool reverse> T read();

        public:
            ELFFile(const std::string& filename);
            ~ELFFile();
            void read_chunk(uint8_t *buf, uint32_t offset, uint32_t size);
            inline const elf32_header &get_header() const { return eh; }
            inline const elf32_program_header &get_program_header(int i) const {
                if (i >= eh.phnum)
                    throw std::out_of_range("program header index out of range");
                return ph[i];
            }
    };
}