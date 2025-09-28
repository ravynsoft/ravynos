/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGShading.h>
#import <Onyx2D/O2Shading.h>

CGShadingRef CGShadingRetain(CGShadingRef self) {
   return O2ShadingRetain(self);
}

void CGShadingRelease(CGShadingRef self) {
   O2ShadingRelease(self);
}

CGShadingRef CGShadingCreateAxial(CGColorSpaceRef colorSpace,CGPoint startPoint,CGPoint endPoint,CGFunctionRef function,bool extendStart,bool extendEnd) {
   return O2ShadingCreateAxial(colorSpace,startPoint,endPoint,function,extendStart,extendEnd);
}

CGShadingRef CGShadingCreateRadial(CGColorSpaceRef colorSpace,CGPoint startPoint,float startRadius,CGPoint endPoint,float endRadius,CGFunctionRef function,bool extendStart,bool extendEnd) {
   return O2ShadingCreateRadial(colorSpace,startPoint,startRadius,endPoint,endRadius,function,extendStart,extendEnd);
}
