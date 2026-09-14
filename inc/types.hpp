#pragma once
#include "errors.hpp"
#include "nbt.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <set>

struct net_type {
    void serialize(std::vector<uint8_t> &buff) const;
    size_t size() const;
};

template <typename B>
requires (
    std::is_integral_v<B> ||
    std::is_floating_point_v<B>
)
struct net_simple_type : net_type {
    B value;
    using base_type = B;

    net_simple_type(std::span<uint8_t> &buff){
        value = read_be<B>(buff);
    }

    net_simple_type(B value):
        value(value)
    {}

    net_simple_type &operator=(B value){
        this->value = value;
        return *this;
    }

    operator B() const {
        return value;
    }

    void serialize(std::vector<uint8_t> &buff) const {
        write_be<B>(buff, value);
    }

    size_t size() const {
        return sizeof value;
    }
};

using net_boolean = net_simple_type<bool>;
using net_byte = net_simple_type<int8_t>;
using net_ubyte = net_simple_type<uint8_t>;
using net_short = net_simple_type<int16_t>;
using net_ushort = net_simple_type<uint16_t>;
using net_int = net_simple_type<int32_t>;
/* my own addition */
using net_uint = net_simple_type<uint32_t>;
using net_long = net_simple_type<int64_t>;
/* my own addition */
using net_ulong = net_simple_type<uint64_t>;
using net_float = net_simple_type<float>;
using net_double = net_simple_type<double>;

/**
 * variadic template bullshit
 */
template <typename... Ts>
    requires((std::is_base_of_v<net_type, Ts> && ...))
struct net_compound : net_type {
    std::tuple<Ts...> fields;

    net_compound(std::span<uint8_t> &buff):
        fields{Ts(buff)...}
    {}

    net_compound(Ts... vals):
        fields(std::move(vals)...)
    {}

    template <size_t I>
    auto &get(){
        return std::get<I>(fields);
    }

    template <size_t I>
    const auto &get() const {
        return std::get<I>(fields);
    }

    void serialize(std::vector<uint8_t> &buff) const {
        std::apply(
            [&](auto &...field){
                (field.serialize(buff), ...);
            },
            fields
        );
    }

    size_t size() const {
        return std::apply(
            [&](auto &...field){
                return (field.size() + ... + 0);
            },
            fields
        );
    }
};

#define NET_COMPOUND_FIELD(num, name)                                        \
    auto &name() { return get<num>(); }                                      \
    const auto &name() const { return get<num>(); }

struct net_string : net_type {
    std::string value;

    net_string(std::span<uint8_t> &buff, size_t n = 32767);
    net_string(std::string_view value);

    void serialize(std::vector<uint8_t> &) const;
    size_t size() const;
};

using net_identifier = net_string;

struct net_var_int : net_type {
    net_int value;
    using base_type = net_int::base_type;

    net_var_int(std::span<uint8_t> &buff);
    net_var_int(base_type value);
    net_var_int &operator=(base_type base);

    operator base_type() const;
    void serialize(std::vector<uint8_t> &) const;
    size_t size() const;
};

struct net_var_long : net_type {
    net_long value;
    using base_type = net_long::base_type;

    net_var_long(std::span<uint8_t> &buff);
    net_var_long(base_type value);
    net_var_long &operator=(base_type base);

    operator base_type() const;
    void serialize(std::vector<uint8_t> &) const;
    size_t size() const;
};

struct net_uuid:
    net_compound<
        net_long, net_long
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, msq);
    NET_COMPOUND_FIELD(1, lsq);
};

template <std::derived_from<net_type> X, int N>
    requires(N > 0)
struct net_array : net_type {
    std::array<X, N> data;

    net_array(std::span<uint8_t> &buff) {
        for (int i = 0; i < N; ++i)
            data[i] = X(buff);
    }

    net_array(const std::array<X, N> &data):
        data(data)
    {}

    void serialize(std::vector<uint8_t> &buff) const {
        for (const X &i : data)
            i.serialize(buff);
    }

    size_t size() const {
        size_t s = 0;

        for (const X &i : data)
            s += i.size();

        return s;
    }
};

template <std::derived_from<net_type> X>
struct net_prefixed_array : net_type {
    std::vector<X> data;

    net_prefixed_array(std::span<uint8_t> &buff) {
        net_var_int len(buff);
        if (len < 0)
            throw malformed_packet();

        data.reserve(std::min<size_t>(len, buff.size()));

        for (int i = 0; i < len; ++i)
            data.emplace_back(buff);
    }

    net_prefixed_array(const std::vector<X> &data):
        data(data)
    {}

    net_prefixed_array(std::vector<X> &&data):
        data(std::move(data))
    {}

    void serialize(std::vector<uint8_t> &buff) const {
        net_var_int(data.size()).serialize(buff);
        for (const X &i : data)
            i.serialize(buff);
    }

