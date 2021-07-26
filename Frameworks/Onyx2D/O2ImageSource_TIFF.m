/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2ImageSource_TIFF.h>
#import <Onyx2D/O2Decoder_TIFF.h>
#import "O2TIFFImageDirectory.h"
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2ColorSpace.h>
#import <Onyx2D/O2Image.h>

@implementation O2ImageSource_TIFF

+(BOOL)isPresentInDataProvider:(O2DataProvider *)provider {
   enum { signatureLength=4 };
   unsigned char signature[2][signatureLength] = {
    { 'M','M',00,42 },
    { 'I','I',42,00 }
   };
   unsigned char check[signatureLength];
   NSInteger     i,size=[provider getBytes:check range:NSMakeRange(0,signatureLength)];
   
   if(size!=signatureLength)
    return NO;
   
   int s;
   for(s=0;s<2;s++){
    for(i=0;i<signatureLength;i++)
     if(signature[s][i]!=check[i])
      break;
      
    if(i==signatureLength)
     return YES;
   }
   return NO;
}


-initWithDataProvider:(O2DataProvider *)provider options:(NSDictionary *)options {
   [super initWithDataProvider:provider options:options];
   
   NSData *data=(NSData *)O2DataProviderCopyData(provider);
   _reader=[[O2Decoder_TIFF alloc] initWithData:data];
   [data release];
   
   if(_reader==nil){
    [self dealloc];
    return nil;
   }
   return self;
}

-(void)dealloc {
   [_reader release];
   [super dealloc];
}

- (CFStringRef)type
{
    return (CFStringRef)@"public.tiff";
}

-(unsigned)count {
   return [[_reader imageFileDirectory] count];
}

-(CFDictionaryRef)copyPropertiesAtIndex:(unsigned)index options:(CFDictionaryRef)options {
   NSArray *entries=[_reader imageFileDirectory];
   
   if([entries count]<=index)
    return nil;
   
   O2TIFFImageDirectory *directory=[entries objectAtIndex:index];
   
   return (CFDictionaryRef)[[directory properties] copy];
}


-(O2ImageRef)createImageAtIndex:(unsigned)index options:(CFDictionaryRef)options {
   NSArray *entries=[_reader imageFileDirectory];
   
   if([entries count]<=index)
    return nil;
   
   O2TIFFImageDirectory *directory=[entries objectAtIndex:index];
   
   int            width=[directory imageWidth];
   int            height=[directory imageLength];
   int            bitsPerPixel=32;
   int            bytesPerRow=(bitsPerPixel/(sizeof(char)*8))*width;
   
   unsigned char *bytes;
   NSData        *bitmap;
   
   bytes=NSZoneMalloc([self zone],bytesPerRow*height);
   if (bytes == NULL) {
        NSLog(@"Can't allocate %d bytes for %dx%d bitmap for provider %@", bytesPerRow*height, width, height, _provider);
         return nil;
    }

   if(![directory getRGBAImageBytes:bytes data:[_reader data]]){
    NSZoneFree([self zone],bytes);
    return nil;
   }

// clamp premultiplied data, this should probably be moved into the O2Image init
   int i;
   for(i=0;i<bytesPerRow*height;i+=4){
    bytes[i]=MIN(bytes[i],bytes[i+3]);
    bytes[i+1]=MIN(bytes[i+1],bytes[i+3]);
    bytes[i+2]=MIN(bytes[i+2],bytes[i+3]);
   }

   bitmap=[[NSData alloc] initWithBytesNoCopy:bytes length:bytesPerRow*height];

    O2DataProvider *provider=O2DataProviderCreateWithCFData((CFDataRef)bitmap);
   O2ColorSpaceRef colorSpace=O2ColorSpaceCreateDeviceRGB();
   O2Image *image=[[O2Image alloc] initWithWidth:width height:height bitsPerComponent:8 bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow
      colorSpace:colorSpace bitmapInfo:kO2BitmapByteOrder32Big|kO2ImageAlphaPremultipliedLast decoder:NULL provider:provider decode:NULL interpolate:NO renderingIntent:kO2RenderingIntentDefault];
      
   [colorSpace release];
   [provider release];
   [bitmap release];
   
   return image;
}

@end
