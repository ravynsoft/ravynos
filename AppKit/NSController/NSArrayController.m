/* Copyright (c) 2007-2008 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSArrayController.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSException.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSPredicate.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSSortDescriptor.h>
#import <AppKit/NSRaise.h>
#import "NSObservationProxy.h"
#import "_NSControllerArray.h"

/*
 * Implementation Notes
 *
 * _arrangedObjects is a cache of a particular arrangement of the content as established by invoking arrangeObjects: with the content array
 *
 * Unless the controller is configured to automatically rearrange its objects when there are changes to the content, it's up to the implementation
 * to keep the _arrangedObjects cache and the _content in sync. This means that objects need to be added and removed from both.
 *
 * Selection Indexes reference into the arrangedObjects array.
 */

@interface NSObjectController(private)
-(id)_defaultNewObject;
-(void)_selectionWillChange;
-(void)_selectionDidChange;
@end

@interface NSArrayController(forwardRefs)
- (void)_setArrangedObjects:(id)value;
- (void)_setContentArray:(id)value;
- (id)_contentArray;
@end

@interface NSArray (private)
-(NSUInteger)_insertObject:(id)obj inArraySortedByDescriptors:(id)desc;
@end

@implementation NSArrayController

#pragma mark -
#pragma mark Initialization and Tear Down

+(void)initialize
{
   [self setKeys:[NSArray arrayWithObjects:@"content", nil] triggerChangeNotificationsForDependentKey:@"contentArray"];
   [self setKeys:[NSArray arrayWithObjects:@"content", @"contentArray", @"selectionIndexes", nil] triggerChangeNotificationsForDependentKey:@"selection"];
   [self setKeys:[NSArray arrayWithObjects:@"content", @"contentArray", @"selectionIndexes", @"selection", nil]triggerChangeNotificationsForDependentKey:@"selectionIndex"];
   [self setKeys:[NSArray arrayWithObjects:@"content", @"contentArray", @"selectionIndexes", @"selection", nil] triggerChangeNotificationsForDependentKey:@"selectedObjects"];
	
   [self setKeys:[NSArray arrayWithObjects:@"selectionIndexes", nil] triggerChangeNotificationsForDependentKey:@"canRemove"];
   [self setKeys:[NSArray arrayWithObjects:@"selectionIndexes", nil] triggerChangeNotificationsForDependentKey:@"canSelectNext"];
   [self setKeys:[NSArray arrayWithObjects:@"selectionIndexes", nil] triggerChangeNotificationsForDependentKey:@"canSelectPrevious"];
}

-(id)initWithCoder:(NSCoder*)coder
{
	if((self = [super initWithCoder:coder])) {
		_flags.avoidsEmptySelection = [coder decodeBoolForKey:@"NSAvoidsEmptySelection"];
		_flags.clearsFilterPredicateOnInsertion = [coder decodeBoolForKey:@"NSClearsFilterPredicateOnInsertion"];
		_flags.filterRestrictsInsertion = [coder decodeBoolForKey:@"NSFilterRestrictsInsertion"];
		_flags.preservesSelection = [coder decodeBoolForKey:@"NSPreservesSelection"];
		_flags.selectsInsertedObjects = [coder decodeBoolForKey:@"NSSelectsInsertedObjects"];
		_flags.alwaysUsesMultipleValuesMarker = [coder decodeBoolForKey:@"NSAlwaysUsesMultipleValuesMarker"];

		id declaredKeys=[coder decodeObjectForKey:@"NSDeclaredKeys"];
		
		if([self automaticallyPreparesContent]) {
			[self prepareContent];
		} else {
			[self _setContentArray:[NSMutableArray array]];
		}

	}
	return self;
}

- (id)initWithContent:(id)content
{
	if ((self = [super init])) {
		_flags.avoidsEmptySelection = YES;
		_flags.clearsFilterPredicateOnInsertion = YES;
		_flags.filterRestrictsInsertion = NO;
		_flags.preservesSelection = YES;
		_flags.selectsInsertedObjects = YES;
		_flags.alwaysUsesMultipleValuesMarker = NO;
		
		[self setAutomaticallyPreparesContent:NO];
		[self _setContentArray:content];
	}
	return self;
}

