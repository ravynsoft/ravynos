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
#import <Onyx2D/O2Paint_image.h>

@implementation O2Paint_image

#if 0
ONYX2D_STATIC int O2PaintReadResampledHighSpan_largb32f_PRE(O2Paint *selfX,int x,int y,O2argb32f *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;
   
   O2ImageBicubic_largb32f_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);
   return length;
}
#endif

ONYX2D_STATIC int O2PaintReadResampledLowSpan_largb32f_PRE(O2Paint *selfX,int x,int y,O2argb32f *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;
   
   O2ImageBilinear_largb32f_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);
   return length;
}

ONYX2D_STATIC int O2PaintReadResampledNoneSpan_largb32f_PRE(O2Paint *selfX,int x,int y,O2argb32f *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;
   
   O2ImagePointSampling_largb32f_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);
   return length;
}

//

void O2PaintApplyMaskSpan(O2argb8u *span,O2argb8u *maskSpan,int length){
   int i;
   
   for(i=0;i<length;i++){
    uint8_t alpha=maskSpan[i].r;
    
    span[i].r=O2Image_8u_mul_8u_div_255(span[i].r,alpha);
    span[i].g=O2Image_8u_mul_8u_div_255(span[i].g,alpha);
    span[i].b=O2Image_8u_mul_8u_div_255(span[i].b,alpha);
    span[i].a=O2Image_8u_mul_8u_div_255(span[i].a,alpha);
   }
}

#if 0
// disable below too
ONYX2D_STATIC int O2PaintReadResampledHighSpan_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;

   O2ImageBicubic_largb8u_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);
   
   if(self->_mask!=NULL){
    O2argb8u maskSpan[length];
    
    O2ImageBicubic_largb8u_PRE(self->_mask,x,y,maskSpan,length,self->_surfaceToMask);
    O2PaintApplyMaskSpan(span,maskSpan,length);
   }
   
   return length;
}
#endif

ONYX2D_STATIC int O2PaintReadResampledLowSpan_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;

   O2ImageBilinear_largb8u_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);

   if(self->_mask!=NULL){
    O2argb8u maskSpan[length];
    
    O2ImageBilinear_largb8u_PRE(self->_mask,x,y,maskSpan,length,self->_surfaceToMask);
    O2PaintApplyMaskSpan(span,maskSpan,length);
   }

   return length;
}

ONYX2D_STATIC int O2PaintReadResampledLowSpanFloatTranslate_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;

   O2ImageBilinearFloatTranslate_largb8u_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);

   if(self->_mask!=NULL){
    O2argb8u maskSpan[length];
    
    O2ImageBilinear_largb8u_PRE(self->_mask,x,y,maskSpan,length,self->_surfaceToMask);
    O2PaintApplyMaskSpan(span,maskSpan,length);
   }
   return length;
}

ONYX2D_STATIC int O2PaintReadResampledNoneSpan_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;

   O2ImagePointSampling_largb8u_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);
   if(self->_mask!=NULL){
    O2argb8u maskSpan[length];
    
    O2ImagePointSampling_largb8u_PRE(self->_mask,x,y,maskSpan,length,self->_surfaceToMask);
    O2PaintApplyMaskSpan(span,maskSpan,length);
   }
   
   return length;
}


ONYX2D_STATIC int O2PaintReadIntegerTranslateSpan_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){   
   O2Paint_image *self=(O2Paint_image *)selfX;
   
   O2ImageIntegerTranslate_largb8u_PRE(self->_image,x,y,span,length,self->m_surfaceToPaintMatrix);

   if(self->_mask!=NULL){
    O2argb8u maskSpan[length];
    
    O2ImagePointSampling_largb8u_PRE(self->_mask,x,y,maskSpan,length,self->_surfaceToMask);
    O2PaintApplyMaskSpan(span,maskSpan,length);
   }

   return length;
}

ONYX2D_STATIC int multiply(O2Paint *selfX,int x,int y,O2argb32f *span,int length){
   O2Paint_image *self=(O2Paint_image *)selfX;

   O2PaintReadSpan_largb32f_PRE(self->_paint,x,y,span,length);

   O2argb32f imageSpan[length];
   
// FIXME: Should this take into account the interpolation quality? (depends on how it is used)
   O2PaintReadResampledNoneSpan_largb32f_PRE(self,x,y,imageSpan,length);

   int i;
   
   for(i=0;i<length;i++,x++){
	//evaluate paint
	O2argb32f s=span[i];
    O2argb32f im=imageSpan[i];
    
	//apply image 
	// paint MULTIPLY image: convert paint to image number of channels, multiply with image, and convert to dst

			im.r *= s.r;
			im.g *= s.g;
			im.b *= s.b;
			im.a *= s.a;
			s = im;

    span[i]=s;
   }
   return length;
}

