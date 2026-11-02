#pragma once
#include "errors.hpp"
#include "nbt.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
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
        fields{Ts{buff}...}
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

#define NET_COMPOUND_FIELD(num, name)              \
    auto &name() { return get<num>(); }            \
    const auto &name() const { return get<num>(); }

template <std::derived_from<net_type>... Ts>
requires (
    sizeof...(Ts) < 64
)
struct net_masked_set : net_type {
    std::tuple<std::optional<Ts>...> fields;

                                            /* we're not going to be reading much of this packet either way */
    net_masked_set(std::span<uint8_t> &buff, uint64_t mask = 0) {
        [&]<size_t... Is>
            (std::index_sequence<Is...>)
        {
            (
                (
                    mask & (1ULL << Is)
                        ? (get<Is>() = buff, 0)
                        : (0)
                ),
                ...
            );
        }(std::index_sequence_for<Ts...>{});
    }

    net_masked_set(std::optional<Ts>... vals):
        fields(std::move(vals)...)
    {}

    template<size_t I>
    auto &get(){
        return *std::get<I>(fields);
    }

    template <size_t I>
    const auto &get() const {
        return *std::get<I>(fields);
    }

    void serialize(std::vector<uint8_t> &buff) const {
        foreach_elem_if_present(
            [&buff, this](size_t i){
                std::get<i>(fields)->serialize(buff);
            }
        );
    }

    size_t size() const {
        size_t res = 0;

        foreach_elem_if_present(
            [&res, this](size_t i){
                res += std::get<i>(fields)->size();
            }
        );

        return res;
    }

    uint64_t mask() const {
        uint64_t mask = 0;

        foreach_elem_if_present(
            [&mask](size_t i){
                mask |= 1ULL << i;
            }
        );

        return mask;
    }
private:
    template<class Func>
    constexpr void foreach_elem_if_present(Func &&func) const {
        [&]<size_t... Is> (std::index_sequence<Is...>){
            (
                [&](size_t i){
                    if (get<i>()) func(i);
                }(Is),
                ...
            );
        };
    }
};

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

/* X is checked in the constructor, not here, so that it may still be incomplete (arrays of net_slot) */
template <typename X>
struct net_prefixed_array : net_type {
    std::vector<X> data;

    net_prefixed_array(std::span<uint8_t> &buff) {
        static_assert(std::derived_from<X, net_type>);

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

/* nbt whose root may be any tag, not just a compound (text components can be a bare string tag) */
struct net_nbt_any : net_type {
    std::unique_ptr<nbt_type_untagged> data;

    net_nbt_any(std::span<uint8_t> &buff);
    net_nbt_any(std::unique_ptr<nbt_type_untagged> &&data);

    void serialize(std::vector<uint8_t> &) const;
    size_t size() const;
};

using net_text_component = net_nbt_any;

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

    bool is_single() const {
        return bits_per_entry == 0;
    }

    bool is_indirect() const {
        return
            1 <= bits_per_entry &&
            bits_per_entry <= max_indirect
        ;
    }

    bool is_direct() const {
        return max_indirect < bits_per_entry;
    }

    /* clamps a raw bit count to what the protocol allows */
    static uint8_t effective_bits(uint8_t bits){
        if (bits == 0)
            return 0;
        if (bits <= max_indirect)
            return std::max(bits, min_indirect);
        return direct_bits;
    }

    /* derives entries_per_long, num_longs & entry_mask from bits_per_entry */
    void compute_layout(){
        if (is_single()){
            entries_per_long = 0;
            num_longs = 0;
            entry_mask = 0;
            return;
        }

        entries_per_long = 64 / bits_per_entry;
        num_longs = (num_entries + entries_per_long - 1) / entries_per_long;
        entry_mask = ((uint64_t)1 << bits_per_entry) - 1;
    }

    template <size_t S>
    net_paletted_container_structure(
        const std::array<uint32_t, S> &data_arr
    ):
        bits_per_entry(0),
        palette({})
    {
        static_assert(S == num_entries);

        std::set<uint32_t> uniques {data_arr.begin(), data_arr.end()};
        bits_per_entry = effective_bits(ceil(log2(uniques.size())));
        compute_layout();

        if (is_single())
            palette = {{(int32_t) data_arr[0]}};
        else {
            if (is_indirect())
                palette = {{uniques.begin(), uniques.end()}};
            else /* if (is_direct()) */
                palette = {{}};

            data.assign(data_arr.begin(), data_arr.end());
        }
    }

    /* flattens a cube indexed [y][z][x] into protocol order */
    template <size_t N>
    static std::array<uint32_t, N * N * N> flatten(
        const std::array<std::array<std::array<uint32_t, N>, N>, N> &cube
    ){
        std::array<uint32_t, N * N * N> res;

        for (size_t y = 0; y < N; y++)
            for (size_t z = 0; z < N; z++)
                for (size_t x = 0; x < N; x++)
                    res[(y * N + z) * N + x] = cube[y][z][x];

        return res;
    }

    template <size_t N>
    net_paletted_container_structure(
        const std::array<std::array<std::array<uint32_t, N>, N>, N> &cube
    ):
        net_paletted_container_structure(flatten(cube))
    {}

    net_paletted_container_structure(std::span<uint8_t> &buff):
        bits_per_entry(buff),
        palette({})
    {
        compute_layout();

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
            for (size_t i = 0; i < num_longs; ++i){
                uint64_t l = (uint64_t)(int64_t) net_long{buff};

                for (uint8_t j = 0; j < entries_per_long; ++j){
                    if (data.size() == num_entries)
                        break;

                    uint32_t val = (l >> (j * bits_per_entry)) & entry_mask;

                    if (is_indirect()){
                        if (val >= palette.data.size())
                            throw malformed_packet();
                        data.emplace_back(palette.data[val]);
                    }
                    else /* if (is_direct()) */
                        data.emplace_back(val);
                }
            }
        }
    }

