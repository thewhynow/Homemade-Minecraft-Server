#pragma once
#include "types.hpp"
#include "errors.hpp"
#include <vector>
#include <span>
#include <memory>
#include <concepts>
#include <algorithm>

enum class nbt_tag : uint8_t {
    end        = 0,
    int8       = 1,
    int16      = 2,
    int32      = 3,
    int64      = 4,
    float32    = 5,
    float64    = 6,
    byte_array = 7,
    string     = 8,
    list       = 9,
    compound   = 10,
    int_array  = 11,
    long_array = 12
};

struct nbt_type {
    virtual ~nbt_type() = default;

    virtual void serialize (std::vector<uint8_t> & buff) const = 0;
    virtual size_t size() const = 0;

    virtual nbt_tag get_tag() const = 0;
};

struct nbt_type_tagged : public nbt_type {
    nbt_tag tag;
    std::string name;

    nbt_type_tagged(std::span<uint8_t> &buff);
    nbt_type_tagged(nbt_tag tag, std::string name = {});

    static std::unique_ptr<nbt_type_tagged> build_object(
        std::span<uint8_t> &buff
    );

    void serialize (std::vector<uint8_t> &buff) const override;
    size_t size() const override;
};


/* empty but here for compat reasons */
struct nbt_type_untagged : public nbt_type {
    nbt_type_untagged(std::span<uint8_t> &buff);

    nbt_type_untagged(nbt_tag tag = {}, std::string name = {});

    static std::unique_ptr<nbt_type_untagged> build_object(
        nbt_tag tag, std::span<uint8_t> &buff
    );

    void serialize (std::vector<uint8_t> &buff) const override;
    size_t size() const override;
};

template <typename B, nbt_tag tag, typename R>
requires
    std::same_as<B, nbt_type_tagged> ||
    std::same_as<B, nbt_type_untagged>
struct nbt_simple_type : B {
    using base_nbt_type = B;
    using base_type = R;

    base_type payload;

    nbt_simple_type(
        std::span<uint8_t> &buff
    ):
        base_nbt_type(buff),
        payload(read_be<base_type>(buff))
    {}

    /**
     * the name is ignored by the untagged base, so both variants
     * take it and only the tagged one writes it out
     */
    nbt_simple_type(base_type value, std::string name = {}):
        base_nbt_type(tag, std::move(name)),
        payload(value)
    {}

    nbt_tag get_tag() const override {
        return tag;
    }

    operator base_type() const {
        return payload;
    }

    nbt_simple_type &operator=(base_type base){
        payload = base;
        return *this;
    }

    void serialize(
        std::vector<uint8_t> &buff
    ) const override {
        base_nbt_type::serialize(buff);
        write_be<base_type>(buff, payload);
    }

    size_t size() const override {
        return base_nbt_type::size() + sizeof payload;
    }
};

/**
 * TAG_Byte_Array, TAG_Int_Array and TAG_Long_Array: a signed
 * 32-bit element count followed by that many big-endian elements
 */
template <typename B, nbt_tag tag, typename E>
requires
    std::same_as<B, nbt_type_tagged> ||
    std::same_as<B, nbt_type_untagged>
struct nbt_array_type : B {
    using base_nbt_type = B;
    using element_type = E;

    std::vector<element_type> data;

    nbt_array_type(std::span<uint8_t> &buff):
        base_nbt_type(buff)
    {
        int32_t length = read_be<int32_t>(buff);

        if (length < 0)
            throw malformed_packet("array length is negative");

        if ((size_t) length > buff.size() / sizeof(element_type))
            throw unfinished_packet();

        data.reserve(length);

        for (int32_t i = 0; i < length; ++i)
            data.emplace_back(read_be<element_type>(buff));
    }

    nbt_array_type(
        std::vector<element_type> data, std::string name = {}
    ):
        base_nbt_type(tag, std::move(name)),
        data(std::move(data))
    {}

