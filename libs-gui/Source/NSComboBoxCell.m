/** <title>NSComboBoxCell</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Gerrit van Dyk <gerritvd@decillion.net>
   Date: 1999
   Author:  Quentin Mathe <qmathe@club-internet.fr>
   Date: 2004

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

#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSException.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSBox.h"
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSComboBox.h"
#import "AppKit/NSComboBoxCell.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSTextView.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSGuiPrivate.h"

static NSNotificationCenter *nc;

@interface GSComboBoxTableView : NSTableView
{
}

@end

@implementation GSComboBoxTableView
- (BOOL) acceptsFirstMouse: (NSEvent *)event
{
  return YES;
}
@end

@interface GSComboWindow : NSPanel
{
   NSBrowser *_browser;
   GSComboBoxTableView *_tableView;
   NSComboBoxCell *_cell;
   BOOL _stopped;
}

+ (GSComboWindow *) defaultPopUp;

- (void) layoutWithComboBoxCell:(NSComboBoxCell *)comboBoxCell;
- (void) positionWithComboBoxCell:(NSComboBoxCell *)comboBoxCell;
- (void) popUpForComboBoxCell: (NSComboBoxCell *)comboBoxCell;
- (void) runModalPopUpWithComboBoxCell:(NSComboBoxCell *)comboBoxCell;
- (void) runLoopWithComboBoxCell:(NSComboBoxCell *)comboBoxCell;
- (void) onWindowEdited: (NSNotification *)notification;
- (void) clickItem: (id)sender;
- (void) reloadData;
- (void) noteNumberOfItemsChanged;
- (void) scrollItemAtIndexToTop: (NSInteger)index;
- (void) scrollItemAtIndexToVisible: (NSInteger)index;
- (void) selectItemAtIndex: (NSInteger)index;
- (void) deselectItemAtIndex: (NSInteger)index;
- (void) moveUpSelection;
- (void) moveDownSelection;
- (void) validateSelection;

@end

@interface NSComboBoxCell (GNUstepPrivate)
- (NSString *) _stringValueAtIndex: (NSInteger)index;
- (void) _performClickWithFrame: (NSRect)cellFrame inView: (NSView *)controlView;
- (void) _didClickWithinButton: (id)sender;
- (BOOL) _isWantedEvent: (NSEvent *)event;
- (GSComboWindow *) _popUp;
- (NSRect) _textCellFrame;
- (void) _setSelectedItem: (NSInteger)index;
- (void) _loadButtonCell;
- (void) _selectCompleted;
@end

// ---

static GSComboWindow *gsWindow = nil;

@implementation GSComboWindow

+ (GSComboWindow *) defaultPopUp
{
  if (gsWindow == nil)
    gsWindow = [[self alloc] initWithContentRect: NSMakeRect(0,0,200,200)
			               styleMask: NSBorderlessWindowMask
			                 backing: NSBackingStoreNonretained // NSBackingStoreBuffered
			                   defer: YES];
  return gsWindow;
}

- (id) initWithContentRect: (NSRect)contentRect
		 styleMask: (NSUInteger)aStyle
		   backing: (NSBackingStoreType)bufferingType
		     defer: (BOOL)flag
{
  NSBox *box;
  NSRect borderRect;
  NSScrollView *scrollView;
  NSTableColumn *column;
  NSCell *cell;
   
  self = [super initWithContentRect: contentRect
		          styleMask: aStyle
		            backing: bufferingType
		              defer: flag];	  
  if (nil == self)
    return self;

  [self setLevel: NSPopUpMenuWindowLevel];
  [self setBecomesKeyOnlyIfNeeded: YES];
  
  box = [[NSBox alloc] initWithFrame: contentRect];
  [box setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [box setBorderType: NSLineBorder];
  [box setTitlePosition: NSNoTitle];
  [box setContentViewMargins: NSMakeSize(0, 0)];
  [self setContentView: box];
  borderRect = contentRect;
  RELEASE(box);
  
  _tableView = [[GSComboBoxTableView alloc] 
                     initWithFrame: NSMakeRect(0, 0, 100, 100)];
  [_tableView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  //[_tableView setBackgroundColor: [NSColor whiteColor]];
  [_tableView setDrawsGrid: NO];
  [_tableView setAllowsEmptySelection: YES];
  [_tableView setAllowsMultipleSelection: NO];
  [_tableView setAutoresizesAllColumnsToFit: YES];
  [_tableView setHeaderView: nil];
  [_tableView setCornerView: nil];
  
  column = [[NSTableColumn alloc] initWithIdentifier: @"content"];
  cell = [[NSCell alloc] initTextCell: @""];
  [column setDataCell: cell];
  RELEASE(cell);
  [_tableView addTableColumn: column];
  RELEASE(column);
  
  [_tableView setDataSource: self];
  [_tableView setDelegate: self];
  [_tableView setAction: @selector(clickItem:)];
  [_tableView setTarget: self];
  
  scrollView = [[NSScrollView alloc] initWithFrame: NSMakeRect(borderRect.origin.x, 
                                                               borderRect.origin.y,
                                                               borderRect.size.width, 
                                                               borderRect.size.height)];
  [scrollView setHasVerticalScroller: YES];
  [scrollView setDocumentView: _tableView];
  [_tableView release];
  [box setContentView: scrollView];
  RELEASE(scrollView);
  
  [_tableView reloadData];

  return self;
}

- (BOOL) canBecomeKeyWindow 
{ 
  return YES; 
}

- (void)dealloc
{
  // Browser, table view and scroll view were not retained so don't release them
  [super dealloc];
}

- (void) layoutWithComboBoxCell: (NSComboBoxCell *)comboBoxCell
{
  NSSize bsize = [[GSTheme theme] sizeForBorderType: NSLineBorder];
  NSSize size;
  CGFloat itemHeight;
  CGFloat textCellWidth;
  CGFloat popUpWidth;
  NSSize intercellSpacing;
  NSInteger num = [comboBoxCell numberOfItems];
  NSInteger max = [comboBoxCell numberOfVisibleItems];
  
  // Manage table view or browser cells height
  
  itemHeight = [comboBoxCell itemHeight];
  if (itemHeight <= 0)
    {
      // FIX ME : raise NSException
      itemHeight = [_tableView rowHeight];
    }
  size.height = itemHeight;
    
  // Manage table view or browser cells width
  
  textCellWidth = [comboBoxCell _textCellFrame].size.width;
  if ([comboBoxCell hasVerticalScroller])
    {
      popUpWidth = textCellWidth + [NSScroller scrollerWidth];
    }
  else 
    {
      popUpWidth = textCellWidth;
    }
  size.width = textCellWidth - bsize.width;
    
  if (size.width < 0)
    {
      size.width = 0;
    }
   
  [_tableView setRowHeight: size.height];
    
  // Just check intercell spacing

  intercellSpacing = [comboBoxCell intercellSpacing];
  if (intercellSpacing.height <= 0)
    {
      // FIX ME : raise NSException
      intercellSpacing.height = [_tableView intercellSpacing].height;
    }
  else 
    {
      [_tableView setIntercellSpacing: intercellSpacing];
    }
    
    
  if (num > max)
    num = max;
   
  [self setFrame: [self frameRectForContentRect: NSMakeRect(0, 0, popUpWidth, 
     2 * bsize.height + (itemHeight + intercellSpacing.height) * (num - 1)
     + itemHeight)] display: NO];
}

- (void) positionWithComboBoxCell: (NSComboBoxCell *)comboBoxCell
{
  NSView *viewWithComboCell = [comboBoxCell controlView];
  NSRect screenFrame;
  NSRect comboWindowFrame;
  NSRect viewWithComboCellFrame;
  NSRect rect;
  NSPoint point, oldPoint;
   
  [self layoutWithComboBoxCell: comboBoxCell];
  
  // Now we can ask for the size
  comboWindowFrame = [self frame];
  if (comboWindowFrame.size.width == 0 || comboWindowFrame.size.height == 0)
    return;
  
  screenFrame = [[[viewWithComboCell window] screen] frame];
  viewWithComboCellFrame = [comboBoxCell _textCellFrame];
  if ([viewWithComboCell isFlipped])
  {
      point = viewWithComboCellFrame.origin;
      point.y = NSMaxY(viewWithComboCellFrame);
  }
  else
  {
      point = viewWithComboCellFrame.origin;
  }

  // Switch to the window coordinates
  point = [viewWithComboCell convertPoint: point toView: nil];
  
  // Switch to the screen coordinates
  point = [[viewWithComboCell window] convertBaseToScreen: point];
  point.y -= 1 + NSHeight(comboWindowFrame);
  
  if (point.y < 0)
    {
      // Off screen, so move it
      oldPoint = point;
      
      point = viewWithComboCellFrame.origin;
      point.y = NSMaxY(viewWithComboCellFrame);
      
      // Switch to the window coordinates  
      point = [viewWithComboCell convertPoint: point toView: nil];
  
      // Switch to the screen coordiantes
      point = [[viewWithComboCell window] convertBaseToScreen: point];
      point.y += 1;  
      
      if (point.y + NSHeight(comboWindowFrame) > NSHeight(screenFrame))
	  point = oldPoint;
    }

  rect.size.width = NSWidth(comboWindowFrame);
  rect.size.height = NSHeight(comboWindowFrame);
  rect.origin.x = point.x;
  rect.origin.y = point.y;
  [self setFrame: rect display: NO];
}

- (void) popUpForComboBoxCell: (NSComboBoxCell *)comboBoxCell
{
  _cell = comboBoxCell;
   
  [self positionWithComboBoxCell: _cell];
  [_cell _selectCompleted];
  [self reloadData];
  [self enableKeyEquivalentForDefaultButtonCell];
  [self runModalPopUpWithComboBoxCell: _cell];

  _cell = nil;
  [self deselectItemAtIndex: 0];
}

- (void) runModalPopUpWithComboBoxCell: (NSComboBoxCell *)comboBoxCell
{
  NSWindow	*onWindow;
  
  onWindow = [[_cell controlView] window];

  [nc addObserver: self selector: @selector(onWindowEdited:) 
    name: NSWindowWillCloseNotification object: onWindow];
  [nc addObserver: self selector: @selector(onWindowEdited:) 
    name: NSWindowWillMoveNotification object: onWindow];
  [nc addObserver: self selector: @selector(onWindowEdited:) 
    name: NSWindowWillMiniaturizeNotification object: onWindow];
    
  // FIX ME: The notification below doesn't exist currently
  // [nc addObserver: self selector: @selector(onWindowEdited:) 
  //   name: NSWindowWillResizeNotification object: onWindow];
   
  
  // FIXME: The code below must be removed when the notifications over will work
  [nc addObserver: self selector: @selector(onWindowEdited:) 
    name: NSWindowDidMoveNotification object: onWindow];
  [nc addObserver: self selector: @selector(onWindowEdited:) 
    name: NSWindowDidMiniaturizeNotification object: onWindow];
  [nc addObserver: self selector: @selector(onWindowEdited:) 
    name: NSWindowDidResizeNotification object: onWindow];
  // End of the code to remove
  
  [self orderFront: self];
  [self makeFirstResponder: _tableView];
  [self runLoopWithComboBoxCell: comboBoxCell];
  
  [nc removeObserver: self name: nil object: onWindow];
  
  [self close];

  [onWindow makeFirstResponder: [_cell controlView]];
}

- (void) runLoopWithComboBoxCell: (NSComboBoxCell *)comboBoxCell
{
  NSEvent *event;
  NSDate *limit = [NSDate distantFuture];
  unichar key;
  CREATE_AUTORELEASE_POOL (pool);

  while (YES)
    {
      event = [NSApp nextEventMatchingMask: NSAnyEventMask
		                 untilDate: limit
		                    inMode: NSDefaultRunLoopMode
		                   dequeue: YES];
      if ([event type] == NSLeftMouseDown
       || [event type] == NSRightMouseDown)
        {		   
          if ([comboBoxCell _isWantedEvent: event] == NO && [event window] != self)
	    {
              break;
	    }
	  else
	    {
	      [NSApp sendEvent: event];
	    }
        }
      else if ([event type] == NSKeyDown)
        {
	  key = [[event characters] characterAtIndex: 0];
          if (key == NSUpArrowFunctionKey)
            {
              [self moveUpSelection]; 
            }
          else if (key == NSDownArrowFunctionKey)
            {
              [self moveDownSelection];
            }
          else if (key == NSNewlineCharacter 
	    || key == NSEnterCharacter 
	    || key == NSCarriageReturnCharacter)
            {
              [self validateSelection];
            }
	  else if (key == 0x001b)
	    {
	      break;
	    }
          else
            {
	      [NSApp sendEvent: event];
	    }
	}
      else
        {
	  [NSApp sendEvent: event];
	}
      
      if (_stopped)
        break;      
    }
    
  _stopped = NO;
    
  [pool drain];
}

// onWindow notifications

- (void) onWindowEdited: (NSNotification *)notification
{
  _stopped = YES;
}

- (void) reloadData
{
  [_tableView reloadData];
  [self selectItemAtIndex: [_cell indexOfSelectedItem]];
}

- (void) noteNumberOfItemsChanged
{
  // FIXME: Probably should only load the additional items
  [self reloadData];
}

- (void) scrollItemAtIndexToTop: (NSInteger)index
{
  NSRect rect;
  
  rect = [_tableView frameOfCellAtColumn: 0 row: index];
  [_tableView scrollPoint: rect.origin]; 
}

- (void) scrollItemAtIndexToVisible: (NSInteger)index
{
  [_tableView scrollRowToVisible: index];
}

- (void) selectItemAtIndex: (NSInteger)index
{ 
  if (index < 0)
    return;
  
  if ([_tableView selectedRow] == index || [_tableView numberOfRows] <= index)
    return;
  
  [_tableView selectRow: index byExtendingSelection: NO];     
}

- (void) deselectItemAtIndex: (NSInteger)index
{
  [_tableView deselectAll: self];
}

// Target/Action method
- (void) clickItem: (id)sender
{  
  if (_cell == nil)
    return;
  
  [_cell _setSelectedItem: [sender selectedRow]];
  [self validateSelection];
  
  [nc postNotificationName: NSComboBoxSelectionDidChangeNotification
	            object: [_cell controlView]
	          userInfo: nil];
}

// Browser delegate methods
- (NSInteger) browser: (NSBrowser *)sender numberOfRowsInColumn: (NSInteger)column
{
  if (_cell == nil)
    return 0;

  return [_cell numberOfItems];
}

- (void) browser: (NSBrowser *)sender 
 willDisplayCell: (id)aCell
	   atRow: (NSInteger)row 
	  column: (NSInteger)column
{
  if (_cell == nil)
    return;

  [aCell setStringValue: [_cell _stringValueAtIndex: row]];
  [aCell setLeaf: YES];
}

// Table view data source methods
- (NSInteger) numberOfRowsInTableView: (NSTableView *)tv
{
  return [_cell numberOfItems];
}

- (id) tableView: (NSTableView *)tv objectValueForTableColumn: (NSTableColumn *)tc row: (NSInteger)row
{
  return [_cell _stringValueAtIndex: row];
}

// Table view delegate methods
- (void) tableViewSelectionDidChange: (NSNotification *)notification
{
  [_cell _setSelectedItem: [[notification object] selectedRow]];
  
  [nc postNotificationName: NSComboBoxSelectionDidChangeNotification
	            object: [_cell controlView]
	          userInfo: nil];
}

// Key actions methods
- (void) moveUpSelection
{
  NSInteger index = [_tableView selectedRow] - 1;

  if (index > -1 && index < [_tableView numberOfRows])
    {
      [_tableView selectRow: index byExtendingSelection: NO];
      [_tableView scrollRowToVisible: index];
    }
}

- (void) moveDownSelection
{
  NSInteger index = [_tableView selectedRow] + 1;
  
  if (index > -1 && index < [_tableView numberOfRows])
    {
      [_tableView selectRow: index byExtendingSelection: NO];
      [_tableView scrollRowToVisible: index];
    }
}

- (void) validateSelection
{
  if (_cell != nil)
    {
      NSText	*textObject = nil;
      id	cv = [_cell controlView];
      NSInteger	index = [_cell indexOfSelectedItem];
      
      if ([cv isKindOfClass: [NSControl class]])
        {
	  textObject = [(NSControl *)cv currentEditor];
	}
      
      if (index != -1)
        {
          [_cell setStringValue: [_cell _stringValueAtIndex: 
            [_cell indexOfSelectedItem]]]; 
          // Will update the editor when needed
        }
      
      if  (textObject != nil)
        {
	  NSRange selectionRange = NSMakeRange(0, [[textObject string] length]);
	  [textObject setSelectedRange: selectionRange];
	  [textObject scrollRangeToVisible: selectionRange];
	}	
      
      [cv sendAction: [_cell action] to: [_cell target]];
      
      _stopped = YES;
    }
}

@end

// ---

/**
 <unit>
 <heading>Class Description</heading> 
 <p>An NSComboBoxCell is what we can call a completion/choices box cell, derived from
 NSTextFieldCell, it allows you to enter text like in a text field but also to click
 in the ellipsis button (indicating the fact other user inputs are possible) on
 the right of it to obtain a list of choices, you can use them as the text field
 value by selecting a row in this list. You can also obtain direct completion
 when it  is enabled via <code>setCompletes:</code> to get a suggested text
 field value updated as you type. </p>
 <p>Like other NSCell classes, NSComboBoxCell has a matching NSControl named NSComboBox
 which is relying on it to implement the combo box behavior in a standalone
 control.</p>
 </unit>
*/ 

