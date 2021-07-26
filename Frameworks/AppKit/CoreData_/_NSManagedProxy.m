/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "_NSManagedProxy.h"
#import <CoreData/NSEntityDescription.h>
#import <CoreData/NSManagedObjectContext.h>
#import <CoreData/NSManagedObject.h>
#import <CoreData/NSFetchRequest.h>

@implementation _NSManagedProxy_observerInfo
@synthesize indexSet;
@synthesize observer;
@synthesize keyPath;
@synthesize options;
@synthesize context;
@end


@implementation _NSManagedProxy

- initWithCoder: (NSCoder *) coder {

   if([coder allowsKeyedCoding]){
    _object = nil;
    _context = nil;
    _entity = nil;

    _entityName = [[coder decodeObjectForKey: @"NSEntityName"] retain];
    NSPredicate *fetchPredicate=[coder decodeObjectForKey: @"NSFetchPredicate"];
       
    _fetchRequest = [[NSFetchRequest alloc] init];
    [_fetchRequest setEntity: _entity];
    [_fetchRequest setPredicate: fetchPredicate];
   
    _observers = [[NSMutableArray alloc] init];
    return self;
   } else {
    [NSException raise:NSInvalidArgumentException format: @"%@ can not initWithCoder:%@", isa, [coder class]];
    return nil;
   }
}


-initWithParent: (_NSManagedProxy *) parent object: (NSManagedObject *) object {
   _object = [object retain];
   _entity = [parent entity];
   _entityName = [parent entityName];
   _context = [parent managedObjectContext];

   _fetchRequest = [[NSFetchRequest alloc] init];
   [_fetchRequest setEntity: _entity];
    
   _observers = [[NSMutableArray alloc] init];
   return self;
}


- (NSString *) description {
    return [NSString stringWithFormat: @"<proxy for %@>", _object];
}


- (id) representedObject {
    return _object;
}


- (NSString *) entityName {
    return _entityName;
}


- (NSEntityDescription *) entity {
    return _entity;
}


- (NSManagedObjectContext *) managedObjectContext {
    return _context;
}


- (void) setManagedObjectContext: (NSManagedObjectContext *) context {
    _context = context;
    _entity = [NSEntityDescription entityForName: _entityName
				   inManagedObjectContext: _context];

    [_fetchRequest setEntity: _entity];
}


- (id) valueForKey: (NSString *) key {
    if(_object)
	return [_object valueForKey: key];
    else
	return nil;
}


- (void) setValue: (id) value forKey: (NSString *) key {
    if([key isEqualToString: @"managedObjectContext"])
	[self setManagedObjectContext: value];
    else if(_object)
	[_object setValue: value forKey: key];
}


- (id) objectAtIndex: (NSUInteger) index {
    if(!_context || !_fetchRequest) return nil;
    id object = [[_fetchRequest _resultsInContext: _context] objectAtIndex: index];
    return [[_NSManagedProxy alloc] initWithParent: self object: object];
}


- (NSUInteger) count {
    NSUInteger result;
    if(!_context || !_fetchRequest) result = 0;
    else result = [_fetchRequest _countInContext: _context];
    NSLog(@"%@ being asked about its count and saying %lu; %@ %@\n", self, result,
	  _context, _fetchRequest);
    return result;
}


- (void) addObserver: (NSObject *) observer
  toObjectsAtIndexes: (NSIndexSet *) indexes
	  forKeyPath: (NSString *) keyPath
	     options: (NSKeyValueObservingOptions) options
	     context: (void *) context
{
    NSLog(@"Proxy for %@ asked to observe by %@ for keypath %@ options 0x%08x\n",
	  _object,
	  observer,
	  keyPath,
	  options);
    _NSManagedProxy_observerInfo *observerInfo
	= [[_NSManagedProxy_observerInfo alloc] init];
    [observerInfo setObserver: observer];
    [observerInfo setIndexSet: indexes];
    [observerInfo setKeyPath: keyPath];
    [observerInfo setOptions: options];
    [observerInfo setContext: context];
    [_observers addObject: observerInfo];
    [self notifyObserver: observerInfo];
}


- (void) removeObserver: (NSObject *) observer
   fromObjectsAtIndexes: (NSIndexSet *) indexes
	     forKeyPath: (NSString *) keyPath
{
}


- (void) notifyObserver: (_NSManagedProxy_observerInfo *) observerInfo {
    NSDictionary *change = [NSDictionary dictionary];
    [[observerInfo observer] observeValueForKeyPath: [observerInfo keyPath]
			     ofObject: self
			     change: change
			     context: [observerInfo context]];
    /* Unnecessary - a single notification will do it.
    for(NSUInteger index = [indexes firstIndex];
	index != NSNotFound;
	index = [indexes indexGreaterThanIndex: index]) {
	id object = [self objectAtIndex: index];
	id value = [object valueForKey: keyPath];
	
	NSDictionary *change = [NSDictionary dictionary];
	[observer observeValueForKeyPath: keyPath
		  ofObject: object
		  change: change
		  context: context];
    }
    */
}


- (void) _refresh {
    NSLog(@"%@ about to refresh", self);
    for(_NSManagedProxy_observerInfo *info in _observers)
	[self notifyObserver: info];
    NSLog(@"%@ refreshed", self);
}


- (id) objectValueForTableColumn: (NSTableColumn *) column {
    return [self valueForKey: @"name"];
}


@end
