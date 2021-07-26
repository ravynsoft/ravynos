/*------------------------------------------------------------------------
 *
 * Derivative of the OpenVG 1.0.1 Reference Implementation
 * -------------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and /or associated documentation files
 * (the "Materials "), to deal in the Materials without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Materials,
 * and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Materials. 
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR
 * THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *-------------------------------------------------------------------*/
#import <Onyx2D/O2Paint_pattern.h>
#import <Onyx2D/O2Surface.h>

@implementation O2Paint_pattern

ONYX2D_STATIC int o2pattern_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){
   O2Paint_pattern *self=(O2Paint_pattern *)selfX;
   O2Image         *image=self->_image;
      
   O2ImageReadPatternSpan_largb8u_PRE(image,x,y,span,length,self->m_surfaceToPaintMatrix,kO2PatternTilingNoDistortion);
   
   return length;
}

ONYX2D_STATIC int o2pattern_largb32f_PRE(O2Paint *selfX,int x,int y,O2argb32f *span,int length){
   O2Paint_pattern *self=(O2Paint_pattern *)selfX;
   int i;
   
   for(i=0;i<length;i++)
    span[i]=O2argb32fInit(1,0,0,1);

   return length;
}

#if 0
static int O2PaintReadPremultipliedPatternSpan(O2Paint *selfX,int x,int y,O2argb32f *span,int length){
   O2Paint_pattern *self=(O2Paint_pattern *)selfX;
   
   O2ImageReadPatternSpan_largb32f_PRE(self->_image,x, y,span,length, self->m_surfaceToPaintMatrix, kO2PatternTilingConstantSpacing);
   return length;
}
#endif

-initWithImage:(O2Image *)image surfaceToPaintTransform:(O2AffineTransform)xform phase:(O2Size)phase {   
//   O2PaintInitWithTransform(self,O2AffineTransformMakeTranslation(phase.width,phase.height));
   O2PaintInitWithTransform(self,O2AffineTransformIdentity);
   _paint_largb8u_PRE=o2pattern_largb8u_PRE;
   _paint_largb32f_PRE=o2pattern_largb32f_PRE;
   _image=[image retain];
   _phase=phase;
   return self;
}

-(void)dealloc {
   [_image release];
   [super dealloc];
}

@end
