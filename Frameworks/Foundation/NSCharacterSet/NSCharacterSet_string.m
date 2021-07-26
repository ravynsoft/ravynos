/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSCharacterSet_string.h>

@implementation NSCharacterSet_string

-initWithCharacters:(unichar *)characters length:(NSUInteger)length inverted:(BOOL)inverted {
   NSUInteger i;

   _length=length;
   _buffer=NSZoneMalloc([self zone],_length*sizeof(unichar));
   for(i=0;i<_length;i++)
    _buffer[i]=characters[i];
   _inverted=inverted;
   return self;
}

-initWithString:(NSString *)string inverted:(BOOL)inverted {
   _length=[string length];
   _buffer=NSZoneMalloc([self zone],_length*sizeof(unichar));
   [string getCharacters:_buffer];
   _inverted=inverted;
   return self;
}

-(void)dealloc {
   NSZoneFree([self zone],_buffer);
   [super dealloc];
}

-(BOOL)characterIsMember:(unichar)character {
   BOOL     result=_inverted?NO:YES;
   NSUInteger i;

   for(i=0;i<_length;i++)
    if(_buffer[i]==character)
     return result;

   return !result;
}

-(NSCharacterSet *)invertedSet {
   return [[[NSCharacterSet_string alloc] initWithCharacters:_buffer length:_length inverted:!_inverted] autorelease];
}

@end
