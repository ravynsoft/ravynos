/** <title>NSArrayController</title>

   <abstract>Controller class for arrays</abstract>

   Copyright <copy>(C) 2006 Free Software Foundation, Inc.</copy>

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

#ifndef _GNUstep_H_NSArrayController
#define _GNUstep_H_NSArrayController

#import <AppKit/NSObjectController.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3,GS_API_LATEST)

@class NSArray;
@class NSMutableArray;
@class NSIndexSet;
@class NSPredicate;

@interface NSArrayController : NSObjectController
{
  NSArray *_arranged_objects;
  NSIndexSet *_selection_indexes;
  NSArray *_sort_descriptors;
  NSPredicate *_filter_predicate;
  struct GSArrayControllerFlagsType { 
    unsigned always_uses_multiple_values_marker: 1;
    unsigned automatically_rearranges_objects: 1;
    unsigned avoids_empty_selection: 1;
    unsigned clears_filter_predicate_on_insertion: 1;
    unsigned preserves_selection: 1;
    unsigned selects_inserted_objects: 1;
  } _acflags;
}

- (void) addObject: (id)obj;
- (void) addObjects: (NSArray*)obj;
- (void) removeObject: (id)obj;
- (void) removeObjects: (NSArray*)obj;
- (BOOL) canInsert;
- (void) insert: (id)sender;

- (BOOL) addSelectedObjects: (NSArray*)obj;
- (BOOL) addSelectionIndexes: (NSIndexSet*)idx;
- (BOOL) setSelectedObjects: (NSArray*)obj;
- (BOOL) setSelectionIndex: (NSUInteger)idx;
- (BOOL) setSelectionIndexes: (NSIndexSet*)idx;
- (BOOL) removeSelectedObjects: (NSArray*)obj;
- (BOOL) removeSelectionIndexes: (NSIndexSet*)idx;
- (BOOL) canSelectNext;
- (BOOL) canSelectPrevious;
- (void) selectNext: (id)sender;
- (void) selectPrevious: (id)sender;
- (NSArray*) selectedObjects;
- (NSUInteger) selectionIndex;
- (NSIndexSet*) selectionIndexes;

- (BOOL) avoidsEmptySelection;
- (void) setAvoidsEmptySelection: (BOOL)flag;
- (BOOL) preservesSelection;
- (void) setPreservesSelection: (BOOL)flag;
- (BOOL) selectsInsertedObjects;
- (void) setSelectsInsertedObjects: (BOOL)flag;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) alwaysUsesMultipleValuesMarker;
- (void) setAlwaysUsesMultipleValuesMarker: (BOOL)flag;
- (BOOL) clearsFilterPredicateOnInsertion;
- (void) setClearsFilterPredicateOnInsertion: (BOOL)flag;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (BOOL) automaticallyRearrangesObjects;
- (void) setAutomaticallyRearrangesObjects: (BOOL)flag;
#endif

- (NSArray*) arrangeObjects: (NSArray*)obj;
- (id) arrangedObjects;
- (void) rearrangeObjects;
- (void) setSortDescriptors: (NSArray*)desc;
- (NSArray*) sortDescriptors;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) setFilterPredicate: (NSPredicate*)filterPredicate;
- (NSPredicate*) filterPredicate;
#endif

- (void) insertObject: (id)obj 
atArrangedObjectIndex: (NSUInteger)idx;
- (void) insertObjects: (NSArray*)obj 
atArrangedObjectIndexes: (NSIndexSet*)idx;
- (void) removeObjectAtArrangedObjectIndex: (NSUInteger)idx;
- (void) removeObjectsAtArrangedObjectIndexes: (NSIndexSet*)idx;

@end

#endif // OS_API_VERSION

#endif // _GNUstep_H_NSArrayController
