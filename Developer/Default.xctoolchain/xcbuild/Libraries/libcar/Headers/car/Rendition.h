/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef _LIBCAR_RENDITION_H
#define _LIBCAR_RENDITION_H

#include <car/AttributeList.h>
#include <ext/optional>

#include <string>
#include <functional>

namespace car {

class Reader;

/*
 * Represents a specific variant of a facet.
 */
class Rendition {
public:
    class Data {
    public:
        enum class Format {
            /*
             * BGRA pixels in order.
             */
            PremultipliedBGRA8,
            /*
             * Gray and alpha in order.
             */
            PremultipliedGA8,
            /*
             * Raw data, JPEG format.
             */
            JPEG,
            /*
             * Raw data.
             */
            Data,
            /*
             * Non-standard formats
             */
            NON_STANDARD_WEBP,
        };

        /*
         * The size of a pixel in the format.
         */
        static size_t FormatSize(Format format);

        /*
         * If the format is saved as raw data, that requires additional headers
         */
        static bool FormatSavedAsRawData(Format format);

    private:
        std::vector<uint8_t> _data;
        Format               _format;

    public:
        Data(std::vector<uint8_t> const &data, Format format);

    public:
        /*
         * The raw data.
         */
        std::vector<uint8_t> const &data() const
        { return _data; }
        std::vector<uint8_t> &data()
        { return _data; }

        /*
         * The pixel format of the data.
         */
        Format format() const
        { return _format; }
        Format &format()
        { return _format; }
    };

public:
    enum class ResizeMode {
        FixedSize,
        Tile,
        Scale,
        Uniform,
        HorizontalUniformVerticalScale,
        HorizontalScaleVerticalUniform,
    };

public:
    struct Slice {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };

private:
    AttributeList  _attributes;

private:
    std::function<ext::optional<Data>(Rendition const *)> _deferredData;
    ext::optional<Data> _data;

private:
    std::string                     _fileName;
    int                             _width;
    int                             _height;
    double                          _scale;
    bool                            _isVector;
    bool                            _isOpaque;
    bool                            _isResizable;
    ResizeMode                      _resizeMode;
    std::vector<Slice>              _slices;
    enum car_rendition_value_layout _layout;
    ext::optional<std::string>      _UTI;

private:
    Rendition(AttributeList const &attributes, std::function<ext::optional<Data>(Rendition const *)> const &data);
    Rendition(AttributeList const &attributes, ext::optional<Data> const &data);

public:
    /*
     * The attributes that describe the rendition.
     */
    AttributeList const &attributes() const
    { return _attributes; }

public:
    /*
     * The file name of the rendition.
     */
    std::string const &fileName() const
    { return _fileName; }
    std::string &fileName()
    { return _fileName; }

    /*
     * The UTI type of the rendition.
     */
    ext::optional<std::string> const &UTI() const
    { return _UTI; }
    ext::optional<std::string> &UTI()
    { return _UTI; }

public:
    /*
     * The width of the rendition, in pixels.
     */
    int width() const
    { return _width; }
    int &width()
    { return _width; }

    /*
     * The height of the rendition, in pixels.
     */
    int height() const
    { return _height; }
    int &height()
    { return _height; }

    /*
     * The scale of the rendition.
     */
    double scale() const
    { return _scale; }
    double &scale()
    { return _scale; }

public:
    /*
     * Is vector
     */
    bool isVector() const
    { return _isVector; }
    bool &isVector()
    { return _isVector; }

    /*
     * Is opaque
     */
    bool isOpaque() const
    { return _isOpaque; }
    bool &isOpaque()
    { return _isOpaque; }

    /*
     * layout
     */
    enum car_rendition_value_layout layout() const
    { return _layout; }
    enum car_rendition_value_layout &layout()
    { return _layout; }

public:
    /*
     * If the rendition is resizable at all.
     */
    bool isResizable() const
    { return _isResizable; }
    bool &isResizable()
    { return _isResizable; }

    /*
     * How the rendition should resize.
     */
    ResizeMode resizeMode() const
    { return _resizeMode; }
    ResizeMode &resizeMode()
    { return _resizeMode; }

    /*
     * How the rendition is sliced for resizing.
     */
    std::vector<Slice> const &slices() const
    { return _slices; }
    std::vector<Slice> &slices()
    { return _slices; }

public:
    /*
     * The rendition pixel data. May incur expensive decoding.
     */
    ext::optional<Data> data() const;

public:
    /*
     * Serialize the rendition for writing to a file.
     */
    std::vector<uint8_t> write() const;

public:
    /*
     * Dump a description of the rendition. For debugging.
     */
    void dump() const;

public:
    /*
     * Load an existing rendition matching the provided attributes.
     */
    static Rendition const Load(
        AttributeList const &attributes,
        struct car_rendition_value *value);

public:
    /*
     * Create a new rendition with the given properties.
     */
    static Rendition Create(
        AttributeList const &attributes,
        std::function<ext::optional<Data>(Rendition const *)> const &data);

    static Rendition Create(
        AttributeList const &attributes,
        ext::optional<Data> const &data);
};

}

#endif /* _LIBCAR_RENDITION_H */
