/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <gtest/gtest.h>
#include <bom/bom_format.h>
#include <bom/bom.h>
#include <car/car_format.h>
#include <car/Facet.h>
#include <car/AttributeList.h>
#include <car/Rendition.h>
#include <car/Facet.h>
#include <car/Writer.h>
#include <car/Reader.h>

#include <cstdio>
#include <string>

#include <vector>

// Test pattern as raw pixed data, in PremultipliedBGRA8 format
static std::vector<uint8_t> test_pixels = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x7f, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

TEST(Writer, TestWriter)
{
    int width = 8;
    int height = 8;

    /* Write out. */
    auto writer_bom = car::Writer::unique_ptr_bom(bom_alloc_empty(bom_context_memory(NULL, 0)), bom_free);
    EXPECT_NE(writer_bom, nullptr);

    auto writer = car::Writer::Create(std::move(writer_bom));
    EXPECT_NE(writer, ext::nullopt);

    car::AttributeList attributes = car::AttributeList({
        { car_attribute_identifier_idiom, car_attribute_identifier_idiom_value_universal },
        { car_attribute_identifier_scale, 2 },
        { car_attribute_identifier_identifier, 1 },
    });

    car::Facet facet = car::Facet::Create("testpattern", attributes);
    writer->addFacet(facet);

    car::Rendition::Data::Format format = car::Rendition::Data::Format::PremultipliedBGRA8;
    auto data = car::Rendition::Data(test_pixels, format);
    car::Rendition rendition = car::Rendition::Create(attributes, data);
    rendition.width() = width;
    rendition.height() = height;
    rendition.scale() = 2;
    rendition.fileName() = "testpattern.png";
    rendition.layout() = car_rendition_value_layout_one_part_scale;
    writer->addRendition(rendition);

    writer->write();

    /* Read back. */
    struct bom_context_memory const *writer_memory = bom_memory(writer->bom());
    struct bom_context_memory reader_memory = bom_context_memory(writer_memory->data, writer_memory->size);
    auto reader_bom = std::unique_ptr<struct bom_context, decltype(&bom_free)>(bom_alloc_load(reader_memory), bom_free);
    EXPECT_NE(reader_bom, nullptr);

    ext::optional<car::Reader> reader = car::Reader::Load(std::move(reader_bom));
    EXPECT_NE(reader, ext::nullopt);

    int facet_count = 0;
    int rendition_count = 0;

    reader->facetIterate([&reader, &facet_count, &rendition_count](car::Facet const &facet) {
        facet_count++;

        EXPECT_EQ(facet.name(), "testpattern");

        auto renditions = reader->lookupRenditions(facet);
        for (auto const &rendition : renditions) {
            rendition_count++;

            auto data = rendition.data()->data();
            EXPECT_EQ(data, test_pixels);
        }
    });

    EXPECT_EQ(facet_count, 1);
    EXPECT_EQ(rendition_count, 1);
}


TEST(Writer, TestWriter100)
{
    int width = 8;
    int height = 8;
    int create_facet_count = 100;
    int create_scales_count = 3;
    int create_rendition_count = create_facet_count * create_scales_count;

    /* Write out. */
    /* Write with half the required number of pre-allocated indexes */
    int preallocated_index_count = 6 + create_facet_count * 2 + create_rendition_count * 2;
    auto writer_bom = car::Writer::unique_ptr_bom(bom_alloc_empty(bom_context_memory(NULL, 0)), bom_free);
    bom_index_reserve(writer_bom.get(), preallocated_index_count / 2);
    EXPECT_NE(writer_bom, nullptr);

    auto writer = car::Writer::Create(std::move(writer_bom));
    EXPECT_NE(writer, ext::nullopt);

    for (int facet_identifier = 1; facet_identifier <= create_facet_count; facet_identifier++) {
      car::AttributeList attributes = car::AttributeList({
          { car_attribute_identifier_idiom, car_attribute_identifier_idiom_value_universal },
          { car_attribute_identifier_scale, 2 },
          { car_attribute_identifier_identifier, facet_identifier },
      });

      car::Facet facet = car::Facet::Create("testpattern_" + std::to_string(facet_identifier), attributes);
      writer->addFacet(facet);

      for (int scale = 1; scale <= create_scales_count; scale++) {
        auto data = car::Rendition::Data(test_pixels, car::Rendition::Data::Format::PremultipliedBGRA8);
        car::Rendition rendition = car::Rendition::Create(attributes, data);
        rendition.width() = width;
        rendition.height() = height;
        rendition.scale() = static_cast<double>(scale);
        rendition.fileName() = "testpattern_" + std::to_string(facet_identifier) + "@" + std::to_string(scale) + "x.png";
        rendition.layout() = car_rendition_value_layout_one_part_scale;
        writer->addRendition(rendition);
      }
    }

    writer->write();

    /* Read back. */
    struct bom_context_memory const *writer_memory = bom_memory(writer->bom());
    struct bom_context_memory reader_memory = bom_context_memory(writer_memory->data, writer_memory->size);
    auto reader_bom = std::unique_ptr<struct bom_context, decltype(&bom_free)>(bom_alloc_load(reader_memory), bom_free);
    EXPECT_NE(reader_bom, nullptr);

    ext::optional<car::Reader> reader = car::Reader::Load(std::move(reader_bom));
    EXPECT_NE(reader, ext::nullopt);

    int facet_count = 0;
    int rendition_count = 0;

    reader->facetIterate([&reader, &facet_count, &rendition_count](car::Facet const &facet) {
        facet_count++;

        ext::optional<uint16_t> facet_identifier = facet.attributes().get(car_attribute_identifier_identifier);
        EXPECT_FALSE(facet_identifier == ext::nullopt);
        EXPECT_EQ(facet.name(), "testpattern_" + std::to_string(*facet_identifier));

        auto renditions = reader->lookupRenditions(facet);
        for (auto const &rendition : renditions) {
            rendition_count++;

            auto data = rendition.data()->data();
            EXPECT_EQ(data, test_pixels);
            std::string fileName = "testpattern_" + std::to_string(*facet_identifier) + "@" + std::to_string((int)rendition.scale()) + "x.png";
            EXPECT_TRUE(strcmp(rendition.fileName().c_str(), fileName.c_str()) == 0);
        }
    });

    EXPECT_EQ(facet_count, create_facet_count);
    EXPECT_EQ(rendition_count, create_rendition_count);
}

