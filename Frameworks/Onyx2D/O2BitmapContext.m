/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2BitmapContext.h>
#import <Onyx2D/O2GraphicsState.h>
#import <Onyx2D/O2Image.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Exceptions.h>
#import <Onyx2D/O2Surface.h>

@implementation O2BitmapContext

-initWithSurface:(O2Surface *)surface flipped:(BOOL)flipped {
   O2AffineTransform flip=flipped?O2AffineTransformIdentity:O2AffineTransformMake(1,0,0,-1,0,O2ImageGetHeight(surface));
   O2GState  *initialState=[[[O2GState alloc] initWithDeviceTransform:flip] autorelease];

   [super initWithGraphicsState:initialState];
   _surface=[surface retain];
   return self;
}

-(Class)surfaceClass {
   return [O2Surface class];
}

-initWithBytes:(void *)bytes width:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bytesPerRow:(size_t)bytesPerRow colorSpace:(O2ColorSpaceRef)colorSpace bitmapInfo:(O2BitmapInfo)bitmapInfo releaseCallback:(O2BitmapContextReleaseDataCallback)releaseCallback releaseInfo:(void *)releaseInfo {
   O2Surface *surface=[[[self surfaceClass] alloc] initWithBytes:bytes width:width height:height bitsPerComponent:bitsPerComponent bytesPerRow:bytesPerRow colorSpace:colorSpace bitmapInfo:bitmapInfo];

   [self initWithSurface:surface flipped:NO];
   [surface release];
   return self;
}

-(void)dealloc {
   [_surface release];
   [super dealloc];
}

-(O2Surface *)surface {
   return _surface;
}

O2ContextRef O2BitmapContextCreateWithData(void *data,size_t width,size_t height,size_t bitsPerComponent,size_t bytesPerRow,O2ColorSpaceRef colorSpace,O2BitmapInfo bitmapInfo,O2BitmapContextReleaseDataCallback releaseCallback,void *releaseInfo) {
   return [O2Context createWithBytes:data width:width height:height bitsPerComponent:bitsPerComponent bytesPerRow:bytesPerRow colorSpace:colorSpace bitmapInfo:bitmapInfo releaseCallback:releaseCallback releaseInfo:releaseInfo];
}

O2ContextRef O2BitmapContextCreate(void *data,size_t width,size_t height,size_t bitsPerComponent,size_t bytesPerRow,O2ColorSpaceRef colorSpace,O2BitmapInfo bitmapInfo) {
   return O2BitmapContextCreateWithData(data,width,height,bitsPerComponent,bytesPerRow,colorSpace,bitmapInfo,NULL,NULL);
}

void  *O2BitmapContextGetData(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;
   
   return [self->_surface pixelBytes];
}

size_t O2BitmapContextGetWidth(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;

   return O2ImageGetWidth(self->_surface);
}

size_t O2BitmapContextGetHeight(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;

   return O2ImageGetHeight(self->_surface);
}

size_t O2BitmapContextGetBitsPerComponent(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;

   return O2ImageGetBitsPerComponent(self->_surface);
}

size_t O2BitmapContextGetBitsPerPixel(O2ContextRef self) {
   size_t result=O2BitmapContextGetBitsPerComponent(self)*O2ColorSpaceGetNumberOfComponents(O2BitmapContextGetColorSpace(self));
   
   switch(O2BitmapContextGetAlphaInfo(self)){
   
    case kO2ImageAlphaNone:
     break;
     
    case kO2ImageAlphaPremultipliedLast:
     result+=O2BitmapContextGetBitsPerComponent(self);
     break;

    case kO2ImageAlphaPremultipliedFirst:
     result+=O2BitmapContextGetBitsPerComponent(self);
     break;

    case kO2ImageAlphaLast:
     result+=O2BitmapContextGetBitsPerComponent(self);
     break;

    case kO2ImageAlphaFirst:
     result+=O2BitmapContextGetBitsPerComponent(self);
     break;

    case kO2ImageAlphaNoneSkipLast:
     break;

    case kO2ImageAlphaNoneSkipFirst:
     break;

    case kO2ImageAlphaOnly:
     break;
   }
   
   return result;
}

size_t O2BitmapContextGetBytesPerRow(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;

   return O2ImageGetBytesPerRow(self->_surface);
}

O2ColorSpaceRef  O2BitmapContextGetColorSpace(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;

   return O2ImageGetColorSpace(self->_surface);
}

O2ImageAlphaInfo O2BitmapContextGetAlphaInfo(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;

   return O2ImageGetAlphaInfo(self->_surface);
}

O2BitmapInfo O2BitmapContextGetBitmapInfo(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;

   return O2ImageGetBitmapInfo(self->_surface);
}

O2ImageRef O2BitmapContextCreateImage(O2ContextRef selfX) {
   O2BitmapContextRef self=(O2BitmapContextRef)selfX;
   
   return O2SurfaceCreateImage(self->_surface);
}

@end
