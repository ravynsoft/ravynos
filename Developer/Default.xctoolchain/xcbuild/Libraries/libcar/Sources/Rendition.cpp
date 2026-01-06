/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <car/Rendition.h>
#include <car/Reader.h>
#include <car/car_format.h>

#include <cassert>
#include <cstring>
#include <cstdio>

#include <zlib.h>

#if defined(__APPLE__)
#include <Availability.h>
#include <TargetConditionals.h>
#if (TARGET_OS_MAC && __MAC_10_11 && __MAC_OS_X_VERSION_MIN_REQUIRED > __MAC_10_11) || (TARGET_OS_IPHONE && __IPHONE_9_0 && __IPHONE_OS_VERSION_MIN_REQUIRED > __IPHONE_9_0)
#define HAVE_LIBCOMPRESSION 1
#endif
#endif

#if HAVE_LIBCOMPRESSION
#include <compression.h>
#define _COMPRESSION_LZVN 0x900
#endif

using car::Rendition;
using car::AttributeList;

Rendition::Data::
Data(std::vector<uint8_t> const &data, Format format) :
    _data  (data),
    _format(format)
{
}

size_t Rendition::Data::
FormatSize(Rendition::Data::Format format)
{
    switch (format) {
        case Format::PremultipliedBGRA8:
            return 4;
        case Format::PremultipliedGA8:
            return 2;
        case Format::JPEG:
        case Format::Data:
        case Format::NON_STANDARD_WEBP:
            return 1;
    }

    abort();
}

bool Rendition::Data::
FormatSavedAsRawData(Rendition::Data::Format format)
{
    switch (format) {
        case Format::PremultipliedBGRA8:
        case Format::PremultipliedGA8:
            return false;
        case Format::JPEG:
        case Format::Data:
        case Format::NON_STANDARD_WEBP:
            return true;
    }

    abort();
}

Rendition::
Rendition(AttributeList const &attributes, std::function<ext::optional<Data>(Rendition const *)> const &data) :
    _attributes  (attributes),
    _deferredData(data),
    _width       (0),
    _height      (0),
    _scale       (1.0),
    _isVector    (false),
    _isOpaque    (false),
    _isResizable (false)
{
}

Rendition::
Rendition(AttributeList const &attributes, ext::optional<Data> const &data) :
    _attributes (attributes),
    _data       (data),
    _width      (0),
    _height     (0),
    _scale      (1.0),
    _isVector   (false),
    _isOpaque   (false),
    _isResizable(false)
{
}

void Rendition::
dump() const
{
    printf("Rendition: %s\n", _fileName.c_str());
    printf("Width: %d\n", _width);
    printf("Height: %d\n", _height);
    printf("Scale: %f\n", _scale);
    printf("Layout: %d\n", _layout);

    printf("Resizable: %d\n", _isResizable);
    if (_isResizable) {
        for (size_t i = 0; i < _slices.size(); i++) {
            Slice const &slice = _slices[i];
            printf("Slice %zd: (%u, %u) %u x %u\n", i, slice.x, slice.y, slice.width, slice.height);
        }
    }

    switch (_resizeMode) {
        case ResizeMode::FixedSize:
            printf("Resize mode: Fixed Size\n");
            break;
        case ResizeMode::Tile:
            printf("Resize mode: Tile\n");
            break;
        case ResizeMode::Scale:
            printf("Resize mode: Scale\n");
            break;
        case ResizeMode::Uniform:
            printf("Resize mode: Uniform\n");
            break;
        case ResizeMode::HorizontalUniformVerticalScale:
            printf("Resize mode: Horizontal Uniform; Vertical Scale\n");
            break;
        case ResizeMode::HorizontalScaleVerticalUniform:
            printf("Resize mode: Horizontal Scale; Vertical Uniform\n");
            break;
    }

    printf("Attributes:\n");
    _attributes.dump();
}

static ext::optional<Rendition::Data> Decode(struct car_rendition_value *value);
static ext::optional<std::vector<uint8_t>> Encode(Rendition const *rendition, ext::optional<Rendition::Data> data);