    void serialize(std::vector<uint8_t> &buff) const {
        bits_per_entry.serialize(buff);

        if (is_single()){
            palette.data[0].serialize(buff);
            return;
        }

        if (is_indirect())
            palette.serialize(buff);

        /* maps a palette ID back to its local index */
        auto encode = [&](uint32_t id) -> uint64_t {
            if (is_direct())
                return id;

            for (size_t i = 0; i < palette.data.size(); ++i)
                if ((uint32_t)(int32_t) palette.data[i] == id)
                    return i;

            return 0;
        };

        size_t entry_index = 0;
        for (size_t i = 0; i < num_longs; ++i){
            uint64_t l = 0;

            for (uint8_t j = 0; j < entries_per_long; ++j){
                if (entry_index == data.size())
                    break;

                l |= encode(data[entry_index]) << (j * bits_per_entry);
                ++entry_index;
            }

            net_long{(int64_t) l}.serialize(buff);
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

    net_bitset (const std::vector<bool> &bits);

    struct bit_ref {
        int64_t *quad;
        uint8_t bit;

        operator bool() const {
            return ((uint64_t) *quad >> bit) & 1;
        }

        bit_ref &operator= (bool value){
            if (value)
                *quad = (uint64_t) *quad | ((uint64_t) 1 << bit);
            else
                *quad = (uint64_t) *quad & ~((uint64_t) 1 << bit);

            return *this;
        }
    };

    /* grows the bitset as needed */
    bit_ref operator[] (size_t i) {
        if (data.size() <= i / 64)
            data.resize(i / 64 + 1, net_long{0});

        return bit_ref {
            &data[i / 64].value,
            (uint8_t) (i % 64)
        };
    }

    bool operator[] (size_t i) const {
        if (data.size() <= i / 64)
            return false;

        return ((uint64_t) data[i / 64].value >> (i % 64)) & 1;
    }
};

struct chunk;

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

    /**
     * treats each section's lights as sky light; sends no block light
     */
    net_light_data (
        const chunk &c
        
    );
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

struct net_teleport_flags : net_int {
    using net_int::net_int;

    enum masks {
        rel_x = 0x0001,
        rel_y = 0x0002,
        rel_z = 0x0004,
        rel_yaw = 0x0008,
        rel_pitch = 0x0010,
        rel_vel_x = 0x0020,
        rel_vel_y = 0x0040,
        rel_vel_z = 0x0080,
        rel_rotate_vel = 0x0100
    };
};

struct net_player_action_add_player :
    net_compound<
        net_string,
        net_prefixed_array<net_game_profile_property>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name);
    NET_COMPOUND_FIELD(1, properties);
};

struct net_player_action_initialize_chat_data :
    net_compound<
        net_uuid,
        net_long,
        net_prefixed_array<net_byte>,
        net_prefixed_array<net_byte>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, chat_session_id);
    NET_COMPOUND_FIELD(1, public_key_expiry);
    NET_COMPOUND_FIELD(2, encoded_public_key);
    NET_COMPOUND_FIELD(3, public_key_signature);
};

struct net_player_action_initialize_chat :
    net_compound<
        net_prefixed_optional<
            net_player_action_initialize_chat_data
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct net_player_action_update_game_mode :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, game_mode);
};

struct net_player_action_update_listed :
    net_compound<
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, listed);
};

struct net_player_action_update_latency :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    /**
     * < 0 -> not connected
     * < 150 -> 5 bars
     * < 300 -> 4 bars
     * < 600 -> 3 bars
     * < 1,000 -> 2 bars
     * >= 1,000 -> 1 bar
     */

    NET_COMPOUND_FIELD(0, ping);
};


struct net_player_action_update_display_name :
    net_compound<
        net_prefixed_optional<
            net_nbt_data
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, display_name);
};

struct net_player_action_update_priority :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, priority);
};

struct net_player_action_update_hat :
    net_compound<
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, visible);
};

