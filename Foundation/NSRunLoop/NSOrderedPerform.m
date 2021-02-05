/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSOrderedPerform.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

@implementation NSOrderedPerform

-initWithSelector:(SEL)selector target:target argument:argument order:(NSUInteger)order modes:(NSArray *)modes {
   _selector=selector;
   _target=[target retain];
   _argument=[argument retain];
   _order=order;
   _modes=[modes copy];
   return self;
}

-(void)dealloc {
   [_modes release];
	[_target release];
	[_argument release];
   [super dealloc];
}

+(NSOrderedPerform *)orderedPerformWithSelector:(SEL)selector target:target argument:argument order:(NSUInteger)order modes:(NSArray *)modes {
   return [[[self alloc] initWithSelector:selector target:target argument:argument order:order modes:modes] autorelease];
}

-(SEL)selector {
   return _selector;
}

-(id)target {
   return _target;
}

-(id)argument {
   return _argument;
}

-(NSUInteger)order {
   return _order;
}

-(BOOL)fireInMode:(NSString *)mode {
   if([_modes containsObject:mode]){
	   @try
	   {
		   [_target performSelector:_selector withObject:_argument];
	   }
	   @catch(id ex)
	   {
		   NSLog(@"exception %@ raised during ordered perform", ex);		   
	   }
    return YES;
   }
   return NO;
}

@end