static Rendition::ResizeMode
ResizeModeFromLayout(enum car_rendition_value_layout layout)
{
    switch (layout) {
        case car_rendition_value_layout_one_part_fixed_size:
        case car_rendition_value_layout_three_part_horizontal_uniform:
        case car_rendition_value_layout_three_part_vertical_uniform:
            return Rendition::ResizeMode::FixedSize;
        case car_rendition_value_layout_one_part_tile:
        case car_rendition_value_layout_three_part_horizontal_tile:
        case car_rendition_value_layout_three_part_vertical_tile:
        case car_rendition_value_layout_nine_part_tile:
            return Rendition::ResizeMode::Tile;
        case car_rendition_value_layout_one_part_scale:
        case car_rendition_value_layout_three_part_horizontal_scale:
        case car_rendition_value_layout_three_part_vertical_scale:
        case car_rendition_value_layout_nine_part_scale:
            return Rendition::ResizeMode::Scale;
        case car_rendition_value_layout_nine_part_horizontal_uniform_vertical_scale:
            return Rendition::ResizeMode::HorizontalUniformVerticalScale;
        case car_rendition_value_layout_nine_part_horizontal_scale_vertical_uniform:
            return Rendition::ResizeMode::HorizontalScaleVerticalUniform;
        case car_rendition_value_layout_nine_part_edges_only:
        case car_rendition_value_layout_six_part:
        case car_rendition_value_layout_gradient:
        case car_rendition_value_layout_effect:
        case car_rendition_value_layout_animation_filmstrip:
        case car_rendition_value_layout_raw_data:
        case car_rendition_value_layout_external_link:
        case car_rendition_value_layout_layer_stack:
        case car_rendition_value_layout_internal_link:
        case car_rendition_value_layout_asset_pack:
            break;
    }
    return Rendition::ResizeMode::FixedSize;
}

Rendition const Rendition::
Load(
    AttributeList const &attributes,
    struct car_rendition_value *value)
{
    Rendition rendition = Rendition(attributes, [value](Rendition const *rendition) -> ext::optional<Data> {
        return Decode(value);
    });

    for (struct car_rendition_info_header *info_header = (struct car_rendition_info_header *)value->info;
        ((uintptr_t)info_header - (uintptr_t)value->info) < value->info_len;
        info_header = (struct car_rendition_info_header *)((intptr_t)info_header + sizeof(struct car_rendition_info_header) + info_header->length)) {
        switch(info_header->magic) {
            case car_rendition_info_magic_slices: {
                std::vector<Slice> slices;
                struct car_rendition_info_slices *info_slices = (struct car_rendition_info_slices *)info_header;
                for (size_t i = 0; i < info_slices->nslices; i++) {
                    Slice slice = {
                        info_slices->slices[i].x,
                        info_slices->slices[i].y,
                        info_slices->slices[i].width,
                        info_slices->slices[i].height,
                    };
                    slices.push_back(slice);
                }
                rendition.slices() = slices;
                break;
            }
            case car_rendition_info_magic_metrics: {
                // Alignment options
                // struct car_rendition_info_metrics *metric = (struct car_rendition_info_metrics *)info_header;
                // metric->nmetrics
                // metric->top_right_inset.width
                // metric->top_right_inset.height
                // metric->bottom_left_inset.width
                // metric->bottom_left_inset.height
                // metric->image_size.width
                // metric->image_size.height
                break;
            }
            case car_rendition_info_magic_composition: {
                // struct car_rendition_info_composition *composition = (struct car_rendition_info_composition *)info_header;
                // composition->blend_mode
                // composition->opacity
                break;
            }
            case car_rendition_info_magic_uti: {
                struct car_rendition_info_uti *uti = (struct car_rendition_info_uti *)info_header;
                rendition.UTI() = std::string(uti->uti, uti->uti_length);
                break;
            }
            case car_rendition_info_magic_bitmap_info: {
                break;
            }
            case car_rendition_info_magic_bytes_per_row: {
                break;
            }
            case car_rendition_info_magic_reference: {
                // struct car_rendition_info_reference *reference = (struct car_rendition_info_reference *)info_header;
                // reference->x
                // reference->y
                // reference->width
                // reference->height
                // reference->layout
                // reference->key_length
                // reference->key[]
                break;
            }
            case car_rendition_info_magic_alpha_cropped_frame: {
                break;
            }
        }
    }

    /* Truncate in case string was padded to fit the name field. */
    std::string name = std::string(value->metadata.name, sizeof(value->metadata.name));
    rendition.fileName() = std::string(name.c_str());

    rendition.width() = value->width;
    rendition.height() = value->height;
    rendition.scale() = static_cast<double>(value->scale_factor) / 100.0;
    rendition.isVector() = static_cast<bool>(value->flags.is_vector);
    rendition.isOpaque() = static_cast<bool>(value->flags.is_opaque);

