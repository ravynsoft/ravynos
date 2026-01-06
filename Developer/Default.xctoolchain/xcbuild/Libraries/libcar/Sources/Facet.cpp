/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <car/Facet.h>
#include <car/AttributeList.h>
#include <car/Rendition.h>
#include <car/Reader.h>

#include <cstring>
#include <cstdlib>
#include <map>

using car::Facet;
using car::AttributeList;
using car::Rendition;
using car::Reader;

Facet::
Facet(std::string const &name, AttributeList const &attributes) :
    _name      (name),
    _attributes(attributes)
{
}

void Facet::
dump() const
{
    fprintf(stderr, "Facet: %s\n", _name.c_str());

    ext::optional<AttributeList> attributes = this->attributes();
    if (attributes) {
        attributes->dump();
    }
}

void Facet::
renditionIterate(Reader const *archive, std::function<void(Rendition const &)> const &iterator) const
{
    ext::optional<AttributeList> attributes = this->attributes();
    if (!attributes) {
        return;
    }

    ext::optional<uint16_t> facet_identifier = attributes->get(car_attribute_identifier_identifier);
    if (facet_identifier) {
        archive->renditionIterate([&facet_identifier, &iterator](Rendition const &rendition) {
            ext::optional<uint16_t> rendition_identifier = rendition.attributes().get(car_attribute_identifier_identifier);
            if (rendition_identifier && *rendition_identifier == *facet_identifier) {
                iterator(rendition);
            }
        });
    }
}

Facet Facet::
Load(std::string const &name, struct car_facet_value const *value)
{
    AttributeList attributes = AttributeList::Load(value->attributes_count, value->attributes);
    return Facet(name, attributes);
}

Facet Facet::
Create(std::string const &name, AttributeList const &attributes)
{
    return Facet(name, attributes);
}

std::vector<uint8_t> Facet::
write() const
{
    std::map<enum car_attribute_identifier, uint16_t> ordered_attributes;
    size_t attributes_count = _attributes.count();
    size_t facet_value_size = sizeof(struct car_facet_value) + (sizeof(struct car_attribute_pair) * attributes_count);
    std::vector<uint8_t> output = std::vector<uint8_t>(facet_value_size);
    struct car_facet_value *facet_value = reinterpret_cast<struct car_facet_value *>(output.data());
    facet_value->attributes_count = 0;

    _attributes.iterate([&ordered_attributes](enum car_attribute_identifier identifier, uint16_t value) {
        ordered_attributes[identifier] = value;
    });

    for (auto const &pair : ordered_attributes) {
        if (facet_value->attributes_count < attributes_count) {
            facet_value->attributes[facet_value->attributes_count].identifier = pair.first;
            facet_value->attributes[facet_value->attributes_count].value = pair.second;
            facet_value->attributes_count += 1;
        }
    }
    return output;
}