-(void)dealloc
{
	// Autorelease things, don't release them - [super dealloc] is doing 
	// some cleaning that need these things to be still alive
	[_selectionIndexes autorelease];
	[_sortDescriptors autorelease];
	[_filterPredicate autorelease];
	[_arrangedObjects autorelease];
	[super dealloc];
}

-(void)awakeFromNib
{
   [self _selectionWillChange];
   [self _selectionDidChange];
}

#pragma mark -
#pragma mark Managing Sort Descriptors

- (void)setSortDescriptors:(NSArray *)value
{
	[_sortDescriptors release];
	_sortDescriptors=[value copy];
	
	if ([self automaticallyRearrangesObjects]) {
		[self rearrangeObjects];
	}
}

- (NSArray *)sortDescriptors
{
	return [[_sortDescriptors retain] autorelease];
}

#pragma mark -
#pragma mark Arranging Objects

- (void)_setArrangedObjects:(id)value
{
	[_arrangedObjects autorelease];
	_arrangedObjects = [[_NSControllerArray alloc] initWithArray:value];
}

-(NSArray*)arrangeObjects:(NSArray*)objects
{
	id sortedObjects=objects;
    
	if([self filterPredicate]) {
		sortedObjects=[sortedObjects filteredArrayUsingPredicate:[self filterPredicate]];
	}
	
	if([self sortDescriptors]) {
		sortedObjects=[sortedObjects sortedArrayUsingDescriptors:[self sortDescriptors]];
	}
	
	return sortedObjects;
}

-(id)arrangedObjects
{
	return [[_arrangedObjects retain] autorelease];
}

-(void)rearrangeObjects
{
	[self _selectionWillChange];
	
	// We may need to restore selection
	NSArray* selectedObjects = [self selectedObjects];

	// Arrange the content
	NSArray* arrangedObjects = [self arrangeObjects:[self _contentArray]];

	[self willChangeValueForKey: @"arrangedObjects"];

	// Cache the new arrangement
	[self _setArrangedObjects: arrangedObjects];

	[self didChangeValueForKey: @"arrangedObjects"];

	if ([self preservesSelection]) {
		// restore the selection
		[self setSelectedObjects: selectedObjects];
	}
	[self _selectionDidChange];
}

#pragma mark -
#pragma mark Managing Content

-(void)setContent:(id)value
{
	// Convert the value, if necessary, into an array so it's compatible with the array controller
	if(value!=nil && ![value isKindOfClass:[NSArray class]]) {
		value=[NSArray arrayWithObject:value];
    }
	
	[super setContent:[[value mutableCopy] autorelease]];
	
	if(_flags.clearsFilterPredicateOnInsertion) {
		[self setFilterPredicate:nil];
	}
	
	// Rearrange objects will preserve selection if needed
	[self rearrangeObjects];
	
}

- (void)_setContentArrayForMultipleSelection:(id)value 
{
	NSUnimplementedMethod();
}

- (void)_setContentArray:(id)value 
{
	[self setContent:value];
}

- (id)_contentArray
{
    id result=[self content];
    return result;
}

-(id)_contentSet
{
	return [NSSet setWithArray:_content];
}

-(void)_setContentSet:(NSSet *)set
{
	[self setContent:[set allObjects]];
}

-(void)setAutomaticallyPreparesContent:(BOOL)flag
{
	_flags.automaticallyPreparesContent = flag;
}

-(BOOL)automaticallyPreparesContent
{
	return _flags.automaticallyPreparesContent;
}

-(void)prepareContent
{
	id array=[NSMutableArray array];
	[array addObject:[[self newObject] autorelease]];
	[self _setContentArray:array];
}

#pragma mark -
#pragma mark Selection Attributes

-(void)setAvoidsEmptySelection:(BOOL)value
{
	_flags.avoidsEmptySelection=value;
}

- (BOOL)avoidsEmptySelection
{
	return _flags.avoidsEmptySelection;
}

