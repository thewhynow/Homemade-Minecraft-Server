#include "inc/types.hpp"
#include "inc/chunk.hpp"
#include "inc/blocks.hpp"
#include <cstring>
#include <bit>

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

net_position::net_position(
    net_long x, net_long z, net_short y
):
    x(x), z(z), y(y)
{}

net_position::net_position(std::span<uint8_t> &buff):
    x(0), z(0), y(0)
{
    net_long val = (net_long) net_ulong(buff);

    x = val >> 38;
    y = val << 52 >> 52;
    z = val << 26 >> 38;
}

void net_position::serialize(
    std::vector<uint8_t> &buff
) const {
    net_long {
        (int64_t) (((x & 0x3FFFFFF) << 38) | ((z & 0x3FFFFFF) << 12) | (y & 0xFFF))
    }.serialize(buff);
}

size_t net_position::size() const {
    return net_long{0}.size();
}

net_level_chunk_with_light_block_entities_packed_xz
::net_level_chunk_with_light_block_entities_packed_xz(
    std::span<uint8_t> &buff
){
    net_ubyte packed{buff};

    x = packed >> 4;
    z = packed & 15;
}

net_level_chunk_with_light_block_entities_packed_xz
::net_level_chunk_with_light_block_entities_packed_xz(
    uint8_t x, uint8_t z
):
    x(x), z(z)
{}

void net_level_chunk_with_light_block_entities_packed_xz
::serialize(std::vector<uint8_t> &buff) const {
    net_ubyte packed {(uint8_t)(((x & 15) << 4) | (z & 15))};
    packed.serialize(buff);
}

size_t net_level_chunk_with_light_block_entities_packed_xz
::size() const {
    return 1;
}

net_chunk_section::net_chunk_section (
    chunk_section section
):
    net_compound(
        {0}, {0},
        section.blocks,
        section.biomes
    )
{
    for (const std::array<std::array<uint32_t, 16>, 16> &slice : section.blocks)
        for (const std::array<uint32_t, 16> &row : slice)
            for (uint32_t b : row)
                if (b != block::air)
                    block_count().value++;
}

net_light_data::net_light_data(
    const chunk &c
):
    net_compound(
        {std::vector<bool>((size_t) (384 / 16 + 2), true)},
        {std::vector<bool>((size_t) (384 / 16 + 2), false)},
        {std::vector<bool>((size_t) (384 / 16 + 2), false)},
        {std::vector<bool>((size_t) (384 / 16 + 2), false)},
        {{}},
        {{}}
    )
{
    /**
     * treating all light as sky light.
     * in the future, seperate sky light & block light
     */

    sky_light_arrays().data.resize (
        384 / 16 + 2,
        { std::vector<net_byte>(2048, net_byte {0}) }
    );

    for (size_t si = 1; si < 384 / 16 + 1; ++si){
        const chunk_section &section = c.sections[si - 1];
        auto sli = sky_light_arrays().data[si].data.begin();

        for (
            const std::array<std::array<uint8_t, 16>, 16> &slice : section.lights
        )
            for (const std::array<uint8_t, 16> &row : slice)
                for (
                    auto it = row.begin(); it != row.end(); it += 2, sli++
                ){
                    *sli =
                        net_byte {
                            (int8_t)((*it & 15) | ((*(it + 1) & 15) << 4))
                        }
                    ;
                }
    }

    /* full sky light in the section above the world */
    std::fill(
        sky_light_arrays().data[384 / 16 + 1].data.begin(),
        sky_light_arrays().data[384 / 16 + 1].data.end(),
        net_byte {(int8_t) 0xFF}
    );
}

net_bitset::net_bitset (const std::vector<bool> &bits):
    net_prefixed_array({})
{
    data.resize(
        (bits.size() + 63) / 64,
        net_long {0}
    );

    for (size_t i = 0; i < bits.size(); ++i)
        if (bits[i])
            data[i / 64].value |= (int64_t) ((uint64_t) 1 << (i % 64));
}
