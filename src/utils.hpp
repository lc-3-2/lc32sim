#pragma once
#include <cstdint>

#define forceinline __attribute__((always_inline)) inline

template<std::size_t from, std::size_t to>
inline uint32_t sext(uint32_t val) {
    static_assert(from < to, "`from` must be less than to");
    static_assert(to <= 32, "`to` must be less than or equal to 32");

    constexpr uint32_t sign_bit = static_cast<uint32_t>(1) << (from - 1);
    constexpr uint32_t extend_mask = (static_cast<uint32_t>(-1) << from) & ((static_cast<uint32_t>(-1) >> (32 - to)));
    if (val & sign_bit) {
        val |= extend_mask;
    }
    return val;
}

template <std::size_t from, std::size_t to>
inline int32_t sext(int32_t val) {
    uint32_t result = sext<from, to>(*reinterpret_cast<uint32_t*>(&val));
    return *reinterpret_cast<int32_t*>(&result);
}

// suffixes to convert literals to various fixed-width types
constexpr std::uint32_t operator "" _u32(unsigned long long v)
{ return static_cast<std::uint32_t>(v); }

constexpr std::uint64_t operator "" _u64(unsigned long long v)
{ return static_cast<std::uint64_t>(v); }