#include "inc/nbt.hpp"
#include "inc/errors.hpp"
#include <bit>
#include <memory>

nbt_type_tagged::nbt_type_tagged(std::span<uint8_t> &buff):
    tag((nbt_tag) read_be<uint8_t>(buff))
{
    uint16_t length = read_be<uint16_t>(buff);

    if (length > buff.size())
        throw unfinished_packet();

    name = std::string((const char*) buff.data(), length);
    buff = buff.subspan(length);
}

nbt_type_tagged::nbt_type_tagged(nbt_tag tag, std::string name):
    tag(tag),
    name(std::move(name))
{}

std::unique_ptr<nbt_type_tagged> nbt_type_tagged::build_object(
    std::span<uint8_t> &buff
){
    if (buff.empty())
        throw unfinished_packet();

   switch ((nbt_tag) buff[0]){
        case nbt_tag::end:
            buff = buff.subspan(1);
            return nullptr;
        case nbt_tag::int8:
            return std::make_unique<nbt_byte_tagged>(buff);
        case nbt_tag::int16:
            return std::make_unique<nbt_short_tagged>(buff);
        case nbt_tag::int32:
            return std::make_unique<nbt_int_tagged>(buff);
        case nbt_tag::int64:
            return std::make_unique<nbt_long_tagged>(buff);
        case nbt_tag::float32:
            return std::make_unique<nbt_float_tagged>(buff);
        case nbt_tag::float64:
            return std::make_unique<nbt_double_tagged>(buff);
        case nbt_tag::byte_array:
            return std::make_unique<nbt_byte_array_tagged>(buff);
        case nbt_tag::string:
            return std::make_unique<nbt_string_tagged>(buff);
        case nbt_tag::list:
            return std::make_unique<nbt_list_tagged>(buff);
        case nbt_tag::compound:
            return std::make_unique<nbt_compound_tagged>(buff);
        case nbt_tag::int_array:
            return std::make_unique<nbt_int_array_tagged>(buff);
        case nbt_tag::long_array:
            return std::make_unique<nbt_long_array_tagged>(buff);
        default:
            throw malformed_packet("bad nbt tag");
    }
}

void nbt_type_tagged::serialize(
    std::vector<uint8_t> &buff
) const {
    if (name.size() > 0xFFFF)
        throw malformed_packet("tag name exceeds 65535 bytes");

    write_be<uint8_t>(buff, (uint8_t) tag);

    write_be<uint16_t>(buff, (uint16_t) name.size());
    buff.insert(buff.end(), name.begin(), name.end());
}

size_t nbt_type_tagged::size() const {
    return sizeof(uint8_t) + sizeof(uint16_t) + name.size();
}

nbt_type_untagged::nbt_type_untagged(std::span<uint8_t> &){}

nbt_type_untagged::nbt_type_untagged(nbt_tag, std::string){}

void nbt_type_untagged::serialize(std::vector<uint8_t> &)
    const {}

std::unique_ptr<nbt_type_untagged> nbt_type_untagged::build_object(
    nbt_tag tag, std::span<uint8_t> &buff
){
    switch (tag){
        case nbt_tag::end:
            throw malformed_packet("TAG_End element in a list");
        case nbt_tag::int8:
            return std::make_unique<nbt_byte_untagged>(buff);
        case nbt_tag::int16:
            return std::make_unique<nbt_short_untagged>(buff);
        case nbt_tag::int32:
            return std::make_unique<nbt_int_untagged>(buff);
        case nbt_tag::int64:
            return std::make_unique<nbt_long_untagged>(buff);
        case nbt_tag::float32:
            return std::make_unique<nbt_float_untagged>(buff);
        case nbt_tag::float64:
            return std::make_unique<nbt_double_untagged>(buff);
        case nbt_tag::byte_array:
            return std::make_unique<nbt_byte_array_untagged>(buff);
        case nbt_tag::string:
            return std::make_unique<nbt_string_untagged>(buff);
        case nbt_tag::list:
            return std::make_unique<nbt_list_untagged>(buff);
        case nbt_tag::compound:
            return std::make_unique<nbt_compound_untagged>(buff);
        case nbt_tag::int_array:
            return std::make_unique<nbt_int_array_untagged>(buff);
        case nbt_tag::long_array:
            return std::make_unique<nbt_long_array_untagged>(buff);
        default:
            throw malformed_packet("bad nbt tag");
    }
}

size_t nbt_type_untagged::size() const {
    return 0;
}


