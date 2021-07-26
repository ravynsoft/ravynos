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
#import <Onyx2D/O2Paint_radialGradient.h>
#import <Onyx2D/O2Shading.h>

@implementation O2Paint_radialGradient

void O2PaintRadialGradient(O2Paint_radialGradient *self,O2Float *g, O2Float *rho, O2Float x, O2Float y) {
	RI_ASSERT(self);
	if( self->_endRadius <= 0.0f )
	{
		*g = 1.0f;
		*rho = 0.0f;
		return;
	}

	O2Float r = self->_endRadius;
	O2Point c = self->_endPoint;
	O2Point f = self->_startPoint;
	O2Point gx=O2PointMake(self->m_surfaceToPaintMatrix.a, self->m_surfaceToPaintMatrix.b);
	O2Point gy=O2PointMake(self->m_surfaceToPaintMatrix.c,self->m_surfaceToPaintMatrix.d);

	O2Point fp = Vector2Subtract(f,c);

	O2Float D = -1.0f / (Vector2Dot(fp,fp) - r*r);
	O2Point p=O2PointMake(x, y);
	p = Vector2Subtract(O2PointApplyAffineTransform(p,self->m_surfaceToPaintMatrix), c);
	O2Point d = Vector2Subtract(p,fp);
	O2Float s = (O2Float)sqrt(r*r*Vector2Dot(d,d) - RI_SQR(p.x*fp.y - p.y*fp.x));
	*g = (Vector2Dot(fp,d) + s) * D;
	if(RI_ISNAN(*g))
		*g = 0.0f;
        
	O2Float dgdx = D*Vector2Dot(fp,gx) + (r*r*Vector2Dot(d,gx) - (gx.x*fp.y - gx.y*fp.x)*(p.x*fp.y - p.y*fp.x)) * (D / s);
	O2Float dgdy = D*Vector2Dot(fp,gy) + (r*r*Vector2Dot(d,gy) - (gy.x*fp.y - gy.y*fp.x)*(p.x*fp.y - p.y*fp.x)) * (D / s);
	*rho = (O2Float)sqrt(dgdx*dgdx + dgdy*dgdy);
	if(RI_ISNAN(*rho))
		*rho = 0.0f;
	RI_ASSERT(*rho >= 0.0f);
}


ONYX2D_STATIC_INLINE O2argb32f radialGradientColorAt(O2Paint_radialGradient *self,int x,int y,int *skip){
   O2argb32f result;
   
   O2Float g, rho;
   O2PaintRadialGradient(self,&g, &rho, x+0.5f, y+0.5f);

   result = O2PaintColorRamp(self,g, rho,skip);

   return result;
}

ONYX2D_STATIC int radial_span_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){
   O2Paint_radialGradient *self=(O2Paint_radialGradient *)selfX;
   int i;
   int previous=-1;
   
   for(i=0;i<length;i++,x++){
    int skip=0;
    O2argb32f value=radialGradientColorAt(self,x,y,&skip);
    
    if(skip!=previous){
     if(previous==-1)
      previous=skip;
     else
      return (previous==1)?-i:i;
    }
    
    span[i]=O2argb8uFromO2argb32f(value);
   }
   return (previous==1)?-length:length;
}

ONYX2D_STATIC int radial_span_largb32f_PRE(O2Paint *selfX,int x,int y,O2argb32f *span,int length){
   O2Paint_radialGradient *self=(O2Paint_radialGradient *)selfX;
   int i;
   int previous=-1;
   
   for(i=0;i<length;i++,x++){
    int skip=0;
    O2argb32f value=radialGradientColorAt(self,x,y,&skip);
    
    if(skip!=previous){
     if(previous==-1)
      previous=skip;
     else
      return (previous==1)?-i:i;
    }
    
    span[i]=value;
   }
   return (previous==1)?-length:length;
}

-initWithShading:(O2Shading *)shading deviceTransform:(O2AffineTransform)deviceTransform {
   [super initWithShading:shading deviceTransform:deviceTransform numberOfSamples:1024];
   _paint_largb8u_PRE=radial_span_largb8u_PRE;
   _paint_largb32f_PRE=radial_span_largb32f_PRE;
   _endRadius=[shading endRadius];

   return self;
}


@end
