/*------------------------------------------------------------------------
 *
 * Derivative of the OpenVG 1.0.1 Reference Implementation
 * -------------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
 * Copyright (c) 2008 Christopher J. W. Lloyd
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

#import <Onyx2D/O2Image.h>
#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2Exceptions.h>
#import <Foundation/NSData.h>

@implementation O2Image

ONYX2D_STATIC BOOL initFunctionsForMonochrome(O2Image *self,size_t bitsPerComponent,size_t bitsPerPixel,O2BitmapInfo bitmapInfo){
   switch(bitsPerComponent){
    case 8:
     switch(bitsPerPixel){
      case 8:
       self->_read_a8u=O2ImageRead_G8_to_A8;
       self->_read_argb8u=O2ImageRead_G8_to_argb8u;
       return YES;
     }
     break;
     
   }
   return NO;
}


ONYX2D_STATIC BOOL initFunctionsForRGBColorSpace(O2Image *self,size_t bitsPerComponent,size_t bitsPerPixel,O2BitmapInfo bitmapInfo){   

   switch(bitsPerComponent){
   
    case 32:
     switch(bitsPerPixel){
      case 32:
       break;
      case 128:
       switch(bitmapInfo&kO2BitmapByteOrderMask){
        case kO2BitmapByteOrder16Little:
        case kO2BitmapByteOrder32Little:
         self->_read_argb32f=O2ImageRead_argb32fLittle_to_argb32f;
         return YES;
         
        case kO2BitmapByteOrder16Big:
        case kO2BitmapByteOrder32Big:
         self->_read_argb32f=O2ImageRead_argb32fBig_to_argb32f;
         return YES;
       }
     }
     break;
        
    case  8:
     switch(bitsPerPixel){
     
      case 8:
       self->_read_a8u=O2ImageRead_G8_to_A8;
       self->_read_argb8u=O2ImageRead_G8_to_argb8u;
       return YES;
      
      case 16:
       self->_read_argb8u=O2ImageRead_GA88_to_argb8u;
       return YES;
       
      case 24:
       switch(bitmapInfo&kO2BitmapAlphaInfoMask){
        case kO2ImageAlphaNone:
			   switch(bitmapInfo&kO2BitmapByteOrderMask){
				   case kO2BitmapByteOrder16Little:
				   case kO2BitmapByteOrder32Little:
					   self->_read_argb8u=O2ImageRead_ABGR8888_to_argb8u;
					   return YES;
					   
				   case kO2BitmapByteOrder16Big:
				   case kO2BitmapByteOrder32Big:
					   self->_read_argb8u=O2ImageRead_RGBA8888_to_argb8u;
					   return YES;
			   }
			   return YES;
       }
       break;
       
      case 32:
        switch(bitmapInfo&kO2BitmapAlphaInfoMask){
         case kO2ImageAlphaNone:
          break;
          
         case kO2ImageAlphaLast:
         case kO2ImageAlphaPremultipliedLast:
          switch(bitmapInfo&kO2BitmapByteOrderMask){
           case kO2BitmapByteOrder16Little:
           case kO2BitmapByteOrder32Little:
            self->_read_argb8u=O2ImageRead_ABGR8888_to_argb8u;
            return YES;

           case kO2BitmapByteOrder16Big:
           case kO2BitmapByteOrder32Big:
            self->_read_argb8u=O2ImageRead_RGBA8888_to_argb8u;
            return YES;
          }
          break;

         case kO2ImageAlphaPremultipliedFirst:
          switch(bitmapInfo&kO2BitmapByteOrderMask){
           case kO2BitmapByteOrder16Little:
           case kO2BitmapByteOrder32Little:
            self->_read_argb8u=O2ImageRead_BGRA8888_to_argb8u;
            return YES;
          }
          break;
                    
         case kO2ImageAlphaFirst:
          break;
          
         case kO2ImageAlphaNoneSkipLast:
          switch(bitmapInfo&kO2BitmapByteOrderMask){
           case kO2BitmapByteOrder16Little:
           case kO2BitmapByteOrder32Little:
            self->_read_argb8u=O2ImageRead_ABGR8888_to_argb8u;
            return YES;

           case kO2BitmapByteOrder16Big:
           case kO2BitmapByteOrder32Big:
            self->_read_argb8u=O2ImageRead_RGBA8888_to_argb8u;
            return YES;
          }
          break;
          
         case kO2ImageAlphaNoneSkipFirst:
          switch(bitmapInfo&kO2BitmapByteOrderMask){
           
           case kO2BitmapByteOrder16Little:
           case kO2BitmapByteOrder32Little:
            self->_read_argb8u=O2ImageRead_BGRX8888_to_argb8u;
            return YES;

           case kO2BitmapByteOrder16Big:
           case kO2BitmapByteOrder32Big:
            self->_read_argb8u=O2ImageRead_XRGB8888_to_argb8u;
            return YES;
          }
          break;
        }
              
        break;
     }
     break;
    
    case 5:
     switch(bitsPerPixel){
     
      case 16:
       if(bitmapInfo==(kO2BitmapByteOrder16Little|kO2ImageAlphaNoneSkipFirst)){
        self->_read_argb8u=O2ImageRead_G3B5X1R5G2_to_argb8u;
        return YES;
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
        case kO2BitmapByteOrder16Little:
        case kO2BitmapByteOrder32Little:
         self->_read_argb8u=O2ImageRead_BARG4444_to_argb8u;
         return YES;
         
        case kO2BitmapByteOrder16Big:
        case kO2BitmapByteOrder32Big:
         self->_read_argb8u=O2ImageRead_RGBA4444_to_argb8u;
         return YES;
       }

       return YES;
     }
     break;
     
    case  2:
     switch(bitsPerPixel){
      case 2:
       break;
      case 6:
       break;
      case 8:
       self->_read_argb8u=O2ImageRead_RGBA2222_to_argb8u;
       return YES;
     }
     break;

    case  1:
     switch(bitsPerPixel){
      case 1:
    //   self->_read_argb32f=O2ImageReadPixelSpan_01;
     //  return YES;
       
      case 3:
       break;
     }
     break;
   }
   return NO;
      
}

ONYX2D_STATIC BOOL initFunctionsForCMYKColorSpace(O2Image *self,size_t bitsPerComponent,size_t bitsPerPixel,O2BitmapInfo bitmapInfo){   
   switch(bitsPerComponent){
        
    case  8:
     switch(bitsPerPixel){
     
      case 32:
        switch(bitmapInfo&kO2BitmapByteOrderMask){
         case kO2BitmapByteOrder16Big:
         case kO2BitmapByteOrder32Big:
          self->_read_argb8u=O2ImageRead_CMYK8888_to_argb8u;
          return YES;
        }
       break;
     }
     break;
   }
   return NO;
}

ONYX2D_STATIC BOOL initFunctionsForIndexedColorSpace(O2Image *self,size_t bitsPerComponent,size_t bitsPerPixel,O2ColorSpaceRef colorSpace,O2BitmapInfo bitmapInfo){

   switch([[(O2ColorSpace_indexed *)colorSpace baseColorSpace] type]){

    case kO2ColorSpaceModelRGB:
     self->_read_argb8u=O2ImageRead_I8_to_argb8u;
     return YES;
    
    default:
     return NO;
   }
}

ONYX2D_STATIC BOOL initFunctionsForParameters(O2Image *self,size_t bitsPerComponent,size_t bitsPerPixel,O2ColorSpaceRef colorSpace,O2BitmapInfo bitmapInfo){

   self->_read_a8u=O2Image_read_a8u_src_argb8u;
   self->_read_a32f=O2ImageRead_ANY_to_A8_to_Af;
   self->_read_argb32f=O2ImageRead_ANY_to_argb8u_to_argb32f;

	if((bitmapInfo&kO2BitmapByteOrderMask)==kO2BitmapByteOrderDefault) {
#ifdef __LITTLE_ENDIAN__
		bitmapInfo|=kO2BitmapByteOrder32Little;
#else
		bitmapInfo|=kO2BitmapByteOrder32Big;
#endif
	}
	
   switch([colorSpace type]){
    case kO2ColorSpaceModelMonochrome:
     return initFunctionsForMonochrome(self,bitsPerComponent,bitsPerPixel,bitmapInfo);
    case kO2ColorSpaceModelRGB:
     return initFunctionsForRGBColorSpace(self,bitsPerComponent,bitsPerPixel,bitmapInfo);
    case kO2ColorSpaceModelCMYK:
     return initFunctionsForCMYKColorSpace(self,bitsPerComponent,bitsPerPixel,bitmapInfo);
    case kO2ColorSpaceModelIndexed:
     return initFunctionsForIndexedColorSpace(self,bitsPerComponent,bitsPerPixel,colorSpace,bitmapInfo);
    default:
     return NO;
   }
   
      
}

-initWithWidth:(size_t)width
        height:(size_t)height
bitsPerComponent:(size_t)bitsPerComponent
  bitsPerPixel:(size_t)bitsPerPixel
   bytesPerRow:(size_t)bytesPerRow
    colorSpace:(O2ColorSpaceRef)colorSpace
    bitmapInfo:(O2BitmapInfo)bitmapInfo
       decoder:(O2ImageDecoder *)decoder
      provider:(O2DataProvider *)provider
        decode:(const O2Float *)decode
   interpolate:(BOOL)interpolate 
renderingIntent:(O2ColorRenderingIntent)renderingIntent {
   _width=width;
   _height=height;
   _bitsPerComponent=bitsPerComponent;
   _bitsPerPixel=bitsPerPixel;
   _bytesPerRow=bytesPerRow;
   _colorSpace=[colorSpace retain];
   _bitmapInfo=bitmapInfo;
    _decoder=[decoder retain];
   _provider=[provider retain];
  // _decode=NULL;
   _interpolate=interpolate;
   _isMask=NO;
   _renderingIntent=renderingIntent;
   _mask=nil;
   
    _directBytes=NULL;
    _directLength=0;

   _clampExternalPixels=NO; // only do this if premultiplied format
   if(!initFunctionsForParameters(self,bitsPerComponent,bitsPerPixel,colorSpace,bitmapInfo)){
    NSLog(@"O2Image failed to init with bpc=%zu, bpp=%zu,colorSpace=%@,bitmapInfo=0x%0X",bitsPerComponent,bitsPerPixel,colorSpace,bitmapInfo);
    [self dealloc];
    return nil;
   }
       
   return self;
}

-initWithJPEGDataProvider:(O2DataProvider *)jpegProvider decode:(const O2Float *)decode interpolate:(BOOL)interpolate renderingIntent:(O2ColorRenderingIntent)renderingIntent {
   O2UnimplementedMethod();
   return nil;
}

-initWithPNGDataProvider:(O2DataProvider *)jpegProvider decode:(const O2Float *)decode interpolate:(BOOL)interpolate renderingIntent:(O2ColorRenderingIntent)renderingIntent {
   O2UnimplementedMethod();
   return nil;
}

-initMaskWithWidth:(size_t)width height:(size_t)height bitsPerComponent:(size_t)bitsPerComponent bitsPerPixel:(size_t)bitsPerPixel bytesPerRow:(size_t)bytesPerRow provider:(O2DataProvider *)provider decode:(const float *)decode interpolate:(BOOL)interpolate {
   O2ColorSpaceRef gray=O2ColorSpaceCreateDeviceGray();
   
    if((self=[self initWithWidth:width height:height bitsPerComponent:bitsPerComponent bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow colorSpace:gray bitmapInfo:kO2ImageAlphaNone decoder:NULL provider:provider decode:decode interpolate:interpolate renderingIntent:kO2RenderingIntentDefault])==nil){
    O2ColorSpaceRelease(gray);
    return nil;
   }
   
   O2ColorSpaceRelease(gray);
   _isMask=YES;
   return self;
}

-(void)dealloc {
   [_colorSpace release];
    [_decoder release];
   [_provider release];
   if(_decode!=NULL)
    NSZoneFree(NULL,_decode);
   [_mask release];
   [_directData release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-(O2Image *)copyWithColorSpace:(O2ColorSpaceRef)colorSpace {
   O2UnimplementedMethod();
   return nil;
}

-(void)setMask:(O2Image *)image {
   [image retain];
   [_mask release];
   _mask=image;
}

-copyWithMask:(O2Image *)image {
   O2UnimplementedMethod();
   return nil;
}

-copyWithMaskingColors:(const O2Float *)components {
   O2UnimplementedMethod();
   return nil;
}

ONYX2D_STATIC_INLINE const void *directBytes(O2Image *self){
   if(self->_directBytes==NULL){
    if([self->_provider isDirectAccess]){
     self->_directData=[[self->_provider data] retain];
     self->_directBytes=[self->_provider bytes];
     self->_directLength=[self->_provider length];
   }
    else {
     self->_directData=(NSData *)O2DataProviderCopyData(self->_provider);
     self->_directBytes=[self->_directData bytes];
     self->_directLength=[self->_directData length];
    }
   }
   return self->_directBytes;
}

ONYX2D_STATIC_INLINE const void *scanlineAtY(O2Image *self,int y){
   const void *bytes=directBytes(self);
   int         offset=self->_bytesPerRow*y;
   int         max=offset+self->_bytesPerRow;
   
   if(max<=self->_directLength)
    return bytes+offset;
    
   return NULL;
}

-(NSData *)directData {
   directBytes(self);
   return _directData;
}

-(const void *)directBytes {
   return directBytes(self);
}

-(void)releaseDirectDataIfPossible {
   if(![_provider isDirectAccess]){
    [_directData release];
    _directData=nil;
    _directBytes=NULL;
    _directLength=0;
   }
}

O2ImageRef O2ImageCreate(size_t width,size_t height,size_t bitsPerComponent,size_t bitsPerPixel,size_t bytesPerRow,O2ColorSpaceRef colorSpace,O2BitmapInfo bitmapInfo,O2DataProviderRef dataProvider,const O2Float *decode,BOOL shouldInterpolate,O2ColorRenderingIntent renderingIntent) {
    return [[O2Image alloc] initWithWidth:width height:height bitsPerComponent:bitsPerComponent bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow colorSpace:colorSpace bitmapInfo:bitmapInfo decoder:NULL provider:dataProvider decode:decode interpolate:shouldInterpolate renderingIntent:renderingIntent];
}

O2ImageRef O2ImageMaskCreate(size_t width,size_t height,size_t bitsPerComponent,size_t bitsPerPixel,size_t bytesPerRow,O2DataProviderRef dataProvider,const O2Float *decode,BOOL shouldInterpolate) {
   return [[O2Image alloc] initMaskWithWidth:width height:height bitsPerComponent:bitsPerComponent bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow provider:dataProvider decode:decode interpolate:shouldInterpolate];
}

O2ImageRef O2ImageCreateCopy(O2ImageRef self) {
   return [self copy];
}

O2ImageRef O2ImageCreateCopyWithColorSpace(O2ImageRef self,O2ColorSpaceRef colorSpace) {
   return [self copyWithColorSpace:colorSpace];
}

O2ImageRef O2ImageCreateWithJPEGDataProvider(O2DataProviderRef jpegProvider,const O2Float *decode,BOOL interpolate,O2ColorRenderingIntent renderingIntent) {
   return [[O2Image alloc] initWithJPEGDataProvider:jpegProvider decode:decode interpolate:interpolate renderingIntent:renderingIntent];
}

O2ImageRef O2ImageCreateWithPNGDataProvider(O2DataProviderRef pngProvider,const O2Float *decode,BOOL interpolate,O2ColorRenderingIntent renderingIntent) {
   return [[O2Image alloc] initWithPNGDataProvider:pngProvider decode:decode interpolate:interpolate renderingIntent:renderingIntent];
}

O2ImageRef O2ImageCreateWithImageInRect(O2ImageRef self,O2Rect rect) {
   rect=O2RectIntegral(rect);
   size_t         x=MAX(0,rect.origin.x);
   size_t         y=MAX(0,rect.origin.y);
   size_t         col,width=rect.size.width;
   size_t         row,height=rect.size.height;
   size_t         childBytesPerRow=(width*self->_bitsPerPixel+7)/8;
   uint8_t       *childPixelBytes=malloc(height*childBytesPerRow);
   size_t         childIndex=0;
   const uint8_t *pixelBytes=directBytes(self);
   
   pixelBytes+=self->_bytesPerRow*y;
   
   for(row=0;row<height;row++){
    const uint8_t *rowBytes=pixelBytes+(x*self->_bitsPerPixel)/8;
    
    for(col=0;col<childBytesPerRow;col++){
		// Copy all of the needed bytes for the row
     childPixelBytes[childIndex++]=rowBytes[col];
	}

    pixelBytes+=self->_bytesPerRow;
   }
   
   NSData *data=[NSData dataWithBytesNoCopy:childPixelBytes length:childIndex];
   O2DataProviderRef provider=O2DataProviderCreateWithCFData((CFDataRef)data);

   O2ImageRef result=O2ImageCreate(width,height,self->_bitsPerComponent,self->_bitsPerPixel,childBytesPerRow,self->_colorSpace,self->_bitmapInfo,provider,self->_decode,self->_interpolate,self->_renderingIntent);
   
   O2DataProviderRelease(provider);
   
   return result;
}

O2ImageRef O2ImageCreateWithMask(O2ImageRef self,O2ImageRef mask) {
   return [self copyWithMask:mask];
}

O2ImageRef O2ImageCreateWithMaskingColors(O2ImageRef self,const O2Float *components) {
   return [self copyWithMaskingColors:components];
}

O2ImageRef O2ImageRetain(O2ImageRef self) {
   return (self!=NULL)?(O2ImageRef)CFRetain(self):NULL;
}

void O2ImageRelease(O2ImageRef self) {
   if(self!=NULL)
    CFRelease(self);
}

BOOL O2ImageIsMask(O2ImageRef self) {
   if(self==NULL)
    return NO;
    
   return self->_isMask;
}

size_t O2ImageGetWidth(O2Image *self) {
   if(self==NULL)
    return 0;

   return self->_width;
}

size_t O2ImageGetHeight(O2Image *self) {
   if(self==NULL)
    return 0;

   return self->_height;
}

size_t O2ImageGetBitsPerComponent(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_bitsPerComponent;
}

size_t O2ImageGetBitsPerPixel(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_bitsPerPixel;
}

size_t O2ImageGetBytesPerRow(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_bytesPerRow;
}

O2ColorSpaceRef O2ImageGetColorSpace(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_colorSpace;
}

O2ImageAlphaInfo O2ImageGetAlphaInfo(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_bitmapInfo&kO2BitmapAlphaInfoMask;
}

O2DataProviderRef O2ImageGetDataProvider(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_provider;
}

const O2Float *O2ImageGetDecode(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_decode;
}

BOOL O2ImageGetShouldInterpolate(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_interpolate;
}

O2ColorRenderingIntent O2ImageGetRenderingIntent(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_renderingIntent;
}

O2BitmapInfo O2ImageGetBitmapInfo(O2ImageRef self) {
   if(self==NULL)
    return 0;

   return self->_bitmapInfo;
}

O2ImageRef O2ImageGetMask(O2ImageRef self) {
   if(self==NULL)
    return 0;
   
   return self->_mask;
}

O2ImageDecoderRef O2ImageGetImageDecoder(O2ImageRef self) {
    if(self==NULL)
        return NULL;
    
    return self->_decoder;
}

O2argb32f *O2ImageRead_ANY_to_argb8u_to_argb32f(O2Image *self,int x,int y,O2argb32f *span,int length){
   O2argb8u *span8888=__builtin_alloca(length*sizeof(O2argb8u));
   O2argb8u *direct=self->_read_argb8u(self,x,y,span8888,length);
   
   if(direct!=NULL)
    span8888=direct;
    
   int i;
   for(i=0;i<length;i++){
    O2argb32f  result;
    
    result.r = O2Float32FromByte(span8888[i].r);
    result.g = O2Float32FromByte(span8888[i].g);
    result.b = O2Float32FromByte(span8888[i].b);
	result.a = O2Float32FromByte(span8888[i].a);
    *span++=result;
   }
   return NULL;
}

float bytesLittleToFloat(const unsigned char *scanline){
   union {
    unsigned char bytes[4];
    float         f;
   } u;

#ifdef __LITTLE_ENDIAN__   
   u.bytes[0]=scanline[0];
   u.bytes[1]=scanline[1];
   u.bytes[2]=scanline[2];
   u.bytes[3]=scanline[3];
#else
   u.bytes[0]=scanline[3];
   u.bytes[1]=scanline[2];
   u.bytes[2]=scanline[1];
   u.bytes[3]=scanline[0];
#endif

   return u.f;
}

O2argb32f *O2ImageRead_argb32fLittle_to_argb32f(O2Image *self,int x,int y,O2argb32f *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;
    
   scanline+=x*16;
   for(i=0;i<length;i++){
    O2argb32f result;
    
    result.r=bytesLittleToFloat(scanline);
    scanline+=4;
    result.g=bytesLittleToFloat(scanline);
    scanline+=4;
    result.b=bytesLittleToFloat(scanline);
    scanline+=4;
    result.a=bytesLittleToFloat(scanline);
    scanline+=4;
        
    *span++=result;
   }
   return NULL;
}

float bytesBigToFloat(const unsigned char *scanline){
   union {
    unsigned char bytes[4];
    float         f;
   } u;

#ifdef __BIG_ENDIAN__   
   u.bytes[0]=scanline[0];
   u.bytes[1]=scanline[1];
   u.bytes[2]=scanline[2];
   u.bytes[3]=scanline[3];
#else
   u.bytes[0]=scanline[3];
   u.bytes[1]=scanline[2];
   u.bytes[2]=scanline[1];
   u.bytes[3]=scanline[0];
#endif

   return u.f;
}

O2argb32f *O2ImageRead_argb32fBig_to_argb32f(O2Image *self,int x,int y,O2argb32f *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;
    
   scanline+=x*16;
   for(i=0;i<length;i++){
    O2argb32f result;
    
    result.r=bytesBigToFloat(scanline);
    scanline+=4;
    result.g=bytesBigToFloat(scanline);
    scanline+=4;
    result.b=bytesBigToFloat(scanline);
    scanline+=4;
    result.a=bytesBigToFloat(scanline);
    scanline+=4;
        
    *span++=result;
   }
   return NULL;
}

uint8_t *O2ImageRead_G8_to_A8(O2Image *self,int x,int y,uint8_t *alpha,int length) {
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;
    
   scanline+=x;
   for(i=0;i<length;i++){
    *alpha++=*scanline++;
   }
   return NULL;
}

uint8_t *O2Image_read_a8u_src_argb8u(O2Image *self,int x,int y,uint8_t *alpha,int length) {
   O2argb8u *span=__builtin_alloca(length*sizeof(O2argb8u));
   int i;
   
   O2argb8u *direct=self->_read_argb8u(self,x,y,span,length);
   if(direct!=NULL)
    span=direct;
    
   for(i=0;i<length;i++){
    *alpha++=span[i].a;
   }
   return NULL;
}

O2Float *O2ImageRead_ANY_to_A8_to_Af(O2Image *self,int x,int y,O2Float *alpha,int length) {
   uint8_t span[length];
   int     i;
   
   self->_read_a8u(self,x,y,span,length);
   for(i=0;i<length;i++)
    alpha[i]=O2Float32FromByte(span[i]);
    
   return NULL;
}

O2argb8u *O2ImageRead_G8_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;

   if(scanline==NULL)
    return NULL;
    
   scanline+=x;
   for(i=0;i<length;i++){
    O2argb8u result;
    
    result.r=*scanline++;
    result.g=result.r;
    result.b=result.r;
    result.a=0xFF;
    
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_GA88_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;

   if(scanline==NULL)
    return NULL;
    
   scanline+=x*2;
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.r = *scanline++;
    result.g=result.r;
    result.b=result.r;
	result.a = *scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_RGBA8888_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.r = scanline[0];
    result.g = scanline[1];
    result.b = scanline[2];
	result.a = scanline[3];
    *span++=result;
    scanline+=4;
   }
   return NULL;
}

O2argb8u *O2ImageRead_ABGR8888_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.a = *scanline++;
    result.b = *scanline++;
    result.g = *scanline++;
	result.r = *scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_BGRA8888_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length) {
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*4;
#ifdef __LITTLE_ENDIAN__
   return (O2argb8u *)scanline;
#endif
   
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.b = *scanline++;
    result.g = *scanline++;
	result.r = *scanline++;
    result.a = *scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_RGB888_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length) {
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*3;
   
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.r = *scanline++;
    result.g = *scanline++;
	result.b = *scanline++;
    result.a = 255;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_BGRX8888_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length) {
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*4;

   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.b = *scanline++;
    result.g = *scanline++;
	result.r = *scanline++;
    result.a = 255; scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_XRGB8888_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length) {
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*4;
   
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.a = *scanline++;
    result.r = *scanline++;
	result.g = *scanline++;
    result.b = *scanline++;
    *span++=result;
   }
   return NULL;
}

// kO2BitmapByteOrder16Little|kO2ImageAlphaNoneSkipFirst
O2argb8u *O2ImageRead_G3B5X1R5G2_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*2;
   for(i=0;i<length;i++){
    unsigned short low=*scanline++;
    unsigned short high=*scanline;
    unsigned short value=low|(high<<8);
    O2argb8u  result;
    
    result.r = ((value>>10)&0x1F)<<3;
    result.g = ((value>>5)&0x1F)<<3;
    result.b = ((value&0x1F)<<3);
    result.a = 255;
    scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_RGBA4444_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*2;
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.r = *scanline&0xF0;
    result.g = (*scanline&0x0F)<<4;
    scanline++;
    result.b = *scanline&0xF0;
    result.a = (*scanline&0x0F)<<4;
    scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_BARG4444_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*2;
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.b = *scanline&0xF0;
    result.a = (*scanline&0x0F)<<4;
    scanline++;
    result.r = *scanline&0xF0;
    result.g = (*scanline&0x0F)<<4;
    scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_RGBA2222_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x;
   for(i=0;i<length;i++){
    O2argb8u  result;
    
    result.r = *scanline&0xC0;
    result.g = (*scanline&0x03)<<2;
    result.b = (*scanline&0x0C)<<4;
	result.a = (*scanline&0x03)<<6;
    scanline++;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_CMYK8888_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
// poor results
   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;

   scanline+=x*4;
   for(i=0;i<length;i++){
    O2argb8u  result;
    unsigned char c=*scanline++;
    unsigned char y=*scanline++;
    unsigned char m=*scanline++;
    unsigned char k=*scanline++;
    unsigned char w=0xff-k;
    
    result.r = c>w?0:w-c;
    result.g = m>w?0:w-m;
    result.b = y>w?0:w-y;
	result.a = 1;
    *span++=result;
   }
   return NULL;
}

O2argb8u *O2ImageRead_I8_to_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length) {
   O2ColorSpace_indexed *indexed=(O2ColorSpace_indexed *)self->_colorSpace;
   unsigned hival=[indexed hival];
   const unsigned char *palette=[indexed paletteBytes];

   const uint8_t *scanline = scanlineAtY(self,y);
   int i;
   
   if(scanline==NULL)
    return NULL;
   
   scanline+=x;

   for(i=0;i<length;i++){
    unsigned index=*scanline++;
    O2argb8u argb;

    RI_INT_CLAMP(index,0,hival); // it is external data after all
    
    argb.r=palette[index*3+0];
    argb.g=palette[index*3+1];
    argb.b=palette[index*3+2];
    argb.a=255;
    *span++=argb;
   }
   
   return NULL;
}

/*-------------------------------------------------------------------*//*!
* \brief	Reads a texel (u,v) at the given mipmap level. Tiling modes and
*			color space conversion are applied. Outputs color in premultiplied
*			format.
* \param	
* \return	
* \note		
*//*-------------------------------------------------------------------*/