struct net_player_actions :
    net_masked_set<
        net_player_action_add_player,
        net_player_action_initialize_chat,
        net_player_action_update_game_mode,
        net_player_action_update_listed,
        net_player_action_update_latency,
        net_player_action_update_display_name,
        net_player_action_update_priority,
        net_player_action_update_hat
    >
{
    using net_masked_set::net_masked_set;

    NET_COMPOUND_FIELD(0, add_player);
    NET_COMPOUND_FIELD(1, initialize_chat);
    NET_COMPOUND_FIELD(2, update_game_mode);
    NET_COMPOUND_FIELD(3, update_listed);
    NET_COMPOUND_FIELD(4, update_latency);
    NET_COMPOUND_FIELD(5, update_display_name);
    NET_COMPOUND_FIELD(6, update_priority);
    NET_COMPOUND_FIELD(7, update_hat);
};

struct net_player_info_update_player :
    net_compound<
        net_uuid,
        net_player_actions
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, uuid);
    NET_COMPOUND_FIELD(1, actions);
};


struct chunk_section;

struct net_chunk_section :
    net_compound<
        net_short,
        net_short,
        net_paletted_container_structure_blocks,
        net_paletted_container_structure_biomes
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, block_count);
    NET_COMPOUND_FIELD(1, fluid_count);
    NET_COMPOUND_FIELD(2, block_states);
    NET_COMPOUND_FIELD(3, biomes);


    net_chunk_section (
        chunk_section section
    );
};

struct net_component_custom_data :
    net_compound<
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct net_component_max_stack_size :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, max_stack_size);
};

struct net_component_max_damage :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, max_damage);
};

struct net_component_damage :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, damage);
};

struct net_component_unbreakable :
    net_compound<>
{
    using net_compound::net_compound;
};


struct net_component_custom_name :
    net_compound<
        net_text_component
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name);
};

using net_component_item_name = net_component_custom_name;

struct net_component_item_model :
    net_compound<
        net_identifier
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, model);
};

struct net_component_lore :
    net_compound<
        net_prefixed_array<net_text_component>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, lines);
};

struct net_component_rarity :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, rarity);

    enum class rarities : int {
        common   = 0,
        uncommon = 1,
        rare     = 2,
        epic     = 3
    };
};

struct net_component_enchantments_enchantment :
    net_compound<
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type_id);
    NET_COMPOUND_FIELD(1, level);
};

struct net_component_enchantments :
    net_compound<
        net_prefixed_array<net_component_enchantments_enchantment>
    >
{
    using net_compound::net_compound;


    NET_COMPOUND_FIELD(0, enchantments);
};

struct net_id_set : net_type {
    net_var_int type;
    std::optional<net_identifier> tag_name;
    std::optional<std::vector<net_var_int>> ids;

    net_id_set(std::span<uint8_t> &buff);

    void serialize(std::vector<uint8_t> &buff) const;

    size_t size() const;
};

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

#error COME_BACK
struct net_structured_component : net_type {};

struct net_exact_data_component_matcher :
    net_compound<
        net_var_int,
        net_structured_component
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type);
    NET_COMPOUND_FIELD(1, value);
};

struct net_partial_data_component_matcher :
    net_compound<
        net_var_int,
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type);
    NET_COMPOUND_FIELD(1, predicate);
};

struct net_block_predicate :
    net_compound<
        net_prefixed_optional<net_id_set>,
        net_prefixed_optional<net_prefixed_array<net_property>>,
        net_prefixed_optional<net_nbt_data>,
        net_prefixed_array<net_exact_data_component_matcher>,
        net_prefixed_array<net_partial_data_component_matcher>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, blocks);
    NET_COMPOUND_FIELD(1, properties);
    NET_COMPOUND_FIELD(2, nbt);
    NET_COMPOUND_FIELD(3, data_components);
    NET_COMPOUND_FIELD(4, partial_data_component_predicates);
};

struct net_component_can_place_on :
    net_compound<
        net_prefixed_array<net_block_predicate>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, block_predicates);
};

using net_component_can_break = net_component_can_place_on;

struct net_attribute_modifiers_attribute_modifier :
    net_compound<
        net_var_int,
        net_identifier,
        net_double,
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, attribute_id);
    NET_COMPOUND_FIELD(1, modifier_id);
    NET_COMPOUND_FIELD(2, value);
    NET_COMPOUND_FIELD(3, operation);
    NET_COMPOUND_FIELD(4, slot);

    enum class operations {
        add = 0,
        multiply_base = 1,
        multiply_total = 2
    };

    enum class slots {
        any = 0,
        main_hand = 1,
        off_hand = 2,
        hand = 3,
        feet = 4,
        legs = 5,
        chest = 6,
        head = 7,
        armor = 8,
        body = 9
    };
};

struct net_attribute_modifiers :
    net_compound<
        net_prefixed_array<net_attribute_modifiers_attribute_modifier>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, modifiers);
};

struct net_component_custom_model_data :
    net_compound<
        net_prefixed_array<net_float>,
        net_prefixed_array<net_boolean>,
        net_prefixed_array<net_string>,
        net_prefixed_array<net_int>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, floats);
    NET_COMPOUND_FIELD(1, flags);
    NET_COMPOUND_FIELD(2, strings);
    NET_COMPOUND_FIELD(3, colors);
};

