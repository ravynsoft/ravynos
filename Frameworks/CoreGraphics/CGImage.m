/* Copyright (c) 2006 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGImage.h>
#import <Foundation/NSString.h>
#import <Onyx2D/O2Image.h>

CGImageRef CGImageRetain(CGImageRef image) {
   return O2ImageRetain(image);
}

void CGImageRelease(CGImageRef image) {
   O2ImageRelease(image);
}

CGImageRef CGImageCreate(size_t width,size_t height,size_t bitsPerComponent,size_t bitsPerPixel,size_t bytesPerRow,CGColorSpaceRef colorSpace,CGBitmapInfo bitmapInfo,CGDataProviderRef dataProvider,const CGFloat *decode,bool interpolate,CGColorRenderingIntent renderingIntent) {
   return O2ImageCreate(width,height,bitsPerComponent,bitsPerPixel,bytesPerRow,colorSpace,bitmapInfo,dataProvider,decode,interpolate,renderingIntent);
}

CGImageRef CGImageMaskCreate(size_t width,size_t height,size_t bitsPerComponent,size_t bitsPerPixel,size_t bytesPerRow,CGDataProviderRef dataProvider,const CGFloat *decode,bool interpolate) {
   return O2ImageMaskCreate(width,height,bitsPerComponent,bitsPerPixel,bytesPerRow,dataProvider,decode,interpolate);
}

CGImageRef CGImageCreateCopy(CGImageRef self) {
   return O2ImageCreateCopy(self);
}

CGImageRef CGImageCreateCopyWithColorSpace(CGImageRef self,CGColorSpaceRef colorSpace) {
   return O2ImageCreateCopyWithColorSpace(self,colorSpace);
}

CGImageRef CGImageCreateWithImageInRect(CGImageRef self,CGRect rect) {
   return O2ImageCreateWithImageInRect(self,rect);
}

CGImageRef CGImageCreateWithJPEGDataProvider(CGDataProviderRef jpegProvider,const CGFloat *decode,bool interpolate,CGColorRenderingIntent renderingIntent) {
   return O2ImageCreateWithJPEGDataProvider(jpegProvider,decode,interpolate,renderingIntent);
}

CGImageRef CGImageCreateWithPNGDataProvider(CGDataProviderRef pngProvider,const CGFloat *decode,bool interpolate,CGColorRenderingIntent renderingIntent) {
   return O2ImageCreateWithPNGDataProvider(pngProvider,decode,interpolate,renderingIntent);
}

CGImageRef CGImageCreateWithMask(CGImageRef self,CGImageRef mask) {
   return O2ImageCreateWithMask(self,mask);
}

CGImageRef CGImageCreateWithMaskingColors(CGImageRef self,const CGFloat *components) {
   return O2ImageCreateWithMaskingColors(self,components);
}

size_t CGImageGetWidth(CGImageRef self) {
   return O2ImageGetWidth(self);
}

size_t CGImageGetHeight(CGImageRef self) {
   return O2ImageGetHeight(self);
}

size_t CGImageGetBitsPerComponent(CGImageRef self) {
   return O2ImageGetBitsPerComponent(self);
}

size_t CGImageGetBitsPerPixel(CGImageRef self) {
   return O2ImageGetBitsPerPixel(self);
}

size_t CGImageGetBytesPerRow(CGImageRef self) {
   return O2ImageGetBytesPerRow(self);
}

CGColorSpaceRef CGImageGetColorSpace(CGImageRef self) {
   return O2ImageGetColorSpace(self);
}

CGBitmapInfo CGImageGetBitmapInfo(CGImageRef self) {
   return O2ImageGetBitmapInfo(self);
}

CGDataProviderRef CGImageGetDataProvider(CGImageRef self) {
   return O2ImageGetDataProvider(self);
}

const CGFloat *CGImageGetDecode(CGImageRef self) {
   return O2ImageGetDecode(self);
}

bool CGImageGetShouldInterpolate(CGImageRef self) {
   return O2ImageGetShouldInterpolate(self);
}

CGColorRenderingIntent CGImageGetRenderingIntent(CGImageRef self) {
   return O2ImageGetRenderingIntent(self);
}

bool CGImageIsMask(CGImageRef self) {
   return O2ImageIsMask(self);
}

CGImageAlphaInfo CGImageGetAlphaInfo(CGImageRef self) {
   return O2ImageGetAlphaInfo(self);
}

