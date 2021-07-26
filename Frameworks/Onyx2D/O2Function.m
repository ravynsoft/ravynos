/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Function.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <stddef.h>

@implementation O2Function

O2FunctionRef O2FunctionInit(O2FunctionRef self,void *info,size_t domainCount,const O2Float *domain,size_t rangeCount,const O2Float *range,const O2FunctionCallbacks *callbacks) {
   int i;
   
   self->_info=info;
   
   self->_domainCount=domainCount*2;
   self->_domain=NSZoneMalloc(NULL,sizeof(float)*self->_domainCount);
   if(domain==NULL){
    for(i=0;i<self->_domainCount;i++)
     self->_domain[i]=i%2;
   }
   else {
    for(i=0;i<self->_domainCount;i++)
     self->_domain[i]=domain[i];
   }
   
   self->_rangeCount=rangeCount*2;
   self->_range=NSZoneMalloc(NULL,sizeof(float)*self->_rangeCount);
   if(range==NULL){
    for(i=0;i<self->_rangeCount;i++)
     self->_range[i]=i%2;
   }
   else {
    for(i=0;i<self->_rangeCount;i++)
     self->_range[i]=range[i];
   }
   
   self->_callbacks=*callbacks;
   return self;
}

O2FunctionRef O2FunctionCreate(void *info,size_t domainDimension,const O2Float *domain,size_t rangeDimension,const O2Float *range,const O2FunctionCallbacks *callbacks) {   
   O2Function *result=[O2Function allocWithZone:NULL];
   
   return O2FunctionInit(result,info,domainDimension,domain,rangeDimension,range,callbacks);
}

O2FunctionRef O2FunctionRetain(O2FunctionRef self) {
   return (self!=NULL)?(O2FunctionRef)CFRetain(self):NULL;
}

void O2FunctionRelease(O2FunctionRef self) {
   if(self!=NULL)
    CFRelease(self);
}

void O2FunctionEvaluate(O2FunctionRef self,O2Float x,O2Float *output){
  float inputs[1];
  int   i;

  if(x<self->_domain[0])
   x=self->_domain[0];
  else if(x>self->_domain[1])
   x=self->_domain[1];

  inputs[0]=x; 
  self->_callbacks.evaluate(self->_info,inputs,output);
  
  for(i=0;i<self->_rangeCount/2;i++){
   if(output[i]<self->_range[i*2])
    output[i]=self->_range[i*2];
   else if(output[i]>self->_range[i*2+1])
    output[i]=self->_range[i*2+1];
  }
}

-(void)dealloc {
   if(_domain!=NULL)
    NSZoneFree(NULL,_domain);
   if(_range!=NULL)
    NSZoneFree(NULL,_range);
   if(_callbacks.releaseInfo!=NULL)
    _callbacks.releaseInfo(_info);
   [super dealloc];
}

-(unsigned)domainCount {
   return _domainCount;
}

-(const float *)domain {
   return _domain;
}

-(unsigned)rangeCount {
   return _rangeCount;
}

-(const float *)range {
   return _range;
}

-(BOOL)isLinear {
   return NO;
}

@end
