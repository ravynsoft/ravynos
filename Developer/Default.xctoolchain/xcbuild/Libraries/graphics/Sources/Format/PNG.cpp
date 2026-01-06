/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <graphics/Format/PNG.h>

#include <iterator>
#include <memory>
#include <ext/optional>

#if _WIN32
#define CINTERFACE
#define COBJMACROS
#endif

#if _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using graphics::Format::PNG;
using graphics::PixelFormat;
using graphics::Image;

#if _WIN32

#include <windows.h>
#include <gdiplus.h>
#include <ole2.h>

static ext::optional<PixelFormat>
PixelFormatForPixelFormat(Gdiplus::PixelFormat pixelFormat)
{
    /* Windows is always little-endian. */
    static PixelFormat::Order const order = PixelFormat::Order::Reversed;

    switch (pixelFormat) {
        case PixelFormat24bppRGB:
            return PixelFormat(PixelFormat::Color::RGB, order, PixelFormat::Alpha::None);
        case PixelFormat32bppRGB:
            return PixelFormat(PixelFormat::Color::RGB, order, PixelFormat::Alpha::IgnoredFirst);
        case PixelFormat32bppARGB:
            return PixelFormat(PixelFormat::Color::RGB, order, PixelFormat::Alpha::First);
        case PixelFormat32bppPARGB:
            return PixelFormat(PixelFormat::Color::RGB, order, PixelFormat::Alpha::PremultipliedFirst);
        case PixelFormat1bppIndexed:
        case PixelFormat4bppIndexed:
        case PixelFormat8bppIndexed:
        case PixelFormat16bppRGB555:
        case PixelFormat16bppRGB565:
        case PixelFormat16bppARGB1555:
        case PixelFormat48bppRGB:
        case PixelFormat64bppARGB:
        case PixelFormat64bppPARGB:
            return ext::nullopt;
        default: abort();
    }
}

template<typename T, typename D>
static std::unique_ptr<T, D>
CustomUnique(T *value, D &&deleter)
{
    return std::unique_ptr<T, D>(value, std::move(deleter));
}

std::pair<ext::optional<Image>, std::string> PNG::
Read(std::vector<uint8_t> const &contents)
{
    /*
     * Start GDI+.
     */
    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR _token;
    Gdiplus::GdiplusStartup(&_token, &input, NULL);
    auto token = CustomUnique(reinterpret_cast<void const *>(_token), [](void const *token) {
        Gdiplus::GdiplusShutdown(reinterpret_cast<ULONG_PTR>(token));
    });
    (void)token;

    /*
     * Create memory handle with contents.
     */
    auto memory = CustomUnique(GlobalAlloc(GMEM_MOVEABLE, contents.size()), [](HGLOBAL handle) {
        GlobalFree(handle);
    });
    if (memory == nullptr) {
        return std::make_pair(ext::nullopt, "failed to allocate memory");
    }
    void *memoryData = GlobalLock(memory.get());
    if (memoryData == nullptr) {
        return std::make_pair(ext::nullopt, "failed to lock memory");
    }
    memcpy(memoryData, contents.data(), contents.size());
    if (GlobalUnlock(memory.get()) || GetLastError() != NO_ERROR) {
        return std::make_pair(ext::nullopt, "failed to unlock memory");
    }

    /*
     * Create stream for data.
     */
    IStream *_stream = nullptr;
    if (CreateStreamOnHGlobal(memory.get(), FALSE, &_stream) != S_OK) {
        return std::make_pair(ext::nullopt, "failed to create stream");
    }
    auto stream = CustomUnique(_stream, [](IStream *stream) {
        IStream_Release(stream);
    });

    /*
     * Load image into bitmap.
     */
    Gdiplus::GpBitmap *_bitmap = nullptr;
    if (Gdiplus::DllExports::GdipCreateBitmapFromStream(stream.get(), &_bitmap) != Gdiplus::Ok) {
        return std::make_pair(ext::nullopt, "failed to load bitmap");
    }
    auto bitmap = CustomUnique(_bitmap, [](Gdiplus::GpBitmap *bitmap) {
        Gdiplus::DllExports::GdipDisposeImage(bitmap);
    });

    /*
     * Get image size information.
     */
    UINT width;
    if (Gdiplus::DllExports::GdipGetImageWidth(bitmap.get(), &width) != Gdiplus::Ok) {
        return std::make_pair(ext::nullopt, "failed to get width");
    }
    UINT height;
    if (Gdiplus::DllExports::GdipGetImageHeight(bitmap.get(), &height) != Gdiplus::Ok) {
        return std::make_pair(ext::nullopt, "failed to get height");
    }

    /*
     * Determine pixel format.
     */
    Gdiplus::PixelFormat pixelFormat;
    if (Gdiplus::DllExports::GdipGetImagePixelFormat(bitmap.get(), &pixelFormat) != Gdiplus::Ok) {
        return std::make_pair(ext::nullopt, "failed to get pixel format");
    }
    ext::optional<PixelFormat> format = PixelFormatForPixelFormat(pixelFormat);
    if (!format) {
        /* Unsupported format, use a supported default. */
        pixelFormat = (Gdiplus::IsAlphaPixelFormat(pixelFormat) ? PixelFormat32bppPARGB : PixelFormat24bppRGB);
        format = PixelFormatForPixelFormat(pixelFormat);
    }

    /*
     * Lock memory to read bitmap pixels.
     */
    Gdiplus::BitmapData data;
    Gdiplus::GpRect rect = { 0, 0, static_cast<INT>(width), static_cast<INT>(height) };
    if (Gdiplus::DllExports::GdipBitmapLockBits(bitmap.get(), &rect, Gdiplus::ImageLockModeRead, pixelFormat, &data) != Gdiplus::Ok) {
        return std::make_pair(ext::nullopt, "failed to lock bits");
    }

    /*
     * Copy pixels from the image.
     */
    auto pixels = std::vector<uint8_t>(width * height * format->bytesPerPixel());
    for (size_t line = 0; line < height; line++) {
        uint8_t *scan = static_cast<uint8_t *>(data.Scan0) + (static_cast<int>(line) * data.Stride);
        uint8_t *out = pixels.data() + (width * line);

        /* Copy pixels from scan line. */
        memcpy(out, scan, width * format->bytesPerPixel());
    }

    /*
     * Done reading; unlock bitmap memory.
     */
    if (Gdiplus::DllExports::GdipBitmapUnlockBits(bitmap.get(), &data) != Gdiplus::Ok) {
        return std::make_pair(ext::nullopt, "failed to unlock bits");
    }

    Image image = Image(width, height, *format, pixels);
    return std::make_pair(image, std::string());
}

