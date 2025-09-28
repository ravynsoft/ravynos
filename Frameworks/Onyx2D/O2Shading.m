/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Shading.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Function.h>
#import <Foundation/NSString.h>

@implementation O2Shading

-initWithColorSpace:(O2ColorSpaceRef)colorSpace startPoint:(O2Point)startPoint endPoint:(O2Point)endPoint function:(O2Function *)function extendStart:(BOOL)extendStart extendEnd:(BOOL)extendEnd domain:(float[])domain {
   _colorSpace=[colorSpace retain];
   _startPoint=startPoint;
   _endPoint=endPoint;
   _function=[function retain];
   _extendStart=extendStart;
   _extendEnd=extendEnd;
   _isRadial=NO;
   _domain[0]=domain[0];
   _domain[1]=domain[1];
   return self;
}

-initWithColorSpace:(O2ColorSpaceRef)colorSpace startPoint:(O2Point)startPoint startRadius:(float)startRadius endPoint:(O2Point)endPoint endRadius:(float)endRadius function:(O2Function *)function extendStart:(BOOL)extendStart extendEnd:(BOOL)extendEnd domain:(float[])domain {
   _colorSpace=[colorSpace retain];
   _startPoint=startPoint;
   _endPoint=endPoint;
   _function=[function retain];
   _extendStart=extendStart;
   _extendEnd=extendEnd;
   _isRadial=YES;
   _startRadius=startRadius;
   _endRadius=endRadius;
   _domain[0]=domain[0];
   _domain[1]=domain[1];
   return self;
}

-(void)dealloc {
   [_colorSpace release];
   [_function release];
   [super dealloc];
}

O2ColorSpaceRef O2ShadingColorSpace(O2Shading *self) {
   return self->_colorSpace;
}

-(O2Point)startPoint {
   return _startPoint;
}

-(O2Point)endPoint {
   return _endPoint;
}

-(float)startRadius {
   return _startRadius;
}

-(float)endRadius {
   return _endRadius;
}

-(BOOL)extendStart {
   return _extendStart;
}

-(BOOL)extendEnd {
   return _extendEnd;
}

-(O2Function *)function {
   return _function;
}

-(BOOL)isAxial {
   return _isRadial?NO:YES;
}

O2ShadingRef O2ShadingCreateAxial(O2ColorSpaceRef colorSpace,O2Point start,O2Point end,O2FunctionRef function,BOOL extendStart,BOOL extendEnd) {
   O2Float domain[2]={0.0f,1.0f};

   return [[O2Shading alloc] initWithColorSpace:colorSpace startPoint:start endPoint:end function:function extendStart:extendStart extendEnd:extendEnd domain:domain];
}

O2ShadingRef O2ShadingCreateRadial(O2ColorSpaceRef colorSpace,O2Point start,O2Float startRadius,O2Point end,O2Float endRadius,O2FunctionRef function,BOOL extendStart,BOOL extendEnd) {
   O2Float domain[2]={0.0f,1.0f};

   return [[O2Shading alloc] initWithColorSpace:colorSpace startPoint:start startRadius:startRadius endPoint:end endRadius:endRadius function:function extendStart:extendStart extendEnd:extendEnd domain:domain];
}

O2ShadingRef O2ShadingRetain(O2ShadingRef self) {
   return (self!=NULL)?(O2ShadingRef)CFRetain(self):NULL;
}

void O2ShadingRelease(O2ShadingRef self) {
   if(self!=NULL)
    CFRelease(self);
}


@end
