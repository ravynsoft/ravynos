/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef _LIBCAR_FACET_H
#define _LIBCAR_FACET_H

#include <car/AttributeList.h>
#include <ext/optional>

#include <string>

namespace car {

class Reader;
class Rendition;

/*
 * A facet represents an entry in an archive, identifiy by name.
 * Facets can have multiple renditions, or versions of the facet.
 */
class Facet {
private:
    std::string   _name;
    AttributeList _attributes;

public:
    Facet(std::string const &name, AttributeList const &attributes);

public:
    /*
     * The name of the facet.
     */
    std::string const &name() const
    { return _name; }

    /*
     * The attributes associated with the facet.
     */
    AttributeList attributes() const
    { return _attributes; }

public:
    /*
     * Iterate renditions for a facet.
     */
    void renditionIterate(Reader const *archive, std::function<void(Rendition const &)> const &iterator) const;

public:
    /*
     * Serialize the rendition for writing to a file.
     */
    std::vector<uint8_t> write() const;

public:
    /*
     * Print debugging information about the facet.
     */
    void dump() const;

public:
    /*
     * Load an existing rendition matching the provided attributes.
     */
    static Facet Load(
        std::string const &name,
        struct car_facet_value const *value);

    /*
     * Create a facet with the provided name and attributes
     */
    static Facet Create(
        std::string const &name,
        AttributeList const &attributes);
};

}

#endif /* _LIBCAR_FACET_H */
