/** <title>NSOutlineView</title>

   <abstract>
   This class is a subclass of NSTableView which provides the user with a way
   to display tree structured data in an outline format.
   It is particularly useful for show hierarchical data such as a
   class inheritance tree or any other set of relationships.<br />
   NB. While it its illegal to have the same item in the view more than once,
   it is possible to have multiple equal items since tests for pointer
   equality are used rather than calls to the -isEqual: method.
   </abstract>

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>
   Date: October 2001

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSOutlineView.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTableHeaderView.h"
#import "AppKit/NSText.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSWindow.h"

#import "GSGuiPrivate.h"
#include <math.h>

static NSMapTableKeyCallBacks keyCallBacks;
static NSNotificationCenter *nc = nil;
static const int current_version = 1;

static NSInteger lastVerticalQuarterPosition;
static NSInteger lastHorizontalHalfPosition;
static NSDragOperation dragOperation;

static NSRect oldDraggingRect;
static id oldDropItem;
static id currentDropItem;
static NSInteger oldDropIndex;
static NSInteger currentDropIndex;

static NSMutableSet *autoExpanded = nil;
static NSDate	*lastDragUpdate = nil;
static NSDate	*lastDragChange = nil;


// Cache the arrow images...
static NSImage *collapsed = nil;
static NSImage *expanded  = nil;
static NSImage *unexpandable  = nil;

@interface NSOutlineView (NotificationRequestMethods)
- (void) _postSelectionIsChangingNotification;
- (void) _postSelectionDidChangeNotification;
- (void) _postColumnDidMoveNotificationWithOldIndex: (NSInteger) oldIndex
                                           newIndex: (NSInteger) newIndex;
// FIXME: There is a method with a similar name.but this is never called
//- (void) _postColumnDidResizeNotification;
- (BOOL) _shouldSelectTableColumn: (NSTableColumn *)tableColumn;
- (BOOL) _shouldSelectRow: (NSInteger)rowIndex;
- (BOOL) _shouldSelectionChange;
- (BOOL) _shouldEditTableColumn: (NSTableColumn *)tableColumn
                            row: (NSInteger) rowIndex;
- (void) _willDisplayCell: (NSCell*)cell
           forTableColumn: (NSTableColumn *)tb
                      row: (NSInteger)index;
- (BOOL) _writeRows: (NSIndexSet *)rows
       toPasteboard: (NSPasteboard *)pboard;
- (BOOL) _isDraggingSource;
- (id) _objectValueForTableColumn: (NSTableColumn *)tb
                              row: (NSInteger)index;
- (void) _setObjectValue: (id)value
          forTableColumn: (NSTableColumn *)tb
                     row: (NSInteger) index;
- (NSInteger) _numRows;
@end

// These methods are private...
@interface NSOutlineView (TableViewInternalPrivate)
- (void) _initOutlineDefaults;
- (void) _autosaveExpandedItems;
- (void) _autoloadExpandedItems;
- (void) _collectItemsStartingWith: (id)startitem
                              into: (NSMutableArray *)allChildren;
- (void) _loadDictionaryStartingWith: (id) startitem
                             atLevel: (NSInteger) level;
- (void) _openItem: (id)item;
- (void) _closeItem: (id)item;
- (void) _removeChildren: (id)startitem;
- (void) _noteNumberOfRowsChangedBelowItem: (id)item by: (NSInteger)n;
@end

@interface	NSOutlineView (Private)
- (void) _autoCollapse;
@end

@implementation NSOutlineView

// Initialize the class when it is loaded
+ (void) initialize
{
  if (self == [NSOutlineView class])
    {
      [self setVersion: current_version];
      nc = [NSNotificationCenter defaultCenter];
      /* We need special map table callbacks, to check for identical
       * objects rather than merely equal objects.
       */
      keyCallBacks = NSObjectMapKeyCallBacks;
      keyCallBacks.isEqual = NSOwnedPointerMapKeyCallBacks.isEqual;
#if 0
/* Old Interface Builder style. */
      collapsed    = [NSImage imageNamed: @"common_outlineCollapsed"];
      expanded     = [NSImage imageNamed: @"common_outlineExpanded"];
      unexpandable = [NSImage imageNamed: @"common_outlineUnexpandable"];
#else
/* Current OSX style images. */
// FIXME ... better ones?
      collapsed    = [NSImage imageNamed: @"common_ArrowRightH"];
      expanded     = [NSImage imageNamed: @"common_ArrowDownH"];
      unexpandable = [[NSImage alloc] initWithSize: [expanded size]];
#endif
      autoExpanded = [NSMutableSet new];
    }
}

// Instance methods

/**
 *  Initalizes the outline view with the given frame.   Invokes
 * the superclass method initWithFrame: as well to initialize the object.
 *
 */
- (id) initWithFrame: (NSRect)frame
{
 self = [super initWithFrame: frame];

 if (self != nil)
   {
     [self _initOutlineDefaults];
     //_outlineTableColumn = nil;
   }

  return self;
}

- (void) dealloc
{
  RELEASE(_items);
  RELEASE(_expandedItems);

  NSFreeMapTable(_itemDict);
  NSFreeMapTable(_levelOfItems);

  if (_autosaveExpandedItems)
    {
      // notify when an item expands...
      [nc removeObserver: self
          name: NSOutlineViewItemDidExpandNotification
          object: self];

      // notify when an item collapses...
      [nc removeObserver: self
          name: NSOutlineViewItemDidCollapseNotification
          object: self];
    }

  [super dealloc];
}

/**
 * Causes the outline column, the column containing the expand/collapse
 * gadget, to resize based on the amount of space needed by widest content.
 */
- (BOOL) autoResizesOutlineColumn
{
  return _autoResizesOutlineColumn;
}

/**
 * Causes the outline column, the column containing the expand/collapse
 * gadget, to resize based on the amount of space needed by widest content.
 */
- (BOOL) autosaveExpandedItems
{
  return _autosaveExpandedItems;
}

/**
 * Collapses the given item only.  This is the equivalent of calling
 * [NSOutlineView-collapseItem:collapseChildren:] with NO.
 */
- (void) collapseItem: (id)item
{
  [self collapseItem: item collapseChildren: NO];
}

/**
 * Collapses the specified item.  If collapseChildren is set to YES,
 * then all of the expandable children of this item all also collapsed
 * in a recursive fashion (i.e. all children, grandchildren and etc).
 */
- (void) collapseItem: (id)item collapseChildren: (BOOL)collapseChildren
{
  const SEL shouldSelector = @selector(outlineView:shouldCollapseItem:);
  BOOL canCollapse = YES;

  if ([_delegate respondsToSelector: shouldSelector])
    {
      canCollapse = [_delegate outlineView: self shouldCollapseItem: item];
    }

  if ([self isExpandable: item] && [self isItemExpanded: item] && canCollapse)
    {
      NSMutableDictionary *infoDict = [NSMutableDictionary dictionary];

      [infoDict setObject: item forKey: @"NSObject"];

      // Send out the notification to let observers know that this is about
      // to occur.
      [nc postNotificationName: NSOutlineViewItemWillCollapseNotification
          object: self
          userInfo: infoDict];

      // recursively find all children and call this method to close them.
      // Note: The children must be collapsed before their parent item so
      // that the selected row indexes are properly updated (and in particular
      // are valid when we post our notifications).
      if (collapseChildren) // collapse all
        {
          int index, numChildren;
          NSMutableArray *allChildren;
          id sitem = (item == nil) ? (id)[NSNull null] : (id)item;

          allChildren = NSMapGet(_itemDict, sitem);
          numChildren = [allChildren count];

          for (index = 0; index < numChildren; index++)
            {
              id child = [allChildren objectAtIndex: index];

              if ([self isExpandable: child])
                {
                  [self collapseItem: child collapseChildren: collapseChildren];
                }
            }
        }

      // collapse...
      [self _closeItem: item];

      // Send out the notification to let observers know that this has
      // occurred.
      [nc postNotificationName: NSOutlineViewItemDidCollapseNotification
          object: self
          userInfo: infoDict];

      // Should only mark the rect below the closed item for redraw
      [self setNeedsDisplay: YES];
    }
}