    nbt_tag get_tag() const override {
        return tag;
    }

    void serialize(std::vector<uint8_t> &buff) const override {
        base_nbt_type::serialize(buff);

        buff.reserve(
            buff.size() +
            sizeof(int32_t) +
            data.size() * sizeof(element_type)
        );

        write_be<int32_t>(buff, (int32_t) data.size());

        for (element_type i : data)
            write_be(buff, i);
    }

    size_t size() const override {
        return
            base_nbt_type::size() +
            sizeof(int32_t) +
            data.size() * sizeof(element_type)
        ;
    }
};

template <typename B>
requires
    std::same_as<B, nbt_type_tagged> ||
    std::same_as<B, nbt_type_untagged>
struct nbt_string_type : B {
    using base_nbt_type = B;

    std::string data;

    nbt_string_type(std::span<uint8_t> &buff):
        base_nbt_type(buff)
    {
        uint16_t length = read_be<uint16_t>(buff);

        if (length > buff.size())
            throw unfinished_packet();

        data = std::string((const char*) buff.data(), length);
        buff = buff.subspan(length);
    }

    nbt_string_type(std::string data, std::string name = {}):
        base_nbt_type(nbt_tag::string, std::move(name)),
        data(std::move(data))
    {}

    nbt_tag get_tag() const override {
        return nbt_tag::string;
    }

    void serialize(std::vector<uint8_t> &buff) const override {
        if (data.size() > 0xFFFF)
            throw malformed_packet("string exceeds 65535 bytes");

        base_nbt_type::serialize(buff);

        buff.reserve(buff.size() + sizeof(uint16_t) + data.size());

        write_be<uint16_t>(buff, (uint16_t) data.size());
        buff.insert(buff.end(), data.begin(), data.end());
    }

    size_t size() const override {
        return base_nbt_type::size() + sizeof(uint16_t) + data.size();
    }
};

template <typename B>
requires
    std::same_as<B, nbt_type_tagged> ||
    std::same_as<B, nbt_type_untagged>
struct nbt_list_type : B {
    using base_nbt_type = B;

    nbt_tag element_tag;
    std::vector<std::unique_ptr<nbt_type_untagged>> data;

    nbt_list_type(std::span<uint8_t> &buff):
        base_nbt_type(buff),
        element_tag((nbt_tag) read_be<int8_t>(buff))
    {
        int32_t length = read_be<int32_t>(buff);

        if (length < 0)
            throw malformed_packet("list length is negative");

        if (element_tag == nbt_tag::end && length)
            throw malformed_packet("non-empty list of TAG_End");

        data.reserve(std::min<size_t>(length, buff.size()));

        for (int32_t i = 0; i < length; ++i)
            data.emplace_back(
                nbt_type_untagged::build_object(element_tag, buff)
            );
    }

    nbt_list_type(
        nbt_tag element_tag,
        std::vector<std::unique_ptr<nbt_type_untagged>> data,
        std::string name = {}
    ):
        base_nbt_type(nbt_tag::list, std::move(name)),
        element_tag(element_tag),
        data(std::move(data))
    {}

    nbt_tag get_tag() const override {
        return nbt_tag::list;
    }

    void serialize(std::vector<uint8_t> &buff) const override {
        base_nbt_type::serialize(buff);

        write_be<int8_t>(buff, (int8_t) element_tag);
        write_be<int32_t>(buff, (int32_t) data.size());

        for (const auto &i : data)
            i->serialize(buff);
    }

    size_t size() const override {
        size_t total =
            base_nbt_type::size() + sizeof(int8_t) + sizeof(int32_t);

        for (const auto &i : data)
            total += i->size();

        return total;
    }
};

template <typename B>
requires
    std::same_as<B, nbt_type_tagged> ||
    std::same_as<B, nbt_type_untagged>
struct nbt_compound_type : B {
    using base_nbt_type = B;

