#include "elf_file.hpp"
#include "exceptions.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>

namespace lc32sim {
    // TODO: investigate potential performance improvement by using mmap()
    template <typename T, bool reverse> T ELFFile::read() {
        char buf[sizeof(T)];
        file.read(buf, sizeof(T));
        if constexpr (reverse) {
            std::reverse(buf, buf + sizeof(T));
        }
        return *reinterpret_cast<T*>(buf);
    }

    ELFFile::ELFFile(const std::string& filename) {
        file.open(filename, std::ios_base::in | std::ios::binary);
        if (!file.is_open()) {
            throw ELFParsingException("File " + filename + " does not exist");
        }

        // Read header
        struct elf32_ident {
            uint8_t magic[4];
            uint8_t _class;
            uint8_t data;
            uint8_t version;
            uint8_t os_abi;
            uint8_t abi_version;
            uint8_t padding[7];
        }__attribute__((packed, aligned(4)));
        static_assert(sizeof(elf32_ident) == 16, "elf32_ident is not 16 bytes");
        elf32_ident ei = read<elf32_ident, false>();

        // Check magic number
        if (ei.magic[0] != 0x7f || ei.magic[1] != 'E' || ei.magic[2] != 'L' || ei.magic[3] != 'F') {
            throw ELFParsingException("Invalid ELF magic number: 0x" + int_to_hex(ei.magic[0]) + int_to_hex(ei.magic[1]) + int_to_hex(ei.magic[2]) + int_to_hex(ei.magic[3]));
        }
        if (ei._class == 2) {
            throw ELFParsingException("64-bit ELF files are not supported");
        } else if (ei._class != 1) {
            throw ELFParsingException("Invalid ELF class: " + std::to_string(ei._class));
        }

        if (ei.data == 1) {
            this->reverse = std::endian::native == std::endian::big;
        } else if (ei.data == 2) {
            this->reverse = std::endian::native == std::endian::little;
        } else {
            throw ELFParsingException("Invalid ELF data encoding: " + std::to_string(ei.data));
        }

        if (ei.version != 1) {
            throw ELFParsingException("Invalid ELF version: " + std::to_string(ei.version));
        }

        // os_abi, abi_version, and padding are ignored

        // Read rest of header for later use
        if (reverse) {
            eh = read<elf32_header, true>();
        } else {
            eh = read<elf32_header, false>();
        }
        if (eh.type == 0x0) {
            throw ELFParsingException("ET_NONE object file type is not supported");
        } else if (eh.type == 0x1) {
            throw ELFParsingException("ET_REL object file type is not supported");
        } else if (eh.type == 0x3) {
            throw ELFParsingException("ET_DYN object file type is not supported");
        } else if (eh.type == 0x4) {
            throw ELFParsingException("ET_CORE object file type is not supported");
        } else if (eh.type == 0xFE00) {
            throw ELFParsingException("ET_LOOS object file type is not supported");
        } else if (eh.type == 0xFEFF) {
            throw ELFParsingException("ET_HIOS object file type is not supported");
        } else if (eh.type == 0xFF00) {
            throw ELFParsingException("ET_LOPROC object file type is not supported");
        } else if (eh.type == 0xFFFF) {
            throw ELFParsingException("ET_HIPROC object file type is not supported");
        } else if (eh.type != 0x2) {
            throw ELFParsingException("Invalid ELF object file type: " + std::to_string(eh.type));
        }

        // Read program headers
        ph = std::make_unique<elf32_program_header[]>(eh.phnum);
        file.seekg(eh.phoff, std::ios::beg);
        std::streamoff phentsize_diff = eh.phentsize - sizeof(elf32_program_header);
        for (uint16_t i = 0; i < eh.phnum; i++) {
            if (reverse) {
                ph[i] = read<elf32_program_header, true>();
            } else {
                ph[i] = read<elf32_program_header, false>();
            }
            file.seekg(phentsize_diff, std::ios::cur);
        }
    }
    ELFFile::~ELFFile() {
        file.close();
    }

    void ELFFile::read_chunk(uint8_t *buf, uint32_t offset, uint32_t size) {
        file.seekg(offset, std::ios::beg);
        file.read(reinterpret_cast<char*>(buf), size);
    }
}