/* O2ImageReadTileSpanExtendEdge__ is used by the image resampling functions to read
   translated spans. When a coordinate is outside the image it uses the edge
   value. This works better than say, zero, with averaging algorithms (bilinear,bicubic, etc)
   as you get good values at the edges.
   
   Ideally the averaging algorithms would only use the available pixels on the edges */
   
ONYX2D_STATIC_INLINE void O2ImageReadTileSpanExtendEdge_largb8u_PRE(O2Image *self,int u, int v, O2argb8u *span,int length){
   int i;
   O2argb8u *direct;
   v = RI_INT_CLAMP(v,0,self->_height-1);
      
   for(i=0;u<0 && i<length;u++,i++){
    direct=self->_read_argb8u(self,0,v,span+i,1);

    if(direct!=NULL)
     span[i]=direct[0];
   }   

   int chunk=RI_MIN(length-i,self->_width-u);
   direct=self->_read_argb8u(self,u,v,span+i,chunk);
   if(direct!=NULL) {
    int k;
    
    for(k=0;k<chunk;k++)
     span[i+k]=direct[k];
   }
   
   i+=chunk;
   u+=chunk;

   for(;i<length;i++){
    direct=self->_read_argb8u(self,self->_width-1,v,span+i,1);
    
    if(direct!=NULL)
     span[i]=direct[0];
   }
}

