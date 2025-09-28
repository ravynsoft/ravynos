/* Copyright(c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGContext.h>
#import <CoreGraphics/CGImage.h>

typedef void (*CGBitmapContextReleaseDataCallback)(void *userInfo, void *data);

COREGRAPHICS_EXPORT CGContextRef CGBitmapContextCreate(void *data, size_t width, size_t height, size_t bitsPerComponent, size_t bytesPerRow, CGColorSpaceRef colorSpace, CGBitmapInfo bitmapInfo);

COREGRAPHICS_EXPORT void *CGBitmapContextGetData(CGContextRef self);
COREGRAPHICS_EXPORT size_t CGBitmapContextGetWidth(CGContextRef self);
COREGRAPHICS_EXPORT size_t CGBitmapContextGetHeight(CGContextRef self);
COREGRAPHICS_EXPORT size_t CGBitmapContextGetBitsPerComponent(CGContextRef self);
COREGRAPHICS_EXPORT size_t CGBitmapContextGetBytesPerRow(CGContextRef self);
COREGRAPHICS_EXPORT CGColorSpaceRef CGBitmapContextGetColorSpace(CGContextRef self);
COREGRAPHICS_EXPORT CGBitmapInfo CGBitmapContextGetBitmapInfo(CGContextRef self);

COREGRAPHICS_EXPORT size_t CGBitmapContextGetBitsPerPixel(CGContextRef self);
COREGRAPHICS_EXPORT CGImageAlphaInfo CGBitmapContextGetAlphaInfo(CGContextRef self);
COREGRAPHICS_EXPORT CGImageRef CGBitmapContextCreateImage(CGContextRef self);
