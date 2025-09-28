/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>

@class O2Pattern;
@class O2Color;

typedef O2Color *O2ColorRef;

#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Pattern.h>

@interface O2Color : NSObject {
    O2ColorSpaceRef _colorSpace;
    unsigned _numberOfComponents;
    O2Float *_components;
    O2Pattern *_pattern;
}

O2ColorRef O2ColorRetain(O2ColorRef self);
void O2ColorRelease(O2ColorRef self);

O2ColorRef O2ColorCreate(O2ColorSpaceRef colorSpace, const O2Float *components);
O2ColorRef O2ColorCreateGenericGray(O2Float gray, O2Float a);
O2ColorRef O2ColorCreateGenericRGB(O2Float r, O2Float g, O2Float b, O2Float a);
O2ColorRef O2ColorCreateGenericCMYK(O2Float c, O2Float m, O2Float y, O2Float k, O2Float a);
O2ColorRef O2ColorCreateWithPattern(O2ColorSpaceRef colorSpace, O2PatternRef pattern, const O2Float *components);

O2ColorRef O2ColorCreateCopy(O2ColorRef self);
O2ColorRef O2ColorCreateCopyWithAlpha(O2ColorRef self, O2Float a);

BOOL O2ColorEqualToColor(O2ColorRef self, O2ColorRef other);

O2ColorSpaceRef O2ColorGetColorSpace(O2ColorRef self);
size_t O2ColorGetNumberOfComponents(O2ColorRef self);
const O2Float *O2ColorGetComponents(O2ColorRef self);
O2Float O2ColorGetAlpha(O2ColorRef self);

O2PatternRef O2ColorGetPattern(O2ColorRef self);

int O2ColorConvertComponentsToDeviceRGB(O2ColorSpaceRef inputSpace, const O2Float *input, O2Float *output);
O2ColorRef O2ColorConvertToDeviceRGB(O2ColorRef self);

@end