-(void)setPreservesSelection:(BOOL)value
{
	_flags.preservesSelection=value;
}

- (BOOL)preservesSelection
{
	return _flags.preservesSelection;
}

- (void)setAlwaysUsesMultipleValuesMarker:(BOOL)flag
{
	_flags.alwaysUsesMultipleValuesMarker = flag;
}

-(BOOL)alwaysUsesMultipleValuesMarker
{
	return _flags.alwaysUsesMultipleValuesMarker;
}

#pragma mark -
#pragma mark Managing Selections

-(NSUInteger)selectionIndex
{
	if (_selectionIndexes == nil) {
		return NSNotFound;
	} else {
		return [_selectionIndexes firstIndex];
	}
}

-(BOOL)setSelectionIndex:(unsigned)index
{
	return [self setSelectionIndexes:[NSIndexSet indexSetWithIndex:index]];
}

- (void)setSelectsInsertedObjects:(BOOL)flag
{
	_flags.selectsInsertedObjects = flag;
}

- (BOOL)selectsInsertedObjects
{
	return _flags.selectsInsertedObjects;
}

- (BOOL)setSelectionIndexes:(NSIndexSet *)value
{
	
	if(_flags.avoidsEmptySelection && [value count]==0 && [[self arrangedObjects] count]) {
		value=[NSIndexSet indexSetWithIndex:0];
	}
	
	if ([_selectionIndexes isEqualToIndexSet: value]) {
		// The selection isn't going to change
		return NO;
	}
	
	[self willChangeValueForKey:@"selectionIndexes"];
	[self _selectionWillChange];
	
	[_selectionIndexes release];
	_selectionIndexes = [value mutableCopy];
	[self _selectionDidChange];
	
	[self didChangeValueForKey:@"selectionIndexes"];
	return YES;
}

- (NSIndexSet *)selectionIndexes
{
    return [[_selectionIndexes retain] autorelease];
}

- (BOOL)addSelectionIndexes:(NSIndexSet *)indices
{
	if ([_selectionIndexes containsIndexes: indices] == YES) {
		// they already selected so the selection won't change
		return NO;
	}
	
	[self willChangeValueForKey: @"selectionIndexes"];
	[self _selectionWillChange];

	[_selectionIndexes addIndexes: indices];

	[self _selectionDidChange];
	[self didChangeValueForKey: @"selectionIndexes"];

	return YES;
}

- (BOOL)removeSelectionIndexes:(NSIndexSet *)indices
{	
	if ([_selectionIndexes containsIndexes: indices] == NO) {
		// they already deselected so the selection won't change
		return NO;
	}
	
	[self willChangeValueForKey: @"selectionIndexes"];
	[self _selectionWillChange];

	[_selectionIndexes removeIndexes: indices];

	[self _selectionDidChange];
	[self didChangeValueForKey: @"selectionIndexes"];
	return YES;
}

- (NSMutableIndexSet*)_indexesOfArrangedObjects:(NSArray*)objects
{
	id set=[NSMutableIndexSet indexSet];
	int i, count=[objects count];
	for(i=0; i<[objects count]; i++) {
		unsigned idx=[[self arrangedObjects] indexOfObject:[objects objectAtIndex:i]];
		if(idx!=NSNotFound) {
			[set addIndex:idx];
		}
	}
	return set;
}

- (BOOL)setSelectedObjects:(NSArray *)objects
{
	NSMutableIndexSet* set = [self _indexesOfArrangedObjects: objects];
	BOOL selectionChanged = [self setSelectionIndexes:set];
	return selectionChanged;
}

-(NSArray *)selectedObjects
{
	id idxs=[self selectionIndexes];
	if(idxs) {
		return [[self arrangedObjects] objectsAtIndexes:idxs];
	}
	return [NSArray array];
}

- (BOOL)addSelectedObjects:(NSArray *)objects
{
	NSMutableIndexSet* set = [self _indexesOfArrangedObjects: objects];
	BOOL selectionChanged = [self addSelectionIndexes: set];
	return selectionChanged;
}

