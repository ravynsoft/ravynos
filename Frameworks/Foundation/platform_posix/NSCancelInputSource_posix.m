/* Copyright (c) 2009 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX

#import "NSCancelInputSource_posix.h"
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSSocket.h>
#import <Foundation/NSRunLoopState.h>


@implementation NSCancelInputSource_posix
-(id)init {
   _cancelWrite=[[NSSocket alloc] initConnectedToSocket:&_cancelRead];
   [_cancelRead retain];
   
   [self initWithSocket:_cancelRead];
   [self setSelectEventMask:NSSelectReadEvent];
   return self;
}

-(void)dealloc {
   [_cancelRead release];
   [_cancelWrite release];
   [super dealloc];
}

/*
-(NSUInteger)processImmediateEvents:(NSUInteger)selectEvent {
   if(selectEvent & NSSelectReadEvent) {
      uint8_t buf[256];
      [_cancelRead read:buf maxLength:256];
      _hasCanceled=NO;
      return NSSelectReadEvent;
   }
   return 0;
}*/

-(void)cancel {
   if(!_hasCanceled) {
      uint8_t buf[]="x";
      _hasCanceled=YES;
      [_cancelWrite write:buf maxLength:1];
   }
}

@end
#endif
