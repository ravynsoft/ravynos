/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSTimer_concrete.h>
#import <Foundation/NSString.h>

@implementation NSTimer_concrete

-initWithFireDate:(NSDate *)date interval:(NSTimeInterval)interval repeats:(BOOL)repeats {
   _timeInterval=interval;
   _fireDate=[date copy];

   _isValid=YES;
   _repeats=repeats;

   return self;
}

-initWithTimeInterval:(NSTimeInterval)interval repeats:(BOOL)repeats {

   _timeInterval=interval;
   _fireDate=[[[NSDate date] addTimeInterval:_timeInterval] retain];

   _isValid=YES;
   _repeats=repeats;

   return self;
}


-(void)dealloc {
   [_fireDate release];
   NSDeallocateObject(self);
   return;
   [super dealloc];
}


-(void)fire {
   if(!_repeats)
    [self invalidate];
   else if(_isValid) {
    NSDate *lastFire=_fireDate;
    // We add to the current time to get the next time, if we add to the fire date
    // and the timer is a long running task, the timer is always behind and always
    // catching up
    _fireDate=[[[NSDate date] addTimeInterval:_timeInterval] retain];
    [lastFire release];
   }
}


-(NSDate *)fireDate {
   return _fireDate;
}

-(NSTimeInterval)timeInterval {
   return _timeInterval;
}


-userInfo {
   return nil;
}

-(void)setFireDate:(NSDate *)date {
   date=[date copy];
   [_fireDate release];
   _fireDate=date;
}

-(BOOL)isValid {
   return _isValid;
}

@end

