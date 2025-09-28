/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSOutputStream.h>
#import <Foundation/NSOutputStream_buffer.h>
#import <Foundation/NSOutputStream_data.h>
#import <Foundation/NSOutputStream_file.h>
#import <Foundation/NSRaise.h>

@implementation NSOutputStream

-initToBuffer:(uint8_t *)buffer capacity:(NSUInteger)capacity {
   [self dealloc];
   return [[NSOutputStream_buffer alloc] initToBuffer:buffer capacity:capacity];
}

-initToFileAtPath:(NSString *)path append:(BOOL)append {
   [self dealloc];
   return [[NSOutputStream_file alloc] initToFileAtPath:path append:append];
}

-initToMemory {
   [self dealloc];
   return [[NSOutputStream_data alloc] initToMemory];
}

+outputStreamToBuffer:(uint8_t *)buffer capacity:(NSUInteger)capacity {
   return [[[self alloc] initToBuffer:buffer capacity:capacity] autorelease];
}

+outputStreamToFileAtPath:(NSString *)path append:(BOOL)append {
   return [[[self alloc] initToFileAtPath:path append:append] autorelease];
}

+outputStreamToMemory {
   return [[[self alloc] initToMemory] autorelease];
}

-(BOOL)hasSpaceAvailable {
   NSInvalidAbstractInvocation();
   return NO;
}

-(NSInteger)write:(const uint8_t *)buffer maxLength:(NSUInteger)length {
   NSInvalidAbstractInvocation();
   return 0;
}


@end