void O2ImageReadTileSpanExtendEdge__largb32f_PRE(O2Image *self,int u, int v, O2argb32f *span,int length){
   int i;
   O2argb32f *direct;
   
   v = RI_INT_CLAMP(v,0,self->_height-1);
   
   for(i=0;i<length && u<0;u++,i++){
    direct=O2Image_read_argb32f(self,0,v,span+i,1);
    if(direct!=NULL)
     span[i]=direct[0];
   }
   
   int chunk=RI_MIN(length-i,self->_width-u);
   direct=O2Image_read_argb32f(self,u,v,span+i,chunk);
   if(direct!=NULL) {
    int k;
    
    for(k=0;k<chunk;k++)
     span[i+k]=direct[k];
   }
   i+=chunk;
   u+=chunk;

   for(;i<length;i++){
    direct=O2Image_read_argb32f(self,self->_width-1,v,span+i,1);
    if(direct!=NULL)
     span[i]=direct[0];
   }
}

ONYX2D_STATIC_INLINE int cubic_8(int v0,int v1,int v2,int v3,int fraction){
  int p = (v3 - v2) - (v0 - v1);
  int q = (v0 - v1) - p;

  return RI_INT_CLAMP((p * (fraction*fraction*fraction))/(256*256*256) + (q * fraction*fraction)/(256*256) + ((v2 - v0) * fraction)/256 + v1,0,255);
}