struct net_component_tooltip_display :
    net_compound<
        net_boolean,
        net_prefixed_array<net_var_int>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, hide_tooltip);
    NET_COMPOUND_FIELD(1, hidden_components);
};

struct net_component_repair_cost :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, cost);
};

struct net_component_creative_slot_lock :
    net_compound<>
{
    using net_compound::net_compound;
};

struct net_component_enchantment_glint_override :
    net_compound<
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, has_glint);
};

struct net_component_intangible_projectile :
    net_compound<
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, empty);
};

struct net_component_food :
    net_compound<
        net_var_int,
        net_float,
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, nutrition);
    NET_COMPOUND_FIELD(1, saturation_modifier);
    NET_COMPOUND_FIELD(2, can_always_eat);
};

template <typename X>
requires std::is_base_of_v<net_type, X>
struct net_id_or_x : net_type {
    /* registry ID PLUS ONE if <value> not present */
    net_var_int id;
    std::optional<X> value;

    net_id_or_x(std::span<uint8_t> &buff):
        id(buff)
    {
        if (id == 0)
            value.emplace(buff);
    }

    net_id_or_x(int32_t registry_id):
        id(registry_id + 1)
    {}

    net_id_or_x(X &&value):
        id(0),
        value(std::move(value))
    {}

    void serialize(std::vector<uint8_t> &buff) const {
        id.serialize(buff);

        if (id == 0)
            value->serialize(buff);
    }

    size_t size() const {
        return id.size() + (id == 0 ? value->size() : 0);
    }
};

template <typename X>
struct net_boxed : net_type {
    std::unique_ptr<X> value;

    net_boxed(std::span<uint8_t> &buff):
        value(std::make_unique<X>(buff))
    {}

    net_boxed(X &&value):
        value(std::make_unique<X>(std::move(value)))
    {}

    void serialize(std::vector<uint8_t> &buff) const {
        value->serialize(buff);
    }

    size_t size() const {
        return value->size();
    }
};

struct net_slot;
using net_slot_box = net_boxed<net_slot>;

struct net_sound_event : net_type {
    net_identifier sound_name;
    net_boolean has_fixed_range;
    std::optional<net_float> fixed_range;

    net_sound_event(std::span<uint8_t> &buff);
    net_sound_event(std::string_view sound_name);
    net_sound_event(std::string_view sound_name, float fixed_range);

    void serialize(std::vector<uint8_t> &buff) const;

    size_t size() const;
};

struct net_dye_color : net_var_int {
    using net_var_int::net_var_int;

    enum class colors : int {
        white      = 0,
        orange     = 1,
        magenta    = 2,
        light_blue = 3,
        yellow     = 4,
        lime       = 5,
        pink       = 6,
        gray       = 7,
        light_gray = 8,
        cyan       = 9,
        purple     = 10,
        blue       = 11,
        brown      = 12,
        green      = 13,
        red        = 14,
        black      = 15
    };
};

struct net_potion_effect_detail :
    net_compound<
        net_var_int,
        net_var_int,
        net_boolean,
        net_boolean,
        net_boolean,
        net_prefixed_optional<
            net_boxed<net_potion_effect_detail>
        >
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, amplifier);
    NET_COMPOUND_FIELD(1, duration); /* -1 for infinite */
    NET_COMPOUND_FIELD(2, ambient);
    NET_COMPOUND_FIELD(3, show_particles);
    NET_COMPOUND_FIELD(4, show_icon);
    NET_COMPOUND_FIELD(5, hidden_effect);
};

struct net_potion_effect :
    net_compound<
        net_var_int,
        net_potion_effect_detail
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type_id);
    NET_COMPOUND_FIELD(1, details);
};

struct net_consume_effect_apply_effects :
    net_compound<
        net_prefixed_array<net_potion_effect>,
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, effects);
    NET_COMPOUND_FIELD(1, probability);
};

struct net_consume_effect_remove_effects :
    net_compound<
        net_id_set
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, effects);
};

struct net_consume_effect_clear_all_effects :
    net_compound<>
{
    using net_compound::net_compound;
};

struct net_consume_effect_teleport_randomly :
    net_compound<
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, diameter);
};

struct net_consume_effect_play_sound :
    net_compound<
        net_sound_event
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, sound);
};

struct net_consume_effect : net_type {
    enum class types : int {
        apply_effects     = 0,
        remove_effects    = 1,
        clear_all_effects = 2,
        teleport_randomly = 3,
        play_sound        = 4
    };

    /* the index of each alternative is its type id */
    using effect_variant = std::variant<
        net_consume_effect_apply_effects,
        net_consume_effect_remove_effects,
        net_consume_effect_clear_all_effects,
        net_consume_effect_teleport_randomly,
        net_consume_effect_play_sound
    >;

    net_var_int type;
    effect_variant effect;

    net_consume_effect(std::span<uint8_t> &buff);
    net_consume_effect(effect_variant &&effect);

    void serialize(std::vector<uint8_t> &buff) const;

    size_t size() const;
};