TEST(Writer, TestWriter100Optimal)
{
    int width = 8;
    int height = 8;
    int create_facet_count = 100;
    int create_scales_count = 3;
    int create_rendition_count = create_facet_count * create_scales_count;

    /* Write out.
     * A fast write has pre-allocated space for BOM indexes
     * A baseline of 6 indexes are required: CAR Header (1), Key Format (1), and FACET (2) and RENDITION (2) trees
     * Each tree entry (facet or rendition) requires 2: one key index, one value index.
     */
    unsigned int preallocated_index_count = 6 + create_facet_count * 2 + create_rendition_count * 2;
    auto writer_bom = car::Writer::unique_ptr_bom(bom_alloc_empty(bom_context_memory(NULL, 0)), bom_free);
    bom_index_reserve(writer_bom.get(), preallocated_index_count);
    EXPECT_NE(writer_bom, nullptr);

    auto writer = car::Writer::Create(std::move(writer_bom));
    EXPECT_NE(writer, ext::nullopt);

    for (int facet_identifier = 1; facet_identifier <= create_facet_count; facet_identifier++) {
      car::AttributeList attributes = car::AttributeList({
          { car_attribute_identifier_idiom, car_attribute_identifier_idiom_value_universal },
          { car_attribute_identifier_scale, 2 },
          { car_attribute_identifier_identifier, facet_identifier },
      });

      car::Facet facet = car::Facet::Create("testpattern_" + std::to_string(facet_identifier), attributes);
      writer->addFacet(facet);

      for (int scale = 1; scale <= create_scales_count; scale++) {
        auto data = car::Rendition::Data(test_pixels, car::Rendition::Data::Format::PremultipliedBGRA8);
        car::Rendition rendition = car::Rendition::Create(attributes, data);
        rendition.width() = width;
        rendition.height() = height;
        rendition.scale() = static_cast<double>(scale);
        rendition.fileName() = "testpattern_" + std::to_string(facet_identifier) + "@" + std::to_string(scale) + "x.png";
        rendition.layout() = car_rendition_value_layout_one_part_scale;
        writer->addRendition(rendition);
      }
    }

    writer->write();

    /* Read back. */
    struct bom_context_memory const *writer_memory = bom_memory(writer->bom());
    struct bom_context_memory reader_memory = bom_context_memory(writer_memory->data, writer_memory->size);
    auto reader_bom = std::unique_ptr<struct bom_context, decltype(&bom_free)>(bom_alloc_load(reader_memory), bom_free);
    EXPECT_NE(reader_bom, nullptr);

    ext::optional<car::Reader> reader = car::Reader::Load(std::move(reader_bom));
    EXPECT_NE(reader, ext::nullopt);

    int facet_count = 0;
    int rendition_count = 0;

    reader->facetIterate([&reader, &facet_count, &rendition_count](car::Facet const &facet) {
        facet_count++;

        ext::optional<uint16_t> facet_identifier = facet.attributes().get(car_attribute_identifier_identifier);
        EXPECT_FALSE(facet_identifier == ext::nullopt);
        EXPECT_EQ(facet.name(), "testpattern_" + std::to_string(*facet_identifier));

        auto renditions = reader->lookupRenditions(facet);
        for (auto const &rendition : renditions) {
            rendition_count++;

            auto data = rendition.data()->data();
            EXPECT_EQ(data, test_pixels);
            std::string fileName = "testpattern_" + std::to_string(*facet_identifier) + "@" + std::to_string((int)rendition.scale()) + "x.png";
            EXPECT_TRUE(strcmp(rendition.fileName().c_str(), fileName.c_str()) == 0);
        }
    });

    EXPECT_EQ(facet_count, create_facet_count);
    EXPECT_EQ(rendition_count, create_rendition_count);
}

