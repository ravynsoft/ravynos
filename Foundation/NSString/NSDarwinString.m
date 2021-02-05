/* Copyright (c) 2008 Johannes Fortmann
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSDarwinString.h"

#if __APPLE__
#import <Foundation/NSException.h>
#import <Foundation/NSStringHashing.h>
#import <Foundation/NSRaiseException.h>

// only needed for Darwin ppc
struct objc_class _NSConstantStringClassReference;

// only needed for Darwin i386/x86
#if __LP64__
int __CFConstantStringClassReference[24] = {0};
#else
int __CFConstantStringClassReference[12] = {0};
#endif

@implementation NSDarwinString

-copy {
   return self;
}

-copyWithZone:(NSZone *)zone {
   return self;
}

-retain {
   return self;
}


- (oneway void)release
{
}


-autorelease {
   return self;
}

-(void)dealloc {
   return;
   [super dealloc];
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

-(NSUInteger)hash {
   return NSStringHashASCII(_bytes,MIN(_length,NSHashStringLength));
}

@end

#endif
