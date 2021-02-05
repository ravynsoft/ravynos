/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSDelayedPerform.h>
#import <Foundation/NSString.h>

@implementation NSDelayedPerform

-initWithObject:object selector:(SEL)selector argument:argument {
   _object=[object retain];
   _selector=selector;
   _argument=[argument retain];
   return self;
}

-(void)dealloc {
	[_object release];
	[_argument release];
	[super dealloc];
}

+(NSDelayedPerform *)delayedPerformWithObject:object selector:(SEL)selector argument:argument {
   return [[[self allocWithZone:NULL] initWithObject:object selector:selector argument:argument] autorelease];
}

-(BOOL)isEqualToPerform:(NSDelayedPerform *)other {
   if(_object!=other->_object)
    return NO;
   
   if(_selector==NULL || other->_selector==NULL)
    return YES;
    
   if(_selector!=other->_selector)
    return NO;
   if(_argument!=other->_argument)
    return NO;

   return YES;
}

-(void)perform {
	@try
	{
		[_object performSelector:_selector withObject:_argument];
	}
	@catch(id ex)
	{
		NSLog(@"exception %@ raised during delayed perform", ex);
	}
}

@end