struct net_component_consumable :
    net_compound<
        net_float,
        net_var_int,
        net_id_or_x<net_sound_event>,
        net_boolean,
        net_prefixed_array<net_consume_effect>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, consume_seconds);
    NET_COMPOUND_FIELD(1, animation);
    NET_COMPOUND_FIELD(2, sound);
    NET_COMPOUND_FIELD(3, has_consume_particles);
    NET_COMPOUND_FIELD(4, effects);

    enum class animations : int {
        none      = 0,
        eat       = 1,
        drink     = 2,
        block     = 3,
        bow       = 4,
        spear     = 5,
        crossbow  = 6,
        spyglass  = 7,
        toot_horn = 8,
        brush     = 9
    };
};

struct net_component_use_remainder :
    net_compound<
        net_slot_box
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, remainder);
};

struct net_component_use_cooldown :
    net_compound<
        net_float,
        net_prefixed_optional<net_identifier>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, seconds);
    NET_COMPOUND_FIELD(1, cooldown_group);
};

struct net_component_use_effects :
    net_compound<
        net_boolean,
        net_boolean,
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, can_sprint);
    NET_COMPOUND_FIELD(1, interact_vibrations);
    NET_COMPOUND_FIELD(2, speed_multiplier);
};

struct net_component_minimum_attack_charge :
    net_compound<
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, minimum_attack_charge);
};

struct net_component_damage_type :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, damage_type);
};

struct net_component_damage_resistant :
    net_compound<
        net_id_set
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, types);
};

struct net_component_attack_range :
    net_compound<
        net_float,
        net_float,
        net_float,
        net_float,
        net_float,
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, min_reach);
    NET_COMPOUND_FIELD(1, max_reach);
    NET_COMPOUND_FIELD(2, min_creative_reach);
    NET_COMPOUND_FIELD(3, max_creative_reach);
    NET_COMPOUND_FIELD(4, hitbox_margin);
    NET_COMPOUND_FIELD(5, mob_factor);
};

struct net_component_tool_rule :
    net_compound<
        net_id_set,
        net_prefixed_optional<net_float>,
        net_prefixed_optional<net_boolean>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, blocks);
    NET_COMPOUND_FIELD(1, speed);
    NET_COMPOUND_FIELD(2, correct_drop_for_blocks);
};

struct net_component_tool :
    net_compound<
        net_prefixed_array<net_component_tool_rule>,
        net_float,
        net_var_int,
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, rules);
    NET_COMPOUND_FIELD(1, default_mining_speed);
    NET_COMPOUND_FIELD(2, damage_per_block);
    NET_COMPOUND_FIELD(3, can_destroy_blocks_in_creative);
};

struct net_component_weapon :
    net_compound<
        net_var_int,
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, damage_per_attack);
    NET_COMPOUND_FIELD(1, disable_blocking_for); /* in seconds */
};

struct net_component_enchantable :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, value);
};

struct net_component_equippable :
    net_compound<
        net_var_int,
        net_id_or_x<net_sound_event>,
        net_prefixed_optional<net_identifier>,
        net_prefixed_optional<net_identifier>,
        net_prefixed_optional<net_id_set>,
        net_boolean,
        net_boolean,
        net_boolean,
        net_boolean,
        net_id_or_x<net_sound_event>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, slot);
    NET_COMPOUND_FIELD(1, equip_sound);
    NET_COMPOUND_FIELD(2, model);
    NET_COMPOUND_FIELD(3, camera_overlay);
    NET_COMPOUND_FIELD(4, allowed_entities);
    NET_COMPOUND_FIELD(5, dispensable);
    NET_COMPOUND_FIELD(6, swappable);
    NET_COMPOUND_FIELD(7, damage_on_hurt);
    NET_COMPOUND_FIELD(8, can_be_sheared);
    NET_COMPOUND_FIELD(9, shearing_sound);

    enum class slots : int {
        main_hand = 0,
        feet      = 1,
        legs      = 2,
        chest     = 3,
        head      = 4,
        off_hand  = 5,
        body      = 6
    };
};

struct net_component_repairable :
    net_compound<
        net_id_set
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, items);
};

struct net_component_glider :
    net_compound<>
{
    using net_compound::net_compound;
};

struct net_component_tooltip_style :
    net_compound<
        net_identifier
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, style);
};

struct net_component_death_protection :
    net_compound<
        net_prefixed_array<net_consume_effect>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, effects);
};

struct net_component_blocks_attacks_damage_reduction :
    net_compound<
        net_float,
        net_prefixed_optional<net_id_set>,
        net_float,
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, horizontal_blocking_angle);
    NET_COMPOUND_FIELD(1, type);
    NET_COMPOUND_FIELD(2, base);
    NET_COMPOUND_FIELD(3, factor);
};