    enum car_rendition_value_layout layout = (enum car_rendition_value_layout)value->metadata.layout;
    rendition.layout() = layout;
    rendition.resizeMode() = ResizeModeFromLayout(layout);

    if (rendition.slices().size() == 0) {
        rendition.slices().push_back({0, 0, value->width, value->height});
    }

    if (layout >= car_rendition_value_layout_three_part_horizontal_tile &&
        layout <= car_rendition_value_layout_nine_part_horizontal_scale_vertical_uniform &&
        rendition.slices().size() > 1) {
        rendition.isResizable() = true;
    }
    return rendition;
}

ext::optional<Rendition::Data> Rendition::
data() const
{
    if(_data) {
        return _data;
    }

    if (_deferredData) {
        return _deferredData(this);
    }

    return ext::nullopt;
}

static ext::optional<Rendition::Data>
Decode(struct car_rendition_value *value)
{
    if (strncmp(value->magic, "ISTC", 4) != 0) {
        return ext::nullopt;
    }

    Rendition::Data::Format format;
    if (value->pixel_format == car_rendition_value_pixel_format_argb) {
        format = Rendition::Data::Format::PremultipliedBGRA8;
    } else if (value->pixel_format == car_rendition_value_pixel_format_ga8) {
        format = Rendition::Data::Format::PremultipliedGA8;
    } else if (value->pixel_format == car_rendition_value_pixel_format_raw_data) {
        format = Rendition::Data::Format::Data;
    } else if (value->pixel_format == car_rendition_value_pixel_format_jpeg) {
        format = Rendition::Data::Format::JPEG;
    } else if (value->pixel_format == car_rendition_value_pixel_format_non_standard_webp) {
        format = Rendition::Data::Format::NON_STANDARD_WEBP;
    } else {
        fprintf(stderr, "error: unsupported pixel format %.4s\n", (char const *)&value->pixel_format);
        return ext::nullopt;
    }

    /* Raw format (as jpeg) embeds the file within another header. */
    if (Rendition::Data::FormatSavedAsRawData(format)) {
        struct car_rendition_data_header_raw *header_raw = (struct car_rendition_data_header_raw *)((uintptr_t)value + sizeof(struct car_rendition_value) + value->info_len);
        if (strncmp(header_raw->magic, "DWAR", sizeof(header_raw->magic)) != 0) {
            fprintf(stderr, "error: raw data header magic is wrong, can't possibly decode\n");
            return ext::nullopt;
        }

        std::vector<uint8_t> contents = std::vector<uint8_t>(header_raw->data, header_raw->data + header_raw->length);
        Rendition::Data data = Rendition::Data(contents, format);
        return data;
    }

    size_t bytes_per_pixel = Rendition::Data::FormatSize(format);
    size_t uncompressed_length = value->width * value->height * bytes_per_pixel;
    Rendition::Data data = Rendition::Data(std::vector<uint8_t>(uncompressed_length), format);
    uint8_t *uncompressed_data = static_cast<uint8_t *>(data.data().data());

    /* Advance past the header and the info section. We just want the data. */
    struct car_rendition_data_header1 *header1 = (struct car_rendition_data_header1 *)((uintptr_t)value + sizeof(struct car_rendition_value) + value->info_len);

    if (strncmp(header1->magic, "MLEC", sizeof(header1->magic)) != 0) {
        fprintf(stderr, "error: header1 magic is wrong (%.4s), can't possibly decode\n", header1->magic);
        return ext::nullopt;
    }

    void *compressed_data = &header1->data;
    size_t compressed_length = header1->length;

    /* Check for the secondary header, and use its values if available. */
    /* todo find a way of determining in advance if this is present */
    struct car_rendition_data_header2 *header2 = (struct car_rendition_data_header2 *)compressed_data;
    if (strncmp(header2->magic, "KCBC", 4) == 0) {
        compressed_data = &header2->data;
        compressed_length = header2->length;
    }

    size_t offset = 0;
    while (offset < uncompressed_length) {
        if (offset != 0) {
            struct car_rendition_data_header2 *header2 = (struct car_rendition_data_header2 *)compressed_data;
            assert(strncmp(header2->magic, "KCBC", sizeof(header2->magic)) == 0);
            compressed_length = header2->length;
            compressed_data = header2->data;
        }

        if (header1->compression == car_rendition_data_compression_magic_zlib) {
            z_stream strm;
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = compressed_length;
            strm.next_in = static_cast<Bytef *>(compressed_data);

            int ret = inflateInit2(&strm, 16 + MAX_WBITS);
            if (ret != Z_OK) {
               return ext::nullopt;
            }

            strm.avail_out = uncompressed_length;
            strm.next_out = static_cast<Bytef *>(uncompressed_data);

            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END) {
                printf("error: decompression failure: %x.\n", ret);
                return ext::nullopt;
            }

            ret = inflateEnd(&strm);
            if (ret != Z_OK) {
                return ext::nullopt;
            }

            offset += (uncompressed_length - strm.avail_out);
        } else if (header1->compression == car_rendition_data_compression_magic_rle) {
            fprintf(stderr, "error: unable to handle RLE\n");
            return ext::nullopt;
        } else if (header1->compression == car_rendition_data_compression_magic_unk1) {
            fprintf(stderr, "error: unable to handle UNKNOWN\n");
            return ext::nullopt;
        } else if (header1->compression == car_rendition_data_compression_magic_lzvn || header1->compression == car_rendition_data_compression_magic_jpeg_lzfse) {
#if HAVE_LIBCOMPRESSION
            compression_algorithm algorithm = header1->compression == car_rendition_data_compression_magic_lzvn ?
                (compression_algorithm)_COMPRESSION_LZVN :
                COMPRESSION_LZFSE;

            size_t compression_result = compression_decode_buffer(uncompressed_data + offset, uncompressed_length - offset, (uint8_t *)compressed_data, compressed_length, NULL, algorithm);
            if (compression_result != 0) {
                offset += compression_result;
                compressed_data = (void *)((uintptr_t)compressed_data + compressed_length);
            } else {
                fprintf(stderr, "error: decompression failure\n");
                return ext::nullopt;
            }
#else
            if (header1->compression == car_rendition_data_compression_magic_lzvn) {
                fprintf(stderr, "error: unable to handle LZVN\n");
                return ext::nullopt;
            } else if (header1->compression == car_rendition_data_compression_magic_jpeg_lzfse) {
                fprintf(stderr, "error: unable to handle LZFSE\n");
                return ext::nullopt;
            } else {
                assert(false);
            }
#endif
        } else if (header1->compression == car_rendition_data_compression_magic_blurredimage) {
            fprintf(stderr, "error: unable to handle BlurredImage\n");
            return ext::nullopt;
        } else {
            fprintf(stderr, "error: unknown compression algorithm %x\n", header1->compression);
            return ext::nullopt;
        }
    }

    return data;
}

