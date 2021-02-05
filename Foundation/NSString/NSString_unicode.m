/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString_unicode.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSRaiseException.h>
#include <string.h>

@implementation NSString_unicode

NSString *NSString_unicodeNew(NSZone *zone,
 const unichar *unicode,NSUInteger length) {
   NSString_unicode *self=NSAllocateObject([NSString_unicode class],length*sizeof(unichar),zone);

    if (self) {
        self->_length=length;
        NSInteger i;
       for(i=0;i<length;i++)
        self->_unicode[i]=unicode[i];
    }
   return self;
}

NSUInteger NSGetUnicodeCStringWithMaxLength(const unichar *characters,NSUInteger length,NSUInteger *location,char *cString,NSUInteger maxLength)
{
    if((length+1)*2 > maxLength) {
        cString[0]='\0';
        return NSNotFound;
    }
    
    NSUInteger ucByteLen = length*sizeof(unichar);
    memcpy(cString, characters, ucByteLen);
    *((unichar *)(cString + ucByteLen)) = 0;
    return ucByteLen;
}

char *NSUnicodeToUnicode(const unichar *characters,NSUInteger length,NSUInteger *resultLength, NSZone *zone,BOOL zeroTerminate) {
	unichar *unicode=NSZoneMalloc(zone,sizeof(unichar)*(length + (zeroTerminate ? 1 : 0)));

     memcpy(unicode, characters, length*sizeof(unichar));
	
    if(zeroTerminate) {
        unicode[length++]='\0';
    }
	*resultLength=length;
    
	return (char *)unicode;
}

-(NSUInteger)length {
   return _length;
}

-(unichar)characterAtIndex:(NSUInteger)location {
   if(location>=_length){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
     location,[self length]);
   }

   return _unicode[location];
}

-(void)getCharacters:(unichar *)buffer {
    memcpy(buffer, _unicode, _length*sizeof(unichar));
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSInteger i,len=range.length,loc=range.location;

   if(NSMaxRange(range)>_length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

    memcpy(buffer, _unicode+loc, len*sizeof(unichar));
}

-(NSUInteger)hash {
   return NSStringHashUnicode(_unicode,MIN(_length,NSHashStringLength));
}

@end
