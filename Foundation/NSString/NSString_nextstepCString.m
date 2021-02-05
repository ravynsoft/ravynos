/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSString_nextstepCString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>
#include <stdio.h>

@implementation NSString_nextstepCString

-(const char *)cString {
   return _bytes;
}

-(const char *)lossyCString {
   return _bytes;
}

-(NSUInteger)cStringLength {
   return _length;
}

-(void)getCString:(char *)buffer maxLength:(NSUInteger)maxLength 
    range:(NSRange)range remainingRange:(NSRange *)leftoverRange {
   NSInteger i,rloc;

   if(NSMaxRange(range)>_length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

   for(i=0,rloc=range.location;i<range.length && i<maxLength;i++,rloc++)
    buffer[i]=_bytes[rloc];
   buffer[i]='\0';

   if(leftoverRange!=NULL){
    leftoverRange->location=rloc;
    leftoverRange->length=_length-rloc;
   }
}

NSString *NSNEXTSTEPCStringNewWithBytes(NSZone *zone,
 const char *bytes,NSUInteger length) {
   NSString_nextstepCString *self=NSAllocateObject(objc_lookUpClass("NSString_nextstepCString"),length*sizeof(char),zone);

    if (self) {
       self->_length=length;
        NSInteger i;
       for(i=0;i<length;i++)
        self->_bytes[i]=bytes[i];
       self->_bytes[i]='\0';
    }
   return self;
}

NSString *NSNEXTSTEPCStringNewWithCharacters(NSZone *zone,
 const unichar *characters,NSUInteger length,BOOL lossy) {
   NSString *string;
   NSUInteger  bytesLength;
   char     *bytes;

   bytes=NSUnicodeToNEXTSTEP(characters,length,lossy,&bytesLength,zone, NO);

   if(bytes==NULL)
    string=nil;
   else{
    string=NSNEXTSTEPCStringNewWithBytes(zone,bytes,bytesLength);
    NSZoneFree(zone,bytes);
   }

   return string;
}

NSString *NSNEXTSTEPCStringNewWithCapacity(NSZone *zone,
  NSUInteger capacity,char **ptr) {
   NSString_nextstepCString *self=NSAllocateObject(objc_lookUpClass("NSString_nextstepCString"),capacity*sizeof(char),zone);

    if (self) {
       self->_length=capacity;
       *ptr=self->_bytes;

       self->_bytes[capacity]='\0';
    }
   return self;
}

@end
