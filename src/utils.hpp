#pragma once
#include <cstdint>

template<std::size_t from, std::size_t to>
inline uint32_t sext(uint32_t val) {
    static_assert(from <= to, "`from` must be less than or equal to to");
    static_assert(to <= 32, "`to` must be less than or equal to 32");

    constexpr uint32_t sign_bit = static_cast<uint32_t>(1) << (from - 1);
    constexpr uint32_t extend_mask = (static_cast<uint32_t>(-1) << from) & ((static_cast<uint32_t>(-1) >> (32 - to)));
    if (val & sign_bit) {
        val |= extend_mask;
    }
    return val;
}