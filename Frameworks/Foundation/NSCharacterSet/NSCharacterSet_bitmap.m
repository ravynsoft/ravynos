/* Copyright (c) 2006-2009 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSCharacterSet_bitmap.h>
#import <Foundation/NSData.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSMutableCharacterSet_bitmap.h>

/*
  I had an implementation in mind long ago but didn't finish it and now forget what it
  was exactly {
    uint8_t _stageOne[256];
    uint8_t _stageTwo[8];
  }

-(BOOL)characterIsMember:(unichar)character {
   unsigned char indexOne=character>>8;
   unsigned      chunkTwo=_stageOne[indexOne];

   if(indexOne==0 || chunkTwo!=0){
    unsigned offset=chunkTwo*8+(character>>3)&0x0F;

    return (_stageTwo[offset]>>(character&0x0F))&0x01;
   }

   return NO;
}

 */

@implementation NSCharacterSet_bitmap

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

NSCharacterSet *NSCharacterSet_bitmapNewWithPath(NSZone *zone,NSString *path) {
   NSUnimplementedFunction();
   return nil;
}

NSCharacterSet *NSCharacterSet_bitmapNewWithBitmap(NSZone *zone,NSData *data) {
   return [[NSCharacterSet_bitmap allocWithZone:NULL] initWithData:data];
}

-(BOOL)characterIsMember:(unichar)character {
   return bitmapIsSet(_bitmap,character);
}

@end
