/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <car/AttributeList.h>
#include <car/car_format.h>

static uint32_t KeyFormat[] = {
    car_attribute_identifier_scale,
    car_attribute_identifier_idiom,
    car_attribute_identifier_subtype,
    car_attribute_identifier_graphics_class,
    car_attribute_identifier_memory_class,
    car_attribute_identifier_size_class_horizontal,
    car_attribute_identifier_size_class_vertical,
    car_attribute_identifier_identifier,
    car_attribute_identifier_element,
    car_attribute_identifier_part,
    car_attribute_identifier_state,
    car_attribute_identifier_value,
    car_attribute_identifier_dimension1,
};
static size_t const KeyFormatCount = sizeof(KeyFormat) / sizeof(*KeyFormat);

TEST(AttributeList, DeserializeAndSerialize)
{
    uint16_t rendition_key[KeyFormatCount] = {
        1,                                        // scale
        car_attribute_identifier_idiom_value_pad, // idiom
        3,                                        // subtype
        4,                                        // graphics_class
        5,                                        // memory_class
        car_attribute_identifier_size_class_value_compact, // size_class_horizontal
        car_attribute_identifier_size_class_value_regular, // size_class_vertical
        8,                                        // identifier
        9,                                        // element
        10,                                       // part
        11,                                       // state
        12,                                       // value
        13,                                       // dimension1
    };

    /* Deserialize and verify. */
    car::AttributeList attributes = car::AttributeList::Load(KeyFormatCount, KeyFormat, rendition_key);

    ext::optional<uint16_t> scale = attributes.get(car_attribute_identifier_scale);
    ASSERT_TRUE(scale);
    EXPECT_EQ(*scale, 1);

    ext::optional<uint16_t> idiom = attributes.get(car_attribute_identifier_idiom);
    ASSERT_TRUE(idiom);
    EXPECT_EQ(*idiom, car_attribute_identifier_idiom_value_pad);

    ext::optional<uint16_t> facet_identifier = attributes.get(car_attribute_identifier_identifier);
    ASSERT_TRUE(facet_identifier);
    EXPECT_EQ(*facet_identifier, 8);

    ext::optional<uint16_t> size_class_horizontal = attributes.get(car_attribute_identifier_size_class_horizontal);
    ASSERT_TRUE(size_class_horizontal);
    EXPECT_EQ(*size_class_horizontal, car_attribute_identifier_size_class_value_compact);

    ext::optional<uint16_t> size_class_vertical = attributes.get(car_attribute_identifier_size_class_vertical);
    ASSERT_TRUE(size_class_vertical);
    EXPECT_EQ(*size_class_vertical, car_attribute_identifier_size_class_value_regular);

    /* Verify serialization matches original. */
    auto output = attributes.write(KeyFormatCount, KeyFormat);
    uint16_t *attributes_out = reinterpret_cast<uint16_t *>(output.data());
    EXPECT_TRUE(0 == memcmp(rendition_key, attributes_out, sizeof(uint16_t) * KeyFormatCount));
}

