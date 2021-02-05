/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString_isoLatin1.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

unichar *NSISOLatin1ToUnicode(const char *cString,NSUInteger length,
  NSUInteger *resultLength,NSZone *zone) {
   unichar *characters=NSZoneMalloc(zone,sizeof(unichar)*length);
   int      i;

   for(i=0;i<length;i++)
    characters[i]=((uint8_t *)cString)[i];

   *resultLength=i;
   return characters;
}

char *NSUnicodeToISOLatin1(const unichar *characters,NSUInteger length,
  BOOL lossy,NSUInteger *resultLength,NSZone *zone,BOOL zeroTerminate) {
   char *isolatin1=NSZoneMalloc(zone,sizeof(char)*(length + (zeroTerminate == YES ? 1 : 0)));
   int   i;

   for(i=0;i<length;i++){

    if(characters[i]<256)
     isolatin1[i]=characters[i];
    else if(lossy)
     isolatin1[i]='\0';
    else {
     NSZoneFree(zone,isolatin1);
     return NULL;
    }
   }
   if(zeroTerminate == YES) {
        isolatin1[i++]='\0';
   }
   *resultLength=i;

   return isolatin1;
}

NSUInteger NSGetISOLatin1CStringWithMaxLength(const unichar *characters,NSUInteger length,
                                              NSUInteger *location,char *cString,NSUInteger maxLength,BOOL lossy)
{
    NSUInteger i,result=0;


    if(length+1 > maxLength) {
        cString[0]='\0';
        return NSNotFound;
    }
    for(i=0;i<length && result<=maxLength;i++){
        const unichar code=characters[i];

        if(code<256)
            cString[result++]=code;
        else {
            if(lossy)
                cString[result++]='\0';
            else {
                return NSNotFound;
            }
        }
    }

    cString[result]='\0';

    *location=i;

    return result;

}

NSString *NSISOLatin1CStringNewWithCharacters(NSZone *zone,
                                               const unichar *characters,NSUInteger length,BOOL lossy) {
    NSString    *string;
    NSUInteger  bytesLength;
    char        *bytes;

    bytes=NSUnicodeToISOLatin1(characters,length,lossy,&bytesLength,zone, NO);

    if(bytes==NULL)
        string=nil;
    else{
        string=NSString_isoLatin1NewWithBytes(zone,bytes,bytesLength);
        NSZoneFree(zone,bytes);
    }

    return string;
}


@implementation NSString_isoLatin1

NSString *NSString_isoLatin1NewWithBytes(NSZone *zone,
 const char *bytes,NSUInteger length) {
    
   NSString_isoLatin1 *self=NSAllocateObject([NSString_isoLatin1 class],length*sizeof(char),zone);
    if (self) {
       self->_length=length;
        int i;
       for(i=0;i<length;i++)
        self->_bytes[i]=((uint8_t *)bytes)[i];
       self->_bytes[i]='\0';
    }
   return self;
}

-(NSUInteger)length {
   return _length;
}

-(unichar)characterAtIndex:(NSUInteger)location {
   if(location>=_length){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
     location,[self length]);
   }

   return _bytes[location];
}

-(void)getCharacters:(unichar *)buffer {
   int i;

   for(i=0;i<_length;i++)
    buffer[i]=_bytes[i];
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSInteger i,loc=range.location,len=range.length;

   if(NSMaxRange(range)>_length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

   for(i=0;i<len;i++)
    buffer[i]=_bytes[loc+i];
}

@end
