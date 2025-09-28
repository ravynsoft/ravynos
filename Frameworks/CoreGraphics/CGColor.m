/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGColor.h>
#import <Onyx2D/O2Color.h>
#import <Onyx2D/O2ColorSpace.h>

CGColorRef CGColorRetain(CGColorRef self) {
   return O2ColorRetain(self);
}

void CGColorRelease(CGColorRef self) {
   O2ColorRelease(self);
}

CGColorRef CGColorCreate(CGColorSpaceRef colorSpace,const CGFloat *components) {
   return O2ColorCreate(colorSpace,components);
}

CGColorRef CGColorCreateGenericGray(CGFloat gray,CGFloat a) {
   return O2ColorCreateGenericGray(gray,a);
}

CGColorRef CGColorCreateGenericRGB(CGFloat r,CGFloat g,CGFloat b,CGFloat a) {
    return O2ColorCreateGenericRGB(r,g,b,a);
}

CGColorRef CGColorCreateGenericCMYK(CGFloat c,CGFloat m,CGFloat y,CGFloat k,CGFloat a) {
   return O2ColorCreateGenericCMYK(c,m,y,k,a);
}

CGColorRef CGColorCreateWithPattern(CGColorSpaceRef colorSpace,CGPatternRef pattern,const CGFloat *components) {
   return O2ColorCreateWithPattern(colorSpace,pattern,components);
}

CGColorRef CGColorCreateCopy(CGColorRef self) {
   return O2ColorCreateCopy(self);
}

CGColorRef CGColorCreateCopyWithAlpha(CGColorRef self,CGFloat a) {
   return O2ColorCreateCopyWithAlpha(self,a);
}

bool CGColorEqualToColor(CGColorRef self,CGColorRef other) {
   return O2ColorEqualToColor(self,other);
}

CGColorSpaceRef CGColorGetColorSpace(CGColorRef self) {
   return O2ColorGetColorSpace(self);
}

size_t CGColorGetNumberOfComponents(CGColorRef self) {
   return O2ColorGetNumberOfComponents(self);
}

const CGFloat *CGColorGetComponents(CGColorRef self) {
   return O2ColorGetComponents(self);
}

CGFloat CGColorGetAlpha(CGColorRef self) {
   return O2ColorGetAlpha(self);
}

CGPatternRef CGColorGetPattern(CGColorRef self) {
   return O2ColorGetPattern(self);
}
