/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <CoreGraphics/CGPDFPage.h>
#import <Onyx2D/O2PDFPage.h>
#import "CGConversions.h"

CGPDFPageRef CGPDFPageRetain(CGPDFPageRef self) {
   return [self retain];
}

void CGPDFPageRelease(CGPDFPageRef self) {
   [self release];
}

CGRect CGPDFPageGetBoxRect(CGPDFPageRef self,CGPDFBox box) {
   CGRect result;
   
   if(![self getRect:&result forBox:box])
    result=CGRectZero;
    
   return result;
}

CGAffineTransform CGPDFPageGetDrawingTransform(CGPDFPageRef self,CGPDFBox box,CGRect rect,int clockwiseDegrees,bool preserveAspectRatio) {
   return CGAffineTransformFromO2(O2PDFPageGetDrawingTransform(self,box,rect,clockwiseDegrees,preserveAspectRatio));
}

