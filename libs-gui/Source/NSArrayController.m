/** <title>NSArrayController</title>

   <abstract>Controller class for arrays</abstract>

   Copyright <copy>(C) 2006, 2020 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: June 2006

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/


#import <Foundation/NSArchiver.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSPredicate.h>
#import <Foundation/NSSortDescriptor.h>
#import <Foundation/NSString.h>

#import "AppKit/NSArrayController.h"
#import "AppKit/NSKeyValueBinding.h"
#import "GSBindingHelpers.h"
#import "GSFastEnumeration.h"

@implementation GSObservableArray

- (id) initWithArray: (NSArray *)array
{
  self = [super init];
  if (self)
    {
      ASSIGN(_array, array);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_array);
  [super dealloc];
}

- (NSUInteger) count
{
  return [_array count];
}

- (NSUInteger) indexOfObject: (id)anObject
{
  return [_array indexOfObject: anObject];
}

- (id) objectAtIndex: (NSUInteger)index
{
  return [_array objectAtIndex: index];
}

- (NSArray *) objectsAtIndexes: (NSIndexSet *)indexes
{
  NSArray *result = [_array objectsAtIndexes: indexes];

  return AUTORELEASE([[GSObservableArray alloc]
                                initWithArray: result]);
}

- (id) valueForKey: (NSString*)key
{
  id result = [_array valueForKey: key];

  if ([result isKindOfClass: [NSArray class]])
    {
      // FIXME: Using the correct memory management here
      // Leads to an issue inside of KVO. For now we leak the
      // object until this gets fixed.
      //return AUTORELEASE([[GSObservableArray alloc]
      return ([[GSObservableArray alloc]
                                initWithArray: result]);
    }

  return result;
}

- (NSArray*) arrayByAddingObject: (id)anObject
{
  NSArray * result = [_array arrayByAddingObject: anObject];

  return AUTORELEASE([[GSObservableArray alloc]
                                initWithArray: result]);
}

- (NSArray*) arrayByAddingObjectsFromArray: (NSArray*)anotherArray
{
  NSArray * result = [_array arrayByAddingObjectsFromArray: anotherArray];

  return AUTORELEASE([[GSObservableArray alloc]
                                initWithArray: result]);
}

- (void) addObserver: (NSObject*)anObserver
	  forKeyPath: (NSString*)aPath
	     options: (NSKeyValueObservingOptions)options
	     context: (void*)aContext
{
  NSIndexSet *indexes = [NSIndexSet indexSetWithIndexesInRange: NSMakeRange(0, [self count])];

  [self addObserver: anObserver
 toObjectsAtIndexes: indexes
         forKeyPath: aPath
            options: options
            context: aContext];
}

- (void) removeObserver: (NSObject*)anObserver forKeyPath: (NSString*)aPath
{
  NSIndexSet *indexes = [NSIndexSet indexSetWithIndexesInRange: NSMakeRange(0, [self count])];

  [self removeObserver: anObserver
  fromObjectsAtIndexes: indexes
            forKeyPath: aPath];
}

@end

@implementation NSArrayController

+ (void) initialize
{
  if (self == [NSArrayController class])
    {
      [self exposeBinding: NSContentArrayBinding];
      [self setKeys: [NSArray arrayWithObjects: NSContentBinding, NSContentObjectBinding, nil] 
            triggerChangeNotificationsForDependentKey: @"arrangedObjects"];
    }
}

- (id) initWithContent: (id)content
{
  if ((self = [super initWithContent: content]) != nil)
    {
      [self setSelectsInsertedObjects: YES];
    }

  return self;
}

- (id) init
{
  NSMutableArray *new = [[NSMutableArray alloc] init];

  self = [self initWithContent: new];
  RELEASE(new);
  return self;
}

- (void) dealloc
{
  DESTROY(_arranged_objects);
  DESTROY(_selection_indexes);
  DESTROY(_sort_descriptors);
  DESTROY(_filter_predicate);
  [super dealloc];
}

- (void) addObject: (id)obj
{
  [self willChangeValueForKey: NSContentBinding];
  [_content addObject: obj];
  if ([self automaticallyRearrangesObjects])
    {
      [self rearrangeObjects];
    }
  else 
    {
      DESTROY(_arranged_objects);
    }
  if ([self selectsInsertedObjects])
    {
      [self addSelectedObjects: [NSArray arrayWithObject: obj]];
    }
  [self didChangeValueForKey: NSContentBinding];
}

- (void) addObjects: (NSArray*)obj
{
  [self willChangeValueForKey: NSContentBinding];
  [_content addObjectsFromArray: obj];
  if ([self automaticallyRearrangesObjects])
    {
      [self rearrangeObjects];
    }
  else 
    {
      DESTROY(_arranged_objects);
    }
  if ([self selectsInsertedObjects])
    {
      [self addSelectedObjects: obj];
    }
  [self didChangeValueForKey: NSContentBinding];
}

- (void) removeObject: (id)obj
{
  [self willChangeValueForKey: NSContentBinding];
  [_content removeObject: obj];
  [self removeSelectedObjects: [NSArray arrayWithObject: obj]];
  if ([self automaticallyRearrangesObjects])
    {
      [self rearrangeObjects];
    }
  else 
    {
      DESTROY(_arranged_objects);
    }
  [self didChangeValueForKey: NSContentBinding];
}

- (void) removeObjects: (NSArray*)obj
{
  [self willChangeValueForKey: NSContentBinding];
  [_content removeObjectsInArray: obj];
  [self removeSelectedObjects: obj];
  if ([self automaticallyRearrangesObjects])
    {
      [self rearrangeObjects];
    }
  else 
    {
      DESTROY(_arranged_objects);
    }
  [self didChangeValueForKey: NSContentBinding];
}

- (BOOL) canInsert
{
  return YES;
}

- (void) setContent: (id)content
{
  [super setContent: content];
  [self rearrangeObjects];
}

- (void) insert: (id)sender
{
  id new = [self newObject];

  [self addObject: new];
  RELEASE(new);
}

- (NSIndexSet*) _indexSetForObjects: (NSArray*)objects
{
  NSMutableIndexSet *tmp = [NSMutableIndexSet new];
  id<NSFastEnumeration> enumerator = objects;

  FOR_IN (id, obj, enumerator)
  {
    NSUInteger index = [_arranged_objects indexOfObject: obj];
    if (NSNotFound != index)
      {
        [tmp addIndex: index];
      }
  }
  END_FOR_IN(enumerator)

  return AUTORELEASE(tmp);
}

- (BOOL) addSelectedObjects: (NSArray*)obj
{
  return [self addSelectionIndexes: [self _indexSetForObjects: obj]];
}

- (BOOL) addSelectionIndexes: (NSIndexSet*)idx
{
  NSMutableIndexSet *tmp = AUTORELEASE([_selection_indexes mutableCopy]);

  [tmp addIndexes: idx];
  return [self setSelectionIndexes: tmp];
}

- (BOOL) setSelectedObjects: (NSArray*)obj
{
  return [self setSelectionIndexes: [self _indexSetForObjects: obj]];
}

- (BOOL) setSelectionIndex: (NSUInteger)idx
{
  return [self setSelectionIndexes: 
                 [NSIndexSet indexSetWithIndex: idx]];
}

- (BOOL) setSelectionIndexes: (NSIndexSet*)idx
{
  if ([_selection_indexes isEqual: idx])
    {
      return NO;
    }
  else
    {
      ASSIGNCOPY(_selection_indexes, idx);
      return YES;
    }
}

- (BOOL) removeSelectedObjects: (NSArray*)obj
{
  return [self removeSelectionIndexes: [self _indexSetForObjects: obj]];
}

- (BOOL) removeSelectionIndexes: (NSIndexSet*)idx
{
  NSMutableIndexSet *tmp = AUTORELEASE([_selection_indexes mutableCopy]);

  [tmp removeIndexes: idx];
  return [self setSelectionIndexes: tmp];
}

- (BOOL) canSelectNext
{
  NSUInteger cur = [self selectionIndex];

  if ((cur == NSNotFound) || (cur + 1 == [_content count]))
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (BOOL) canSelectPrevious
{
  NSUInteger cur = [self selectionIndex];

  if ((cur == NSNotFound) || (cur == 0))
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (void) selectNext: (id)sender
{
  NSUInteger cur = [self selectionIndex];

  [self setSelectionIndexes: 
          [NSIndexSet indexSetWithIndex: cur + 1]];
}

- (void) selectPrevious: (id)sender
{
  NSUInteger cur = [self selectionIndex];

  [self setSelectionIndexes: 
          [NSIndexSet indexSetWithIndex: cur - 1]];
}

- (NSArray*) selectedObjects
{
  // We make the selection work on the arranged objects
  return [[self arrangedObjects] objectsAtIndexes: _selection_indexes];
}

- (NSUInteger) selectionIndex
{
  return [_selection_indexes firstIndex];
}

- (NSIndexSet*) selectionIndexes
{
  return AUTORELEASE([_selection_indexes copy]);
}

- (BOOL) avoidsEmptySelection
{
  return _acflags.avoids_empty_selection;
}

- (void) setAvoidsEmptySelection: (BOOL)flag
{
  _acflags.avoids_empty_selection = flag;
}

- (BOOL) preservesSelection
{
  return _acflags.preserves_selection;
}

- (void) setPreservesSelection: (BOOL)flag
{
  _acflags.preserves_selection = flag;
}

- (BOOL) alwaysUsesMultipleValuesMarker
{
  return _acflags.always_uses_multiple_values_marker;
}

- (void) setAlwaysUsesMultipleValuesMarker: (BOOL)flag
{
  _acflags.always_uses_multiple_values_marker = flag;
}

- (BOOL) clearsFilterPredicateOnInsertion
{
  return _acflags.clears_filter_predicate_on_insertion;
}

- (void) setClearsFilterPredicateOnInsertion: (BOOL)flag
{
  _acflags.clears_filter_predicate_on_insertion = flag;
}

- (BOOL) automaticallyRearrangesObjects
{
  return _acflags.automatically_rearranges_objects;
}

- (void) setAutomaticallyRearrangesObjects: (BOOL)flag
{
  _acflags.automatically_rearranges_objects = flag;
}

- (BOOL) selectsInsertedObjects
{
  return _acflags.selects_inserted_objects;
}

- (void) setSelectsInsertedObjects: (BOOL)flag
{
  _acflags.selects_inserted_objects = flag;
}

- (NSArray*) arrangeObjects: (NSArray*)obj
{
  NSArray *temp = obj;

  if (_filter_predicate != nil)
    {
      temp = [obj filteredArrayUsingPredicate: _filter_predicate];
    }

  return [temp sortedArrayUsingDescriptors: _sort_descriptors];
}

- (id) arrangedObjects
{
  if (_arranged_objects == nil)
    {
      [self rearrangeObjects];
    }
  return _arranged_objects;
}

- (void) rearrangeObjects
{
  [self willChangeValueForKey: @"arrangedObjects"];
  DESTROY(_arranged_objects);
  _arranged_objects = [[GSObservableArray alloc]
                          initWithArray: [self arrangeObjects: _content]];
  [self didChangeValueForKey: @"arrangedObjects"];
}

- (void) setSortDescriptors: (NSArray*)desc
{
  ASSIGNCOPY(_sort_descriptors, desc);
}

- (NSArray*) sortDescriptors
{
  return AUTORELEASE([_sort_descriptors copy]);
}

- (void) setFilterPredicate: (NSPredicate*)filterPredicate
{
  ASSIGN(_filter_predicate, filterPredicate);
}

- (NSPredicate*) filterPredicate
{
  return _filter_predicate;
}

- (void) insertObject: (id)obj 
atArrangedObjectIndex: (NSUInteger)idx
{
  // FIXME
  [self addObject: obj];
}

- (void) insertObjects: (NSArray*)obj 
atArrangedObjectIndexes: (NSIndexSet*)idx
{
  // FIXME
  [self addObjects: obj];
}

- (void) removeObjectAtArrangedObjectIndex: (NSUInteger)idx
{
  [self removeObject: [_arranged_objects objectAtIndex: idx]];
}

- (void) removeObjectsAtArrangedObjectIndexes: (NSIndexSet*)idx
{
  [self removeObjects: [_arranged_objects objectsAtIndexes: idx]];
}

- (void) bind: (NSString *)binding 
     toObject: (id)anObject
  withKeyPath: (NSString *)keyPath
      options: (NSDictionary *)options
{
  if ([binding isEqual: NSContentArrayBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueBinding alloc] initWithBinding: @"content" 
                                              withName: binding
                                              toObject: anObject
                                           withKeyPath: keyPath
                                               options: options
                                            fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else
    {
      [super bind: binding 
         toObject: anObject 
      withKeyPath: keyPath 
          options: options];
    }
}

- (Class) valueClassForBinding: (NSString *)binding
{
  if ([binding isEqual: NSContentArrayBinding])
    {
      return [NSArray class];
    }
  else
    {
      return [super valueClassForBinding: binding];
    }
}

- (void) encodeWithCoder: (NSCoder *)coder
{ 
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeBool: [self avoidsEmptySelection] forKey: @"NSAvoidsEmptySelection"];
      [coder encodeBool: [self preservesSelection] forKey: @"NSPreservesSelection"];
      [coder encodeBool: [self selectsInsertedObjects] forKey: @"NSSelectsInsertedObjects"];
      [coder encodeBool: [self clearsFilterPredicateOnInsertion] forKey:
               @"NSClearsFilterPredicateOnInsertion"];
      [coder encodeBool: [self automaticallyRearrangesObjects] forKey: @"NSAutomaticallyRearrangesObjects"];
    }
  else
    {
    }
}

- (id) initWithCoder: (NSCoder *)coder
{ 
  if ((self = [super initWithCoder: coder]) == nil)
    {
      return nil;
    }

  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSAvoidsEmptySelection"])
        {
          [self setAvoidsEmptySelection: 
                [coder decodeBoolForKey: @"NSAvoidsEmptySelection"]];
        }
      if ([coder containsValueForKey: @"NSPreservesSelection"])
        {
          [self setPreservesSelection: 
                [coder decodeBoolForKey: @"NSPreservesSelection"]];
        }
      if ([coder containsValueForKey: @"NSSelectsInsertedObjects"])
        {
          [self setSelectsInsertedObjects: 
                [coder decodeBoolForKey: @"NSSelectsInsertedObjects"]];
        }
      if ([coder containsValueForKey: @"NSClearsFilterPredicateOnInsertion"])
        {
          [self setClearsFilterPredicateOnInsertion: 
                [coder decodeBoolForKey: @"NSClearsFilterPredicateOnInsertion"]];
        }
      if ([coder containsValueForKey: @"NSAutomaticallyRearrangesObjects"])
        {
          [self setAutomaticallyRearrangesObjects: 
                [coder decodeBoolForKey: @"NSAutomaticallyRearrangesObjects"]];
        }
    }
  else
    {
    }

  return self; 
}

@end
