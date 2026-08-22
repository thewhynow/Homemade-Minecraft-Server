#pragma once
#include "errors.hpp"
#include "nbt.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

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
