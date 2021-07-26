/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSObservationProxy.h"
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSIndexSet.h>

void NSStringKVCSplitOnDot(NSString *self,NSString **before,NSString **after){
	NSRange range=[self rangeOfString:@"."];
	if(range.location!=NSNotFound)
	{
		*before=[self substringToIndex:range.location];
		*after=[self substringFromIndex:range.location+1];
	}
	else
	{
		*before=self;
		*after=nil;
	}
}

@implementation _NSObservationProxy 

-initWithKeyPath:(NSString *)keyPath observer:(id)observer object:(id)object {
   _keyPath=[keyPath retain];
   _observer=observer;
   _object=object;
	return self;
}

-(void)dealloc {
   [_keyPath release];
   [super dealloc];
}

-observer {
	return _observer;
}

-keyPath {
   return _keyPath;
}

-(void *)context {
   return _context;
}

-(NSKeyValueObservingOptions)options {
   return _options;
}

-(void)setNotifyObject:(BOOL)val
{
   _notifyObject=val;
}

- (BOOL)isEqual:(id)other
{
	if([other isMemberOfClass:isa])
	{
		_NSObservationProxy *o=other;
		if(o->_observer==_observer && [o->_keyPath isEqual:_keyPath] && [o->_object isEqual:_object])
			return YES;
	}
	return NO;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
   if(_notifyObject) {
    [_object observeValueForKeyPath:_keyPath ofObject:_object change:change context:_context];
   }

   [_observer observeValueForKeyPath:_keyPath ofObject:_object change:change context:_context];
}

-(NSString *)description {
	return [NSString stringWithFormat:@"observation proxy for %@ on key path %@", _observer, _keyPath];
}
@end