#elif defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>

/*
 * Smart pointer for CoreFoundation types.
 */
template<typename CF>
class CFHandleDeleter
{
public:
    void operator()(CF_CONSUMED CF object)
    {
        if (object != nullptr) {
            CFRelease(object);
        }
    }
};

template<typename CF>
using CFHandle = std::unique_ptr<typename std::remove_pointer<CF>::type, CFHandleDeleter<CF>>;

static ext::optional<PixelFormat>
PixelFormatFromCGColorSpaceAndCGBitmapInfo(CGColorSpaceRef colorSpace, CGBitmapInfo bitmapInfo)
{
    /* Float components are not supported. */
    if ((bitmapInfo & kCGBitmapFloatComponents) != 0) {
        return ext::nullopt;
    }

    /* Convert color space. */
    PixelFormat::Color color;
    switch (CGColorSpaceGetModel(colorSpace)) {
        case kCGColorSpaceModelMonochrome:
            color = PixelFormat::Color::Grayscale;
            break;
        case kCGColorSpaceModelRGB:
            color = PixelFormat::Color::RGB;
            break;
        default:
            return ext::nullopt;
    }

    /* Convert byte order. */
    PixelFormat::Order order;
    switch (bitmapInfo & kCGBitmapByteOrderMask) {
        case kCGBitmapByteOrderDefault:
        case kCGBitmapByteOrder16Big:
        case kCGBitmapByteOrder32Big:
            order = PixelFormat::Order::Forward;
            break;
        case kCGBitmapByteOrder16Little:
        case kCGBitmapByteOrder32Little:
            order = PixelFormat::Order::Reversed;
            break;
        default:
            return ext::nullopt;
    }

    /* Convert alpha format. */
    PixelFormat::Alpha alpha;
    switch (bitmapInfo & kCGBitmapAlphaInfoMask) {
        case kCGImageAlphaNone:
            alpha = PixelFormat::Alpha::None;
            break;
        case kCGImageAlphaFirst:
            alpha = PixelFormat::Alpha::First;
            break;
        case kCGImageAlphaLast:
            alpha = PixelFormat::Alpha::Last;
            break;
        case kCGImageAlphaPremultipliedFirst:
            alpha = PixelFormat::Alpha::PremultipliedFirst;
            break;
        case kCGImageAlphaPremultipliedLast:
            alpha = PixelFormat::Alpha::PremultipliedLast;
            break;
        case kCGImageAlphaNoneSkipFirst:
            alpha = PixelFormat::Alpha::IgnoredFirst;
            break;
        case kCGImageAlphaNoneSkipLast:
            alpha = PixelFormat::Alpha::IgnoredLast;
            break;
        default:
            return ext::nullopt;
    }

    return PixelFormat(color, order, alpha);
}