/**
 * <p>No special instructions to use NSComboBoxCell or text to detail the implementation.</p>
 */
@implementation NSComboBoxCell

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSComboBoxCell class])
    {
      [NSComboBoxCell setVersion: 2];
      nc = [NSNotificationCenter defaultCenter];
    }
}

- (id) initTextCell: (NSString *)aString
{
  self = [super initTextCell: aString];

  // Implicitly set by allocation:
  //
  //_dataSource = nil;
  //_buttonCell = nil;
  //_usesDataSource = NO;
  //_completes = NO;
  _popUpList = [[NSMutableArray alloc] init];
  _hasVerticalScroller = YES;
  _visibleItems = 10;
  _intercellSpacing = NSMakeSize(3.0, 2.0);
  _itemHeight = 16;
  _selectedItem = -1;
  
  [self _loadButtonCell];

  return self;
}

- (void) dealloc
{
  RELEASE(_buttonCell);
  RELEASE(_popUpList);
  
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  NSComboBoxCell *c = [super copyWithZone: zone];

  c->_buttonCell = [_buttonCell copyWithZone: zone];
  [c->_buttonCell setTarget: c];
  c->_popUpList = [_popUpList copyWithZone: zone];

  return c;
}

/**
 * Returns YES when the combo box cell displays a vertical scroller for its
 * list, returns NO otherwise. 
 * Take note that the scroller will be displayed even when the sum of the items
 * height in the list is inferior to the minimal height of the list displayed
 * area. 
 */
