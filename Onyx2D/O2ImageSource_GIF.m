/* Copyright (c) 2009 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2ImageSource_GIF.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Image.h>

@implementation O2ImageSource_GIF

+(BOOL)isPresentInDataProvider:(O2DataProvider *)provider {
   enum { signatureLength=4 };
   unsigned char signature[signatureLength] = { 'G','I','F','8' };
   unsigned char check[signatureLength];
   NSInteger     i,size=[provider getBytes:check range:NSMakeRange(0,signatureLength)];
   
   if(size!=signatureLength)
    return NO;
    
   for(i=0;i<signatureLength;i++)
    if(signature[i]!=check[i])
     return NO;
     
   return YES;
}

-initWithDataProvider:(O2DataProvider *)provider options:(NSDictionary *)options {
   [super initWithDataProvider:provider options:options];

   NSInputStream *stream=[_provider inputStream];
   if((_gif=DGifOpen(stream))==NULL){
    [self dealloc];
    return nil;
   }
   
   DGifSlurp(_gif);
   return self;
}

-(void)dealloc {
   if(_gif!=NULL)
    DGifCloseFile(_gif);

   [super dealloc];
}

- (CFStringRef)type
{
    return (CFStringRef)@"com.compuserve.gif";
}

-(unsigned)count {
   return _gif->ImageCount;
}

-(CFDictionaryRef)copyPropertiesAtIndex:(unsigned)index options:(CFDictionaryRef)options {
   return nil;
}

-(O2Image *)createImageAtIndex:(unsigned)index options:(NSDictionary *)options {

   if(index>=_gif->ImageCount)
    return nil;
    
   SavedImage     *gifImage=_gif->SavedImages+index;
   ColorMapObject *colorMap=_gif->SColorMap;
   if(colorMap==NULL)
    colorMap=gifImage->ImageDesc.ColorMap;
   int             bgColorIndex=(_gif->SColorMap==NULL)?-1:_gif->SBackGroundColor;
   int             colorCount=colorMap->ColorCount;
   GifColorType   *colorLUT=colorMap->Colors;
   unsigned char  *gifRaster=gifImage->RasterBits;
   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
   size_t         width=gifImage->ImageDesc.Width;
   size_t         height=gifImage->ImageDesc.Height;
   size_t         bytesPerRow=width*sizeof(O2rgba8u_BE);
   size_t         bitsPerComponent=8;
   size_t         bitsPerPixel=32;
   O2rgba8u_BE    *pixels=NSZoneMalloc(NULL,bytesPerRow*height);
   O2rgba8u_BE    *scanline=pixels;
   BOOL           interlace=gifImage->ImageDesc.Interlace;
   int            interlacePass=1;
   int            interlaceDelta=8;
   NSData        *bitmap=[[NSData alloc] initWithBytesNoCopy:pixels length:bytesPerRow*height];
  
   size_t r,c;
   
   for(r=0;r<height;r++){

    for(c=0;c<width;c++){
     unsigned char colorIndex=*gifRaster;
     
     if(colorIndex==bgColorIndex){
      scanline->a=0;
      scanline->r=0;
      scanline->g=0;
      scanline->b=0;
     }
     else if(colorIndex<colorCount){
      GifColorType *gifColor=colorLUT+colorIndex;

      scanline->a=255;
      scanline->r=gifColor->Red;
      scanline->g=gifColor->Green;
      scanline->b=gifColor->Blue;
     }
     else {
      scanline->a=0;
      scanline->r=0;
      scanline->g=0;
      scanline->b=0;
     }
     scanline++;
     gifRaster++;
    }
    
    if(interlace){
     scanline+=(interlaceDelta-1)*width;
     if(scanline>=pixels+width*height){
      interlacePass++;
      
      switch(interlacePass){
       case 2:
        scanline=pixels+width*4;
        interlaceDelta=8;
        break;
       case 3:
        scanline=pixels+width*2;
        interlaceDelta=4;
        break;
       case 4:
        scanline=pixels+width*1;
        interlaceDelta=2;
        break;
      }
     }
    }
   }
  
    O2DataProvider *provider=O2DataProviderCreateWithCFData((CFDataRef)bitmap);
   O2BitmapInfo    info=kO2BitmapByteOrder32Big|kO2ImageAlphaPremultipliedLast;

   O2Image        *result=[[O2Image alloc] initWithWidth:width height:height bitsPerComponent:bitsPerComponent bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow colorSpace:colorSpace bitmapInfo:info decoder:NULL provider:provider decode:NULL interpolate:NO renderingIntent:kO2RenderingIntentDefault];

   [colorSpace release];
   [provider release];
   [bitmap release];
   
   return result;
}

@end