std::pair<ext::optional<Image>, std::string> PNG::
Read(std::vector<uint8_t> const &contents)
{
    /*
     * Load the image.
     */
    auto data = CFHandle<CFDataRef>(CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, contents.data(), contents.size(), kCFAllocatorNull));
    if (data == NULL) {
        return std::make_pair(ext::nullopt, "unable to create data");
    }

    auto options = CFHandle<CFDictionaryRef>(CFDictionaryCreate(kCFAllocatorDefault, NULL, NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    if (options == NULL) {
        return std::make_pair(ext::nullopt, "unable to create options");
    }

    auto imageSource = CFHandle<CGImageSourceRef>(CGImageSourceCreateWithData(data.get(), options.get()));
    if (imageSource == NULL) {
        return std::make_pair(ext::nullopt, "unable to create image source");
    }

    auto image = CFHandle<CGImageRef>(CGImageSourceCreateImageAtIndex(imageSource.get(), 0, NULL));
    if (image == NULL) {
        return std::make_pair(ext::nullopt, "unable to create image");
    }

    /*
     * Determine image properties.
     */
    size_t width = CGImageGetWidth(image.get());
    size_t height = CGImageGetHeight(image.get());
    CGColorSpaceRef inputColorSpace = CGImageGetColorSpace(image.get());
    CGImageAlphaInfo inputAlphaInfo = CGImageGetAlphaInfo(image.get());
    CGBitmapInfo inputBitmapInfo = CGImageGetBitmapInfo(image.get());

    if (auto format = PixelFormatFromCGColorSpaceAndCGBitmapInfo(inputColorSpace, inputBitmapInfo | inputAlphaInfo)) {
        /*
         * Supported image format; use the image data as-is.
         */
        CGDataProviderRef dataProvider = CGImageGetDataProvider(image.get());
        if (dataProvider == NULL) {
            return std::make_pair(ext::nullopt, "unable to get data provider");
        }

        auto data = CFHandle<CFDataRef>(CGDataProviderCopyData(dataProvider));
        if (data == NULL) {
            return std::make_pair(ext::nullopt, "unable to copy data");
        }

        /* Create result buffer. */
        auto pixels = std::vector<uint8_t>(width * height * format->bytesPerPixel());
        if (CFDataGetLength(data.get()) != pixels.size()) {
            return std::make_pair(ext::nullopt, "data is of unexpected length");
        }

        /* Copy the image data to the result. */
        CFRange range = CFRangeMake(0, CFDataGetLength(data.get()));
        CFDataGetBytes(data.get(), range, pixels.data());

        Image image = Image(width, height, *format, pixels);
        return std::make_pair(image, std::string());
    } else {
        /*
         * Convert to a supported image format. Note that CoreGraphics does not support
         * gray + alpha bitmap contexts, so always convert here to full color with alpha.
         */
        auto colorSpace = CFHandle<CGColorSpaceRef>(CGColorSpaceCreateDeviceRGB());
        if (colorSpace == NULL) {
            return std::make_pair(ext::nullopt, "unable to create color space");
        }

        /* Create a context for the result. */
        size_t channels = 4;
        size_t bitsPerComponent = 8;
        size_t bytesPerRow = (width * channels);
        CGBitmapInfo bitmapInfo = (kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
        std::vector<uint8_t> backing = std::vector<uint8_t>(width * height * channels);
        auto bitmapContext = CFHandle<CGContextRef>(CGBitmapContextCreate(backing.data(), width, height, bitsPerComponent, bytesPerRow, colorSpace.get(), bitmapInfo));
        if (bitmapContext == NULL) {
            return std::make_pair(ext::nullopt, "unable to create bitmap context");
        }

        /* Draw the image into the context. */
        CGRect rect = CGRectMake(0, 0, width, height);
        CGContextDrawImage(bitmapContext.get(), rect, image.get());

        /*
         * If the input image is grayscale, convert the pixel data to grayscale.
         */
        if (CGColorSpaceGetModel(inputColorSpace) == kCGColorSpaceModelMonochrome) {
            /* Convert to grayscale. */
            auto intermediateFormat = *PixelFormatFromCGColorSpaceAndCGBitmapInfo(colorSpace.get(), bitmapInfo);
            auto format = PixelFormat(PixelFormat::Color::Grayscale, intermediateFormat.order(), intermediateFormat.alpha());
            std::vector<uint8_t> pixels = PixelFormat::Convert(backing, intermediateFormat, format);
            Image image = Image(width, height, format, pixels);
            return std::make_pair(image, std::string());
        } else {
            /* Not grayscale, return it as-is. */
            PixelFormat format = *PixelFormatFromCGColorSpaceAndCGBitmapInfo(colorSpace.get(), bitmapInfo);
            Image image = Image(width, height, format, backing);
            return std::make_pair(image, std::string());
        }
    }
}

#else

#include <png.h>
#include <string.h>

static void
png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    unsigned char **contents_ptr = (unsigned char **)png_get_io_ptr(png_ptr);
    if (contents_ptr == NULL) {
        return;
    }

    memcpy(data, *contents_ptr, length);
    *contents_ptr += length;
}

std::pair<ext::optional<Image>, std::string> PNG::
Read(std::vector<uint8_t> const &contents)
{
    if (contents.size() < 8 || png_sig_cmp(const_cast<png_bytep>(static_cast<png_byte const *>(contents.data())), 0, 8)) {
        return std::make_pair(ext::nullopt, "contents is not a PNG");
    }

    png_struct *png_struct_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_struct_ptr == NULL) {
        return std::make_pair(ext::nullopt, "png_create_read_struct returned error");
    }

    png_info *info_struct_ptr = png_create_info_struct(png_struct_ptr);
    if (info_struct_ptr == NULL) {
        png_destroy_read_struct(&png_struct_ptr, NULL, NULL);
        return std::make_pair(ext::nullopt, "png_create_info_struct returned error");
    }

    if (setjmp(png_jmpbuf(png_struct_ptr))) {
        png_destroy_read_struct(&png_struct_ptr, &info_struct_ptr, NULL);
        return std::make_pair(ext::nullopt, "setjmp/png_jmpbuf returned error");
    }

    unsigned char const *contents_ptr = static_cast<unsigned char const *>(contents.data());
    png_set_read_fn(png_struct_ptr, &contents_ptr, png_user_read_data);

    png_read_info(png_struct_ptr, info_struct_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_method, compression_method, filter_method;
    if (!png_get_IHDR(png_struct_ptr, info_struct_ptr, &width, &height, &bit_depth, &color_type, &interlace_method, &compression_method, &filter_method)) {
        png_destroy_read_struct(&png_struct_ptr, &info_struct_ptr, NULL);
        return std::make_pair(ext::nullopt, "failed to read PNG header");
    }

    /* Convert to a standard bit depth. */
    if (bit_depth == 16) {
        png_set_strip_16(png_struct_ptr);
    } else if (bit_depth < 8) {
        if (color_type == PNG_COLOR_TYPE_GRAY) {
            png_set_expand_gray_1_2_4_to_8(png_struct_ptr);
        } else {
            png_set_packing(png_struct_ptr);
        }
    }

    /* Convert palleted images to RGB. */
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_struct_ptr);
    }

    /* Handle interlaced images. */
    (void)png_set_interlace_handling(png_struct_ptr);

    /* Apply transforms. */
    png_read_update_info(png_struct_ptr, info_struct_ptr);

    if (!png_get_IHDR(png_struct_ptr, info_struct_ptr, &width, &height, &bit_depth, &color_type, &interlace_method, &compression_method, &filter_method)) {
        png_destroy_read_struct(&png_struct_ptr, &info_struct_ptr, NULL);
        return std::make_pair(ext::nullopt, "png_get_IHDR returned error");
    }

    /* Determine output pixel format. */
    PixelFormat::Color color;
    PixelFormat::Alpha alpha;
    switch (color_type) {
        case PNG_COLOR_TYPE_RGB:
            color = PixelFormat::Color::RGB;
            alpha = PixelFormat::Alpha::None;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            color = PixelFormat::Color::RGB;
            alpha = PixelFormat::Alpha::Last;
            break;
        case PNG_COLOR_TYPE_GRAY:
            color = PixelFormat::Color::Grayscale;
            alpha = PixelFormat::Alpha::None;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            color = PixelFormat::Color::Grayscale;
            alpha = PixelFormat::Alpha::Last;
            break;
        case PNG_COLOR_TYPE_PALETTE: {
            /* Converted to RGB. */
            color = PixelFormat::Color::RGB;

            /* Alpha is separate for palleted images. */
            png_bytep trans_alpha = NULL;
            int num_trans = 0;
            png_color_16p trans_color = NULL;
            png_get_tRNS(png_struct_ptr, info_struct_ptr, &trans_alpha, &num_trans, &trans_color);
            alpha = (trans_alpha != NULL ? PixelFormat::Alpha::Last : PixelFormat::Alpha::None);
            break;
        }
        default:
            png_destroy_read_struct(&png_struct_ptr, &info_struct_ptr, NULL);
            return std::make_pair(ext::nullopt, "unhandled PNG color type");
    }
    PixelFormat format = PixelFormat(color, PixelFormat::Order::Forward, alpha);

    png_uint_32 row_bytes = png_get_rowbytes(png_struct_ptr, info_struct_ptr);
    if (row_bytes != (width * (bit_depth / 8) * format.bytesPerPixel())) {
        png_destroy_read_struct(&png_struct_ptr, &info_struct_ptr, NULL);
        return std::make_pair(ext::nullopt, "unable to transform PNG pixel data");
    }

    png_byte **row_pointers = (png_byte **)malloc(height * sizeof(png_bytep));
    if (row_pointers == NULL) {
        png_destroy_read_struct(&png_struct_ptr, &info_struct_ptr, NULL);
        return std::make_pair(ext::nullopt, "could not allocate memory");
    }

    auto pixels = std::vector<uint8_t>(width * height * format.bytesPerPixel());
    unsigned char *bytes = static_cast<unsigned char *>(pixels.data());
    for (int row = 0; row < height; row++) {
        row_pointers[row] = bytes + (row * row_bytes);
    }
    png_read_image(png_struct_ptr, row_pointers);

    /* Clean up. */
    png_read_end(png_struct_ptr, info_struct_ptr);
    png_destroy_read_struct(&png_struct_ptr, &info_struct_ptr, (png_infopp)NULL);
    free(row_pointers);

    Image image = Image(width, height, format, pixels);
    return std::make_pair(image, std::string());
}