/**
 * Expands the given item only.  This is the equivalent of calling
 * [NSOutlineView-expandItem:expandChildren:] with NO.
 */
- (void) expandItem: (id)item
{
  [self expandItem: item expandChildren: NO];
}

/**
 * Expands the specified item.  If expandChildren is set to YES, then all
 * of the expandable children of this item all also expanded in a recursive
 * fashion (i.e.  all children, grandchildren and etc).
 */
- (void) expandItem: (id)item expandChildren: (BOOL)expandChildren
{
  const SEL shouldExpandSelector = @selector(outlineView:shouldExpandItem:);
  BOOL canExpand = YES;

  if ([_delegate respondsToSelector: shouldExpandSelector])
    {
      canExpand = [_delegate outlineView: self shouldExpandItem: item];
    }

  // if the item is expandable
  if ([self isExpandable: item])
    {
      // if it is not already expanded and it can be expanded, then expand
      if (![self isItemExpanded: item] && canExpand)
        {
          NSMutableDictionary *infoDict = [NSMutableDictionary dictionary];

          [infoDict setObject: item forKey: @"NSObject"];

          // Send out the notification to let observers know that this is about
          // to occur.
          [nc postNotificationName: NSOutlineViewItemWillExpandNotification
              object: self
              userInfo: infoDict];

          // insert the root element, if necessary otherwise insert the
          // actual object.
          [self _openItem: item];

          // Send out the notification to let observers know that this has
          // occurred.
          [nc postNotificationName: NSOutlineViewItemDidExpandNotification
              object: self
              userInfo: infoDict];
        }

      // recursively find all children and call this method to open them.
      if (expandChildren) // expand all
        {
          int index, numChildren;
          NSMutableArray *allChildren;
          id sitem = (item == nil) ? (id)[NSNull null] : (id)item;

          allChildren = NSMapGet(_itemDict, sitem);
          numChildren = [allChildren count];

          for (index = 0; index < numChildren; index++)
            {
              id child = [allChildren objectAtIndex: index];

              if ([self isExpandable: child])
                {
                  [self expandItem: child expandChildren: expandChildren];
                }
            }
        }

      // Should only mark the rect below the expanded item for redraw
      [self setNeedsDisplay: YES];
    }
}

- (NSRect) frameOfOutlineCellAtRow: (NSInteger)row
{
  NSRect frameRect;

  if (![self isExpandable: [self itemAtRow: row]])
    return NSZeroRect;

  frameRect = [self frameOfCellAtColumn: 0
                                    row: row];
  
  if (_indentationMarkerFollowsCell)
    {
      frameRect.origin.x += _indentationPerLevel * [self levelForRow: row];
    }

  return frameRect;
}

/**
 * Returns whether or not the indentation marker or "knob" is indented
 * along with the content inside the cell.
 */
- (BOOL) indentationMarkerFollowsCell
{
  return _indentationMarkerFollowsCell;
}

/**
 * Returns the amount of indentation, in points, for each level
 * of the tree represented by the outline view.
 */
- (CGFloat) indentationPerLevel
{
  return _indentationPerLevel;
}

/**
 * Returns YES, if the item is able to be expanded, NO otherwise.
 *
 * Returns NO when the item is nil (as Cocoa does).
 */
- (BOOL) isExpandable: (id)item
{
  if (item == nil)
    {
      return NO;
    }
  return [_dataSource outlineView: self isItemExpandable: item];
}

/**
 * Returns YES if the item is expanded or open, NO otherwise.
 *
 * Returns YES when the item is nil (as Cocoa does).
 */
- (BOOL) isItemExpanded: (id)item
{
  if (item == nil)
    {
      return YES;
    }
  // Check the array to determine if it is expanded.
  if ([_expandedItems indexOfObjectIdenticalTo: item] == NSNotFound)
    {
      return NO;
    }
  return YES;
}

/**
 * Returns the item at a given row. If no item exists for the given row,
 * returns nil.
 */
- (id) itemAtRow: (NSInteger)row
{
  if ((row >= [_items count]) || (row < 0))
    {
      return nil;
    }
  return [_items objectAtIndex: row];
}

/**
 * Returns the level for a given item.
 */
- (NSInteger) levelForItem: (id)item
{
  if (item != nil)
    {
      id object = NSMapGet(_levelOfItems, item);
      return [object integerValue];
    }

  return -1;
}

/**
 * Returns the level for the given row.
 */
- (NSInteger) levelForRow: (NSInteger)row
{
  return [self levelForItem: [self itemAtRow: row]];
}

/**
 * Returns the outline table column.
 */
- (NSTableColumn *) outlineTableColumn
{
  return _outlineTableColumn;
}

/**
 * Returns the parent of the given item or nil if the item is not found.
 */
- (id) parentForItem: (id)item
{
  NSArray *allKeys = NSAllMapTableKeys(_itemDict);
  NSEnumerator *en = [allKeys objectEnumerator];
  NSInteger index;
  id parent;

  while ((parent = [en nextObject]))
    {
      NSMutableArray *childArray = NSMapGet(_itemDict, parent);

      if ((index = [childArray indexOfObjectIdenticalTo: item]) != NSNotFound)
        {
          return (parent == [NSNull null]) ? (id)nil : (id)parent;
        }
    }

  return nil;
}

/**
 * Causes an item to be reloaded.  This is the equivalent of calling
 * [NSOutlineView-reloadItem:reloadChildren:] with reloadChildren set to NO.
 */
- (void) reloadItem: (id)item
{
  [self reloadItem: item reloadChildren: NO];
}

/**
 * Causes an item and all of it's children to be reloaded if reloadChildren is
 * set to YES, if it's set to NO, then only the item itself is refreshed
 * from the datasource.
 */
- (void) reloadItem: (id)item reloadChildren: (BOOL)reloadChildren
{
  NSInteger index;
  id parent;
  BOOL expanded;
  id dsobj = nil;
  id object = (item == nil) ? (id)[NSNull null] : (id)item;
  NSArray *allKeys = NSAllMapTableKeys(_itemDict);
  NSEnumerator *en = [allKeys objectEnumerator];

  expanded = [self isItemExpanded: item];

  // find the parent of the item
  while ((parent = [en nextObject]))
    {
      NSMutableArray *childArray = NSMapGet(_itemDict, parent);

      if ((index = [childArray indexOfObjectIdenticalTo: object]) != NSNotFound)
        {
          parent = (parent == [NSNull null]) ? (id)nil : (id)parent;
          dsobj = [_dataSource outlineView: self
                               child: index
                               ofItem: parent];

          if (dsobj != item)
            {
              [childArray replaceObjectAtIndex: index withObject: dsobj];
              // FIXME We need to correct _items, _itemDict, _levelOfItems,
              // _expandedItems and _selectedItems
            }
          break;
        }
    }

  if (reloadChildren)
    {
      [self _removeChildren: dsobj];
      [self _loadDictionaryStartingWith: dsobj
            atLevel: [self levelForItem: dsobj]];

      if (expanded)
        {
          [self _openItem: dsobj];
        }
    }
  [self setNeedsDisplay: YES];
}

/**
 * Returns the corresponding row in the outline view for the given item.
 * Returns -1 if item is nil or not found.
 */
- (NSInteger) rowForItem: (id)item
{
  NSInteger row;
  if (item == nil)
    return -1;

  row = [_items indexOfObjectIdenticalTo: item];
  return (row == NSNotFound) ? -1 : row;
}

/**
 * When set to YES this causes the outline column, the column containing
 * the expand/collapse gadget, to resize based on the amount of space
 * needed by widest content.
 */
- (void) setAutoresizesOutlineColumn: (BOOL)resize
{
  _autoResizesOutlineColumn = resize;
}

/**
 * When set to YES, the outline view will save the state of all expanded or
 * collapsed items in the view to the users defaults for the application the
 * outline view is running in.
 */