struct net_component_blocks_attacks :
    net_compound<
        net_float,
        net_float,
        net_prefixed_array<net_component_blocks_attacks_damage_reduction>,
        net_float,
        net_float,
        net_float,
        net_prefixed_optional<net_id_set>,
        net_prefixed_optional<net_id_or_x<net_sound_event>>,
        net_prefixed_optional<net_id_or_x<net_sound_event>>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, block_delay_seconds);
    NET_COMPOUND_FIELD(1, disable_cooldown_scale);
    NET_COMPOUND_FIELD(2, damage_reductions);
    NET_COMPOUND_FIELD(3, item_damage_threshold);
    NET_COMPOUND_FIELD(4, item_damage_base);
    NET_COMPOUND_FIELD(5, item_damage_factor);
    NET_COMPOUND_FIELD(6, bypassed_by);
    NET_COMPOUND_FIELD(7, block_sound);
    NET_COMPOUND_FIELD(8, disable_sound);
};

/* same layout as minecraft:enchantments */
using net_component_stored_enchantments = net_component_enchantments;

struct net_component_dyed_color :
    net_compound<
        net_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

using net_component_map_color = net_component_dyed_color;

struct net_component_map_id :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, id);
};

struct net_component_map_decorations :
    net_compound<
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct net_component_map_post_processing :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type);

    enum class types : int {
        lock  = 0,
        scale = 1
    };
};

struct net_component_charged_projectiles :
    net_compound<
        net_prefixed_array<net_slot>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, projectiles);
};

struct net_component_bundle_contents :
    net_compound<
        net_prefixed_array<net_slot>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, items);
};

struct net_component_potion_contents :
    net_compound<
        net_prefixed_optional<net_var_int>,
        net_prefixed_optional<net_int>,
        net_prefixed_array<net_potion_effect>,
        net_prefixed_optional<net_string>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, potion_id);
    NET_COMPOUND_FIELD(1, custom_color);
    NET_COMPOUND_FIELD(2, custom_effects);
    NET_COMPOUND_FIELD(3, custom_name);
};

struct net_component_potion_duration_scale :
    net_compound<
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, effect_multiplier);
};

struct net_component_suspicious_stew_effects_effect :
    net_compound<
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type_id);
    NET_COMPOUND_FIELD(1, duration);
};

struct net_component_suspicious_stew_effects :
    net_compound<
        net_prefixed_array<net_component_suspicious_stew_effects_effect>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, effects);
};

struct net_component_writable_book_content_page :
    net_compound<
        net_string,
        net_prefixed_optional<net_string>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, raw_content);
    NET_COMPOUND_FIELD(1, filtered_content);
};

struct net_component_writable_book_content :
    net_compound<
        net_prefixed_array<net_component_writable_book_content_page>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, pages);
};

struct net_component_written_book_content_page :
    net_compound<
        net_text_component,
        net_prefixed_optional<net_text_component>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, raw_content);
    NET_COMPOUND_FIELD(1, filtered_content);
};

struct net_component_written_book_content :
    net_compound<
        net_string,
        net_prefixed_optional<net_string>,
        net_string,
        net_var_int,
        net_prefixed_array<net_component_written_book_content_page>,
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, raw_title);
    NET_COMPOUND_FIELD(1, filtered_title);
    NET_COMPOUND_FIELD(2, author);
    NET_COMPOUND_FIELD(3, generation);
    NET_COMPOUND_FIELD(4, pages);
    NET_COMPOUND_FIELD(5, resolved);
};

struct net_trim_material_override :
    net_compound<
        net_identifier,
        net_string
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, armor_material_type);
    NET_COMPOUND_FIELD(1, overridden_asset_name);
};

struct net_trim_material :
    net_compound<
        net_string,
        net_prefixed_array<net_trim_material_override>,
        net_text_component
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, suffix);
    NET_COMPOUND_FIELD(1, overrides);
    NET_COMPOUND_FIELD(2, description);
};

struct net_trim_pattern :
    net_compound<
        net_string,
        net_var_int,
        net_text_component,
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, asset_name);
    NET_COMPOUND_FIELD(1, template_item);
    NET_COMPOUND_FIELD(2, description);
    NET_COMPOUND_FIELD(3, decal);
};

struct net_component_trim :
    net_compound<
        net_id_or_x<net_trim_material>,
        net_id_or_x<net_trim_pattern>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, trim_material);
    NET_COMPOUND_FIELD(1, trim_pattern);
};

struct net_component_debug_stick_state :
    net_compound<
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct net_component_entity_data :
    net_compound<
        net_var_int,
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, entity_type);
    NET_COMPOUND_FIELD(1, data);
};

struct net_component_bucket_entity_data :
    net_compound<
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct net_component_block_entity_data :
    net_compound<
        net_var_int,
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, block_entity_type);
    NET_COMPOUND_FIELD(1, data);
};

struct net_instrument :
    net_compound<
        net_id_or_x<net_sound_event>,
        net_float,
        net_float,
        net_text_component
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, sound_event);
    NET_COMPOUND_FIELD(1, use_duration);
    NET_COMPOUND_FIELD(2, range);
    NET_COMPOUND_FIELD(3, description);
};

struct net_component_instrument :
    net_compound<
        net_id_or_x<net_instrument>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, instrument);
};

