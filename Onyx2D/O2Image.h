/*------------------------------------------------------------------------
 *
 * Derivative of the OpenVG 1.0.1 Reference Implementation
 * -------------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
 * Copyright (c) 2008 Christopher J. W. Lloyd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and /or associated documentation files
 * (the "Materials "), to deal in the Materials without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Materials,
 * and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Materials. 
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR
 * THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *-------------------------------------------------------------------*/

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2AffineTransform.h>
#import <Onyx2D/O2argb8u.h>
#import <Onyx2D/O2argb32f.h>

#ifdef __cplusplus
extern "C" {
#endif

@class O2Image;

typedef O2Image *O2ImageRef;

@class O2ColorSpace, O2DataProvider;

#define kO2BitmapAlphaInfoMask 0x1F

typedef enum {
    kO2ImageAlphaNone,
    kO2ImageAlphaPremultipliedLast,
    kO2ImageAlphaPremultipliedFirst,
    kO2ImageAlphaLast,
    kO2ImageAlphaFirst,
    kO2ImageAlphaNoneSkipLast,
    kO2ImageAlphaNoneSkipFirst,
    kO2ImageAlphaOnly,
} O2ImageAlphaInfo;

enum {
    kO2BitmapFloatComponents = 0x100,
};

#define kO2BitmapByteOrderMask 0x7000

enum {
    kO2BitmapByteOrderDefault = 0,
    kO2BitmapByteOrder16Little = 0x1000,
    kO2BitmapByteOrder32Little = 0x2000,
    kO2BitmapByteOrder16Big = 0x3000,
    kO2BitmapByteOrder32Big = 0x4000,

#ifdef __BIG_ENDIAN__
    kO2BitmapByteOrder16Host = kO2BitmapByteOrder16Big,
    kO2BitmapByteOrder32Host = kO2BitmapByteOrder32Big,
#endif

#ifdef __LITTLE_ENDIAN__
    kO2BitmapByteOrder16Host = kO2BitmapByteOrder16Little,
    kO2BitmapByteOrder32Host = kO2BitmapByteOrder32Little,
#endif
};

typedef unsigned O2BitmapInfo;

#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Pattern.h>
#import <Onyx2D/O2ImageDecoder.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/VGmath.h>

typedef O2argb8u *(*O2ImageFunction_read_argb8u)(O2Image *self, int x, int y, O2argb8u *span, int length);
typedef O2argb32f *(*O2ImageFunction_read_argb32f)(O2Image *self, int x, int y, O2argb32f *span, int length);
typedef uint8_t *(*O2ImageFunction_read_a8u)(O2Image *self, int x, int y, uint8_t *span, int length);
typedef O2Float *(*O2ImageFunction_read_a32f)(O2Image *self, int x, int y, O2Float *span, int length);

@interface O2Image : NSObject <NSCopying> {
    size_t _width;
    size_t _height;
    size_t _bitsPerComponent;
    size_t _bitsPerPixel;
    size_t _bytesPerRow;

    O2ColorSpaceRef _colorSpace;
    O2BitmapInfo _bitmapInfo;
    O2ImageDecoder *_decoder;
    O2DataProvider *_provider;
    O2Float *_decode;
    BOOL _interpolate;
    BOOL _isMask;
    BOOL _clampExternalPixels;
    O2ColorRenderingIntent _renderingIntent;
    O2Image *_mask;

    NSData *_directData;
    const unsigned char *_directBytes;
    unsigned _directLength;
  @public
    O2ImageFunction_read_argb8u _read_argb8u;
    O2ImageFunction_read_argb32f _read_argb32f;
    O2ImageFunction_read_a8u _read_a8u;
    O2ImageFunction_read_a32f _read_a32f;
}

- initWithWidth:(size_t)width
              height:(size_t)height
    bitsPerComponent:(size_t)bitsPerComponent
        bitsPerPixel:(size_t)bitsPerPixel
         bytesPerRow:(size_t)bytesPerRow
          colorSpace:(O2ColorSpaceRef)colorSpace
          bitmapInfo:(O2BitmapInfo)bitmapInfo
             decoder:(O2ImageDecoder *)decoder
            provider:(O2DataProvider *)provider
              decode:(const O2Float *)decode
         interpolate:(BOOL)interpolate
     renderingIntent:(O2ColorRenderingIntent)renderingIntent;

- initMaskWithWidth:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bitsPerPixel:(size_t)bitsPerPixel bytesPerRow:(size_t)bytesPerRow provider:(O2DataProvider *)provider decode:(const float *)decode interpolate:(BOOL)interpolate;

- initWithJPEGDataProvider:(O2DataProvider *)jpegProvider decode:(const O2Float *)decode interpolate:(BOOL)interpolate renderingIntent:(O2ColorRenderingIntent)renderingIntent;
- initWithPNGDataProvider:(O2DataProvider *)jpegProvider decode:(const O2Float *)decode interpolate:(BOOL)interpolate renderingIntent:(O2ColorRenderingIntent)renderingIntent;

- (O2Image *)copyWithColorSpace:(O2ColorSpaceRef)colorSpace;

- (void)setMask:(O2Image *)mask;
- copyWithMask:(O2Image *)image;
- copyWithMaskingColors:(const O2Float *)components;

O2ImageRef O2ImageCreate(size_t width, size_t height, size_t bitsPerComponent, size_t bitsPerPixel, size_t bytesPerRow, O2ColorSpaceRef colorSpace, O2BitmapInfo bitmapInfo, O2DataProviderRef dataProvider, const O2Float *decode, BOOL shouldInterpolate, O2ColorRenderingIntent renderingIntent);
O2ImageRef O2ImageMaskCreate(size_t width, size_t height, size_t bitsPerComponent, size_t bitsPerPixel, size_t bytesPerRow, O2DataProviderRef dataProvider, const O2Float *decode, BOOL shouldInterpolate);
O2ImageRef O2ImageCreateCopy(O2ImageRef image);
O2ImageRef O2ImageCreateCopyWithColorSpace(O2ImageRef self, O2ColorSpaceRef colorSpace);
O2ImageRef O2ImageCreateWithJPEGDataProvider(O2DataProviderRef jpegProvider, const O2Float *decode, BOOL interpolate, O2ColorRenderingIntent renderingIntent);
O2ImageRef O2ImageCreateWithPNGDataProvider(O2DataProviderRef pngProvider, const O2Float *decode, BOOL interpolate, O2ColorRenderingIntent renderingIntent);

O2ImageRef O2ImageCreateWithImageInRect(O2ImageRef self, O2Rect rect);
O2ImageRef O2ImageCreateWithMask(O2ImageRef self, O2ImageRef mask);
O2ImageRef O2ImageCreateWithMaskingColors(O2ImageRef self, const O2Float *components);

O2ImageRef O2ImageRetain(O2ImageRef self);
void O2ImageRelease(O2ImageRef self);
BOOL O2ImageIsMask(O2ImageRef self);

size_t O2ImageGetWidth(O2ImageRef self);
size_t O2ImageGetHeight(O2ImageRef self);

size_t O2ImageGetBitsPerComponent(O2ImageRef self);
size_t O2ImageGetBitsPerPixel(O2ImageRef self);
size_t O2ImageGetBytesPerRow(O2ImageRef self);
O2ColorSpaceRef O2ImageGetColorSpace(O2ImageRef self);

O2ImageAlphaInfo O2ImageGetAlphaInfo(O2ImageRef self);
O2DataProviderRef O2ImageGetDataProvider(O2ImageRef self);
const O2Float *O2ImageGetDecode(O2ImageRef self);
BOOL O2ImageGetShouldInterpolate(O2ImageRef self);
O2ColorRenderingIntent O2ImageGetRenderingIntent(O2ImageRef self);
O2BitmapInfo O2ImageGetBitmapInfo(O2ImageRef self);

O2ImageRef O2ImageGetMask(O2ImageRef self);

O2ImageDecoderRef O2ImageGetImageDecoder(O2ImageRef self);

- (NSData *)directData;
- (const void *)directBytes;
- (void)releaseDirectDataIfPossible;

@end

O2ColorRenderingIntent O2ImageRenderingIntentWithName(const char *name);
const char *O2ImageNameWithIntent(O2ColorRenderingIntent intent);

/* O2Image and O2Surface can read and write any image format provided functions are provided to translate to/from either O2argb8u or O2argb32f spans.

  The return value will be either NULL or a pointer to the image data if direct access is available for the given format. You must use the return value if it is not NULL. If the value is not NULL you don't do a write back. This is a big boost for native ARGB format as it avoids a copy out and a copy in.
  
  The nomenclature for the argb8u functions is the bit sequencing in big endian
 */

uint8_t *O2ImageRead_G8_to_A8(O2Image *self, int x, int y, uint8_t *alpha, int length);
O2Float *O2ImageRead_ANY_to_A8_to_Af(O2Image *self, int x, int y, O2Float *alpha, int length);
uint8_t *O2Image_read_a8u_src_argb8u(O2Image *self, int x, int y, uint8_t *alpha, int length);

O2argb8u *O2ImageRead_G8_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_GA88_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_RGBA8888_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_ABGR8888_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_BGRA8888_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_RGB888_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_BGRX8888_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_XRGB8888_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_G3B5X1R5G2_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_RGBA4444_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_BARG4444_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_RGBA2222_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_CMYK8888_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb8u *O2ImageRead_I8_to_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);

