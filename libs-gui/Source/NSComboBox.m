/** <title>NSComboBox</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Gerrit van Dyk <gerritvd@decillion.net>
   Date: 1999

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
#import "AppKit/NSComboBox.h"
#import "AppKit/NSComboBoxCell.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSTextView.h"

/*
 * Class variables
 */
static Class usedCellClass;
static Class comboBoxCellClass;
static NSNotificationCenter *nc;

/*
 * Declaration of private cell method
 */

@interface NSComboBoxCell (GNUstepPrivate)
- (void) _performClickWithFrame: (NSRect)cellFrame
			 inView: (NSView *)controlView;
@end

/**
 <unit>
 <heading>Class Description</heading> 
 <p>An NSComboBox is what we can call a completion/choices box, derived from
 NSTextField, it allows you to enter text like in a text field but also to click
 in the ellipsis button (indicating the fact other user inputs are possible) on
 the right of it to obtain a list of choices, you can use them as the text field
 value by selecting a row in this list. You can also obtain direct completion
 when it  is enabled via <code>setCompletes:</code> to get a suggested text
 field value updated as you type.</p>
 <p>Like other NSControl classes, NSComboBox is a wrapper around a core piece which
 implements the combo box behavior, a cell, which is in this case an
 NSComboBoxCell.</p>
 </unit>
*/ 

/**
<p>No special instructions to use NSComboBox or text to detail the implementation.</p>
 */
@implementation NSComboBox

+ (void) initialize
{
  if (self == [NSComboBox class])
    {
       [self setVersion: 1];
       comboBoxCellClass = [NSComboBoxCell class];
       usedCellClass = comboBoxCellClass;
       nc = [NSNotificationCenter defaultCenter];
    }
}

/*
 * Setting the Cell class
 */
+ (Class) cellClass
{
  return usedCellClass;
}

+ (void) setCellClass: (Class)factoryId
{
  usedCellClass = factoryId ? factoryId : comboBoxCellClass;
}

/**
 * Returns YES when the combo box cell displays a vertical scroller for its
 * list, returns NO otherwise. 
 * Take note that the scroller will be displayed even when the sum of the items
 * height in the list is inferior to the minimal height of the list displayed
 * area. 
 */
