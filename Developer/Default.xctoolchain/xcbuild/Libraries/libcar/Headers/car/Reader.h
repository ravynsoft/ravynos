/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef _LIBCAR_READER_H
#define _LIBCAR_READER_H

#include <car/AttributeList.h>
#include <bom/bom.h>
#include <ext/optional>

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace car {

class Facet;
class Rendition;

/*
 * An archive within a BOM file holding facets and their renditions.
 */
class Reader {
public:
    typedef std::unique_ptr<struct bom_context, decltype(&bom_free)> unique_ptr_bom;
    typedef std::unique_ptr<struct bom_tree_context, decltype(&bom_tree_free)> unique_ptr_bom_tree;

private:
    typedef struct {
        void *key;
        size_t key_len;
        void *value;
        size_t value_len;
    } KeyValuePair;

private:
    unique_ptr_bom                                  _bom;
    ext::optional<struct car_key_format *>          _keyfmt;
    std::unordered_map<std::string, void *>         _facetValues;
    std::unordered_multimap<uint16_t, KeyValuePair> _renditionValues;

private:
    Reader(unique_ptr_bom bom);

public:
    void facetFastIterate(std::function<void(void *key, size_t key_len, void *value, size_t value_len)> const &facet) const;
    void renditionFastIterate(std::function<void(void *key, size_t key_len, void *value, size_t value_len)> const &iterator) const;

public:
    /*
     * The BOM backing this archive.
     */
    struct bom_context *bom() const
    { return _bom.get(); }

    /*
     * The key format
     */
    struct car_key_format *keyfmt() const
    { return *_keyfmt; }

    /*
     * The number of Facets read
     */
    int facetCount() const
    { return _facetValues.size(); }

    /*
     * The number of Renditions read
     */
     int renditionCount() const
     { return _renditionValues.size(); }

public:
    /*
     * Iterate all facets.
     */
    void facetIterate(std::function<void(Facet const &)> const &facet) const;

    /*
     * Iterate all renditions.
     */
    void renditionIterate(std::function<void(Rendition const &)> const &iterator) const;

public:
    /*
     * Lookup a Facet by name
     */
    ext::optional<car::Facet> lookupFacet(std::string name) const;

    /*
     * Lookup Rendition list for a Facet
     */
    std::vector<car::Rendition> lookupRenditions(Facet const &) const;

public:
    /*
     * Print debug information about the archive.
     */
    void dump() const;

public:
    /*
     * Load an existing archive from a BOM.
     */
    static ext::optional<Reader> Load(unique_ptr_bom bom);
};

}

#endif /* _LIBCAR_READER_H */
