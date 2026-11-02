#include "inc/types.hpp"
#include "inc/chunk.hpp"
#include "registry/blocks.hpp"
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

net_id_set::net_id_set(std::span<uint8_t> &buff):
    type(buff)
{
    if (type == 0)
        tag_name = net_identifier {buff};
    else {
        if (type < 0)
            throw malformed_packet("negative id set length");

        ids.emplace();
        ids->reserve(std::min<size_t>(type - 1, buff.size()));

        for (int32_t i = 0; i < type - 1; ++i)
            ids->emplace_back(net_var_int {buff});
    }
}

void net_id_set::serialize(std::vector<uint8_t> &buff) const {
    type.serialize(buff);

    if (type == 0)
        tag_name->serialize(buff);
    else
        for (const net_var_int &i : *ids)
            i.serialize(buff);
}

size_t net_id_set::size() const {
    size_t res = type.size();

    if (type == 0)
        res += tag_name->size();
    else
        for (const net_var_int &i : *ids)
            res += i.size();

    return res;
}

/*
struct net_property : net_type {
    net_string name;
    net_boolean is_exact_match;

    std::optional<net_string> exact_value;
    std::optional<net_string> min_value;
    std::optional<net_string> max_value;

    net_property(std::span<uint8_t> &buff);

    void serialize(std::vector<uint8_t> &buff) const;

    size_t size() const;
};
*/

net_property::net_property(std::span<uint8_t> &buff):
    name(buff),
    is_exact_match(buff)
{
    if (is_exact_match)
        exact_value.emplace(buff);
    else {
        min_value.emplace(buff);
        max_value.emplace(buff);
    }
}

void net_property::serialize(std::vector<uint8_t> &buff) const {
    name.serialize(buff);
    is_exact_match.serialize(buff);

    if (is_exact_match)
        exact_value->serialize(buff);
    else {
        min_value->serialize(buff);
        max_value->serialize(buff);
    }
}

size_t net_property::size() const {
    size_t res = name.size();
    res += is_exact_match.size();

    if (is_exact_match)
        res += exact_value->size();
    else {
        res += min_value->size();
        res += max_value->size();
    }

    return res;
}


net_nbt_any::net_nbt_any(std::span<uint8_t> &buff):
    data(nbt_type_untagged::build_object(read_be<nbt_tag>(buff), buff))
{}

net_nbt_any::net_nbt_any(
    std::unique_ptr<nbt_type_untagged> &&data
):
    data(std::move(data))
{}

void net_nbt_any::serialize(std::vector<uint8_t> &buff) const {
    write_be(buff, data->get_tag());
    data->serialize(buff);
}

size_t net_nbt_any::size() const {
    return sizeof(nbt_tag) + data->size();
}

net_sound_event::net_sound_event(std::span<uint8_t> &buff):
    sound_name(buff),
    has_fixed_range(buff),
    fixed_range(
        has_fixed_range
            ? std::optional<net_float>(std::in_place, buff)
            : std::nullopt
    )
{}

net_sound_event::net_sound_event(std::string_view sound_name):
    sound_name(sound_name),
    has_fixed_range(false)
{}

net_sound_event::net_sound_event(
    std::string_view sound_name, float fixed_range
):
    sound_name(sound_name),
    has_fixed_range(true),
    fixed_range(fixed_range)
{}

void net_sound_event::serialize(std::vector<uint8_t> &buff) const {
    sound_name.serialize(buff);
    has_fixed_range.serialize(buff);

    if (has_fixed_range)
        fixed_range->serialize(buff);
}

size_t net_sound_event::size() const {
    size_t res = sound_name.size();
    res += has_fixed_range.size();

    if (has_fixed_range)
        res += fixed_range->size();

    return res;
}

static net_consume_effect::effect_variant read_consume_effect(
    int32_t type, std::span<uint8_t> &buff
){
    using types = net_consume_effect::types;

    switch ((types) type){
        case types::apply_effects:
            return net_consume_effect_apply_effects{buff};
        case types::remove_effects:
            return net_consume_effect_remove_effects{buff};
        case types::clear_all_effects:
            return net_consume_effect_clear_all_effects{buff};
        case types::teleport_randomly:
            return net_consume_effect_teleport_randomly{buff};
        case types::play_sound:
            return net_consume_effect_play_sound{buff};
    }

    throw malformed_packet("bad consume effect type");
}

net_consume_effect::net_consume_effect(std::span<uint8_t> &buff):
    type(buff),
    effect(read_consume_effect(type, buff))
{}

net_consume_effect::net_consume_effect(effect_variant &&effect):
    type((int32_t) effect.index()),
    effect(std::move(effect))
{}

void net_consume_effect::serialize(std::vector<uint8_t> &buff) const {
    type.serialize(buff);

    std::visit(
        [&](const auto &i){ i.serialize(buff); },
        effect
    );
}

size_t net_consume_effect::size() const {
    return type.size() + std::visit(
        [](const auto &i){ return i.size(); },
        effect
    );
}

net_component_lodestone_tracker::net_component_lodestone_tracker(
    std::span<uint8_t> &buff
):
    has_global_position(buff),
    dimension(
        has_global_position
            ? std::optional<net_identifier>(std::in_place, buff)
            : std::nullopt
    ),
    position(
        has_global_position
            ? std::optional<net_position>(std::in_place, buff)
            : std::nullopt
    ),
    tracked(buff)
{}

void net_component_lodestone_tracker::serialize(
    std::vector<uint8_t> &buff
) const {
    has_global_position.serialize(buff);

    if (has_global_position) {
        dimension->serialize(buff);
        position->serialize(buff);
    }

    tracked.serialize(buff);
}

size_t net_component_lodestone_tracker::size() const {
    size_t res = has_global_position.size();

    if (has_global_position) {
        res += dimension->size();
        res += position->size();
    }

    return res + tracked.size();
}

static net_resolvable_profile::profile_variant read_resolvable_profile(
    int32_t kind, std::span<uint8_t> &buff
){
    using kinds = net_resolvable_profile::kinds;

    switch ((kinds) kind){
        case kinds::partial:
            return net_resolvable_profile_partial{buff};
        case kinds::complete:
            return net_game_profile{buff};
    }

    throw malformed_packet("bad resolvable profile kind");
}

net_resolvable_profile::net_resolvable_profile(std::span<uint8_t> &buff):
    kind(buff),
    profile(read_resolvable_profile(kind, buff)),
    body(buff),
    cape(buff),
    elytra(buff),
    model(buff)
{}

void net_resolvable_profile::serialize(std::vector<uint8_t> &buff) const {
    kind.serialize(buff);

    std::visit(
        [&](const auto &i){ i.serialize(buff); },
        profile
    );

    body.serialize(buff);
    cape.serialize(buff);
    elytra.serialize(buff);
    model.serialize(buff);
}

size_t net_resolvable_profile::size() const {
    return
        kind.size() +
        std::visit([](const auto &i){ return i.size(); }, profile) +
        body.size() +
        cape.size() +
        elytra.size() +
        model.size()
    ;
}
