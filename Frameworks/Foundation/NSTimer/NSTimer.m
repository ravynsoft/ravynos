/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSTimer.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSTimer_targetAction.h>
#import <Foundation/NSTimer_invocation.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSRaise.h>

@implementation NSTimer : NSObject

+allocWithZone:(NSZone *)zone {
   if(self==objc_lookUpClass("NSTimer"))
    return NSAllocateObject([NSTimer_targetAction class],0,NULL);

   return NSAllocateObject(self,0,zone);
}

-initWithFireDate:(NSDate *)date interval:(NSTimeInterval)interval target:target selector:(SEL)selector userInfo:userInfo repeats:(BOOL)repeats {
   NSInvalidAbstractInvocation();
   return self;
}

+(NSTimer *)timerWithTimeInterval:(NSTimeInterval)timeInterval
  invocation:(NSInvocation *)invocation repeats:(BOOL)repeats {

   return NSAutorelease([NSAllocateObject([NSTimer_invocation class],0,NULL)
    initWithTimeInterval:timeInterval repeats:repeats
      invocation:invocation]);
}

+(NSTimer *)timerWithTimeInterval:(NSTimeInterval)timeInterval
  target:target selector:(SEL)selector
  userInfo:userInfo repeats:(BOOL)repeats {

   return NSAutorelease([
     NSAllocateObject([NSTimer_targetAction class],0,NULL)
       initWithTimeInterval:timeInterval repeats:repeats userInfo:userInfo
         target:target selector:selector]);
}

+(NSTimer *)scheduledTimerWithTimeInterval:(NSTimeInterval)timeInterval
  invocation:(NSInvocation *)invocation repeats:(BOOL)repeats {
   NSTimer *result=[self timerWithTimeInterval:timeInterval invocation:invocation repeats:repeats];

   [[NSRunLoop currentRunLoop] addTimer:result forMode:NSDefaultRunLoopMode];

   return result;
}

+(NSTimer *)scheduledTimerWithTimeInterval:(NSTimeInterval)timeInterval
  target:target selector:(SEL)selector
  userInfo:userInfo repeats:(BOOL)repeats {
   NSTimer *result=[self timerWithTimeInterval:timeInterval target:target selector:selector userInfo:userInfo repeats:repeats];

   [[NSRunLoop currentRunLoop] addTimer:result forMode:NSDefaultRunLoopMode];

   return result;
}

-(NSDate *)fireDate {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSTimeInterval)timeInterval {
   NSInvalidAbstractInvocation();
   return 0;
}

-userInfo {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)setFireDate:(NSDate *)date {
   NSInvalidAbstractInvocation();
}

-(void)fire {
   NSInvalidAbstractInvocation();
}

-(void)invalidate {
   NSInvalidAbstractInvocation();
}

-(BOOL)isValid {
   NSInvalidAbstractInvocation();
   return NO;
}

@end