- (void) setAutosaveExpandedItems: (BOOL)flag
{
  if (flag == _autosaveExpandedItems)
    {
      return;
    }

  _autosaveExpandedItems = flag;
  if (flag)
    {
      [self _autoloadExpandedItems];
      // notify when an item expands...
      [nc addObserver: self
          selector: @selector(_autosaveExpandedItems)
          name: NSOutlineViewItemDidExpandNotification
          object: self];

      // notify when an item collapses...
      [nc addObserver: self
          selector: @selector(_autosaveExpandedItems)
          name: NSOutlineViewItemDidCollapseNotification
          object: self];
    }
  else
    {
      // notify when an item expands...
      [nc removeObserver: self
          name: NSOutlineViewItemDidExpandNotification
          object: self];

      // notify when an item collapses...
      [nc removeObserver: self
          name: NSOutlineViewItemDidCollapseNotification
          object: self];
    }
}

/**
 * If set to YES, the indentation marker will follow the content at each level.
 * Otherwise, the indentation marker will remain at the left most position of
 * the view regardless of how many levels in the content is indented.
 */
- (void) setIndentationMarkerFollowsCell: (BOOL)followsCell
{
  _indentationMarkerFollowsCell = followsCell;
}

/**
 * Sets the amount, in points, that each level is to be indented by.
 */
- (void) setIndentationPerLevel: (CGFloat)newIndentLevel
{
  _indentationPerLevel = newIndentLevel;
}

/**
 * Sets the outline table column in which to place the indentation marker.
 */
- (void)setOutlineTableColumn: (NSTableColumn *)outlineTableColumn
{
  _outlineTableColumn = outlineTableColumn;
}

/**
 * Returns YES, by default.   Subclasses should override this method if
 * a different behaviour is required.
 */
- (BOOL)shouldCollapseAutoExpandedItemsForDeposited: (BOOL)deposited
{
  return YES;
}

/**
 * Sets the data source for this outline view.
 */
- (void) setDataSource: (id)anObject
{
#define CHECK_REQUIRED_METHOD(selector_name) \
  if (anObject && ![anObject respondsToSelector: @selector(selector_name)]) \
    [NSException raise: NSInternalInconsistencyException \
                 format: @"data source does not respond to %@", @#selector_name]

  CHECK_REQUIRED_METHOD(outlineView:child:ofItem:);
  CHECK_REQUIRED_METHOD(outlineView:isItemExpandable:);
  CHECK_REQUIRED_METHOD(outlineView:numberOfChildrenOfItem:);
  CHECK_REQUIRED_METHOD(outlineView:objectValueForTableColumn:byItem:);

  // Is the data source editable?
  _dataSource_editable = [anObject respondsToSelector:
    @selector(outlineView:setObjectValue:forTableColumn:byItem:)];

  /* We do *not* retain the dataSource, it's like a delegate */
  _dataSource = anObject;
  [self tile];
  [self reloadData];
}

/**
 * Forces a from scratch reload of all data in the outline view.
 */
- (void) reloadData
{
  // release the old array
  if (_items != nil)
    {
      RELEASE(_items);
    }

  if (_itemDict != NULL)
    {
      NSFreeMapTable(_itemDict);
    }

  if (_levelOfItems != NULL)
    {
      NSFreeMapTable(_levelOfItems);
    }

  // create a new empty one
  _items = [[NSMutableArray alloc] init];
  _itemDict = NSCreateMapTable(keyCallBacks,
                               NSObjectMapValueCallBacks,
                               64);
  _levelOfItems = NSCreateMapTable(keyCallBacks,
                                   NSObjectMapValueCallBacks,
                                   64);

  // reload all the open items...
  [self _openItem: nil];
  [super reloadData];
}

/**
 * Sets the delegate of the outlineView.
 */
- (void) setDelegate: (id)anObject
{
  const SEL sel = @selector(outlineView:willDisplayCell:forTableColumn:item:);

  if (_delegate)
    [nc removeObserver: _delegate name: nil object: self];
  _delegate = anObject;

#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(outlineView##notif_name:)]) \
    [nc addObserver: _delegate \
      selector: @selector(outlineView##notif_name:) \
      name: NSOutlineView##notif_name##Notification object: self]

  SET_DELEGATE_NOTIFICATION(ColumnDidMove);
  SET_DELEGATE_NOTIFICATION(ColumnDidResize);
  SET_DELEGATE_NOTIFICATION(SelectionDidChange);
  SET_DELEGATE_NOTIFICATION(SelectionIsChanging);
  SET_DELEGATE_NOTIFICATION(ItemDidExpand);
  SET_DELEGATE_NOTIFICATION(ItemDidCollapse);
  SET_DELEGATE_NOTIFICATION(ItemWillExpand);
  SET_DELEGATE_NOTIFICATION(ItemWillCollapse);

  _del_responds = [_delegate respondsToSelector: sel];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding] == NO)
    {
      float indentation = _indentationPerLevel;
      [aCoder encodeValueOfObjCType: @encode(BOOL)
                                 at: &_autoResizesOutlineColumn];
      [aCoder encodeValueOfObjCType: @encode(BOOL)
                                 at: &_indentationMarkerFollowsCell];
      [aCoder encodeValueOfObjCType: @encode(BOOL)
                                 at: &_autosaveExpandedItems];
      [aCoder encodeValueOfObjCType: @encode(float)
                                 at: &indentation];
      [aCoder encodeConditionalObject: _outlineTableColumn];
    }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  // Since we only have one version....
  self = [super initWithCoder: aDecoder];
  if (self == nil)
    return self;

  [self _initOutlineDefaults];

  if ([aDecoder allowsKeyedCoding])
    {
      // init the table column... (this can't be chosen on IB either)...
      if ([_tableColumns count] > 0)
        {
          _outlineTableColumn = [_tableColumns objectAtIndex: 0];
        }
    }
  else
    {
      float indentation;
      // overrides outline defaults with archived values
      [aDecoder decodeValueOfObjCType: @encode(BOOL)
                                   at: &_autoResizesOutlineColumn];
      [aDecoder decodeValueOfObjCType: @encode(BOOL)
                                   at: &_indentationMarkerFollowsCell];
      [aDecoder decodeValueOfObjCType: @encode(BOOL)
                                   at: &_autosaveExpandedItems];
      [aDecoder decodeValueOfObjCType: @encode(float)
                                   at: &indentation];
      _indentationPerLevel = indentation;
      _outlineTableColumn = [aDecoder decodeObject];
    }
  return self;
}

- (void) mouseDown: (NSEvent *)theEvent
{
  NSPoint location = [theEvent locationInWindow];

  location = [self convertPoint: location  fromView: nil];
  _clickedRow = [self rowAtPoint: location];
  _clickedColumn = [self columnAtPoint: location];

  if (_clickedRow != -1
      && [_tableColumns objectAtIndex: _clickedColumn] == _outlineTableColumn)
    {
      NSImage *image;

      id item = [self itemAtRow:_clickedRow];
      NSInteger level = [self levelForRow: _clickedRow];
      NSInteger position = 0;

      if ([self isItemExpanded: item])
        {
          image = expanded;
        }
      else
        {
          image = collapsed;
        }

      if (_indentationMarkerFollowsCell)
        {
          position = _indentationPerLevel * level;
        }

      position += _columnOrigins[_clickedColumn];

      if ([self isExpandable:item]
        && location.x >= position
        && location.x <= position + [image size].width)
        {
          BOOL withChildren =
	    ([theEvent modifierFlags] & NSAlternateKeyMask) ? YES : NO;
          if (![self isItemExpanded: item])
            {
              [self expandItem: item expandChildren: withChildren];
            }
          else
            {
              [self collapseItem: item collapseChildren: withChildren];
            }
          return;
        }
    }

  [super mouseDown: theEvent];
}

- (void)keyDown: (NSEvent*)event
{
   NSString *characters = [event characters];

   if ([characters length] == 1)
     {
       unichar c = [characters characterAtIndex: 0];

       NSIndexSet *selected = [self selectedRowIndexes];
       NSInteger i;
       for (i = [selected firstIndex]; i != NSNotFound; i = [selected indexGreaterThanIndex: i])
	 {
	   id item = [self itemAtRow: i];
	   switch (c)
	     {
	     case NSLeftArrowFunctionKey:
	       {
		 if ([self isItemExpanded: item])
		   {
		     [self collapseItem: item];
		   }
		 else
		   {
		     id parent = [self parentForItem: item];
		     if (parent != nil)
		       {
			 NSInteger parentRow = [self rowForItem: parent];
			 [self selectRow: parentRow
			       byExtendingSelection: NO];
			 [self scrollRowToVisible: parentRow];
		       }
		   }
		 return;
	       }
	     case NSRightArrowFunctionKey:
	       [self expandItem: item];
	       return;
	     default:
	       break;
	     }
	 }
     }
 
   [super keyDown: event];
}

/*
 * Drawing
 */
- (void) drawRow: (NSInteger)rowIndex clipRect: (NSRect)aRect
{
  NSInteger startingColumn;
  NSInteger endingColumn;
  NSRect drawingRect;
  NSCell *imageCell = nil;
  NSRect imageRect;
  NSInteger i;
  CGFloat x_pos;

  if (_dataSource == nil)
    {
      return;
    }

  /* Using columnAtPoint: here would make it called twice per row per drawn
     rect - so we avoid it and do it natively */

  if (rowIndex >= _numberOfRows)
    {
      return;
    }

  /* Determine starting column as fast as possible */
  x_pos = NSMinX (aRect);
  i = 0;
  while ((i < _numberOfColumns) && (x_pos > _columnOrigins[i]))
    {
      i++;
    }
  startingColumn = (i - 1);

  if (startingColumn == -1)
    startingColumn = 0;

  /* Determine ending column as fast as possible */
  x_pos = NSMaxX (aRect);
  // Nota Bene: we do *not* reset i
  while ((i < _numberOfColumns) && (x_pos > _columnOrigins[i]))
    {
      i++;
    }
  endingColumn = (i - 1);

  if (endingColumn == -1)
    endingColumn = _numberOfColumns - 1;

  /* Draw the row between startingColumn and endingColumn */
  for (i = startingColumn; i <= endingColumn; i++)
    {
      id item = [self itemAtRow: rowIndex];
      NSTableColumn *tb = [_tableColumns objectAtIndex: i];
      NSCell *cell = [self preparedCellAtColumn: i row: rowIndex];

      [self _willDisplayCell: cell
            forTableColumn: tb
            row: rowIndex];
      if (i == _editedColumn && rowIndex == _editedRow)
        {
          [cell _setInEditing: YES];
          [cell setShowsFirstResponder: YES];
        }
      else
        {
          [cell setObjectValue: [_dataSource outlineView: self
                                             objectValueForTableColumn: tb
                                                  byItem: item]];
        }
      drawingRect = [self frameOfCellAtColumn: i
                          row: rowIndex];

      if (tb == _outlineTableColumn)
        {
          NSImage *image = nil;
          NSInteger level = 0;
          CGFloat indentationFactor = 0.0;
          // float originalWidth = drawingRect.size.width;

          // display the correct arrow...
          if ([self isItemExpanded: item])
            {
              image = expanded;
            }
          else
            {
              image = collapsed;
            }

          if (![self isExpandable: item])
            {
              image = unexpandable;
            }

          level = [self levelForItem: item];
          indentationFactor = _indentationPerLevel * level;
          imageCell = [[NSCell alloc] initImageCell: image];
          imageRect = [self frameOfOutlineCellAtRow: rowIndex];

          if ([_delegate respondsToSelector: @selector(outlineView:willDisplayOutlineCell:forTableColumn:item:)])
            {
              [_delegate outlineView: self
                         willDisplayOutlineCell: imageCell
                         forTableColumn: tb
                         item: item];
            }

          /* Do not indent if the delegate set the image to nil. */
          if ([imageCell image])
            {
              imageRect.size.width = [image size].width;
              imageRect.size.height = [image size].height;
              [imageCell drawWithFrame: imageRect inView: self];
              drawingRect.origin.x
                += indentationFactor + imageRect.size.width + 5;
              drawingRect.size.width
                -= indentationFactor + imageRect.size.width + 5;
            }
          else
            {
              drawingRect.origin.x += indentationFactor;
              drawingRect.size.width -= indentationFactor;
            }

          RELEASE(imageCell);
        }

      [cell drawWithFrame: drawingRect inView: self];
      if (i == _editedColumn && rowIndex == _editedRow)
        {
          [cell _setInEditing: NO];
          [cell setShowsFirstResponder: NO];
        }
    }
}

- (void) drawRect: (NSRect)aRect
{
  NSInteger index = 0;

  if (_autoResizesOutlineColumn)
    {
      CGFloat widest = 0;
      for (index = 0; index < _numberOfRows; index++)
        {
          CGFloat offset = [self levelForRow: index] *
            [self indentationPerLevel];
          NSRect drawingRect = [self frameOfCellAtColumn: 0
                                     row: index];
          CGFloat length = drawingRect.size.width + offset;
          if (widest < length) widest = length;
        }
      // [_outlineTableColumn setWidth: widest];
    }

  [super drawRect: aRect];
}

- (void) setDropItem: (id)item
      dropChildIndex: (NSInteger)childIndex
{
  if (item != nil && [_items indexOfObjectIdenticalTo: item] == NSNotFound)
    {
      /* FIXME raise an exception, or perhaps we should support
       * setting an item which is not visible (inside a collapsed
       * item presumably), or perhaps we should treat this as
       * cancelling the drop?
       */
      return;
    }
  currentDropItem = item;
  currentDropIndex = childIndex;
}

/*
 *  Drag'n'drop support
 */

- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>) sender
{
  //NSLog(@"draggingEntered");
  oldDropItem = currentDropItem = nil;
  oldDropIndex = currentDropIndex = -1;
  lastVerticalQuarterPosition = -1;
  dragOperation = NSDragOperationCopy;
  oldDraggingRect = NSMakeRect(0.,0., 0., 0.);
  return NSDragOperationCopy;
}