ONYX2D_STATIC int stencil(O2Paint *selfX,int x,int y,O2argb32f *span,int length){
   O2Paint_image *self=(O2Paint_image *)selfX;

   self->_paint->_paint_largb32f_PRE(self->_paint,x,y,span,length);

   O2argb32f imageSpan[length];
// FIXME: Should this take into account the interpolation quality? (depends on how it is used)
   O2PaintReadResampledNoneSpan_largb32f_PRE(self,x,y,imageSpan,length);

   int i;
   
   for(i=0;i<length;i++,x++){
	//evaluate paint
	O2argb32f s=span[i];
    O2argb32f im=imageSpan[i];
    
	//apply image 
	// paint STENCIL image: convert paint to dst, convert image to dst number of channels, multiply

{
 // FIX
 // This needs to be changed to a nonpremultplied form. This is the only case which used ar, ag, ab premultiplied values for source.

	O2Float ar = s.a, ag = s.a, ab = s.a;
			//the result will be in paint color space.
			//dst == RGB && image == RGB: RGB*RGB
			//dst == RGB && image == L  : RGB*LLL
			//dst == L   && image == RGB: L*(0.2126 R + 0.7152 G + 0.0722 B)
			//dst == L   && image == L  : L*L

			s.r *= im.r;
			s.g *= im.g;
			s.b *= im.b;
			s.a *= im.a;
			ar *= im.r;
			ag *= im.g;
			ab *= im.b;
			//in nonpremultiplied form the result is
			// s.rgb = paint.a * paint.rgb * image.a * image.rgb
			// s.a = paint.a * image.a
			// argb = paint.a * image.a * image.rgb

	RI_ASSERT(s.r >= 0.0f && s.r <= s.a && s.r <= ar);
	RI_ASSERT(s.g >= 0.0f && s.g <= s.a && s.g <= ag);
	RI_ASSERT(s.b >= 0.0f && s.b <= s.a && s.b <= ab);
}

    span[i]=s;
   }
   return length;
}


-initWithImage:(O2Image *)image mask:(O2ImageRef)mask mode:(O2SurfaceMode)mode paint:(O2Paint *)paint interpolationQuality:(O2InterpolationQuality)interpolationQuality surfaceToImage:(O2AffineTransform)surfaceToImage surfaceToMask:(O2AffineTransform)surfaceToMask {
   bool integerTranslate=FALSE;
   bool floatTranslate=FALSE;
   
   O2PaintInitWithTransform(self,surfaceToImage);
   
   if(surfaceToImage.a==1.0f && surfaceToImage.b==0.0f && surfaceToImage.c==0.0f && ABS(surfaceToImage.d)==1.0f){
    if(surfaceToImage.tx==RI_FLOOR_TO_INT(surfaceToImage.tx) && surfaceToImage.ty==RI_FLOOR_TO_INT(surfaceToImage.ty))
     integerTranslate=TRUE;
    else
     floatTranslate=TRUE;
   }
#if 0
   else
      NSLog(@"surfaceToImage a=%f, b=%f, c=%f, d=%f, tx=%f, ty=%f",surfaceToImage.a,surfaceToImage.b,surfaceToImage.c,surfaceToImage.d,surfaceToImage.tx,surfaceToImage.ty);
#endif

   switch(mode){
   
    case VG_DRAW_IMAGE_MULTIPLY:
     _paint_largb32f_PRE=multiply;
     break;
     
    case VG_DRAW_IMAGE_STENCIL:
     _paint_largb32f_PRE=stencil;
     break;
     
    default:
     switch(interpolationQuality){
#if 0 
// This is slow and buggy right now
      case kO2InterpolationHigh:
       if(integerTranslate)
        _paint_largb8u_PRE=O2PaintReadIntegerTranslateSpan_largb8u_PRE;
       else
       _paint_largb8u_PRE=O2PaintReadResampledHighSpan_largb8u_PRE;
       _paint_largb32f_PRE=O2PaintReadResampledHighSpan_largb32f_PRE;
       break;
#else
      case kO2InterpolationHigh:
#endif       
      case kO2InterpolationLow:
      
       if(integerTranslate)
        _paint_largb8u_PRE=O2PaintReadIntegerTranslateSpan_largb8u_PRE;
       else if(floatTranslate)
        _paint_largb8u_PRE=O2PaintReadResampledLowSpanFloatTranslate_largb8u_PRE;
       else
       _paint_largb8u_PRE=O2PaintReadResampledLowSpan_largb8u_PRE;
        
       _paint_largb32f_PRE=O2PaintReadResampledLowSpan_largb32f_PRE;
       break;

      case kO2InterpolationNone:
      default:
       if(integerTranslate)
        _paint_largb8u_PRE=O2PaintReadIntegerTranslateSpan_largb8u_PRE;
       else
       _paint_largb8u_PRE=O2PaintReadResampledNoneSpan_largb8u_PRE;
       _paint_largb32f_PRE=O2PaintReadResampledNoneSpan_largb32f_PRE;
       break;
    
     }
     break;
   }
   
   _image=O2ImageRetain(image);
   _mask=O2ImageRetain(mask);
   _mode=mode;
   _paint=[paint retain];
   _interpolationQuality=interpolationQuality;
   _surfaceToMask=surfaceToMask;
   
   return self;
}

-(void)dealloc {
   [_image release];
   O2PaintRelease(_paint);
   [super dealloc];
}

@end
