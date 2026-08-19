#pragma once
#include <cstdint>
#include <cstring>
#include <bit>
#include <span>
#include <vector>
#include <type_traits>
#include "errors.hpp"

template <typename T>
using wire_raw_type =
    std::conditional_t<sizeof(T) == 1, uint8_t,
    std::conditional_t<sizeof(T) == 2, uint16_t,
    std::conditional_t<sizeof(T) == 4, uint32_t,
                                       uint64_t
    >>>
;

template <typename T>
T read_be(std::span<uint8_t> &buff){
    wire_raw_type<T> raw;

    if (buff.size() < sizeof raw)
        throw unfinished_packet();

    memcpy(&raw, buff.data(), sizeof raw);

    if constexpr (std::endian::native != std::endian::big)
        raw = std::byteswap(raw);

    buff = buff.subspan(sizeof raw);

    return std::bit_cast<T>(raw);
}

template <typename T>
void write_be(std::vector<uint8_t> &buff, T value){
    wire_raw_type<T> raw = std::bit_cast<wire_raw_type<T>>(value);

    if constexpr (std::endian::native != std::endian::big)
        raw = std::byteswap(raw);

    size_t off = buff.size();
    buff.resize(off + sizeof raw);
    memcpy(buff.data() + off, &raw, sizeof raw);
}