#endif

#include <zlib.h>

std::pair<ext::optional<std::vector<uint8_t>>, std::string> PNG::
Write(Image const &image)
{
    std::vector<uint8_t> png;

    uint32_t const crc32_initial = crc32(0, NULL, 0);
    uint32_t crc32_big;
    uint8_t *crc32p = reinterpret_cast<uint8_t *>(&crc32_big);

    /*
     * Write out PNG header.
     */
    uint8_t const header[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
    png.insert(png.end(), std::begin(header), std::end(header));

    /*
     * Determine if the image has an alpha channel.
     */
    bool alpha;
    switch (image.format().alpha()) {
        case PixelFormat::Alpha::None:
        case PixelFormat::Alpha::IgnoredFirst:
        case PixelFormat::Alpha::IgnoredLast:
            /* No alpha. */
            alpha = false;
            break;
        case PixelFormat::Alpha::First:
        case PixelFormat::Alpha::Last:
        case PixelFormat::Alpha::PremultipliedFirst:
        case PixelFormat::Alpha::PremultipliedLast:
            /* Has alpha. */
            alpha = true;
            break;
        default: abort();
    }

    /*
     * Determine PNG color format.
     */
    uint8_t color_type;
    switch (image.format().color()) {
        case PixelFormat::Color::Grayscale:
            /* GA or G. */
            color_type = (alpha ? 0x04 : 0x00);
            break;
        case PixelFormat::Color::RGB:
            /* RGBA or RGB. */
            color_type = (alpha ? 0x06 : 0x02);
            break;
        default: abort();
    }

    /*
     * Convert image data to PNG format.
     */
    PixelFormat format = PixelFormat(
        image.format().color(),
        PixelFormat::Order::Forward,
        (alpha ? PixelFormat::Alpha::Last : PixelFormat::Alpha::None));
    std::vector<uint8_t> data = PixelFormat::Convert(image.data(), image.format(), format);

    /*
     * Write out the PNG header.
     */
    uint32_t width_big = htonl(image.width());
    uint8_t *width_buf = reinterpret_cast<uint8_t *>(&width_big);

    uint32_t height_big = htonl(image.height());
    uint8_t *height_buf = reinterpret_cast<uint8_t *>(&height_big);

    uint8_t const ihdr[] = {
        0x0, 0x0, 0x0, 0xD, // chunk length
        'I', 'H', 'D', 'R', // chunk type
        width_buf[0], width_buf[1], width_buf[2], width_buf[3], // width
        height_buf[0], height_buf[1], height_buf[2], height_buf[3], // height
        0x8, // bit depth
        color_type, // color type
        0x0, // compression method
        0x0, // filter method
        0x0, // interlace method
    };
    png.insert(png.end(), std::begin(ihdr), std::end(ihdr));

    crc32_big = crc32(crc32_initial, ihdr + 4, sizeof(ihdr) - 4);
    crc32_big = htonl(crc32_big);
    uint8_t const ihdr_crc32[] = { crc32p[0], crc32p[1], crc32p[2], crc32p[3] };
    png.insert(png.end(), std::begin(ihdr_crc32), std::end(ihdr_crc32));

    /*
     * Compress the pixel data with DEFLATE.
     */
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, 8, 15, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
        return std::make_pair(ext::nullopt, "deflate init failed");
    }

    /* Add filter bytes between rows in the image. */
    uint32_t filter_stride = image.width() * format.bytesPerPixel();
    size_t filter_size = (data.size() / filter_stride);
    std::vector<uint8_t> buffer = std::vector<uint8_t>(data.size() + filter_size);
    for (size_t i = 0; i < image.height(); i++) {
        size_t offset = (i * filter_stride);
        size_t filter_offset = i;

        *(buffer.data() + offset + filter_offset) = 0; // filter format
        memcpy(buffer.data() + offset + filter_offset + 1, data.data() + offset, filter_stride);
    }

    strm.avail_in = buffer.size();
    strm.next_in = buffer.data();

    /* Maximum compressed size is x1.01 + 12 the uncompressed size. */
    std::vector<uint8_t> compressed = std::vector<uint8_t>(static_cast<size_t>(buffer.size() * 1.01) + 12);

    do {
        strm.avail_out = compressed.size() - strm.total_out;
        strm.next_out = (Bytef *)(compressed.data() + strm.total_out);

        ret = deflate(&strm, Z_FINISH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            return std::make_pair(ext::nullopt, "deflate failed");
        }
    } while (ret != Z_STREAM_END);

    /* Shrink down to compressed size. */
    compressed.resize(strm.total_out);

    ret = deflateEnd(&strm);
    if (ret != Z_OK) {
        return std::make_pair(ext::nullopt, "deflate end failed");
    }

    /*
     * Write out IDAT chunk with the image data.
     */
    uint32_t size_big = htonl(compressed.size());
    uint8_t *size_buf = reinterpret_cast<uint8_t *>(&size_big);

    uint8_t const idat[] = {
        size_buf[0], size_buf[1], size_buf[2], size_buf[3], // chunk length
        'I', 'D', 'A', 'T', // chunk type
    };
    png.insert(png.end(), std::begin(idat), std::end(idat));
    png.insert(png.end(), compressed.begin(), compressed.end());

    crc32_big = crc32(crc32_initial, idat + 4, sizeof(idat) - 4);
    crc32_big = crc32(crc32_big, compressed.data(), compressed.size());
    crc32_big = htonl(crc32_big);
    uint8_t const idat_crc32[] = { crc32p[0], crc32p[1], crc32p[2], crc32p[3] };
    png.insert(png.end(), std::begin(idat_crc32), std::end(idat_crc32));

    /*
     * Write out final IEND chunk.
     */
    uint8_t const iend[] = {
        0x0, 0x0, 0x0, 0x0, // chunk length
        'I', 'E', 'N', 'D', // chunk type
    };
    png.insert(png.end(), std::begin(iend), std::end(iend));

    crc32_big = crc32(crc32_initial, iend + 4, sizeof(iend) - 4);
    crc32_big = htonl(crc32_big);
    uint8_t const iend_crc32[] = { crc32p[0], crc32p[1], crc32p[2], crc32p[3] };
    png.insert(png.end(), std::begin(iend_crc32), std::end(iend_crc32));

    return std::make_pair(png, std::string());
}

