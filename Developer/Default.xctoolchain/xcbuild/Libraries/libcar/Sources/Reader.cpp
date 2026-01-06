/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <car/Reader.h>
#include <car/Facet.h>
#include <car/Rendition.h>
#include <car/car_format.h>

#include <limits>
#include <random>

#include <cassert>
#include <cstring>

using car::Reader;
using car::AttributeList;
using car::Facet;
using car::Rendition;

Reader::
Reader(unique_ptr_bom bom) :
    _bom(std::move(bom)),
    _keyfmt(ext::nullopt),
    _facetValues({ }),
    _renditionValues({ })
{
}

struct _car_iterator_ctx {
    Reader const *reader;
    void *iterator;
};

static void
_car_tree_iterator(Reader const *reader, const char *tree_variable, bom_tree_iterator tree_iterator, void *iterator)
{
    assert(reader != NULL);
    assert(iterator != NULL);

    struct bom_tree_context *tree = bom_tree_alloc_load(reader->bom(), tree_variable);
    if (tree == NULL) {
        return;
    }

    struct _car_iterator_ctx iterator_ctx = { reader, iterator };
    bom_tree_iterate(tree, tree_iterator, &iterator_ctx);
    bom_tree_free(tree);
}

void Reader::
facetIterate(std::function<void(Facet const &)> const &iterator) const
{
    for (const auto &item : _facetValues) {
        Facet facet = Facet::Load(item.first, (struct car_facet_value *)item.second);
        iterator(facet);
    }
}

static void
_car_facet_fast_iterator(struct bom_tree_context *tree, void *key, size_t key_len, void *value, size_t value_len, void *ctx)
{
    struct _car_iterator_ctx *iterator_ctx = (struct _car_iterator_ctx *)ctx;
    (*reinterpret_cast<std::function<void(void *key, size_t key_len, void *value, size_t value_len)> const *>(iterator_ctx->iterator))(key, key_len, value, value_len);
}

void Reader::
facetFastIterate(std::function<void(void *key, size_t key_len, void *value, size_t value_len)> const &iterator) const
{
    _car_tree_iterator(this, car_facet_keys_variable, _car_facet_fast_iterator, const_cast<void *>(reinterpret_cast<void const *>(&iterator)));
}

void Reader::
renditionIterate(std::function<void(Rendition const &)> const &iterator) const
{
    auto keyfmt = *_keyfmt;
    for (const auto &it : _renditionValues) {
        KeyValuePair kv = (KeyValuePair)it.second;
        car_rendition_key *rendition_key = (car_rendition_key *)kv.key;
        struct car_rendition_value *rendition_value = (struct car_rendition_value *)kv.value;
        AttributeList attributes = AttributeList::Load(keyfmt->num_identifiers, keyfmt->identifier_list, rendition_key);
        Rendition rendition = Rendition::Load(attributes, rendition_value);
        iterator(rendition);
    }
}

static void
_car_rendition_fast_iterator(struct bom_tree_context *tree, void *key, size_t key_len, void *value, size_t value_len, void *ctx)
{
    struct _car_iterator_ctx *iterator_ctx = (struct _car_iterator_ctx *)ctx;
    (*reinterpret_cast<std::function<void(void *key, size_t key_len, void *value, size_t value_len)> const *>(iterator_ctx->iterator))(key, key_len, value, value_len);
}

void Reader::
renditionFastIterate(std::function<void(void *key, size_t key_len, void *value, size_t value_len)> const &iterator) const
{
    _car_tree_iterator(this, car_renditions_variable, _car_rendition_fast_iterator, const_cast<void *>(reinterpret_cast<void const *>(&iterator)));
}

void Reader::
dump() const
{
    int header_index = bom_variable_get(_bom.get(), car_header_variable);
    struct car_header *header = (struct car_header *)bom_index_get(_bom.get(), header_index, NULL);

    printf("Magic: %.4s\n", header->magic);
    printf("UI version: %x\n", header->ui_version);
    printf("Storage version: %x\n", header->storage_version);
    printf("Storage Timestamp: %x\n", header->storage_timestamp);
    printf("Rendition Count: %x\n", header->rendition_count);
    printf("Creator: %s\n", header->file_creator);
    printf("Other Creator: %s\n", header->other_creator);
    printf("UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n", header->uuid[0], header->uuid[1], header->uuid[2], header->uuid[3], header->uuid[4], header->uuid[5], header->uuid[6], header->uuid[7], header->uuid[8], header->uuid[9], header->uuid[10], header->uuid[11], header->uuid[12], header->uuid[13], header->uuid[14], header->uuid[15]);
    printf("Associated Checksum: %x\n", header->associated_checksum);
    printf("Schema Version: %x\n", header->schema_version);
    printf("Color space ID: %x\n", header->color_space_id);
    printf("Key Semantics: %x\n", header->key_semantics);
    printf("\n");

    int extended_metadata_index = bom_variable_get(_bom.get(), car_extended_metadata_variable);
    if (extended_metadata_index != -1) {
        struct car_extended_metadata *extended = (struct car_extended_metadata *)bom_index_get(_bom.get(), extended_metadata_index, NULL);
        printf("Extended metadata: %s\n", extended->contents);
        printf("\n");
    }

    int key_format_index = bom_variable_get(_bom.get(), car_key_format_variable);
    struct car_key_format *keyfmt = (struct car_key_format *)bom_index_get(_bom.get(), key_format_index, NULL);

    printf("Key Format: %.4s\n", keyfmt->magic);
    printf("Identifier Count: %d\n", keyfmt->num_identifiers);
    for (uint32_t i = 0; i < keyfmt->num_identifiers; i++) {
        uint32_t identifier = keyfmt->identifier_list[i];
        if (identifier < sizeof(car_attribute_identifier_names) / sizeof(*car_attribute_identifier_names)) {
            printf("Identifier: %s (%d)\n", car_attribute_identifier_names[identifier] ? car_attribute_identifier_names[identifier] : "(unknown)", identifier);
        } else {
            printf("Identifier: (unknown) (%d)\n", identifier);
        }
    }
    printf("\n");

    auto part_element_iterator = [](struct bom_tree_context *tree, void *key, size_t key_len, void *value, size_t value_len, void *ctx) {
        struct car_part_element_key *part_element_key = (struct car_part_element_key *)key;
        printf("%s ID: %x\n", (char const *)ctx, part_element_key->part_element_id);

        struct car_part_element_value *part_element_value = (struct car_part_element_value *)value;
        printf("Unknown 1: %x\n", part_element_value->unknown1);
        printf("Unknown 2: %x\n", part_element_value->unknown2);
        printf("Name: %.*s\n", (int)(value_len - sizeof(struct car_part_element_value)), part_element_value->name);
        printf("\n");
    };

    struct bom_tree_context *part = bom_tree_alloc_load(_bom.get(), car_part_info_variable);
    if (part != NULL) {
        bom_tree_iterate(part, part_element_iterator, (void *)"Part");
        bom_tree_free(part);
    }

    struct bom_tree_context *element = bom_tree_alloc_load(_bom.get(), car_element_info_variable);
    if (element != NULL) {
        bom_tree_iterate(element, part_element_iterator, (void *)"Element");
        bom_tree_free(element);
    }
}

