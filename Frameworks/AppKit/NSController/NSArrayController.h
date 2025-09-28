/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
   Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSObjectController.h>

@class NSPredicate, NSIndexSet, NSMutableIndexSet;

@interface NSArrayController : NSObjectController {
    struct {
        NSUInteger avoidsEmptySelection : 1;
        NSUInteger clearsFilterPredicateOnInsertion : 1;
        NSUInteger filterRestrictsInsertion : 1;
        NSUInteger preservesSelection : 1;
        NSUInteger selectsInsertedObjects : 1;
        NSUInteger alwaysUsesMultipleValuesMarker : 1;
        NSUInteger automaticallyPreparesContent : 1;
        NSUInteger automaticallyRearrangesObjects : 1;
    } _flags;
    NSMutableIndexSet *_selectionIndexes;
    NSArray *_sortDescriptors;
    id _filterPredicate;
    NSMutableArray *_arrangedObjects; // A cache of the current arrangement
}

#pragma mark -
#pragma mark Managing Sort Descriptors

- (void)setSortDescriptors:(NSArray *)descriptors;
- (NSArray *)sortDescriptors;

#pragma mark -
#pragma mark Arranging Objects

- (NSArray *)arrangeObjects:(NSArray *)objects;
- (id)arrangedObjects;
- (void)rearrangeObjects;

#pragma mark -
#pragma mark Managing Content

- (void)setAutomaticallyPreparesContent:(BOOL)flag;
- (BOOL)automaticallyPreparesContent;

#pragma mark -
#pragma mark Selection Attributes

- (void)setAvoidsEmptySelection:(BOOL)flag;
- (BOOL)avoidsEmptySelection;
- (void)setPreservesSelection:(BOOL)flag;
- (BOOL)preservesSelection;
- (void)setAlwaysUsesMultipleValuesMarker:(BOOL)flag;
- (BOOL)alwaysUsesMultipleValuesMarker;

#pragma mark -
#pragma mark Managing Selections

- (NSUInteger)selectionIndex;
- (BOOL)setSelectionIndex:(unsigned)index;
- (void)setSelectsInsertedObjects:(BOOL)flag;
- (BOOL)selectsInsertedObjects;
- (BOOL)setSelectionIndexes:(NSIndexSet *)indices;
- (NSIndexSet *)selectionIndexes;
- (BOOL)addSelectionIndexes:(NSIndexSet *)indices;
- (BOOL)removeSelectionIndexes:(NSIndexSet *)indices;
- (BOOL)setSelectedObjects:(NSArray *)objects;
- (NSArray *)selectedObjects;
- (BOOL)addSelectedObjects:(NSArray *)objects;
- (BOOL)removeSelectedObjects:(NSArray *)objects;
- (void)selectNext:sender;
- (BOOL)canSelectNext;
- (void)selectPrevious:sender;
- (BOOL)canSelectPrevious;

#pragma mark -
#pragma mark Inserting

- (BOOL)canInsert;
- (void)insert:(id)sender;

#pragma mark -
#pragma mark Adding and Removing Objects

- (void)add:(id)sender;
- (void)addObject:(id)object;
- (void)addObjects:(NSArray *)objects;
- (void)insertObject:(id)object atArrangedObjectIndex:(unsigned)index;
- (void)insertObjects:(NSArray *)objects atArrangedObjectIndexes:(NSIndexSet *)indices;
- (void)removeObjectAtArrangedObjectIndex:(unsigned)index;
- (void)removeObjectsAtArrangedObjectIndexes:(NSIndexSet *)indices;
- (void)remove:(id)sender;
- (void)removeObject:(id)object;
- (void)removeObjects:(NSArray *)objects;

#pragma mark -
#pragma mark Filtering Content

- (void)setClearsFilterPredicateOnInsertion:(BOOL)flag;
- (BOOL)clearsFilterPredicateOnInsertion;
- (void)setFilterPredicate:(NSPredicate *)predicate;
- (NSPredicate *)filterPredicate;

#pragma mark -
#pragma mark Automatic Content Rearranging

- (void)setAutomaticallyRearrangesObjects:(BOOL)flag;
- (BOOL)automaticallyRearrangesObjects;
- (NSArray *)automaticRearrangementKeyPaths;
- (void)didChangeArrangementCriteria;

@end
