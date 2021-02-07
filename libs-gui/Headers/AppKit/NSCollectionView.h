/* -*-objc-*-
   NSCollectionView.h

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: Doug Simons (doug.simons@testplant.com)
           Frank LeGrand (frank.legrand@testplant.com)
   Date: February 2013
   
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

#ifndef _GNUstep_H_NSCollectionView
#define _GNUstep_H_NSCollectionView

#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSDragging.h>
#import <AppKit/NSNibDeclarations.h>
#import <AppKit/NSView.h>

@class NSCollectionViewItem;
@class NSCollectionView;

enum
{  
  NSCollectionViewDropOn = 0,
  NSCollectionViewDropBefore = 1,
};
typedef NSInteger NSCollectionViewDropOperation;

@protocol NSCollectionViewDelegate <NSObject>

- (NSImage *)collectionView:(NSCollectionView *)collectionView
draggingImageForItemsAtIndexes:(NSIndexSet *)indexes
                  withEvent:(NSEvent *)event
                     offset:(NSPointPointer)dragImageOffset;
- (BOOL)collectionView:(NSCollectionView *)collectionView
   writeItemsAtIndexes:(NSIndexSet *)indexes
          toPasteboard:(NSPasteboard *)pasteboard;
- (BOOL)collectionView:(NSCollectionView *)collectionView
 canDragItemsAtIndexes:(NSIndexSet *)indexes
             withEvent:(NSEvent *)event;
- (NSDragOperation)collectionView:(NSCollectionView *)collectionView
                     validateDrop:(id < NSDraggingInfo >)draggingInfo
                    proposedIndex:(NSInteger *)proposedDropIndex
                    dropOperation:(NSCollectionViewDropOperation *)proposedDropOperation;
- (BOOL)collectionView:(NSCollectionView *)collectionView
            acceptDrop:(id < NSDraggingInfo >)draggingInfo
                 index:(NSInteger)index
         dropOperation:(NSCollectionViewDropOperation)dropOperation;
- (NSArray *)collectionView:(NSCollectionView *)collectionView
namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropURL
   forDraggedItemsAtIndexes:(NSIndexSet *)indexes;

@end


@interface NSCollectionView : NSView //<NSDraggingDestination, NSDraggingSource>
{
  NSArray *_content;
  IBOutlet NSCollectionViewItem *itemPrototype;
  NSMutableArray *_items;
  
  BOOL _allowsMultipleSelection;
  BOOL _isSelectable;
  NSIndexSet *_selectionIndexes;
  
  NSArray *_backgroundColors;
  IBOutlet id <NSCollectionViewDelegate> delegate;
  
  NSSize _itemSize;
  NSSize _maxItemSize;
  NSSize _minItemSize;
  CGFloat _tileWidth;
  CGFloat _verticalMargin;
  CGFloat _horizontalMargin;

  NSUInteger _maxNumberOfColumns;
  NSUInteger _maxNumberOfRows;
  NSUInteger _numberOfColumns;
  
  NSDragOperation _draggingSourceOperationMaskForLocal;
  NSDragOperation _draggingSourceOperationMaskForRemote;
  
  NSUInteger _draggingOnRow;
  NSUInteger _draggingOnIndex;
}

- (BOOL) allowsMultipleSelection;
- (void) setAllowsMultipleSelection: (BOOL)flag;

- (NSArray *) backgroundColors;
- (void) setBackgroundColors: (NSArray *)colors;

- (NSArray *)content;
- (void)setContent:(NSArray *)content;

- (id < NSCollectionViewDelegate >) delegate;
- (void) setDelegate: (id < NSCollectionViewDelegate >)aDelegate;

- (NSCollectionViewItem *) itemPrototype;
- (void) setItemPrototype: (NSCollectionViewItem *)prototype;

- (NSSize) maxItemSize;
- (void) setMaxItemSize: (NSSize)size;

- (NSUInteger) maxNumberOfColumns;
- (void) setMaxNumberOfColumns: (NSUInteger)number;

- (NSUInteger) maxNumberOfRows;
- (void) setMaxNumberOfRows: (NSUInteger)number;

- (NSSize) minItemSize;
- (void) setMinItemSize: (NSSize)size;

- (BOOL) isSelectable;
- (void) setSelectable: (BOOL)flag;

- (NSIndexSet *) selectionIndexes;
- (void) setSelectionIndexes: (NSIndexSet *)indexes;

- (NSRect) frameForItemAtIndex: (NSUInteger)index;
- (NSCollectionViewItem *) itemAtIndex: (NSUInteger)index;
- (NSCollectionViewItem *) newItemForRepresentedObject:(id)object;

- (void) tile;

- (void) setDraggingSourceOperationMask: (NSDragOperation)dragOperationMask 
                               forLocal: (BOOL)localDestination;
							   
- (NSImage *) draggingImageForItemsAtIndexes: (NSIndexSet *)indexes
                                   withEvent: (NSEvent *)event
                                      offset: (NSPointPointer)dragImageOffset;


@end

#endif /* _GNUstep_H_NSCollectionView */
