/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGBitmapContext.h>
#import <Onyx2D/O2BitmapContext.h>

CGContextRef CGBitmapContextCreate(void *bytes,size_t width,size_t height,size_t bitsPerComponent,size_t bytesPerRow,CGColorSpaceRef colorSpace,CGBitmapInfo bitmapInfo) {
   return O2BitmapContextCreate(bytes,width,height,bitsPerComponent,bytesPerRow,colorSpace,bitmapInfo);
}

void *CGBitmapContextGetData(CGContextRef self) {
   return O2BitmapContextGetData(self);
}

size_t CGBitmapContextGetWidth(CGContextRef self) {
   return O2BitmapContextGetWidth(self);
}

size_t CGBitmapContextGetHeight(CGContextRef self) {
   return O2BitmapContextGetHeight(self);
}

size_t CGBitmapContextGetBitsPerComponent(CGContextRef self) {
   return O2BitmapContextGetBitsPerComponent(self);
}

size_t CGBitmapContextGetBytesPerRow(CGContextRef self) {
   return O2BitmapContextGetBytesPerRow(self);
}

CGColorSpaceRef CGBitmapContextGetColorSpace(CGContextRef self) {
   return O2BitmapContextGetColorSpace(self);
}

CGBitmapInfo CGBitmapContextGetBitmapInfo(CGContextRef self) {
   return O2BitmapContextGetBitmapInfo(self);
}

size_t CGBitmapContextGetBitsPerPixel(CGContextRef self) {
   return O2BitmapContextGetBitsPerPixel(self);
}

CGImageAlphaInfo CGBitmapContextGetAlphaInfo(CGContextRef self) {
   return O2BitmapContextGetAlphaInfo(self);
}

CGImageRef CGBitmapContextCreateImage(CGContextRef self) {
   return O2BitmapContextCreateImage(self);
}

