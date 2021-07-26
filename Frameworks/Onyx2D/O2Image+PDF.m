#import <Onyx2D/O2Image+PDF.h>
#import <Onyx2D/O2ColorSpace+PDF.h>
#import <Onyx2D/O2DataProvider.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFStream.h>
#import <Onyx2D/O2PDFContext.h>

#ifdef __APPLE__
#else
#import "O2Defines_zlib.h"
#endif

#if ZLIB_PRESENT
#import <zlib.h>
#endif

@implementation O2Image(PDF)

O2ColorRenderingIntent O2ImageRenderingIntentWithName(const char *name) {
   if(name==NULL)
    return kO2RenderingIntentDefault;
    
   if(strcmp(name,"AbsoluteColorimetric")==0)
    return kO2RenderingIntentAbsoluteColorimetric;
   else if(strcmp(name,"RelativeColorimetric")==0)
    return kO2RenderingIntentRelativeColorimetric;
   else if(strcmp(name,"Saturation")==0)
    return kO2RenderingIntentSaturation;
   else if(strcmp(name,"Perceptual")==0)
    return kO2RenderingIntentPerceptual;
   else
    return kO2RenderingIntentDefault; // unknown
}

const char *O2ImageNameWithIntent(O2ColorRenderingIntent intent){
   switch(intent){
   
    case kO2RenderingIntentAbsoluteColorimetric:
     return "AbsoluteColorimetric";

    case kO2RenderingIntentRelativeColorimetric:
     return "RelativeColorimetric";

    case kO2RenderingIntentSaturation:
     return "Saturation";
     
    default:
    case kO2RenderingIntentDefault:
    case kO2RenderingIntentPerceptual:
     return "Perceptual";
   } 
}

