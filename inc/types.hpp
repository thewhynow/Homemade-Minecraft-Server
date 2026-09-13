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
#include <variant>
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

    net_position(std::span<uint8_t> &buff);
    void serialize(std::vector<uint8_t> &buff);
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

struct net_paletted_container_structure_single :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, value);
};

struct net_paletted_container_structure_indirect :
    net_compound<
        net_prefixed_array<net_var_int>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, palette);
};

struct net_paletted_container_structure_direct :
    net_compound<>
{};

template <bool Blocks>
struct net_paletted_container_structure : net_type {
    static constexpr size_t  entry_count  = Blocks ? 4096 : 64;
    static constexpr uint8_t min_indirect = Blocks ? 4 : 1;
    static constexpr uint8_t max_indirect = Blocks ? 8 : 3;
    static constexpr uint8_t direct_bits  = Blocks ? 15 : 7;
    net_ubyte bits_per_entry;

    /* palette ID's, wire order; direct -> empty */
    std::vector<uint32_t> _palette;
    /* always palette ID's, not local indices */
    std::vector<uint32_t> data;

    std::variant<
        net_paletted_container_structure_single,
        net_paletted_container_structure_indirect,
        net_paletted_container_structure_direct
    > palette;

    /**
     * stores the raw ID's - NOT the mapped ones
     */

    net_paletted_container_structure(
        const std::vector<uint32_t> &data
    ):
        bits_per_entry(0),
        palette(net_paletted_container_structure_direct{})
    {
        std::set<int32_t> uniques {data.begin(), data.end()};

        if (uniques.size() == 1){
            bits_per_entry = 0;
            palette = net_paletted_container_structure_single {
                {(int) data[0]}
            };
            return;
        }

        bits_per_entry = ceil(log2(uniques.size()));

        if constexpr (Blocks){
            if (bits_per_entry <= 8){
                if (bits_per_entry < 4)
                    bits_per_entry = 4;

                palette = net_paletted_container_structure_indirect {
                    {{uniques.begin(), uniques.end()}}
                };
            }
            else if (bits_per_entry <= 15){
                if (bits_per_entry < 15)
                    bits_per_entry = 15;

                palette = net_paletted_container_structure_direct {};
            }
            else
                throw std::runtime_error("too many bits per entry");
        }
        else {
            if (bits_per_entry <= 3){
                if (bits_per_entry < 3)
                    bits_per_entry = 3;

                palette = net_paletted_container_structure_indirect {
                    {{uniques.begin(), uniques.end()}}
                };
            }
            else if (bits_per_entry <= 7){
                if (bits_per_entry < 7)
                    bits_per_entry = 7;

                palette = net_paletted_container_structure_direct {};
            }
            else
                throw std::runtime_error("too many bits per entry");
        }

        this->data = data;
    }

    net_paletted_container_structure(std::span<uint8_t> &buff):
        bits_per_entry(buff)
    {

        if (bits_per_entry == 0){
            palette = net_paletted_container_structure_single {buff};
            return;
        }

        if constexpr (Blocks){
            if (bits_per_entry <= 8){
                if (bits_per_entry < 4)
                    bits_per_entry = 4;

                palette = net_paletted_container_structure_indirect {buff};
            }
            else if (bits_per_entry <= 15){
                if (bits_per_entry < 15)
                    bits_per_entry = 15;

                palette = net_paletted_container_structure_direct {};
            }
            else
                throw std::runtime_error("too many bits per entry");
        }
        else {
            if (bits_per_entry <= 3){
                if (bits_per_entry < 3)
                    bits_per_entry = 3;

                palette = net_paletted_container_structure_indirect {buff};
            }
            else if (bits_per_entry <= 7){
                if (bits_per_entry < 7)
                    bits_per_entry = 7;

                palette = net_paletted_container_structure_direct {};
            }
            else
                throw std::runtime_error("too many bits per entry");
        }

        size_t num_of_entries;
        if constexpr (Blocks)
            num_of_entries = 4096;
        else
            num_of_entries = 64;

        if (
            std::holds_alternative<net_paletted_container_structure_single>(palette)
        ){
            net_paletted_container_structure_single &single_palette =
                std::get<net_paletted_container_structure_single>(palette)
            ;

            for (size_t i = 0; i < num_of_entries; ++i)
                data.push_back(single_palette.value());
        }
        else {
            uint8_t entries_per_long = 64 / bits_per_entry;
            size_t num_longs = (num_of_entries + entries_per_long - 1) / entries_per_long;
            uint64_t entry_mask = ((uint64_t)1 << bits_per_entry) - 1;

            size_t entry_index = 0;

            for (size_t i = 0; i < num_longs; ++i){
                net_long l{buff};

                for (uint8_t j = 0; j < entries_per_long; ++j){
                    uint8_t bit_index = entry_index % entries_per_long * bits_per_entry;
                    uint32_t val = (l >> bit_index) & entry_mask;

                    if (
                        std::holds_alternative<net_paletted_container_structure_indirect>(palette)
                    ){
                        net_paletted_container_structure_indirect &indirect_palette =
                            std::get<net_paletted_container_structure_indirect>(palette)
                        ;
                        data.push_back(indirect_palette.palette().data[val]);
                    }
                    else if (
                        std::holds_alternative<net_paletted_container_structure_direct>(palette)
                    ){
                        data.push_back(val);
                    }

                    ++entry_index;
                }
            }
 
        }
    }

