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
#import <Onyx2D/O2Paint_ramp.h>
#import <Onyx2D/O2Shading.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Function.h>

@implementation O2Paint_ramp

ONYX2D_STATIC_INLINE void GrayAToRGBA(float *input,float *output){
   output[0]=input[0];
   output[1]=input[0];
   output[2]=input[0];
   output[3]=input[1];
}

ONYX2D_STATIC_INLINE void RGBAToRGBA(float *input,float *output){
   output[0]=input[0];
   output[1]=input[1];
   output[2]=input[2];
   output[3]=input[3];
}

ONYX2D_STATIC_INLINE void CMYKAToRGBA(float *input,float *output){
   float white=1-input[3];
   
   output[0]=(input[0]>white)?0:white-input[0];
   output[1]=(input[1]>white)?0:white-input[1];
   output[2]=(input[2]>white)?0:white-input[2];
   output[3]=input[4];
}

-initWithShading:(O2Shading *)shading deviceTransform:(O2AffineTransform)deviceTransform numberOfSamples:(int)numberOfSamples {
   O2PaintInitWithTransform(self,O2AffineTransformInvert(deviceTransform));
   
   _startPoint=[shading startPoint];
   _endPoint=[shading endPoint];
   _extendStart=[shading extendStart];
   _extendEnd=[shading extendEnd];
   
   O2Function      *function=[shading function];
   O2ColorSpace    *colorSpace=O2ShadingColorSpace(shading);
   O2ColorSpaceModel colorSpaceType=[colorSpace type];
   float            output[O2ColorSpaceGetNumberOfComponents(colorSpace)+1];
   void           (*outputToRGBA)(float *,float *);
   float            rgba[4];

   switch(colorSpaceType){

    case kO2ColorSpaceModelMonochrome:
     outputToRGBA=GrayAToRGBA;
     break;
     
    case kO2ColorSpaceModelRGB:
     outputToRGBA=RGBAToRGBA;
     break;
     
    case kO2ColorSpaceModelCMYK:
     outputToRGBA=CMYKAToRGBA;
     break;
     
    default:
     NSLog(@"shading can't deal with colorspace %@",colorSpace);
     return nil;
   }

   _numberOfColorStops=numberOfSamples;
   
   _colorStops=NSZoneMalloc(NULL,_numberOfColorStops*sizeof(GradientStop));
   int i;
   
   isOpaque=TRUE;
   
   for(i=0;i<_numberOfColorStops;i++){
    _colorStops[i].offset=(O2Float)i/(O2Float)(_numberOfColorStops-1);

// FIXME: This assumes range=0..1, we need to map this to the functions range

    O2FunctionEvaluate(function,_colorStops[i].offset,output);
    outputToRGBA(output,rgba);
    
    if(rgba[3]<1.0f)
     isOpaque=FALSE;
     
    _colorStops[i].color32f=O2argb32fPremultiply(O2argb32fInit(rgba[0],rgba[1],rgba[2],rgba[3]));
    _colorStops[i].color8u=O2argb8uFromO2argb32f(_colorStops[i].color32f);
   }

   return self;
}

-(void)dealloc {
   if(_colorStops!=NULL)
    NSZoneFree(NULL,_colorStops);
   
   [super dealloc];
}

