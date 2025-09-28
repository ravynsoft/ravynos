/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSControllerSelectionProxy.h"
#import "_NSControllerArray.h"
#import <AppKit/NSArrayController.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSKeyValueCoding.h>
#import <AppKit/NSObservationProxy.h>
#import <Foundation/NSException.h>

@interface  NSControllerSelectionProxy (FwdDecls)

- (void)_startObservingKey:(NSString*)key;
- (void)_startObservingSelectedObjects;
- (void)_stopObservingSelectedObjects;

@end

@implementation NSControllerSelectionProxy
-(id)initWithController:(id)cont
{
	if((self=[super init]))
	{
		_cachedValues=[NSMutableDictionary new];
		_controller = cont; // Don't retain that or we'll get a retain loop with the controller
      _observationProxies = [NSMutableArray new];
	}
	return self;
}

-(id)observableSelection {
   if(!_observableSelection)
      _observableSelection = [[_NSControllerArray alloc] initWithArray:[_controller selectedObjects]];
   return _observableSelection;
}

-(void)dealloc
{
	[self _stopObservingSelectedObjects];
	
   [_cachedKeysForKVO release];
	[_cachedValues release];
   [_observableSelection release];
   
   if([_observationProxies count]>0)
		[NSException raise:NSInvalidArgumentException
                  format:@"NSControllerSelectionProxy still being observed by %@ on %@",
       [[_observationProxies objectAtIndex:0] observer],
       [[_observationProxies objectAtIndex:0] keyPath]];
   
   [_observationProxies release];
	[super dealloc];
}

- (void)_startObservingKey:(NSString*)key
{
	// Stop observing all the current selectedObjects for the keys we're interested in
	NSArray *selectedObjects = [_controller selectedObjects];
	for (id obj in selectedObjects) {
		[obj addObserver: self forKeyPath: key options: NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew context: nil];
	}	
}

-(id)valueForKey:(NSString*)key
{
	// Don't use [NSNull null] as the nil marker because we want to be able to tell the diff between a real nil and a real [NSNull null] value
	static id nilMarker = nil;
	if (nilMarker == nil) {
		nilMarker = [[NSObject alloc] init];
	}
	
	id val=[_cachedValues objectForKey:key];
	if(val) {
		if (val == nilMarker) 
			val = nil;
		return val;
	}
	NSArray *selectedObjects = [_controller selectedObjects];
	NSMutableArray *allValues = nil;
	
	if ([key hasPrefix:@"@"]) { // operator
		val = [selectedObjects valueForKey:key];
	} else {
		// Get all selected objects property values, swizzling nil values with the nilMarker
		NSMutableArray *allValues = [NSMutableArray arrayWithCapacity:[selectedObjects count]];
		id en=[selectedObjects objectEnumerator];
		id obj;
		while((obj=[en nextObject]))
		{
			id val=[obj valueForKey:key];
			if(val==nil)
				val=nilMarker;
			[allValues addObject:val];
		}
		switch([allValues count])
		{
			case 0:
				val=NSNoSelectionMarker;
				break;
			case 1:
				val=[allValues lastObject];
				break;
			default:
			{
				if([_controller alwaysUsesMultipleValuesMarker])
				{
					val=NSMultipleValuesMarker;
				}
				else
				{
					val=[allValues objectAtIndex:0];
					id en=[allValues objectEnumerator];
					id obj;
					while((obj=[en nextObject]) && val!=NSMultipleValuesMarker)
					{
						if(![val isEqual:obj])
							val=NSMultipleValuesMarker;
					}
				}
				break;
			}
		}
	}
	
	[_cachedValues setValue:val forKey:key];
	
	if ([_cachedKeysForKVO containsObject: key] == NO) {
		// start observing this key - and add it to the cachedKeys array (this bit could probably be simplified)
		[self _startObservingKey: key];
		[_cachedKeysForKVO autorelease];
		_cachedKeysForKVO=[[_cachedValues allKeys] retain];
	}
	if (val == nilMarker) {
		val = nil;
	}
	return val;
}

