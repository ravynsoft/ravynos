/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <bom/bom.h>
#include <car/Reader.h>
#include <car/Facet.h>
#include <car/Rendition.h>
#include <graphics/Image.h>
#include <graphics/Format/PNG.h>

#include <iterator>
#include <string>
#include <fstream>

#include <cassert>

#if _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

static void
rendition_dump(car::Rendition const &rendition, std::string const &path)
{
    ext::optional<car::Rendition::Data> data = rendition.data();
    if (!data) {
        fprintf(stderr, "warning: failed to get image data for rendition\n");
        return;
    }

    switch (data->format()) {
        case car::Rendition::Data::Format::JPEG:
        case car::Rendition::Data::Format::Data: {
            std::ofstream file;
            file.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
            if (file.fail()) {
                fprintf(stderr, "error: failed to open file\n");
                return;
            }
            std::copy(data->data().begin(), data->data().end(), std::ostream_iterator<char>(file));
            file.close();
            break;
        }

        case car::Rendition::Data::Format::PremultipliedGA8:
        case car::Rendition::Data::Format::PremultipliedBGRA8: {
            /* Note premultiplied *first*, as the order is reversed. */
            auto format = graphics::PixelFormat(
                (data->format() == car::Rendition::Data::Format::PremultipliedGA8 ?
                    graphics::PixelFormat::Color::Grayscale :
                    graphics::PixelFormat::Color::RGB),
                graphics::PixelFormat::Order::Reversed,
                graphics::PixelFormat::Alpha::PremultipliedFirst);

            auto image = graphics::Image(rendition.width(), rendition.height(), format, data->data());
            auto png = graphics::Format::PNG::Write(image);
            if (!png.first) {
                fprintf(stderr, "failed to encode png: %s\n", png.second.c_str());
                return;
            }

            std::ofstream file;
            file.open(path, std::ios::out | std::ios::trunc | std::ios::binary);
            if (file.fail()) {
                fprintf(stderr, "error: failed to open file\n");
                return;
            }

            std::copy(png.first->begin(), png.first->end(), std::ostream_iterator<char>(file));
            break;
        }

        default:
            fprintf(stderr, "error: unknown color format\n");
            break;
    }
}

int
main(int argc, char **argv)
{
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "error: missing input\n");
        return 1;
    }

    std::string output = ".";
    if (argc > 2) {
        output = argv[2];
    }

    struct bom_context_memory memory = bom_context_memory_file(argv[1], false, 0);
    auto bom = std::unique_ptr<struct bom_context, decltype(&bom_free)>(bom_alloc_load(memory), bom_free);
    if (bom == nullptr) {
        fprintf(stderr, "error: unable to load BOM\n");
        return 1;
    }

    bom_variable_iterate(bom.get(), [](struct bom_context *context, const char *name, int data_index, void *ctx) {
        size_t data_len;
        void *data = bom_index_get(context, data_index, &data_len);
        (void)data;

        printf("Variable: %s [%08zx]\n", name, data_len);
        return true;
    }, NULL);
    printf("\n");

    ext::optional<car::Reader> car = car::Reader::Load(std::move(bom));
    if (!car) {
        fprintf(stderr, "error: unable to load car archive\n");
        return 1;
    }

    car->dump();
    printf("\n");

    int facet_count = 0;
    int rendition_count = 0;

    car->facetIterate([&car, &facet_count, &rendition_count, output](car::Facet const &facet) {
        facet_count++;
        facet.dump();

        auto renditions = car->lookupRenditions(facet);
        for (auto const &rendition : renditions) {
            rendition.dump();
            rendition_dump(rendition, output + "/" + rendition.fileName());
            rendition_count++;
        }
    });

    printf("Found %d facets and %d renditions\n", facet_count, rendition_count);
    return 0;
}