- (BOOL) hasVerticalScroller 
{ 
  return _hasVerticalScroller; 
}

/**
 * Sets whether the combo box cell list displays a vertical scroller, by default
 * it is the case. When <var>flag</var> is NO and the combo cell list has more
 * items (either in its default list or from its data source) than the number
 * returned by <code>numberOfVisibleItems</code>, only a subset of them will be
 * displayed. Uses scroll related methods to position this subset in the combo
 * box cell list.
 * Take note that the scroller will be displayed even when the sum of the items
 * height in the list is inferior to the minimal height of the list displayed
 * area.
 */
- (void) setHasVerticalScroller: (BOOL)flag
{
  _hasVerticalScroller = flag;
}

/**
 * Returns the width and the height (as the values of an NSSize variable)
 * between each item of the combo box cell list.
 */
- (NSSize) intercellSpacing
{ 
  return _intercellSpacing; 
}

/**
 * Sets the width and the height between each item of the combo box cell list to
 * the values in <var>aSize</var>.
 */
- (void) setIntercellSpacing: (NSSize)aSize
{
  _intercellSpacing = aSize;
}

/**
 * Returns the height of the items in the combo box cell list.
 */
- (CGFloat) itemHeight 
{ 
  return _itemHeight; 
}

/**
 * Sets the height of the items in the combo box cell list to
 * <var>itemHeight</var>.
 */