- (void) draggingExited: (id <NSDraggingInfo>) sender
{
  [self setNeedsDisplayInRect: oldDraggingRect];
  [self _autoCollapse];
  [self displayIfNeeded];
  DESTROY(lastDragUpdate);
  DESTROY(lastDragChange);
}

// TODO: Move the part that starts at 'Compute the indicator rect area' to GSTheme
- (void) drawDropAboveIndicatorWithDropItem: (id)currentDropItem 
                                      atRow: (NSInteger)row 
                             childDropIndex: (NSInteger)currentDropIndex
{
  NSInteger level = 0;
  NSBezierPath *path = nil;
  NSRect newRect = NSZeroRect;

  /* Compute the indicator rect area */
  if (currentDropItem == nil && currentDropIndex == 0)
    {
      newRect = NSMakeRect([self visibleRect].origin.x,
                           0,
                           [self visibleRect].size.width,
                           2);
    }
  else if (row == _numberOfRows)
    {
      newRect = NSMakeRect([self visibleRect].origin.x,
                           row * _rowHeight - 2,
                           [self visibleRect].size.width,
                           2);
    }
  else
    {
      newRect = NSMakeRect([self visibleRect].origin.x,
                           row * _rowHeight - 1,
                           [self visibleRect].size.width,
                           2);
    }
  level = [self levelForItem: currentDropItem] + 1;
  newRect.origin.x += level * _indentationPerLevel;
  newRect.size.width -= level * _indentationPerLevel;

  [[NSColor darkGrayColor] set];

  /* The rectangle is a line across the cell indicating the
   * insertion position.  We adjust by enough pixels to allow for
   * a ring drawn on the left end.
   */
  newRect.size.width -= 7;
  newRect.origin.x += 7;
  NSRectFill(newRect);

  /* We make the redraw rectangle big enough to hold both the
   * line and the circle (8 pixels high).
   */
  newRect.size.width += 7;
  newRect.origin.x -= 7;
  newRect.size.height = 8;
  newRect.origin.y -= 3;
  oldDraggingRect = newRect;
  if (newRect.size.width < 8)
    oldDraggingRect.size.width = 8;

  /* We draw the circle at the left of the line, and make it
   * a little smaller than the redraw rectangle so that the
   * bezier path will draw entirely inside the redraw area
   * and we won't leave artifacts behind on the screen.
   */
  newRect.size.width = 7;
  newRect.size.height = 7;
  newRect.origin.x += 0.5;
  newRect.origin.y += 0.5;
  path = [NSBezierPath bezierPath];
  [path appendBezierPathWithOvalInRect: newRect];
  [path stroke];
}

