/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <car/Facet.h>
#include <car/AttributeList.h>
#include <car/car_format.h>

static struct car_facet_value *
TestFacetValue(size_t *size)
{
    static struct car_attribute_pair const attributes[] = {
        { car_attribute_identifier_element, 85 },
        { car_attribute_identifier_part, 181 },
        { car_attribute_identifier_identifier, 4258 },
    };

    static struct car_facet_value *facet_value = nullptr;
    static size_t const facet_value_size = sizeof(*facet_value) + sizeof(attributes);

    if (facet_value == nullptr) {
        facet_value = static_cast<car_facet_value *>(malloc(facet_value_size));
        facet_value->hot_spot = { 0, 0 };
        facet_value->attributes_count = sizeof(attributes) / sizeof(*attributes);
        memcpy(&facet_value->attributes, &attributes, sizeof(attributes));
    }

    if (size != nullptr) {
        *size = facet_value_size;
    }
    return facet_value;
}

TEST(Facet, Deserialize)
{
    auto facet = car::Facet::Load("name", TestFacetValue(nullptr));
    EXPECT_EQ(facet.name(), "name");
    EXPECT_EQ(*facet.attributes().get(car_attribute_identifier_element), 85);
    EXPECT_EQ(*facet.attributes().get(car_attribute_identifier_part), 181);
    EXPECT_EQ(*facet.attributes().get(car_attribute_identifier_identifier), 4258);
}

TEST(Facet, Serialize)
{
    size_t test_facet_value_size = 0;
    uint8_t *test_facet_value = reinterpret_cast<uint8_t *>(TestFacetValue(&test_facet_value_size));
    std::vector<uint8_t> test_facet_value_vector(test_facet_value, test_facet_value + test_facet_value_size);

    car::AttributeList attributes = car::AttributeList({
        { car_attribute_identifier_element, 85 },
        { car_attribute_identifier_part, 181 },
        { car_attribute_identifier_identifier, 4258 },
    });

    car::Facet facet = car::Facet::Create("name", attributes);
    auto facet_value = facet.write();

    EXPECT_EQ(facet_value, test_facet_value_vector);
}

