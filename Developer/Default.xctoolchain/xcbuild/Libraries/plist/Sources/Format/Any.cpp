/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/Any.h>

using plist::Format::Format;
using plist::Format::Any;
using plist::Format::Type;
using plist::Object;

Any::
Any(Type type, Contents const &contents) :
    _type(type)
{
    switch (_type) {
        case Type::Binary:
            _contents.binary = contents.binary;
            break;
        case Type::XML:
            _contents.xml = contents.xml;
            break;
        case Type::ASCII:
            _contents.ascii = contents.ascii;
            break;
    }
}

Any::
Any(Any const &any) :
    Any(any._type, any._contents)
{
}

Any::
~Any()
{
    switch (_type) {
        case Type::Binary:
            _contents.binary.~Binary();
            break;
        case Type::XML:
            _contents.xml.~XML();
            break;
        case Type::ASCII:
            _contents.ascii.~ASCII();
            break;
    }
}

namespace plist { namespace Format {

template<typename T>
static std::unique_ptr<Any>
IdentifyImpl(std::vector<uint8_t> const &contents)
{
    std::unique_ptr<T> format = T::Identify(contents);
    if (format != nullptr) {
        return std::unique_ptr<Any>(new Any(Any::Create<T>(*format)));
    }

    return nullptr;
}

template<>
std::unique_ptr<Any> Format<Any>::
Identify(std::vector<uint8_t> const &contents)
{
#define FORMAT(T) \
    { \
        std::unique_ptr<Any> result = IdentifyImpl<T>(contents); \
        if (result != nullptr) { \
            return result; \
        } \
    }

    FORMAT(Binary);
    FORMAT(XML);
    FORMAT(ASCII);

#undef FORMAT

    return nullptr;
}

template<typename T>
static std::pair<std::unique_ptr<Object>, std::string>
DeserializeImpl(std::vector<uint8_t> const &contents, Any const &format)
{
    return T::Deserialize(contents, *format.format<T>());
}

template<>
std::pair<std::unique_ptr<Object>, std::string> Format<Any>::
Deserialize(std::vector<uint8_t> const &contents, Any const &format)
{
    switch (format.type()) {
        case Type::Binary:
            return DeserializeImpl<Binary>(contents, format);
        case Type::XML:
            return DeserializeImpl<XML>(contents, format);
        case Type::ASCII:
            return DeserializeImpl<ASCII>(contents, format);
    }

    abort();
}

template<typename T>
static std::pair<std::unique_ptr<std::vector<uint8_t>>, std::string>
SerializeImpl(Object const *object, Any const &format)
{
    return T::Serialize(object, *format.format<T>());
}

template<>
std::pair<std::unique_ptr<std::vector<uint8_t>>, std::string> Format<Any>::
Serialize(Object const *object, Any const &format)
{
    if (object == nullptr) {
        return std::make_pair(std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>()), "invalid object to serialize");
    }

    switch (format.type()) {
        case Type::Binary:
            return SerializeImpl<Binary>(object, format);
        case Type::XML:
            return SerializeImpl<XML>(object, format);
        case Type::ASCII:
            return SerializeImpl<ASCII>(object, format);
    }

    abort();
}

} }
