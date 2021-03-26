/* Copyright (c) 2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

/*  zlib decode is based on the public domain zlib decode v0.2 by Sean Barrett 2006-11-18  http://www.nothings.org/stb_image.c  V 0.57 */

#import <Onyx2D/O2PDFFilter.h>
#import <Onyx2D/O2PDFObject.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Foundation/NSData.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Onyx2D/O2zlib.h>
#import <Onyx2D/O2LZW.h>
#import <string.h>
#import <Onyx2D/O2ImageSource_JPEG.h>

NSData *O2PDFFilterWithName(const char *name,NSData *data,O2PDFDictionary *parameters) {
   return [O2PDFFilter decodeWithName:name data:data parameters:parameters];
}

@implementation O2PDFFilter


+(NSData *)FlateDecode_data:(NSData *)data parameters:(O2PDFDictionary *)parameters {
   int len;
   unsigned char *result=stbi_zlib_decode_malloc([data bytes],[data length],&len);
   
   if(result==NULL)
    return nil;

   return [NSData dataWithBytesNoCopy:result length:len];
}

+(NSData *)LZWDecode_data:(NSData *)data parameters:(O2PDFDictionary *)parameters {
   return LZWDecodeWithExpectedResultLength(data,[data length]*10);
}

+(NSData *)decodeWithName:(const char *)name data:(NSData *)data parameters:(O2PDFDictionary *)parameters {
   if((strcmp(name,"FlateDecode")==0) || (strcmp(name,"FL")==0))
     data=[self FlateDecode_data:data parameters:parameters];
   else if((strcmp(name,"DCTDecode")==0) || (strcmp(name,"DCT")==0)){
    return O2DCTDecode(data);
   }
   else if((strcmp(name,"LZWDecode")==0) || (strcmp(name,"LZW")==0)){
    data=LZWDecodeWithExpectedResultLength(data,[data length]*10);
   }
   else {
    O2PDFError(__FILE__,__LINE__,@"Unknown O2PDFFilter name = %s, parameters=%@",name,parameters);
    return nil;
   }
    
   O2PDFInteger predictor=0;
   
    if([parameters getIntegerForKey:"Predictor" value:&predictor]){
     if(predictor>1){
      NSMutableData *mutable=[NSMutableData data];
      const  char *bytes=[data bytes];
      unsigned             length=[data length];
      O2PDFInteger colors;
      O2PDFInteger bitsPerComponent;
      O2PDFInteger columns;
      int          bytesPerRow;
      int          row,rowLength,numberOfRows;
      
      if(![parameters getIntegerForKey:"Colors" value:&colors])
       colors=1;
      if(![parameters getIntegerForKey:"BitsPerComponent" value:&bitsPerComponent])
       bitsPerComponent=8;
      if(![parameters getIntegerForKey:"Columns" value:&columns])
       columns=1;
       
//NSLog(@"predictor=%d,colors=%d,bpc=%d,columns=%d,length=%d",predictor,colors,bitsPerComponent,columns,length);

      bytesPerRow=(((colors*bitsPerComponent)*columns)+7)/8;
      rowLength=bytesPerRow+1;
      numberOfRows=length/rowLength;

#if 0
      if((length%rowLength)!=0)
       ;//NSLog(@"length mod rowLength=%d",length%rowLength);
        
#endif
      char *change=__builtin_alloca(rowLength);
       
      for(row=0;row<numberOfRows;row++){
       int i,filter=bytes[0];
       
       for(i=0;i<rowLength-1;i++)
        change[i]=bytes[1+i];
        
       if(filter==0){
        // do nothing
       }
       else if(filter==1){
        int last=change[0];
        
        for(i=1;i<rowLength-1;i++){
         last=last+change[i];              
         change[i]=last;
        }
       }
       else if(filter==2){
        int last=change[0];
        
        for(i=1;i<rowLength-1;i++){
         last=last-change[i];              
         change[i]=last;
        }
       }
       else {
        NSLog(@"unsupported filter %d for predictor %d",filter,predictor);
       }
       
       [mutable appendBytes:change length:rowLength-1];
       bytes+=rowLength;
       length-=rowLength;
      }
      if(length>1)
       [mutable appendBytes:bytes length:length-1];
      data=mutable;
     }
    }
   
    return data;
   }

@end