- (void) setItemHeight: (CGFloat)itemHeight
{
  if (itemHeight > 14.0)
    _itemHeight = itemHeight;
}

/**
 * Returns the maximum number of allowed items to be displayed in the combo box
 * cell list.
 */
- (NSInteger) numberOfVisibleItems 
{ 
  return _visibleItems; 
}

/**
 * Sets the maximum number of allowed items to be displayed in the combo box
 * cell list.
 */
- (void) setNumberOfVisibleItems: (NSInteger)visibleItems
{
  if (visibleItems > 10)
    _visibleItems = visibleItems;
}

/** 
 * Marks the combo box cell in order to have its items list reloaded in the
 * case it uses a data source, and to have it redisplayed.
 */
- (void) reloadData
{
  [_popup reloadData];
}

/**
 * Informs the combo box cell that the number of items in its data source has
 * changed, in order to permit to the scrollers in its displayed list being
 * updated without needing the reload of the data. 
 * It is recommended to use this method with a data source that continually
 * receives data in the background, to keep the the combo box cell responsive to
 * the user while the data is received.
 * Take a look at the <code>NSComboBoxDataSource</code> informal protocol
 * specification to know more on the messages NSComboBox sends to its data
 * source. 
 */
- (void) noteNumberOfItemsChanged
{
  [_popup noteNumberOfItemsChanged];
}

/**
 * Returns YES when the combo box cell uses a data source (which is external) to
 * populate its items list, otherwise returns NO in the case it uses its default
 * list.
 */
- (BOOL) usesDataSource 
{ 
  return _usesDataSource; 
}

/**
 * Sets according to <var>flag</var> whether the combo box cell uses a data
 * source (which is external) to populate its items list.
 */
- (void) setUsesDataSource: (BOOL)flag
{
  _usesDataSource = flag;
}

/**
 * Scrolls the combo box cell list vertically in order to have the item at
 * <var>index</var> in the closest position relative to the top. There is no
 * need to have the list displayed when this method is invoked.
 */
- (void) scrollItemAtIndexToTop: (NSInteger)index
{
  [_popup scrollItemAtIndexToTop: index];
}

/**
 * Scrolls the combo box cell list vertically in order to have the item at
 * <var>index</var> visible. There is no need to have the list displayed when
 * this method is invoked. 
 */
- (void) scrollItemAtIndexToVisible: (NSInteger)index
{
  [_popup scrollItemAtIndexToVisible: index];
}

/**
 * Selects the combo box cell list row at <var>index</var>. 
 * Take note no changes occurs in the combo box cell list when this method is
 * called. 
 * Posts an NSComboBoxSelectionDidChangeNotification to the default notification
 * center when there is a new selection different from the previous one.
 */
- (void) selectItemAtIndex: (NSInteger)index
{
  // Method called by GSComboWindow when a selection is done in the table view or 
  // the browser
  
  if (index < 0 || [self numberOfItems] <= index)
    return; // FIXME: Probably we should raise an exception

  if (_selectedItem != index)
    {
      [self _setSelectedItem: index];
      
      [_popup selectItemAtIndex: index]; 
      // This method call will not create a infinite loop when the index has been 
      // already set by a mouse click because the method is not completed when the 
      // current index is not different from the index parameter

      [nc postNotificationName: NSComboBoxSelectionDidChangeNotification
	                object: [self controlView]
	              userInfo: nil];
    }
}

/**
 * Deselects the combo box cell list row at <var>index</var> in the case this
 * row is selected. 
 * Posts an NSComboBoxSelectionDidChangeNotification to the default notification
 * center, when there is a new selection.
 */
- (void) deselectItemAtIndex: (NSInteger)index
{
  if (_selectedItem == index)
    {
      [self _setSelectedItem: -1];

      [_popup deselectItemAtIndex: index];

      [nc postNotificationName: NSComboBoxSelectionDidChangeNotification
  	                object: [self controlView]
	              userInfo: nil];
    }
}

/**
 * Returns the index of the selected item in the combo box cell list or -1 when
 * there is no selection, the selected item can be related to the data source
 * object in the case <code>usesDataSource</code> returns YES else to the
 * default items list. 
 */
- (NSInteger) indexOfSelectedItem
{
  return _selectedItem;
}

/**
 * Returns the number of items in the the combo box cell list, the numbers of
 * items can be be related to the data source object in the case
 * <code>usesDataSource</code> returns YES else to the default items list.
 */
- (NSInteger) numberOfItems
{
  if (_usesDataSource)
    {
      if (_dataSource == nil)
        {
          NSLog(@"%@: No data source currently specified", self);
        }
      else if ([_dataSource respondsToSelector: 
                 @selector(numberOfItemsInComboBox:)])
        {
          return [_dataSource numberOfItemsInComboBox: 
            (NSComboBox *)[self controlView]];
        }
      else if ([_dataSource respondsToSelector: 
                  @selector(numberOfItemsInComboBoxCell:)])
        {
          return [_dataSource numberOfItemsInComboBoxCell: self];
        }
    }
  else
    {
      return [_popUpList count];
    }
     
  return 0;
}

/**
 * Returns the combo box cell data source object which is reponsible to provide
 * the data to be displayed. To know how to implement a data source object,
 * take a  look at the NSComboBoxDataSource informal protocol description. In
 * the case <code>usesDataSource</code> returns NO, this method logs a warning.
 */
- (id) dataSource 
{ 
  return _dataSource; 
}

/**
 * Sets the combo box cell data source to <var>aSource</var>. Just calling this
 * method doesn't set <code>usesDataSource</code> to return YES, you must call
 * <code>setUsesDataSource:</code> with YES before or a warning will be logged.
 * To know how to implement a data source objects, take a  look at the
 * NSComboBoxDataSource informal protocol description. When <var>aSource</var>
 * doesn't respond to the methods <code>numberOfItemsInComboBox:</code>
 * <code>comboBox:objectValueForItemAtIndex:</code>, this method
 * logs a warning.
 */