-(int)count
{
	return [_cachedValues count];
}

-(id)keyEnumerator
{
	return [_cachedValues keyEnumerator];
}

-(void)setValue:(id)value forKey:(NSString *)key
{
	[[self observableSelection] setValue:value forKey:key];
}

-(NSString*)description
{
	return [NSString stringWithFormat:
		@"%@ <0x%x>",
		[self class],
		self];
}

- (void)_stopObservingSelectedObjects
{
	// Stop observing all the current selectedObjects for the keys we're interested in
	NSArray *selectedObjects = [_controller selectedObjects];
	for (id obj in selectedObjects) {
		for(id key in _cachedKeysForKVO) {
			[obj removeObserver: self forKeyPath: key];
		}
	}
}

- (void)_startObservingSelectedObjects
{
	// Start observing all the current selectedObjects for the keys we're interested in
	NSArray *selectedObjects = [_controller selectedObjects];
	for (id obj in selectedObjects) {
		for(id key in _cachedKeysForKVO) {
			[obj addObserver: self forKeyPath: key options: NSKeyValueObservingOptionOld | NSKeyValueObservingOptionNew context: nil];
		}
	}
}

-(void)controllerWillChange
{
	// While the controller is changing it can ask for our attention multiple times - but
	// asking for selected objects while the content/arranged objects are in flux is quite
	// risky - so we keep track of how many times the controller will change and only 
	// make a move on the first call of a will/did pair.
	_respondingToSelectionChanges++;
	if (_respondingToSelectionChanges > 1) {
		return;
	}
	
	[self _stopObservingSelectedObjects];
	
   [_cachedKeysForKVO autorelease];
   _cachedKeysForKVO=[[_cachedValues allKeys] retain];
	
  for(id key in _cachedKeysForKVO)
   {
      [self willChangeValueForKey:key];
   }
   [_cachedValues removeAllObjects];
   [_observableSelection release];
   _observableSelection=nil;
}

-(void)controllerDidChange
{
	// Reflecting the willChange logic - we only proceed if the will/did change tracker is reset to 0.
	_respondingToSelectionChanges--;
	if (_respondingToSelectionChanges > 0) {
		return;
	}
	
   [_cachedValues removeAllObjects];
   for(id key in _cachedKeysForKVO)
   {
      [self didChangeValueForKey:key];
   }

	[self _startObservingSelectedObjects];
	_respondingToSelectionChanges = NO;
}

- (void)observeValueForKeyPath:(NSString *)keyPath 
                      ofObject:(id)object 
                        change:(NSDictionary *)change
                       context:(void *)context
{
   // remove cached value for this key path
   [_cachedValues removeObjectForKey:keyPath];
	
	// Pass the change on up to those observing the proxy
	if([[change objectForKey:NSKeyValueChangeNotificationIsPriorKey] boolValue] == YES) {
		[self willChangeValueForKey: keyPath];
	} else {
		[self didChangeValueForKey: keyPath];
	}
}

- (void)addObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(void *)context
{
   _NSObservationProxy *proxy=[[_NSObservationProxy alloc] initWithKeyPath:keyPath observer:observer object:self];
   [proxy setNotifyObject:YES];
   [_observationProxies addObject:proxy];
   
   [[self observableSelection] addObserver:proxy forKeyPath:keyPath options:options context:context];

   [proxy release];
}

- (void)removeObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath
{
   _NSObservationProxy *proxy=[[_NSObservationProxy alloc] initWithKeyPath:keyPath observer:observer object:self];
   int idx=[_observationProxies indexOfObject:proxy];
   if(idx==NSNotFound) {
   }

   [proxy release];
   
   if(idx!=NSNotFound) {
      [[self observableSelection] removeObserver:[_observationProxies objectAtIndex:idx] forKeyPath:keyPath];
      [_observationProxies removeObjectAtIndex:idx];
   }
   
}

@end