/*-------------------------------------------------------------------*//*!
* \brief	Returns the average color within an offset range in the color ramp.
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

ONYX2D_STATIC_INLINE O2argb32f readStopColor(GradientStop *colorRampStops, int i) {
	return  colorRampStops[i].color32f;
}

O2argb32f O2PaintIntegrateColorRamp(O2Paint_ramp *self,O2Float gmin, O2Float gmax) { 
	O2argb32f c=O2argb32fInit(0,0,0,0);
    
	if(gmin == 1.0f || gmax == 0.0f)
		return c;

   int i=RI_FLOOR_TO_INT(gmin*(self->_numberOfColorStops-1));
   
   for(;i<self->_numberOfColorStops-1;i++) {
    if(gmin >= self->_colorStops[i].offset && gmin < self->_colorStops[i+1].offset) {
     O2Float s = self->_colorStops[i].offset;
     O2Float e = self->_colorStops[i+1].offset;
     RI_ASSERT(s < e);
     O2Float g = (gmin - s) / (e - s);

     O2argb32f sc = readStopColor(self->_colorStops, i);
     O2argb32f ec = readStopColor(self->_colorStops, i+1);
     O2argb32f rc = O2argb32fAdd(O2argb32fMultiplyByFloat(sc, (1.0f-g)),O2argb32fMultiplyByFloat(ec , g));

     //subtract the average color from the start of the stop to gmin
     c=O2argb32fSubtract(c,O2argb32fMultiplyByFloat(O2argb32fAdd(sc,rc) , 0.5f*(gmin - s)));
     break;
		  }
   }

	for(;i<self->_numberOfColorStops-1;i++) {
		O2Float s = self->_colorStops[i].offset;
		O2Float e = self->_colorStops[i+1].offset;
		RI_ASSERT(s <= e);

		O2argb32f sc = readStopColor(self->_colorStops, i);
		O2argb32f ec = readStopColor(self->_colorStops, i+1);

		//average of the stop
		c=O2argb32fAdd(c , O2argb32fMultiplyByFloat(O2argb32fAdd(sc , ec), 0.5f*(e-s)));

		if(gmax >= self->_colorStops[i].offset && gmax < self->_colorStops[i+1].offset) {
			O2Float g = (gmax - s) / (e - s);
			O2argb32f rc = O2argb32fAdd(O2argb32fMultiplyByFloat(sc , (1.0f-g)),O2argb32fMultiplyByFloat( ec , g));

			//subtract the average color from gmax to the end of the stop
			c=O2argb32fSubtract(c,O2argb32fMultiplyByFloat(O2argb32fAdd(rc , ec) , 0.5f*(e - gmax)));
			break;
		}
	}

	return c;
}

// We already sample a lot, this is excessive
O2argb32f O2PaintColorRamp(O2Paint_ramp *self,O2Float gradient, O2Float rho,int *skip)  {
	RI_ASSERT(self);
	RI_ASSERT(rho >= 0.0f);

	O2argb32f c=O2argb32fInit(0,0,0,0);

	if(rho == 0.0f) {	//filter size is zero or gradient is degenerate

			gradient = RI_CLAMP(gradient, 0.0f, 1.0f);

        int i;
		for(i=0;i<self->_numberOfColorStops-1;i++) {
			if(gradient >= self->_colorStops[i].offset && gradient < self->_colorStops[i+1].offset)
			{
				O2Float s = self->_colorStops[i].offset;
				O2Float e = self->_colorStops[i+1].offset;
				RI_ASSERT(s < e);
				O2Float g = RI_CLAMP((gradient - s) / (e - s), 0.0f, 1.0f);	//clamp needed due to numerical inaccuracies

				O2argb32f sc = readStopColor(self->_colorStops, i);
				O2argb32f ec = readStopColor(self->_colorStops, i+1);
				return O2argb32fAdd(O2argb32fMultiplyByFloat(sc , (1.0f-g)) , O2argb32fMultiplyByFloat(ec , g));	//return interpolated value
			}
		}
		return readStopColor(self->_colorStops, self->_numberOfColorStops-1);
	}

	O2Float gmin = gradient - rho*0.5f;			//filter starting from the gradient point (if starts earlier, radial gradient center will be an average of the first and the last stop, which doesn't look good)
	O2Float gmax = gradient + rho*0.5f;

    if(gmin<0.0f){
     *skip=0;
    c=O2argb32fMultiplyByFloat(readStopColor(self->_colorStops, 0), (RI_MIN(gmax, 0.0f) - gmin));
    }
    if(gmax>1.0f){
     *skip=0;
    c=O2argb32fMultiplyByFloat(readStopColor(self->_colorStops, self->_numberOfColorStops-1) , (gmax - RI_MAX(gmin, 1.0f)));
    }

		gmin = RI_CLAMP(gmin, 0.0f, 1.0f);
		gmax = RI_CLAMP(gmax, 0.0f, 1.0f);
		c=O2argb32fAdd(c, O2PaintIntegrateColorRamp(self,gmin, gmax));
		c=O2argb32fMultiplyByFloat(c , 1.0f/rho);
		c=O2argb32fClamp(c);	//clamp needed due to numerical inaccuracies

   return c;
}

@end
