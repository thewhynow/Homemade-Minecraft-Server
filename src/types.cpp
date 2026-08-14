#include "inc/types.hpp"

net_string::net_string(
    std::span<uint8_t> &buff, size_t n
){
    net_var_int size{buff};

    if (size.value < 0 || size.value > (net_int)n * 3)
        throw std::out_of_range(std::to_string(__LINE__));

    if ((size_t) size.value > buff.size())
        throw std::length_error(std::to_string(__LINE__));

    value = std::string(
        (char*) buff.data(), size.value
    );

    buff = buff.subspan(size.value);
}

net_string::net_string(std::string_view value):
    value(value)
{}

void net_string::serialize(
    std::vector<uint8_t> &buff
) const {
    net_var_int size = value.size();

    buff.reserve(size.size() + value.size());

    size.serialize(buff);
    buff.insert(
        buff.end(), value.c_str(), 
        value.c_str() + value.size()
    );
}

size_t net_string::size() const {
    return 
        net_var_int(value.size()).size() 
        + value.size();
}

net_var_int::net_var_int(std::span<uint8_t> &buff){
    value = 0;

    auto it = buff.begin();

    for (int p = 0; p < 32; p += 7){
        if (it == buff.end())
            throw std::length_error(std::to_string(__LINE__));

        uint8_t curr = *(it++);
        value |= (net_int)(curr & 0x7F) << p;

        if ((curr & 0x80) == 0){
            buff = buff.subspan(it - buff.begin());
            return;
        }
    }

    throw std::out_of_range(std::to_string(__LINE__));
}

net_var_int::net_var_int(net_int value):
    value(value)
{}

void net_var_int::serialize(
    std::vector<uint8_t> &buff
) const {
    net_uint temp = value;
    buff.reserve(buff.size() + size());

    while ((temp & ~0x7F) != 0){
        buff.emplace_back((temp & 0x7F) | 0x80);
        temp >>= 7;
    }

    buff.emplace_back(temp);
}

size_t net_var_int::size() const {
    return 
        /* ciel(significant bits / 7), minimum 1 */
        (std::bit_width((net_uint)value | 1U) + 6) / 7;
}

net_var_long::net_var_long(std::span<uint8_t> &buff){
    value = 0;
    auto it = buff.begin();

    for (int p = 0; p < 64; p += 7){
        if (it == buff.end())
            throw std::length_error(std::to_string(__LINE__));

        uint8_t curr = *(it++);
        value |= (net_long)(curr & 0x7F) << p;

        if ((curr & 0x80) == 0){
            buff = buff.subspan(it - buff.begin());
            return;
        }
    }

    throw std::out_of_range(std::to_string(__LINE__));
}

net_var_long::net_var_long(net_long value):
    value(value)
{}

void net_var_long::serialize(
    std::vector<uint8_t> &buff
) const {
    net_ulong temp = value;
    buff.reserve(buff.size() + size());

    while ((temp & ~0x7F) != 0){
        buff.emplace_back((temp & 0x7F) | 0x80);
        temp >>= 7;
    }

    buff.emplace_back(temp);
}

size_t net_var_long::size() const {
    return 
        /* ciel(significant bits / 7), minimum 1 */
        (std::bit_width((net_ulong)value | 1U) + 6) / 7;
}

net_position::net_position(std::span<uint8_t> &buff){
    net_ulong raw;

    if (buff.size() < sizeof raw)
        throw std::length_error(std::to_string(__LINE__));

    std::memcpy(&raw, buff.data(), sizeof raw);
    if constexpr (
        std::endian::native == std::endian::little
    )
        raw = std::byteswap(raw);

    net_long val = raw;
    x = val >> 38;
    y = val << 52 >> 52;
    z = val << 26 >> 38;

    buff = buff.subspan(sizeof raw);
}