O2argb32f *O2ImageRead_ANY_to_argb8u_to_argb32f(O2Image *self, int x, int y, O2argb32f *span, int length);

O2argb32f *O2ImageRead_argb32fLittle_to_argb32f(O2Image *self, int x, int y, O2argb32f *span, int length);
O2argb32f *O2ImageRead_argb32fBig_to_argb32f(O2Image *self, int x, int y, O2argb32f *span, int length);

O2argb8u *O2Image_read_argb8u(O2Image *self, int x, int y, O2argb8u *span, int length);
O2argb32f *O2Image_read_argb32f(O2Image *self, int x, int y, O2argb32f *span, int length);
uint8_t *O2Image_read_a8u(O2Image *self, int x, int y, uint8_t *coverage, int length);
O2Float *O2ImageReadSpan_Af_MASK(O2Image *self, int x, int y, O2Float *coverage, int length);

void O2ImageReadTileSpanExtendEdge_largb32f_PRE(O2Image *self, int u, int v, O2argb32f *span, int length);

void O2ImageBicubic_largb8u_PRE(O2Image *self, int x, int y, O2argb8u *span, int length, O2AffineTransform surfaceToImage);
void O2ImageBicubic_largb32f_PRE(O2Image *self, int x, int y, O2argb32f *span, int length, O2AffineTransform surfaceToImage);
void O2ImageBilinear_largb8u_PRE(O2Image *self, int x, int y, O2argb8u *span, int length, O2AffineTransform surfaceToImage);
void O2ImageBilinear_largb32f_PRE(O2Image *self, int x, int y, O2argb32f *span, int length, O2AffineTransform surfaceToImage);
void O2ImagePointSampling_largb8u_PRE(O2Image *self, int x, int y, O2argb8u *span, int length, O2AffineTransform surfaceToImage);
void O2ImagePointSampling_largb32f_PRE(O2Image *self, int x, int y, O2argb32f *span, int length, O2AffineTransform surfaceToImage);
void O2ImageBilinearFloatTranslate_largb8u_PRE(O2Image *self, int x, int y, O2argb8u *span, int length, O2AffineTransform surfaceToImage);
void O2ImageIntegerTranslate_largb8u_PRE(O2Image *self, int x, int y, O2argb8u *span, int length, O2AffineTransform surfaceToImage);

void O2ImageReadPatternSpan_largb8u_PRE(O2Image *self, O2Float x, O2Float y, O2argb8u *span, int length, O2AffineTransform surfaceToImage, O2PatternTiling distortion);
void O2ImageReadPatternSpan_largb32f_PRE(O2Image *self, O2Float x, O2Float y, O2argb32f *span, int length, O2AffineTransform surfaceToImage, O2PatternTiling distortion);

#ifdef __cplusplus
}
#endif
