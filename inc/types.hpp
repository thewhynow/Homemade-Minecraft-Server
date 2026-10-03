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
