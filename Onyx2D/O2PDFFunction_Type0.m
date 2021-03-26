/* Copyright (c) 2010 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "O2PDFFunction_Type0.h"
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFContext.h>
#import <Foundation/NSArray.h>
#import <stddef.h>

@implementation O2PDFFunction_Type0

static void evaluate(void *info,const float *input,float *output) {
   O2PDFFunction_Type0 *self=info;

   if(self->_sizeCount!=1){
    O2PDFError(__FILE__,__LINE__,@"Type0 can not handle size count!=1");
    return;
   }
   
   if(self->_domainCount/2!=1){
    O2PDFError(__FILE__,__LINE__,@"Type0 can not handle size more than one input");
    return;
   }
   
   if(self->_bitsPerSample!=8){
    O2PDFError(__FILE__,__LINE__,@"Type0 can not handle bps!=8");
    return;
   }
   if(self->_order!=1){
    O2PDFError(__FILE__,__LINE__,@"Type0 can not handle order!=1");
    return;
   }
// FIXME, scale input to domain, more bounds checking
   int      i,numberOfOutputs=self->_rangeCount/2;

   int      inputIndex=MIN(MAX(0,input[0]*(self->_size[0]-1)),self->_size[self->_sizeCount-1]);
   int      bytesPerSample=self->_bitsPerSample/8;
   int      sampleOffset=inputIndex*(bytesPerSample*numberOfOutputs);
      
   const uint8_t *samples=self->_bytes+sampleOffset;
   
    for(i=0;i<numberOfOutputs;i++){
     output[i]=samples[i];
     output[i]=output[i]/255.0;
    }
}

-initWithDomain:(O2PDFArray *)domain range:(O2PDFArray *)range size:(O2PDFArray *)size bitsPerSample:(O2PDFInteger)bps order:(O2PDFInteger)order encode:(O2PDFArray *)encode decode:(O2PDFArray *)decode data:(NSData *)data {
   if([super initWithDomain:domain range:range]==nil)
    return nil;

   _info=self;
   _callbacks.evaluate=evaluate;

   _sizeCount=0;
   [size getIntegers:&_size count:&_sizeCount];
   _bitsPerSample=bps;
   _order=order;
   [encode getNumbers:&_encode count:&_encodeCount];
   [decode getNumbers:&_decode count:&_decodeCount];
   _data=[data retain];
   _dataLength=[data length];
   _bytes=[data bytes];
     
   return self;
}

-(void)dealloc {
   if(_size!=NULL)
    NSZoneFree(NULL,_size);
   if(_encode!=NULL)
    NSZoneFree(NULL,_encode);
   if(_decode!=NULL)
    NSZoneFree(NULL,_decode);
   [_data release];
   [super dealloc];
}

-(BOOL)isLinear {
   return NO;
}

-(O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context {
   O2PDFDictionary *result=[O2PDFDictionary pdfDictionary];
   int              i;
   
   [result setIntegerForKey:"FunctionType" value:4];
   [result setObjectForKey:"Domain" value:[O2PDFArray pdfArrayWithNumbers:_domain count:_domainCount]];
   [result setObjectForKey:"Range" value:[O2PDFArray pdfArrayWithNumbers:_range count:_rangeCount]];
   // FIXME  stream
   
   return [context encodeIndirectPDFObject:result];
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   [result appendFormat:@"<%@ %p:",isa,self];
   [result appendFormat:@">"];
   
   return result;
}

@end
