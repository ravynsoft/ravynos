/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSOutputStream_buffer.h>
#import <Foundation/NSError.h>

@implementation NSOutputStream_buffer

-initToBuffer:(uint8_t *)buffer capacity:(NSUInteger)capacity {
   _delegate=self;
   _error=nil;
   _status=NSStreamStatusNotOpen;
   _buffer=buffer;
   _capacity=capacity;
   _position=0;
   return self;
}

-(void)dealloc {
   [_error release];
   [super dealloc];
}

-delegate {
   return _delegate;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
   if(_delegate==nil)
    _delegate=self;
}

-(NSError *)streamError {
   return _error;
}

-(NSStreamStatus)streamStatus {
   return _status;
}

-(void)open {
   if(_status==NSStreamStatusNotOpen)
    _status=NSStreamStatusOpen;
}

-(void)close {
   _status=NSStreamStatusClosed;
}

-(BOOL)hasSpaceAvailable {
   if(_status!=NSStreamStatusOpen)
    return NO;
    
   return (_position<_capacity)?YES:NO;
}

-(NSInteger)write:(const uint8_t *)buffer maxLength:(NSUInteger)maxLength {
   if(_status!=NSStreamStatusOpen)
    return -1;
   else {
    int i;
    
    for(i=0;i<maxLength && _position<_capacity;i++)
     _buffer[_position++]=buffer[i];
    
    if(_position>=_capacity)
     _status=NSStreamStatusAtEnd;
     
    return i;
   }
}

@end