-(O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context {
	O2PDFStream     *result=[O2PDFStream pdfStream];
   O2PDFDictionary *dictionary=[result dictionary];

   [dictionary setNameForKey:"Type" value:"XObject"];
   [dictionary setNameForKey:"Subtype" value:"Image"];
   [dictionary setIntegerForKey:"Width" value:_width];
   [dictionary setIntegerForKey:"Height" value:_height];
   if(_colorSpace!=nil)
    [dictionary setObjectForKey:"ColorSpace" value:[_colorSpace encodeReferenceWithContext:context]];
   [dictionary setIntegerForKey:"BitsPerComponent" value:_bitsPerComponent];
   [dictionary setNameForKey:"Intent" value:O2ImageNameWithIntent(_renderingIntent)];
   [dictionary setBooleanForKey:"ImageMask" value:_isMask];
   if(_mask!=nil)
    [dictionary setObjectForKey:"Mask" value:[_mask encodeReferenceWithContext:context]];
   if(_decode!=NULL)
    [dictionary setObjectForKey:"Decode" value:[O2PDFArray pdfArrayWithNumbers:_decode count:O2ColorSpaceGetNumberOfComponents(_colorSpace)*2]];
	[dictionary setBooleanForKey:"Interpolate" value:_interpolate];

    if(O2ImageDecoderGetCompressionType(_decoder)==O2ImageCompressionJPEG) {
        // If the image is JPEG compressed, we can put the JPEG data in the PDF, using a DCTDecode filter
        O2DataProviderRef dataProvider=O2ImageDecoderGetDataProvider(_decoder);
        CFDataRef dctData=O2DataProviderCopyData(dataProvider);
        
        [dictionary setNameForKey:"Filter" value:"DCTDecode"];
        
        [[result mutableData] appendData:(NSData *)dctData];
        CFRelease(dctData);
    } else {
#define CHUNK 65536
        
        // Input buffer for image data
        uint8_t in[CHUNK + 3]; // CHUNK size + some additional room for rgb
        int idx = 0;
        
#if ZLIB_PRESENT
        // Put ZLIB compressed data in the PDF, using a DCTDecode filter
        [dictionary setNameForKey:"Filter" value:"FlateDecode"];
        
        // allocate deflate state
        unsigned have;
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        
        deflateInit(&strm, 9);
        
        // Compressed output buffer
        uint8_t out[CHUNK + 3];
#else
        // No compression : out buffer = in buffer
        uint8_t *out = in;
#endif
        
        const void *bytes = [self directBytes];
        
        /* FIX, generate soft mask for alpha data
         [dictionary setObjectForKey:"SMask" value:[softMask encodeReferenceWithContext:context]];
         */
        
        // It would be nice if jpg data would stay jpg data (instead of an uncompress stream), as it does
        // with Quartz and CGImage
        
        // Export RGB bytes, without the alpha data, in the expected order
        // TODO : support non 32 bits pixels, respect the premultiplied state, non-RGB pixels...
        const uint8_t *ptr = bytes;
        int alphaInfo = _bitmapInfo & kO2BitmapAlphaInfoMask;
        BOOL hasAlpha = alphaInfo != kO2ImageAlphaNone;
        BOOL alphaFirst = alphaInfo == kO2ImageAlphaPremultipliedFirst || alphaInfo == kO2ImageAlphaFirst || alphaInfo == kO2ImageAlphaNoneSkipFirst;
        BOOL alphaLast = hasAlpha && (alphaFirst == NO);
        BOOL bigEndian = _bitmapInfo & kO2BitmapByteOrder32Big;
        BOOL littleEndian = bigEndian == NO;
        int bytesPerPixel = _bitsPerPixel/8;
        for (int i = 0; i < _height; i++, ptr += _bytesPerRow) {
            const uint8_t *linePtr = ptr;
            for (int j = 0; j < _width; j++, linePtr += bytesPerPixel) {
                const uint8_t *pixelPtr = linePtr;
                /*
                 AlphaFirst => The Alpha channel is next to the Red channel
                 (ARGB and BGRA are both Alpha First formats)
                 AlphaLast => The Alpha channel is next to the Blue channel
                 (RGBA and ABGR are both Alpha Last formats)
                 
                 LittleEndian => Blue comes before Red
                 (BGRA and ABGR are Little endian formats)
                 BigEndian => Red comes before Blue
                 (ARGB and RGBA are Big endian formats).
                 */
                uint8_t r = 0, g = 0, b = 0;
                if ((alphaFirst && bigEndian) || (alphaLast && littleEndian)) {
                    // Skip the alpha
                    ++pixelPtr;
                }
                if (bigEndian) {
                    r = *pixelPtr++;
                    g = *pixelPtr++;
                    b = *pixelPtr++;
                } else {
                    b = *pixelPtr++;
                    g = *pixelPtr++;
                    r = *pixelPtr++;
                }
                in[idx++] = r;
                in[idx++] = g;
                in[idx++] = b;
                BOOL flush = (i == _height - 1 && j == _width - 1) || (idx > CHUNK);
                if (flush) {
#if ZLIB_PRESENT
                    strm.avail_in = idx;
                    flush = ((i == _height - 1 && j == _width - 1)) ? Z_FINISH : Z_NO_FLUSH;
                    strm.next_in = in;
                    
                    // run deflate() on input until the output buffer is not full
                    do {
                        strm.avail_out = idx;
                        strm.next_out = out;
                        deflate(&strm, flush); 
                        have = idx - strm.avail_out;
                        [[result mutableData] appendBytes:out length: have];
                    } while (strm.avail_out == 0);
#else
                    [[result mutableData] appendBytes:out length: idx];
#endif
                    idx = 0;
                }
            }
        }
#if ZLIB_PRESENT
        deflateEnd(&strm);
#endif
    }
	return [context encodeIndirectPDFObject:result];
}



+(O2Image *)imageWithPDFObject:(O2PDFObject *)object {
   O2PDFStream     *stream=(O2PDFStream *)object;
   O2PDFDictionary *dictionary=[stream dictionary];
   O2PDFInteger width;
   O2PDFInteger height;
   O2PDFObject *colorSpaceObject=NULL;
   O2PDFInteger bitsPerComponent;
   const char  *intent;
   O2PDFBoolean isImageMask=NO;
   O2PDFObject *imageMaskObject=NULL;
   O2ColorSpaceRef colorSpace=NULL;
    int               componentsPerPixel;
   O2PDFArray     *decodeArray;
   float            *decode=NULL;
   BOOL              interpolate;
   O2PDFStream *softMaskStream=nil;
   O2Image *softMask=NULL;
    
   if(![dictionary getIntegerForKey:"Width" value:&width]){
    O2PDFError(__FILE__,__LINE__,@"Image has no Width");
    return NULL;
   }
   if(![dictionary getIntegerForKey:"Height" value:&height]){
    O2PDFError(__FILE__,__LINE__,@"Image has no Height");
    return NULL;
   }
    
   if(![dictionary getIntegerForKey:"BitsPerComponent" value:&bitsPerComponent]){
    O2PDFError(__FILE__,__LINE__,@"Image has no BitsPerComponent");
    return NULL;
   }
          
   if(![dictionary getNameForKey:"Intent" value:&intent])
    intent=NULL;
     
   [dictionary getBooleanForKey:"ImageMask" value:&isImageMask];
    
   if(isImageMask)
    O2PDFFix(__FILE__,__LINE__,@"ImageMask present");
    
   if([dictionary getObjectForKey:"Mask" value:&imageMaskObject]){
    O2PDFFix(__FILE__,__LINE__,@"Mask present");
   }

   if([dictionary getObjectForKey:"ColorSpace" value:&colorSpaceObject]){
    if((colorSpace=[O2ColorSpace createColorSpaceFromPDFObject:colorSpaceObject])==NULL){
     O2PDFError(__FILE__,__LINE__,@"Unable to create ColorSpace %@",colorSpaceObject);
     return NULL;
    }
   }
   
   if(!isImageMask && colorSpace==NULL){
    O2PDFError(__FILE__,__LINE__,@"Image has no ColorSpace %@",dictionary);
    return NULL;
   }
  
   if(colorSpace==NULL)
    componentsPerPixel=1;
   else
    componentsPerPixel=O2ColorSpaceGetNumberOfComponents(colorSpace);

   if(![dictionary getArrayForKey:"Decode" value:&decodeArray])
    decode=NULL;
   else {
    unsigned count;
     
    if(![decodeArray getNumbers:&decode count:&count]){
     O2PDFError(__FILE__,__LINE__,@"Unable to read decode array %@",decodeArray);
     return NULL;
    }
    
    if(count!=componentsPerPixel*2){
     O2PDFError(__FILE__,__LINE__,@"Invalid decode array, count=%d, should be %d",count,componentsPerPixel*2);
      return NULL;
     }
    }
    
   if(![dictionary getBooleanForKey:"Interpolate" value:&interpolate])
    interpolate=NO;
    
   if([dictionary getStreamForKey:"SMask" value:&softMaskStream]){
    softMask=[self imageWithPDFObject:softMaskStream];
   }
    
    int               bitsPerPixel=componentsPerPixel*bitsPerComponent;
    int               bytesPerRow=((width*bitsPerPixel)+7)/8;
    NSData           *data=[stream data];
    O2DataProvider * provider;
    O2Image *image=NULL;
       
//     NSLog(@"width=%d,height=%d,bpc=%d,bpp=%d,bpr=%d,cpp=%d",width,height,bitsPerComponent,bitsPerPixel,bytesPerRow,componentsPerPixel);
     
    if(height*bytesPerRow!=[data length]){
    O2PDFError(__FILE__,__LINE__,@"Invalid data length=%d,should be %d=%d",[data length],height*bytesPerRow,[data length]-height*bytesPerRow);
   }
   
   if(height*bytesPerRow>[data length]){
    // provide some gray data
     NSMutableData *mutable=[NSMutableData dataWithLength:height*bytesPerRow];
     char *mbytes=[mutable mutableBytes];
      int i;
      for(i=0;i<height*bytesPerRow;i++)
     mbytes[i]=i;
       
     data=mutable;
    }
        
    provider=O2DataProviderCreateWithCFData((CFDataRef)data);
    if(isImageMask){      
     image=[[O2Image alloc] initMaskWithWidth:width height:height bitsPerComponent:bitsPerComponent bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow provider:provider decode:decode interpolate:interpolate];
    }
    else {
        image=[[O2Image alloc] initWithWidth:width height:height bitsPerComponent:bitsPerComponent bitsPerPixel:bitsPerPixel bytesPerRow:bytesPerRow colorSpace:colorSpace bitmapInfo:0 decoder:NULL provider:provider decode:decode interpolate:interpolate renderingIntent:O2ImageRenderingIntentWithName(intent)];

     if(softMask!=NULL)
     [image setMask:softMask];
    }

   if(decode!=NULL)
    NSZoneFree(NULL,decode);
    
   O2DataProviderRelease(provider);
   O2ColorSpaceRelease(colorSpace);
   
	return image;
   }

@end