static ext::optional<std::vector<uint8_t>>
Encode(Rendition const *rendition, ext::optional<Rendition::Data> data)
{
    if (!data || data->data().size() == 0) {
        return ext::nullopt;
    }

    /*
     * If the format is already as required, nothing to do.
     */
    if (Rendition::Data::FormatSavedAsRawData(data->format())) {
        return data->data();
    }

    /* The selected algorithm, only zlib for now. */
    enum car_rendition_data_compression_magic compression_magic = car_rendition_data_compression_magic_zlib;
    size_t bytes_per_pixel = Rendition::Data::FormatSize(data->format());

    size_t uncompressed_length = rendition->width() * rendition->height() * bytes_per_pixel;
    void *uncompressed_data = static_cast<void *>(data->data().data());

    std::vector<uint8_t> compressed_vector;
    if (compression_magic == car_rendition_data_compression_magic_zlib) {
        z_stream zlibStream;
        memset(&zlibStream, 0, sizeof(zlibStream));
        zlibStream.next_in = static_cast<Bytef *>(uncompressed_data);
        zlibStream.avail_in = static_cast<uInt>(uncompressed_length);

        int deflateLevel = Z_DEFAULT_COMPRESSION;
        int windowSize = 16 + MAX_WBITS;
        int err = deflateInit2(&zlibStream, deflateLevel, Z_DEFLATED, windowSize, 8, Z_DEFAULT_STRATEGY);
        if (err != Z_OK) {
            return ext::nullopt;
        }

        while (true) {
            uint8_t tmp[4096];
            zlibStream.next_out = (Bytef*)&tmp;
            zlibStream.avail_out = (uInt)sizeof(tmp);
            err = deflate(&zlibStream, Z_FINISH);
            size_t block_size = sizeof(tmp) - zlibStream.avail_out;
            compressed_vector.resize(compressed_vector.size() + block_size);
            memcpy(&compressed_vector[compressed_vector.size() - block_size], &tmp[0], block_size);
            if (err == Z_STREAM_END) {  /* Done */
                break;
            }
            if (err != Z_OK) {  /* Z_OK -> Made progress, else err */
                deflateEnd(&zlibStream);
                fprintf(stderr, "Zlib error %d", err);
                return ext::nullopt;
            }
        }
        deflateEnd(&zlibStream);

        /* The gzip header includes an operating system field. For consistent results, clear it. */
        if (compressed_vector.size() > 9) {
            compressed_vector[9] = 0;
        }
    }

    std::vector<uint8_t> output = std::vector<uint8_t>(sizeof(struct car_rendition_data_header1));

    struct car_rendition_data_header1 *header1 = reinterpret_cast<struct car_rendition_data_header1 *>(output.data());
    memcpy(header1->magic, "MLEC", sizeof(header1->magic));
    header1->length = compressed_vector.size();
    header1->compression = compression_magic;
    output.insert(output.end(), compressed_vector.begin(), compressed_vector.end());

    return output;
}

