/*------------------------------------------------------------------------
 *
 * Derivative of the OpenVG 1.0.1 Reference Implementation
 * -------------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
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

#import <Onyx2D/O2Image.h>

#import <Onyx2D/VGmath.h>
#import <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int RIuint32;
typedef short RIint16;
typedef unsigned int VGbitfield;

typedef enum {
    VG_TILE_FILL,
    VG_TILE_PAD,
    VG_TILE_REPEAT,
} VGTilingMode;

typedef enum {
    VG_DRAW_IMAGE_NORMAL,
    VG_DRAW_IMAGE_MULTIPLY,
    VG_DRAW_IMAGE_STENCIL,
} O2SurfaceMode;

#define RI_INT32_MAX (0x7fffffff)
#define RI_INT32_MIN (-0x7fffffff - 1)

@class O2Surface;

typedef void (*O2SurfaceWriteSpan_argb8u)(O2Surface *self, int x, int y, O2argb8u *span, int length);
typedef void (*O2SurfaceWriteSpan_argb32f)(O2Surface *self, int x, int y, O2argb32f *span, int length);

@interface O2Surface : O2Image {
    unsigned char *_pixelBytes;
    O2SurfaceWriteSpan_argb8u _writeargb8u;
    O2SurfaceWriteSpan_argb32f _writeargb32f;

    BOOL m_ownsData;
    pthread_mutex_t _lock;
}

- initWithBytes:(void *)bytes width:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bytesPerRow:(size_t)bytesPerRow colorSpace:(O2ColorSpaceRef)colorSpace bitmapInfo:(O2BitmapInfo)bitmapInfo;

- (void *)pixelBytes;

- (void)setWidth:(size_t)width height:(size_t)height reallocateOnlyIfRequired:(BOOL)roir;

void *O2SurfaceGetPixelBytes(O2Surface *surface);
size_t O2SurfaceGetWidth(O2Surface *surface);
size_t O2SurfaceGetHeight(O2Surface *surface);
size_t O2SurfaceGetBytesPerRow(O2Surface *surface);

O2ImageRef O2SurfaceCreateImage(O2Surface *surface);

void O2SurfaceLock(O2Surface *surface);
void O2SurfaceUnlock(O2Surface *surface);

BOOL O2SurfaceIsValidFormat(int format);

void O2SurfaceWriteSpan_argb8u_PRE(O2Surface *self, int x, int y, O2argb8u *span, int length);
void O2SurfaceWriteSpan_largb32f_PRE(O2Surface *self, int x, int y, O2argb32f *span, int length);

void O2SurfaceWriteMaskPixel(O2Surface *self, int x, int y, O2Float m); //can write only to VG_A_8

typedef struct O2GaussianKernel *O2GaussianKernelRef;

O2GaussianKernelRef O2CreateGaussianKernelWithDeviation(O2Float stdDeviation);
O2GaussianKernelRef O2GaussianKernelRetain(O2GaussianKernelRef kernel);
void O2GaussianKernelRelease(O2GaussianKernelRef kernel);

void O2SurfaceColorMatrix(O2Surface *self, O2Surface *src, const O2Float *matrix, BOOL filterFormatLinear, BOOL filterFormatPremultiplied, VGbitfield channelMask);
void O2SurfaceGaussianBlur(O2Surface *self, O2Image *src, O2GaussianKernelRef kernel, O2ColorRef color);
void O2SurfaceLookup(O2Surface *self, O2Surface *src, const uint8_t *redLUT, const uint8_t *greenLUT, const uint8_t *blueLUT, const uint8_t *alphaLUT, BOOL outputLinear, BOOL outputPremultiplied, BOOL filterFormatLinear, BOOL filterFormatPremultiplied, VGbitfield channelMask);

@end

#ifdef __cplusplus
}
#endif
