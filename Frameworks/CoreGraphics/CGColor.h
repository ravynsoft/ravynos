/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CoreGraphicsExport.h>

typedef struct O2Color *CGColorRef;

#import "CGGeometry.h"
#import "CGColorSpace.h"
#import "CGPattern.h"

COREGRAPHICS_EXPORT CGColorRef CGColorRetain(CGColorRef self);
COREGRAPHICS_EXPORT void CGColorRelease(CGColorRef self);

COREGRAPHICS_EXPORT CGColorRef CGColorCreate(CGColorSpaceRef colorSpace, const CGFloat *components);
COREGRAPHICS_EXPORT CGColorRef CGColorCreateGenericGray(CGFloat gray, CGFloat a);
COREGRAPHICS_EXPORT CGColorRef CGColorCreateGenericRGB(CGFloat r, CGFloat g, CGFloat b, CGFloat a);
COREGRAPHICS_EXPORT CGColorRef CGColorCreateGenericCMYK(CGFloat c, CGFloat m, CGFloat y, CGFloat k, CGFloat a);
COREGRAPHICS_EXPORT CGColorRef CGColorCreateWithPattern(CGColorSpaceRef colorSpace, CGPatternRef pattern, const CGFloat *components);

COREGRAPHICS_EXPORT CGColorRef CGColorCreateCopy(CGColorRef self);
COREGRAPHICS_EXPORT CGColorRef CGColorCreateCopyWithAlpha(CGColorRef self, CGFloat a);

COREGRAPHICS_EXPORT bool CGColorEqualToColor(CGColorRef self, CGColorRef other);

COREGRAPHICS_EXPORT CGColorSpaceRef CGColorGetColorSpace(CGColorRef self);
COREGRAPHICS_EXPORT size_t CGColorGetNumberOfComponents(CGColorRef self);
COREGRAPHICS_EXPORT const CGFloat *CGColorGetComponents(CGColorRef self);
COREGRAPHICS_EXPORT CGFloat CGColorGetAlpha(CGColorRef self);

COREGRAPHICS_EXPORT CGPatternRef CGColorGetPattern(CGColorRef self);