Rendition Rendition::
Create(
    AttributeList const &attributes,
    std::function<ext::optional<Data>(Rendition const *)> const &data)
{
    return Rendition(attributes, data);
}

Rendition Rendition::
Create(
    AttributeList const &attributes,
    ext::optional<Data> const &data)
{
    return Rendition(attributes, data);
}

std::vector<uint8_t> Rendition::
write() const
{
    // Create header
    struct car_rendition_value header;
    memset(static_cast<void *>(&header), 0, sizeof(struct car_rendition_value));
    strncpy(header.magic, "ISTC", 4);
    header.version = 1;
    // header.flags.is_header_flagged_fpo = 0;
    // header.flags.is_excluded_from_contrast_filter = 0;
    header.flags.is_vector = _isVector;
    header.flags.is_opaque = _isOpaque;
    header.flags.bitmap_encoding = 1;
    // header.flags.reserved = 0;

    header.width = _width;
    header.height = _height;
    header.scale_factor = static_cast<uint32_t>(_scale * 100);
    header.color_space_id = 1;

    header.metadata.layout = _layout;
    strncpy(header.metadata.name, _fileName.c_str(), 128);

    /*
     * Serialize slices. There's always at least one slice.
     */
    size_t nslices = (_slices.empty() ? 1 : _slices.size());
    size_t info_slices_size = sizeof(car_rendition_info_slices) + sizeof(struct car_rendition_info_slice) * nslices;
    struct car_rendition_info_slices *info_slices = (struct car_rendition_info_slices *)malloc(info_slices_size);
    info_slices->header.magic = car_rendition_info_magic_slices;
    info_slices->header.length = info_slices_size - sizeof(car_rendition_info_header);
    info_slices->nslices = nslices;

    if (!_slices.empty()) {
        /*
         * Copy in the specified slices.
         */
        for (size_t i = 0; i < nslices; i++) {
            info_slices->slices[i].x = _slices[i].x;
            info_slices->slices[i].y = _slices[i].y;
            info_slices->slices[i].width = _slices[i].width;
            info_slices->slices[i].height = _slices[i].height;
        }
    } else {
        /*
         * No slices defined; create a single slice for the whole image.
         */
        info_slices->slices[0].x = 0;
        info_slices->slices[0].y = 0;
        info_slices->slices[0].width = _width;
        info_slices->slices[0].height = _height;
    }

    struct car_rendition_info_metrics info_metrics;
    info_metrics.header.magic = car_rendition_info_magic_metrics;
    info_metrics.header.length = sizeof(struct car_rendition_info_metrics) - sizeof(struct car_rendition_info_header);
    info_metrics.nmetrics = 1;
    info_metrics.top_right_inset.width = 0;
    info_metrics.top_right_inset.height = 0;
    info_metrics.bottom_left_inset.width = 0;
    info_metrics.bottom_left_inset.height = 0;
    info_metrics.image_size.width = _width;
    info_metrics.image_size.height = _height;

    struct car_rendition_info_composition info_composition;
    info_composition.header.magic = car_rendition_info_magic_composition;
    info_composition.header.length = sizeof(struct car_rendition_info_composition) - sizeof(struct car_rendition_info_header);
    info_composition.blend_mode = 0;
    info_composition.opacity = 1;

    struct car_rendition_info_bitmap_info info_bitmap_info;
    info_bitmap_info.header.magic = car_rendition_info_magic_bitmap_info;
    info_bitmap_info.header.length = sizeof(struct car_rendition_info_bitmap_info) - sizeof(struct car_rendition_info_header);
    info_bitmap_info.exif_orientation = 1; // XXX FIXME

    size_t bytes_per_pixel = 0;
    auto renditionData = this->data();
    switch (renditionData->format()) {
        case Rendition::Data::Format::PremultipliedBGRA8:
            bytes_per_pixel = 4;
            header.pixel_format = car_rendition_value_pixel_format_argb;
            break;
        case Rendition::Data::Format::PremultipliedGA8:
            bytes_per_pixel = 2;
            header.pixel_format = car_rendition_value_pixel_format_ga8;
            break;
        case Rendition::Data::Format::JPEG:
            header.pixel_format = car_rendition_value_pixel_format_jpeg;
            break;
        case Rendition::Data::Format::Data:
            header.pixel_format = car_rendition_value_pixel_format_raw_data;
            break;
        case Rendition::Data::Format::NON_STANDARD_WEBP:
            header.pixel_format = car_rendition_value_pixel_format_non_standard_webp;
            break;
    }

    struct car_rendition_info_bytes_per_row info_bytes_per_row;
    info_bytes_per_row.header.magic = car_rendition_info_magic_bytes_per_row;
    info_bytes_per_row.header.length = sizeof(struct car_rendition_info_bytes_per_row) - sizeof(struct car_rendition_info_header);
    info_bytes_per_row.bytes_per_row = _width * bytes_per_pixel;

    // Write bitmap data
    ext::optional<std::vector<uint8_t>> data = Encode(this, renditionData);
    if (!data) {
        printf("Error: no bitmap data for %s\n", this->fileName().c_str());
        data = ext::optional<std::vector<uint8_t>>(std::vector<uint8_t>());
    }

    size_t compressed_data_length = data->size();
    uint8_t *compressed_data = reinterpret_cast<uint8_t *>(data->data());

    // Assemble Header and info segments
    size_t rendition_header_size = sizeof(struct car_rendition_value) + info_slices_size + \
         sizeof(struct car_rendition_info_header) + info_metrics.header.length + \
         sizeof(struct car_rendition_info_header) + info_composition.header.length + \
         sizeof(struct car_rendition_info_header) + info_bitmap_info.header.length + \
         sizeof(struct car_rendition_info_header) + info_bytes_per_row.header.length;
    size_t extra_headers_size = 0;

    header.info_len = rendition_header_size - sizeof(struct car_rendition_value);
    header.bitmaps.bitmap_count = 1;
    header.bitmaps.payload_size = compressed_data_length;

    if (Rendition::Data::FormatSavedAsRawData(renditionData->format())) {
        extra_headers_size += sizeof(struct car_rendition_data_header_raw);
    }

    std::vector<uint8_t> output = std::vector<uint8_t>(rendition_header_size + extra_headers_size + compressed_data_length);
    uint8_t *output_bytes = reinterpret_cast<uint8_t *>(output.data());

    memcpy(output_bytes, &header, sizeof(struct car_rendition_value));
    output_bytes += sizeof(struct car_rendition_value);

    memcpy(output_bytes, info_slices, info_slices_size);
    output_bytes += info_slices_size;
    free(info_slices);

    memcpy(output_bytes, &info_metrics, sizeof(struct car_rendition_info_metrics));
    output_bytes += sizeof(struct car_rendition_info_metrics);

    memcpy(output_bytes, &info_composition, sizeof(struct car_rendition_info_composition));
    output_bytes += sizeof(struct car_rendition_info_composition);

    memcpy(output_bytes, &info_bitmap_info, sizeof(struct car_rendition_info_bitmap_info));
    output_bytes += sizeof(struct car_rendition_info_bitmap_info);

    memcpy(output_bytes, &info_bytes_per_row, sizeof(struct car_rendition_info_bytes_per_row));
    output_bytes += sizeof(struct car_rendition_info_bytes_per_row);

    /* JPEG, RAW and non-standard formats adds one more header at the end */
    if (Rendition::Data::FormatSavedAsRawData(renditionData->format())) {
        struct car_rendition_data_header_raw raw_header;
        strncpy(raw_header.magic, "DWAR", 4);
        raw_header.length = compressed_data_length;
        memcpy(output_bytes, &raw_header, sizeof(struct car_rendition_data_header_raw));
        output_bytes += sizeof(struct car_rendition_data_header_raw);
    }

    memcpy(output_bytes, compressed_data, compressed_data_length);

    return output;
}

