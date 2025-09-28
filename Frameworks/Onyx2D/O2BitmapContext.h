/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

@class O2Surface, O2BitmapContext;

typedef O2BitmapContext *O2BitmapContextRef;

typedef void (*O2BitmapContextReleaseDataCallback)(void *userInfo, void *data);

#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2Image.h>

@interface O2BitmapContext : O2Context {
    O2Surface *_surface;
}

- (Class)surfaceClass;

- initWithSurface:(O2Surface *)surface flipped:(BOOL)flipped;
-initWithBytes:(void *)bytes width:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bytesPerRow:(size_t)bytesPerRow colorSpace:(O2ColorSpaceRef)colorSpace bitmapInfo:(O2BitmapInfo)bitmapInfo releaseCallback:(O2BitmapContextReleaseDataCallback)releaseCallback releaseInfo:(void *)releaseInfo;

- (O2Surface *)surface;

O2ContextRef O2BitmapContextCreateWithData(void *bytes, size_t width, size_t height, size_t bitsPerComponent, size_t bytesPerRow, O2ColorSpaceRef colorSpace, O2BitmapInfo bitmapInfo, O2BitmapContextReleaseDataCallback releaseCallback, void *releaseInfo);

O2ContextRef O2BitmapContextCreate(void *bytes, size_t width, size_t height, size_t bitsPerComponent, size_t bytesPerRow, O2ColorSpaceRef colorSpace, O2BitmapInfo bitmapInfo);

void *O2BitmapContextGetData(O2ContextRef self);
size_t O2BitmapContextGetWidth(O2ContextRef self);
size_t O2BitmapContextGetHeight(O2ContextRef self);
size_t O2BitmapContextGetBitsPerComponent(O2ContextRef self);
size_t O2BitmapContextGetBitsPerPixel(O2ContextRef self);
size_t O2BitmapContextGetBytesPerRow(O2ContextRef self);
O2ColorSpaceRef O2BitmapContextGetColorSpace(O2ContextRef self);
O2ImageAlphaInfo O2BitmapContextGetAlphaInfo(O2ContextRef self);
O2BitmapInfo O2BitmapContextGetBitmapInfo(O2ContextRef self);
O2ImageRef O2BitmapContextCreateImage(O2ContextRef self);

@end

@interface O2Context (O2BitmapContext)
+ (O2Context *)createWithBytes:(void *)bytes width:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bytesPerRow:(size_t)bytesPerRow colorSpace:(O2ColorSpaceRef)colorSpace bitmapInfo:(O2BitmapInfo)bitmapInfo releaseCallback:(O2BitmapContextReleaseDataCallback)releaseCallback releaseInfo:(void *)releaseInfo;
@end