ONYX2D_STATIC_INLINE O2argb8u bicubic_largb8u_PRE(O2argb8u a,O2argb8u b,O2argb8u c,O2argb8u d,int fraction) {
  O2argb8u result;
  
  result.r=cubic_8(a.r, b.r, c.r, d.r, fraction);
  result.b=cubic_8(a.g, b.g, c.g, d.g, fraction);
  result.g=cubic_8(a.b, b.b, c.b, d.b, fraction);
  result.a=cubic_8(a.a, b.a, c.a, d.a, fraction);
  
  return result;
}

void O2ImageBicubic_largb8u_PRE(O2Image *self,int x, int y,O2argb8u *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    uv.x -= 0.5f;
    uv.y -= 0.5f;
    int u = RI_FLOOR_TO_INT(uv.x);
    int ufrac=coverageFromZeroToOne(uv.x-u);
        
    int v = RI_FLOOR_TO_INT(uv.y);
    int vfrac=coverageFromZeroToOne(uv.y-v);
        
    O2argb8u t0,t1,t2,t3;
    O2argb8u cspan[4];
     
    O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u - 1,v - 1,cspan,4);
    t0 = bicubic_largb8u_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);
      
    O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u - 1,v,cspan,4);
    t1 = bicubic_largb8u_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);
     
    O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u - 1,v+1,cspan,4);     
    t2 = bicubic_largb8u_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);
     
    O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u - 1,v+2,cspan,4);     
    t3 = bicubic_largb8u_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);

    span[i]=bicubic_largb8u_PRE(t0,t1,t2,t3, vfrac);

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

