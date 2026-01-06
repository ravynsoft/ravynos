/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef _LIBCAR_ATTRIBUTELIST_H
#define _LIBCAR_ATTRIBUTELIST_H

/* Unfortunate, but needed for car_attribute_identifier. */
#include <car/car_format.h>
#include <ext/optional>

#include <vector>
#include <unordered_map>

namespace std {
template<> struct hash<enum car_attribute_identifier> {
    size_t operator()(enum car_attribute_identifier identifier) const
    {
        return (size_t)identifier;
    }
};
}

namespace car {

/*
 * Stores a list of attribute identifiers and values. Used as a key for
 * renditions, which are uniquely identified by their attribute list.
 */
class AttributeList {
private:
    std::unordered_map<enum car_attribute_identifier, uint16_t> _values;

public:
    AttributeList(std::unordered_map<enum car_attribute_identifier, uint16_t> const &values);

public:
    /*
     * Get the value of an attribute.
     */
    ext::optional<uint16_t> get(enum car_attribute_identifier identifier) const;

    /*
     * Set the value of an attribute. Appends the attribute if not found.
     */
    void set(enum car_attribute_identifier identifier, uint16_t value);

    /*
     * Iterate over the contents of the attribute list. Unordered.
     */
    template<typename T>
    void iterate(T iterator) const
    {
        for (auto const &entry : _values) {
            iterator(entry.first, entry.second);
        }
    }

    /*
     * The number of attributes in the list.
     */
    size_t count() const;

public:
    /*
     * Write an attribute list into an a vector of bytes using the identifier
     * order provided.
     */
    std::vector<uint8_t> write(
        size_t count,
        uint32_t const *identifiers) const;

public:
    /*
     * Print debugging information about the list.
     */
    void dump() const;

public:
    /*
     * Load an attribute list from buffers of identifiers and values.
     */
    static AttributeList Load(
        size_t count,
        uint32_t const *identifiers,
        uint16_t const *values);

    /*
     * Load an attribute list from a buffer of identifier value pairs.
     */
    static AttributeList Load(
        size_t count,
        struct car_attribute_pair const *pairs);
};

}

#endif /* _LIBCAR_ATTRIBUTELIST_H */
