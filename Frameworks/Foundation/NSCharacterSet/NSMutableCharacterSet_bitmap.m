/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSMutableCharacterSet_bitmap.h>
#import <Foundation/NSData.h>
#import <Foundation/NSException.h>

@implementation NSMutableCharacterSet_bitmap

-init {
   NSUInteger i;

   for(i=0;i<NSBitmapCharacterSetSize;i++)
    _bitmap[i]=0x00;

   return self;
}

-initWithCharacterSet:(NSCharacterSet *)set {
   NSUInteger i;

   for(i=0;i<NSBitmapCharacterSetSize;i++)
    _bitmap[i]=0x00;

   for(i=0;i<0xFFFF;i++)
    if([set characterIsMember:i])
     bitmapSet(_bitmap,i);

   return self;
}

-initWithData:(NSData *)data {
   const uint8_t *bytes=[data bytes];
   NSUInteger i;

   if([data length]!=NSBitmapCharacterSetSize)
    [NSException raise:@"NSCharacterSetFailedException"
                format:@"NSCharacterSet bitmap short %d in init",[data length]];

   for(i=0;i<NSBitmapCharacterSetSize;i++)
    _bitmap[i]=bytes[i];

   return self;
}

-initWithString:(NSString *)string {
   NSUInteger i,length=[string length];
   unichar    buffer[length];
   
   [string getCharacters:buffer];
   
   for(i=0;i<length;i++)
    bitmapSet(_bitmap,buffer[i]);
   
   return self;
}

-initWithRange:(NSRange)range {
   NSUInteger i;
   
   for(i=range.location;i<NSMaxRange(range);i++)
    bitmapSet(_bitmap,i);
   
   return self;
}

-(void)addCharactersInString:(NSString *)string {
   NSUInteger i,length=[string length];
   unichar  unicode[length];

   [string getCharacters:unicode];

   for(i=0;i<length;i++)
    bitmapSet(_bitmap,unicode[i]);
}

-(void)addCharactersInRange:(NSRange)range {
   NSUInteger i,limit=NSMaxRange(range);

   if(limit>0xFFFF)
    limit=0xFFFF+1;

   for(i=range.location;i<limit;i++)
    bitmapSet(_bitmap,i);
}

-(void)formUnionWithCharacterSet:(NSCharacterSet *)other {
   BOOL (*method)()=(void *)[other methodForSelector:@selector(characterIsMember:)];
   NSUInteger code;

   for(code=0;code<=0xFFFF;code++)
    if(method(other,@selector(characterIsMember:),(unichar)code))
     bitmapSet(_bitmap,code);
}

-(void)removeCharactersInString:(NSString *)string {
   NSUInteger i,length=[string length];
   unichar  unicode[length];

   [string getCharacters:unicode];

   for(i=0;i<length;i++)
    bitmapClear(_bitmap,unicode[i]);
}

-(void)removeCharactersInRange:(NSRange)range {
   NSUInteger i,limit=NSMaxRange(range);

   if(limit>0xFFFF)
    limit=0xFFFF+1;

   for(i=range.location;i<limit;i++)
    bitmapClear(_bitmap,i);
}

-(void)formIntersectionWithCharacterSet:(NSCharacterSet *)other {
   BOOL (*method)()=(void *)[other methodForSelector:@selector(characterIsMember:)];
   NSUInteger code;

   for(code=0;code<=0xFFFF;code++)
    if(!method(other,@selector(characterIsMember:),(unichar)code))
     bitmapClear(_bitmap,code);
}

-(void)invert {
   int i;

   for(i=0;i<NSBitmapCharacterSetSize;i++)
    _bitmap[i]=~_bitmap[i];
}

-(BOOL)characterIsMember:(unichar)character {
   return bitmapIsSet(_bitmap,character);
}

@end