- (void) setDataSource: (id)aSource
{
  if (_usesDataSource == NO)
    {
      NSLog(@"%@: This method is invalid, this combo box is not set to use a data source", 
        self);
    }
  else
    {
      _dataSource = aSource;
    }
}

/**
 * Adds an item to the combo box cell default items list which is used when
 * <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void) addItemWithObjectValue: (id)object
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
    }
  else
    {
      [_popUpList addObject: object];
    }
    
  [self reloadData];
}

/**
 * Adds several items in an array to the combo box cell default items list which
 * is used when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void) addItemsWithObjectValues: (NSArray *)objects
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
    }
  else
    {
      [_popUpList addObjectsFromArray: objects];
    }
    
  [self reloadData];
}

/**
 * Inserts an item in the combo box cell default items list which
 * is used when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void) insertItemWithObjectValue: (id)object atIndex: (NSInteger)index
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
    }
  else
    {
      [_popUpList insertObject: object atIndex: index];
    }
    
  [self reloadData];
}

/**
 * Removes an item in the combo box cell default items list which
 * is used when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void) removeItemWithObjectValue: (id)object
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
    }
  else
    {
      [_popUpList removeObject: object];
    }
    
  [self reloadData];
}

/**
 * Removes the item with the specified <var>index</var> in the combo box cell
 * default items list which is used when <code>usesDataSource</code> returns NO.
 * In the case <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void) removeItemAtIndex: (NSInteger)index
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
    }
  else
    {
      [_popUpList removeObjectAtIndex: index];
    }
    
  [self reloadData];
}

/**
 * Removes all the items in the combo box cell default items list which is used
 * when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void) removeAllItems
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
    }
  else
    {
      [_popUpList removeAllObjects];
    }
    
  [self reloadData];
}

/**
 * Selects the first item in the default combo box cell list which is equal to
 * <var>object</var>. In the case <code>usesDataSource</code> returns YES, this
 * method logs a warning. 
 * Take note that this method doesn't update the text field part value. 
 * Posts an NSComboBoxSelectionDidChange notification to the default
 * notification center when the new selection is different than the previous
 * one. 
 */
- (void) selectItemWithObjectValue: (id)object
{
 if (_usesDataSource)
   {
     NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
       self);
   }
 else
   {
     NSInteger i = [_popUpList indexOfObject: object];

     if (i == NSNotFound)
       i = -1;

     [self selectItemAtIndex: i];
   }
}

/**
 * Returns the object value at <var>index</var> within combo box cell default
 * items list. When the index is beyond the end of the list, an NSRangeException is
 * raised. In the case <code>usesDataSource</code> returns YES, this method logs
 * a warning.
 */
- (id) itemObjectValueAtIndex: (NSInteger)index
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
      return nil;
    }
  else
    {
      return [_popUpList objectAtIndex: index];
    }
}

/* FIXME: Not sure, if this is the best way to implement objectValue,
 * perhaps it would be better to store the current value with setObjectValue: 
 * whenever it changes.
 */
- (id) objectValue
{
  NSInteger index = [self indexOfSelectedItem];

  if (index == -1)
    {
      return nil;
    }
  else
    {
      if (_usesDataSource)
        {
	  if (_dataSource == nil)
	    {
	      NSLog(@"%@: No data source currently specified", self);
	      return nil;
	    }
	  if ([_dataSource respondsToSelector: 
			       @selector(comboBox:objectValueForItemAtIndex:)])
	    {
	      return [_dataSource comboBox: (NSComboBox *)[self controlView] 
				  objectValueForItemAtIndex: index];
	    }
	  else if ([_dataSource respondsToSelector: 
				    @selector(comboBoxCell:objectValueForItemAtIndex:)])
	    {
	      return [_dataSource comboBoxCell: self
				  objectValueForItemAtIndex: index];
	    }
	}
      else
        {
	  return [self itemObjectValueAtIndex: index];
	}
    }
    
  return nil;
}

/**
 * Returns the object value of the selected item in the combo box cell default
 * items list or nil when there is no selection. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (id) objectValueOfSelectedItem
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
      return nil;
    }
  else
    {
      NSInteger index = [self indexOfSelectedItem];

      if (index == -1)
        {
          return nil;
        }
      else
        {
          return [_popUpList objectAtIndex: index];
        }
    }
}

/**
 * Returns the lowest index associated with a value in the combo box
 * cell default items list, which is equal to <var>object</var>, and returns
 * NSNotFound when there is no such value. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (NSInteger) indexOfItemWithObjectValue: (id)object
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
      return 0;
    }
    
  return [_popUpList indexOfObject: object];
}

/** 
 * Returns the combo box cell default items list in an array.
 */
- (NSArray *) objectValues
{
  if (_usesDataSource)
    {
      NSLog(@"%@: This method is invalid, this combo box is set to use a data source", 
        self);
      return nil;
    }
    
  return _popUpList;
}

// Text completion
/**
 * Returns a string by looking in the combo box cell list for an item wich
 * starts with <var>substring</var>, or nil when there is no such string. 
 * <var>substring</var> is equal to what the user entered in the text field
 * part.
 * You rarely needs to call this method explicitly in your code.
 * By default, the implementation of this method first checks whether the combo
 * box cell uses a data source and whether the data source responds to 
 * <code>comboBox:completedString:</code> or <code>comboBoxCell:completedString:</code>. 
 * When it is the case, it uses this method to return <var>str</var>, else this
 * method goes through the combo box cell items one by one and returns the first
 * item found starting with <var>substring</var>.
 * In the case, you want another behavior, you can override this method without
 * need to call the superclass method.
 */
- (NSString *) completedString: (NSString *)substring
{
  if (nil == substring)
    {
      return nil;
    }

  if (_usesDataSource)
    {
      if (_dataSource == nil)
        {
	  NSLog(@"%@: No data source currently specified", self);
	}
      else if ([_dataSource respondsToSelector: @selector(comboBox:completedString:)])
        {
	  return [_dataSource comboBox: (NSComboBox *)[self controlView] 
		       completedString: substring];
	}
      else if ([_dataSource respondsToSelector: @selector(comboBoxCell:completedString:)])
        {
	  return [_dataSource comboBoxCell: self completedString: substring];
	}
      else
        {
          NSInteger i;

          for (i = 0; i < [self numberOfItems]; i++)
            {
	      NSString *str = [self _stringValueAtIndex: i];

	      if ([str length] > [substring length] && [str hasPrefix: substring])
	        return str;
            }	
	}
    }
  else
    {
      NSUInteger i;

      for (i = 0; i < [_popUpList count]; i++)
        {
	  NSString *str = [[_popUpList objectAtIndex: i] description];

	  if ([str length] > [substring length] && [str hasPrefix: substring])
	    return str;
        }
    }
  
  return substring;
}

