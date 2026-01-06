/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <plist/Format/Binary.h>
#include <plist/Format/ABPContext.h>
#include <plist/Format/ABPReader.h>
#include <plist/Format/ABPWriter.h>
#include <plist/Objects.h>

#include <cerrno>
#include <cstring>

using plist::Format::Type;
using plist::Format::Format;
using plist::Format::Binary;
using plist::Object;

Binary::
Binary()
{
}

Type Binary::
FormatType()
{
    return Type::Binary;
}

namespace plist { namespace Format {

template<>
std::unique_ptr<Binary> Format<Binary>::
Identify(std::vector<uint8_t> const &contents)
{
    size_t length = strlen(ABPLIST_MAGIC ABPLIST_VERSION);

    if (contents.size() < length) {
        return nullptr;
    }

    if (std::memcmp(contents.data(), ABPLIST_MAGIC ABPLIST_VERSION, length) == 0) {
        return std::unique_ptr<Binary>(new Binary(Binary::Create()));
    }

    return nullptr;
}

template<>
std::pair<std::unique_ptr<Object>, std::string> Format<Binary>::
Deserialize(std::vector<uint8_t> const &contents, Binary const &format)
{
    ABPReader reader = ABPReader(&contents);

    std::unique_ptr<Object> object = nullptr;
    if (reader.open()) {
        Object *topObject = reader.readTopLevelObject();
        if (topObject != nullptr) {
            object = topObject->copy();
        }
        reader.close();
    }

    return std::make_pair(std::move(object), reader.error());
}

template<>
std::pair<std::unique_ptr<std::vector<uint8_t>>, std::string> Format<Binary>::
Serialize(Object const *object, Binary const &format)
{
    bool success;

    auto contents = std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>());
    ABPWriter writer = ABPWriter(contents.get());

    success = writer.open();
    if (!success) {
        return std::make_pair(nullptr, "open failed");
    }

    success = writer.writeTopLevelObject(object);
    if (!success) {
        return std::make_pair(nullptr, "write failed");
    }

    success = writer.finalize();
    if (!success) {
        return std::make_pair(nullptr, "finalize failed");
    }

    success = writer.close();
    if (!success) {
        return std::make_pair(nullptr, "close failed");
    }

    return std::make_pair(std::move(contents), std::string());
}

} }

Binary Binary::
Create()
{
    return Binary();
}
