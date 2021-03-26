/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFFunction_Type3.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFContext.h>
#import <Foundation/NSArray.h>
#import <stddef.h>

@implementation O2PDFFunction_Type3

static void evaluate(void *info,const float *input,float *output) {
   O2PDFFunction_Type3 *self=info;
   float                x=input[0];
   float                bounds[2],encode[2];
   int                  i;
      
   for(i=0;i<self->_boundsCount;i++){
    if(x<self->_bounds[i])
     break;
   }   

   bounds[0]=(i==0)?self->_domain[0]:self->_bounds[i-1];
   bounds[1]=(i==self->_boundsCount)?self->_domain[self->_domainCount-1]:self->_bounds[i];
   encode[0]=self->_encode[i*2];
   encode[1]=self->_encode[i*2+1];

   x-=bounds[0];
   x=(bounds[1]-bounds[0])/x;
   x=(encode[1]-encode[0])/x;
   x+=encode[0];

   O2FunctionEvaluate(self->_functions[i],x,output);
}

-initWithDomain:(O2PDFArray *)domain range:(O2PDFArray *)range functions:(NSArray *)functions bounds:(O2PDFArray *)bounds encode:(O2PDFArray *)encode {
   int i;
   
   if([super initWithDomain:domain range:range]==nil)
    return nil;

   _info=self;
   _callbacks.evaluate=evaluate;

   if((_functionCount=[functions count])==0){
    [self dealloc];
    return nil;
   }
   _functions=NSZoneMalloc(NULL,sizeof(O2Function *)*_functionCount);
   for(i=0;i<_functionCount;i++)
    _functions[i]=[[functions objectAtIndex:i] retain];
    
   if(![bounds getNumbers:&_bounds count:&_boundsCount]){
    [self dealloc];
    return nil;
   }
   if(![encode getNumbers:&_encode count:&_encodeCount]){
    [self dealloc];
    return nil;
   }
      
   if(_rangeCount==0){
    int i;
    
    for(i=0;i<_functionCount;i++){
    // All the functions _should_ have the same _rangeCount
     if([_functions[i] rangeCount]>_rangeCount)
      _rangeCount=[_functions[i] rangeCount];
    }
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
   int i;
   
   if(_functions!=NULL){
    for(i=0;i<_functionCount;i++)
     [_functions[i] release];
    NSZoneFree(NULL,_functions);
   }
   
   if(_bounds!=NULL)
    NSZoneFree(NULL,_bounds);
   if(_encode!=NULL)
    NSZoneFree(NULL,_encode);
   [super dealloc];
}

-(BOOL)isLinear {
   if(_functionCount==1)
    return [_functions[0] isLinear];

   return NO;
}

-(O2PDFObject *)encodeReferenceWithContext:(O2PDFContext *)context {
   O2PDFDictionary *result=[O2PDFDictionary pdfDictionary];
   int              i;
   
   [result setIntegerForKey:"FunctionType" value:3];
   [result setObjectForKey:"Domain" value:[O2PDFArray pdfArrayWithNumbers:_domain count:_domainCount]];
   [result setObjectForKey:"Range" value:[O2PDFArray pdfArrayWithNumbers:_range count:_rangeCount]];
   [result setObjectForKey:"Bounds" value:[O2PDFArray pdfArrayWithNumbers:_bounds count:_boundsCount]];
   [result setObjectForKey:"Encode" value:[O2PDFArray pdfArrayWithNumbers:_encode count:_encodeCount]];
   O2PDFArray *fnArray=[O2PDFArray pdfArray];
   for(i=0;i<_functionCount;i++)
    [fnArray addObject:[_functions[i] encodeReferenceWithContext:context]];
   [result setObjectForKey:"Functions" value:fnArray];
   
   return [context encodeIndirectPDFObject:result];
}

-(NSString *)description {
   NSMutableString *result=[NSMutableString string];
   [result appendFormat:@"<%@ %p:",isa,self];
   int i;
   
   for(i=0;i<_boundsCount;i++)
    [result appendFormat:@"bounds[%d]=%f,",i,_bounds[i]];
   for(i=0;i<_encodeCount;i++)
    [result appendFormat:@"encode[%d]=%f,",i,_encode[i]];
   for(i=0;i<_functionCount;i++)
    [result appendFormat:@"function[%d]=%@",i,_functions[i]];
   [result appendFormat:@">"];
   
   return result;
}

@end
