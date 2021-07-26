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
#import <Onyx2D/O2Paint_color.h>
#import <Onyx2D/O2Surface.h>

@implementation O2Paint_color

ONYX2D_STATIC int color_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){
   O2Paint_color *self=(O2Paint_color *)selfX;
   O2argb8u  rgba=self->_argb8u_PRE;
   int i;
   
   for(i=0;i<length;i++)
    span[i]=rgba;
    
   return length;
}

ONYX2D_STATIC int color_largb32f_PRE(O2Paint *selfX,int x,int y,O2argb32f *span,int length){
   O2Paint_color *self=(O2Paint_color *)selfX;
   O2argb32f  rgba=self->_argb32f_PRE;
   int i;
   
   for(i=0;i<length;i++)
    span[i]=rgba;

   return length;
}

-initWithGray:(O2Float)gray alpha:(O2Float)alpha surfaceToPaintTransform:(O2AffineTransform)transform {
   O2PaintInitWithTransform(self,transform);
   
   if(alpha==1.0f)
    isOpaque=TRUE;

   _paint_largb8u_PRE=color_largb8u_PRE;
   _paint_largb32f_PRE=color_largb32f_PRE;
   
   _argb32f_PRE=O2argb32fInit(gray,gray,gray,alpha);
   _argb32f_PRE=O2argb32fClamp(_argb32f_PRE);
   _argb32f_PRE=O2argb32fPremultiply(_argb32f_PRE);
   _argb8u_PRE=O2argb8uFromO2argb32f(_argb32f_PRE);
   return self;
}

-initWithRed:(O2Float)red green:(O2Float)green blue:(O2Float)blue alpha:(O2Float)alpha surfaceToPaintTransform:(O2AffineTransform)transform {
   O2PaintInitWithTransform(self,transform);
   
   if(alpha==1.0f)
    isOpaque=TRUE;
    
   _paint_largb8u_PRE=color_largb8u_PRE;
   _paint_largb32f_PRE=color_largb32f_PRE;
   _argb32f_PRE=O2argb32fInit(red,green,blue,alpha);
   _argb32f_PRE=O2argb32fClamp(_argb32f_PRE);
   _argb32f_PRE=O2argb32fPremultiply(_argb32f_PRE);
   _argb8u_PRE=O2argb8uFromO2argb32f(_argb32f_PRE);
   return self;
}

@end