/* When the drop item is nil and the drop child index is -1 */
- (void) drawDropOnRootIndicator
{
  NSRect indicatorRect = [self visibleRect];

  /* Remember indicator area to be redrawn next time */
  oldDraggingRect = indicatorRect;

  [[NSColor darkGrayColor] set];
  NSFrameRectWithWidth(indicatorRect, 2.0);
}

// TODO: Move a method common to -drapOnRootIndicator and the one below to GSTheme
- (void) drawDropOnIndicatorWithDropItem: (id)currentDropItem
{
  NSInteger row = [_items indexOfObjectIdenticalTo: currentDropItem];
  NSInteger level = [self levelForItem: currentDropItem];
  NSRect newRect = [self frameOfCellAtColumn: 0
                                         row: row];

  newRect.origin.x = _bounds.origin.x;
  newRect.size.width = _bounds.size.width + 2;
  newRect.origin.x -= _intercellSpacing.height / 2;
  newRect.size.height += _intercellSpacing.height;

  /* Remember indicator area to be redrawn next time */
  oldDraggingRect = newRect;
  oldDraggingRect.origin.y -= 1;
  oldDraggingRect.size.height += 2;
  
  newRect.size.height -= 1;
  newRect.origin.x += 3;
  newRect.size.width -= 3;

  if (_drawsGrid)
    {
      //newRect.origin.y += 1;
      //newRect.origin.x += 1;
      //newRect.size.width -= 2;
      newRect.size.height += 1;
    }
 
  newRect.origin.x += level * _indentationPerLevel;
  newRect.size.width -= level * _indentationPerLevel;

  [[NSColor darkGrayColor] set];
  NSFrameRectWithWidth(newRect, 2.0);
}

/* Returns the row whose item is the parent that owns the child at the given row. 
Also returns the child index relative to this parent. */
- (NSInteger) _parentRowForRow: (NSInteger)row 
                       atLevel: (NSInteger)level 
           andReturnChildIndex: (NSInteger *)childIndex
{
  NSInteger i;
  NSInteger lvl;

  *childIndex = 0;

  for (i = row - 1; i >= 0; i--)
    {
      BOOL foundParent;
      BOOL foundSibling;

      lvl = [self levelForRow: i];

      foundParent = (lvl == level - 1);
      foundSibling = (lvl == level);

      if (foundParent)
      {
          break;
      }
      else if (foundSibling)
      {
        (*childIndex)++;
      }
    }

  return i;
}

- (NSDragOperation) draggingUpdated: (id <NSDraggingInfo>) sender
{
  NSPoint p = [self convertPoint: [sender draggingLocation] fromView: nil];
  /* The insertion row.
   * The insertion row is identical to the hovered row, except when p is in 
   * the hovered row bottom part (the last quarter).
   */
  NSInteger row;
  /* A row can be divided into 4 vertically stacked portions.
   * We call each portion a quarter. 
   * verticalQuarterPosition is the number of quarters that exists between the 
   * top left origin (NSOutlineView is flipped) and the hovered row (precisely 
   * up to the quarter occupied by the pointer in this row). 
   */
  NSInteger verticalQuarterPosition;
  /* An indentation unit can be divided into 2 portions (left and right).
   * We call each portion a half.
   * We use it to compute the insertion level. */
  NSInteger horizontalHalfPosition;
  /* The quarter (0, 1, 2 or 3) occupied by the pointer within the hovered row
   * (not in the insertion row). */
  NSInteger positionInRow;
  /* The previous row level (the row before the insertion row) */
  NSInteger levelBefore;
  /* The next row level (the row after the insertion row) */
  NSInteger levelAfter;
  /* The insertion level that may vary with the horizontal pointer position, 
   * when the pointer is between two rows and the bottom row is a parent.
   */
  NSInteger level;


  ASSIGN(lastDragUpdate, [NSDate date]);
  //NSLog(@"draggingUpdated");

  /* _bounds.origin is (0, 0) when the outline view is not clipped.
   * When the view is scrolled, _bounds.origin.y returns the scrolled height. */
  verticalQuarterPosition =
    GSRoundTowardsInfinity(((p.y + _bounds.origin.y) / _rowHeight) * 4.);
  horizontalHalfPosition =
    GSRoundTowardsInfinity(((p.x + _bounds.origin.y) / _indentationPerLevel) * 2.);

  /* We add an extra quarter to shift the insertion row below the hovered row. */
  row = (verticalQuarterPosition + 1) / 4;
  positionInRow = verticalQuarterPosition % 4;
  if (row > _numberOfRows)
    {
      row = _numberOfRows; // beyond the last real row
      positionInRow = 1;   // inside the root item (we could also use 2)
    }

  //NSLog(@"horizontalHalfPosition = %d", horizontalHalfPosition);
  //NSLog(@"verticalQuarterPosition = %d", verticalQuarterPosition);
  //NSLog(@"insertion row = %d", row);

  if (row == 0)
    {
      levelBefore = 0;
    }
  else
    {
      levelBefore = [self levelForRow: (row - 1)];
    }
  if (row == _numberOfRows)
    {
      levelAfter = 0;
    }
  else
    {
      levelAfter = [self levelForRow: row];
    }
  //NSLog(@"level before = %d", levelBefore);
  //NSLog(@"level after = %d", levelAfter);

  if ((lastVerticalQuarterPosition != verticalQuarterPosition)
    || (lastHorizontalHalfPosition != horizontalHalfPosition))
    {
      NSInteger minInsertionLevel = levelAfter;
      NSInteger maxInsertionLevel = levelBefore;
      NSInteger pointerInsertionLevel = GSRoundTowardsInfinity((float)horizontalHalfPosition / 2.);

      /* Save positions to avoid executing this code when the general
       * position of the mouse is unchanged.
       */
      lastVerticalQuarterPosition = verticalQuarterPosition;
      lastHorizontalHalfPosition = horizontalHalfPosition;

      /* When the row before is an empty parent, we allow to insert the dragged 
       * item as its child. 
       */
      if ([self isExpandable: [self itemAtRow: (row - 1)]])
        {
          maxInsertionLevel++;
        } 

      /* Find the insertion level to be used with a drop above
       *
       * In the outline below, when the pointer moves horizontally on 
       * the dashed line, it can insert at three levels: x level, C level or 
       * B/D level but not at A level.
       * 
       * + A
       *    + B
       *       + C
       *          - x
       * --- pointer ---
       *    + D 
       */
      if (pointerInsertionLevel < minInsertionLevel)
        {
          level = minInsertionLevel;
        }
      else if (pointerInsertionLevel > maxInsertionLevel)
        {
          level = maxInsertionLevel;
        }
      else
        {
          level = pointerInsertionLevel; 
        }

      //NSLog(@"min insert level = %d", minInsertionLevel);
      //NSLog(@"max insert level = %d", maxInsertionLevel);
      //NSLog(@"insert level = %d", level);
      //NSLog(@"row = %d and position in row = %d", row, positionInRow);

      if (positionInRow > 0 && positionInRow < 3) /* Drop on */
	{
	  /* We are directly over the middle of a row ... so the drop
	   * should be directory on the item in that row.
	   */
	  currentDropItem = [self itemAtRow: row];
	  currentDropIndex = NSOutlineViewDropOnItemIndex;
	}
      else /* Drop above */
	{
          NSInteger childIndex = 0;
          NSInteger parentRow = [self _parentRowForRow: row 
                                               atLevel: level 
                                   andReturnChildIndex: &childIndex];

	  //NSLog(@"found %d (proposed childIndex = %d)", parentRow, childIndex);

	  currentDropItem = (parentRow == -1 ? nil : [self itemAtRow: parentRow]);
          currentDropIndex = childIndex;
	}

      if ([_dataSource respondsToSelector:
	@selector(outlineView:validateDrop:proposedItem:proposedChildIndex:)])
        {
           dragOperation = [_dataSource outlineView: self
                                       validateDrop: sender
                                       proposedItem: currentDropItem
				 proposedChildIndex: currentDropIndex];
        }

      //NSLog(@"Drop on %@ %d", currentDropItem, currentDropIndex);

      if ((currentDropItem != oldDropItem)
	|| (currentDropIndex != oldDropIndex))
        {
          oldDropItem = currentDropItem;
          oldDropIndex = currentDropIndex;

	  ASSIGN(lastDragChange, lastDragUpdate);
          [self lockFocus];

          [self setNeedsDisplayInRect: oldDraggingRect];
          [self displayIfNeeded];

	  if (dragOperation != NSDragOperationNone)
	    {
	      if (currentDropIndex != NSOutlineViewDropOnItemIndex && currentDropItem != nil)
		{
		  [self drawDropAboveIndicatorWithDropItem: currentDropItem 
						     atRow: row
					    childDropIndex: currentDropIndex];
		}
	      else if (currentDropIndex == NSOutlineViewDropOnItemIndex && currentDropItem == nil)
		{
		  [self drawDropOnRootIndicator];
		}
	      else
		{
		  [self drawDropOnIndicatorWithDropItem: currentDropItem];
		}
	    }

          [_window flushWindow];
          [self unlockFocus];

        }
    }
  else if (row != _numberOfRows)
    {
      /* If we have been hovering over an item for more than half a second,
       * we should expand it.
       */
      if (lastDragChange != nil && [lastDragUpdate timeIntervalSinceDate: lastDragChange] >= 0.5)
	{
	  id item = [_items objectAtIndex: row];
	  if ([self isExpandable: item] && ![self isItemExpanded: item])
	    {
	      [self expandItem: item expandChildren: NO];
	      if ([self isItemExpanded: item])
		{
		  [autoExpanded addObject: item];
		}
	    }
	  /* Set the change date even if we didn't actually expand ... so
	   * we don't keep trying to expand the same item unnecessarily.
	   */
	  ASSIGN(lastDragChange, lastDragUpdate);
	}
    }

  return dragOperation;
}