- (BOOL)removeSelectedObjects:(NSArray *)objects
{
	NSMutableIndexSet* set = [self _indexesOfArrangedObjects: objects];
	BOOL selectionChanged = [self removeSelectionIndexes: set];
	return selectionChanged;
}

-(void)selectNext:(id)sender
{
	id idxs=[[[self selectionIndexes] mutableCopy] autorelease];
	if(!idxs) {
		[self setSelectionIndexes:[NSIndexSet indexSetWithIndex:0]];
        return;
    }
	[idxs shiftIndexesStartingAtIndex:0 by:1];
	
	if([idxs lastIndex]<[[self arrangedObjects] count]) {
		[self setSelectionIndexes:idxs];
	}
}

-(BOOL)canSelectNext
{
	id idxs=[[[self selectionIndexes] mutableCopy] autorelease];
	
	if(idxs && [idxs lastIndex]<[[self arrangedObjects] count]-1) {
		return YES;
	}
	
	return NO;
}

-(void)selectPrevious:(id)sender {
	id idxs=[[[self selectionIndexes] mutableCopy] autorelease];
	if(!idxs) {
		[self setSelectionIndexes:[NSIndexSet indexSetWithIndex:0]];
        return;
	}
	if([idxs firstIndex]>0) {
		[idxs shiftIndexesStartingAtIndex:0 by:-1];
		
		[self setSelectionIndexes:idxs];
	}
}

-(BOOL)canSelectPrevious
{
	id idxs=[[[self selectionIndexes] mutableCopy] autorelease];
	
	if(idxs && [idxs firstIndex]>0) {
		return YES;
	}
	return NO;
}

#pragma mark -
#pragma mark Inserting

-(BOOL)canInsert
{
	return [self isEditable];
}

-(void)insert:(id)sender
{
	if(![self canInsert]) {
		return;
	}
	
	id toAdd=nil;
	if([self automaticallyPreparesContent]) {
		toAdd=[[self newObject] autorelease];
	} else {
		toAdd=[[self _defaultNewObject] autorelease];
	}
	
	[self addObject:toAdd];
}

#pragma mark -
#pragma mark Adding and Removing Objects

-(void)add:(id)sender {
	
	if(![self canAdd]) {
		return;
	}
	
	[self insert:sender];
}

- (void)addObject:(id)object {
	// Don't check canAdd here as this can be used programmatically to add objects
	[self insertObject: object atArrangedObjectIndex: [[self arrangedObjects] count]];
}

- (void)addObjects:(NSArray *)objects
{
	for (id object in objects) {
		[self addObject: object];
	}
}

-(void)insertObject:object atArrangedObjectIndex:(unsigned)index
{
	if(_flags.clearsFilterPredicateOnInsertion) {
		[self setFilterPredicate:nil];
	}
	
	[self willChangeValueForKey: @"content"];
	[self willChangeValueForKey: @"arrangedObjects"];
	[_content addObject:object];
	[_arrangedObjects insertObject: object atIndex: index];
	[self didChangeValueForKey: @"content"];
	[self didChangeValueForKey: @"arrangedObjects"];

	if (_flags.selectsInsertedObjects) {
		[self addSelectedObjects: [NSArray arrayWithObject: object]];
	}

	if ([self automaticallyRearrangesObjects]) {
		[self rearrangeObjects];
	}
	
}

-(void)insertObjects:(NSArray *)objects atArrangedObjectIndexes:(NSIndexSet *)indices
{
	if(_flags.clearsFilterPredicateOnInsertion) {
		[self setFilterPredicate:nil];
	}
	
	// Batch up the changes to avoid over observing
	[self willChangeValueForKey: @"content"];
	[self willChangeValueForKey: @"arrangedObjects"];

	int i = 0;
	NSUInteger index = [indices firstIndex];
	while (index != NSNotFound) {
		id object = [objects objectAtIndex: i++];
		[_content addObject:object];
		[_arrangedObjects insertObject: object atIndex: index];
		index = [indices indexGreaterThanIndex: index];
	}
	
	[self didChangeValueForKey: @"content"];
	[self didChangeValueForKey: @"arrangedObjects"];

	if (_flags.selectsInsertedObjects) {
		[self addSelectedObjects: objects];
	}
	
	if ([self automaticallyRearrangesObjects]) {
		[self rearrangeObjects];
	}
}

