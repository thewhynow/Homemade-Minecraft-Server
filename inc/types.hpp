#pragma once
#include <cstdlib>
#include <vector>
#include <span>
#include <cstdint>
#include <cstring>
#include <string>
#include <optional>
#include <bit>
#include <type_traits>
#include <utility>
#include <algorithm>
#include "errors.hpp"
#include "utils.hpp"
#include "nbt.hpp"

struct net_type {
    void serialize (std::vector<uint8_t> &buff) const;
    size_t size() const;
};

#define DECLARE_SIMPLE_NET_TYPE(net_name, base_name)            \
    struct net_name : net_type {                                \
        base_name value;                                        \
        using base_type = base_name;                            \
                                                                \
        net_name(std::span<uint8_t> &buff);                     \
        net_name(base_type);                                    \
        net_name &operator=(base_type base);                    \
                                                                \
        operator base_type() const;                             \
        void serialize(std::vector<uint8_t> &buff) const;       \
        size_t size() const;                                    \
    }

DECLARE_SIMPLE_NET_TYPE(net_boolean, bool);
DECLARE_SIMPLE_NET_TYPE(net_byte,    int8_t);
DECLARE_SIMPLE_NET_TYPE(net_ubyte,   uint8_t);
DECLARE_SIMPLE_NET_TYPE(net_short,   int16_t);
DECLARE_SIMPLE_NET_TYPE(net_ushort,  uint16_t);
DECLARE_SIMPLE_NET_TYPE(net_int,     int32_t);
/* my own addition */
DECLARE_SIMPLE_NET_TYPE(net_uint,    uint32_t);
DECLARE_SIMPLE_NET_TYPE(net_long,    int64_t);
/* my own addition */
DECLARE_SIMPLE_NET_TYPE(net_ulong,   uint64_t);
DECLARE_SIMPLE_NET_TYPE(net_float,   float);
DECLARE_SIMPLE_NET_TYPE(net_double,  double);

/**
 * variadic template bullshit
 */
template<typename... Ts>
requires (
    (std::is_base_of_v<net_type, Ts> && ...)
)
struct net_compound : net_type {
    std::tuple<Ts...> fields;

    net_compound(std::span<uint8_t> &buff):
        fields{Ts(buff)...}
    {}

    net_compound(Ts... vals):
        fields(std::move(vals)...)
    {}

    template<size_t I>
    auto &get(){
        return std::get<I>(fields);
    }

    template<size_t I>
    const auto &get() const {
        return std::get<I>(fields);
    }

    void serialize(std::vector<uint8_t> &buff) const {
        std::apply(
            [&](auto&... field){
                (field.serialize(buff), ...);
            },
            fields
        );
    }

    size_t size() const {
        return std::apply(
            [&](auto&... field){
                return (field.size() + ... + 0);
            },
            fields
        );
    }
};

#define NET_COMPOUND_FIELD(num, name)  \
    auto &name(){ return get<num>(); } \
    const auto &name() const { return get<num>(); }

struct net_string : net_type {
    std::string value;

    net_string(
        std::span<uint8_t> &buff, size_t n = 32767
    );
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

struct net_uuid : net_compound<net_long, net_long> {
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, msq);
    NET_COMPOUND_FIELD(1, lsq);
};

template<std::derived_from<net_type> X, int N>
requires (N > 0)
struct net_array : net_type {
    std::array<X, N> data;

    net_array(std::span<uint8_t> &buff){
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

    net_prefixed_array(std::span<uint8_t> &buff){
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

    void serialize(std::vector<uint8_t> &buff) const{
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
            net_boolean(buff) ?
            std::optional<X>(std::in_place, buff) :
            std::nullopt
        )
    {}

    net_prefixed_optional(nullptr_t):
        field(std::nullopt)
    {}

    net_prefixed_optional(const X &field):
        field(field)
    {}

    net_prefixed_optional(X &&field):
        field(std::move(field))
    {}

    void serialize(std::vector<uint8_t> &buff) const {
        if (field){
            net_boolean(true).serialize(buff);
            field->serialize(buff);
        }
        else
            net_boolean(false).serialize(buff);
    }

    size_t size() const {
        return
            net_boolean(false).size() +
            (field ? field->size() : 0)
        ;
    }
};

struct net_game_profile_property : 
    net_compound<
        net_string,
        net_string,
        net_prefixed_optional<net_string>
    >
{
    using net_compound::net_compound;

    NET_COMPOUND_FIELD(0, name);
    NET_COMPOUND_FIELD(1, value);
    NET_COMPOUND_FIELD(2, signature);
};

struct net_game_profile : 
    net_compound<
        net_uuid,
        net_string,
        net_prefixed_array<net_game_profile_property>
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

struct net_select_known_packs_known_pack :
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

struct net_registry_data_entry :
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

struct net_position {
    net_long x;
    net_long z;
    net_short y;

    net_position(std::span<uint8_t> &buff);
};