- (BOOL) performDragOperation: (id<NSDraggingInfo>)sender
{
  BOOL	result = NO;

  if ([_dataSource
        respondsToSelector:
          @selector(outlineView:acceptDrop:item:childIndex:)])
    {
      result = [_dataSource outlineView: self
			     acceptDrop: sender
				   item: currentDropItem
			     childIndex: currentDropIndex];
    }

  [self _autoCollapse];

  return result;
}

- (BOOL) prepareForDragOperation: (id<NSDraggingInfo>)sender
{
  [self setNeedsDisplayInRect: oldDraggingRect];
  [self displayIfNeeded];

  return YES;
}

- (NSArray*) namesOfPromisedFilesDroppedAtDestination: (NSURL *)dropDestination
{
  if ([_dataSource respondsToSelector:
                    @selector(outlineView:namesOfPromisedFilesDroppedAtDestination:forDraggedItems:)])
    {
      NSUInteger count = [_selectedRows count];
      NSMutableArray *itemArray = [NSMutableArray arrayWithCapacity: count];
      NSUInteger index = [_selectedRows firstIndex];
      
      while (index != NSNotFound)
        {
          [itemArray addObject: [self itemAtRow: index]];
          index = [_selectedRows indexGreaterThanIndex: index];
        }

      return [_dataSource outlineView: self
                          namesOfPromisedFilesDroppedAtDestination: dropDestination
                          forDraggedItems: itemArray];
    }
  else
    {
      return nil;
    }
}

// Autosave methods...
- (void) setAutosaveName: (NSString *)name
{
  [super setAutosaveName: name];
  [self _autoloadExpandedItems];
}

- (void) editColumn: (NSInteger) columnIndex
                row: (NSInteger) rowIndex
          withEvent: (NSEvent *) theEvent
             select: (BOOL) flag
{
  NSText *t;
  NSTableColumn *tb;
  NSRect drawingRect;
  unsigned length = 0;

  // We refuse to edit cells if the delegate can not accept results
  // of editing.
  if (_dataSource_editable == NO)
    {
      flag = YES;
    }

   if (rowIndex != _selectedRow)
    {
      [NSException raise:NSInvalidArgumentException
	      format:@"Attempted to edit unselected row"];
    }

   if (rowIndex < 0 || rowIndex >= _numberOfRows
      || columnIndex < 0 || columnIndex >= _numberOfColumns)
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Row/column out of index in edit"];
    }

  [self scrollRowToVisible: rowIndex];
  [self scrollColumnToVisible: columnIndex];

  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }

  // Now (_textObject == nil)

  t = [_window fieldEditor: YES  forObject: self];

  if ([t superview] != nil)
    {
      if ([t resignFirstResponder] == NO)
        {
          return;
        }
    }

  _editedRow = rowIndex;
  _editedColumn = columnIndex;

  // Prepare the cell
  // NB: need to be released when no longer used
  _editedCell = [[self preparedCellAtColumn: columnIndex row: rowIndex] copy];

  [_editedCell setEditable: _dataSource_editable];
  tb = [_tableColumns objectAtIndex: columnIndex];
  [_editedCell setObjectValue: [self _objectValueForTableColumn: tb
                                     row: rowIndex]];

  // But of course the delegate can mess it up if it wants
  [self _willDisplayCell: _editedCell
        forTableColumn: tb
        row: rowIndex];

  /* Please note the important point - calling stringValue normally
     causes the _editedCell to call the validateEditing method of its
     control view ... which happens to be this object :-)
     but we don't want any spurious validateEditing to be performed
     before the actual editing is started (otherwise you easily end up
     with the table view picking up the string stored in the field
     editor, which is likely to be the string resulting from the last
     edit somewhere else ... getting into the bug that when you TAB
     from one cell to another one, the string is copied!), so we must
     call stringValue when _textObject is still nil.  */
  if (flag)
    {
      length = [[_editedCell stringValue] length];
    }

  _textObject = [_editedCell setUpFieldEditorAttributes: t];
  // FIXME: Which background color do we want here?
  [_textObject setBackgroundColor: [NSColor selectedControlColor]];
  [_textObject setDrawsBackground: YES];

  drawingRect = [self frameOfCellAtColumn: columnIndex  row: rowIndex];

  if (tb == [self outlineTableColumn])
    {
      id item = nil;
      NSImage *image = nil;
      NSCell *imageCell = nil;
      NSRect imageRect;
      NSInteger level = 0;
      CGFloat indentationFactor = 0.0;

      item = [self itemAtRow: rowIndex];
      // determine which image to use...
      if ([self isItemExpanded: item])
        {
          image = expanded;
        }
      else
        {
          image = collapsed;
        }

      if (![self isExpandable: item])
        {
          image = unexpandable;
        }

      level = [self levelForItem: item];
      indentationFactor = _indentationPerLevel * level;
      // create the image cell..
      imageCell = [[NSCell alloc] initImageCell: image];
      imageRect = [self frameOfOutlineCellAtRow: rowIndex];

      if ([_delegate respondsToSelector: @selector(outlineView:willDisplayOutlineCell:forTableColumn:item:)])
        {
          [_delegate outlineView: self
                     willDisplayOutlineCell: imageCell
                     forTableColumn: tb
                     item: item];
        }


      if ([imageCell image])
        {

          imageRect.size.width = [image size].width;
          imageRect.size.height = [image size].height;
          
          // draw...
          [self lockFocus];
          [imageCell drawWithFrame: imageRect inView: self];
          [self unlockFocus];
          
          // move the drawing rect over like in the drawRow routine...
          drawingRect.origin.x += indentationFactor + 5 + imageRect.size.width;
          drawingRect.size.width
            -= indentationFactor + 5 + imageRect.size.width;
        }
      else
        {
          // move the drawing rect over like in the drawRow routine...
          drawingRect.origin.x += indentationFactor;
          drawingRect.size.width -= indentationFactor;
        }

      RELEASE(imageCell);
    }

  if (flag)
    {
      [_editedCell selectWithFrame: drawingRect
                   inView: self
                   editor: _textObject
                   delegate: self
                   start: 0
                   length: length];
    }
  else
    {
      [_editedCell editWithFrame: drawingRect
                   inView: self
                   editor: _textObject
                   delegate: self
                   event: theEvent];
    }

  return;
}

@end /* implementation of NSOutlineView */

@implementation NSOutlineView (NotificationRequestMethods)
/*
 * (NotificationRequestMethods)
 */