ONYX2D_STATIC_INLINE float cubic_f(float v0,float v1,float v2,float v3,float fraction){
  float p = (v3 - v2) - (v0 - v1);
  float q = (v0 - v1) - p;

  return RI_CLAMP((p * (fraction*fraction*fraction)) + (q * fraction*fraction) + ((v2 - v0) * fraction) + v1,0,1);
}

O2argb32f bicubic_largb32f_PRE(O2argb32f a,O2argb32f b,O2argb32f c,O2argb32f d,float fraction) {
  return O2argb32fInit(
   cubic_f(a.r, b.r, c.r, d.r, fraction),
   cubic_f(a.g, b.g, c.g, d.g, fraction),
   cubic_f(a.b, b.b, c.b, d.b, fraction),
   cubic_f(a.a, b.a, c.a, d.a, fraction));
}

void O2ImageBicubic_largb32f_PRE(O2Image *self,int x, int y,O2argb32f *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    uv.x -= 0.5f;
    uv.y -= 0.5f;
    int u = RI_FLOOR_TO_INT(uv.x);
    float ufrac=uv.x-u;
        
    int v = RI_FLOOR_TO_INT(uv.y);
    float vfrac=uv.y-v;
        
    O2argb32f t0,t1,t2,t3;
    O2argb32f cspan[4];
     
    O2ImageReadTileSpanExtendEdge__largb32f_PRE(self,u - 1,v - 1,cspan,4);
    t0 = bicubic_largb32f_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);
      
    O2ImageReadTileSpanExtendEdge__largb32f_PRE(self,u - 1,v,cspan,4);
    t1 = bicubic_largb32f_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);
     
    O2ImageReadTileSpanExtendEdge__largb32f_PRE(self,u - 1,v+1,cspan,4);     
    t2 = bicubic_largb32f_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);
     
    O2ImageReadTileSpanExtendEdge__largb32f_PRE(self,u - 1,v+2,cspan,4);     
    t3 = bicubic_largb32f_PRE(cspan[0],cspan[1],cspan[2],cspan[3],ufrac);

    span[i]=bicubic_largb32f_PRE(t0,t1,t2,t3, vfrac);

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