/** 
 * Returns YES when the combo box cell automatic completion is active, returns
 * NO otherwise. 
 * Take a look at the <code>setCompletes:</code> method documentation to know
 * how the automatic completion works. 
 */
- (BOOL) completes 
{ 
  return _completes; 
}

/** 
 * Sets whether the combo box cell automatic completion is active or not.
 * The automatic completion tries to complete what the user types in the text
 * field part, it tries to complete only when the the user adds characters at
 * the end of the string, not when it deletes characters or when the insertion
 * point precedes the end of the string.
 * To do the automatic completion, the <code>completedString:</code> method is
 * called, and when the returned string is longer than the current one in the text
 * field, the completion occurs and the completed part gets selected.
 */
- (void) setCompletes: (BOOL)completes
{
  _completes = completes;
}

- (BOOL) isButtonBordered
{
  return [_buttonCell isBordered];
}

- (void) setButtonBordered:(BOOL)flag
{
  [_buttonCell setBordered: flag];
}

#define ComboBoxHeight 21 // FIX ME: All this stuff shouldn't be hardcoded
#define ButtonWidth 17
#define ButtonHeight 17
#define BorderSize 2
// The inset border for the top and the bottom of the button

/*
 * Inlined methods
 */
 
static inline NSRect textCellFrameFromRect(NSRect cellRect) 
// Not the drawed part, precises just the part which receives events
{
  return NSMakeRect(NSMinX(cellRect),
		    NSMinY(cellRect),
		    NSWidth(cellRect) - ButtonWidth - BorderSize,
		    NSHeight(cellRect));
}

static inline NSRect buttonCellFrameFromRect(NSRect cellRect)
{
  return NSMakeRect(NSMaxX(cellRect) - ButtonWidth - BorderSize,
		    NSMinY(cellRect) + BorderSize,
		    ButtonWidth,
		    ButtonHeight);
}

// Overridden
+ (BOOL) prefersTrackingUntilMouseUp
{
  return YES; 
  
  /* Needed to have the clickability of the button take in account when the tracking happens.
     This method is call by the NSControl -mouseDown: method with the code :
     [_cell trackMouse: e
		inRect: _bounds
	        ofView: self
          untilMouseUp: [[_cell class] prefersTrackingUntilMouseUp]] */
}

- (NSSize) cellSize
{
  NSSize textSize;
  NSSize buttonSize;
  NSSize mySize;

  /* Simple version takes the size from text field. A more useful one could 
     loop over the strings of the combo box and calculate the maximal width of 
     all strings. */
  textSize = [super cellSize];
  // Or should we use the hard coded values from above here?
  buttonSize = [_buttonCell cellSize];

  mySize.height = MAX(textSize.height, buttonSize.height);
  mySize.width = textSize.width + BorderSize + buttonSize.width;

  return mySize;
}

- (void) drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
  NSRect rect = cellFrame;

  // FIX ME: Is this test case below with the method call really needed ?
  if ([GSCurrentContext() isDrawingToScreen]) 
    {		           
      [super drawInteriorWithFrame: textCellFrameFromRect(rect)
			    inView: controlView];
      [_buttonCell drawWithFrame: buttonCellFrameFromRect(rect)
		          inView: controlView];
    }
  else
    {
      [super drawInteriorWithFrame: rect inView: controlView];
    }
    
  // Used by GSComboWindow to appear in the right position
  _lastValidFrame = cellFrame; 
}

- (void) highlight: (BOOL)flag
	 withFrame: (NSRect)cellFrame
	    inView: (NSView *)controlView
{
  NSRect rect = cellFrame;

  // FIX ME: Is this test case below with the method call really needed ?
  if ([GSCurrentContext() isDrawingToScreen])
    {
      [super highlight: flag
	     withFrame: textCellFrameFromRect(rect)
	        inView: controlView];
      [_buttonCell highlight: flag
		   withFrame: buttonCellFrameFromRect(rect)
		      inView: controlView];
    }
  else
    {
      [super highlight: flag withFrame: rect inView: controlView];
    }
}

/** Overrides NSCell <code>trackMouse:inRect:ofView:untilMouseUp:</code> method to establish a
 * new method behavior.
 * In the case <var>flag</var> is NO, returns NO when the mouse down occurs in the text
 * cell part or when the mouse down occurs in the button cell part followed by a
 * mouse up outside, otherwise returns YES (when both the mouse down and the
 * mouse up occurs in the button cell part).
 * In the case <var>flag</var> is YES, returns NO when the mouse occurs in the text
 * cell part, otherwise returns YES (when the mouse down occurs in the button cell
 * part).
 */
- (BOOL) trackMouse: (NSEvent *)theEvent 
	     inRect: (NSRect)cellFrame
	     ofView: (NSView *)controlView 
       untilMouseUp: (BOOL)flag
{
  NSPoint point;
  BOOL isFlipped = [controlView isFlipped];
  NSRect buttonRect = buttonCellFrameFromRect(cellFrame);
  NSRect textRect = textCellFrameFromRect(cellFrame);
  BOOL result = NO;

  // FIXME: May be that should be set by NSActionCell
  if (_control_view != controlView)
    _control_view = controlView;
  
  // Used by GSComboWindow to appear in the right position
  _lastValidFrame = cellFrame;
  point = [controlView convertPoint: [theEvent locationInWindow]
			   fromView: nil];

  if (NSMouseInRect(point, textRect, isFlipped))
    {
      return NO;
    }
  else if (NSMouseInRect(point, buttonRect, isFlipped))
    {
      NSEvent *e = theEvent;
      BOOL isMouseUp = NO;  
      NSUInteger eventMask = NSLeftMouseDownMask | NSLeftMouseUpMask
       | NSMouseMovedMask | NSLeftMouseDraggedMask | NSOtherMouseDraggedMask
       | NSRightMouseDraggedMask;
      NSPoint location;
      
      while (isMouseUp == NO) // Loop until mouse goes up
        {
          location = [controlView convertPoint: [e locationInWindow] fromView: nil];
             
	  // Ask the cell to track the mouse only when the mouse is within the cell
          if (NSMouseInRect(location, buttonRect, isFlipped))
	    {
	      [_buttonCell setHighlighted: YES];
	      [controlView setNeedsDisplayInRect: cellFrame];
		
	      result = [_buttonCell trackMouse: e
		                        inRect: buttonRect
		                        ofView: controlView
		                  untilMouseUp: [NSButtonCell prefersTrackingUntilMouseUp]];
              isMouseUp = result;

	      [_buttonCell setHighlighted: NO];
	      [controlView setNeedsDisplayInRect: cellFrame];
            }
		
          if (isMouseUp == NO)
	    {
	      e = [NSApp nextEventMatchingMask: eventMask
			             untilDate: [NSDate distantFuture]
		  		        inMode: NSEventTrackingRunLoopMode
				       dequeue: YES];
            
	      if ([e type] == NSLeftMouseUp)
	        isMouseUp = YES;
	    }
	}
	
      if (flag)
        {
	  return YES;
	}
      else
        {	   
          return NO;
	}
    }

  return NO; // Pathological case, normally never happens
}