- (void) _postSelectionIsChangingNotification
{
  [nc postNotificationName:
        NSOutlineViewSelectionIsChangingNotification
      object: self];
}
- (void) _postSelectionDidChangeNotification
{
  [nc postNotificationName:
        NSOutlineViewSelectionDidChangeNotification
      object: self];
}
- (void) _postColumnDidMoveNotificationWithOldIndex: (NSInteger) oldIndex
                                           newIndex: (NSInteger) newIndex
{
  [nc postNotificationName:
        NSOutlineViewColumnDidMoveNotification
      object: self
      userInfo: [NSDictionary
                  dictionaryWithObjectsAndKeys:
                  [NSNumber numberWithInteger: newIndex],
                  @"NSNewColumn",
                    [NSNumber numberWithInteger: oldIndex],
                  @"NSOldColumn",
                  nil]];
}

- (void) _postColumnDidResizeNotificationWithOldWidth: (float) oldWidth
{
  [nc postNotificationName:
        NSOutlineViewColumnDidResizeNotification
      object: self
      userInfo: [NSDictionary
                  dictionaryWithObjectsAndKeys:
                    [NSNumber numberWithFloat: oldWidth],
                  @"NSOldWidth",
                  nil]];
}

- (BOOL) _shouldSelectTableColumn: (NSTableColumn *)tableColumn
{
  if ([_delegate respondsToSelector:
    @selector (outlineView:shouldSelectTableColumn:)] == YES)
    {
      if ([_delegate outlineView: self  shouldSelectTableColumn: tableColumn]
        == NO)
        {
          return NO;
        }
    }

  return YES;
}

- (BOOL) _shouldSelectRow: (NSInteger)rowIndex
{
  id item = [self itemAtRow: rowIndex];

  if ([_delegate respondsToSelector:
    @selector (outlineView:shouldSelectItem:)] == YES)
    {
      if ([_delegate outlineView: self  shouldSelectItem: item] == NO)
        {
          return NO;
        }
    }

  return YES;
}

- (BOOL) _shouldSelectionChange
{
  if ([_delegate respondsToSelector:
    @selector (selectionShouldChangeInTableView:)] == YES)
    {
      if ([_delegate selectionShouldChangeInTableView: self] == NO)
        {
          return NO;
        }
    }

  return YES;
}

- (void) _didChangeSortDescriptors: (NSArray *)oldSortDescriptors
{
  if ([_dataSource 
	respondsToSelector: @selector(outlineView:sortDescriptorsDidChange:)])
    {
      [_dataSource outlineView: self
      sortDescriptorsDidChange: oldSortDescriptors];
    }
}

- (void) _didClickTableColumn: (NSTableColumn *)tc
{
  if ([_delegate 
	respondsToSelector: @selector(outlineView:didClickTableColumn:)])
    {
      [_delegate outlineView: self didClickTableColumn: tc];
    }
}

- (BOOL) _shouldEditTableColumn: (NSTableColumn *)tableColumn
                            row: (NSInteger) rowIndex
{
  if ([_delegate respondsToSelector:
    @selector(outlineView:shouldEditTableColumn:item:)])
    {
      id item = [self itemAtRow: rowIndex];

      if ([_delegate outlineView: self shouldEditTableColumn: tableColumn
                     item: item] == NO)
        {
          return NO;
        }
    }

  return YES;
}

- (void) _willDisplayCell: (NSCell*)cell
           forTableColumn: (NSTableColumn *)tb
                      row: (NSInteger)index
{
  if (_del_responds)
    {
      id item = [self itemAtRow: index];

      [_delegate outlineView: self
                 willDisplayCell: cell
                 forTableColumn: tb
                 item: item];
    }
}

- (BOOL) _writeRows: (NSIndexSet *)rows
       toPasteboard: (NSPasteboard *)pboard
{
  NSUInteger count = [rows count];
  NSMutableArray *itemArray = [NSMutableArray arrayWithCapacity: count];
  NSUInteger index = [rows firstIndex];

  while (index != NSNotFound)
    {
      [itemArray addObject: [self itemAtRow: index]];
      index = [rows indexGreaterThanIndex: index];
    }

  if ([_dataSource respondsToSelector:
                     @selector(outlineView:writeItems:toPasteboard:)] == YES)
    {
      return [_dataSource outlineView: self
                          writeItems: itemArray
                          toPasteboard: pboard];
    }
  return NO;
}

- (BOOL) _isDraggingSource
{
  return [_dataSource respondsToSelector:
                        @selector(outlineView:writeItems:toPasteboard:)];
}

- (id) _objectValueForTableColumn: (NSTableColumn *)tb
                              row: (NSInteger) index
{
  id result = nil;

  if ([_dataSource respondsToSelector:
    @selector(outlineView:objectValueForTableColumn:byItem:)])
    {
      id item = [self itemAtRow: index];

      result = [_dataSource outlineView: self
                            objectValueForTableColumn: tb
                            byItem: item];
    }

  return result;
}

- (void) _setObjectValue: (id)value
          forTableColumn: (NSTableColumn *)tb
                     row: (NSInteger) index
{
  if ([_dataSource respondsToSelector:
    @selector(outlineView:setObjectValue:forTableColumn:byItem:)])
    {
      id item = [self itemAtRow: index];

      [_dataSource outlineView: self
                   setObjectValue: value
                   forTableColumn: tb
                   byItem: item];
    }
}

- (NSInteger) _numRows
{
  return [_items count];
}

@end

@implementation NSOutlineView (TableViewInternalPrivate)

- (void) _initOutlineDefaults
{
  _itemDict = NSCreateMapTable(keyCallBacks,
                               NSObjectMapValueCallBacks,
                               64);
  _items = [[NSMutableArray alloc] init];
  _expandedItems = [[NSMutableArray alloc] init];
  _levelOfItems = NSCreateMapTable(keyCallBacks,
                                   NSObjectMapValueCallBacks,
                                   64);

  _indentationMarkerFollowsCell = YES;
  _autoResizesOutlineColumn = NO;
  _autosaveExpandedItems = NO;
  _indentationPerLevel = 10.0;
}

- (void) _autosaveExpandedItems
{
  if (_autosaveExpandedItems && _autosaveName != nil)
    {
      NSUserDefaults *defaults;
      NSString       *tableKey;

      defaults  = [NSUserDefaults standardUserDefaults];
      tableKey = [NSString stringWithFormat: @"NSOutlineView Expanded Items %@",
                           _autosaveName];
      [defaults setObject: _expandedItems  forKey: tableKey];
      [defaults synchronize];
    }
}

- (void) _autoloadExpandedItems
{
  if (_autosaveExpandedItems && _autosaveName != nil)
    {
      NSUserDefaults *defaults;
      id             config;
      NSString       *tableKey;

      defaults  = [NSUserDefaults standardUserDefaults];
      tableKey = [NSString stringWithFormat: @"NSOutlineView Expanded Items %@",
        _autosaveName];
      config = [defaults objectForKey: tableKey];
      if (config != nil)
        {
          NSEnumerator *en = [config objectEnumerator];
          id item = nil;

          while ((item = [en nextObject]) != nil)
            {
              [self expandItem: item];
            }
        }
    }
}

// Collect all of the items under a given element.
- (void)_collectItemsStartingWith: (id)startitem
                             into: (NSMutableArray *)allChildren
{
  NSUInteger num;
  NSUInteger i;
  id sitem = (startitem == nil) ? (id)[NSNull null] : (id)startitem;
  NSMutableArray *anarray;

  anarray = NSMapGet(_itemDict, sitem);
  num = [anarray count];
  for (i = 0; i < num; i++)
    {
      id anitem = [anarray objectAtIndex: i];

      // Only collect the children if the item is expanded
      if ([self isItemExpanded: startitem])
        {
          [allChildren addObject: anitem];
        }

      [self _collectItemsStartingWith: anitem
            into: allChildren];
    }
}

- (BOOL) _isItemLoaded: (id)item
{
  id sitem = (item == nil) ? (id)[NSNull null] : (id)item;
  id object = NSMapGet(_itemDict, sitem);

  // NOTE: We could store the loaded items in a map to ensure we only load 
  // the children of item when it gets expanded for the first time. This would
  // allow to write: return (NSMapGet(_loadedItemDict, sitem) != nil);
  // The last line isn't truly correct because it implies an item without 
  // children will get incorrectly reloaded automatically on each 
  // expand/collapse.
  return ([object count] != 0);
}