    size_t size() const {
        size_t s = 0;
        for (const X &i : data)
            s += i.size();
        return s + net_var_int(data.size()).size();
    }
};

template <std::derived_from<net_type> X>
struct net_prefixed_optional : net_type {
    std::optional<X> field;

    net_prefixed_optional(std::span<uint8_t> &buff):
        field(
            net_boolean(buff)
            ? std::optional<X>(std::in_place, buff)
            : std::nullopt
        )
    {}

    net_prefixed_optional(std::nullptr_t):
        field(std::nullopt)
    {}

    net_prefixed_optional(const X &field):
        field(field)
    {}

    net_prefixed_optional(X &&field):
        field(std::move(field))
    {}

    void serialize(std::vector<uint8_t> &buff) const {
        if (field) {
            net_boolean(true).serialize(buff);
            field->serialize(buff);
        } else
            net_boolean(false).serialize(buff);
    }

    size_t size() const {
        return
            net_boolean(false).size() +
            (field ? field->size() : 0)
        ;
    }
};

struct net_game_profile_property:
    net_compound<
        net_string,
        net_string,
        net_prefixed_optional<
            net_string
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name);
    NET_COMPOUND_FIELD(1, value);
    NET_COMPOUND_FIELD(2, signature);
};

struct net_game_profile:
    net_compound<
        net_uuid,
        net_string,
        net_prefixed_array<
            net_game_profile_property
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, uuid);
    NET_COMPOUND_FIELD(1, username);
    NET_COMPOUND_FIELD(2, properties);
};

struct net_nbt_data : net_type {
    nbt_compound_untagged data;

    net_nbt_data(std::span<uint8_t> &buff);
    net_nbt_data(nbt_compound_untagged &&data);

    void serialize(std::vector<uint8_t> &) const;
    size_t size() const;
};

struct net_select_known_packs_known_pack:
    net_compound<
        net_string,
        net_string,
        net_string
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name_space);
    NET_COMPOUND_FIELD(1, id);
    NET_COMPOUND_FIELD(2, version);
};

struct net_registry_data_entry:
    net_compound<
        net_identifier,
        net_prefixed_optional<
            net_nbt_data
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, id);
    NET_COMPOUND_FIELD(1, data);
};

struct net_position : net_type {
    net_long x;
    net_long z;
    net_short y;

    net_position(net_long x, net_long z, net_short y);
    net_position(std::span<uint8_t> &buff);
    void serialize(std::vector<uint8_t> &buff) const;
    size_t size() const;
};

struct net_login_death_location :
    net_compound<
        net_identifier,
        net_position
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, dimension);
    NET_COMPOUND_FIELD(1, position);
};

struct net_tag :
    net_compound<
        net_identifier,
        net_prefixed_array<
            net_var_int
        >
    >
                       {
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name);
    NET_COMPOUND_FIELD(1, entries);
};

struct net_update_tags_tagged_registry :
    net_compound<
        net_identifier,
        net_prefixed_array<
            net_tag
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, registry);
    NET_COMPOUND_FIELD(1, tags);
};

template <bool Blocks>
struct net_paletted_container_structure : net_type {
    static constexpr size_t  num_entries  = Blocks ? 4096 : 64;
    static constexpr uint8_t min_indirect = Blocks ? 4 : 1;
    static constexpr uint8_t max_indirect = Blocks ? 8 : 3;
    static constexpr uint8_t direct_bits  = Blocks ? 15 : 7;
    net_ubyte bits_per_entry;
    uint8_t entries_per_long;
    size_t num_longs;
    uint64_t entry_mask;

    /* palette ID's; direct -> empty */
    net_prefixed_array<net_var_int> palette;
    /* always palette ID's, not local indices */
    std::vector<uint32_t> data;

    bool is_single(){
        return bits_per_entry == 0;
    }

    bool is_indirect(){
        return
            1 <= bits_per_entry &&
            bits_per_entry <= max_indirect
        ;
    }

    bool is_direct(){
        return max_indirect < bits_per_entry;
    }

    template <size_t S>
    net_paletted_container_structure(
        const std::array<uint32_t, S> &data_arr
    ):
        bits_per_entry(0),
        palette({})
    {
        std::set<int32_t> uniques {data_arr.begin(), data_arr.end()};
        bits_per_entry = ceil(log2(uniques.size()));

        entries_per_long = 64 / bits_per_entry;
        num_longs = (num_entries + entries_per_long - 1) / entries_per_long;
        entry_mask = ((uint64_t)1 << bits_per_entry) - 1;

        if (is_single())
            palette = {{data_arr[0]}};
        else if (is_indirect()){
            palette = {{uniques.begin(), uniques.end()}};

            data.reserve(data_arr.size());
            for (uint32_t val : data_arr)
                data.push_back(
                    std::distance(
                        uniques.begin(), uniques.find(val)
                    )
                );
        }
        else /* if (is_direct()) */ {
            palette = {{}};
            data = data_arr;
        }
   }

