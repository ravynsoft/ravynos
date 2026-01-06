/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <car/Writer.h>
#include <car/car_format.h>

#include <random>
#include <set>
#include <unordered_set>
#include <vector>

#include <ctime>
#include <cstring>

using car::Writer;
using car::Facet;
using car::Rendition;

Writer::
Writer(unique_ptr_bom bom) :
    _bom(std::move(bom))
{
}

ext::optional<Writer> Writer::
Create(unique_ptr_bom bom)
{
    return Writer(std::move(bom));
}

void Writer::
addFacet(Facet const &facet)
{
    _facets.insert({ facet.name(), facet });
}

void Writer::
addRendition(Rendition const &rendition)
{
    auto identifier = rendition.attributes().get(car_attribute_identifier_identifier);
    if (identifier != ext::nullopt) {
        _renditions.insert({ *identifier, rendition });
    }
}

void Writer::
addRendition(void *key, size_t key_len, void *value, size_t value_len)
{
    KeyValuePair kv = {key, key_len, value, value_len};
    _rawRenditions.emplace_back(kv);
}

static std::vector<enum car_attribute_identifier>
DetermineKeyFormat(
    std::unordered_map<std::string, Facet> const &facets,
    std::unordered_multimap<uint16_t, Rendition> const &renditions)
{
    std::unordered_set<enum car_attribute_identifier> format;
    auto insert = [&format](enum car_attribute_identifier identifier, uint16_t value) {
        format.insert(identifier);
    };

    for (auto const &item : facets) {
        item.second.attributes().iterate(insert);
    }

    for (auto const &item : renditions) {
        item.second.attributes().iterate(insert);
    }

    /* Sort attributes to preserve ordering. */
    auto ordered = std::set<enum car_attribute_identifier>(format.begin(), format.end());
    return std::vector<enum car_attribute_identifier>(ordered.begin(), ordered.end());
}

void Writer::
write() const
{
    /*
     * A fast write has pre-allocated space for BOM indexes
     * A baseline of 8 indexes are required: CAR Header (1), Key Format (1), FACET (2) and RENDITION (2) trees, and 2 freelist entries.
     * Each tree entry (facet or rendition) requires 2: one key index, one value index.
     */
    uint32_t facet_count = _facets.size();
    uint32_t rendition_count = _renditions.size() + _rawRenditions.size();
    uint32_t bom_index_count = 8 + facet_count * 2 + rendition_count * 2;
    bom_index_reserve(_bom.get(), bom_index_count);

    /* Write header. */
    struct car_header *header = (struct car_header *)malloc(sizeof(struct car_header));
    if (header == NULL) {
        return;
    }

    strncpy(header->magic, "RATC", 4);
    header->ui_version = 0x131; // TODO
    header->storage_version = 0xC; // TODO
    header->storage_timestamp = static_cast<uint32_t>(time(NULL)); // TODO
    header->rendition_count = 0;
    strncpy(header->file_creator, "asset catalog compiler\n", sizeof(header->file_creator));
    strncpy(header->other_creator, "version 1.0", sizeof(header->other_creator));

    std::random_device device;
    std::uniform_int_distribution<int> distribution = std::uniform_int_distribution<int>(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
    for (size_t i = 0; i < sizeof(header->uuid); i++) {
        header->uuid[i] = distribution(device);
    }

    header->associated_checksum = 0; // TODO
    header->schema_version = 4; // TODO
    header->color_space_id = 1; // TODO
    header->key_semantics = 1; // TODO

    int header_index = bom_index_add(_bom.get(), header, sizeof(struct car_header));
    bom_variable_add(_bom.get(), car_header_variable, header_index);
    free(header);

    /* Write key format. */
    struct car_key_format *keyfmt;
    size_t keyfmt_size;
    if (_keyfmt == ext::nullopt) {
      std::vector<enum car_attribute_identifier> format = DetermineKeyFormat(_facets, _renditions);
      keyfmt_size = sizeof(struct car_key_format) + (format.size() * sizeof(uint32_t));
      keyfmt = (struct car_key_format *)malloc(keyfmt_size);
      strncpy(keyfmt->magic, "tmfk", 4);
      keyfmt->reserved = 0;
      keyfmt->num_identifiers = format.size();
      for (size_t i = 0; i < format.size(); ++i) {
          keyfmt->identifier_list[i] = static_cast<uint32_t>(format[i]);
      }
    } else {
      keyfmt = *_keyfmt;
      keyfmt_size = sizeof(struct car_key_format) + (keyfmt->num_identifiers * sizeof(uint32_t));
    }

    int key_format_index = bom_index_add(_bom.get(), keyfmt, keyfmt_size);
    bom_variable_add(_bom.get(), car_key_format_variable, key_format_index);

    /* Write facets. */
    struct bom_tree_context *facets_tree_context = bom_tree_alloc_empty(_bom.get(), car_facet_keys_variable);
    bom_tree_reserve(facets_tree_context, facet_count);
    if (facets_tree_context != NULL) {
        for (auto const &item : _facets) {
            auto facet_value = item.second.write();
            bom_tree_add(
                facets_tree_context,
                reinterpret_cast<void const *>(item.first.c_str()),
                item.first.size(),
                reinterpret_cast<void const *>(facet_value.data()),
                facet_value.size());
        }
        bom_tree_free(facets_tree_context);
    }

    /* Write renditions. */
    struct bom_tree_context *renditions_tree_context = bom_tree_alloc_empty(_bom.get(), car_renditions_variable);
    bom_tree_reserve(renditions_tree_context, rendition_count);
    if (renditions_tree_context != NULL) {
        for (auto const &item : _renditions) {
            auto attributes_value = item.second.attributes().write(keyfmt->num_identifiers, keyfmt->identifier_list);
            auto rendition_value = item.second.write();
            bom_tree_add(
                renditions_tree_context,
                reinterpret_cast<void const *>(attributes_value.data()),
                attributes_value.size(),
                reinterpret_cast<void const *>(rendition_value.data()),
                rendition_value.size());
        }
        for (auto const &item : _rawRenditions) {
            bom_tree_add(
                renditions_tree_context,
                item.key,
                item.keyLength,
                item.value,
                item.valueLength);
        }
        bom_tree_free(renditions_tree_context);
    }

    /* Add freelist entries. */
    bom_free_indices_add(_bom.get(), 2);

    if (_keyfmt == ext::nullopt) {
      free(keyfmt);
    }
}