-(void)removeObjectAtArrangedObjectIndex:(unsigned)index
{
	[self removeSelectionIndexes: [NSIndexSet indexSetWithIndex: index]];
	
	id object = [_arrangedObjects objectAtIndex: index];
	[self willChangeValueForKey: @"content"];
	[self willChangeValueForKey: @"arrangedObjects"];
	[_arrangedObjects removeObjectAtIndex: index];
	[_content removeObject: object];
	[self didChangeValueForKey: @"content"];
	[self didChangeValueForKey: @"arrangedObjects"];

	if ([self automaticallyRearrangesObjects]) {
		[self rearrangeObjects];
	}	
}

-(void)removeObjectsAtArrangedObjectIndexes:(NSIndexSet*)indexes
{	
	[self removeSelectionIndexes: indexes];

	// Batch up the changes to avoid over observing
	[self willChangeValueForKey: @"content"];
	[self willChangeValueForKey: @"arrangedObjects"];

	// Work in reverse order to avoid messing up the array
	NSUInteger index = [indexes lastIndex];
	while (index != NSNotFound) {
		id object = [_arrangedObjects objectAtIndex: index];
		[_arrangedObjects removeObjectAtIndex: index];
		[_content removeObject: object];
		index = [indexes indexLessThanIndex: index];
	}

	[self didChangeValueForKey: @"content"];
	[self didChangeValueForKey: @"arrangedObjects"];

	if ([self automaticallyRearrangesObjects]) {
		[self rearrangeObjects];
	}
}

-(void)remove:(id)sender
{
	if(![self canRemove]) {
		return;
    }
	
	[self removeObjectsAtArrangedObjectIndexes: [self selectionIndexes]];
}

-(void)removeObject:(id)object
{
	// Don't check canremove/editable here as this can be used programmatically to remove objects
	
	NSUInteger index=[_arrangedObjects indexOfObject:object];
	[self removeObjectAtArrangedObjectIndex: index];
}

- (void)removeObjects:(NSArray *)objects {
	// Don't check canRemove here as this can be used programmatically to remove objects
	
	NSMutableIndexSet* indexes = [NSMutableIndexSet indexSet];

	for (id object in objects) {
		NSUInteger index = [_arrangedObjects indexOfObject: object];
		[indexes addIndex: index];
	}
	[self removeObjectsAtArrangedObjectIndexes: indexes];
}

#pragma mark -
#pragma mark Filtering Content

-(void)setClearsFilterPredicateOnInsertion:(BOOL)flag
{
	_flags.clearsFilterPredicateOnInsertion = flag;
}

-(BOOL)clearsFilterPredicateOnInsertion
{
	return _flags.clearsFilterPredicateOnInsertion;
}

- (NSPredicate *)filterPredicate
{
   return [[_filterPredicate retain] autorelease];
}

- (void)setFilterPredicate:(NSPredicate *)value
{
	value=[value copy];
	[_filterPredicate release];
	_filterPredicate = value;
	[self rearrangeObjects];
}

#pragma mark -
#pragma mark Automatic Content Rearranging

- (void)setAutomaticallyRearrangesObjects:(BOOL)flag
{
	_flags.automaticallyRearrangesObjects = flag;
}

- (BOOL)automaticallyRearrangesObjects
{
	return _flags.automaticallyRearrangesObjects;
}

- (NSArray*)automaticRearrangementKeyPaths
{
	NSUnimplementedMethod();
	return nil;
}

- (void)didChangeArrangementCriteria
{
	NSUnimplementedMethod();
}

#pragma mark -
#pragma mark Debugging

- (NSString*)description
{
	return [NSString stringWithFormat: @"<%@ %x> content: %@ arrangedObjects: %@", [self class], self, _content, _arrangedObjects];
}

@end