void O2ImageBilinear_largb8u_PRE(O2Image *self,int x, int y,O2argb8u *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5)*surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5)*surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   double a=surfaceToImage.a*COVERAGE_MULTIPLIER_FLOAT;
   double b=surfaceToImage.b*COVERAGE_MULTIPLIER_FLOAT;
   int i;
   
   du-=0.5;
   dv-=0.5;

// Coordinates are pre-scaled by 256 to avoid float multiplication and
// subtraction inside the loop for generating the coverage amount.

   du*=COVERAGE_MULTIPLIER_FLOAT;
   dv*=COVERAGE_MULTIPLIER_FLOAT;
   
   for(i=0;i<length;i++,x++){
    int uvx=du;
    int uvy=dv;
    
    int u=uvx>>8;
    int v=uvy>>8;
    
    uint32_t fu = uvx&0xFF;
    uint32_t oneMinusFu=inverseCoverage(fu);
    
	O2argb8u line0[2];
	O2argb8u line1[2];
    O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u,v,line0,2);
    O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u,v+1,line1,2);

    O2argb8u c0 = O2argb8uMultiplyByCoverageAdd(line0[0],oneMinusFu,line0[1],fu);
    O2argb8u c1 = O2argb8uMultiplyByCoverageAdd(line1[0],oneMinusFu,line1[1],fu);
    
    uint32_t fv = uvy&0xFF;
    uint32_t oneMinusFv=inverseCoverage(fv);

    span[i]=O2argb8uMultiplyByCoverageAdd(c0,oneMinusFv,c1,fv);
        
    du+=a;
    dv+=b;
   }
}


