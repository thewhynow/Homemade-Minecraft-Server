#include "inc/types.hpp"
#include <cstring>
#include <bit>

#define DEFINE_SIMPLE_NET_TYPE(net_name, base_name)       \
    net_name::net_name(                                   \
        std::span<uint8_t> &buff                          \
    ):                                                    \
        value(read_be<base_name>(buff))                   \
    {}                                                    \
                                                          \
    net_name::net_name(base_type value):                  \
        value(value)                                      \
    {}                                                    \
                                                          \
    net_name::operator base_type() const {                \
        return value;                                     \
    }                                                     \
                                                          \
    net_name &net_name::operator=(base_type base){        \
        value = base;                                     \
        return *this;                                     \
    }                                                     \
                                                          \
    void net_name::serialize(                             \
        std::vector<uint8_t> &buff                        \
    ) const {                                             \
        write_be<base_name>(buff, value);                 \
    }                                                     \
                                                          \
    size_t net_name::size() const {                       \
        return sizeof value;                              \
    }


DEFINE_SIMPLE_NET_TYPE(net_boolean, bool);
DEFINE_SIMPLE_NET_TYPE(net_byte,    int8_t);
DEFINE_SIMPLE_NET_TYPE(net_ubyte,   uint8_t);
DEFINE_SIMPLE_NET_TYPE(net_short,   int16_t);
DEFINE_SIMPLE_NET_TYPE(net_ushort,  uint16_t);
DEFINE_SIMPLE_NET_TYPE(net_int,     int32_t);
/* my own addition */
DEFINE_SIMPLE_NET_TYPE(net_uint,    uint32_t);
DEFINE_SIMPLE_NET_TYPE(net_long,    int64_t);
/* my own addition */
DEFINE_SIMPLE_NET_TYPE(net_ulong,   uint64_t);
DEFINE_SIMPLE_NET_TYPE(net_float,   float);
DEFINE_SIMPLE_NET_TYPE(net_double,  double);

net_string::net_string(
    std::span<uint8_t> &buff, size_t n
){
    net_var_int size(buff);

    if (size < 0 || size > (net_int) n * 3)
        throw malformed_packet("string size exceeds bounds");

    if ((size_t) size > buff.size())
        throw unfinished_packet();

    value = std::string(
        (char*) buff.data(), size
    );

    buff = buff.subspan(size);
}

net_string::net_string(std::string_view value):
    value(value)
{}

void net_string::serialize(
    std::vector<uint8_t> &buff
) const {
    net_var_int size(value.size());

    buff.reserve(buff.size() + size.size() + value.size());

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

net_var_int::net_var_int(std::span<uint8_t> &buff):
    value(0)
{
    auto it = buff.begin();

    for (int p = 0; p < 32; p += 7){
        if (it == buff.end())
            throw unfinished_packet();

        uint8_t curr = *(it++);
        value = value | (net_int)(curr & 0x7F) << p;

        if ((curr & 0x80) == 0){
            buff = buff.subspan(it - buff.begin());
            return;
        }
    }

    throw malformed_packet("varint exceeds size bounds");
}

net_var_int::net_var_int(base_type value):
    value(value)
{}

net_var_int::operator base_type() const {
    return value;
}

net_var_int &net_var_int::operator=(base_type base){
    value = base;
    return *this;
}

void net_var_int::serialize(
    std::vector<uint8_t> &buff
) const {
    net_uint temp = (net_uint) value;
    buff.reserve(buff.size() + size());

    while ((temp & ~0x7F) != 0){
        buff.emplace_back((temp & 0x7F) | 0x80);
        temp = temp >> 7;
    }

    buff.emplace_back(temp);
}

size_t net_var_int::size() const {
    return 
        /* ciel(significant bits / 7), minimum 1 */
        (std::bit_width((net_uint)value | 1U) + 6) / 7;
}

net_var_long::net_var_long(std::span<uint8_t> &buff):
    value(0)
{
    auto it = buff.begin();

    for (int p = 0; p < 64; p += 7){
        if (it == buff.end())
            throw unfinished_packet();

        uint8_t curr = *(it++);
        value = value | (net_long)(curr & 0x7F) << p;

        if ((curr & 0x80) == 0){
            buff = buff.subspan(it - buff.begin());
            return;
        }
    }

    throw malformed_packet("varlong exceeds size bounds");
}

net_var_long::net_var_long(base_type value):
    value(value)
{}

net_var_long::operator base_type() const {
    return value;
}

net_var_long &net_var_long::operator=(base_type base){
    value = base;
    return *this;
}

void net_var_long::serialize(
    std::vector<uint8_t> &buff
) const {
    net_ulong temp = (net_ulong) value;
    buff.reserve(buff.size() + size());

    while ((temp & ~0x7F) != 0){
        buff.emplace_back((temp & 0x7F) | 0x80);
        temp = temp >> 7;
    }

    buff.emplace_back(temp);
}

size_t net_var_long::size() const {
    return 
        /* ciel(significant bits / 7), minimum 1 */
        (std::bit_width((net_ulong)value | 1U) + 6) / 7;
}

net_nbt_data::net_nbt_data(
    std::span<uint8_t> &buff
):
    data(/* need to read tag first */
        (read_be<nbt_tag>(buff), buff)
    )
{}

net_nbt_data::net_nbt_data(
    nbt_compound_untagged &&data
):
    data(std::move(data))
{}

void net_nbt_data::serialize(
    std::vector<uint8_t> &buff
) const {
    write_be(buff, nbt_tag::compound);
    data.serialize(buff);
}

size_t net_nbt_data::size() const {
    return sizeof(nbt_tag::compound)
        + data.size();
}

net_position::net_position(std::span<uint8_t> &buff):
    x(0), z(0), y(0)
{
    net_long val = (net_long) net_ulong(buff);

    x = val >> 38;
    y = val << 52 >> 52;
    z = val << 26 >> 38;
}
