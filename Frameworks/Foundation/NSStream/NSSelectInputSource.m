/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRunLoop.h>

@implementation NSSelectInputSource

-initWithSocket:(NSSocket *)socket {
   _socket=[socket retain];
   _delegate=nil;
   _eventMask=0;
   _isValid=YES;
   return self;
}

-(void)dealloc {
   [_socket release];
   [super dealloc];
}

+(id)socketInputSourceWithSocket:(NSSocket *)socket {
   return [[[self alloc] initWithSocket:socket] autorelease];
}

-(NSSocket *)socket {
   return _socket;
}

-(BOOL)isValid {
   return _isValid;
}

-(void)invalidate {
   _isValid=NO;
   _delegate=nil;
}

-delegate {
   return _delegate;
}

-(void)setDelegate:object {
   _delegate=object;
}

-(NSUInteger)selectEventMask {
   return _eventMask;
}

-(void)setSelectEventMask:(NSUInteger)eventMask {
   _eventMask=eventMask;
}

-(NSUInteger)processImmediateEvents:(NSUInteger)selectEvent {

   if((selectEvent&=_eventMask)==0)
    return 0;

   [_delegate selectInputSource:self selectEvent:selectEvent];
   
   return selectEvent;
}

@end
