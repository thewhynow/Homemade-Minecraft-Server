#pragma once
#include <cstdlib>
#include <vector>
#include <span>
#include <bit>

using net_boolean = bool;
using net_byte    = int8_t;
using net_ubyte   = uint8_t;
using net_short   = int16_t;
using net_ushort  = uint16_t;
using net_int     = int32_t;
/* my own addition */
using net_uint    = uint32_t;
using net_long    = int64_t;
/* my own addition */
using net_ulong   = uint64_t;
using net_float   = float;
using net_double  = double;

template<typename T>
T net_type_from_span(std::span<uint8_t> &buff){
    std::make_unsigned_t<T> raw;

    if (buff.size() < sizeof raw)
        throw std::length_error(std::to_string(__LINE__));

    std::memcpy(&raw, buff.data(), sizeof raw);
    if constexpr (
        std::endian::native == std::endian::little
    )
        raw = std::byteswap(raw);

    buff = buff.subspan(sizeof raw);

    return (T) raw;
}

template <typename T>
void net_type_serialize(
    T val, std::vector<uint8_t> &buff
){
    if constexpr (sizeof(T) == 1)
        buff.emplace_back((uint8_t) val);
    else {
        std::make_unsigned_t<T> raw =
            (std::make_unsigned_t<T>) val;

        if constexpr (
            std::endian::native == std::endian::little
        )
            raw = std::byteswap(raw);

        size_t off = buff.size();
        buff.resize(off + sizeof raw);
        std::memcpy(buff.data() + off, &raw, sizeof raw);
    }
}

struct net_type {
    void serialize (
        std::vector<uint8_t> &buff
    ) const;
    size_t size() const;
};

#define NET_TYPE_VIRTUAL_FUNCTION_OVERRIDE                 \
    void serialize(std::vector<uint8_t> &) const; \
    size_t size() const;

struct net_string : net_type {
    std::string value;

    net_string(
        std::span<uint8_t> &buff, size_t n = 32767
    );
    net_string(std::string_view value);

    NET_TYPE_VIRTUAL_FUNCTION_OVERRIDE
};

struct net_var_int : net_type {
    net_int value;

    net_var_int(std::span<uint8_t> &buff);
    net_var_int(net_int value);

    NET_TYPE_VIRTUAL_FUNCTION_OVERRIDE
};

struct net_var_long : net_type {
    net_long value;

    net_var_long(std::span<uint8_t> &buff);
    net_var_long(net_long value);

    NET_TYPE_VIRTUAL_FUNCTION_OVERRIDE
};

struct net_position {
    net_long x : 26;
    net_long z : 26;
    net_short y : 12;

    net_position(std::span<uint8_t> &buff);
};