- (void) resetCursorRect: (NSRect)cellFrame inView: (NSView *)controlView
{
  [super resetCursorRect: textCellFrameFromRect(cellFrame)
	 inView: controlView];
}

- (void) setEnabled: (BOOL)flag
{
  [_buttonCell setEnabled: flag];
  [super setEnabled: flag];
}

// NSCoding
/**
 * Encodes the combo box cell using <var>encoder</var>. take note that when it
 * uses a data source, the data source is conditionally encoded.
 */
- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];

  if ([coder allowsKeyedCoding])
    {
      [coder encodeBool: [self hasVerticalScroller] forKey: @"NSHasVerticalScroller"];
      [coder encodeInteger: [self numberOfVisibleItems] forKey: @"NSVisibleItemCount"];
      [coder encodeBool: [self completes] forKey: @"NSCompletes"];
      [coder encodeDouble: _intercellSpacing.width forKey: @"NSIntercellSpacingWidth"];
      [coder encodeDouble: _intercellSpacing.height forKey: @"NSIntercellSpacingHeight"];
      [coder encodeDouble: [self itemHeight] forKey: @"NSRowHeight"];
      [coder encodeBool: [self usesDataSource] forKey: @"NSUsesDataSource"];
      [coder encodeObject: [self dataSource] forKey: @"NSDataSource"];
      [coder encodeObject: _popUpList forKey: @"NSPopUpListData"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(id) at: &_popUpList];
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_usesDataSource];
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_hasVerticalScroller];
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_completes];
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_usesDataSource];
      encode_NSInteger(coder, &_visibleItems);
      [coder encodeValueOfObjCType: @encode(NSSize) at: &_intercellSpacing];
      [coder encodeValueOfObjCType: @encode(float) at: &_itemHeight];
      encode_NSInteger(coder, &_selectedItem);
      
      if (_usesDataSource == YES)
	[coder encodeConditionalObject: _dataSource];      
    }
}

/**
 * Initializes the combo box cell with data linked to <var>decoder</var>. Take
 * note that when the decoded instance uses a data source,
 * <code>initWithCoder:<var> decodes the data source. 
 * Finally, returns thr initialized object.
 */
- (id) initWithCoder: (NSCoder *)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      //id delegate = [aDecoder decodeObjectForKey: @"NSDelegate"];
      //id table = [aDecoder decodeObjectForKey: @"NSTableView"];

      if ([aDecoder containsValueForKey: @"NSHasVerticalScroller"])
        {
	  [self setHasVerticalScroller: [aDecoder decodeBoolForKey: 
						      @"NSHasVerticalScroller"]];
	}
      if ([aDecoder containsValueForKey: @"NSVisibleItemCount"])
        {
	  [self setNumberOfVisibleItems: [aDecoder decodeIntegerForKey:
						       @"NSVisibleItemCount"]];
	}
      if ([aDecoder containsValueForKey: @"NSCompletes"])
        {
          [self setCompletes: [aDecoder decodeBoolForKey: @"NSCompletes"]];
        }
      if ([aDecoder containsValueForKey: @"NSIntercellSpacingWidth"])
        {
          _intercellSpacing.width = [aDecoder decodeDoubleForKey: 
                                                @"NSIntercellSpacingWidth"];
        }
      if ([aDecoder containsValueForKey: @"NSIntercellSpacingHeight"])
        {
          _intercellSpacing.height = [aDecoder decodeDoubleForKey: 
                                                 @"NSIntercellSpacingHeight"];
        }
      if ([aDecoder containsValueForKey: @"NSRowHeight"])
        {
	  [self setItemHeight: [aDecoder decodeDoubleForKey: 
                                           @"NSRowHeight"]];
	}
      if ([aDecoder containsValueForKey: @"NSUsesDataSource"])
        {
          [self setUsesDataSource: [aDecoder decodeBoolForKey: 
                                               @"NSUsesDataSource"]];
        }
      if ([aDecoder containsValueForKey: @"NSDataSource"])
        {
          [self setDataSource: [aDecoder decodeObjectForKey: @"NSDataSource"]];
        }
      if ([aDecoder containsValueForKey: @"NSPopUpListData"])
        {
          ASSIGN(_popUpList, [aDecoder decodeObjectForKey: @"NSPopUpListData"]);
        }
    }
  else
    {
      BOOL dummy;
      
      if ([aDecoder versionForClassName: @"NSComboBoxCell"] < 2)
        {
          // In previous version we decode _buttonCell, we just discard the decoded value here
          id previouslyEncodedButton;
          [aDecoder decodeValueOfObjCType: @encode(id) at: &previouslyEncodedButton];
        }

      [aDecoder decodeValueOfObjCType: @encode(id) at: &_popUpList];
      RETAIN(_popUpList);
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_usesDataSource];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasVerticalScroller];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_completes];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &dummy];
      decode_NSInteger(aDecoder, &_visibleItems);
      [aDecoder decodeValueOfObjCType: @encode(NSSize) at: &_intercellSpacing];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_itemHeight];
      decode_NSInteger(aDecoder, &_selectedItem);

      if (_usesDataSource == YES)
	[self setDataSource: [aDecoder decodeObject]];      
    }
  
  [self _loadButtonCell];

  return self;
}

- (void) selectWithFrame: (NSRect)aRect 
		  inView: (NSView *)controlView
		  editor: (NSText *)textObj 
		delegate: (id)anObject
		   start: (NSInteger)selStart 
		  length: (NSInteger)selLength
{
  [super selectWithFrame: textCellFrameFromRect(aRect)
                  inView: controlView
                  editor: textObj 
                delegate: anObject
                   start: selStart 
                  length: selLength];
  
  [nc addObserver: self 
         selector: @selector(textDidChange:)
	     name: NSTextDidChangeNotification 
	   object: textObj];
  [nc addObserver: self 
         selector: @selector(textViewDidChangeSelection:)
	     name: NSTextViewDidChangeSelectionNotification 
	   object: textObj];
	  
  // This method is called when the cell obtains the focus;
  // don't know why the next method editWithFrame: is not called
}

