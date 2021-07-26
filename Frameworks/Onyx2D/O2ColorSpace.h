/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

@class O2ColorSpace;

typedef O2ColorSpace *O2ColorSpaceRef;

#import <Onyx2D/O2Function.h>

typedef enum {
    kO2RenderingIntentDefault,
    kO2RenderingIntentAbsoluteColorimetric,
    kO2RenderingIntentRelativeColorimetric,
    kO2RenderingIntentSaturation,
    kO2RenderingIntentPerceptual,
} O2ColorRenderingIntent;

typedef enum {
    kO2ColorSpaceModelMonochrome,
    kO2ColorSpaceModelRGB,
    kO2ColorSpaceModelCMYK,
    kO2ColorSpaceModelLab,
    kO2ColorSpaceModelDeviceN,
    kO2ColorSpaceModelIndexed,
    kO2ColorSpaceModelPattern,
} O2ColorSpaceModel;

@interface O2ColorSpace : NSObject <NSCopying> {
    O2ColorSpaceModel _type;
    BOOL _isPlatformRGB;
}

- (O2ColorSpaceModel)type;

O2ColorSpaceRef O2ColorSpaceRetain(O2ColorSpaceRef self);
void O2ColorSpaceRelease(O2ColorSpaceRef self);

O2ColorSpaceRef O2ColorSpaceCreateDeviceGray(void);
O2ColorSpaceRef O2ColorSpaceCreateDeviceRGB(void);
O2ColorSpaceRef O2ColorSpaceCreateDeviceCMYK(void);
O2ColorSpaceRef O2ColorSpaceCreatePattern(O2ColorSpaceRef baseSpace);
O2ColorSpaceRef O2ColorSpaceCreateDeviceN(const char **names, O2ColorSpaceRef alternateSpace, O2FunctionRef tintTransform);

O2ColorSpaceRef O2ColorSpaceCreatePlatformRGB(void);
BOOL O2ColorSpaceIsPlatformRGB(O2ColorSpaceRef self);

size_t O2ColorSpaceGetNumberOfComponents(O2ColorSpaceRef self);

O2ColorSpaceModel O2ColorSpaceGetModel(O2ColorSpaceRef self);

- initWithPlatformRGB;

- (BOOL)isEqualToColorSpace:(O2ColorSpaceRef)other;

@end

@interface O2ColorSpace_indexed : O2ColorSpace {
    O2ColorSpace *_base;
    unsigned _hival;
    unsigned char *_bytes;
}

- initWithColorSpace:(O2ColorSpaceRef)baseColorSpace hival:(unsigned)hival bytes:(const unsigned char *)bytes;
- (O2ColorSpaceRef)baseColorSpace;
- (unsigned)hival;
- (const unsigned char *)paletteBytes;

@end

@interface O2ColorSpace_DeviceN : O2ColorSpace {
  @public
    unsigned _numberOfComponents;
  @protected
    char **_names;
    O2ColorSpaceRef _alternateSpace;
    O2FunctionRef _tintTransform;
}

- initWithComponentNames:(const char **)names alternateSpace:(O2ColorSpaceRef)altSpace tintTransform:(O2FunctionRef)tintTransform;

- (O2ColorSpaceRef)alternateSpace;
- (O2FunctionRef)tintTransform;

@end