struct net_component_piercing_weapon :
    net_compound<
        net_boolean,
        net_boolean,
        net_prefixed_optional<net_sound_event>,
        net_prefixed_optional<net_sound_event>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, deals_knockback);
    NET_COMPOUND_FIELD(1, dismounts);
    NET_COMPOUND_FIELD(2, sound);
    NET_COMPOUND_FIELD(3, hit_sound);
};

struct net_kinetic_weapon_conditions :
    net_compound<
        net_var_int,
        net_float,
        net_float
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, max_duration_ticks);
    NET_COMPOUND_FIELD(1, min_speed);
    NET_COMPOUND_FIELD(2, min_relative_speed);
};

struct net_component_kinetic_weapon :
    net_compound<
        net_var_int,
        net_var_int,
        net_prefixed_optional<net_kinetic_weapon_conditions>,
        net_prefixed_optional<net_kinetic_weapon_conditions>,
        net_prefixed_optional<net_kinetic_weapon_conditions>,
        net_float,
        net_float,
        net_prefixed_optional<net_sound_event>,
        net_prefixed_optional<net_sound_event>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, contact_cooldown_ticks);
    NET_COMPOUND_FIELD(1, delay_ticks);
    NET_COMPOUND_FIELD(2, dismount_conditions);
    NET_COMPOUND_FIELD(3, knockback_conditions);
    NET_COMPOUND_FIELD(4, damage_conditions);
    NET_COMPOUND_FIELD(5, forward_movement);
    NET_COMPOUND_FIELD(6, damage_multiplier);
    NET_COMPOUND_FIELD(7, sound);
    NET_COMPOUND_FIELD(8, hit_sound);
};

struct net_component_swing_animation :
    net_compound<
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type);
    NET_COMPOUND_FIELD(1, duration);

    enum class types : int {
        none  = 0,
        whack = 1,
        stab  = 2
    };
};

struct net_component_additional_trade_cost :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, additional_trade_cost);
};

struct net_component_dye :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_component_provides_trim_material :
    net_compound<
        net_id_or_x<net_trim_material>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, material);
};

struct net_component_ominous_bottle_amplifier :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, amplifier);
};

struct net_jukebox_song :
    net_compound<
        net_id_or_x<net_sound_event>,
        net_text_component,
        net_float,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, sound_event);
    NET_COMPOUND_FIELD(1, description);
    NET_COMPOUND_FIELD(2, duration);
    NET_COMPOUND_FIELD(3, output);
};

struct net_component_jukebox_playable :
    net_compound<
        net_id_or_x<net_jukebox_song>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, jukebox_song);
};

struct net_component_provides_banner_patterns :
    net_compound<
        net_id_set
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, key);
};

struct net_component_recipes :
    net_compound<
        net_nbt_any
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct net_component_lodestone_tracker : net_type {
    net_boolean has_global_position;
    std::optional<net_identifier> dimension;
    std::optional<net_position> position;
    net_boolean tracked;

    net_component_lodestone_tracker(std::span<uint8_t> &buff);

    void serialize(std::vector<uint8_t> &buff) const;

    size_t size() const;
};

struct net_firework_explosion :
    net_compound<
        net_var_int,
        net_prefixed_array<net_int>,
        net_prefixed_array<net_int>,
        net_boolean,
        net_boolean
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, shape);
    NET_COMPOUND_FIELD(1, colors);
    NET_COMPOUND_FIELD(2, fade_colors);
    NET_COMPOUND_FIELD(3, has_trail);
    NET_COMPOUND_FIELD(4, has_twinkle);

    enum class shapes : int {
        small_ball = 0,
        large_ball = 1,
        star       = 2,
        creeper    = 3,
        burst      = 4
    };
};

struct net_component_firework_explosion :
    net_compound<
        net_firework_explosion
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, explosion);
};

struct net_component_fireworks :
    net_compound<
        net_var_int,
        net_prefixed_array<net_firework_explosion>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, flight_duration);
    NET_COMPOUND_FIELD(1, explosions);
};

struct net_resolvable_profile_partial :
    net_compound<
        net_prefixed_optional<net_string>,
        net_prefixed_optional<net_uuid>,
        net_prefixed_array<net_game_profile_property>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, username);
    NET_COMPOUND_FIELD(1, uuid);
    NET_COMPOUND_FIELD(2, properties);
};

struct net_resolvable_profile : net_type {
    enum class kinds : int {
        partial  = 0,
        complete = 1
    };

    enum class models : int {
        wide = 0,
        slim = 1
    };

    /* the index of each alternative is its kind */
    using profile_variant = std::variant<
        net_resolvable_profile_partial,
        net_game_profile
    >;

    net_var_int kind;
    profile_variant profile;
    net_prefixed_optional<net_identifier> body;
    net_prefixed_optional<net_identifier> cape;
    net_prefixed_optional<net_identifier> elytra;
    net_prefixed_optional<net_var_int> model;

    net_resolvable_profile(std::span<uint8_t> &buff);

    void serialize(std::vector<uint8_t> &buff) const;