- (void) editWithFrame: (NSRect)frame
                inView: (NSView *)controlView
	        editor: (NSText *)textObj 
	      delegate: (id)delegate 
	         event: (NSEvent *)theEvent
{
  [super editWithFrame: textCellFrameFromRect(frame)
                inView: controlView
                editor: textObj
              delegate: delegate
                 event: theEvent];
  
  /*
  [nc addObserver: self 
         selector: @selector(textDidChange:)
	     name: NSTextDidChangeNotification 
	   object: textObj];
  [nc addObserver: self 
         selector: @selector(textViewDidChangeSelection:)
	     name: NSTextViewDidChangeSelectionNotification 
	   object: textObj]; */
}

- (void) endEditing: (NSText *)editor
{
  /* Close the pop up if it is still open. This may happen, e.g., when the
     user presses the Tab key to shift focus to a different cell or view. */
  if (_popup)
    [_popup onWindowEdited: nil];

  [super endEditing: editor];
  [nc removeObserver: self name: NSTextDidChangeNotification object: editor];
  [nc removeObserver: self 
                name: NSTextViewDidChangeSelectionNotification 
	      object: editor];
}

- (void) textViewDidChangeSelection: (NSNotification *)notification
{
  _prevSelectedRange = [[[notification userInfo] 
    objectForKey: @"NSOldSelectedCharacterRange"] rangeValue];
}

- (void) textDidChange: (NSNotification *)notification
{
  NSText *textObject = [notification object];

  if ([self completes])
    {
      NSString *myString = [[textObject string] copy];
      NSString *more;
      NSUInteger myStringLength = [myString length];
      NSUInteger location, length;
      NSRange selectedRange = [textObject selectedRange];
      
      if (myStringLength != 0
        && selectedRange.location == myStringLength
        && _prevSelectedRange.location < selectedRange.location)
        {
          more = [self completedString: myString];
          if ((more != nil) && [more isEqualToString: myString] == NO)
            {
	      [textObject setString: more];
	      location = myStringLength;
              length = [more length] - location;
	      [textObject setSelectedRange: NSMakeRange(location, length)];
	      [textObject scrollRangeToVisible: NSMakeRange(location, length)];
	    }
        }
      RELEASE(myString);
    }
}

@end

@implementation NSComboBoxCell (GNUstepPrivate)

- (NSString *) _stringValueAtIndex: (NSInteger)index
{
  if (_usesDataSource == NO)
    {
      return [[self itemObjectValueAtIndex: index] description];
    }
  else
    {
      if (_dataSource == nil)
        {
	  NSLog(@"%@: No data source currently specified", self);
	  return nil;
	}
      else if ([_dataSource respondsToSelector: 
			   @selector(comboBox:objectValueForItemAtIndex:)])
        {
	  return [[_dataSource comboBox: (NSComboBox *)[self controlView] 
			       objectValueForItemAtIndex: index] description];
	}
      else if ([_dataSource respondsToSelector: 
				@selector(comboBoxCell:objectValueForItemAtIndex:)])
        {
	  return [[_dataSource comboBoxCell: self
			      objectValueForItemAtIndex: index] description];
	}
    }

  return nil;
}

- (void) _performClickWithFrame: (NSRect)cellFrame 
                         inView: (NSView *)controlView
{
  NSWindow *cvWindow = [controlView window];
  NSRect buttonRect = buttonCellFrameFromRect(cellFrame);

  _control_view = controlView;
  [controlView lockFocus];
  [_buttonCell highlight: YES 
	       withFrame: buttonRect
	          inView: controlView];
  [controlView unlockFocus];
  [cvWindow flushWindow];

  [self _didClickWithinButton: self];
  
  [controlView lockFocus];
  [_buttonCell highlight: NO 
	       withFrame: buttonRect
	          inView: controlView];
  [controlView unlockFocus];
  [cvWindow flushWindow];

}

- (void) _didClickWithinButton: (id)sender
{
  NSView *controlView = [self controlView];
  
  if ((_cell.is_disabled) || (controlView == nil))
    return;

  [nc postNotificationName: NSComboBoxWillPopUpNotification 
                    object: controlView 
		  userInfo: nil];
  
  _popup = [self _popUp];
  [_popup popUpForComboBoxCell: self];
  _popup = nil;

  [nc postNotificationName: NSComboBoxWillDismissNotification
                    object: controlView
                  userInfo: nil];
}

- (BOOL) _isWantedEvent: (NSEvent *)event
{
  NSPoint loc;
  NSWindow *window = [event window];
  NSView *controlView = [self controlView];
  
  if (window == [[self controlView] window])
    {
      loc = [event locationInWindow];
      loc = [controlView convertPoint: loc fromView: nil];
      return NSMouseInRect(loc, [self _textCellFrame], [controlView isFlipped]);
    }
  else
    {
      return NO;
    }
}

- (GSComboWindow *) _popUp
{
  return [GSComboWindow defaultPopUp];
}

- (NSRect) _textCellFrame
{
  return textCellFrameFromRect(_lastValidFrame);
}

- (void) _setSelectedItem: (NSInteger)index
{
  _selectedItem = index;
}

- (void) _loadButtonCell
{
  _buttonCell = [[NSButtonCell alloc] initImageCell: 
		    [NSImage imageNamed: @"NSComboArrow"]];
  [_buttonCell setImagePosition: NSImageOnly];
  [_buttonCell setButtonType: NSMomentaryPushButton];
  [_buttonCell setHighlightsBy: NSPushInCellMask];
  [_buttonCell setBordered: YES];
  [_buttonCell setTarget: self];
  [_buttonCell setAction: @selector(_didClickWithinButton:)];
  [_buttonCell setEnabled: [self isEnabled]];
}

- (void) _selectCompleted
{
  NSString *more;
  NSUInteger index = NSNotFound;

  more = [self completedString: [self stringValue]];
  if (_usesDataSource)
    {
      if (_dataSource == nil)
        {
	  NSLog(@"%@: No data source currently specified", self);
	}
      else
        {
	  if ([_dataSource respondsToSelector: 
			       @selector(comboBoxCell:indexOfItemWithStringValue:)])
	    {
	      index = [_dataSource comboBoxCell: self 
		     indexOfItemWithStringValue: more];
	    }
	}
    }
  else
    {
      index = [[self objectValues] indexOfObject: more];
    }

  if (index != NSNotFound)
    {
      [self _setSelectedItem: index];
    }
  // Otherwise keep old selection
}

@end