ext::optional<Reader> Reader::
Load(unique_ptr_bom bom)
{
    int header_index = bom_variable_get(bom.get(), car_header_variable);

    size_t header_len;
    struct car_header *header = (struct car_header *)bom_index_get(bom.get(), header_index, &header_len);
    if (header_len < sizeof(struct car_header) || strncmp(header->magic, "RATC", 4) || header->storage_version < 8) {
        return ext::nullopt;
    }

    auto reader = Reader(std::move(bom));

    /*
     * Iterate through the facets as fast as possible just save the name and value pointer for lookups later.
     */
    reader.facetFastIterate([&reader](void *key, size_t key_len, void *value, size_t value_len) {
        auto name = std::string(static_cast<char *>(key), key_len);
        reader._facetValues.insert({ name, value });
    });

    /* Load the key format from the BOM. */
    int key_format_index = bom_variable_get(reader.bom(), car_key_format_variable);
    struct car_key_format *keyfmt = (struct car_key_format *)bom_index_get(reader.bom(), key_format_index, NULL);
    if (!keyfmt) {
        return ext::nullopt;
    }

    reader._keyfmt = ext::optional<struct car_key_format*>(keyfmt);

    /*
     * The index into the attribute list for the identifer for the matching facet.
     * The attribute list is a list of uint16_t in the key portion of the entry for the rendition.
     */
    size_t identifier_index = 0;

    /* Scan the key format for the facet identifier index. */
    for (size_t i = 0; i < keyfmt->num_identifiers; i++) {
        if (keyfmt->identifier_list[i] == car_attribute_identifier_identifier) {
            identifier_index = i;
            break;
        }
    }

    /* Iterate through the renditions as fast as possible. Save the key and value pointers, indexed by the Facet identifier. */
    reader.renditionFastIterate([identifier_index,&reader](void *key, size_t key_len, void *value, size_t value_len) {
        KeyValuePair kv;
        kv.key = key;
        kv.key_len = key_len;
        kv.value = value;
        kv.value_len = value_len;
        car_rendition_key *rendition_key = (car_rendition_key *)key;
        reader._renditionValues.insert({ rendition_key[identifier_index], kv });
    });

    return std::move(reader);
}

ext::optional<Facet>
Reader::lookupFacet(std::string name) const
{
    ext::optional<Facet> result;

    auto lookup = _facetValues.find(name);

    if (lookup == _facetValues.end()) {
        return result;
    }

    struct car_facet_value *facet_value = (struct car_facet_value *)lookup->second;
    AttributeList attributes = AttributeList::Load(facet_value->attributes_count, facet_value->attributes);
    result = Facet::Create(name, attributes);

    return result;
}

std::vector<Rendition> Reader::
lookupRenditions(Facet const &facet) const
{
    std::vector<Rendition> result;
    ext::optional<uint16_t> facet_identifier = facet.attributes().get(car_attribute_identifier_identifier);

    if (!facet_identifier) {
        return result;
    }

    if (!_keyfmt) {
        // Expected to be ready
        return result;
    }

    auto keyfmt = *_keyfmt;
    auto lookupRendition = _renditionValues.equal_range(*facet_identifier);
    for (auto it = lookupRendition.first; it != lookupRendition.second; ++it) {
        KeyValuePair value = (KeyValuePair)it->second;
        car_rendition_key *rendition_key = (car_rendition_key *)value.key;
        struct car_rendition_value *rendition_value = (struct car_rendition_value *)value.value;
        AttributeList attributes = AttributeList::Load(keyfmt->num_identifiers, keyfmt->identifier_list, rendition_key);
        Rendition rendition = Rendition::Load(attributes, rendition_value);
        result.push_back(rendition);
    }
    return result;
}