    std::vector<std::unique_ptr<nbt_type_tagged>> data;

    nbt_compound_type(std::span<uint8_t> &buff):
        base_nbt_type(buff)
    {
        for (
            std::unique_ptr<nbt_type_tagged> elem =
                nbt_type_tagged::build_object(buff);
            elem;
            elem = nbt_type_tagged::build_object(buff)
        )
            data.emplace_back(std::move(elem));
    }

    nbt_compound_type(
        std::vector<std::unique_ptr<nbt_type_tagged>> data,
        std::string name = {}
    ):
        base_nbt_type(nbt_tag::compound, std::move(name)),
        data(std::move(data))
    {}

    nbt_tag get_tag() const override {
        return nbt_tag::compound;
    }

    void serialize(std::vector<uint8_t> &buff) const override {
        base_nbt_type::serialize(buff);

        for (const auto &i : data)
            i->serialize(buff);

        write_be<int8_t>(buff, (int8_t) nbt_tag::end);
    }

    size_t size() const override {
        size_t total = base_nbt_type::size() + sizeof(int8_t);

        for (const auto &i : data)
            total += i->size();

        return total;
    }
};

using nbt_byte_untagged   = nbt_simple_type<nbt_type_untagged, nbt_tag::int8, int8_t>;
using nbt_byte_tagged     = nbt_simple_type<nbt_type_tagged,   nbt_tag::int8, int8_t>;

using nbt_short_untagged  = nbt_simple_type<nbt_type_untagged, nbt_tag::int16, int16_t>;
using nbt_short_tagged    = nbt_simple_type<nbt_type_tagged,   nbt_tag::int16, int16_t>;

using nbt_int_untagged    = nbt_simple_type<nbt_type_untagged, nbt_tag::int32, int32_t>;
using nbt_int_tagged      = nbt_simple_type<nbt_type_tagged,   nbt_tag::int32, int32_t>;

using nbt_long_untagged   = nbt_simple_type<nbt_type_untagged, nbt_tag::int64, int64_t>;
using nbt_long_tagged     = nbt_simple_type<nbt_type_tagged,   nbt_tag::int64, int64_t>;

using nbt_float_untagged  = nbt_simple_type<nbt_type_untagged, nbt_tag::float32, float>;
using nbt_float_tagged    = nbt_simple_type<nbt_type_tagged,   nbt_tag::float32, float>;

using nbt_double_untagged = nbt_simple_type<nbt_type_untagged, nbt_tag::float64, double>;
using nbt_double_tagged   = nbt_simple_type<nbt_type_tagged,   nbt_tag::float64, double>;


using nbt_byte_array_untagged = nbt_array_type<nbt_type_untagged, nbt_tag::byte_array, uint8_t>;
using nbt_byte_array_tagged   = nbt_array_type<nbt_type_tagged,   nbt_tag::byte_array, uint8_t>;

using nbt_int_array_untagged  = nbt_array_type<nbt_type_untagged, nbt_tag::int_array, int32_t>;
using nbt_int_array_tagged    = nbt_array_type<nbt_type_tagged,   nbt_tag::int_array, int32_t>;

using nbt_long_array_untagged = nbt_array_type<nbt_type_untagged, nbt_tag::long_array, int64_t>;
using nbt_long_array_tagged   = nbt_array_type<nbt_type_tagged,   nbt_tag::long_array, int64_t>;

using nbt_string_untagged     = nbt_string_type<nbt_type_untagged>;
using nbt_string_tagged       = nbt_string_type<nbt_type_tagged>;

using nbt_list_untagged       = nbt_list_type<nbt_type_untagged>;
using nbt_list_tagged         = nbt_list_type<nbt_type_tagged>;

using nbt_compound_untagged   = nbt_compound_type<nbt_type_untagged>;
using nbt_compound_tagged     = nbt_compound_type<nbt_type_tagged>;
