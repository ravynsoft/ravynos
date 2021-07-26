/* Copyright (c) 2010 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2ImageSource_ICNS.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Image.h>

typedef struct O2ICNSNode {
   struct O2ICNSNode *next;
   size_t             width;
   size_t             height;
   size_t             bitsPerPixel;
   bool               isMask;
   void              *samples;
} O2ICNSNode;

static uint8_t fourBitCLUT[16][3]={
   { 0xFF, 0xFF, 0xFF }, { 0xFC, 0xF3, 0x05 }, { 0xFF, 0x64, 0x02 }, { 0xDD, 0x08, 0x06 },
   { 0xF2, 0x08, 0x84 }, { 0x46, 0x00, 0xA5 }, { 0x00, 0x00, 0xD4 }, { 0x02, 0xAB, 0xEA },
   { 0x1F, 0xB7, 0x14 }, { 0x00, 0x64, 0x11 }, { 0x56, 0x2C, 0x05 }, { 0x90, 0x71, 0x3A },
   { 0xC0, 0xC0, 0xC0 }, { 0x80, 0x80, 0x80 }, { 0x40, 0x40, 0x40 }, { 0x00, 0x00, 0x00 },
};

static uint8_t eightBitCLUT[256][3]={
   { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xCC }, { 0xFF, 0xFF, 0x99 }, { 0xFF, 0xFF, 0x66 },
   { 0xFF, 0xFF, 0x33 }, { 0xFF, 0xFF, 0x00 }, { 0xFF, 0xCC, 0xFF }, { 0xFF, 0xCC, 0xCC },
   { 0xFF, 0xCC, 0x99 }, { 0xFF, 0xCC, 0x66 }, { 0xFF, 0xCC, 0x33 }, { 0xFF, 0xCC, 0x00 },
   { 0xFF, 0x99, 0xFF }, { 0xFF, 0x99, 0xCC }, { 0xFF, 0x99, 0x99 }, { 0xFF, 0x99, 0x66 },
   { 0xFF, 0x99, 0x33 }, { 0xFF, 0x99, 0x00 }, { 0xFF, 0x66, 0xFF }, { 0xFF, 0x66, 0xCC },
   { 0xFF, 0x66, 0x99 }, { 0xFF, 0x66, 0x66 }, { 0xFF, 0x66, 0x33 }, { 0xFF, 0x66, 0x00 },
   { 0xFF, 0x33, 0xFF }, { 0xFF, 0x33, 0xCC }, { 0xFF, 0x33, 0x99 }, { 0xFF, 0x33, 0x66 },
   { 0xFF, 0x33, 0x33 }, { 0xFF, 0x33, 0x00 }, { 0xFF, 0x00, 0xFF }, { 0xFF, 0x00, 0xCC },
   { 0xFF, 0x00, 0x99 }, { 0xFF, 0x00, 0x66 }, { 0xFF, 0x00, 0x33 }, { 0xFF, 0x00, 0x00 },
   { 0xCC, 0xFF, 0xFF }, { 0xCC, 0xFF, 0xCC }, { 0xCC, 0xFF, 0x99 }, { 0xCC, 0xFF, 0x66 },
   { 0xCC, 0xFF, 0x33 }, { 0xCC, 0xFF, 0x00 }, { 0xCC, 0xCC, 0xFF }, { 0xCC, 0xCC, 0xCC },
   { 0xCC, 0xCC, 0x99 }, { 0xCC, 0xCC, 0x66 }, { 0xCC, 0xCC, 0x33 }, { 0xCC, 0xCC, 0x00 },
   { 0xCC, 0x99, 0xFF }, { 0xCC, 0x99, 0xCC }, { 0xCC, 0x99, 0x99 }, { 0xCC, 0x99, 0x66 },
   { 0xCC, 0x99, 0x33 }, { 0xCC, 0x99, 0x00 }, { 0xCC, 0x66, 0xFF }, { 0xCC, 0x66, 0xCC },
   { 0xCC, 0x66, 0x99 }, { 0xCC, 0x66, 0x66 }, { 0xCC, 0x66, 0x33 }, { 0xCC, 0x66, 0x00 },
   { 0xCC, 0x33, 0xFF }, { 0xCC, 0x33, 0xCC }, { 0xCC, 0x33, 0x99 }, { 0xCC, 0x33, 0x66 },
   { 0xCC, 0x33, 0x33 }, { 0xCC, 0x33, 0x00 }, { 0xCC, 0x00, 0xFF }, { 0xCC, 0x00, 0xCC },
   { 0xCC, 0x00, 0x99 }, { 0xCC, 0x00, 0x66 }, { 0xCC, 0x00, 0x33 }, { 0xCC, 0x00, 0x00 },
   { 0x99, 0xFF, 0xFF }, { 0x99, 0xFF, 0xCC }, { 0x99, 0xFF, 0x99 }, { 0x99, 0xFF, 0x66 },
   { 0x99, 0xFF, 0x33 }, { 0x99, 0xFF, 0x00 }, { 0x99, 0xCC, 0xFF }, { 0x99, 0xCC, 0xCC },
   { 0x99, 0xCC, 0x99 }, { 0x99, 0xCC, 0x66 }, { 0x99, 0xCC, 0x33 }, { 0x99, 0xCC, 0x00 },
   { 0x99, 0x99, 0xFF }, { 0x99, 0x99, 0xCC }, { 0x99, 0x99, 0x99 }, { 0x99, 0x99, 0x66 },
   { 0x99, 0x99, 0x33 }, { 0x99, 0x99, 0x00 }, { 0x99, 0x66, 0xFF }, { 0x99, 0x66, 0xCC },
   { 0x99, 0x66, 0x99 }, { 0x99, 0x66, 0x66 }, { 0x99, 0x66, 0x33 }, { 0x99, 0x66, 0x00 },
   { 0x99, 0x33, 0xFF }, { 0x99, 0x33, 0xCC }, { 0x99, 0x33, 0x99 }, { 0x99, 0x33, 0x66 },
   { 0x99, 0x33, 0x33 }, { 0x99, 0x33, 0x00 }, { 0x99, 0x00, 0xFF }, { 0x99, 0x00, 0xCC },
   { 0x99, 0x00, 0x99 }, { 0x99, 0x00, 0x66 }, { 0x99, 0x00, 0x33 }, { 0x99, 0x00, 0x00 },
   { 0x66, 0xFF, 0xFF }, { 0x66, 0xFF, 0xCC }, { 0x66, 0xFF, 0x99 }, { 0x66, 0xFF, 0x66 },
   { 0x66, 0xFF, 0x33 }, { 0x66, 0xFF, 0x00 }, { 0x66, 0xCC, 0xFF }, { 0x66, 0xCC, 0xCC },
   { 0x66, 0xCC, 0x99 }, { 0x66, 0xCC, 0x66 }, { 0x66, 0xCC, 0x33 }, { 0x66, 0xCC, 0x00 },
   { 0x66, 0x99, 0xFF }, { 0x66, 0x99, 0xCC }, { 0x66, 0x99, 0x99 }, { 0x66, 0x99, 0x66 },
   { 0x66, 0x99, 0x33 }, { 0x66, 0x99, 0x00 }, { 0x66, 0x66, 0xFF }, { 0x66, 0x66, 0xCC },
   { 0x66, 0x66, 0x99 }, { 0x66, 0x66, 0x66 }, { 0x66, 0x66, 0x33 }, { 0x66, 0x66, 0x00 },
   { 0x66, 0x33, 0xFF }, { 0x66, 0x33, 0xCC }, { 0x66, 0x33, 0x99 }, { 0x66, 0x33, 0x66 },
   { 0x66, 0x33, 0x33 }, { 0x66, 0x33, 0x00 }, { 0x66, 0x00, 0xFF }, { 0x66, 0x00, 0xCC },
   { 0x66, 0x00, 0x99 }, { 0x66, 0x00, 0x66 }, { 0x66, 0x00, 0x33 }, { 0x66, 0x00, 0x00 },
   { 0x33, 0xFF, 0xFF }, { 0x33, 0xFF, 0xCC }, { 0x33, 0xFF, 0x99 }, { 0x33, 0xFF, 0x66 },
   { 0x33, 0xFF, 0x33 }, { 0x33, 0xFF, 0x00 }, { 0x33, 0xCC, 0xFF }, { 0x33, 0xCC, 0xCC },
   { 0x33, 0xCC, 0x99 }, { 0x33, 0xCC, 0x66 }, { 0x33, 0xCC, 0x33 }, { 0x33, 0xCC, 0x00 },
   { 0x33, 0x99, 0xFF }, { 0x33, 0x99, 0xCC }, { 0x33, 0x99, 0x99 }, { 0x33, 0x99, 0x66 },
   { 0x33, 0x99, 0x33 }, { 0x33, 0x99, 0x00 }, { 0x33, 0x66, 0xFF }, { 0x33, 0x66, 0xCC },
   { 0x33, 0x66, 0x99 }, { 0x33, 0x66, 0x66 }, { 0x33, 0x66, 0x33 }, { 0x33, 0x66, 0x00 },
   { 0x33, 0x33, 0xFF }, { 0x33, 0x33, 0xCC }, { 0x33, 0x33, 0x99 }, { 0x33, 0x33, 0x66 },
   { 0x33, 0x33, 0x33 }, { 0x33, 0x33, 0x00 }, { 0x33, 0x00, 0xFF }, { 0x33, 0x00, 0xCC },
   { 0x33, 0x00, 0x99 }, { 0x33, 0x00, 0x66 }, { 0x33, 0x00, 0x33 }, { 0x33, 0x00, 0x00 },
   { 0x00, 0xFF, 0xFF }, { 0x00, 0xFF, 0xCC }, { 0x00, 0xFF, 0x99 }, { 0x00, 0xFF, 0x66 },
   { 0x00, 0xFF, 0x33 }, { 0x00, 0xFF, 0x00 }, { 0x00, 0xCC, 0xFF }, { 0x00, 0xCC, 0xCC },
   { 0x00, 0xCC, 0x99 }, { 0x00, 0xCC, 0x66 }, { 0x00, 0xCC, 0x33 }, { 0x00, 0xCC, 0x00 },
   { 0x00, 0x99, 0xFF }, { 0x00, 0x99, 0xCC }, { 0x00, 0x99, 0x99 }, { 0x00, 0x99, 0x66 },
   { 0x00, 0x99, 0x33 }, { 0x00, 0x99, 0x00 }, { 0x00, 0x66, 0xFF }, { 0x00, 0x66, 0xCC },
   { 0x00, 0x66, 0x99 }, { 0x00, 0x66, 0x66 }, { 0x00, 0x66, 0x33 }, { 0x00, 0x66, 0x00 },
   { 0x00, 0x33, 0xFF }, { 0x00, 0x33, 0xCC }, { 0x00, 0x33, 0x99 }, { 0x00, 0x33, 0x66 },
   { 0x00, 0x33, 0x33 }, { 0x00, 0x33, 0x00 }, { 0x00, 0x00, 0xFF }, { 0x00, 0x00, 0xCC },
   { 0x00, 0x00, 0x99 }, { 0x00, 0x00, 0x66 }, { 0x00, 0x00, 0x33 }, { 0xEE, 0x00, 0x00 },
   { 0xDD, 0x00, 0x00 }, { 0xBB, 0x00, 0x00 }, { 0xAA, 0x00, 0x00 }, { 0x88, 0x00, 0x00 },
   { 0x77, 0x00, 0x00 }, { 0x55, 0x00, 0x00 }, { 0x44, 0x00, 0x00 }, { 0x22, 0x00, 0x00 },
   { 0x11, 0x00, 0x00 }, { 0x00, 0xEE, 0x00 }, { 0x00, 0xDD, 0x00 }, { 0x00, 0xBB, 0x00 },
   { 0x00, 0xAA, 0x00 }, { 0x00, 0x88, 0x00 }, { 0x00, 0x77, 0x00 }, { 0x00, 0x55, 0x00 },
   { 0x00, 0x44, 0x00 }, { 0x00, 0x22, 0x00 }, { 0x00, 0x11, 0x00 }, { 0x00, 0x00, 0xEE },
   { 0x00, 0x00, 0xDD }, { 0x00, 0x00, 0xBB }, { 0x00, 0x00, 0xAA }, { 0x00, 0x00, 0x88 },
   { 0x00, 0x00, 0x77 }, { 0x00, 0x00, 0x55 }, { 0x00, 0x00, 0x44 }, { 0x00, 0x00, 0x22 },
   { 0x00, 0x00, 0x11 }, { 0xEE, 0xEE, 0xEE }, { 0xDD, 0xDD, 0xDD }, { 0xBB, 0xBB, 0xBB },
   { 0xAA, 0xAA, 0xAA }, { 0x88, 0x88, 0x88 }, { 0x77, 0x77, 0x77 }, { 0x55, 0x55, 0x55 },
   { 0x44, 0x44, 0x44 }, { 0x22, 0x22, 0x22 }, { 0x11, 0x11, 0x11 }, { 0x00, 0x00, 0x00 },
};

@implementation O2ImageSource_ICNS

#define BigEndianOSType(a,b,c,d) ((((uint32_t)(a))<<24)|(((uint32_t)(b))<<16)|(((uint32_t)(c))<<8)|(((uint32_t)(d))<<0))

+(BOOL)isPresentInDataProvider:(O2DataProvider *)provider {
   enum { signatureLength=4 };
   unsigned char signature[signatureLength] = { 'i','c','n','s' };
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
   _data=(NSData *)O2DataProviderCopyData(provider);
   _bytes=[_data bytes];
   _length=[_data length];
   _position=0;
   _images=[NSMutableArray new];
   return self;
}

-(void)dealloc {
   [_data release];
   [_images release];
   [super dealloc];
}

static uint8_t nextUnsigned8(O2ImageSource_ICNS *self) {
   if(self->_position<self->_length)
    return self->_bytes[self->_position++];

   [NSException raise:NSInvalidArgumentException format:@"Attempt to read past end of ICNS, length=%d",self->_length];
   return 0;
}

static uint32_t nextUnsigned32(O2ImageSource_ICNS *self) {
   unsigned result;
   uint32_t byte0=nextUnsigned8(self);
   uint32_t byte1=nextUnsigned8(self);
   uint32_t byte2=nextUnsigned8(self);
   uint32_t byte3=nextUnsigned8(self);

   result=byte0;
   result<<=8;
   result|=byte1;
   result<<=8;
   result|=byte2;
   result<<=8;
   result|=byte3;

   return result;
}

-(O2ICNSNode *)createNodeForWidth:(size_t)width height:(size_t)height bitsPerPixel:(size_t)bitsPerPixel isMask:(bool)isMask {
   O2ICNSNode *result=NSZoneMalloc(NULL,sizeof(O2ICNSNode));
   result->width=width;
   result->height=height;
   result->bitsPerPixel=bitsPerPixel;   
   result->isMask=isMask;
   if(result->isMask)
    result->samples=NSZoneCalloc(NULL,width*height,sizeof(uint8_t));
   else {
    result->samples=NSZoneCalloc(NULL,width*height,sizeof(O2rgba8u_BE));
    
    O2rgba8u_BE *pixels=result->samples;
    int i;
    
    for(i=0;i<width*height;i++)
     pixels[i].a=0xFF;
   }
   
   if(result->isMask){
    result->next=_maskNodes;
    _maskNodes=result;
   }
   else {
    result->next=_iconNodes;
    _iconNodes=result;
   }
    
   return result;
}

-(void)parseIcon {
   uint32_t type=nextUnsigned32(self);
   uint32_t length=nextUnsigned32(self)-8;
   uint32_t width=0;
   uint32_t height=0;
   uint32_t bitsPerPixel=0;
   bool     isMask=FALSE;
   bool     maskFollows=FALSE;
   
   //NSLog(@" .icns icon type=%c %c %c %c",type>>24,type>>16,type>>8,type);

   switch(type){
   
    case BigEndianOSType('I','C','O','N'):
     width=32;
     height=32;
     bitsPerPixel=1;
     break;
     
    case BigEndianOSType('I','C','N','#'):
     width=32;
     height=32;
     bitsPerPixel=1;
     maskFollows=TRUE;
     break;
     
    case BigEndianOSType('i','c','m','#'):
     width=16;
     height=12;
     bitsPerPixel=1;
     isMask=TRUE;
     break;
     
    case BigEndianOSType('i','c','m','4'):
     width=16;
     height=12;
     bitsPerPixel=4;
     break;
     
    case BigEndianOSType('i','c','m','8'):
     width=16;
     height=12;
     bitsPerPixel=8;
     break;
     
    case BigEndianOSType('i','c','s','#'):
     width=16;
     height=16;
     bitsPerPixel=1;
     isMask=TRUE;
     break;
     
    case BigEndianOSType('i','c','s','4'):
     width=16;
     height=16;
     bitsPerPixel=4;
     break;
     
    case BigEndianOSType('i','c','s','8'):
     width=16;
     height=16;
     bitsPerPixel=8;
     break;
     
    case BigEndianOSType('i','s','3','2'):
     width=16;
     height=16;
     bitsPerPixel=24;
     break;
     
    case BigEndianOSType('s','8','m','k'):
     width=16;
     height=16;
     bitsPerPixel=8;
     isMask=TRUE;
     break;
     
    case BigEndianOSType('i','c','l','4'):
     width=32;
     height=32;
     bitsPerPixel=4;
     break;
     
    case BigEndianOSType('i','c','l','8'):
     width=32;
     height=32;
     bitsPerPixel=8;
     break;
     
    case BigEndianOSType('i','l','3','2'):
     width=32;
     height=32;
     bitsPerPixel=24;
     break;
     
    case BigEndianOSType('l','8','m','k'):
     width=32;
     height=32;
     bitsPerPixel=8;
     isMask=TRUE;
     break;
     
    case BigEndianOSType('i','c','h','#'):
     width=48;
     height=48;
     bitsPerPixel=1;
     isMask=TRUE;
     break;
     
    case BigEndianOSType('i','c','h','4'):
     width=48;
     height=48;
     bitsPerPixel=4;
     break;
     
    case BigEndianOSType('i','c','h','8'):
     width=48;
     height=48;
     bitsPerPixel=8;
     break;
     
    case BigEndianOSType('i','h','3','2'):
     width=48;
     height=48;
     bitsPerPixel=24;
     break;
     
    case BigEndianOSType('h','8','m','k'):
     width=48;
     height=48;
     bitsPerPixel=8;
     isMask=TRUE;
     break;
     
    case BigEndianOSType('i','t','3','2'):
     width=128;
     height=128;
     bitsPerPixel=24;
     break;
     
    case BigEndianOSType('t','8','m','k'):
     width=128;
     height=128;
     bitsPerPixel=8;
     isMask=TRUE;
     break;
     
    case BigEndianOSType('i','c','0','8'):
     width=256;
     height=256;
     bitsPerPixel=0;
     break;
     
    case BigEndianOSType('i','c','0','9'):
     width=512;
     height=512;
     bitsPerPixel=0;
     break;
    
    default:
     //NSLog(@"unknown .icns icon type=%c %c %c %c",type>>24,type>>16,type>>8,type);
     break;
   }
   
   if(isMask){
    O2ICNSNode    *node=[self createNodeForWidth:width height:height bitsPerPixel:bitsPerPixel isMask:TRUE];
    uint8_t       *pixels=node->samples;
    int            pixelOffset=0,byteOffset=0,byteLength;
    const uint8_t *bytes=_bytes+_position;
    
    if(bitsPerPixel==1)
     byteLength=MIN(length,(width*height)/8);
    else
     byteLength=MIN(length,width*height);

    for(byteOffset=0;byteOffset<byteLength;byteOffset++){
     switch(bitsPerPixel){
     
      case 1:;
       int i;
       uint8_t mask=bytes[byteOffset];
       
       for(i=0;i<8;i++)
        pixels[pixelOffset++]=((mask<<i)&0x80)?0xFF:0x00;
       break;

      case 8:
       pixels[pixelOffset++]=bytes[byteOffset];
       break;
     }
    }
   }
#if 0
   else if(bitsPerPixel==1){
    O2ICNSNode    *node=[self createNodeForWidth:width height:height bitsPerPixel:bitsPerPixel isMask:FALSE];
    O2rgba8u_BE   *pixels=node->samples;
    int            pixelOffset=0,byteOffset=0,byteLength=MIN(length,(width*height)/8);
    const uint8_t *bytes=_bytes+_position;
    
    for(byteOffset=0;byteOffset<byteLength;byteOffset++){
     uint8_t mask=bytes[byteOffset];
     int i;
       
     for(i=0;i<8;i++){
      if((mask<<i)&0x80){
       pixels[pixelOffset].r=0xFF;
       pixels[pixelOffset].g=0xFF;
       pixels[pixelOffset].b=0xFF;
      }
      else {
       pixels[pixelOffset].r=0x00;
       pixels[pixelOffset].g=0x00;
       pixels[pixelOffset].b=0x00;
      }
      pixelOffset++;
     }
    }
   }
   else if(bitsPerPixel==4){
    O2ICNSNode    *node=[self createNodeForWidth:width height:height bitsPerPixel:bitsPerPixel isMask:FALSE];
    O2rgba8u_BE   *pixels=node->samples;
    int            pixelOffset=0,byteOffset=0,byteLength=MIN(length,(width*height)/2);
    const uint8_t *bytes=_bytes+_position;
    
    for(byteOffset=0;byteOffset<byteLength;byteOffset++){
     uint8_t indexes=bytes[byteOffset];
     uint8_t hi=indexes>>4;
     uint8_t lo=indexes&0x0F;
     
     pixels[pixelOffset].r=fourBitCLUT[hi][0];
     pixels[pixelOffset].g=fourBitCLUT[hi][1];
     pixels[pixelOffset].b=fourBitCLUT[hi][2];
     pixelOffset++;
     pixels[pixelOffset].r=fourBitCLUT[lo][0];
     pixels[pixelOffset].g=fourBitCLUT[lo][1];
     pixels[pixelOffset].b=fourBitCLUT[lo][2];
     pixelOffset++;
    }
   }
   else if(bitsPerPixel==8){
    O2ICNSNode    *node=[self createNodeForWidth:width height:height bitsPerPixel:bitsPerPixel isMask:FALSE];
    O2rgba8u_BE   *pixels=node->samples;
    int            pixelOffset=0,byteOffset=0,byteLength=MIN(length,(width*height));
    const uint8_t *bytes=_bytes+_position;
    
    for(byteOffset=0;byteOffset<byteLength;byteOffset++){
     uint8_t index=bytes[byteOffset];
     
     pixels[pixelOffset].r=eightBitCLUT[index][0];
     pixels[pixelOffset].g=eightBitCLUT[index][1];
     pixels[pixelOffset].b=eightBitCLUT[index][2];
     pixelOffset++;
    }
   }
#endif
   else if(bitsPerPixel==24){
    O2ICNSNode    *node=[self createNodeForWidth:width height:height bitsPerPixel:bitsPerPixel isMask:FALSE];
    int            pixelCount=width*height;
    O2rgba8u_BE   *pixels=node->samples;
        
    if(length==width*height*3){
     int            pixelOffset=0,byteOffset=0;
     const uint8_t *bytes=_bytes+_position;
    
     for(byteOffset=0;byteOffset<length;){     
      pixels[pixelOffset].r=bytes[byteOffset++];
      pixels[pixelOffset].g=bytes[byteOffset++];
      pixels[pixelOffset].b=bytes[byteOffset++];
      pixelOffset++;
     }
    }
    else {
     int            pixelOffset,rleOffset=0;
     const uint8_t *rleBytes=_bytes+_position;
     enum { COMPONENT_R,COMPONENT_G,COMPONENT_B} componentSlot=COMPONENT_R;
        
     // 24 is RLE encoded.
    if(rleBytes[0]==0 && rleBytes[1]==0 && rleBytes[2]==0 && rleBytes[3]==0)
     rleOffset+=4;

    for(;rleOffset<length;){
      for(pixelOffset=0;pixelOffset<pixelCount;){
       if(rleBytes[rleOffset]&0x80){
        unsigned  rleInfo=rleBytes[rleOffset++];
        unsigned  i,chunkLength=(rleInfo&0x7F)+3;
        uint8_t componentValue=rleBytes[rleOffset++];

        for(i=0;i<chunkLength;i++){
         switch(componentSlot){
          case COMPONENT_R:
           pixels[pixelOffset++].r=componentValue;
           break;
          case COMPONENT_G:
           pixels[pixelOffset++].g=componentValue;
           break;
          case COMPONENT_B:
           pixels[pixelOffset++].b=componentValue;
           break;
         }
        }
       }
       else {
        unsigned i,chunkLength=rleBytes[rleOffset++]+1;
       
        for(i=0;i<chunkLength;i++)
         switch(componentSlot){
          case COMPONENT_R:
           pixels[pixelOffset++].r=rleBytes[rleOffset++];
           break;
          case COMPONENT_G:
           pixels[pixelOffset++].g=rleBytes[rleOffset++];
           break;
          case COMPONENT_B:
           pixels[pixelOffset++].b=rleBytes[rleOffset++];
           break;
         }
       }
      }
      componentSlot++;
     }
    }
   }

   _position+=length;
}

-(O2ICNSNode *)maskNodeForIconNode:(O2ICNSNode *)imageNode {
   O2ICNSNode *check,*best=NULL;
   
   for(check=_maskNodes;check!=NULL;check=check->next){
    if(check->width==imageNode->width && check->height==imageNode->height){
     if(best==NULL)
      best=check;
     else if(check->bitsPerPixel>best->bitsPerPixel && check->bitsPerPixel<=imageNode->bitsPerPixel)
      best=check;
    }
   }
   
   return best;
}

-(void)parseImages {
   uint32_t magic=nextUnsigned32(self);
   uint32_t length=nextUnsigned32(self);

   while(_position<_length)
    [self parseIcon];

   O2ICNSNode *iconNode;
   
   for(iconNode=_iconNodes;iconNode!=NULL;iconNode=iconNode->next){
    O2ICNSNode  *maskNode=[self maskNodeForIconNode:iconNode];
    O2rgba8u_BE *pixels=iconNode->samples;
    int          i,pixelCount=iconNode->width*iconNode->height;
    
    if(maskNode!=NULL){
     uint8_t *coverage=maskNode->samples;
     
     for(i=0;i<pixelCount;i++){
      pixels[i].a=coverage[i];
     }
    }
    
    for(i=0;i<pixelCount;i++){
     pixels[i].r=O2Image_8u_mul_8u_div_255(pixels[i].r,pixels[i].a);
     pixels[i].g=O2Image_8u_mul_8u_div_255(pixels[i].g,pixels[i].a);
     pixels[i].b=O2Image_8u_mul_8u_div_255(pixels[i].b,pixels[i].a);
    }
    
    O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
    NSData         *bitmap=[[NSData alloc] initWithBytesNoCopy:pixels length:pixelCount*sizeof(O2rgba8u_BE)];
       O2DataProvider *provider=O2DataProviderCreateWithCFData((CFDataRef)bitmap);
    O2BitmapInfo    info=kO2BitmapByteOrder32Big|kO2ImageAlphaPremultipliedLast;

    O2Image *result=[[O2Image alloc] initWithWidth:iconNode->width height:iconNode->height bitsPerComponent:8 bitsPerPixel:32 bytesPerRow:iconNode->width*sizeof(O2rgba8u_BE) colorSpace:colorSpace bitmapInfo:info decoder:NULL provider:provider decode:NULL interpolate:NO renderingIntent:kO2RenderingIntentDefault];

    O2ColorSpaceRelease(colorSpace);
    O2DataProviderRelease(provider);
    [bitmap release];
    [_images addObject:result];
    O2ImageRelease(result);
   }
   
   O2ICNSNode *next;
   
   for(iconNode=_iconNodes;iconNode!=NULL;iconNode=next){
    next=iconNode->next;
    // samples is used by the image data
    free(iconNode);
   }
   _iconNodes=NULL;
   
   for(iconNode=_maskNodes;iconNode!=NULL;iconNode=next){
    next=iconNode->next;
    free(iconNode->samples);
    free(iconNode);
   }
   _maskNodes=NULL;  
}

-(void)parseIfNeeded {
   if(!_parsed){
    _parsed=TRUE;
    [self parseImages];
   }
}

- (CFStringRef)type
{
    return (CFStringRef)@"com.apple.icns";
}


-(unsigned)count {
   [self parseIfNeeded];
   return [_images count];
}

-(CFDictionaryRef)copyPropertiesAtIndex:(unsigned)index options:(CFDictionaryRef)options {
   return nil;
}

-(O2Image *)createImageAtIndex:(unsigned)index options:(NSDictionary *)options {
   [self parseIfNeeded];
   
   if(index>=[self count])
    return nil;
   
   return [[_images objectAtIndex:index] retain];
}

@end