- (BOOL)hasVerticalScroller
{
  return [_cell hasVerticalScroller];
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
- (void)setHasVerticalScroller:(BOOL)flag
{
  [_cell setHasVerticalScroller:flag];
}

/**
 * Returns the width and the height (as the values of an NSSize variable)
 * between each item of the combo box cell list.
 */
- (NSSize)intercellSpacing
{
  return [_cell intercellSpacing];
}

/**
 * Sets the width and the height between each item of the combo box cell list to
 * the values in <var>aSize</var>.
 */
- (void)setIntercellSpacing:(NSSize)aSize
{
  [_cell setIntercellSpacing:aSize];
}

/**
 * Returns the height of the items in the combo box cell list.
 */
- (CGFloat)itemHeight
{
  return [_cell itemHeight];
}

/**
 * Sets the height of the items in the combo box cell list to
 * <var>itemHeight</var>.
 */
- (void)setItemHeight:(CGFloat)itemHeight
{
  [_cell setItemHeight:itemHeight];
}

/**
 * Returns the maximum number of allowed items to be displayed in the combo box
 * cell list.
 */
- (NSInteger)numberOfVisibleItems
{
  return [_cell numberOfVisibleItems];
}

/**
 * Sets the maximum number of allowed items to be displayed in the combo box
 * cell list.
 */
- (void)setNumberOfVisibleItems:(NSInteger)visibleItems
{
  [_cell setNumberOfVisibleItems:visibleItems];
}

/** 
 * Marks the combo box cell in order to have its items list reloaded in the
 * case it uses a data source, and to have it redisplayed.
 */
- (void)reloadData
{
  [_cell reloadData];
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
- (void)noteNumberOfItemsChanged
{
  [_cell noteNumberOfItemsChanged];
}

/**
 * Returns YES when the combo box cell uses a data source (which is external) to
 * populate its items list, otherwise returns NO in the case it uses its default
 * list.
 */
- (BOOL)usesDataSource
{
  return [_cell usesDataSource];
}

/**
 * Sets according to <var>flag</var> whether the combo box cell uses a data
 * source (which is external) to populate its items list.
 */
- (void)setUsesDataSource:(BOOL)flag
{
  [_cell setUsesDataSource:flag];
}

/**
 * Scrolls the combo box cell list vertically in order to have the item at
 * <var>index</var> in the closest position relative to the top. There is no
 * need to have the list displayed when this method is invoked.
 */
- (void)scrollItemAtIndexToTop:(NSInteger)index
{
  [_cell scrollItemAtIndexToTop:index];
}

/**
 * Scrolls the combo box cell list vertically in order to have the item at
 * <var>index</var> visible. There is no need to have the list displayed when
 * this method is invoked. 
 */
- (void)scrollItemAtIndexToVisible:(NSInteger)index
{
  [_cell scrollItemAtIndexToVisible:index];
}

/**
 * Selects the combo box cell list row at <var>index</var>. 
 * Take note no changes occurs in the combo box cell list when this method is
 * called. 
 * Posts an NSComboBoxSelectionDidChangeNotification to the default notification
 * center when there is a new selection different from the previous one.
 */
- (void)selectItemAtIndex:(NSInteger)index
{
  [_cell selectItemAtIndex:index];
}

/**
 * Deselects the combo box cell list row at <var>index</var> in the case this
 * row is selected. 
 * Posts an NSComboBoxSelectionDidChangeNotification to the default notification
 * center, when there is a new selection.
 */
- (void)deselectItemAtIndex:(NSInteger)index
{
  [_cell deselectItemAtIndex:index];
}

/**
 * Returns the index of the selected item in the combo box cell list or -1 when
 * there is no selection, the selected item can be related to the data source
 * object in the case <code>usesDataSource</code> returns YES else to the
 * default items list. 
 */
- (NSInteger)indexOfSelectedItem
{
  return [_cell indexOfSelectedItem];
}

/**
 * Returns the number of items in the the combo box cell list, the numbers of
 * items can be be related to the data source object in the case
 * <code>usesDataSource</code> returns YES else to the default items list.
 */
- (NSInteger)numberOfItems
{
  return [_cell numberOfItems];
}

/**
 * Returns the combo box cell data source object which is reponsible to provide
 * the data to be displayed. To know how to implement a data source object,
 * take a  look at the NSComboBoxDataSource informal protocol description. In
 * the case <code>usesDataSource</code> returns NO, this method logs a warning.
 */
- (id)dataSource
{
  return [_cell dataSource];
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
- (void)setDataSource:(id)aSource
{
  [_cell setDataSource:aSource];
}

/**
 * Adds an item to the combo box cell default items list which is used when
 * <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void)addItemWithObjectValue:(id)object
{
  [_cell addItemWithObjectValue:object];
}

/**
 * Adds several items in an array to the combo box cell default items list which
 * is used when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void)addItemsWithObjectValues:(NSArray *)objects
{
  [_cell addItemsWithObjectValues:objects];
}

/**
 * Inserts an item in the combo box cell default items list which
 * is used when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void)insertItemWithObjectValue:(id)object atIndex:(NSInteger)index
{
  [_cell insertItemWithObjectValue:object atIndex:index];
}

/**
 * Removes an item in the combo box cell default items list which
 * is used when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void)removeItemWithObjectValue:(id)object
{
  [_cell removeItemWithObjectValue:object];
}

/**
 * Removes the item with the specified <var>index</var> in the combo box cell
 * default items list which is used when <code>usesDataSource</code> returns NO.
 * In the case <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void)removeItemAtIndex:(NSInteger)index
{
  [_cell removeItemAtIndex:index];
}

/**
 * Removes all the items in the combo box cell default items list which is used
 * when <code>usesDataSource</code> returns NO. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (void)removeAllItems
{
  [_cell removeAllItems];
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
- (void)selectItemWithObjectValue:(id)object
{
  [_cell selectItemWithObjectValue:object];
}

/**
 * Returns the object value at <var>index</var> within combo box cell default
 * items list. When the index is beyond the end of the list, an NSRangeException is
 * raised. In the case <code>usesDataSource</code> returns YES, this method logs
 * a warning.
 */
- (id)itemObjectValueAtIndex:(NSInteger)index
{
  return [_cell itemObjectValueAtIndex:index];
}

/**
 * Returns the object value of the selected item in the combo box cell default
 * items list or nil when there is no selection. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (id)objectValueOfSelectedItem
{
  return [_cell objectValueOfSelectedItem];
}

/**
 * Returns the lowest index associated with a value in the combo box
 * cell default items list, which is equal to <var>object</var>, and returns
 * NSNotFound when there is no such value. In the case
 * <code>usesDataSource</code> returns YES, this method logs a warning.
 */
- (NSInteger)indexOfItemWithObjectValue:(id)object
{
  return [_cell indexOfItemWithObjectValue:object];
}

/** 
 * Returns the combo box cell default items list in an array.
 */
- (NSArray *)objectValues
{
  return [_cell objectValues];
}

/** 
 * Returns YES when the combo box cell automatic completion is active, returns
 * NO otherwise. 
 * Take a look at the <code>setCompletes:</code> method documentation to know
 * how the automatic completion works. 
 */
- (BOOL)completes
{
  return [_cell completes];
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
- (void)setCompletes:(BOOL)completes
{
  [_cell setCompletes: completes];
}

- (BOOL) isButtonBordered
{
  return [_cell isButtonBordered];
}

- (void) setButtonBordered:(BOOL)flag
{
  [_cell setButtonBordered: flag];
}

- (void) setDelegate: (id)anObject
{
  [super setDelegate: anObject];

#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(comboBox##notif_name:)]) \
    [nc addObserver: _delegate \
           selector: @selector(comboBox##notif_name:) \
               name: NSComboBox##notif_name##Notification object: self]

  SET_DELEGATE_NOTIFICATION(SelectionDidChange);
  SET_DELEGATE_NOTIFICATION(SelectionIsChanging);
  SET_DELEGATE_NOTIFICATION(WillPopUp);
  SET_DELEGATE_NOTIFICATION(WillDismiss);
}

// Overridden
- (void) mouseDown: (NSEvent*)theEvent
{
  BOOL buttonClicked; 
  // buttonClicked is set to the value NO when the click occurs in the text cell
  // and to the value YES when it occurs in the button cell.
  
  buttonClicked = [_cell trackMouse: theEvent inRect: [self bounds] 
    ofView: self untilMouseUp: YES];
  
  if (!buttonClicked)
    [super mouseDown: theEvent];
}

- (BOOL) textView: (NSTextView *)textView doCommandBySelector: (SEL)command
{
  if ([super textView: textView doCommandBySelector: command])
    return YES;
  if (sel_isEqual(command, @selector(moveDown:)))
    {
      [_cell _performClickWithFrame: [self bounds] inView: self];
      return YES;
    }
  return NO;
}

- (void) setFrame: (NSRect)frame
{
  NSRect rect = NSMakeRect(frame.origin.x, frame.origin.y, frame.size.width, 21);
  // FIX ME: We shouldn't harcode the height value
  
  [super setFrame: rect];
}

@end