void O2ImageBilinear_largb32f_PRE(O2Image *self,int x, int y,O2argb32f *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;

   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    uv.x -= 0.5f;
	uv.y -= 0.5f;
	int u = RI_FLOOR_TO_INT(uv.x);
	int v = RI_FLOOR_TO_INT(uv.y);
	O2argb32f c00c01[2];
    O2ImageReadTileSpanExtendEdge__largb32f_PRE(self,u,v,c00c01,2);

    O2argb32f c01c11[2];
    O2ImageReadTileSpanExtendEdge__largb32f_PRE(self,u,v+1,c01c11,2);

    O2Float fu = uv.x - (O2Float)u;
    O2Float fv = uv.y - (O2Float)v;
    O2argb32f c0 = O2argb32fAdd(O2argb32fMultiplyByFloat(c00c01[0],(1.0f - fu)),O2argb32fMultiplyByFloat(c00c01[1],fu));
    O2argb32f c1 = O2argb32fAdd(O2argb32fMultiplyByFloat(c01c11[0],(1.0f - fu)),O2argb32fMultiplyByFloat(c01c11[1],fu));
    span[i]=O2argb32fAdd(O2argb32fMultiplyByFloat(c0,(1.0f - fv)),O2argb32fMultiplyByFloat(c1, fv));

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

void O2ImagePointSampling_largb8u_PRE(O2Image *self,int x, int y,O2argb8u *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,RI_FLOOR_TO_INT(uv.x), RI_FLOOR_TO_INT(uv.y),span+i,1);

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

void O2ImagePointSampling_largb32f_PRE(O2Image *self,int x, int y,O2argb32f *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    O2ImageReadTileSpanExtendEdge__largb32f_PRE(self,RI_FLOOR_TO_INT(uv.x), RI_FLOOR_TO_INT(uv.y),span+i,1);

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

// Float translate, or float translate with flip
void O2ImageBilinearFloatTranslate_largb8u_PRE(O2Image *self,int x, int y,O2argb8u *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   du-=0.5;
   dv-=0.5;
   
   O2Float uvx=du;
   O2Float uvy=dv;

   int u = RI_FLOOR_TO_INT(uvx);
   int v = RI_FLOOR_TO_INT(uvy);
   unsigned fu = coverageFromZeroToOne(uvx - (O2Float)u);
   unsigned oneMinusFu=inverseCoverage(fu);
   unsigned fv = coverageFromZeroToOne(uvy - (O2Float)v);
   unsigned oneMinusFv=inverseCoverage(fv);

   O2argb8u line0[length+1];
   O2argb8u line1[length+1];
   
   O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u,v+0,line0,length+1);
   O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,u,v+1,line1,length+1);

   for(i=0;i<length;i++){
    O2argb8u c0 = O2argb8uMultiplyByCoverageAdd(line0[i],oneMinusFu,line0[i+1],fu);
    O2argb8u c1 = O2argb8uMultiplyByCoverageAdd(line1[i],oneMinusFu,line1[i+1],fu);
    
    span[i]=O2argb8uMultiplyByCoverageAdd(c0,oneMinusFv,c1,fv);
   }
}

// Translate or translate with -1 flip
void O2ImageIntegerTranslate_largb8u_PRE(O2Image *self,int x, int y,O2argb8u *span,int length, O2AffineTransform surfaceToImage){
   int du=x+surfaceToImage.tx;
   int dv=((y+0.5)*surfaceToImage.d+surfaceToImage.ty)-0.5;
   
   O2ImageReadTileSpanExtendEdge_largb8u_PRE(self,du,dv,span,length);
}

//clamp premultiplied color to alpha to enforce consistency
ONYX2D_STATIC void clampSpan_largb32f_PRE(O2argb32f *span,int length){
   int i;
   
   for(i=0;i<length;i++){
    span[i].r = RI_MIN(span[i].r, span[i].a);
    span[i].g = RI_MIN(span[i].g, span[i].a);
    span[i].b = RI_MIN(span[i].b, span[i].a);
   }
}

ONYX2D_STATIC_INLINE void O2RGBPremultiplySpan(O2argb32f *span,int length){
   int i;
      
   for(i=0;i<length;i++){
    span[i].r*=span[i].a;
    span[i].g*=span[i].a;
    span[i].b*=span[i].a; 
   }
}

