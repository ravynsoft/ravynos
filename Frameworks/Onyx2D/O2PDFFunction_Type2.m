/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFFunction_Type2.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFContext.h>
#import <Foundation/NSString.h>
#import <stddef.h>
#import <math.h>

@implementation O2PDFFunction_Type2

static void evaluate(void *info,const float *input,float *output) {
   O2PDFFunction_Type2 *self=info;
   float x=input[0];
   int i;
   
   if(self->_N==1.0){
    for(i=0;i<self->_C0Count;i++){
     output[i]=self->_C0[i]+x*(self->_C1[i]-self->_C0[i]);
    }
   }
   else {
    for(i=0;i<self->_C0Count;i++){
     output[i]=self->_C0[i]+pow(x,self->_N)*(self->_C1[i]-self->_C0[i]);
    }
   }
}

-initWithDomain:(O2PDFArray *)domain range:(O2PDFArray *)range C0:(O2PDFArray *)C0 C1:(O2PDFArray *)C1 N:(O2PDFReal)N {
   if([super initWithDomain:domain range:range]==nil)
    return nil;
   
   _info=self;
   _callbacks.evaluate=evaluate;
   
   [C0 getNumbers:&_C0 count:&_C0Count];
   [C1 getNumbers:&_C1 count:&_C1Count];
   if(_C0Count!=_C1Count){
    NSLog(@"_C0Count(%d)!=_C1Count(%d)",_C0Count,_C1Count);
    [self dealloc];
    return nil;
   }
   
   _N=N;

   if(_rangeCount==0){
    int i;
    
    _rangeCount=_C0Count*2;
    if(_range!=NULL)
     NSZoneFree(NULL,_range);
     
    _range=NSZoneMalloc(NULL,sizeof(float)*_rangeCount);
    for(i=0;i<_rangeCount/2;i++){
     _range[i*2]=0;
     _range[i*2+1]=1;
    }
   }
   
   return self;
}

-(void)dealloc {
   if(_C0!=NULL)
    NSZoneFree(NULL,_C0);
   if(_C1!=NULL)
    NSZoneFree(NULL,_C1);
    
   [super dealloc];
}

-(BOOL)isLinear {
   return (_N==1.0)?YES:NO;
}

-(O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context {
   O2PDFDictionary *result=[O2PDFDictionary pdfDictionary];
   
   [result setIntegerForKey:"FunctionType" value:2];
   [result setObjectForKey:"Domain" value:[O2PDFArray pdfArrayWithNumbers:_domain count:_domainCount]];
   [result setObjectForKey:"Range" value:[O2PDFArray pdfArrayWithNumbers:_range count:_rangeCount]];
   [result setObjectForKey:"C0" value:[O2PDFArray pdfArrayWithNumbers:_C0 count:_C0Count]];
   [result setObjectForKey:"C1" value:[O2PDFArray pdfArrayWithNumbers:_C1 count:_C1Count]];
   [result setNumberForKey:"N" value:_N];
   
   return [context encodeIndirectPDFObject:result];
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   int i;
   
   [result appendFormat:@"<%@ %x:",isa,self];
   for(i=0;i<_C0Count;i++)
    [result appendFormat:@"C0[%d]=%f,",i,_C0[i]];
   for(i=0;i<_C1Count;i++)
    [result appendFormat:@"C1[%d]=%f,",i,_C1[i]];
   [result appendFormat:@"N=%f",_N];
   [result appendFormat:@">"];
   return result;
}


@end
