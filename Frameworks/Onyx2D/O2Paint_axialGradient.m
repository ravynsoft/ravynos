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
#import <Onyx2D/O2Paint_axialGradient.h>
#import <Onyx2D/O2Shading.h>

@implementation O2Paint_axialGradient

// O2PaintColorRamp is expensive, this could be  more accurate
// Interestingly, using this produces stair step artifacts at the same angles that Quartz2D produces artifacts
// The artifacts are different but shaped the same
ONYX2D_STATIC_INLINE O2argb8u O2PaintFastAxialRamp(O2Paint_ramp *self,O2Float gradient, O2Float rho,int *skip)  {
   if(gradient<=0.0)
    return self->_colorStops[0].color8u;
   else if(gradient>=1.0)
    return self->_colorStops[self->_numberOfColorStops-1].color8u;
   else {
    int i=RI_FLOOR_TO_INT(gradient*(self->_numberOfColorStops-1));
    return self->_colorStops[i].color8u;
   }
}

ONYX2D_STATIC int linear_span_largb8u_PRE(O2Paint *selfX,int x,int y,O2argb8u *span,int length){
   O2Paint_axialGradient *self=(O2Paint_axialGradient *)selfX;
   int i;
   int previous=-1;
   O2Point point=O2PointMake(x+0.5f, y+0.5f);
   O2Size  delta=O2SizeMake(1.0f,0.0f);
   
   point=O2PointApplyAffineTransform(point,self->m_surfaceToPaintMatrix);
   point=Vector2Subtract(point,self->_startPoint);
   delta=O2SizeApplyAffineTransform(delta,self->m_surfaceToPaintMatrix);
   
   if(self->_rho==0.0f){	//points are equal, gradient is always 1.0f
   for(i=0;i<length;i++,x++,point.x+=delta.width){
     int skip=0;
     O2argb8u  value=O2PaintFastAxialRamp(self,1.0f, self->_rho,&skip);
    
     if(skip!=previous){
      if(previous==-1)
       previous=skip;
      else
       return (previous==1)?-i:i;
     }

     span[i]=value;
    }
   }
   else {
    O2Float gx=point.x*self->_u.x*self->_oou;
    O2Float dx=delta.width*self->_u.x*self->_oou;
    O2Float gy=point.y*self->_u.y*self->_oou;

    for(i=0;i<length;i++,x++,gx+=dx){
     O2Float g=gx+gy;

    int skip=0;
     O2argb8u value=O2PaintFastAxialRamp(self,g, self->_rho,&skip);
    
    if(skip!=previous){
     if(previous==-1)
      previous=skip;
     else
      return (previous==1)?-i:i;
    }
    
     span[i]=value;
   }
   }
   
   return (previous==1)?-length:length;
}

ONYX2D_STATIC int linear_span_largb32f_PRE(O2Paint *selfX,int x,int y,O2argb32f *span,int length){
   O2Paint_axialGradient *self=(O2Paint_axialGradient *)selfX;
   int i;
   int previous=-1;
   O2Point point=O2PointMake(x+0.5f, y+0.5f);
   O2Size  delta=O2SizeMake(1.0f,0.0f);
   
   point=O2PointApplyAffineTransform(point,self->m_surfaceToPaintMatrix);
   point=Vector2Subtract(point,self->_startPoint);
   delta=O2SizeApplyAffineTransform(delta,self->m_surfaceToPaintMatrix);

   for(i=0;i<length;i++,x++,point.x+=delta.width){
    O2Float g;
    
    if(self->_rho==0.0f)	//points are equal, gradient is always 1.0f
     g=1.0f;
    else {    
     O2Point p=point;   

     g=Vector2Dot(p, self->_u)*self->_oou;
    }

    int skip=0;
    O2argb32f  value=O2PaintColorRamp(self,g, self->_rho,&skip);
    
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

// Calculate the number of samples based on the length of the gradient in device space
   O2Point startPoint=[shading startPoint];
   O2Point endPoint=[shading endPoint];
   O2Point deviceStart=O2PointApplyAffineTransform(startPoint,deviceTransform);
   O2Point deviceEnd=O2PointApplyAffineTransform(endPoint,deviceTransform);
   O2Float deltax=deviceStart.x-deviceEnd.x;
   O2Float deltay=deviceStart.y-deviceEnd.y;
   O2Float distance=sqrt(deltax*deltax+deltay*deltay);
   int     numberOfSamples=RI_INT_CLAMP(distance,2,8192);

   [super initWithShading:shading deviceTransform:deviceTransform numberOfSamples:numberOfSamples];

   _paint_largb8u_PRE=linear_span_largb8u_PRE;
   _paint_largb32f_PRE=linear_span_largb32f_PRE;
   _u=Vector2Subtract(_endPoint,_startPoint);
   O2Float usq=Vector2Dot(_u,_u);
   
   if(usq<=0.0f){	//points are equal, gradient is always 1.0f
    _oou=0.0f;
    _rho=0.0f;
   }
   else {
    _oou = 1.0f / usq;
    O2Float dgdx = _oou * _u.x * self->m_surfaceToPaintMatrix.a + _oou * _u.y * self->m_surfaceToPaintMatrix.b;
    O2Float dgdy = _oou * _u.x * self->m_surfaceToPaintMatrix.c + _oou * _u.y * self->m_surfaceToPaintMatrix.d;
    _rho = (O2Float)sqrt(dgdx*dgdx + dgdy*dgdy);
   }

   return self;
}


@end