    void serialize(std::vector<uint8_t> &buff) const {
        bits_per_entry.serialize(buff);

        if (
            std::holds_alternative<net_paletted_container_structure_single>(palette)
        )
            std::get<net_paletted_container_structure_single>(palette).serialize(buff);
        else if (
            std::holds_alternative<net_paletted_container_structure_indirect>(palette)
        ){
            const net_paletted_container_structure_indirect &indirect_palette = 
                std::get<net_paletted_container_structure_indirect>(palette)
            ;

            indirect_palette.serialize(buff);

            size_t num_of_entries;
            if constexpr (Blocks)
                num_of_entries = 4096;
            else
                num_of_entries = 64;

            uint8_t entries_per_long = 64 / bits_per_entry;
            size_t num_longs = (num_of_entries + entries_per_long - 1) / entries_per_long;

            size_t entry_index = 0;
            for (size_t long_index = 0; long_index < num_longs; ++long_index){
                net_long l {0};
                for (uint8_t i = 0; i < entries_per_long; ++i){
                    uint8_t bit_index = entry_index % entries_per_long * bits_per_entry;

                    l = l | (uint64_t)indirect_palette.palette().data[data[entry_index]] << bit_index;
                    ++entry_index;
                }

                l.serialize(buff);
            }
        }
        /* net_paletted_cpontainer_structure_direct */
        else {
            size_t num_of_entries;
            if constexpr (Blocks)
                num_of_entries = 4096;
            else
                num_of_entries = 64;

            uint8_t entries_per_long = 64 / bits_per_entry;
            size_t num_longs = (num_of_entries + entries_per_long - 1) / entries_per_long;

            size_t entry_index = 0;
            for (size_t long_index = 0; long_index < num_longs; ++long_index){
                net_long l {0};
                for (uint8_t i = 0; i < entries_per_long; ++i){
                    uint8_t bit_index = entry_index % entries_per_long * bits_per_entry;
                    l = l | (uint64_t)data[entry_index] << bit_index;

                    ++entry_index;
                }

                l.serialize(buff);
            }
 
        }
    }

    size_t size() const {
        size_t res = bits_per_entry.size();

        if (
            std::holds_alternative<net_paletted_container_structure_single>(palette)
        )
            res += std::get<net_paletted_container_structure_single>(palette).size();
        else if (
            std::holds_alternative<net_paletted_container_structure_indirect>(palette)
        ){
            const net_paletted_container_structure_indirect &indirect_palette =
                std::get<net_paletted_container_structure_indirect>(palette)
            ;

            res += indirect_palette.size();

            size_t num_of_entries;
            if constexpr (Blocks)
                num_of_entries = 4096;
            else
                num_of_entries = 64;

            uint8_t entries_per_long = 64 / bits_per_entry;
            size_t num_longs = (num_of_entries + entries_per_long - 1) / entries_per_long;

            res += num_longs * sizeof(uint64_t);
        }
        /* net_paletted_container_direct */
        else {
            size_t num_of_entries;
            if constexpr (Blocks)
                num_of_entries = 4096;
            else
                num_of_entries = 64;

            uint8_t entries_per_long = 64 / bits_per_entry;
            size_t num_longs = (num_of_entries + entries_per_long - 1) / entries_per_long;

            res += num_longs * sizeof(uint64_t);
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


