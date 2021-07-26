/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSString_unicodePtr.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSRaiseException.h>
#import <Foundation/NSStringUTF8.h>

@implementation NSString_unicodePtr

NSString *NSString_unicodePtrNewNoCopy(NSZone *zone,const unichar *unicode,NSUInteger length,BOOL freeWhenDone) {
   NSString_unicodePtr *self=NSAllocateObject([NSString_unicodePtr class],0,zone);

    if (self) {
       self->_length=length;
       self->_freeWhenDone=freeWhenDone;
       self->_unicode=unicode;
    }
   return self;
}

NSString *NSString_unicodePtrNew(NSZone *zone,const unichar *unicode,NSUInteger length) {
   unichar *copy=NSZoneMalloc(NULL,length*sizeof(unichar));
   int      i;
   
   for(i=0;i<length;i++)
    copy[i]=unicode[i];
    
   return NSString_unicodePtrNewNoCopy(zone,copy,length,YES);
}

-(void)dealloc {
   if(_freeWhenDone)
   NSZoneFree(NSZoneFromPointer((void *)_unicode),(void *)_unicode);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(NSUInteger)length {
   return _length;
}

-(NSUInteger)lengthOfBytesUsingEncoding:(NSStringEncoding)encoding {
    switch (encoding) {
        case NSUTF8StringEncoding:
            return NSConvertUTF16toUTF8(_unicode, _length,NULL);
        case NSUnicodeStringEncoding:
            return _length;

        default:
            NSUnimplementedMethod();
            NSLog(@"For encoding: %i", encoding);
            return 0;
    }
}

-(unichar)characterAtIndex:(NSUInteger)location {
   if(location>=_length){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
     location,[self length]);
   }

   return _unicode[location];
}

-(void)getCharacters:(unichar *)buffer {
   int i;

   for(i=0;i<_length;i++)
    buffer[i]=_unicode[i];
}

-(void)getCharacters:(unichar *)buffer range:(NSRange)range {
   NSInteger i,len=range.length,loc=range.location;

   if(NSMaxRange(range)>_length){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }

   for(i=0;i<len;i++)
    buffer[i]=_unicode[loc+i];
}

-(NSUInteger)hash {
   return NSStringHashUnicode(_unicode,MIN(_length,NSHashStringLength));
}

@end
