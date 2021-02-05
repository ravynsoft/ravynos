/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSValue_concrete.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSData_concrete.h>
#include <string.h>

@implementation NSValue_concrete

NSValue *NSValue_concreteNew(NSZone *zone,const void *value,const char *type) {
   NSValue_concrete *self;
   NSUInteger         size,alignment,typelen=strlen(type);

   NSGetSizeAndAlignment(type,&size,&alignment);

   self=NSAllocateObject([NSValue_concrete class],typelen+1+size,zone);
    if (self) {
       NSByteCopy(type,self->_bytes,typelen+1);
       NSByteCopy(value,self->_bytes+typelen+1,size);
    }
   return self;
}

-(void)getValue:(void *)value {
   NSUInteger size,alignment,typelen=strlen(_bytes);

   NSGetSizeAndAlignment(_bytes,&size,&alignment);

   NSByteCopy(_bytes+typelen+1,value,size);
}

-(const char *)objCType {
   return _bytes;
}

@end