    size_t size() const;
};

struct net_component_profile :
    net_compound<
        net_resolvable_profile
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, profile);
};

struct net_component_note_block_sound :
    net_compound<
        net_identifier
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, sound);
};

struct net_banner_pattern :
    net_compound<
        net_identifier,
        net_string
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, asset_id);
    NET_COMPOUND_FIELD(1, translation_key);
};

struct net_component_banner_patterns_layer :
    net_compound<
        net_id_or_x<net_banner_pattern>,
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, pattern_type);
    NET_COMPOUND_FIELD(1, color);
};

struct net_component_banner_patterns :
    net_compound<
        net_prefixed_array<net_component_banner_patterns_layer>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, layers);
};

struct net_component_base_color :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_component_pot_decorations :
    net_compound<
        net_prefixed_array<net_var_int>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, decorations);
};

struct net_component_container :
    net_compound<
        net_prefixed_array<net_slot>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, items);
};

struct net_component_block_state_property :
    net_compound<
        net_string,
        net_string
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name);
    NET_COMPOUND_FIELD(1, value);
};

struct net_component_block_state :
    net_compound<
        net_prefixed_array<net_component_block_state_property>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, properties);
};

struct net_component_bees_bee :
    net_compound<
        net_var_int,
        net_nbt_data,
        net_var_int,
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, entity_type);
    NET_COMPOUND_FIELD(1, entity_data);
    NET_COMPOUND_FIELD(2, ticks_in_hive);
    NET_COMPOUND_FIELD(3, min_ticks_in_hive);
};

struct net_component_bees :
    net_compound<
        net_prefixed_array<net_component_bees_bee>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, bees);
};

struct net_component_lock :
    net_compound<
        net_nbt_any
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, key);
};

struct net_component_container_loot :
    net_compound<
        net_nbt_data
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, data);
};

struct net_component_break_sound :
    net_compound<
        net_id_or_x<net_sound_event>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, sound_event);
};

struct net_component_sulfur_cube_content :
    net_compound<
        net_slot_box
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, content);
};

struct net_component_villager_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_wolf_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_wolf_sound_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_wolf_collar :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_component_fox_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);

    enum class variants : int {
        red  = 0,
        snow = 1
    };
};

struct net_component_salmon_size :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, type);

    enum class types : int {
        small  = 0,
        medium = 1,
        large  = 2
    };
};

struct net_component_parrot_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);

    enum class variants : int {
        red_blue    = 0,
        blue        = 1,
        green       = 2,
        yellow_blue = 3,
        gray        = 4
    };
};

struct net_component_tropical_fish_pattern :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, pattern);

    enum class patterns : int {
        kob       = 0,
        sunstreak = 1,
        snooper   = 2,
        dasher    = 3,
        brinely   = 4,
        spotty    = 5,
        flopper   = 6,
        stripey   = 7,
        glitter   = 8,
        blockfish = 9,
        betty     = 10,
        clayfish  = 11
    };
};

struct net_component_tropical_fish_base_color :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_component_tropical_fish_pattern_color :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_component_mooshroom_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);

    enum class variants : int {
        red   = 0,
        brown = 1
    };
};

struct net_component_rabbit_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);

    enum class variants : int {
        brown           = 0,
        white           = 1,
        black           = 2,
        white_splotched = 3,
        gold            = 4,
        salt            = 5,
        evil            = 6
    };
};

struct net_component_pig_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_pig_sound_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_cow_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_cow_sound_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_chicken_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_chicken_sound_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_frog_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_horse_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);

    enum class variants : int {
        white      = 0,
        creamy     = 1,
        chestnut   = 2,
        brown      = 3,
        black      = 4,
        gray       = 5,
        dark_brown = 6
    };
};

struct net_painting_variant :
    net_compound<
        net_int,
        net_int,
        net_identifier,
        net_prefixed_optional<net_text_component>,
        net_prefixed_optional<net_text_component>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, width);
    NET_COMPOUND_FIELD(1, height);
    NET_COMPOUND_FIELD(2, asset_id);
    NET_COMPOUND_FIELD(3, title);
    NET_COMPOUND_FIELD(4, author);
};

struct net_component_painting_variant :
    net_compound<
        net_id_or_x<net_painting_variant>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_llama_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);

    enum class variants : int {
        creamy = 0,
        white  = 1,
        brown  = 2,
        gray   = 3
    };
};

struct net_component_axolotl_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);

    enum class variants : int {
        lucy = 0,
        wild = 1,
        gold = 2,
        cyan = 3,
        blue = 4
    };
};

struct net_component_zombie_nautilus_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_cat_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_cat_sound_variant :
    net_compound<
        net_var_int
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, variant);
};

struct net_component_cat_collar :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_component_sheep_color :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_component_shulker_color :
    net_compound<
        net_dye_color
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, color);
};

struct net_slot :
    net_type
{
    using net_type::net_type;

    net_var_int item_count;
    std::optional<net_var_int> item_id;
    std::optional<net_var_int> num_add;
    std::optional<net_var_int> num_sub;


};