    net_paletted_container_structure(std::span<uint8_t> &buff):
        bits_per_entry(buff),
        palette({}),
        entries_per_long(64 / bits_per_entry),
        num_longs((num_entries + entries_per_long - 1) / entries_per_long),
        entry_mask(((uint64_t)1 << bits_per_entry) - 1)
    {
        if (is_single()){
            net_var_int value {buff};
            palette.data.emplace_back(value);
        }
        else {
            if (is_indirect())
                palette = {buff};
            else /* if (is_direct()) */
                palette = {{}};

            data.reserve(num_entries);
            size_t entry_index = 0;
            for (size_t i = 0; i < num_longs; ++i){
                net_long l{buff};

                for (uint8_t j = 0; j < entries_per_long; ++j){
                    uint8_t bit_index =
                        entry_index % entries_per_long * bits_per_entry;
                    uint32_t val = (l >> bit_index) & entry_mask;

                    if (is_indirect())
                        data.emplace_back(palette.data[val]);
                    else /* if (is_direct()) */
                        data.emplace_back(val);

                    ++entry_index;
                }
            }
        }
    }

    void serialize(std::vector<uint8_t> &buff) const {
        bits_per_entry.serialize(buff);

        if (is_single())
            palette.data[0].serialize(buff);
        else {
            if (is_indirect())
                palette.serialize(buff);

            size_t entry_index = 0;
            for (size_t i = 0; i < num_longs; ++i){
                net_long l {0};
                for (uint8_t j = 0; j < entries_per_long; ++j){
                    uint8_t bit_index =
                        entry_index % entries_per_long * bits_per_entry;
                    l = l | data[entry_index] << bit_index;
                    l.serialize(buff);
                    ++entry_index;
                }
            }
        }
    }

    size_t size() const {
        size_t res = bits_per_entry.size();

        if (is_single())
            res += palette.data[0].size();
        else {
            if (is_indirect())
                res += palette.size();

            res += num_longs * net_long{0}.size();
        }

        return res;
    }
};

using net_paletted_container_structure_blocks =
    net_paletted_container_structure<true>
;

using net_paletted_container_structure_biomes = 
    net_paletted_container_structure<false>
;

struct net_heightmap :
    net_compound<
        net_var_int,
        net_prefixed_array<
            net_long
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type);
    NET_COMPOUND_FIELD(1, data);

    enum class types {
        /* all blocks other than air, cave air, and void air */
        world_surface             = 1,
        /* "solid" blocks except bamboo saplings, cacti, & fluids */
        motion_blocking           = 4,
        /* same as motion_blocking excluding leaf blocks */
        motion_blocking_no_leaves = 5
    };
};

struct net_bitset :
    net_prefixed_array<net_long>
{
    using net_prefixed_array::net_prefixed_array;

    struct bit_ref {
        uint64_t *quad;
        uint8_t bit;

        operator bool() const {
            return !!(*quad >> bit);
        }

        bit_ref &operator= (bool value){
            if (value)
                *quad |= 1 << bit;
            else
                *quad &= ~(1 << bit);

            return *this;
        }
    };

    bit_ref operator[] (size_t i) {
        return bit_ref {
            (uint64_t*) &data[i / 64],
            (uint8_t) (i % 64)
        };
    }
};

struct net_light_data :
    net_compound<
        net_bitset,
        net_bitset,
        net_bitset,
        net_bitset,
        net_prefixed_array<
            net_prefixed_array<net_byte>
        >,
        net_prefixed_array<
            net_prefixed_array<net_byte>
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, sky_light_mask);
    NET_COMPOUND_FIELD(1, block_light_mask);
    NET_COMPOUND_FIELD(2, empty_sky_light_mask);
    NET_COMPOUND_FIELD(3, empty_block_light_mask);
    NET_COMPOUND_FIELD(4, sky_light_arrays);
    NET_COMPOUND_FIELD(5, block_light_arrays);
};

struct net_level_chunk_with_light_block_entities_packed_xz :
    net_type
{
    uint8_t x, z;

    net_level_chunk_with_light_block_entities_packed_xz(std::span<uint8_t> &buff);
    net_level_chunk_with_light_block_entities_packed_xz(uint8_t x, uint8_t z);

    void serialize(std::vector<uint8_t> &buff) const;
    size_t size() const;
};

struct net_level_chunk_with_light_block_entity :
    net_compound<
        net_level_chunk_with_light_block_entities_packed_xz,
        net_short,
        net_var_int,
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, packed_xz);
    NET_COMPOUND_FIELD(1, y);
    NET_COMPOUND_FIELD(2, type);
    NET_COMPOUND_FIELD(3, data);
};