O2argb8u *O2Image_read_argb8u(O2Image *self,int x,int y,O2argb8u *span,int length){
   return self->_read_argb8u(self,x,y,span,length);
}
   
O2argb32f *O2Image_read_argb32f(O2Image *self,int x,int y,O2argb32f *span,int length) {   
   O2argb32f *direct=self->_read_argb32f(self,x,y,span,length);
   
   if(direct!=NULL)
    span=direct;
    
   if(self->_clampExternalPixels)
    clampSpan_largb32f_PRE(span,length); // We don't need to do this for internally generated images (context)

   return NULL;
}

uint8_t *O2Image_read_a8u(O2Image *self,int x,int y,uint8_t *coverage,int length){
   return self->_read_a8u(self,x,y,coverage,length);
}

O2Float *O2ImageReadSpan_Af_MASK(O2Image *self,int x,int y,O2Float *coverage,int length) {
   return self->_read_a32f(self,x,y,coverage,length);
}


void O2ImageReadTexelTileRepeat_largb8u_PRE(O2Image *self,int u,int v,O2argb8u *span,int length){
   int i;

   v = RI_INT_MOD(v, self->_height);
   
   for(i=0;i<length;i++,u++){
    u = RI_INT_MOD(u,self->_width);

    O2argb8u *direct=O2Image_read_argb8u(self,u,v,span+i,1);
    
    if(direct!=NULL)
     span[i]=direct[0];
   }
}

void O2ImageReadTexelTileRepeat_largb32f_PRE(O2Image *self,int u, int v, O2argb32f *span,int length){
   int i;

   v = RI_INT_MOD(v, self->_height);
   
   for(i=0;i<length;i++,u++){
    u = RI_INT_MOD(u, self->_width);

    O2argb32f *direct=O2Image_read_argb32f(self,u,v,span+i,1);
    if(direct!=NULL)
     span[i]=direct[0];
   }
}

void O2ImagePattern_Bilinear(O2Image *self,O2Float x, O2Float y,O2argb32f *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    uv.x -= 0.5f;
	uv.y -= 0.5f;
	int u = RI_FLOOR_TO_INT(uv.x);
	int v = RI_FLOOR_TO_INT(uv.y);
	O2argb32f c00c01[2];
    O2ImageReadTexelTileRepeat_largb32f_PRE(self,u,v,c00c01,2);

    O2argb32f c01c11[2];
    O2ImageReadTexelTileRepeat_largb32f_PRE(self,u,v+1,c01c11,2);

    O2Float fu = uv.x - (O2Float)u;
    O2Float fv = uv.y - (O2Float)v;
    O2argb32f c0 = O2argb32fAdd(O2argb32fMultiplyByFloat(c00c01[0],(1.0f - fu)),O2argb32fMultiplyByFloat(c00c01[1],fu));
    O2argb32f c1 = O2argb32fAdd(O2argb32fMultiplyByFloat(c01c11[0],(1.0f - fu)),O2argb32fMultiplyByFloat(c01c11[1],fu));
    span[i]=O2argb32fAdd(O2argb32fMultiplyByFloat(c0,(1.0f - fv)),O2argb32fMultiplyByFloat(c1, fv));

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

void O2ImagePattern_PointSampling_largb8u_PRE(O2Image *self,O2Float x,O2Float y,O2argb8u *span,int length,O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    O2ImageReadTexelTileRepeat_largb8u_PRE(self,RI_FLOOR_TO_INT(uv.x), RI_FLOOR_TO_INT(uv.y),span+i,1);

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

void O2ImagePattern_PointSampling_largb32f_PRE(O2Image *self,O2Float x, O2Float y,O2argb32f *span,int length, O2AffineTransform surfaceToImage){
   double du=(x+0.5) * surfaceToImage.a+(y+0.5)* surfaceToImage.c+surfaceToImage.tx;
   double dv=(x+0.5) * surfaceToImage.b+(y+0.5)* surfaceToImage.d+surfaceToImage.ty;
   int i;
   
   for(i=0;i<length;i++,x++){
    O2Point uv=O2PointMake(du,dv);

    O2ImageReadTexelTileRepeat_largb32f_PRE(self,RI_FLOOR_TO_INT(uv.x), RI_FLOOR_TO_INT(uv.y),span+i,1);

    du+=surfaceToImage.a;
    dv+=surfaceToImage.b;
   }
}

void O2ImageReadPatternSpan_largb8u_PRE(O2Image *self,O2Float x, O2Float y, O2argb8u *span,int length, O2AffineTransform surfaceToImage, O2PatternTiling distortion) {
    
   switch(distortion){
    case kO2PatternTilingNoDistortion:
    case kO2PatternTilingConstantSpacingMinimalDistortion:
    case kO2PatternTilingConstantSpacing:
      O2ImagePattern_PointSampling_largb8u_PRE(self,x,y,span,length,surfaceToImage);
      break;
   }
}

void O2ImageReadPatternSpan_largb32f_PRE(O2Image *self,O2Float x, O2Float y, O2argb32f *span,int length, O2AffineTransform surfaceToImage, O2PatternTiling distortion)	{
    
   switch(distortion){
    case kO2PatternTilingNoDistortion:
      O2ImagePattern_PointSampling_largb32f_PRE(self,x,y,span,length,surfaceToImage);
      break;

    case kO2PatternTilingConstantSpacingMinimalDistortion:
    case kO2PatternTilingConstantSpacing:
     default:
      O2ImagePattern_Bilinear(self,x,y,span,length,surfaceToImage);
      break;
   }
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@:%p> width=%d,height=%d,bpc=%d,bpp=%d,bpr=%d,bminfo=%x data length=%d",isa,self,_width,_height,_bitsPerComponent,_bitsPerPixel,_bytesPerRow,_bitmapInfo,[_provider length]];
}

@end
