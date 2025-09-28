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

#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2DataProvider.h>

@implementation O2Surface

#define RI_MAX_GAUSSIAN_STD_DEVIATION	128.0f

/*-------------------------------------------------------------------*//*!
* \brief	Converts from the current internal format to another.
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

	//From Section 3.4.2 of OpenVG 1.0.1 spec
	//1: sRGB = gamma(lRGB)
	//2: lRGB = invgamma(sRGB)
	//3: lL = 0.2126 lR + 0.7152 lG + 0.0722 lB
	//4: lRGB = lL
	//5: sL = gamma(lL)
	//6: lL = invgamma(sL)
	//7: sRGB = sL

	//Source/Dest lRGB sRGB   lL   sL 
	//lRGB          —    1    3    3,5 
	//sRGB          2    —    2,3  2,3,5 
	//lL            4    4,1  —    5 
	//sL            7,2  7    6    — 

#if 0
// Can't use 'gamma' ?
static O2Float dogamma(O2Float c)
{    
	if( c <= 0.00304f )
		c *= 12.92f;
	else
		c = 1.0556f * (O2Float)pow(c, 1.0f/2.4f) - 0.0556f;
	return c;
}

static O2Float invgamma(O2Float c)
{
	if( c <= 0.03928f )
		c /= 12.92f;
	else
		c = (O2Float)pow((c + 0.0556f)/1.0556f, 2.4f);
	return c;
}

#endif
static O2Float lRGBtoL(O2Float r, O2Float g, O2Float b)
{
	return 0.2126f*r + 0.7152f*g + 0.0722f*b;
}

static void colorToBytesLittle(O2Float color,uint8_t *scanline){
   union {
    unsigned char bytes[4];
    float         f;
   } u;
   
   u.f=color;
   
#ifdef __LITTLE_ENDIAN__   
   scanline[0]=u.bytes[0];
   scanline[1]=u.bytes[1];
   scanline[2]=u.bytes[2];
   scanline[3]=u.bytes[3];
#else
   scanline[3]=u.bytes[0];
   scanline[2]=u.bytes[1];
   scanline[1]=u.bytes[2];
   scanline[0]=u.bytes[3];
#endif
}

static void O2SurfaceWrite_argb32f_to_argb32fLittle(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*16;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    colorToBytesLittle(rgba.r,scanline);
    scanline+=4;
    colorToBytesLittle(rgba.g,scanline);
    scanline+=4;
    colorToBytesLittle(rgba.b,scanline);
    scanline+=4;
    colorToBytesLittle(rgba.a,scanline);
    scanline+=4;
   }
}

static void colorToBytesBig(O2Float color,uint8_t *scanline){
   union {
    unsigned char bytes[4];
    float         f;
   } u;
   
   u.f=color;
   
#ifdef __BIG_ENDIAN__   
   scanline[0]=u.bytes[0];
   scanline[1]=u.bytes[1];
   scanline[2]=u.bytes[2];
   scanline[3]=u.bytes[3];
#else
   scanline[3]=u.bytes[0];
   scanline[2]=u.bytes[1];
   scanline[1]=u.bytes[2];
   scanline[0]=u.bytes[3];
#endif
}

static void O2SurfaceWrite_argb32f_to_argb32fBig(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*16;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    colorToBytesBig(rgba.r,scanline);
    scanline+=4;
    colorToBytesBig(rgba.g,scanline);
    scanline+=4;
    colorToBytesBig(rgba.b,scanline);
    scanline+=4;
    colorToBytesBig(rgba.a,scanline);
    scanline+=4;
   }
}

static unsigned char colorToNibble(O2Float c){
	return RI_INT_MIN(RI_INT_MAX(RI_FLOOR_TO_INT(c * (O2Float)0xF + 0.5f), 0), 0xF);
}

static void O2SurfaceWrite_argb32f_to_GA88(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*2;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=O2ByteFromFloat(rgba.r);
    *scanline++=O2ByteFromFloat(rgba.a);
   }
}

static void O2SurfaceWrite_argb32f_to_G8(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=O2ByteFromFloat(lRGBtoL(rgba.r,rgba.g,rgba.b));
   }
}


static void O2SurfaceWrite_argb32f_to_argb8u(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=O2ByteFromFloat(rgba.r);
    *scanline++=O2ByteFromFloat(rgba.g);
    *scanline++=O2ByteFromFloat(rgba.b);
    *scanline++=O2ByteFromFloat(rgba.a);
   }
}

static void O2SurfaceWrite_argb8u_to_argb8u(O2Surface *self,int x,int y,O2argb8u *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb8u rgba=*span++;

    *scanline++=rgba.r;
    *scanline++=rgba.g;
    *scanline++=rgba.b;
    *scanline++=rgba.a;
   }
}

static void O2SurfaceWrite_argb32f_to_ABGR8888(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=O2ByteFromFloat(rgba.a);
    *scanline++=O2ByteFromFloat(rgba.b);
    *scanline++=O2ByteFromFloat(rgba.g);
    *scanline++=O2ByteFromFloat(rgba.r);
   }
}

static void O2SurfaceWrite_argb8u_to_ABGR8888(O2Surface *self,int x,int y,O2argb8u *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb8u rgba=*span++;

    *scanline++=rgba.a;
    *scanline++=rgba.b;
    *scanline++=rgba.g;
    *scanline++=rgba.r;
   }
}

static void O2SurfaceWrite_argb8u_to_BGRA8888(O2Surface *self,int x,int y,O2argb8u *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb8u rgba=*span++;

    *scanline++=rgba.b;
    *scanline++=rgba.g;
    *scanline++=rgba.r;
    *scanline++=rgba.a;
   }
}

static void O2SurfaceWrite_argb32f_to_RGBA4444(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*2;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=colorToNibble(rgba.r)<<4|colorToNibble(rgba.g);
    *scanline++=colorToNibble(rgba.b)<<4|colorToNibble(rgba.a);
   }
}

static void O2SurfaceWrite_argb32f_to_BARG4444(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*2;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=colorToNibble(rgba.b)<<4|colorToNibble(rgba.a);
    *scanline++=colorToNibble(rgba.r)<<4|colorToNibble(rgba.g);
   }
}

static void O2SurfaceWrite_argb32f_to_RGBA2222(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=colorToNibble(rgba.a)<<6|colorToNibble(rgba.a)<<6|colorToNibble(rgba.a)<<2|colorToNibble(rgba.b);
   }
}

static void O2SurfaceWrite_argb32f_to_CMYK8888(O2Surface *self,int x,int y,O2argb32f *span,int length){
   uint8_t* scanline = self->_pixelBytes + y * self->_bytesPerRow;
   int i;
   
   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    *scanline++=O2ByteFromFloat(1.0-rgba.r);
    *scanline++=O2ByteFromFloat(1.0-rgba.g);
    *scanline++=O2ByteFromFloat(1.0-rgba.b);
    *scanline++=O2ByteFromFloat(0);
   }
}

static void O2SurfaceWrite_argb32f_to_argb8u_to_ANY(O2Surface *self,int x,int y,O2argb32f *span,int length){
   O2argb8u span8888[length];
   int i;
   
   for(i=0;i<length;i++){
    O2argb32f rgba=*span++;

    span8888[i].r=O2ByteFromFloat(rgba.r);
    span8888[i].g=O2ByteFromFloat(rgba.g);
    span8888[i].b=O2ByteFromFloat(rgba.b);
    span8888[i].a=O2ByteFromFloat(rgba.a);
   }
   self->_writeargb8u(self,x,y,span8888,length);
}

static BOOL initFunctionsForParameters(O2Surface *self,size_t bitsPerComponent,size_t bitsPerPixel,O2ColorSpaceRef colorSpace,O2BitmapInfo bitmapInfo){
   self->_writeargb32f=O2SurfaceWrite_argb32f_to_argb8u_to_ANY;// default
   
   switch(bitsPerComponent){
   
    case 32:
     switch(bitsPerPixel){
      case 32:
       break;
      case 128:
       switch(bitmapInfo&kO2BitmapByteOrderMask){
        case kO2BitmapByteOrderDefault:
        case kO2BitmapByteOrder16Little:
        case kO2BitmapByteOrder32Little:
         self->_writeargb32f=O2SurfaceWrite_argb32f_to_argb32fLittle;
         return YES;
         
        case kO2BitmapByteOrder16Big:
        case kO2BitmapByteOrder32Big:
         self->_writeargb32f=O2SurfaceWrite_argb32f_to_argb32fBig;
         return YES;
       }
     }
     break;
     
    case  8:
     switch(bitsPerPixel){
     
      case 8:
       self->_writeargb32f=O2SurfaceWrite_argb32f_to_G8;
       return YES;

      case 16:
       self->_writeargb32f=O2SurfaceWrite_argb32f_to_GA88;
       return YES;

      case 24:
       break;
       
      case 32:
       if([colorSpace type]==kO2ColorSpaceModelRGB){

        switch(bitmapInfo&kO2BitmapAlphaInfoMask){
         case kO2ImageAlphaNone:
          break;
          
         case kO2ImageAlphaLast:
         case kO2ImageAlphaPremultipliedLast:
          switch(bitmapInfo&kO2BitmapByteOrderMask){
           case kO2BitmapByteOrderDefault:
           case kO2BitmapByteOrder16Little:
           case kO2BitmapByteOrder32Little:
            self->_writeargb32f=O2SurfaceWrite_argb32f_to_ABGR8888;
            self->_writeargb8u=O2SurfaceWrite_argb8u_to_ABGR8888;
            return YES;

           case kO2BitmapByteOrder16Big:
           case kO2BitmapByteOrder32Big:
            self->_writeargb32f=O2SurfaceWrite_argb32f_to_argb8u;
            self->_writeargb8u=O2SurfaceWrite_argb8u_to_argb8u;
            return YES;
          }

          break;
          
         case kO2ImageAlphaPremultipliedFirst:
          switch(bitmapInfo&kO2BitmapByteOrderMask){
           case kO2BitmapByteOrderDefault:
           case kO2BitmapByteOrder16Little:
           case kO2BitmapByteOrder32Little:
            self->_writeargb8u=O2SurfaceWrite_argb8u_to_BGRA8888;
            return YES;
          }
          break;
                    
         case kO2ImageAlphaFirst:
          break;
          
         case kO2ImageAlphaNoneSkipLast:
          break;
          
         case kO2ImageAlphaNoneSkipFirst:
          break;
        }
       }
       else if([colorSpace type]==kO2ColorSpaceModelCMYK){
        switch(bitmapInfo&kO2BitmapByteOrderMask){
         case kO2BitmapByteOrderDefault:
         case kO2BitmapByteOrder16Little:
         case kO2BitmapByteOrder32Little:
          break;
         
         case kO2BitmapByteOrder16Big:
         case kO2BitmapByteOrder32Big:
          self->_writeargb32f=O2SurfaceWrite_argb32f_to_CMYK8888;
         return YES;
        }
       }
       break;
     }
     break;
     
    case  4:
     switch(bitsPerPixel){
      case 4:
       break;
      case 12:
       break;
      case 16:
       switch(bitmapInfo&kO2BitmapByteOrderMask){
        case kO2BitmapByteOrderDefault:
        case kO2BitmapByteOrder16Little:
        case kO2BitmapByteOrder32Little:
         self->_writeargb32f=O2SurfaceWrite_argb32f_to_BARG4444;
         return YES;
         
        case kO2BitmapByteOrder16Big:
        case kO2BitmapByteOrder32Big:
         self->_writeargb32f=O2SurfaceWrite_argb32f_to_RGBA4444;
         return YES;
       }
     }
     break;
     
    case  2:
     switch(bitsPerPixel){
      case 2:
       break;
      case 6:
       break;
      case 8:
       self->_writeargb32f=O2SurfaceWrite_argb32f_to_RGBA2222;
       return YES;
     }
     break;

    case  1:
     switch(bitsPerPixel){
      case 1:
       //  self->_writeargb32f=O2SurfaceWriteSpan_largb32f_PRE_01;
       return YES;
       
      case 3:
       break;
     }
     break;
   }
   return NO;   
}

-initWithBytes:(void *)bytes width:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bytesPerRow:(size_t)bytesPerRow colorSpace:(O2ColorSpaceRef)colorSpace bitmapInfo:(O2BitmapInfo)bitmapInfo {
   O2DataProvider *provider;
   int bitsPerPixel=32;
   
   if(bytes!=NULL){
    provider=[[[O2DataProvider alloc] initWithBytes:bytes length:bytesPerRow*height] autorelease];
    m_ownsData=NO;
   }
   else {
    if(bytesPerRow>0 && bytesPerRow<(width*bitsPerPixel)/8){
     NSLog(@"invalid bytes per row=%zu",bytesPerRow);
     bytesPerRow=0;
    }
    
    if(bytesPerRow==0)
     bytesPerRow=(width*bitsPerPixel)/8;
     
    NSMutableData *data=[NSMutableData dataWithLength:bytesPerRow*height*sizeof(uint8_t)]; // this will also zero the bytes
    provider=[O2DataProviderCreateWithCFData((CFDataRef)data) autorelease];
  	m_ownsData=YES;
   }
   
    if([super initWithWidth:width height:height bitsPerComponent:bitsPerComponent bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow colorSpace:colorSpace bitmapInfo:bitmapInfo decoder: NULL provider:provider decode:NULL interpolate:YES renderingIntent:kO2RenderingIntentDefault]==nil)
    return nil;
   
   if([provider isDirectAccess])
    _pixelBytes=(void *)[provider bytes];

   if(!initFunctionsForParameters(self,bitsPerComponent,_bitsPerPixel,colorSpace,bitmapInfo))
    NSLog(@"O2Surface -init error, return");

   _clampExternalPixels=NO; // only set to yes if premultiplied
   pthread_mutex_init(&_lock,NULL);
   return self;
}

-(void)dealloc {
    _pixelBytes=NULL; // if we own it, it's in the provider, if not, no release
    pthread_mutex_destroy(&_lock);
    
    [super dealloc];
}

void O2SurfaceLock(O2Surface *surface) {
   pthread_mutex_lock(&(surface->_lock));
}

void O2SurfaceUnlock(O2Surface *surface) {
   pthread_mutex_unlock(&(surface->_lock));
}

-(void *)pixelBytes {
   return _pixelBytes;
}

-(void)setWidth:(size_t)width height:(size_t)height reallocateOnlyIfRequired:(BOOL)roir {

   if(!m_ownsData)
    return;

   _width=width;
   _height=height;
   _bytesPerRow=width*_bitsPerPixel/8;
   
   NSUInteger size=_bytesPerRow*height*sizeof(uint8_t);
   NSUInteger allocateSize=[[_provider data] length];
   
   if((size>allocateSize) || (!roir && size!=allocateSize)){
    [_provider release];
    
    NSMutableData *data=[NSMutableData dataWithLength:size];
       _provider=O2DataProviderCreateWithCFData((CFDataRef)data);
    _pixelBytes=[data mutableBytes];
   }
}

void *O2SurfaceGetPixelBytes(O2Surface *surface) {
  return surface->_pixelBytes;
}

size_t O2SurfaceGetWidth(O2Surface *surface) {
  return surface->_width;
}

size_t O2SurfaceGetHeight(O2Surface *surface) {
  return surface->_height;
}

size_t O2SurfaceGetBytesPerRow(O2Surface *surface) {
   return surface->_bytesPerRow;
}


O2ImageRef O2SurfaceCreateImage(O2Surface *self) {
   NSData           *data=[[NSData alloc] initWithBytes:self->_pixelBytes length:self->_bytesPerRow*self->_height];
   O2DataProviderRef provider=O2DataProviderCreateWithCFData((CFDataRef)data);
  
  O2Image *result=O2ImageCreate(self->_width,self->_height,self->_bitsPerComponent,self->_bitsPerPixel,self->_bytesPerRow,self->_colorSpace,
     self->_bitmapInfo,provider,self->_decode,self->_interpolate,self->_renderingIntent);
  
  O2DataProviderRelease(provider);
  [data release];
  
  return result;
}

void O2SurfaceWriteSpan_argb8u_PRE(O2Surface *self,int x,int y,O2argb8u *span,int length) {   
   if(length==0)
    return;
   
   self->_writeargb8u(self,x,y,span,length);
}

void O2SurfaceWriteSpan_largb32f_PRE(O2Surface *self,int x,int y,O2argb32f *span,int length) {   
   if(length==0)
    return;

   self->_writeargb32f(self,x,y,span,length);
}


/*-------------------------------------------------------------------*//*!
* \brief	Applies Gaussian blur filter.
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

static O2argb32f gaussianReadPixel(int x, int y, int w, int h,O2argb32f *image)
{
	if(x < 0 || x >= w || y < 0 || y >= h) {	//apply tiling mode
	 return O2argb32fInit(0,0,0,0);
	}
	else
	{
		RI_ASSERT(x >= 0 && x < w && y >= 0 && y < h);
		return image[y*w+x];
	}
}

typedef struct O2GaussianKernel {
 int      refCount;
 int      xSize;
 int      xShift;
 O2Float  xScale;
 O2Float *xValues;

 int      ySize;
 int      yShift;
 O2Float  yScale;
 O2Float *yValues;
} O2GaussianKernel;

O2GaussianKernel *O2CreateGaussianKernelWithDeviation(O2Float stdDeviation){
   O2GaussianKernel *kernel=NSZoneMalloc(NULL,sizeof(O2GaussianKernel));
   
   kernel->refCount=1;
   
   O2Float stdDeviationX=stdDeviation;
   O2Float stdDeviationY=stdDeviation;
   
	RI_ASSERT(stdDeviationX > 0.0f && stdDeviationY > 0.0f);
	RI_ASSERT(stdDeviationX <= RI_MAX_GAUSSIAN_STD_DEVIATION && stdDeviationY <= RI_MAX_GAUSSIAN_STD_DEVIATION);
       
	//find a size for the kernel
	O2Float totalWeightX = stdDeviationX*(O2Float)sqrt(2.0f*M_PI);
	O2Float totalWeightY = stdDeviationY*(O2Float)sqrt(2.0f*M_PI);
	const O2Float tolerance = 0.99f;	//use a kernel that covers 99% of the total Gaussian support

	O2Float expScaleX = -1.0f / (2.0f*stdDeviationX*stdDeviationX);
	O2Float expScaleY = -1.0f / (2.0f*stdDeviationY*stdDeviationY);

	int kernelWidth = 0;
	O2Float e = 0.0f;
	O2Float sumX = 1.0f;	//the weight of the middle entry counted already
	do{
		kernelWidth++;
		e = (O2Float)exp((O2Float)(kernelWidth * kernelWidth) * expScaleX);
		sumX += e*2.0f;	//count left&right lobes
	}while(sumX < tolerance*totalWeightX);

	int kernelHeight = 0;
	e = 0.0f;
	O2Float sumY = 1.0f;	//the weight of the middle entry counted already
	do{
		kernelHeight++;
		e = (O2Float)exp((O2Float)(kernelHeight * kernelHeight) * expScaleY);
		sumY += e*2.0f;	//count left&right lobes
	}while(sumY < tolerance*totalWeightY);

	//make a separable kernel
    kernel->xSize=kernelWidth*2+1;
    kernel->xValues=NSZoneMalloc(NULL,sizeof(O2Float)*kernel->xSize);
    kernel->xShift = kernelWidth;
    kernel->xScale = 0.0f;
    int i;
	for(i=0;i<kernel->xSize;i++){
		int x = i-kernel->xShift;
		kernel->xValues[i] = (O2Float)exp((O2Float)x*(O2Float)x * expScaleX);
		kernel->xScale += kernel->xValues[i];
	}
	kernel->xScale = 1.0f / kernel->xScale;	//NOTE: using the mathematical definition of the scaling term doesn't work since we cut the filter support early for performance

    kernel->ySize=kernelHeight*2+1;
    kernel->yValues=NSZoneMalloc(NULL,sizeof(O2Float)*kernel->ySize);
    kernel->yShift = kernelHeight;
    kernel->yScale = 0.0f;
	for(i=0;i<kernel->ySize;i++)
	{
		int y = i-kernel->yShift;
		kernel->yValues[i] = (O2Float)exp((O2Float)y*(O2Float)y * expScaleY);
		kernel->yScale += kernel->yValues[i];
	}
	kernel->yScale = 1.0f / kernel->yScale;	//NOTE: using the mathematical definition of the scaling term doesn't work since we cut the filter support early for performance
    
    return kernel;
}

O2GaussianKernelRef O2GaussianKernelRetain(O2GaussianKernelRef kernel) {
   if(kernel!=NULL)
    kernel->refCount++;
    
   return kernel;
}

void O2GaussianKernelRelease(O2GaussianKernelRef kernel) {
   if(kernel!=NULL){
    kernel->refCount--;
    if(kernel->refCount<=0){
     NSZoneFree(NULL,kernel->xValues);
     NSZoneFree(NULL,kernel->yValues);
     NSZoneFree(NULL,kernel);
    }
   }
}

static O2argb32f argbFromColor(O2ColorRef color){   
   size_t    count=O2ColorGetNumberOfComponents(color);
   const float *components=O2ColorGetComponents(color);

   if(count==2)
    return O2argb32fInit(components[0],components[0],components[0],components[1]);
   if(count==4)
    return O2argb32fInit(components[0],components[1],components[2],components[3]);
    
   return O2argb32fInit(1,0,0,1);
}

void O2SurfaceGaussianBlur(O2Surface *self,O2Image * src, O2GaussianKernel *kernel,O2ColorRef color){
   O2argb32f argbColor=argbFromColor(color);
   
	//the area to be written is an intersection of source and destination image areas.
	//lower-left corners of the images are aligned.
	int w = RI_INT_MIN(self->_width, src->_width);
	int h = RI_INT_MIN(self->_height, src->_height);
	RI_ASSERT(w > 0 && h > 0);
    
	O2argb32f *tmp=NSZoneMalloc(NULL,src->_width*src->_height*sizeof(O2argb32f));

	//copy source region to tmp and do conversion
    int i,j;
	for(j=0;j<src->_height;j++){
     O2argb32f *tmpRow=tmp+j*src->_width;
     int         i,width=src->_width;
     O2argb32f *direct=O2Image_read_argb32f(src,0,j,tmpRow,width);
     
     if(direct!=NULL){
      for(i=0;i<width;i++)
       tmpRow[i]=direct[i];
     }
     for(i=0;i<width;i++){
      tmpRow[i].a=argbColor.a*tmpRow[i].a;
      tmpRow[i].r=argbColor.r*tmpRow[i].a;
      tmpRow[i].g=argbColor.g*tmpRow[i].a;
      tmpRow[i].b=argbColor.b*tmpRow[i].a;
     }
     
  	}

	O2argb32f *tmp2=NSZoneMalloc(NULL,w*src->_height*sizeof(O2argb32f));

	//horizontal pass
	for(j=0;j<src->_height;j++){
		for(i=0;i<w;i++){
			O2argb32f sum=O2argb32fInit(0,0,0,0);
            int ki;
			for(ki=0;ki<kernel->xSize;ki++){
				int x = i+ki-kernel->xShift;
				sum=O2argb32fAdd(sum, O2argb32fMultiplyByFloat(gaussianReadPixel(x, j, src->_width, src->_height, tmp),kernel->xValues[ki]));
			}
			tmp2[j*w+i] = O2argb32fMultiplyByFloat(sum, kernel->xScale);
		}
	}
	//vertical pass
	for(j=0;j<h;j++){
		for(i=0;i<w;i++){
			O2argb32f sum=O2argb32fInit(0,0,0,0);
            int kj;
			for(kj=0;kj<kernel->ySize;kj++){
				int y = j+kj-kernel->yShift;
				sum=O2argb32fAdd(sum,  O2argb32fMultiplyByFloat(gaussianReadPixel(i, y, w, src->_height, tmp2), kernel->yValues[kj]));
			}
            sum=O2argb32fMultiplyByFloat(sum, kernel->yScale);
			O2SurfaceWriteSpan_largb32f_PRE(self,i, j, &sum,1);
		}
	}
    NSZoneFree(NULL,tmp);
    NSZoneFree(NULL,tmp2);
}

@end