- (void) _loadDictionaryStartingWith: (id) startitem
                             atLevel: (NSInteger) level
{
  NSInteger num = 0;
  NSInteger i = 0;
  id sitem = (startitem == nil) ? (id)[NSNull null] : (id)startitem;
  NSMutableArray *anarray = nil;

  /* Check to see if item is expandable and expanded before getting the number 
   * of items. For macos compatibility the topmost item (startitem==nil)
   * is always considered expandable and must not be checked.
   * We must load the item only if expanded, otherwise an outline view is not 
   * usable with a big tree structure. For example, an outline view to browse 
   * file system would try to traverse every file/directory on -reloadData.
   */
  if ((startitem == nil
    || [_dataSource outlineView: self isItemExpandable: startitem])
    && [self isItemExpanded: startitem])
    {
      num = [_dataSource outlineView: self
			 numberOfChildrenOfItem: startitem];
    }

  if (num > 0)
    {
      anarray = [NSMutableArray array];
      NSMapInsert(_itemDict, sitem, anarray);
    }

  NSMapInsert(_levelOfItems, sitem, [NSNumber numberWithInteger: level]);

  for (i = 0; i < num; i++)
    {
      id anitem = [_dataSource outlineView: self
                               child: i
                               ofItem: startitem];

      [anarray addObject: anitem];
      [self _loadDictionaryStartingWith: anitem
            atLevel: level + 1];
    }
}

- (void)_closeItem: (id)item
{
  NSUInteger i, numChildren;
  NSMutableArray *removeAll = [NSMutableArray array];

  [self _collectItemsStartingWith: item into: removeAll];
  numChildren = [removeAll count];

  // close the item...
  if (item != nil)
    {
      [_expandedItems removeObjectIdenticalTo: item];
    }

  // For the close method it doesn't matter what order they are
  // removed in.
  for (i = 0; i < numChildren; i++)
    {
      id child = [removeAll objectAtIndex: i];
      [_items removeObjectIdenticalTo: child];
    }
  [self _noteNumberOfRowsChangedBelowItem: item by: -numChildren];
}

- (void)_openItem: (id)item
{
  NSUInteger insertionPoint, numChildren, numDescendants;
  NSInteger i;
  id object;
  id sitem = (item == nil) ? (id)[NSNull null] : (id)item;

  // open the item...
  if (item != nil)
    {
      [_expandedItems addObject: item];
    }

  // Load the children of the item if needed
  if ([self _isItemLoaded: item] == NO)
    {
      [self _loadDictionaryStartingWith: item
                                atLevel: [self levelForItem: item]];
    }

  object = NSMapGet(_itemDict, sitem);
  numChildren = numDescendants = [object count];

  insertionPoint = [_items indexOfObjectIdenticalTo: item];
  if (insertionPoint == NSNotFound)
    {
      insertionPoint = 0;
    }
  else
    {
      insertionPoint++;
    }

  for (i = numChildren-1; i >= 0; i--)
    {
      id obj = NSMapGet(_itemDict, sitem);
      id child = [obj objectAtIndex: i];

      // Add all of the children...
      if ([self isItemExpanded: child])
        {
          NSUInteger numItems;
          NSInteger j;
          NSMutableArray *insertAll = [NSMutableArray array];

          [self _collectItemsStartingWith: child into: insertAll];
          numItems = [insertAll count];
          numDescendants += numItems;
          for (j = numItems-1; j >= 0; j--)
            {
              [_items insertObject: [insertAll objectAtIndex: j]
                      atIndex: insertionPoint];
            }
        }

      // Add the parent
      [_items insertObject: child atIndex: insertionPoint];
    }

  [self _noteNumberOfRowsChangedBelowItem: item by: numDescendants];
}

- (void) _removeChildren: (id)startitem
{
  NSUInteger i, numChildren;
  id sitem = (startitem == nil) ? (id)[NSNull null] : (id)startitem;
  NSMutableArray *anarray;

  anarray = NSMapGet(_itemDict, sitem);
  numChildren = [anarray count];
  for (i = 0; i < numChildren; i++)
    {
      id child = [anarray objectAtIndex: i];

      [self _removeChildren: child];
      NSMapRemove(_itemDict, child);
      [_items removeObjectIdenticalTo: child];
      [_expandedItems removeObjectIdenticalTo: child];
    }
  [anarray removeAllObjects];
  [self _noteNumberOfRowsChangedBelowItem: startitem by: -numChildren];
}

- (void) _noteNumberOfRowsChangedBelowItem: (id)item by: (NSInteger)numItems
{
  BOOL selectionDidChange = NO;
  NSUInteger rowIndex, nextIndex;

  // check for trivial case
  if (numItems == 0)
    return;

  // if a row below item is selected, update the selected row indexes
  /* Note: We update the selected row indexes directly instead of calling
   * -selectRowIndexes:extendingSelection: to avoid posting bogus selection
   * did change notifications. */
  rowIndex = [_items indexOfObjectIdenticalTo: item];
  rowIndex = (rowIndex == NSNotFound) ? 0 : rowIndex + 1;
  nextIndex = [_selectedRows indexGreaterThanOrEqualToIndex: rowIndex];
  if (nextIndex != NSNotFound)
    {
      if (numItems > 0)
	{
	  [_selectedRows shiftIndexesStartingAtIndex: rowIndex by: numItems];
	  if (_selectedRow >= rowIndex)
	    {
	      _selectedRow += numItems;
	    }
	}
      else
        {
	  numItems = -numItems;
	  [_selectedRows shiftIndexesStartingAtIndex: rowIndex + numItems
						  by: -numItems];
	  if (nextIndex < rowIndex + numItems)
	    {
	      /* Don't post the notification here, as the table view is in
	       * an inconsistent state. */
	      selectionDidChange = YES;
	    }

	  /* If the selection becomes empty after removing items and the
	   * receiver does not allow empty selections, select the root item. */
          if ([_selectedRows firstIndex] == NSNotFound &&
              [self allowsEmptySelection] == NO)
            {
              [_selectedRows addIndex: 0];
            }

	  if (_selectedRow >= rowIndex + numItems)
	    {
	      _selectedRow -= numItems;
	    }
	  else if (_selectedRow >= rowIndex)
	    {
	      /* If the item at _selectedRow was removed, we arbitrarily choose
	       * another selected item (if there is still any). The policy
	       * implemented below chooses the index most close to item. */
	      NSUInteger r1 = [_selectedRows indexLessThanIndex: rowIndex];
	      NSUInteger r2 = [_selectedRows indexGreaterThanOrEqualToIndex: rowIndex];
	      if (r1 != NSNotFound && r2 != NSNotFound)
		{
		  _selectedRow = (rowIndex - r1) <= (r2 - rowIndex) ? r1 : r2;
		}
	      else if (r1 != NSNotFound)
		{
		  _selectedRow = r1;
		}
	      else if (r2 != NSNotFound)
		{
		  _selectedRow = r2;
		}
	      else
		{
		  _selectedRow = -1;
		}
	    }
        }
    }

  [self noteNumberOfRowsChanged];
  if (selectionDidChange)
    {
      [self _postSelectionDidChangeNotification];
    }
}

- (NSCell *) preparedCellAtColumn: (NSInteger)columnIndex row: (NSInteger)rowIndex
{
  NSCell *cell = nil;
  NSTableColumn *tb = [_tableColumns objectAtIndex: columnIndex];

  if ([_delegate respondsToSelector:
        @selector(outlineView:dataCellForTableColumn:item:)])
    {
      id item = [self itemAtRow: rowIndex];
      cell = [_delegate outlineView: self dataCellForTableColumn: tb
                                                            item: item];
    }
  if (cell == nil)
    {
      cell = [tb dataCellForRow: rowIndex];
    }
  return cell;
}

@end

@implementation	NSOutlineView (Private)
/* Collapse all the items which were automatically expanded to allow drop.
 */
- (void) _autoCollapse
{
  NSEnumerator	*e;
  id		item;

  e = [autoExpanded objectEnumerator];
  while ((item = [e nextObject]) != nil)
    {
      [self collapseItem: item collapseChildren: YES];
    }
  [autoExpanded removeAllObjects];
}
@end
