/** <title>NSTableView</title>

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: March 2000, June 2000, August 2000, September 2000
   
   Author: Pierre-Yves Rivaille <pyrivail@ens-lyon.fr>
   Date: August 2001, January 2002

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: March 2004

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

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFormatter.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSSortDescriptor.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSKeyedArchiver.h>

#import "AppKit/NSTableView.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTableHeaderView.h"
#import "AppKit/NSText.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSWindow.h"
#import "AppKit/PSOperators.h"
#import "AppKit/NSCachedImageRep.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSDragging.h"
#import "AppKit/NSCustomImageRep.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSBindingHelpers.h"

#include <math.h>
static NSNotificationCenter *nc = nil;

static const int currentVersion = 5;

static NSRect oldDraggingRect;
static NSInteger oldDropRow;
static NSTableViewDropOperation oldDropOperation;
static NSTableViewDropOperation currentDropOperation;
static NSInteger currentDropRow;
static NSInteger lastQuarterPosition;
static NSDragOperation currentDragOperation;

/*
 * Nib compatibility struct.  This structure is used to 
 * pull the attributes out of the nib that we need to fill
 * in the flags.
 */
typedef struct _tableViewFlags
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int columnOrdering:1;
  unsigned int columnResizing:1;
  unsigned int drawsGrid:1;
  unsigned int emptySelection:1;
  unsigned int multipleSelection:1;
  unsigned int columnSelection:1;
  unsigned int _unused:26;
#else
  unsigned int _unused:26;
  unsigned int columnSelection:1;
  unsigned int multipleSelection:1;
  unsigned int emptySelection:1;
  unsigned int drawsGrid:1;
  unsigned int columnResizing:1;
  unsigned int columnOrdering:1;
#endif
} GSTableViewFlags;

#define ALLOWS_MULTIPLE (1)
#define ALLOWS_EMPTY (1 << 1)
#define SHIFT_DOWN (1 << 2)
#define CONTROL_DOWN (1 << 3)
#define ADDING_ROW (1 << 4)

@interface NSTableView (NotificationRequestMethods)
- (void) _postSelectionIsChangingNotification;
- (void) _postSelectionDidChangeNotification;
- (void) _postColumnDidMoveNotificationWithOldIndex: (NSInteger) oldIndex
					   newIndex: (NSInteger) newIndex;
- (void) _postColumnDidResizeNotification;
- (BOOL) _shouldSelectTableColumn: (NSTableColumn *)tableColumn;
- (BOOL) _shouldSelectRow: (NSInteger)rowIndex;

- (BOOL) _shouldSelectionChange;
- (void) _didChangeSortDescriptors: (NSArray *)oldSortDescriptors;
- (void) _didClickTableColumn: (NSTableColumn *)tc;
- (BOOL) _shouldEditTableColumn: (NSTableColumn *)tableColumn
			    row: (NSInteger) rowIndex;
- (void) _willDisplayCell: (NSCell*)cell
	   forTableColumn: (NSTableColumn *)tb
		      row: (NSInteger)index;

- (BOOL) _writeRows: (NSIndexSet *)rows
       toPasteboard: (NSPasteboard *)pboard;
- (BOOL) _isDraggingSource;
- (id)_objectValueForTableColumn: (NSTableColumn *)tb
			     row: (NSInteger)index;
- (void)_setObjectValue: (id)value
	 forTableColumn: (NSTableColumn *)tb
		    row: (NSInteger)index;

- (BOOL) _isEditableColumn: (NSInteger)columnIndex
                       row: (NSInteger)rowIndex;
- (BOOL) _isCellSelectableColumn: (NSInteger)columnIndex
                             row: (NSInteger)rowIndex;
- (BOOL) _isCellEditableColumn: (NSInteger)columnIndex
			   row: (NSInteger)rowIndex;
- (NSInteger) _numRows;
@end

@interface NSTableView (SelectionHelper)
- (void) _setSelectingColumns: (BOOL)flag;
- (NSArray *) _indexSetToArray: (NSIndexSet*)indexSet;
- (NSArray *) _selectedRowArray;
- (BOOL) _selectRow: (NSInteger)rowIndex;
- (BOOL) _selectUnselectedRow: (NSInteger)rowIndex;
- (BOOL) _unselectRow: (NSInteger)rowIndex;
- (void) _unselectAllRows;
- (NSArray *) _selectedColumArray;
- (void) _unselectAllColumns;
@end

@interface NSTableView (EventLoopHelper)
- (void) _trackCellAtColumn:(NSInteger)column row:(NSInteger)row withEvent:(NSEvent *)ev;
- (BOOL) _startDragOperationWithEvent:(NSEvent *)theEvent;
@end

/*
 *  A specific struct and its associated quick sort function
 *  This is used by the -sizeToFit method
 */
typedef struct {
  CGFloat width;
  BOOL isMax;
} columnSorting;


static
void quick_sort_internal(columnSorting *data, int p, int r)
{
  if (p < r)
    {
      int q;
      {
	CGFloat x = data[p].width;
	BOOL y = data[p].isMax;
	int i = p - 1;
	int j = r + 1;
	columnSorting exchange;
	while (1)
	  {
	    j--;
	    for (; 
		(data[j].width > x)
		  || ((data[j].width == x) 
		      && (data[j].isMax == YES)
		      && (y == NO));
		j--)
	      ;

	    i++;
	    for (;
		(data[i].width < x)
		  || ((data[i].width == x) 
		      && (data[i].isMax == NO)
		      && (y == YES));
		i++)
	      ;
	    if (i < j)
	      {
		exchange = data[j];
		data[j] = data[i];
		data[i] = exchange;
	      }
	    else
	      {
		q = j;
		break;
	      }
	  }
      }
      quick_sort_internal(data, p, q);
      quick_sort_internal(data, q + 1, r);
    }
}

/* 
 * Now some auxiliary functions used to manage real-time user selections. 
 *
 */

static void computeNewSelection
(NSTableView *tv,
 NSIndexSet *_oldSelectedRows,
 NSMutableIndexSet *_selectedRows,
 NSInteger _originalRow,
 NSInteger _oldRow,
 NSInteger _currentRow,
 NSInteger *_selectedRow,
 unsigned selectionMode)
{
  if (!(selectionMode & ALLOWS_MULTIPLE))
    {
      if ((selectionMode & SHIFT_DOWN) && 
	  (selectionMode & ALLOWS_EMPTY) && 
	  !(selectionMode & ADDING_ROW))
	  // we will unselect the selected row
	  // ic, sc : ok
        {
	  NSUInteger count = [_selectedRows count];

	  if ((count == 0) && (_oldRow == -1))
	    {
	      NSLog(@"how did you get there ?");
	      NSLog(@"you're supposed to have clicked on a selected row,");
	      NSLog(@"but there's no selected row!");
	      return;
	    }
	  else if (count > 1)
	    {
	      [tv _unselectAllRows];
	      [tv _postSelectionIsChangingNotification];
	    }
	  else if (_currentRow != _originalRow)
	    {
	      if (*_selectedRow == _originalRow)
	        {
		  // we are already selected, don't do anything
		}
	      else
	        {
		  //begin checking code
		  if (count > 0)
		    {
		      NSLog(@"There should not be any row selected");
		    }
		  //end checking code
		  
		  if ([tv _selectRow: _originalRow])
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (_currentRow == _originalRow)
	    {
	      if (count == 0)
	        {
		  // the row is already deselected
		  // nothing to do !
		}
	      else
	        {
		  [tv _unselectRow: _originalRow];
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
      else  //(!(selectionMode & ALLOWS_MULTIPLE) && 
	    //(!(selectionMode & SHIFT_DOWN) || 
	    //!(selectionMode & ALLOWS_EMPTY) ||
	    //(selectionMode & ADDING_ROW)))
	  // we'll be selecting exactly one row
	  // ic, sc : ok
        {
	  NSUInteger count = [_selectedRows count];
	  
	  if ([tv _shouldSelectRow: _currentRow] == NO)
	    {
	      return;
	    }
	  
	  if ((count != 1) || (_oldRow == -1))
	    {
	      // this is the first call that goes thru shouldSelectRow
	      // Therefore we don't know anything about the selection
	      BOOL notified = ![_selectedRows containsIndex: _currentRow];
	      [tv _unselectAllRows];
	      [_selectedRows addIndex: _currentRow];
	      *_selectedRow = _currentRow;
	      
	      if (notified == YES)
	        {
		  [tv setNeedsDisplayInRect: [tv rectOfRow: _currentRow]];
		}
	      else
	        {
		  if (count > 1)
		    {
		      notified = YES;
		    }
		}
	      if (notified == YES)
	        {
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	  else
	    {
	      // we know there is only one column selected
	      // this column is *_selectedRow

	      //begin checking code
	      if (![_selectedRows containsIndex: *_selectedRow])
	        {
		  NSLog(@"*_selectedRow is not the only selected row!");
		}
	      //end checking code
	      
	      if (*_selectedRow == _currentRow)
	        {
		  // currentRow is already selecteed
		  return;
		}
	      else
	        {
		  [tv _unselectRow: *_selectedRow];
		  // CHANGE: This does a check more
		  [tv _selectRow: _currentRow];
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
    }
  else if ((selectionMode & ALLOWS_MULTIPLE)
	   && (selectionMode & SHIFT_DOWN)
	   && (selectionMode & ADDING_ROW))
    // we add new row to the current selection
    {
      if (_oldRow == -1)
	// this is the first pass
	{
	  BOOL notified = NO;
	  NSInteger i;
	  NSInteger diff = _currentRow - _originalRow;

	  if (diff >= 0)
	    {
	      for (i = _originalRow; i <= _currentRow; i++)
		{
		  if ([_selectedRows containsIndex: i] ||
		      [tv _selectRow: i])
		    {
		      *_selectedRow = i;
		      notified = YES;
		    }
		}
	    }
	  else
	    {
	      // this case does happen, (sometimes)
	      for (i = _originalRow; i >= _currentRow; i--)
		{
		  if ([_selectedRows containsIndex: i] ||
		      [tv _selectRow: i])
		    {
		      *_selectedRow = i;
		      notified = YES;
		    }
		}
	    }

	  if (notified == YES)
	    {
	      [tv _postSelectionIsChangingNotification];
	    }
	}
      else // new multiple selection, after first pass
	{ 
	  NSInteger oldDiff, newDiff, i;
	  oldDiff = _oldRow - _originalRow;
	  newDiff = _currentRow - _originalRow;
	  if (oldDiff >= 0 && newDiff >= 0)
	    {
	      if (newDiff >= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow + 1; i <= _currentRow; i++)
		    {
		      if ([_selectedRows containsIndex: i] ||
			  [tv _selectRow: i])
			{
			  *_selectedRow = i;
			  notified = YES;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i > _currentRow; i--)
		    {
		      if ([_oldSelectedRows containsIndex: i])
			{
			  // this row was in the old selection
			  // leave it selected
			  continue;
			}
		      
		      if ([tv _unselectRow: i])
		        {
			  notified = YES;
			}
		    }
		  if (*_selectedRow == -1)
		    {
		      NSUInteger last = [_selectedRows lastIndex];

		      if (last == NSNotFound)
			{
			  *_selectedRow = -1;
			}
		      else
			{
			  *_selectedRow = last;
			}
		    }
		  if (notified)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff <= 0)
	    {
	      if (newDiff <= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;
		  for (i = _oldRow - 1; i >= _currentRow; i--)
		    {
		      if ([_selectedRows containsIndex: i] ||
			  [tv _selectRow: i])
			{
			  *_selectedRow = i;
			  notified = YES;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i < _currentRow; i++)
		    {
		      if ([_oldSelectedRows containsIndex: i])
			{
			  // this row was in the old selection
			  // leave it selected
			  continue;
			}

		      if ([tv _unselectRow: i])
			{
			  notified = YES;
			}
		    }
		  if (*_selectedRow == -1)
		    {
		      NSUInteger first = [_selectedRows firstIndex];

		      if (first == NSNotFound)
			{
			  *_selectedRow = -1;
			}
		      else
			{
			  *_selectedRow = first;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff >= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      {
		for (i = _oldRow; i < _originalRow; i++)
		  {
		    if ([_oldSelectedRows containsIndex: i])
		      {
			// this row was in the old selection
			// leave it selected
			continue;
		      }
		    
		    if ([tv _unselectRow: i])
		      {
			notified = YES;
		      }
		  }
	      }
	      // then we're extending it
	      for (i = _originalRow + 1; i <= _currentRow; i++)
	        {
		  if ([_selectedRows containsIndex: i] ||
		      [tv _selectRow: i])
		    {
		      *_selectedRow = i;
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];

		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	  else if (oldDiff >= 0 && newDiff <= 0)
	    {
	      BOOL notified = NO;

	      // we're reducing the selection
	      for (i = _oldRow; i > _originalRow; i--)
	        {
		  if ([_oldSelectedRows containsIndex: i])
		    {
		      // this row was in the old selection
		      // leave it selected
		      continue;
		    }
		    
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      // then we're extending it
	      for (i = _originalRow - 1; i >= _currentRow; i--)
	        {
		  if ([_selectedRows containsIndex: i] ||
		      [tv _selectRow: i])
		    {
		      *_selectedRow = i;
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger last = [_selectedRows lastIndex];

		  if (last == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = last;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }

	}
    }
  else if ((selectionMode & ALLOWS_MULTIPLE)
	   && ((selectionMode & SHIFT_DOWN) == 0)
	   && (selectionMode & ALLOWS_EMPTY)
	)
    // ic, sr : ok
    // new multiple selection (empty possible)
    {
      if (_oldRow == -1)
	// this is the first pass
	// we'll clear the selection first
	{
	  NSInteger diff, i;
	  NSUInteger count = [_selectedRows count];
	  BOOL notified = NO;
      	  diff = _currentRow - _originalRow;

	  if (count > 0)
	    {
	      notified = YES;
	    }

	  [tv _unselectAllRows];

	  if (diff >= 0)
	    {
	      for (i = _originalRow; i <= _currentRow; i++)
		{
		  if ([tv _selectRow: i])
		    {
		      notified = YES;
		    }
		}
	    }
	  else
	    {
	      // this case does happen (sometimes)
	      for (i = _originalRow; i >= _currentRow; i--)
		{
		  if ([tv _selectRow: i])
		    {
		      notified = YES;
		    }
		}
	    }
	  if (notified == YES)
	    {
	      [tv _postSelectionIsChangingNotification];
	    }
	}
      else // new multiple selection, after first pass
	{ 
	  NSInteger oldDiff, newDiff, i;
	  oldDiff = _oldRow - _originalRow;
	  newDiff = _currentRow - _originalRow;
	  if (oldDiff >= 0 && newDiff >= 0)
	    {
	      if (newDiff >= oldDiff)
		{
		  BOOL notified = NO;
		  for (i = _oldRow + 1; i <= _currentRow; i++)
		    {
		      if ([tv _selectRow: i])
			{
			  notified = YES;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i > _currentRow; i--)
		    {
		      if ([tv _unselectRow: i])
			{
			  notified = YES;
			}
		    }
		  if (*_selectedRow == -1)
		    {
		      NSUInteger last = [_selectedRows lastIndex];

		      if (last == NSNotFound)
			{
			  *_selectedRow = -1;
			}
		      else
			{
			  *_selectedRow = last;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff <= 0)
	    {
	      if (newDiff <= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow - 1; i >= _currentRow; i--)
		    {
		      if ([tv _selectRow: i])
			{
			  notified = YES;
			}
		    } 
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i < _currentRow; i++)
		    {
		      if ([tv _unselectRow: i])
			{
			  notified = YES;
			}
		    }
		  if (*_selectedRow == -1)
		    {
		      NSUInteger first = [_selectedRows firstIndex];

		      if (first == NSNotFound)
			{
			  *_selectedRow = -1;
			}
		      else
			{
			  *_selectedRow = first;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff >= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      for (i = _oldRow; i < _originalRow; i++)
	        {
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      // then we're extending it
	      for (i = _originalRow + 1; i <= _currentRow; i++)
	        {
		  if ([tv _selectRow: i])
		    {
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];

		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	  else if (oldDiff >= 0 && newDiff <= 0)
	    {
	      BOOL notified = NO;

	      // we're reducing the selection
	      for (i = _oldRow; i > _originalRow; i--)
	        {
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      // then we're extending it
	      for (i = _originalRow - 1; i >= _currentRow; i--)
	        {
		  if ([tv _selectRow: i])
		    {
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger last = [_selectedRows lastIndex];

		  if (last == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = last;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
    }
  else if (((selectionMode & ALLOWS_MULTIPLE)
	    && ((selectionMode & SHIFT_DOWN) == 0)
	    && ((selectionMode & ALLOWS_EMPTY) == 0)
	    && (selectionMode & ADDING_ROW))
	   // the following case can be assimilated to the 
	   // one before, although it will lead to an
	   // extra redraw
	   // TODO: solve this issue
	   ||
	   ((selectionMode & ALLOWS_MULTIPLE)
	    && ((selectionMode & SHIFT_DOWN) == 0)
	    && ((selectionMode & ALLOWS_EMPTY) == 0)
	    && ((selectionMode & ADDING_ROW) == 0))
	)
    {
      if (_oldRow == -1)
	{
	  // if we can select the _originalRow, we'll clear the old selection
	  // else we'll add to the old selection
	  if ([tv _shouldSelectRow: _currentRow] == YES)
	    {
	      // let's clear the old selection
	      // this code is copied from another case
	      // (AM = 1, SD=0, AE=1, AR=*, first pass)
	      NSInteger diff, i;
	      NSUInteger count = [_selectedRows count];
	      BOOL notified = NO;
	      diff = _currentRow - _originalRow;
	      
	      if (count > 0)
		{
		  notified = YES;
		}
	      
	      [tv _unselectAllRows];
	      
	      if (diff >= 0)
		{
		  for (i = _originalRow; i <= _currentRow; i++)
		    {
		      if ([tv _selectRow: i])
		        {
			  notified = YES;
			}
		    }	      
		}
	      else
		{
		  for (i = _originalRow; i >= _currentRow; i--)
		    {
		      if ([tv _selectRow: i])
		        {
			  notified = YES;
			}
		    }	      
		}
              if (notified == YES)
                {
                  [tv _postSelectionIsChangingNotification];
                }
	    }
	  else
	    {
	      // let's add to the old selection
	      // this code is copied from another case
	      // (AM=1, SD=1, AE=*, AR=1)
	      NSInteger diff, i;
	      BOOL notified = NO;
	      diff = _currentRow - _originalRow;
	      
	      if (diff >= 0)
		{
		  for (i = _originalRow; i <= _currentRow; i++)
		    {
		      if ([_selectedRows containsIndex: i] || 
			  [tv _selectRow: i])
			{
			  *_selectedRow = i;
			  notified = YES;
			}
		    }
		}
	      else
		{
		  // this case does happen (sometimes)
		  for (i = _originalRow; i >= _currentRow; i--)
		    {
		      if ([_selectedRows containsIndex: i] || 
			  [tv _selectRow: i])
			{
			  *_selectedRow = i;
			  notified = YES;
			}
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
      else if ([_selectedRows containsIndex: _originalRow])
	// as the originalRow is selected,
	// we are in a new selection
	{
	  // this code is copied from another case
	  // (AM=1, SD=0, AE=1, AR=*, after first pass)
	  NSInteger oldDiff, newDiff, i;
	  oldDiff = _oldRow - _originalRow;
	  newDiff = _currentRow - _originalRow;
	  if (oldDiff >= 0 && newDiff >= 0)
	    {
	      if (newDiff >= oldDiff)
		{
		  BOOL notified = NO;
		  for (i = _oldRow + 1; i <= _currentRow; i++)
		    {
		      if ([tv _selectRow: i])
		        {
			  notified = YES;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i > _currentRow; i--)
		    {
		      if ([tv _unselectRow: i])
		        {
			  notified = YES;
			}
		    }
		  if (*_selectedRow == -1)
		    {
		      NSUInteger last = [_selectedRows lastIndex];
		      
		      if (last == NSNotFound)
		        {
			  *_selectedRow = -1;
			}
		      else
		        {
			  *_selectedRow = last;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff <= 0)
	    {
	      if (newDiff <= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow - 1; i >= _currentRow; i--)
		    {
		      if ([tv _selectRow: i])
		        {
			  notified = YES;
			}
		    } 
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i < _currentRow; i++)
		    {
		      if ([tv _unselectRow: i])
		        {
			  notified = YES;
			}
		    }

		  if (*_selectedRow == -1)
		    {
		      NSUInteger first = [_selectedRows firstIndex];

		      if (first == NSNotFound)
			{
			  *_selectedRow = -1;
			}
		      else
			{
			  *_selectedRow = first;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff >= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      for (i = _oldRow; i < _originalRow; i++)
	        {
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      // then we're extending it
	      for (i = _originalRow + 1; i <= _currentRow; i++)
	        {
		  if ([tv _selectRow: i])
		    {
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];

		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	  else if (oldDiff >= 0 && newDiff <= 0)
	    {
	      BOOL notified = NO;

	      // we're reducing the selection
	      for (i = _oldRow; i > _originalRow; i--)
	        {
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      // then we're extending it
	      for (i = _originalRow - 1; i >= _currentRow; i--)
	        {
		  if ([tv _selectRow: i])
		    {
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger last = [_selectedRows lastIndex];

		  if (last == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = last;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
      else
	// as the originalRow is not selection, 
	// we are adding to the old selection
	{
	  // this code is copied from another case
	  // (AM=1, SD=1, AE=*, AR=1, after first pass)
	  NSInteger oldDiff, newDiff, i;
	  oldDiff = _oldRow - _originalRow;
	  newDiff = _currentRow - _originalRow;

	  if (oldDiff >= 0 && newDiff >= 0)
	    {
	      if (newDiff >= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;
		  for (i = _oldRow + 1; i <= _currentRow; i++)
		    {
		      if ([_selectedRows containsIndex: i] || 
			  [tv _selectRow: i])
			{
			  *_selectedRow = i;
			  notified = YES;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i > _currentRow; i--)
		    {
		      if ([_oldSelectedRows containsIndex: i])
			{
			  // this row was in the old selection
			  // leave it selected
			  continue;
			}
		      
		      if ([tv _unselectRow: i])
		        {
			  notified = YES;
			}
		    }

		  if (*_selectedRow == -1)
		    {
		      NSUInteger last = [_selectedRows lastIndex];

		      if (last == NSNotFound)
		        {
			  *_selectedRow = -1;
			}
		      else
		        {
			  *_selectedRow = last;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff <= 0)
	    {
	      if (newDiff <= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;
		  for (i = _oldRow - 1; i >= _currentRow; i--)
		    {
		      if ([_selectedRows containsIndex: i] || 
			  [tv _selectRow: i])
			{
			  *_selectedRow = i;
			  notified = YES;
			}
		    }

		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i < _currentRow; i++)
		    {
		      if ([_oldSelectedRows containsIndex: i])
			{
			  // this row was in the old selection
			  // leave it selected
			  continue;
			}
		      if ([tv _unselectRow: i])
		        {
			  notified = YES;
			}
		    }
		  if (*_selectedRow == -1)
		    {
		      NSUInteger first = [_selectedRows firstIndex];

		      if (first == NSNotFound)
			{
			  *_selectedRow = -1;
			}
		      else
			{
			  *_selectedRow = first;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff >= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      for (i = _oldRow; i < _originalRow; i++)
	        {
		  if ([_oldSelectedRows containsIndex: i])
		    {
		      // this row was in the old selection
		      // leave it selected
		      continue;
		    }
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      // then we're extending it
	      for (i = _originalRow + 1; i <= _currentRow; i++)
	        {
		  if ([_selectedRows containsIndex: i] ||
		      [tv _selectRow: i])
		    {
		      *_selectedRow = i;
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		  
		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	  else if (oldDiff >= 0 && newDiff <= 0)
	    {
	      BOOL notified = NO;

	      // we're reducing the selection
	      for (i = _oldRow; i > _originalRow; i--)
	        {
		  if ([_oldSelectedRows containsIndex: i])
		    {
		      // this row was in the old selection
		      // leave it selected
		      continue;
		    }
		    
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}

	      // then we're extending it
	      for (i = _originalRow - 1; i >= _currentRow; i--)
	        {
		  if ([_selectedRows containsIndex: i] ||
		      [tv _selectRow: i])
		    {
		      *_selectedRow = i;
		      notified = YES;
		    }
		}

	      if (*_selectedRow == -1)
		{
		  NSUInteger last = [_selectedRows lastIndex];

		  if (last == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = last;
		    }
		}
	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
    }
  else if ((selectionMode & ALLOWS_MULTIPLE)
	   && (selectionMode & SHIFT_DOWN)
	   && (selectionMode & ALLOWS_EMPTY)
	   && ((selectionMode & ADDING_ROW) == 0))
    {
      if (_oldRow == -1)
	// this is the first pass
	{
	  NSInteger diff, i;
	  BOOL notified = NO;

      	  diff = _currentRow - _originalRow;

	  if (diff >= 0)
	    {
	      for (i = _originalRow; i <= _currentRow; i++)
		{
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		  
		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	    }
	  else
	    {
	      // this case does happen (sometimes)
	      for (i = _originalRow; i >= _currentRow; i--)
		{
		  if ([tv _unselectRow: i])
		    {
		      notified = YES;
		    }
		}
	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		  
		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	    }
	  if (notified == YES)
	    {
	      [tv _postSelectionIsChangingNotification];
	    }
	}
      else // new multiple antiselection, after first pass
	{ 
	  NSInteger oldDiff, newDiff, i;

	  oldDiff = _oldRow - _originalRow;
	  newDiff = _currentRow - _originalRow;
	  if (oldDiff >= 0 && newDiff >= 0)
	    {
	      if (newDiff >= oldDiff)
		// we're extending the antiselection
		{
		  BOOL notified = NO;
		  for (i = _oldRow + 1; i <= _currentRow; i++)
		    {
		      if ([tv _unselectRow: i])
			{
			  notified = YES;
			}
		    }

		  if (*_selectedRow == -1)
		    {
			NSUInteger first = [_selectedRows firstIndex];
			
			if (first == NSNotFound)
			  {
			    *_selectedRow = -1;
			  }
			else
			  {
			    *_selectedRow = first;
			  }
		    }

		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i > _currentRow; i--)
		    {
		      if ([_oldSelectedRows containsIndex: i])
			{
			  // this row was in the old selection
			  // select it
			  [tv setNeedsDisplayInRect: [tv rectOfRow: i]];
			  [_selectedRows addIndex: i];
			  *_selectedRow = i;
			  notified = YES;
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff <= 0)
	    {
	      if (newDiff <= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;
		  for (i = _oldRow - 1; i >= _currentRow; i--)
		    {
		      if ([tv _unselectRow: i])
			{
			  notified = YES;
			}
		    }

		  if (*_selectedRow == -1)
		    {
		      NSUInteger first = [_selectedRows firstIndex];
		  
		      if (first == NSNotFound)
		        {
			  *_selectedRow = -1;
			}
		      else
		        {
			  *_selectedRow = first;
			}
		    }

		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i < _currentRow; i++)
		    {
		      if ([_oldSelectedRows containsIndex: i])
			{
			  // this row was in the old selection
			  // select it
			  [tv setNeedsDisplayInRect:
				[tv rectOfRow: i]];
			  [_selectedRows addIndex: i];
			  *_selectedRow = i;
			  notified = YES;
			}
		    }

		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff >= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      {
		for (i = _oldRow; i < _originalRow; i++)
		  {
		    if ([_oldSelectedRows containsIndex: i])
		      {
			// this row was in the old selection
			// select it
			[tv setNeedsDisplayInRect:
			      [tv rectOfRow: i]];
			[_selectedRows addIndex: i];
			*_selectedRow = i;
			notified = YES;
		      }
		  }
	      }
	      // then we're extending it
	      {
		for (i = _originalRow + 1; i <= _currentRow; i++)
		  {
		    if ([tv _unselectRow: i])
		      {
			notified = YES;
		      }
		  }
	      }

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		  
		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}

	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	  else if (oldDiff >= 0 && newDiff <= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      {
		for (i = _oldRow; i > _originalRow; i--)
		  {
		    if ([_oldSelectedRows containsIndex: i])
		      {
			// this row was in the old selection
			// select it
			[tv setNeedsDisplayInRect:
			      [tv rectOfRow: i]];
			[_selectedRows addIndex: i];
			*_selectedRow = i;
			notified = YES;
		      }
		  }
	      }
	      // then we're extending it
	      {
		for (i = _originalRow - 1; i >= _currentRow; i--)
		  {
		    if ([tv _unselectRow: i])
		      {
			notified = YES;
		      }
		  }
	      }

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		  
		  if (first == NSNotFound)
		    {
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}

	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
    }
  else if ((selectionMode & ALLOWS_MULTIPLE)
	   && (selectionMode & SHIFT_DOWN)
	   && ((selectionMode & ALLOWS_EMPTY) == 0)
	   && ((selectionMode & ADDING_ROW) == 0))
    {
      if (_oldRow == -1)
	// this is the first pass
	{
	  NSInteger diff, i;
	  NSUInteger count = [_selectedRows count];
	  BOOL notified = NO;
      	  diff = _currentRow - _originalRow;

	  if (diff >= 0)
	    {
	      for (i = _originalRow; i <= _currentRow; i++)
		{
		  if ((count > 1) && [tv  _unselectRow: i])
		    {
		      notified = YES;
		      count--;
		    }
		}
	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		  
		  if (first == NSNotFound)
		    {
		      NSLog(@"error!");
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	    }
	  else
	    {
	      // this case does happen (sometimes)
	      for (i = _originalRow; i >= _currentRow; i--)
		{
		  if ((count > 1) && [tv  _unselectRow: i])
		    {
		      notified = YES;
		      count--;
		    }
		}
	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		  
		  if (first == NSNotFound)
		    {
		      NSLog(@"error!");
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}
	    }
	  if (notified == YES)
	    {
	      [tv _postSelectionIsChangingNotification];
	    }
	}
      else // new multiple antiselection, after first pass
	{ 
	  NSInteger oldDiff, newDiff, i;
	  NSUInteger count = [_selectedRows count];
	  oldDiff = _oldRow - _originalRow;
	  newDiff = _currentRow - _originalRow;
	  if (oldDiff >= 0 && newDiff >= 0)
	    {
	      if (newDiff >= oldDiff)
		// we're extending the antiselection
		{
		  BOOL notified = NO;
		  for (i = _oldRow + 1; i <= _currentRow; i++)
		    {
		      if ((count > 1) && [tv  _unselectRow: i])
		        {
			  notified = YES;
			  count--;
			}
		    }

		  if (*_selectedRow == -1)
		    {
		      NSUInteger first = [_selectedRows firstIndex];
		  
		      if (first == NSNotFound)
		        {
			  NSLog(@"error!");
			  *_selectedRow = -1;
			}
		      else
		        {
			  *_selectedRow = first;
			}
		    }

		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i > _currentRow; i--)
		    {
		      if (([_oldSelectedRows containsIndex: i]))
			{
			  // this row was in the old selection
			  // select it
			  if ([tv _selectUnselectedRow: i])
			    {
			      notified = YES;
			    }
			}
		    }
		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff <= 0)
	    {
	      if (newDiff <= oldDiff)
		// we're extending the selection
		{
		  BOOL notified = NO;
		  for (i = _oldRow - 1; i >= _currentRow; i--)
		    {
		      if ((count > 1) && [tv  _unselectRow: i])
		        {
			  notified = YES;
			  count--;
			}
		    }

		  if (*_selectedRow == -1)
		    {
		      NSUInteger first = [_selectedRows firstIndex];
		      
		      if (first == NSNotFound)
		        {
			  NSLog(@"error!");
			  *_selectedRow = -1;
			}
		      else
		        {
			  *_selectedRow = first;
			}
		    }

		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	      else
		// we're reducing the selection
		{
		  BOOL notified = NO;

		  for (i = _oldRow; i < _currentRow; i++)
		    {
		      if ([_oldSelectedRows containsIndex: i])
			{
			  // this row was in the old selection
			  // select it
			  if ([tv _selectUnselectedRow: i])
			    {
			      notified = YES;
			    }
			  count++;
			}
		    }

		  if (notified == YES)
		    {
		      [tv _postSelectionIsChangingNotification];
		    }
		}
	    }
	  else if (oldDiff <= 0 && newDiff >= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      {
		for (i = _oldRow; i < _originalRow; i++)
		  {
		    if ([_oldSelectedRows containsIndex: i])
		      {
			// this row was in the old selection
			// select it
			if ([tv _selectUnselectedRow: i])
			  {
			    notified = YES;
			  }
		      }
		  }
	      }
	      // then we're extending it
	      {
		for (i = _originalRow + 1; i <= _currentRow; i++)
		  {
		    if ((count > 1) && [tv  _unselectRow: i])
		      {
			notified = YES;
			count--;
		      }
		  }
	      }

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		    
		  if (first == NSNotFound)
		    {
		      NSLog(@"error!");
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}

	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	  else if (oldDiff >= 0 && newDiff <= 0)
	    {
	      BOOL notified = NO;
	      
	      // we're reducing the selection
	      {
		for (i = _oldRow; i > _originalRow; i--)
		  {
		    if ([_oldSelectedRows containsIndex: i])
		      {
			// this row was in the old selection
			// select it
			if ([tv _selectUnselectedRow: i])
			  {
			    notified = YES;
			  }
		      }
		  }
	      }
	      // then we're extending it
	      {
		for (i = _originalRow - 1; i >= _currentRow; i--)
		  {
		    if ((count > 1) && [tv  _unselectRow: i])
		      {
			notified = YES;
			count--;
		      }
		  }
	      }

	      if (*_selectedRow == -1)
		{
		  NSUInteger first = [_selectedRows firstIndex];
		    
		  if (first == NSNotFound)
		    {
		      NSLog(@"error!");
		      *_selectedRow = -1;
		    }
		  else
		    {
		      *_selectedRow = first;
		    }
		}

	      if (notified == YES)
		{
		  [tv _postSelectionIsChangingNotification];
		}
	    }
	}
    }
}

@interface GSTableCornerView : NSView
{}
@end

@implementation GSTableCornerView

- (BOOL) isFlipped
{
  return YES;
}

- (void) drawRect: (NSRect)aRect
{
  [[GSTheme theme] drawTableCornerView: self withClip: aRect];
}

@end

@interface NSTableView (TableViewInternalPrivate)
- (void) _setSelectingColumns: (BOOL)flag;
- (BOOL) _editNextEditableCellAfterRow: (NSInteger)row
				column: (NSInteger)column;
- (BOOL) _editPreviousEditableCellBeforeRow: (NSInteger)row
				     column: (NSInteger)column;
- (void) _editNextCellAfterRow:(NSInteger)row inColumn:(NSInteger)column;
- (void) _autosaveTableColumns;
- (void) _autoloadTableColumns;
@end


@implementation NSTableView 

+ (void) initialize
{
  if (self == [NSTableView class])
    {
      [self setVersion: currentVersion];
      nc = [NSNotificationCenter defaultCenter];
      // FIXME
      [self exposeBinding: NSContentBinding];
      [self exposeBinding: NSSelectionIndexesBinding];
      [self exposeBinding: NSSortDescriptorsBinding];
    }
}

/*
 * Initializing/Releasing 
 */

- (void) _initDefaults
{
  _isValidating     = NO;
  _drawsGrid        = YES;
  _rowHeight        = 16.0;
  _intercellSpacing = NSMakeSize (5.0, 2.0);
  ASSIGN(_selectedColumns, [NSMutableIndexSet indexSet]);
  ASSIGN(_selectedRows, [NSMutableIndexSet indexSet]);
  _allowsEmptySelection = YES;
  _allowsMultipleSelection = NO;
  _allowsColumnSelection = YES;
  _allowsColumnResizing = YES;
  _allowsColumnReordering = YES;
  _autoresizesAllColumnsToFit = NO;
  _selectingColumns = NO;
  _verticalMotionDrag = NO;
  _editedColumn = -1;
  _editedRow = -1;
  _clickedRow = -1;
  _clickedColumn = -1;
  _selectedColumn = -1;
  _selectedRow = -1;
  _highlightedTableColumn = nil;
  _draggingSourceOperationMaskForLocal = NSDragOperationCopy 
      | NSDragOperationLink | NSDragOperationGeneric | NSDragOperationPrivate;
  _draggingSourceOperationMaskForRemote = NSDragOperationNone;
  ASSIGN(_sortDescriptors, [NSArray array]);
}

- (id) initWithFrame: (NSRect)frameRect
{
  self = [super initWithFrame: frameRect];
  if (!self)
    return self;

  [self _initDefaults];
  ASSIGN(_gridColor, [NSColor gridColor]); 
  ASSIGN(_backgroundColor, [NSColor controlBackgroundColor]); 
  ASSIGN(_tableColumns, [NSMutableArray array]);

  _headerView = [NSTableHeaderView new];
  [_headerView setFrameSize: NSMakeSize (frameRect.size.width, 22.0)];
  [_headerView setTableView: self];
  _cornerView = [GSTableCornerView new];
  [self tile];
  return self;
}

- (void) dealloc
{
  [self abortEditing];

  RELEASE (_gridColor);
  RELEASE (_backgroundColor);
  RELEASE (_tableColumns);
  RELEASE (_selectedColumns);
  RELEASE (_selectedRows);
  RELEASE (_sortDescriptors);
  TEST_RELEASE (_headerView);
  TEST_RELEASE (_cornerView);
  if (_autosaveTableColumns == YES)
    {
      [nc removeObserver: self 
	  name: NSTableViewColumnDidResizeNotification
	  object: self];
    }
  TEST_RELEASE (_autosaveName);
  if (_numberOfColumns > 0)
    {
      NSZoneFree (NSDefaultMallocZone (), _columnOrigins);
    }
  if (_delegate != nil)
    {
      [nc removeObserver: _delegate  name: nil  object: self];
      _delegate = nil;
    }
  [super dealloc];
}

- (BOOL) isFlipped
{
  return YES;
}

/*
 * Table Dimensions 
 */

- (NSInteger) numberOfColumns
{
  return _numberOfColumns;
}

- (NSInteger) numberOfRows
{
  return [self _numRows];
}

/* 
 * Columns 
 */

- (void) addTableColumn: (NSTableColumn *)aColumn
{
  [aColumn setTableView: self];
  [_tableColumns addObject: aColumn];
  _numberOfColumns++;
  if (_numberOfColumns > 1)
    {
      _columnOrigins = NSZoneRealloc (NSDefaultMallocZone (), _columnOrigins,
				      (sizeof (CGFloat)) * _numberOfColumns);
    }
  else 
    {
      _columnOrigins = NSZoneMalloc (NSDefaultMallocZone (), sizeof (CGFloat));
    }      
  [self tile];
}

- (void) removeTableColumn: (NSTableColumn *)aColumn
{
  NSInteger columnIndex = [self columnWithIdentifier: [aColumn identifier]];

  if (columnIndex == -1)
    {
      NSLog (@"Warning: Tried to remove not-existent column from table");
      return;
    }

  /* Remove selection on this column */
  [self deselectColumn: columnIndex];
  /* Shift column indexes on the right by one */
  if (_selectedColumn > columnIndex)
    {
      _selectedColumn--;
    }

  [_selectedColumns removeIndex: columnIndex];

  /* Now really remove the column */

  /* NB: Set table view to nil before removing the column from the
     array, because removing it from the array could deallocate it !  */
  [aColumn setTableView: nil];
  [_tableColumns removeObject: aColumn];
  _numberOfColumns--;
  if (_numberOfColumns > 0)
    {
      _columnOrigins = NSZoneRealloc (NSDefaultMallocZone (), _columnOrigins,
				      (sizeof (CGFloat)) * _numberOfColumns);
    }
  else 
    {
      NSZoneFree (NSDefaultMallocZone (), _columnOrigins);
    }      
  [self tile];
}

- (void) moveColumn: (NSInteger)columnIndex toColumn: (NSInteger)newIndex
{
  /* The range of columns which need to be shifted, 
     extremes included */
  NSInteger minRange, maxRange;
  /* Amount of shift for these columns */
  NSInteger shift;
  BOOL selected = NO;

  if ((columnIndex < 0) || (columnIndex > (_numberOfColumns - 1)))
    {
      NSLog (@"Attempt to move column outside table");
      return;
    }
  if ((newIndex < 0) || (newIndex > (_numberOfColumns - 1)))
    {
      NSLog (@"Attempt to move column to outside table");
      return;
    }

  if (columnIndex == newIndex)
    return;

  if (columnIndex > newIndex)
    {
      minRange = newIndex;
      maxRange = columnIndex - 1;
      shift = +1;
    }
  else // columnIndex < newIndex
    {
      minRange = columnIndex + 1;
      maxRange = newIndex;
      shift = -1;
    }

  /* Rearrange selection */
  if (_selectedColumn == columnIndex)
    {
      _selectedColumn = newIndex;
    }
  else if ((_selectedColumn >= minRange) && (_selectedColumn <= maxRange)) 
    {
      _selectedColumn += shift;
    }

  if ([_selectedColumns containsIndex: columnIndex])
    {
      selected = YES;
    }
  [_selectedColumns shiftIndexesStartingAtIndex: columnIndex + 1 by: -1];
  [_selectedColumns shiftIndexesStartingAtIndex: newIndex by: 1];
  if (selected)
    {
      [_selectedColumns addIndex: newIndex];
    }

  /* Update edited cell */
  if (_editedColumn == columnIndex)
    {
      _editedColumn = newIndex;
    }
  else if ((_editedColumn >= minRange) && (_editedColumn <= maxRange)) 
    {
      _editedColumn += shift;
    }

  /* Now really move the column */
  if (columnIndex < newIndex)
    {
      [_tableColumns insertObject: [_tableColumns objectAtIndex: columnIndex]
		     atIndex: newIndex + 1];
      [_tableColumns removeObjectAtIndex: columnIndex];
    }
  else
    {
      [_tableColumns insertObject: [_tableColumns objectAtIndex: columnIndex]
		     atIndex: newIndex];
      [_tableColumns removeObjectAtIndex: columnIndex + 1];
    }
  /* Tile */
  [self tile];

  /* Post notification */

  [self _postColumnDidMoveNotificationWithOldIndex: columnIndex
                                          newIndex: newIndex];

  [self _autosaveTableColumns];
}

- (NSArray *) tableColumns
{
  return AUTORELEASE ([_tableColumns mutableCopyWithZone: 
				       NSDefaultMallocZone ()]);
}

- (NSInteger) columnWithIdentifier: (id)identifier
{
  NSEnumerator	*enumerator = [_tableColumns objectEnumerator];
  NSTableColumn	*tb;
  NSInteger      return_value = 0;
  
  while ((tb = [enumerator nextObject]) != nil)
    {
      // Also handle a nil identifier.
      if ((identifier == [tb identifier]) || 
          [[tb identifier] isEqual: identifier])
        return return_value;
      else
        return_value++;
    }
  return -1;
}

- (NSTableColumn *) tableColumnWithIdentifier:(id)anObject
{
  NSInteger indexOfColumn = [self columnWithIdentifier: anObject];

  if (indexOfColumn == -1)
    return nil;
  else 
    return [_tableColumns objectAtIndex: indexOfColumn];
}

/* 
 * Data Source 
 */

- (id) dataSource
{
  return _dataSource;
}

- (void) setDataSource: (id)anObject
{
  /* Used only for readability */
  const SEL sel_a = @selector (numberOfRowsInTableView:);
  const SEL sel_b = @selector (tableView:objectValueForTableColumn:row:);
  const SEL sel_c = @selector(tableView:setObjectValue:forTableColumn:row:);
  GSKeyValueBinding *theBinding;

  // If we have content binding the data source is used only
  // like a delegate
  theBinding = [GSKeyValueBinding getBinding: NSContentBinding 
                                  forObject: self];
  if (theBinding == nil)
    { 
      if (anObject && [anObject respondsToSelector: sel_a] == NO) 
        {
          [NSException 
            raise: NSInternalInconsistencyException 
            format: @"Data Source doesn't respond to numberOfRowsInTableView:"];
        }
      
      if (anObject && [anObject respondsToSelector: sel_b] == NO) 
        {
          /* This method isn't required.
             [NSException raise: NSInternalInconsistencyException 
             format: @"Data Source doesn't respond to "
             @"tableView:objectValueForTableColumn:row:"];
          */  
        }
    }

  _dataSource_editable = [anObject respondsToSelector: sel_c];

  /* We do *not* retain the dataSource, it's like a delegate */
  _dataSource = anObject;

  [self tile];
  [self reloadData];
}

/* 
 * Loading data 
 */

- (void) reloadData
{
  [self noteNumberOfRowsChanged];
  [self setNeedsDisplay: YES];
}

/* 
 * Target-action 
 */

- (void) setAction: (SEL)aSelector
{
  _action = aSelector;
}

- (SEL) action
{
  return _action;
}

- (void) setDoubleAction: (SEL)aSelector
{
  _doubleAction = aSelector;
}

- (SEL) doubleAction
{
  return _doubleAction;
}

- (void) setTarget:(id)anObject
{
  _target = anObject;
}

- (id) target
{
  return _target;
}

- (NSInteger) clickedColumn
{
  return _clickedColumn;
}

- (NSInteger) clickedRow
{
  return _clickedRow;
}

/*
 * The NSTableHeaderView calls this method when it receives a double click.
 */

- (void) _sendDoubleActionForColumn: (NSInteger)columnIndex
{
  _clickedColumn = columnIndex;
  _clickedRow = -1;
  [self sendAction: _doubleAction  to: _target]; 
}

/*
 * And this when it gets a simple click which turns out to be for 
 * selecting/deselecting a column.
 * We don't support subtracting a column from the selection (Cocoa doesn't 
 * either).
 * However we support adding a distinct column with the control key (unlike 
 * Cocoa where the user can only make column range selection).
 */
- (void) _selectColumn: (NSInteger)columnIndex
	     modifiers: (unsigned int)modifiers
{
  NSIndexSet *oldIndexes = [self selectedColumnIndexes];
  BOOL addRange = ((modifiers & NSShiftKeyMask)
    && _allowsMultipleSelection && [oldIndexes count] > 0);
  BOOL addSingle = ((modifiers & NSControlKeyMask)
    && _allowsMultipleSelection);
  BOOL shouldSelect = ([self _shouldSelectionChange] 
    && [self _shouldSelectTableColumn: [_tableColumns objectAtIndex: columnIndex]]);
  NSIndexSet *newIndexes = [NSIndexSet indexSetWithIndex: columnIndex];

  if (_allowsColumnSelection == NO || shouldSelect == NO)
    {
      return;
    }

  if (_selectingColumns == NO)
    {
      [self _setSelectingColumns: YES];
    }

  /* Single select has priority over range select when both modifiers are pressed */
  if (addSingle)
    { 
      [self selectColumnIndexes: newIndexes byExtendingSelection: YES];
    }
  else if (addRange)
    {
      NSUInteger firstIndex = [oldIndexes firstIndex];
      NSUInteger lastIndex = [oldIndexes lastIndex];
      NSRange range;

      /* We extend the selection to the left or the right of the last selected 
         column. */
      if (columnIndex > [self selectedColumn])
        {
          lastIndex = columnIndex;
        }
      else
        {
          firstIndex = columnIndex;
        }

      range = NSMakeRange(firstIndex, lastIndex - firstIndex + 1);
      newIndexes = [NSIndexSet indexSetWithIndexesInRange: range]; 
      [self selectColumnIndexes: newIndexes byExtendingSelection: YES];
    }
  else
    {
      [self selectColumnIndexes: newIndexes byExtendingSelection: NO];
    }
}


/*
 *Configuration 
 */ 

- (void) setAllowsColumnReordering: (BOOL)flag
{
  _allowsColumnReordering = flag;
}

- (BOOL) allowsColumnReordering
{
  return _allowsColumnReordering;
}

- (void) setAllowsColumnResizing: (BOOL)flag
{
  _allowsColumnResizing = flag;
}

- (BOOL) allowsColumnResizing
{
  return _allowsColumnResizing;
}

- (void) setAllowsMultipleSelection: (BOOL)flag
{
  _allowsMultipleSelection = flag;
}

- (BOOL) allowsMultipleSelection
{
  return _allowsMultipleSelection;
}

- (void) setAllowsEmptySelection: (BOOL)flag
{
  _allowsEmptySelection = flag;
}

- (BOOL) allowsEmptySelection
{
  return _allowsEmptySelection;
}

- (void) setAllowsColumnSelection: (BOOL)flag
{
  _allowsColumnSelection = flag;
}

- (BOOL) allowsColumnSelection
{
  return _allowsColumnSelection;
}

/* 
 * Drawing Attributes 
 */

- (void) setIntercellSpacing: (NSSize)aSize
{
  _intercellSpacing = aSize;
  [self setNeedsDisplay: YES];
}

- (NSSize) intercellSpacing
{
  return _intercellSpacing;
}

- (void) setRowHeight: (CGFloat)rowHeight
{
  _rowHeight = rowHeight;
  [self tile];
}

- (CGFloat) rowHeight
{
  return _rowHeight;
}

- (void) setBackgroundColor: (NSColor *)aColor
{
  ASSIGN (_backgroundColor, aColor);
}

- (NSColor *) backgroundColor
{
  return _backgroundColor;
}

- (void) setUsesAlternatingRowBackgroundColors: (BOOL)useAlternatingRowColors
{
  // FIXME
}

- (BOOL) usesAlternatingRowBackgroundColors
{
  // FIXME
  return NO;
}

- (void)setSelectionHighlightStyle: (NSTableViewSelectionHighlightStyle)s
{
  // FIXME implement me really
  _selectionHighlightStyle = s;
  if (_selectionHighlightStyle == NSTableViewSelectionHighlightStyleSourceList)
    {
      // should also set draggingDestinationFeedbackStyle to NSTableViewDraggingDestinationFeedbackStyleSourceList
      // but we don't have it yet anyway
    }
}

- (NSTableViewSelectionHighlightStyle) selectionHighlightStyle
{
  return _selectionHighlightStyle;
}

/*
 * Selecting Columns and Rows
 */
- (void) selectColumn: (NSInteger)columnIndex 
 byExtendingSelection: (BOOL)flag
{
  if (columnIndex < 0 || columnIndex > _numberOfColumns)
    {
      NSDebugLLog(@"NSTableView", @"Column index %d out of table in selectColumn", (int)columnIndex);
      return;
    }

  _selectingColumns = YES;

  if (flag == NO)
    {
      /* If the current selection is the one we want, just ends editing
       * This is not just a speed up, it prevents us from sending
       * a NSTableViewSelectionDidChangeNotification.
       * This behaviour is required by the specifications */
      if ([_selectedColumns count] == 1
	  && [_selectedColumns containsIndex: columnIndex] == YES)
	{
	  /* Stop editing if any */
	  if (_textObject != nil)
	    {
	      [self validateEditing];
	      [self abortEditing];
	    }  
	  return;
	} 

      /* If _numberOfColumns == 1, we can skip trying to deselect the
	 only column - because we have been called to select it. */
      if (_numberOfColumns > 1)
	{
	  [self _unselectAllColumns];
	}
    }
  else // flag == YES
    {
      if (_allowsMultipleSelection == NO)
	{
	  [NSException raise: NSInternalInconsistencyException
		       format: @"Can not extend selection in table view when multiple selection is disabled"];  
	}
    }

  /* Stop editing if any */
  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }

  /* Now select the column and post notification only if needed */ 
  if ([_selectedColumns containsIndex: columnIndex] == NO)
    {
      [_selectedColumns addIndex: columnIndex];
      _selectedColumn = columnIndex;

      [self setNeedsDisplayInRect: [self rectOfColumn: columnIndex]];
      if (_headerView)
	{
	  [_headerView setNeedsDisplayInRect: 
			 [_headerView headerRectOfColumn: columnIndex]];
	}
      [self _postSelectionDidChangeNotification];
    }
  else /* Otherwise simply change the last selected column */
    {
      _selectedColumn = columnIndex;
    }
}

- (void) selectRow: (NSInteger)rowIndex
byExtendingSelection: (BOOL)flag
{
  if (rowIndex < 0 || rowIndex >= _numberOfRows)
    {
      NSDebugLLog(@"NSTableView", @"Row index %d out of table in selectRow", (int)rowIndex);
      return;
    }

  if (_selectingColumns)
    {
      _selectingColumns = NO;
      if (_headerView)
	{
	  [_headerView setNeedsDisplay: YES];
	}
    }

  if (flag == NO)
    {
      /* If the current selection is the one we want, just ends editing
       * This is not just a speed up, it prevents us from sending
       * a NSTableViewSelectionDidChangeNotification.
       * This behaviour is required by the specifications */
      if ([_selectedRows count] == 1
	  && [_selectedRows containsIndex: rowIndex] == YES)
	{
	  /* Stop editing if any */
	  if (_textObject != nil)
	    {
	      [self validateEditing];
	      [self abortEditing];
	    }

	   /* reset the _clickedRow for keyboard navigation  */
	  _clickedRow = rowIndex;
	  return;
	} 

      /* If _numberOfRows == 1, we can skip trying to deselect the
	 only row - because we have been called to select it. */
      if (_numberOfRows > 1)
	{
	  [self _unselectAllRows];
	}
    }
  else // flag == YES
    {
      if (_allowsMultipleSelection == NO)
	{
	  [NSException raise: NSInternalInconsistencyException
		       format: @"Can not extend selection in table view when multiple selection is disabled"];  
	}
    }

  /* Stop editing if any */
  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }  

  /*
   * Now select the row and post notification only if needed
   * also update the _clickedRow for keyboard navigation.
   */ 
  if ([self _selectUnselectedRow: rowIndex])
    {
      _clickedRow = rowIndex;
      [self _postSelectionDidChangeNotification];
    }
  else /* Otherwise simply change the last selected row */
    {
      _selectedRow = rowIndex;
      _clickedRow = rowIndex;
    }
}

- (void) selectColumnIndexes: (NSIndexSet *)indexes byExtendingSelection: (BOOL)extend
{
  BOOL empty = ([indexes firstIndex] == NSNotFound);
  BOOL changed = NO;
  NSUInteger col;
  
  if (!_selectingColumns)
    {
      _selectingColumns = YES;
      if (_headerView)
	{
	  [_headerView setNeedsDisplay: YES];
	}
    }

  /* Stop editing if any */
  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }
  
  if (extend == NO)
    {
      /* If the current selection is the one we want, just ends editing
       * This is not just a speed up, it prevents us from sending
       * a NSTableViewSelectionDidChangeNotification.
       * This behaviour is required by the specifications */
      if ([_selectedColumns isEqual: indexes])
        {
	  if (!empty)
	    {
	      _selectedColumn = [indexes lastIndex];
	    }
	  return;
	}

      [self _unselectAllColumns];
      changed = YES;
    }

  if (!empty)
    {
      if ([indexes lastIndex] >= _numberOfColumns)
        {
	  [NSException raise: NSInvalidArgumentException
                      format: @"Column index out of table in selectColumn"];
	}

      /* This check is not fully correct, as both sets may contain just 
	 the same entry, but works according to the old specification. */
      if (_allowsMultipleSelection == NO && 
	  [_selectedColumns count] + [indexes count] > 1)
        {
	  [NSException raise: NSInternalInconsistencyException
                      format: @"Can not set multiple selection in table view when multiple selection is disabled"];  
	}

      col = [indexes firstIndex];
      while (col != NSNotFound)
        {
	  if (![_selectedColumns containsIndex: col])
	    {
	      [self setNeedsDisplayInRect: [self rectOfColumn: col]];
	      if (_headerView)
	        {
		  [_headerView setNeedsDisplayInRect: 
				   [_headerView headerRectOfColumn: col]];
		}
	      changed = YES;
	    }
	  col = [indexes indexGreaterThanIndex: col];
	}
      [_selectedColumns addIndexes: indexes];
      _selectedColumn = [indexes lastIndex];
    }

  if (changed)
    {
      [self _postSelectionDidChangeNotification];
    }
}

- (void) selectRowIndexes: (NSIndexSet *)indexes byExtendingSelection: (BOOL)extend
{
  BOOL empty = ([indexes firstIndex] == NSNotFound);
  BOOL changed = NO;
  NSUInteger row;
  
  if (_selectingColumns)
    {
      _selectingColumns = NO;
      if (_headerView)
	{
	  [_headerView setNeedsDisplay: YES];
	}
    }

  /* Stop editing if any */
  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }
  
  if (extend == NO)
    {
      /* If the current selection is the one we want, just ends editing
       * This is not just a speed up, it prevents us from sending
       * a NSTableViewSelectionDidChangeNotification.
       * This behaviour is required by the specifications */
      if ([_selectedRows isEqual: indexes])
        {
	  if (!empty)
	    {
	      _selectedRow = [indexes lastIndex];
	    }
	  return;
	}

      [self _unselectAllRows];
      changed = YES;
    }

  if (!empty)
    {
      if ([indexes lastIndex] >= _numberOfRows)
        {
	  [NSException raise: NSInvalidArgumentException
                      format: @"Row index out of table in selectRow"];
	}

      /* This check is not fully correct, as both sets may contain just 
	 the same entry, but works according to the old specification. */
      if (_allowsMultipleSelection == NO && 
	  [_selectedRows count] + [indexes count] > 1)
        {
	  [NSException raise: NSInternalInconsistencyException
		       format: @"Can not set multiple selection in table view when multiple selection is disabled"];  
	}

      row = [indexes firstIndex];
      while (row != NSNotFound)
        {
	  if (![_selectedRows containsIndex: row])
	    {
	      [self setNeedsDisplayInRect: [self rectOfRow: row]];
	    }
	  row = [indexes indexGreaterThanIndex: row];
	}
      [_selectedRows addIndexes: indexes];
      _selectedRow = [indexes lastIndex];
      changed = YES;
    }

  if (changed)
    {
      [self _postSelectionDidChangeNotification];
    }
}

- (NSIndexSet *) selectedColumnIndexes
{
  return [[_selectedColumns copy] autorelease];
}

- (NSIndexSet *) selectedRowIndexes
{
  return [[_selectedRows copy] autorelease];
}

- (void) deselectColumn: (NSInteger)columnIndex
{
  if ([_selectedColumns containsIndex: columnIndex] == NO)
    {
      return;
    }

  /* Now by internal consistency we assume columnIndex is in fact a
     valid column index, since it was the index of a selected column */

  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }
  
  _selectingColumns = YES;

  [_selectedColumns removeIndex: columnIndex];

  if (_selectedColumn == columnIndex)
    {
      NSUInteger less = [_selectedColumns indexLessThanIndex: columnIndex];
      NSUInteger greater = [_selectedColumns indexGreaterThanIndex: columnIndex];

      if (less == NSNotFound)
        {
	  if (greater == NSNotFound)
	    {
	      _selectedColumn = -1;
	    }
	  else
	    {
	      _selectedColumn = greater;		
	    }  
	}
      else if (greater == NSNotFound)
        {
	  _selectedColumn = less;
	}
      else if (columnIndex - less > greater - columnIndex)
        {
	  _selectedColumn = greater;		
	}
      else 
        {
	  _selectedColumn = less;
	}
    }
      
  [self setNeedsDisplayInRect: [self rectOfColumn: columnIndex]];
  if (_headerView)
    {
      [_headerView setNeedsDisplayInRect: 
		     [_headerView headerRectOfColumn: columnIndex]];
    }

  [self _postSelectionDidChangeNotification];
}

- (void) deselectRow: (NSInteger)rowIndex
{
  if ([_selectedRows containsIndex: rowIndex] == NO)
    {
      return;
    }

  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }

  _selectingColumns = NO;

  [_selectedRows removeIndex: rowIndex];

  if (_selectedRow == rowIndex)
    {
      NSUInteger less = [_selectedRows indexLessThanIndex: rowIndex];
      NSUInteger greater = [_selectedRows indexGreaterThanIndex: rowIndex];

      if (less == NSNotFound)
        {
	  if (greater == NSNotFound)
	    {
	      _selectedRow = -1;
	    }
	  else
	    {
	      _selectedRow = greater;		
	    }  
	}
      else if (greater == NSNotFound)
        {
	  _selectedRow = less;
	}
      else if (rowIndex - less > greater - rowIndex)
        {
	  _selectedRow = greater;		
	}
      else 
        {
	  _selectedRow = less;
	}
    }

  [self _postSelectionDidChangeNotification];
}

- (NSInteger) numberOfSelectedColumns
{
  return [_selectedColumns count];
}

- (NSInteger) numberOfSelectedRows
{
  return [_selectedRows count];
}

- (NSInteger) selectedColumn
{
  return _selectedColumn;
}

- (NSInteger) selectedRow
{
  return _selectedRow;
}

- (BOOL) isColumnSelected: (NSInteger)columnIndex
{
  return [_selectedColumns containsIndex: columnIndex];
}

- (BOOL) isRowSelected: (NSInteger)rowIndex
{
  return [_selectedRows containsIndex: rowIndex];
}

- (NSEnumerator *) selectedColumnEnumerator
{
  return [[self _selectedColumArray] objectEnumerator];
}

- (NSEnumerator *) selectedRowEnumerator
{
  return [[self _selectedRowArray] objectEnumerator];
}

- (void) selectAll: (id) sender
{
  if (_allowsMultipleSelection == NO)
    return;

  /* Ask the delegate if we can select all columns or rows */
  if (_selectingColumns == YES)
    {
      if ([_selectedColumns count] == (NSUInteger)_numberOfColumns)
	{
	  // Nothing to do !
	  return;
	}

      {
	NSEnumerator *enumerator = [_tableColumns objectEnumerator];
	NSTableColumn *tb;
	while ((tb = [enumerator nextObject]) != nil)
	  {
	    if ([self _shouldSelectTableColumn: tb] == NO)
	      {
		return;
	      }
	  }
      }
    }
  else // selecting rows
    {
      if ([_selectedRows count] == (NSUInteger)_numberOfRows)
	{
	  // Nothing to do !
	  return;
	}

      {
	NSInteger row; 
	
	for (row = 0; row < _numberOfRows; row++)
	  {
	    if ([self _shouldSelectRow: row] == NO)
	      return;
	  }
      }
    }

  /* Stop editing if any */
  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }  

  /* Do the real selection */
  if (_selectingColumns == YES)
    {
      [_selectedColumns removeAllIndexes];
      [_selectedColumns addIndexesInRange: NSMakeRange(0, _numberOfColumns)];
    }
  else // selecting rows
    {
      [_selectedRows removeAllIndexes];
      [_selectedRows addIndexesInRange: NSMakeRange(0, _numberOfRows)];
    }
  
  [self setNeedsDisplay: YES];
  [self _postSelectionDidChangeNotification];
}

- (void) deselectAll: (id) sender
{
  if (_allowsEmptySelection == NO)
    return;

  if ([self _shouldSelectionChange] == NO)
    {
      return;
    }
  
  if (_textObject != nil)
    {
      [self validateEditing];
      [self abortEditing];
    }
	  
  if (([_selectedColumns count] > 0) || ([_selectedRows count] > 0))
    {
      [_selectedColumns removeAllIndexes];
      [_selectedRows removeAllIndexes];
      _selectedColumn = -1;
      _selectedRow = -1;
      _selectingColumns = NO;
      [self setNeedsDisplay: YES];
      [self _postSelectionDidChangeNotification];
    }
  else
    {
      _selectedColumn = -1;
      _selectedRow = -1;
      _selectingColumns = NO;
    }
}

/* 
 * Grid Drawing attributes 
 */

- (void) setDrawsGrid: (BOOL)flag
{
  _drawsGrid = flag;
}

- (BOOL) drawsGrid
{
  return _drawsGrid;
}

- (void) setGridColor: (NSColor *)aColor
{
  ASSIGN (_gridColor, aColor);
}

- (NSColor *) gridColor
{
  return _gridColor;
}

- (void) setGridStyleMask: (NSTableViewGridLineStyle)gridType
{
  // FIXME
}

- (NSTableViewGridLineStyle) gridStyleMask
{
  // FIXME
  return 0;
}

/*
 * Providing Cells
 */

- (NSCell *) preparedCellAtColumn: (NSInteger)columnIndex row: (NSInteger)rowIndex
{
  NSCell *cell = nil;
  NSTableColumn *tb = [_tableColumns objectAtIndex: columnIndex];

  if ([_delegate respondsToSelector: 
        @selector(tableView:dataCellForTableColumn:row:)])
    {
      cell = [_delegate tableView: self dataCellForTableColumn: tb
                                                           row: rowIndex];
    }
  if (cell == nil)
    {
      cell = [tb dataCellForRow: rowIndex];
    }
  return cell;
}

/* 
 * Editing Cells 
 */

- (BOOL) abortEditing
{ 
  if (_textObject)
    {
      [_editedCell endEditing: _textObject];
      DESTROY(_editedCell);
      [self setNeedsDisplayInRect: 
	      [self frameOfCellAtColumn: _editedColumn row: _editedRow]];
      _editedRow = -1;
      _editedColumn = -1;
      _textObject = nil;
      return YES;
    }
  else
    return NO;
}

- (NSText *) currentEditor
{
  if (_textObject && ([_window firstResponder] == _textObject))
    return _textObject;
  else
    return nil;
}

- (void) validateEditing
{
  if (_textObject && (_isValidating == NO))
    {
      NSFormatter *formatter;
      NSString *string;
      id newObjectValue = nil;
      BOOL validatedOK = YES;

      // Avoid potential recursive sequences...
      _isValidating = YES;
      
      formatter = [_editedCell formatter];
      string = AUTORELEASE([[_textObject text] copy]);

      if (formatter != nil)
        {
          NSString *error;
	  
          if ([formatter getObjectValue: &newObjectValue 
                         forString: string 
                         errorDescription: &error] == YES)
            {
              [_editedCell setObjectValue: newObjectValue];
              
              if (_dataSource_editable)
                {
                  NSTableColumn *tb;
              
                  tb = [_tableColumns objectAtIndex: _editedColumn];
                  
                  [self _setObjectValue: newObjectValue
                        forTableColumn: tb
                        row: _editedRow];
                }

              _isValidating = NO;
              return;
            }
          else
            {
              SEL sel = @selector(control:didFailToFormatString:errorDescription:);

              if ([_delegate respondsToSelector: sel])
                {
                  validatedOK = [_delegate control: self 
                                           didFailToFormatString: string 
                                           errorDescription: error];
                }
              // Allow an empty string to fall through
              else if (![string isEqualToString: @""])
                {
                  validatedOK = NO;
                }
            }
        }

      if (validatedOK)
        {
          [_editedCell setStringValue: string];
          
          if (_dataSource_editable)
            {
              NSTableColumn *tb;
              
              tb = [_tableColumns objectAtIndex: _editedColumn];
              
              [self _setObjectValue: string // newObjectValue
                    forTableColumn: tb
                    row: _editedRow];
            }
        }

      // Avoid potential recursive sequences...
      _isValidating = NO;
    }
}

- (void) editColumn: (NSInteger) columnIndex 
                row: (NSInteger) rowIndex 
          withEvent: (NSEvent *) theEvent 
             select: (BOOL) flag
{
  NSText *t;
  NSTableColumn *tb;
  NSRect drawingRect;
  NSUInteger length = 0;

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

  tb = [_tableColumns objectAtIndex: columnIndex];
  [_editedCell setObjectValue: [self _objectValueForTableColumn: tb
				     row: rowIndex]];
  
  // But of course the delegate can mess it up if it wants
  [self _willDisplayCell: _editedCell
	forTableColumn: tb
	row: rowIndex];

  /* Please note the important point - calling stringValue normally
     causes the _editedCell to call the validateEditing method of its
     control view ... which happens to be this NSTableView object :-)
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

- (NSInteger) editedRow
{
  return _editedRow;  
}

- (NSInteger) editedColumn
{
  return _editedColumn;
}


static inline NSTimeInterval computePeriod(NSPoint mouseLocationWin,
                                           CGFloat minYVisible, 
                                           CGFloat maxYVisible)
{
    /* We have three zones of speed. 
       0   -  50 pixels: period 0.2  <zone 1>
       50  - 100 pixels: period 0.1  <zone 2>
       100 - 150 pixels: period 0.01 <zone 3> */
    CGFloat distance = 0;
    
    if (mouseLocationWin.y < minYVisible) 
      {
	distance = minYVisible - mouseLocationWin.y; 
      }
    else if (mouseLocationWin.y > maxYVisible)
      {
	distance = mouseLocationWin.y - maxYVisible;
      }
    
    if (distance < 50)
      return 0.2;
    else if (distance < 100)
      return 0.1;
    else 
      return 0.01;
}

- (void) _trackCellAtColumn: (NSInteger) columnIndex
		row: (NSInteger) rowIndex
		withEvent: (NSEvent *) theEvent
{
  if (rowIndex == -1 || columnIndex == -1)
    {
      return;
    }
  
  /* we should copy the cell here, as we do on editing.
     otherwise validation on a cell being edited could
     cause the cell we are selecting to get it's objectValue */
  NSCell *cell = [[self preparedCellAtColumn: columnIndex row: rowIndex] copy];
  NSTableColumn *tb = [_tableColumns objectAtIndex: columnIndex];
  id originalValue = RETAIN([self _objectValueForTableColumn: tb
                                                         row: rowIndex]);
  [cell setObjectValue: originalValue]; 
  NSRect cellFrame = [self frameOfCellAtColumn: columnIndex
                                           row: rowIndex];
  [cell setHighlighted: YES];
  [self setNeedsDisplayInRect: cellFrame];
  /* give delegate a chance to i.e set target */
  [self _willDisplayCell: cell
		forTableColumn: tb
	 	row: rowIndex];
   
  if ([cell trackMouse: theEvent 
		   inRect: cellFrame
		   ofView: self
	     untilMouseUp: [[cell class]
			     prefersTrackingUntilMouseUp]])
    {
      id newValue = [cell objectValue];

      /* don't check editability that only pertains to editColumn:... */
      if (originalValue != newValue
	  && ![originalValue isEqual: newValue])
        {
	  [self _setObjectValue: newValue 
			forTableColumn: tb
			row: rowIndex];
	}
    }
  RELEASE(originalValue);    
  [cell setHighlighted: NO];
  [self setNeedsDisplayInRect: cellFrame];
  RELEASE(cell);
}

- (BOOL) _startDragOperationWithEvent: (NSEvent *) theEvent
{
  NSPasteboard *pboard = [NSPasteboard pasteboardWithName: NSDragPboard];
  NSPoint startPoint = [self convertPoint: [theEvent locationInWindow] 
                                 fromView: nil];

  if ([self canDragRowsWithIndexes: _selectedRows atPoint: startPoint]
    && [self _writeRows: _selectedRows toPasteboard: pboard])
    {
      NSPoint	p = NSZeroPoint;
      NSImage	*dragImage;
      NSSize	s;
      // FIXME
      NSArray *cols = nil;

      dragImage = [self dragImageForRowsWithIndexes: _selectedRows
                        tableColumns: cols
                        event: theEvent
                        offset: &p];

      /*
       * Store image offset in s ... the returned
       * value is the position of the center of
       * the image, so we adjust to the bottom left
       * corner.
       */
       s = [dragImage size];
       s.width = p.x - s.width/2;
       s.height = p.y + s.height/2; // View is flipped

       /*
	* Reuse the current mouse location and adjust
	* it to determine the location of the bottom
	* left corner of the image in this view's
	* coordinate system.
	*/
       p = startPoint;
       p.x += s.width;
       p.y += s.height;
	

       [self dragImage: dragImage
		    at: p
		offset: NSMakeSize(0, 0)
		 event: theEvent
	    pasteboard: pboard
	        source: self
	     slideBack: YES];
      return YES;
    }
  return NO;
}

- (void) mouseDown: (NSEvent *)theEvent
{
  NSPoint initialLocation = [theEvent locationInWindow];
  NSPoint location;
  NSInteger clickCount = [theEvent clickCount];

  // Pathological case -- ignore mouse down
  if ((_numberOfRows == 0) || (_numberOfColumns == 0))
    {
      return; 
    }
  
  /* Stop editing if any */
  if (_textObject != nil)
    {
      if (_editedCell != nil 
          && [_editedCell isEntryAcceptable:[_textObject text]] == NO)
        {
          NSBeep();
          return;
        }
      [self validateEditing];
      [self abortEditing];
    }  

  // Determine row and column which were clicked
  location = [self convertPoint: initialLocation fromView: nil];
  _clickedRow  = [self rowAtPoint: location];
  _clickedColumn = [self columnAtPoint: location];
  
  if ([theEvent type] == NSLeftMouseDown
       && clickCount > 1)
    {
      // Double-click event

      if (![self isRowSelected: _clickedRow])
        {
	  return;
	}

      if (![self _isCellSelectableColumn: _clickedColumn row: _clickedRow])
        {
	  // Send double-action but don't edit
	  [self _trackCellAtColumn: _clickedColumn
                               row: _clickedRow
                         withEvent: theEvent];
	  if (_clickedRow != -1)
	    [self sendAction: _doubleAction to: _target];
	}
      else if (clickCount == 2) // if < 2, dont want to abort editing
        {
	  // It is OK to edit column.  Go on, do it.
          [self editColumn: _clickedColumn
		   row: _clickedRow
		   withEvent: theEvent
		   select: YES];
	}
    }
  else 
    {
#define COMPUTE_NEW_SELECTION do { \
if (originalRow == -1) \
  { \
    originalRow = currentRow; \
  } \
if (currentRow >= 0 && currentRow < _numberOfRows) \
  { \
    computeNewSelection(self, \
    oldSelectedRows, \
    _selectedRows, \
    originalRow, \
    oldRow, \
    currentRow, \
    &_selectedRow, \
    selectionMode); \
    [self displayIfNeeded]; \
  } \
} while (0);

      // Selection
      NSUInteger modifiers = [theEvent modifierFlags];
      NSUInteger eventMask = (NSLeftMouseUpMask 
				| NSLeftMouseDownMask
				| NSLeftMouseDraggedMask 
				| NSPeriodicMask);
      unsigned selectionMode = 0;
      NSPoint mouseLocationWin;
      NSPoint mouseLocationView;
      NSDate *distantFuture = [NSDate distantFuture];
      NSEvent *lastEvent;
      NSIndexSet *oldSelectedRows;
      BOOL startedPeriodicEvents = NO;
      BOOL mouseBelowView = NO;
      BOOL done = NO;
      BOOL mouseMoved = NO;
      BOOL didTrackCell = NO;
      BOOL dragOperationPossible = [self _isDraggingSource];
      NSRect visibleRect = [self convertRect: [self visibleRect]
				 toView: nil];
      CGFloat minYVisible = NSMinY (visibleRect);
      CGFloat maxYVisible = NSMaxY (visibleRect);
      NSTimeInterval oldPeriod = 0;
      NSInteger originalRow = _clickedRow;
      NSInteger oldRow = -1;
      NSInteger currentRow = -1;
      BOOL getNextEvent = YES;
      BOOL sendAction = NO;

      if (_allowsMultipleSelection == YES)
	{
	  selectionMode |= ALLOWS_MULTIPLE;
	}

      if (_allowsEmptySelection == YES)
	{
	  selectionMode |= ALLOWS_EMPTY;
	}
      
      if (modifiers & NSShiftKeyMask)
	{
	  selectionMode |= SHIFT_DOWN;
	}
      
      if (![_selectedRows containsIndex: _clickedRow])
	{
	  selectionMode |= ADDING_ROW;
	}

      if (modifiers & NSControlKeyMask)
	{
	  selectionMode |= CONTROL_DOWN;
	  if (_allowsMultipleSelection == YES && _selectedRow != -1)
	    {
	      originalRow = _selectedRow;
	      selectionMode |= SHIFT_DOWN;
	      selectionMode |= ADDING_ROW;
	    }
	}
      
      // is the delegate ok for a new selection ?
      if ([self _shouldSelectionChange] == NO)
	{
	  return;
	}

      // if we are in column selection mode, stop it
      [self _setSelectingColumns: NO];

      // let's sort the _selectedRows
      oldSelectedRows = [_selectedRows copy];
      lastEvent = theEvent;

      while (done != YES)
	{
	  /*
	    Wrap each iteration in an autorelease pool. Otherwise, we end
	    up allocating huge amounts of objects if the button is held
	    down for a long time.
	  */
	  CREATE_AUTORELEASE_POOL(arp);
	  NSEventType eventType = [lastEvent type];
	  
	  mouseLocationWin = [lastEvent locationInWindow]; 
	  mouseLocationView = [self convertPoint: mouseLocationWin 
					    fromView: nil];
	
	  switch (eventType)
	    {
	    case NSLeftMouseUp:
	      if ((mouseLocationWin.y > minYVisible) 
		  && (mouseLocationWin.y < maxYVisible))
		{
		  // mouse up within table
		  if (startedPeriodicEvents == YES)
		    {
		      [NSEvent stopPeriodicEvents];
		      startedPeriodicEvents = NO;
		    }
		  mouseLocationView.x = _bounds.origin.x;
		  oldRow = currentRow;
		  currentRow = [self rowAtPoint: mouseLocationView];
		  
		  if (oldRow != currentRow)
		    {
		      COMPUTE_NEW_SELECTION;
		    }
		  
		  if (!didTrackCell && currentRow == _clickedRow)
		    {
		      /*
		       * a dragging operation is still possible so
		       * selections were never dragged,
		       * and a drag operation was never attempted.
		       * the cell was clicked, 
		       * track the cell with the old mouseDown event
		       * then it will get the current event mouseUp.
		       */
		      [self _trackCellAtColumn: _clickedColumn
			    row: _clickedRow
			    withEvent: theEvent];
		    }
		}
	      else
		{
		  // Mouse dragged out of the table
		  // we don't care
		}
	      done = YES;
	      break;

	    case NSLeftMouseDown:
	    case NSLeftMouseDragged:
	      if (fabs(mouseLocationWin.x - initialLocation.x) > 1
	          || fabs(mouseLocationWin.y - initialLocation.y) > 1)
		{
		  mouseMoved = YES;
		}

	      if (dragOperationPossible == YES)
		{
		  if ([_selectedRows containsIndex:_clickedRow] == NO
 		      || (_verticalMotionDrag == NO
 			  && fabs(mouseLocationWin.y - initialLocation.y) > 2))
		    {
		      dragOperationPossible = NO;
		    }
 		  else if ((fabs(mouseLocationWin.x - initialLocation.x) >= 4)
 			   || (_verticalMotionDrag
 			       && fabs(mouseLocationWin.y - initialLocation.y) >= 4))
		    {
		      if ([self _startDragOperationWithEvent: theEvent])
			{
                          RELEASE(oldSelectedRows);
                          IF_NO_GC(DESTROY(arp));
			  return;
			}
		      else
			{
			  dragOperationPossible = NO;
			}
		    }
		}
	      else if ((mouseLocationWin.y > minYVisible) 
		       && (mouseLocationWin.y < maxYVisible))
		{
		  // mouse dragged within table
		  if (startedPeriodicEvents == YES)
		    {
		      [NSEvent stopPeriodicEvents];
		      startedPeriodicEvents = NO;
		    }

		  mouseLocationView.x = _bounds.origin.x;
		  oldRow = currentRow;
		  currentRow = [self rowAtPoint: mouseLocationView];
		  if (oldRow != currentRow)
		    {
		      COMPUTE_NEW_SELECTION;
		    }
		  
		  if (eventType == NSLeftMouseDown)
		    {
		      /*
		       * Can never get here from a dragging source
		       * so they need to track in mouse up.
		       */
		      NSCell *cell = [self preparedCellAtColumn: _clickedColumn 
                                                            row: _clickedRow];
		      
		      [self _trackCellAtColumn: _clickedColumn
				row: _clickedRow
				withEvent: theEvent];
		      didTrackCell = YES;

		      if ([[cell class] prefersTrackingUntilMouseUp])
		        {
		          /* the mouse could have gone up outside of the cell
		           * avoid selecting the row under mouse cursor */ 
			  sendAction = YES;
			  done = YES;
			}
		    }
		  /*
		   * Since we may have tracked a cell which may have caused
		   * a change to the currentEvent we may need to loop over
		   * the current event
		   */ 
		  getNextEvent = (lastEvent == [NSApp currentEvent]);
		}
	      else
		{
		  // Mouse dragged out of the table
		  NSTimeInterval period = computePeriod(mouseLocationWin, 
					       minYVisible, 
					       maxYVisible);

		  if (startedPeriodicEvents == YES)
		    {
		      /* Check - if the mouse did not change zone, 
			 we do nothing */
		      if (period == oldPeriod)
			break;

		      [NSEvent stopPeriodicEvents];
		    }
		  /* Start periodic events */
		  oldPeriod = period;
		  [NSEvent startPeriodicEventsAfterDelay: 0
			   withPeriod: oldPeriod];
		  startedPeriodicEvents = YES;
		  if (mouseLocationWin.y <= minYVisible) 
		    mouseBelowView = YES;
		  else
		    mouseBelowView = NO;
		}
	      break;
	    case NSPeriodic:
	      if (mouseBelowView == YES)
		{
		  if (currentRow == -1 && oldRow != -1)
		    currentRow = oldRow + 1;

		  if (currentRow != -1 && currentRow < _numberOfRows - 1)
		    {
		      oldRow = currentRow;
		      currentRow++;
		      [self scrollRowToVisible: currentRow];
		      if (dragOperationPossible == NO)
			COMPUTE_NEW_SELECTION;
		    }
		}
	      else
		{
		  if (currentRow == -1 && oldRow != -1)
		    currentRow = oldRow - 1;

		  if (currentRow > 0)
		    {
		      oldRow = currentRow;
		      currentRow--;
		      [self scrollRowToVisible: currentRow];
		      if (dragOperationPossible == NO)
			COMPUTE_NEW_SELECTION;
		    }
		}
	      break;
	    default:
	      break;
	    }
	  
	  if (done == NO)
	    {
	      /* in certain cases we are working with events that have already
	       * occurred and been dequeued by NSCell classes, in these cases
	       * getNextEvent is set to NO, use the current event.
	       */
	      if (getNextEvent == YES)
	        {
		  lastEvent = [NSApp nextEventMatchingMask: eventMask 
				 untilDate: distantFuture
				 inMode: NSEventTrackingRunLoopMode 
				 dequeue: YES]; 
		}
	      else
		{
		  lastEvent = [NSApp currentEvent];
		  getNextEvent = YES;
		}
	    }
	  IF_NO_GC(DESTROY(arp));
	}

      if (startedPeriodicEvents == YES)
	[NSEvent stopPeriodicEvents];

      if (![_selectedRows isEqual: oldSelectedRows])
	{
	  [self _postSelectionDidChangeNotification];
	}
      
      RELEASE(oldSelectedRows);

      if (!mouseMoved)
        sendAction = YES;

      /* If this was a simple click (ie. no dragging), we send our action. */
      if (sendAction)
	{
	  /*
	  _clickedRow and _clickedColumn are already set at the start of
	  this function.

	  TODO: should we ask the data source/column for the cell for this
	  row/column and check whether it has its own action/target?
	  */
	  if (_clickedRow != -1)
	    [self sendAction: _action  to: _target];
	}
    }
  
  _clickedRow = _selectedRow;
}


/* helpers for keyboard selection */
#define CHECK_CHANGING(x) { \
if (!x) \
  { \
    [self _postSelectionIsChangingNotification]; \
    x = YES; \
  } \
}
static BOOL selectContiguousRegion(NSTableView *self,
  				   NSIndexSet *_selectedRows,
				   NSInteger originalRow,
				   NSInteger oldRow,
				   NSInteger currentRow)
{
  NSInteger first = (oldRow < currentRow) ? oldRow : currentRow;
  NSInteger last = (oldRow < currentRow) ? currentRow : oldRow;
  NSInteger row;
  BOOL notified = NO;

  if (![_selectedRows containsIndex: currentRow])
    {
      CHECK_CHANGING(notified);
      [self _selectRow: currentRow];
    }

  /*
   * check if the old row is not between the current row and the original row 
   * and not the original or current rows
   */
  if (((!((oldRow < currentRow
	   && currentRow > originalRow
	   && oldRow > originalRow)
	  || (oldRow > currentRow
	      && currentRow < originalRow
	      && oldRow < originalRow)))
       && (!(oldRow == currentRow
	      || oldRow == originalRow))))
    {
      CHECK_CHANGING(notified);
      [self _unselectRow: oldRow]; 
    }

  /* 
   * there is an off by one here it could be on either end of the loop 
   * but its either oldRow or currentRow so above we select the currentRow
   * and possibly unselect the oldRow, one of the two will then
   * be selected or deselected again in in this loop 
   */
  for (row = first; row < last; row++)
    {
	      
      /* check if the old row is between the current row and the original row */
      if ((row < currentRow
	   && row > originalRow
	   && currentRow > oldRow)
	  || (row > currentRow
	      && row < originalRow
	      && currentRow < oldRow))
	{
	  if (![_selectedRows containsIndex: row])
	    {
	      CHECK_CHANGING(notified);
	      [self _selectRow: row];
	    }
	}
      else if (row == currentRow || row == originalRow)
	{
	  if (![_selectedRows containsIndex: row])
	    {
	      CHECK_CHANGING(notified);
	      [self _selectRow: row];
	    }
	}
      else
	{
	  if ([_selectedRows containsIndex: row])
	    {
              CHECK_CHANGING(notified);
	      [self _unselectRow: row];
	    }
	}
    }
  return notified;
}         

- (void) keyDown: (NSEvent *)theEvent
{
   NSInteger oldRow = -1;
   NSInteger currentRow = _selectedRow;
   NSInteger originalRow = -1;
   NSString *characters = [theEvent characters];
   NSUInteger len = [characters length];
   NSUInteger modifiers = [theEvent modifierFlags];
   CGFloat rowHeight = [self rowHeight];
   NSRect visRect = [self visibleRect];
   BOOL modifySelection = YES;
   NSPoint noModPoint = NSZeroPoint;
   NSInteger visRows;
   NSUInteger i;
   BOOL gotMovementKey = NO;
   
   // will not contain partial rows.
   visRows = visRect.size.height / [self rowHeight]; 

   // _clickedRow is stored between calls as the first selected row 
   // when doing multiple selection, so the selection may grow and shrink.
   
   /*
    * do a contiguous selection on shift
    */
   if (modifiers & NSShiftKeyMask)
     {
       originalRow = _clickedRow;
       if (_allowsMultipleSelection == YES)
	 {
	   oldRow = _selectedRow;
	 }
     }
   
   /* just scroll don't modify any selection */
   if (modifiers & NSControlKeyMask)
     {
       modifySelection = NO;
     }

   for (i = 0; i < len; i++)
     {
       unichar c = [characters characterAtIndex: i];

       switch (c)
         {
	   case NSUpArrowFunctionKey:
	     gotMovementKey = YES;
   	     if (modifySelection == NO)
	       {
   		 noModPoint.x = visRect.origin.x;
		 noModPoint.y = NSMinY(visRect) - rowHeight;
	       }
	     else
	       {
		 currentRow--;
	       }
	     break;
	   case NSDownArrowFunctionKey:
	     gotMovementKey = YES;
   	     if (modifySelection == NO)
	       {
   		 noModPoint.x = visRect.origin.x;
		 noModPoint.y = NSMinY(visRect) + rowHeight;
	       }
	     else
	       {
 	         currentRow++;
	       }
	     break;
	   case NSPageDownFunctionKey:
	     gotMovementKey = YES;
   	     if (modifySelection == NO)
	       {
   		 noModPoint.x = visRect.origin.x;
		 noModPoint.y = NSMinY(visRect) + (rowHeight * visRows) - rowHeight;
	       }
	     else
	       { 
		 currentRow += visRows;
	       }
	     break;
	   case NSPageUpFunctionKey:
	     gotMovementKey = YES;
	     if (modifySelection == NO)
	       {
   		 noModPoint.x = visRect.origin.x;
		 noModPoint.y = NSMinY(visRect) - (rowHeight * visRows) + rowHeight;
	       }
	     else 
	       {
	         currentRow -= visRows;
	       }
	     break;
	   case NSHomeFunctionKey:
	     gotMovementKey = YES;
	     if (modifySelection == NO)
	       {
		 noModPoint.x = visRect.origin.x;
		 noModPoint.y = NSMinY(_bounds);
	       }
	     else
	       {
	         currentRow = 0;
	       }
	     break;
	   case NSEndFunctionKey:
	     gotMovementKey = YES;
	     if (modifySelection == NO)
	       {
		 noModPoint.x = visRect.origin.x;
		 noModPoint.y = NSMaxY(_bounds);
	       }
	     else
	       {
		 currentRow = _numberOfRows - 1;
	       }
	     break;
	   default:
	     break;
        }
     }
  
  /*
   * if scrolled off the bottom or top the selection.
   * the modifiers might have changed so recompute the selection.
   */
  if (gotMovementKey == NO)
    {
      /* no handled keys. */
      [super keyDown: theEvent];
      return;
    }
  else if (currentRow < 0)
    {
      currentRow = 0;
    }
  else if (currentRow >= _numberOfRows)
    {
      currentRow = _numberOfRows - 1;
    }
  
  if (_numberOfRows)
    {
      if (modifySelection)
        {
	  BOOL notified = NO;

          [self _setSelectingColumns: NO];
     
          if (originalRow == -1)
            {
	      /* we're not extending any selection */
              originalRow = currentRow;
              _clickedRow = currentRow;
	    }

          if (_clickedRow == -1)
            {
	      /* user must have hit a key with no selected rows */
              _clickedRow = currentRow;
	    }
	  
	  if ((!(modifiers & NSShiftKeyMask && _allowsMultipleSelection)))
	    {
	      NSUInteger first = [_selectedRows firstIndex];
	      NSUInteger last = [_selectedRows lastIndex];

	      if ((first == last && first == currentRow) == 0)
	        {
		  CHECK_CHANGING(notified)
		  [self _unselectAllRows];
		  [self _selectRow: currentRow];
	          _selectedRow = currentRow;
		}
	    }
	  else
	    {   
	      notified = selectContiguousRegion(self, _selectedRows,
			         originalRow, oldRow, currentRow);
	      _selectedRow = currentRow;
	    }
	  
	  if (notified)
            {
              [self _postSelectionDidChangeNotification];
	    }
	  
	  [self scrollRowToVisible: currentRow];
	  [self displayIfNeeded];
        }
      else
        {
	  noModPoint = [self convertPoint: noModPoint
		  		   toView: _super_view];
	  noModPoint = 
	     [(NSClipView *)_super_view constrainScrollPoint: noModPoint];
	  [(NSClipView *)_super_view scrollToPoint: noModPoint];
	}
    }
}

/* 
 * Auxiliary Components 
 */

- (void) setHeaderView: (NSTableHeaderView*)aHeaderView
{
  
  if ([_headerView respondsToSelector:@selector(setTableView:)])
    [_headerView setTableView: nil];
      
  ASSIGN (_headerView, aHeaderView);
      
  if ([_headerView respondsToSelector:@selector(setTableView:)])
    [_headerView setTableView: self];
      
  [self tile]; // resizes corner and header views, then displays
  
  if (_super_view != nil)
    {
      id ssv = [_super_view superview];
      if ([ssv isKindOfClass: [NSScrollView class]])
        [ssv tile]; // draws any border type over corner and header views 
    } 
}

- (NSTableHeaderView*) headerView
{
  return _headerView;
}

- (void) setCornerView: (NSView*)aView
{
  ASSIGN (_cornerView, aView);
  [self tile]; // resizes corner and header views, then displays
  if (_super_view)
    {
      id ssv = [_super_view superview];
      if ([ssv isKindOfClass: [NSScrollView class]])
        [ssv tile]; // draws any border type over corner and header views 
    }
}

- (NSView*) cornerView
{
  return _cornerView;
}

/* 
 * Layout 
 */

- (NSRect) rectOfColumn: (NSInteger)columnIndex
{
  NSRect rect;

  if (columnIndex < 0 || columnIndex > _numberOfColumns)
    {
      NSDebugLLog(@"NSTableView", @"Column index %d out of table in rectOfColumn", (int)columnIndex);
      return NSZeroRect;
    }

  rect.origin.x = _columnOrigins[columnIndex];
  rect.origin.y = _bounds.origin.y;
  rect.size.width = [[_tableColumns objectAtIndex: columnIndex] width];
  rect.size.height = _numberOfRows * _rowHeight;
  return rect;
}

- (NSRect) rectOfRow: (NSInteger)rowIndex
{
  NSRect rect;

  if (rowIndex < 0 || rowIndex >= _numberOfRows)
    {
      NSDebugLLog(@"NSTableView", @"Row index %d out of table in rectOfRow", (int)rowIndex);
      return NSZeroRect;
    }

  rect.origin.x = _bounds.origin.x;
  rect.origin.y = _bounds.origin.y + (_rowHeight * rowIndex);
  rect.size.width = _bounds.size.width;
  rect.size.height = _rowHeight;
  return rect;
}

/** Returns the indexes of the table columns which intersects the given rect.

The rect is expressed in  the receiver coordinate space.

Hidden table columns are never tested. */
- (NSIndexSet *) columnIndexesInRect: (NSRect)aRect
{
  NSRange range = [self columnsInRect: aRect];
  NSMutableIndexSet *indexes = [NSMutableIndexSet indexSetWithIndexesInRange: range];
  NSUInteger i;

  for (i = range.location; i < range.length; i++)
    {
      NSTableColumn *tableColumn = [_tableColumns objectAtIndex: i];

      if ([tableColumn isHidden])
        [indexes removeIndex: i];
    }

  return indexes;
} 

/** Returns the index range of the table columns which intersects the given rect.

The rect is expressed in  the receiver coordinate space.

The returned range can include hidden table column indexes.

This method is deprecated, use -columnIndexesInRect:. */
- (NSRange) columnsInRect: (NSRect)aRect
{
  NSRange range;

  range.location = [self columnAtPoint: aRect.origin];
  range.length = [self columnAtPoint: 
			 NSMakePoint (NSMaxX (aRect), _bounds.origin.y)];
  range.length -= range.location;
  range.length += 1;
  return range;
}

- (NSRange) rowsInRect: (NSRect)aRect
{
  NSRange range;
  NSInteger lastRowInRect;

  range.location = [self rowAtPoint: aRect.origin];
  lastRowInRect = [self rowAtPoint: 
			 NSMakePoint (_bounds.origin.x, NSMaxY (aRect))];
  
  if (lastRowInRect == -1)
    {
      lastRowInRect = _numberOfRows - 1;
    }
  
  range.length = lastRowInRect;
  range.length -= range.location;
  range.length += 1;
  return range;
}

- (NSInteger) columnAtPoint: (NSPoint)aPoint
{
  if ((NSMouseInRect (aPoint, _bounds, YES)) == NO)
    {
      return -1;
    }
  else
    {
      NSInteger i = 0;
      
      while ((i < _numberOfColumns) && (aPoint.x >= _columnOrigins[i]))
	{
	  i++;
	}
      return i - 1;
    }
}

- (NSInteger) rowAtPoint: (NSPoint)aPoint
{
  /* NB: Y coordinate system is flipped in NSTableView */
  if ((NSMouseInRect (aPoint, _bounds, YES)) == NO)
    {
      return -1;
    }
  else if (_rowHeight == 0.0)
    {
      return -1;
    }
  else
    {
      NSInteger return_value;

      aPoint.y -= _bounds.origin.y;
      return_value = (NSInteger) (aPoint.y / _rowHeight);
      /* This could happen if point lies on the grid line or below the last row */
      if (return_value >= _numberOfRows)
	{
	  return_value = -1;
	}
      return return_value;
    }
}

- (NSRect) frameOfCellAtColumn: (NSInteger)columnIndex 
			   row: (NSInteger)rowIndex
{
  NSRect frameRect;

  if ((columnIndex < 0) 
      || (rowIndex < 0)
      || (columnIndex > (_numberOfColumns - 1))
      || (rowIndex > (_numberOfRows - 1)))
    return NSZeroRect;
      
  frameRect.origin.y  = _bounds.origin.y + (rowIndex * _rowHeight);
  frameRect.origin.y += _intercellSpacing.height / 2;
  frameRect.size.height = _rowHeight - _intercellSpacing.height;

  frameRect.origin.x = _columnOrigins[columnIndex];
  frameRect.origin.x  += _intercellSpacing.width / 2;
  frameRect.size.width = [[_tableColumns objectAtIndex: columnIndex] width];
  frameRect.size.width -= _intercellSpacing.width;

  // We add some space to separate the cell from the grid
  if (_drawsGrid)
    {
      frameRect.size.width -= 4;
      frameRect.origin.x += 2;
    }

  // Safety check
  if (frameRect.size.width < 0)
    frameRect.size.width = 0;
  
  return frameRect;
}

- (void) setAutoresizesAllColumnsToFit: (BOOL)flag
{
  _autoresizesAllColumnsToFit = flag;
}

- (BOOL) autoresizesAllColumnsToFit
{
  return _autoresizesAllColumnsToFit;
}

- (NSTableViewColumnAutoresizingStyle) columnAutoresizingStyle
{
  // FIXME
  return NSTableViewNoColumnAutoresizing;
}

- (void) setColumnAutoresizingStyle: (NSTableViewColumnAutoresizingStyle)style
{
  // FIXME
}

- (void) sizeLastColumnToFit
{
  if ((_super_view != nil) && (_numberOfColumns > 0))
    {
      CGFloat excess_width;
      CGFloat last_column_width;
      NSTableColumn *lastColumn;

      lastColumn = [_tableColumns objectAtIndex: (_numberOfColumns - 1)];
      if ([lastColumn isResizable] == NO)
        {
          return;
        }
      excess_width = NSMaxX([self convertRect: [_super_view bounds] 
				     fromView: _super_view]) - NSMaxX(_bounds);
      last_column_width = [lastColumn width] + excess_width;
      // This will automatically retile the table
      [lastColumn setWidth: last_column_width];
    }
}

- (void) setFrame: (NSRect)frameRect
{
  NSRect tmpRect = frameRect;

  if ([_super_view respondsToSelector: @selector(documentVisibleRect)])
    {
      CGFloat rowsHeight = ((_numberOfRows * _rowHeight) + 1);
      NSRect docRect = [(NSClipView *)_super_view documentVisibleRect];
      
      if (rowsHeight < docRect.size.height)
	{
	  tmpRect.size.height = docRect.size.height;
	}
      else 
        {
	  tmpRect.size.height = rowsHeight;
        }
      // TODO width?
    }
  [super setFrame: tmpRect];
}

- (void) setFrameSize: (NSSize)frameSize
{
  NSSize tmpSize = frameSize;
  
  if ([_super_view respondsToSelector: @selector(documentVisibleRect)])
    {
      CGFloat rowsHeight = ((_numberOfRows * _rowHeight) + 1);
      NSRect docRect = [(NSClipView *)_super_view documentVisibleRect];
      
      if (rowsHeight < docRect.size.height)
	{
	  tmpSize.height = docRect.size.height;
	}
      else
        {
          tmpSize.height = rowsHeight;
        }
      // TODO width?
    }
  [super setFrameSize: tmpSize];
}

- (void) viewWillMoveToSuperview:(NSView *)newSuper
{
  [super viewWillMoveToSuperview: newSuper];
  /* need to potentially enlarge to fill the documentRect of the clip view */
  [self setFrame: _frame];
}

- (void) sizeToFit
{
  NSTableColumn *tb;
  NSInteger i, j;
  CGFloat remainingWidth;
  columnSorting *columnInfo;
  CGFloat *currentWidth;
  CGFloat *maxWidth;
  CGFloat *minWidth;
  BOOL *isResizable;
  NSInteger numberOfCurrentColumns = 0;
  CGFloat previousPoint;
  CGFloat nextPoint;
  CGFloat toAddToCurrentColumns;

  if ((_super_view == nil) || (_numberOfColumns == 0))
    return;

  columnInfo = NSZoneMalloc(NSDefaultMallocZone(),
			    sizeof(columnSorting) * 2 
			    * _numberOfColumns);
  currentWidth = NSZoneMalloc(NSDefaultMallocZone(),
			      sizeof(CGFloat) * _numberOfColumns);
  maxWidth = NSZoneMalloc(NSDefaultMallocZone(),
			  sizeof(CGFloat) * _numberOfColumns);
  minWidth = NSZoneMalloc(NSDefaultMallocZone(),
			  sizeof(CGFloat) * _numberOfColumns);
  isResizable = NSZoneMalloc(NSDefaultMallocZone(),
			     sizeof(BOOL) * _numberOfColumns);

  remainingWidth = NSMaxX([self convertRect: [_super_view bounds] 
				fromView: _super_view]);

  /*
   *  We store the minWidth and the maxWidth of every column
   *  because we'll use those values *a lot*
   *  At the same time we set every column to its mininum width
   */
  for (i = 0; i < _numberOfColumns; i++)
    {
      tb = [_tableColumns objectAtIndex: i];
      isResizable[i] = [tb isResizable];
      if (isResizable[i] == YES)
	{
	  minWidth[i] = [tb minWidth];
	  maxWidth[i] = [tb maxWidth];
	  
	  if (minWidth[i] < 0)
	    minWidth[i] = 0;
	  if (minWidth[i] > maxWidth[i])
	    {
	      minWidth[i] = [tb width];
	      maxWidth[i] = minWidth[i];
	    }
	  columnInfo[i * 2].width = minWidth[i];
	  columnInfo[i * 2].isMax = 0;
	  currentWidth[i] = minWidth[i];
	  remainingWidth -= minWidth[i];
	  
	  columnInfo[i * 2 + 1].width = maxWidth[i];
	  columnInfo[i * 2 + 1].isMax = 1;
	}
      else
	{
	  minWidth[i] = [tb width];
	  columnInfo[i * 2].width = minWidth[i];
	  columnInfo[i * 2].isMax = 0;
	  currentWidth[i] = minWidth[i];
	  remainingWidth -= minWidth[i];
	  
	  maxWidth[i] = minWidth[i];
	  columnInfo[i * 2 + 1].width = maxWidth[i];
	  columnInfo[i * 2 + 1].isMax = 1;
	}
    } 

  // sort the info we have
  quick_sort_internal(columnInfo, 0, 2 * _numberOfColumns - 1);

  previousPoint = columnInfo[0].width;
  numberOfCurrentColumns = 1;
  
  if (remainingWidth >= 0.)
    {
      for (i = 1; i < 2 * _numberOfColumns; i++)
	{
	  nextPoint = columnInfo[i].width;
	  
	  if (numberOfCurrentColumns > 0 && 
	      (nextPoint - previousPoint) > 0.)
	    {
	      NSInteger verification = 0;
	      
	      if ((nextPoint - previousPoint) * numberOfCurrentColumns
		  <= remainingWidth)
		{
		  toAddToCurrentColumns = nextPoint - previousPoint;
		  remainingWidth -= 
		    (nextPoint - previousPoint) * numberOfCurrentColumns;

		  for (j = 0; j < _numberOfColumns; j++)
		    {
		      if (minWidth[j] <= previousPoint
			  && maxWidth[j] >= nextPoint)
			{
			  verification++;
			  currentWidth[j] += toAddToCurrentColumns;
			}
		    }
		  if (verification != numberOfCurrentColumns)
		    {
		      NSLog(@"[NSTableView sizeToFit]: unexpected error");
		    }
		}
	      else
		{
		  int remainingInt = floor(remainingWidth);
		  int quotient = remainingInt / numberOfCurrentColumns;
		  int remainder = remainingInt - quotient * numberOfCurrentColumns;
		  int oldRemainder = remainder;

		  for (j = _numberOfColumns - 1; j >= 0; j--)
		    {
		      if (minWidth[j] <= previousPoint
			  && maxWidth[j] >= nextPoint)
			{
			  currentWidth[j] += quotient;
			  if (remainder > 0 
			      && maxWidth[j] >= currentWidth[j] + 1)
			    {
			      remainder--;
			      currentWidth[j]++;
			    }
			}
		    }
		  while (oldRemainder > remainder && remainder > 0)
		    {
		      oldRemainder = remainder;
		      for (j = 0; j < _numberOfColumns; j++)
			{
			  if (minWidth[j] <= previousPoint
			      && maxWidth[j] >= nextPoint)
			    {
			      if (remainder > 0 
				  && maxWidth[j] >= currentWidth[j] + 1)
				{
				  remainder--;
				  currentWidth[j]++;
				}
			    }
			  
			}
		    }
		  if (remainder > 0)
		    NSLog(@"There is still free space to fill.\
 However it seems better to use integer width for the columns");
		  else
		    remainingWidth = 0.;
		}
	      
	      
	    }
	  else if (numberOfCurrentColumns < 0)
	    {
	      NSLog(@"[NSTableView sizeToFit]: unexpected error");
	    }
	  
	  if (columnInfo[i].isMax)
	    numberOfCurrentColumns--;
	  else
	    numberOfCurrentColumns++;
	  previousPoint = nextPoint;
	  
	  if (remainingWidth == 0.)
	    {
	      break;
	    }
	}
    }

  _tilingDisabled = YES;

  remainingWidth = 0.;
  for (i = 0; i < _numberOfColumns; i++)
    {
      if (isResizable[i] == YES)
	{
	  tb = [_tableColumns objectAtIndex: i];
	  remainingWidth += currentWidth[i];
	  [tb setWidth: currentWidth[i]];
	}
      else
	{
	  remainingWidth += minWidth[i];
	}
    }

  _tilingDisabled = NO;
  NSZoneFree(NSDefaultMallocZone(), columnInfo);
  NSZoneFree(NSDefaultMallocZone(), currentWidth);
  NSZoneFree(NSDefaultMallocZone(), maxWidth);
  NSZoneFree(NSDefaultMallocZone(), minWidth);
  NSZoneFree(NSDefaultMallocZone(), isResizable);

  [self tile];
}
/*
- (void) sizeToFit
{
  NSCell *cell;
  NSEnumerator	*enumerator;
  NSTableColumn	*tb;
  float table_width;
  float width;
  float candidate_width;
  int row;

  _tilingDisabled = YES;

  // First Step
  // Resize Each Column to its Minimum Width
  table_width = _bounds.origin.x;
  enumerator = [_tableColumns objectEnumerator];
  while ((tb = [enumerator nextObject]) != nil)
    {
      // Compute min width of column 
      width = [[tb headerCell] cellSize].width;
      for (row = 0; row < _numberOfRows; row++)
	{
	  cell = [self preparedCellAtColumn: [_tableColumns indexOfObject: tb] 
                                        row: row];
	  [cell setObjectValue: [_dataSource tableView: self
					     objectValueForTableColumn: tb
					     row: row]]; 
	  [self _willDisplayCell: cell
	        forTableColumn: tb
	        row: row];
	  candidate_width = [cell cellSize].width;

	  if (_drawsGrid)
	    candidate_width += 4;

	  if (candidate_width > width)
	    {
	      width = candidate_width;
	    }
	}
      width += _intercellSpacing.width;
      [tb setWidth: width];
      // It is necessary to ask the column for the width, since it might have 
      // been changed by the column to constrain it to a min or max width
      table_width += [tb width];
    }

  // Second Step
  // If superview (clipview) is bigger than that, divide remaining space 
  // between all columns
  if ((_super_view != nil) && (_numberOfColumns > 0))
    {
      float excess_width;

      excess_width = NSMaxX ([self convertRect: [_super_view bounds] 
				      fromView: _super_view]);
      excess_width -= table_width;
      // Since we resized each column at its minimum width, 
      // it's useless to try shrinking more: we can't
      if (excess_width <= 0)
	{
	  _tilingDisabled = NO;
	  [self tile];
	  NSLog(@"exiting sizeToFit");
	  return;
	}
      excess_width = excess_width / _numberOfColumns;

      enumerator = [_tableColumns objectEnumerator];
      while ((tb = [enumerator nextObject]) != nil)
	{
	  [tb setWidth: ([tb width] + excess_width)];
	}
    }

  _tilingDisabled = NO;
  [self tile];
  NSLog(@"exiting sizeToFit");
}
*/

- (void) noteNumberOfRowsChanged
{
  NSRect newFrame;

  _numberOfRows = [self _numRows];
 
  /* If we are selecting rows, we have to check that we have no
     selected rows below the new end of the table */
  if (!_selectingColumns)
    {
      NSUInteger row = [_selectedRows lastIndex];
      
      if (row == NSNotFound)
        {
          if (!_allowsEmptySelection)
            {
              /* We shouldn't allow empty selection - try
                 selecting the last row */
              NSInteger lastRow = _numberOfRows - 1;
		      
              if (lastRow > -1)
                {
                  [self _postSelectionIsChangingNotification];
                  [_selectedRows addIndex: lastRow];
                  _selectedRow = lastRow;
                  [self _postSelectionDidChangeNotification];
                }
              else
                {
                  /* problem - there are no rows at all */
                  _selectedRow = -1;
                }
            }
        }
      /* Check that all selected rows are in the new range of rows */
      else if (row >= _numberOfRows)
        {
          [_selectedRows removeIndexesInRange: 
             NSMakeRange(_numberOfRows,  row + 1 - _numberOfRows)];
          if (_selectedRow >= _numberOfRows)
            {
              row = [_selectedRows lastIndex];
              [self _postSelectionIsChangingNotification];
              
              if (row != NSNotFound)
                {
                  _selectedRow = row;
                }
              else
                {
                  /* Argh - all selected rows were outside the table */
                  if (_allowsEmptySelection)
                    {
                      _selectedRow = -1;
                    }
                  else
                    {
                      /* We shouldn't allow empty selection - try
                         selecting the last row */
                      NSInteger lastRow = _numberOfRows - 1;
                      
                      if (lastRow > -1)
                        {
                          [_selectedRows addIndex: lastRow];
                          _selectedRow = lastRow;
                        }
                      else
                        {
                          /* problem - there are no rows at all */
                          _selectedRow = -1;
                        }
                    }
                }
              [self _postSelectionDidChangeNotification];
            }
        }
    }
  
  newFrame = _frame;
  newFrame.size.height = (_numberOfRows * _rowHeight) + 1;
  if (NO == NSEqualRects(newFrame, NSUnionRect(newFrame, _frame)))
    {
      [_super_view setNeedsDisplayInRect: _frame];
    }
  [self setFrame: newFrame];

  /* If we are shorter in height than the enclosing clipview, we
     should redraw us now. */
  if (_super_view != nil)
    {
      NSRect superviewBounds; // Get this *after* [self setFrame:]
      superviewBounds = [_super_view bounds];
      if ((superviewBounds.origin.x <= _frame.origin.x) 
        && (NSMaxY(superviewBounds) >= NSMaxY(_frame)))
        {
          [self setNeedsDisplay: YES];
        }
    }
}

- (void) tile
{
  CGFloat table_width = 0;
  CGFloat table_height;

  if (_tilingDisabled == YES)
    return;

  if (_numberOfColumns > 0)
    {
      NSInteger i;
      CGFloat width;
  
      _columnOrigins[0] = _bounds.origin.x;
      width = [[_tableColumns objectAtIndex: 0] width];
      table_width += width;
      for (i = 1; i < _numberOfColumns; i++)
	{
	  _columnOrigins[i] = _columnOrigins[i - 1] + width;
	  width = [[_tableColumns objectAtIndex: i] width];
	  table_width += width;
	}
    }
  /* + 1 for the last grid line */
  table_height = (_numberOfRows * _rowHeight) + 1;
  [self setFrameSize: NSMakeSize (table_width, table_height)];
  [self setNeedsDisplay: YES];

  if (_headerView != nil)
    {
      CGFloat innerBorderWidth = [[NSUserDefaults standardUserDefaults]
				   boolForKey: @"GSScrollViewNoInnerBorder"] ? 0.0 : 1.0;

      [_headerView setFrameSize: 
		     NSMakeSize (_frame.size.width,
				 [_headerView frame].size.height)];
      [_cornerView setFrameSize: 
		     NSMakeSize ([NSScroller scrollerWidth] + innerBorderWidth,
				 [_headerView frame].size.height)];
      [_headerView setNeedsDisplay: YES];
      [_cornerView setNeedsDisplay: YES];
    }  
}

/* 
 * Drawing 
 */

- (void) drawRow: (NSInteger)rowIndex clipRect: (NSRect)clipRect
{
  [[GSTheme theme] drawTableViewRow: rowIndex
		   clipRect: clipRect
		   inView: self];
}

- (void) noteHeightOfRowsWithIndexesChanged: (NSIndexSet*)indexes
{
  // FIXME
}

- (void) drawGridInClipRect: (NSRect)aRect
{
  [[GSTheme theme] drawTableViewGridInClipRect: aRect
		   inView: self];
}

- (void) highlightSelectionInClipRect: (NSRect)clipRect
{
  [[GSTheme theme] highlightTableViewSelectionInClipRect: clipRect
		   inView: self
		   selectingColumns: _selectingColumns];
}

- (void) drawBackgroundInClipRect: (NSRect)clipRect
{
  [[GSTheme theme] drawTableViewBackgroundInClipRect: clipRect
		   inView: self
		   withBackgroundColor: _backgroundColor];
}

- (void) drawRect: (NSRect)aRect
{
  [[GSTheme theme] drawTableViewRect: aRect
		   inView: self];
}

- (BOOL) isOpaque
{
  return YES;
}

/* 
 * Scrolling 
 */

- (void) scrollRowToVisible: (NSInteger)rowIndex
{
  if (_super_view != nil)
    {
      NSRect rowRect = [self rectOfRow: rowIndex];
      NSRect visibleRect = [self visibleRect];
      
      // If the row is over the top, or it is partially visible 
      // on top,
      if ((rowRect.origin.y < visibleRect.origin.y))	
	{
	  // Then make it visible on top
	  NSPoint newOrigin;  
	  
	  newOrigin.x = visibleRect.origin.x;
	  newOrigin.y = rowRect.origin.y;
	  newOrigin = [self convertPoint: newOrigin  toView: _super_view];
	  [(NSClipView *)_super_view scrollToPoint: newOrigin];
	  return;
	}
      // If the row is under the bottom, or it is partially visible on
      // the bottom,
      if (NSMaxY (rowRect) > NSMaxY (visibleRect))
	{
	  // Then make it visible on bottom
	  NSPoint newOrigin;  
	  
	  newOrigin.x = visibleRect.origin.x;
	  newOrigin.y = visibleRect.origin.y;
	  newOrigin.y += NSMaxY (rowRect) - NSMaxY (visibleRect);
	  newOrigin = [self convertPoint: newOrigin  toView: _super_view];
	  [(NSClipView *)_super_view scrollToPoint: newOrigin];
	  return;
	}
    }
}

- (void) scrollColumnToVisible: (NSInteger)columnIndex
{
  if (_super_view != nil)
    {
      NSRect columnRect = [self rectOfColumn: columnIndex];
      NSRect visibleRect = [self visibleRect];
      CGFloat diff;

      // If the row is out on the left, or it is partially visible 
      // on the left
      if ((columnRect.origin.x < visibleRect.origin.x))	
	{
	  // Then make it visible on the left
	  NSPoint newOrigin;  
	  
	  newOrigin.x = columnRect.origin.x;
	  newOrigin.y = visibleRect.origin.y;
	  newOrigin = [self convertPoint: newOrigin  toView: _super_view];
	  [(NSClipView *)_super_view scrollToPoint: newOrigin];
	  return;
	}
      diff = NSMaxX (columnRect) - NSMaxX (visibleRect);
      // If the row is out on the right, or it is partially visible on
      // the right,
      if (diff > 0)
	{
	  // Then make it visible on the right
	  NSPoint newOrigin;

	  newOrigin.x = visibleRect.origin.x;
	  newOrigin.y = visibleRect.origin.y;
	  newOrigin.x += diff;
	  newOrigin = [self convertPoint: newOrigin  toView: _super_view];
	  [(NSClipView *)_super_view scrollToPoint: newOrigin];
	  return;
	}
    }
}


/* 
 * Text delegate methods 
 */

- (void) textDidBeginEditing: (NSNotification *)aNotification
{
  [super textDidBeginEditing: aNotification];
}

- (void) textDidChange: (NSNotification *)aNotification
{
  // MacOS-X asks us to inform the cell if possible.
  if ((_editedCell != nil) && [_editedCell respondsToSelector: 
						 @selector(textDidChange:)])
    [_editedCell textDidChange: aNotification];

  [super textDidChange: aNotification];
}

- (void) textDidEndEditing: (NSNotification *)aNotification
{
  id textMovement;
  NSInteger row, column;

  /* Save values */
  row = _editedRow;
  column = _editedColumn;

  [super textDidEndEditing: aNotification];

  textMovement = [[aNotification userInfo] objectForKey: @"NSTextMovement"];
  if (textMovement)
    {
      switch ([(NSNumber *)textMovement intValue])
	{
	case NSReturnTextMovement:
	  [self _editNextCellAfterRow: row inColumn: column];
	  // Send action ?
	  break;
	case NSTabTextMovement:
	  if ([self _editNextEditableCellAfterRow: row  column: column] == YES)
	    {
	      break;
	    }
	  [_window selectKeyViewFollowingView: self];
	  break;
	case NSBacktabTextMovement:
	  if ([self _editPreviousEditableCellBeforeRow: row  column: column] == YES)
	    {
	      break;
	    }
	  [_window selectKeyViewPrecedingView: self];
	  break;
	}
    }
}

- (BOOL) textShouldBeginEditing: (NSText *)textObject
{
  if (_delegate && [_delegate respondsToSelector:
				@selector(control:textShouldBeginEditing:)])
    return [_delegate control: self
		      textShouldBeginEditing: textObject];
  else
    return YES;
}

- (BOOL) textShouldEndEditing: (NSText*)textObject
{
  if ([_delegate respondsToSelector:
		   @selector(control:textShouldEndEditing:)])
    {
      if ([_delegate control: self
		     textShouldEndEditing: textObject] == NO)
	{
	  NSBeep ();
	  return NO;
	}
      
      return YES;
    }

  if ([_delegate respondsToSelector: 
		   @selector(control:isValidObject:)] == YES)
    {
      NSFormatter *formatter;
      id newObjectValue;
      
      formatter = [_editedCell formatter];
      
      if ([formatter getObjectValue: &newObjectValue 
		     forString: [_textObject text] 
		     errorDescription: NULL] == YES)
	{
	  if ([_delegate control: self
			 isValidObject: newObjectValue] == NO)
	    return NO;
	}
    }

  return [_editedCell isEntryAcceptable: [textObject text]];
}

/* 
 * Persistence 
 */

- (NSString *) autosaveName
{
  return _autosaveName;
}

- (BOOL) autosaveTableColumns
{
  return _autosaveTableColumns;
}

- (void) setAutosaveName: (NSString *)name
{
  ASSIGN (_autosaveName, name);
  [self _autoloadTableColumns];
}

- (void) setAutosaveTableColumns: (BOOL)flag
{
  if (flag == _autosaveTableColumns)
    {
      return;
    }

  _autosaveTableColumns = flag;
  if (flag)
    {
      [self _autoloadTableColumns];
      [nc addObserver: self 
          selector: @selector(_autosaveTableColumns)
	  name: NSTableViewColumnDidResizeNotification
	  object: self];
    }
  else
    {
      [nc removeObserver: self 
	  name: NSTableViewColumnDidResizeNotification
	  object: self];    
    }
}

/* 
 * Delegate 
 */

- (void) setDelegate: (id)anObject
{
  const SEL sel = @selector(tableView:willDisplayCell:forTableColumn:row:);

  if (_delegate)
    [nc removeObserver: _delegate name: nil object: self];
  _delegate = anObject;
  
#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(tableView##notif_name:)]) \
    [nc addObserver: _delegate \
      selector: @selector(tableView##notif_name:) \
      name: NSTableView##notif_name##Notification object: self]

  SET_DELEGATE_NOTIFICATION(ColumnDidMove);
  SET_DELEGATE_NOTIFICATION(ColumnDidResize);
  SET_DELEGATE_NOTIFICATION(SelectionDidChange);
  SET_DELEGATE_NOTIFICATION(SelectionIsChanging);
  
  /* Cache */
  _del_responds = [_delegate respondsToSelector: sel];
}

- (id) delegate
{
  return _delegate;
}


/* indicator image */
- (NSImage *) indicatorImageInTableColumn: (NSTableColumn *)aTableColumn
{
  // TODO
  NSLog(@"Method %s is not implemented for class %s",
	"indicatorImageInTableColumn:", "NSTableView");
  return nil;
}

- (void) setIndicatorImage: (NSImage *)anImage
	     inTableColumn: (NSTableColumn *)aTableColumn
{
  // TODO
  NSLog(@"Method %s is not implemented for class %s",
	"setIndicatorImage:inTableColumn:", "NSTableView");
}

/* highlighting columns */
- (NSTableColumn *) highlightedTableColumn
{
  return _highlightedTableColumn;
}

- (void) setHighlightedTableColumn: (NSTableColumn *)aTableColumn
{
  NSUInteger tableColumnIndex;

  tableColumnIndex = [_tableColumns indexOfObject: aTableColumn];

  if (tableColumnIndex == NSNotFound)
    {
      NSLog(@"setHighlightedTableColumn received an invalid\
 NSTableColumn object");
      return;
    }

  // we do not need to retain aTableColumn as it is already in
  // _tableColumns array
  _highlightedTableColumn = aTableColumn;

  [_headerView setNeedsDisplay: YES];
}

/* dragging rows */
- (NSImage*) dragImageForRows: (NSArray*)dragRows
                        event: (NSEvent*)dragEvent
              dragImageOffset: (NSPoint*)dragImageOffset
{
  // FIXME
  NSImage *dragImage = [[NSImage alloc]
			 initWithSize: NSMakeSize(8, 8)];

  return AUTORELEASE(dragImage);
}

- (NSImage *) dragImageForRowsWithIndexes: (NSIndexSet*)rows
                             tableColumns: (NSArray*)cols
                                    event: (NSEvent*)event
                                   offset: (NSPoint*)offset
{
  // FIXME
  NSArray *rowArray;

  rowArray = [self _indexSetToArray: rows];
  return [self dragImageForRows: rowArray 
               event: event
               dragImageOffset: offset];
}

- (void) setDropRow: (NSInteger)row
      dropOperation: (NSTableViewDropOperation)operation
{
  if (row < -1 || row > _numberOfRows 
    || (operation == NSTableViewDropOn && row == _numberOfRows))    
    {
      currentDropRow = -1;
      currentDropOperation = NSTableViewDropOn;
    }
  else
    {
      currentDropRow = row;
      currentDropOperation = operation;
    }
}

- (void) setVerticalMotionCanBeginDrag: (BOOL)flag
{
  _verticalMotionDrag = flag;
}

- (BOOL) verticalMotionCanBeginDrag
{
  return _verticalMotionDrag;
}

- (NSArray*) namesOfPromisedFilesDroppedAtDestination: (NSURL *)dropDestination
{
  if ([_dataSource respondsToSelector:
                    @selector(tableView:namesOfPromisedFilesDroppedAtDestination:forDraggedRowsWithIndexes:)])
    {
      return [_dataSource tableView: self
                          namesOfPromisedFilesDroppedAtDestination: dropDestination
                          forDraggedRowsWithIndexes: _selectedRows];
    }
  else
    {
      return nil;
    }
}

/*
 * Encoding/Decoding
 */

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      unsigned int vFlags = 0; 
      NSSize intercellSpacing = [self intercellSpacing];
      GSTableViewFlags tableViewFlags;

      // make sure the corner view is properly encoded...
      [super encodeWithCoder: aCoder];
      
      if ([self dataSource])
	{
	  [aCoder encodeObject: [self dataSource] forKey: @"NSDataSource"];
	}
      if ([self delegate])
	{
	  [aCoder encodeObject: [self delegate] forKey: @"NSDelegate"];
	}
      if ([self target])
	{
	  [aCoder encodeObject: [self target] forKey: @"NSTarget"];
	}
      if ([self action])
	{
	  [aCoder encodeObject: NSStringFromSelector([self action]) forKey: @"NSAction"];
	}
      if ([self doubleAction] != NULL)
	{
	  [aCoder encodeObject: NSStringFromSelector([self doubleAction]) forKey: @"NSDoubleAction"];
	}

      [aCoder encodeObject: [self backgroundColor] forKey: @"NSBackgroundColor"];
      [aCoder encodeObject: [self gridColor] forKey: @"NSGridColor"];
      [aCoder encodeFloat: intercellSpacing.height forKey: @"NSIntercellSpacingHeight"];
      [aCoder encodeFloat: intercellSpacing.width forKey: @"NSIntercellSpacingWidth"];
      [aCoder encodeFloat: [self rowHeight] forKey: @"NSRowHeight"];
      [aCoder encodeObject: [self tableColumns] forKey: @"NSTableColumns"];

      if (_headerView)
        {
          [aCoder encodeObject: _headerView forKey: @"NSHeaderView"];
        }
      if (_cornerView)
        {
          [aCoder encodeObject: _cornerView forKey: @"NSCornerView"];
        }

      if ([[self sortDescriptors] count] > 0)
        {
          [aCoder encodeObject: _sortDescriptors forKey: @"NSSortDescriptors"];
        }

      tableViewFlags.columnSelection = [self allowsColumnSelection];
      tableViewFlags.multipleSelection = [self allowsMultipleSelection];
      tableViewFlags.emptySelection = [self allowsEmptySelection];
      tableViewFlags.drawsGrid = [self drawsGrid]; 
      tableViewFlags.columnResizing = [self allowsColumnResizing];
      tableViewFlags.columnOrdering = [self allowsColumnReordering];
      
      memcpy((void *)&vFlags,(void *)&tableViewFlags,sizeof(unsigned int));

      // encode..
      [aCoder encodeInt: vFlags forKey: @"NSTvFlags"];
    }
  else
    {
      float rowHeight;
      int number;

      [super encodeWithCoder: aCoder];
      [aCoder encodeConditionalObject: _dataSource];
      [aCoder encodeObject: _tableColumns];
      [aCoder encodeObject: _gridColor];
      [aCoder encodeObject: _backgroundColor];
      [aCoder encodeObject: _headerView];
      [aCoder encodeObject: _cornerView];
      [aCoder encodeConditionalObject: _delegate];
      [aCoder encodeConditionalObject: _target];

      number = _numberOfRows;
      [aCoder encodeValueOfObjCType: @encode(int) at: &number];
      number = _numberOfColumns;
      [aCoder encodeValueOfObjCType: @encode(int) at: &number];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_drawsGrid];
      rowHeight = _rowHeight;
      [aCoder encodeValueOfObjCType: @encode(float) at: &rowHeight];
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_doubleAction];
      [aCoder encodeSize: _intercellSpacing];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsMultipleSelection];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsEmptySelection];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsColumnSelection];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsColumnResizing];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsColumnReordering];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_autoresizesAllColumnsToFit];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_verticalMotionDrag];
      [aCoder encodeObject: _sortDescriptors];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return self;

  if ([aDecoder allowsKeyedCoding])
    {
      NSSize intercellSpacing;
      NSArray *columns;
      NSEnumerator *e;
      NSTableColumn *col;

      // assign defaults, so that there's color in case none is specified
      [self _initDefaults];
      ASSIGN(_gridColor, [NSColor gridColor]); 
      ASSIGN(_backgroundColor, [NSColor controlBackgroundColor]); 
      ASSIGN(_tableColumns, [NSMutableArray array]);
      ASSIGN(_sortDescriptors, [NSArray array]);

      //
      // Check for nil on some of these, since they are usually set
      // in NSIBOutletConnector objects we don't want to override
      // that setting unless they're directly encoded with the 
      // object. 
      // 
      // I'm not sure why IB encodes nil values for these, but
      // the behaviour here should match that on Mac OS X.
      //
      if ([aDecoder containsValueForKey: @"NSDataSource"])
        {
	  id obj = [aDecoder decodeObjectForKey: @"NSDataSource"];
	  if(obj != nil)
	    {
	      [self setDataSource: obj];
	    }
	}
      if ([aDecoder containsValueForKey: @"NSDelegate"])
        {      
	  id obj = [aDecoder decodeObjectForKey: @"NSDelegate"];
	  if(obj != nil)
	    {
	      [self setDelegate: obj];
	    }
	}
      if ([aDecoder containsValueForKey: @"NSTarget"])
        {
	  id obj = [aDecoder decodeObjectForKey: @"NSTarget"];
	  if(obj != nil)
	    {
	      [self setTarget: obj];
	    }
	}
      if ([aDecoder containsValueForKey: @"NSAction"])
        {
          NSString *action = [aDecoder decodeObjectForKey: @"NSAction"];
	  if(action != nil)
	    {
	      [self setAction: NSSelectorFromString(action)];
	    }
	}
      if ([aDecoder containsValueForKey: @"NSDoubleAction"])
        {
          NSString *action = [aDecoder decodeObjectForKey: @"NSDoubleAction"];
	  if(action != nil)
	    {
	      [self setDoubleAction: NSSelectorFromString(action)];
	    }
	}

      if ([aDecoder containsValueForKey: @"NSBackgroundColor"])
        {
          [self setBackgroundColor: [aDecoder decodeObjectForKey: @"NSBackgroundColor"]];
        }
      if ([aDecoder containsValueForKey: @"NSGridColor"])
        {
          [self setGridColor: [aDecoder decodeObjectForKey: @"NSGridColor"]];
        }

      intercellSpacing = [self intercellSpacing];
      if ([aDecoder containsValueForKey: @"NSIntercellSpacingHeight"])
        {
          intercellSpacing.height = [aDecoder decodeFloatForKey: @"NSIntercellSpacingHeight"];
        }
      if ([aDecoder containsValueForKey: @"NSIntercellSpacingWidth"])
        {
          intercellSpacing.width = [aDecoder decodeFloatForKey: @"NSIntercellSpacingWidth"];
        }
      [self setIntercellSpacing: intercellSpacing];

      if ([aDecoder containsValueForKey: @"NSDraggingSourceMaskForLocal"])
        {
          [self setDraggingSourceOperationMask: 
                    [aDecoder decodeIntForKey: @"NSDraggingSourceMaskForLocal"]
                forLocal: YES];
        }
      if ([aDecoder containsValueForKey: @"NSDraggingSourceMaskForNonLocal"])
        {
          [self setDraggingSourceOperationMask: 
                    [aDecoder decodeIntForKey: @"NSDraggingSourceMaskForNonLocal"]
                forLocal: NO];
        }

      if ([aDecoder containsValueForKey: @"NSRowHeight"])
        {
          [self setRowHeight: [aDecoder decodeFloatForKey: @"NSRowHeight"]];
        }

      if ([aDecoder containsValueForKey: @"NSCornerView"])
        {
	  NSView *aView = [aDecoder decodeObjectForKey: @"NSCornerView"];
          [self setCornerView: aView];
	  [aView setHidden: NO];
        }
      else
        {
          _cornerView = [GSTableCornerView new];
        }

      if ([aDecoder containsValueForKey: @"NSHeaderView"])
        {
          [self setHeaderView: [aDecoder decodeObjectForKey: @"NSHeaderView"]];
        }

      if ([aDecoder containsValueForKey: @"NSSortDescriptors"])
        {
          ASSIGN(_sortDescriptors, [aDecoder decodeObjectForKey: @"NSSortDescriptors"]);
        }

      if ([aDecoder containsValueForKey: @"NSTvFlags"])
        {
          unsigned int flags = [aDecoder decodeIntForKey: @"NSTvFlags"];
          GSTableViewFlags tableViewFlags;
          memcpy((void *)&tableViewFlags,(void *)&flags,sizeof(struct _tableViewFlags));

          [self setAllowsColumnSelection: tableViewFlags.columnSelection];
          [self setAllowsMultipleSelection: tableViewFlags.multipleSelection];
          [self setAllowsEmptySelection: tableViewFlags.emptySelection];
          [self setDrawsGrid: tableViewFlags.drawsGrid];
          [self setAllowsColumnResizing: tableViewFlags.columnResizing];
          [self setAllowsColumnReordering: tableViewFlags.columnOrdering];
        }
 
      // get the table columns...
      columns = [aDecoder decodeObjectForKey: @"NSTableColumns"];
      e = [columns objectEnumerator];
      while ((col = [e nextObject]) != nil)
        {
          /* Will initialize -[NSTableColumn tableView], _numberOfColumns and 
             allocate _columnsOrigins */
          [self addTableColumn: col];
        }

      [self tile]; /* Initialize _columnOrigins */
    }
  else
    {
      int version = [aDecoder versionForClassName: 
                                  @"NSTableView"];
      id aDelegate;
      float rowHeight;
      int number;

      [self _initDefaults];
      _dataSource      = [aDecoder decodeObject];
      _tableColumns    = RETAIN([aDecoder decodeObject]);
      _gridColor       = RETAIN([aDecoder decodeObject]);
      _backgroundColor = RETAIN([aDecoder decodeObject]);
      _headerView      = RETAIN([aDecoder decodeObject]);
      _cornerView      = RETAIN([aDecoder decodeObject]);
      aDelegate        = [aDecoder decodeObject];
      _target          = [aDecoder decodeObject];

      [self setDelegate: aDelegate];
      [_headerView setTableView: self];
      [_tableColumns makeObjectsPerformSelector: @selector(setTableView:)
                     withObject: self];

      [aDecoder decodeValueOfObjCType: @encode(int) at: &number];
      _numberOfRows = number;
      [aDecoder decodeValueOfObjCType: @encode(int) at: &number];
      _numberOfColumns = number;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_drawsGrid];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &rowHeight];
      _rowHeight = rowHeight;
      [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_doubleAction];
      _intercellSpacing = [aDecoder decodeSize];
      
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsMultipleSelection];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsEmptySelection];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsColumnSelection];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsColumnResizing];
      if (version >= 3)
        {
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsColumnReordering];
        }
      if (version >= 2)
        {
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_autoresizesAllColumnsToFit];
        } 
      
      if (version >= 4)
        {
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_verticalMotionDrag];
        }
      if (version >= 5)
	{
	  ASSIGN(_sortDescriptors, [aDecoder decodeObject]);
	}
     
      if (_numberOfColumns > 0)
        {
          _columnOrigins = NSZoneMalloc (NSDefaultMallocZone (), 
                                         sizeof(CGFloat) * _numberOfColumns);
        }
      [self tile]; /* Initialize _columnOrigins */
    }
  
  return self;
}

- (void) updateCell: (NSCell*)aCell
{
  NSInteger i, j;

  if (aCell == nil)
    return;

  return;

  for (i = 0; i < _numberOfColumns; i++)
    {
      if ([self preparedCellAtColumn: i row: -1] == aCell)
	{
	  [self setNeedsDisplayInRect: [self rectOfColumn: i]];
	}
      else
	{
	  NSRect columnRect = [self rectOfColumn: i];
	  NSRect rowRect;
	  NSRect visibleRect = [self convertRect: [_super_view bounds]
				     toView: self];
	  NSPoint top = NSMakePoint(NSMinX(visibleRect),
				    NSMinY(visibleRect));
	  NSPoint bottom = NSMakePoint(NSMinX(visibleRect),
				       NSMaxY(visibleRect));
	  NSInteger firstVisibleRow = [self rowAtPoint: top];
	  NSInteger lastVisibleRow = [self rowAtPoint: bottom];

	  if (firstVisibleRow == -1)
	    firstVisibleRow = 0;

	  if (lastVisibleRow == -1)
	    lastVisibleRow = _numberOfColumns - 1;

	  for (j = firstVisibleRow; j < lastVisibleRow; j++)
	    {
	      if ([self preparedCellAtColumn: i row: j] == aCell)
		{
		  rowRect = [self rectOfRow: j];
		  [self setNeedsDisplayInRect:
			  NSIntersectionRect(columnRect, rowRect)];
		}
	    }
	}
    }
}

- (void) _userResizedTableColumn: (NSInteger)index
			   width: (CGFloat)width
{
  [[_tableColumns objectAtIndex: index] setWidth: width];
}

- (CGFloat *) _columnOrigins
{
  return _columnOrigins;
}

- (void) _mouseDownInHeaderOfTableColumn: (NSTableColumn *)tc
{
  if ([_delegate 
	respondsToSelector:
	  @selector(tableView:mouseDownInHeaderOfTableColumn:)])
    {
      [_delegate tableView: self
		 mouseDownInHeaderOfTableColumn: tc];
    }
}

- (void) _clickTableColumn: (NSTableColumn *)tc
{
  NSSortDescriptor *oldMainSortDescriptor = nil;
  NSSortDescriptor *newMainSortDescriptor = [tc sortDescriptorPrototype];
  NSMutableArray *newSortDescriptors = 
    [NSMutableArray arrayWithArray: [self sortDescriptors]];
  NSEnumerator *e = [newSortDescriptors objectEnumerator];
  NSSortDescriptor *descriptor = nil;
  NSMutableArray *outdatedDescriptors = [NSMutableArray array];

  if ([[self sortDescriptors] count] > 0)
    {
      oldMainSortDescriptor = [[self sortDescriptors] objectAtIndex: 0];
    }

  /* Remove every main descriptor equivalents (normally only one) */
  while ((descriptor = [e nextObject]) != nil)
    {
      if ([[descriptor key] isEqual: [newMainSortDescriptor key]])
        [outdatedDescriptors addObject: descriptor];
    }

  /* Invert the sort direction when the same column header is clicked twice */
  if ([[newMainSortDescriptor key] isEqual: [oldMainSortDescriptor key]])
    {
      newMainSortDescriptor = [oldMainSortDescriptor reversedSortDescriptor];
    }

  [newSortDescriptors removeObjectsInArray: outdatedDescriptors];
  if (newMainSortDescriptor != nil)
	[newSortDescriptors insertObject: newMainSortDescriptor atIndex: 0];

  [self setSortDescriptors: newSortDescriptors];

  [self _didClickTableColumn: tc];
}

- (void) _editNextCellAfterRow: (NSInteger) row
		inColumn: (NSInteger) column
{
  if (++row >= _numberOfRows)
    row = 0;

  if ([self _shouldSelectRow: row])
    {
      [self selectRowIndexes: [NSIndexSet indexSetWithIndex: row]
	      byExtendingSelection: NO];

      if ([self _isCellEditableColumn: column row:row])
        {
          [self editColumn: column
                row: row
                withEvent: nil
                select: YES]; 
        }
    }
}

-(BOOL) _editNextEditableCellAfterRow: (NSInteger)row
                               column: (NSInteger)column
{
  NSInteger i, j;
  
  if (row > -1)
    {
      // First look for cells in the same row
      for (j = column + 1; j < _numberOfColumns; j++)
        {
          if ([self _isCellEditableColumn: j row: row])
            {
              [self editColumn: j row: row withEvent: nil select: YES];
              return YES;
            }
        }
    }

  // Otherwise, make the big cycle.
  for (i = row + 1; i < _numberOfRows; i++)
    {
      for (j = 0; j < _numberOfColumns; j++)
        {
          if ([self _isCellEditableColumn: j row: i])
            {
              // Need to select row to be able to edit it.
              [self selectRow: i byExtendingSelection: NO];
              [self editColumn: j row: i withEvent: nil select: YES];
              return YES;
            }
        }
    }

// Should we loop around or not?
#if 0
  // Nothing found? Search in the rows before the current
  for (i = 0; i < row; i++)
    {
      for (j = 0; j < _numberOfColumns; j++)
        {
          if ([self _isCellEditableColumn: j row: i])
            {
              // Need to select row to be able to edit it.
              [self selectRow: i byExtendingSelection: NO];
              [self editColumn: j row: i withEvent: nil select: YES];
              return YES;
            }
        }
    }

  // Still nothing? Look at the beginning of the current row
  if (row > -1)
    {
      // First look for cells in the same row
      for (j = 0; j < column; j++)
        {
          if ([self _isCellEditableColumn: j row: row])
            {
              [self editColumn: j row: row withEvent: nil select: YES];
              return YES;
            }
        }
    }
#endif

  return NO;
}

-(BOOL) _editPreviousEditableCellBeforeRow: (NSInteger)row
				    column: (NSInteger)column
{
  NSInteger i, j;

  if (row > -1)
    {
      // First look for cells in the same row
      for (j = column - 1; j > -1; j--)
        {
          if ([self _isCellEditableColumn: j row: row])
            {
              [self editColumn: j row: row withEvent: nil select: YES];
              return YES;
            }
        }
    }

  // Otherwise, make the big cycle.
  for (i = row - 1; i > -1; i--)
    {
      for (j = _numberOfColumns - 1; j > -1; j--)
        {
          if ([self _isCellEditableColumn: j row: i])
            {
              // Need to select row to be able to edit it.
              [self selectRow: i byExtendingSelection: NO];
              [self editColumn: j row: i withEvent: nil select: YES];
              return YES;
            }
        }
    }

// Should we loop around or not?
#if 0
  // Nothing found? Search in the rows after the current
  for (i = _numberOfRows - 1; i > row; i--)
    {
      for (j = _numberOfColumns - 1; j > -1; j--)
        {
          if ([self _isCellEditableColumn: j row: i])
            {
              // Need to select row to be able to edit it.
              [self selectRow: i byExtendingSelection: NO];
              [self editColumn: j row: i withEvent: nil select: YES];
              return YES;
            }
        }
    }

  // Still nothing? Look at the end of the current row
  if (row > -1)
    {
      // First look for cells in the same row
      for (j = _numberOfColumns - 1; j > column; j++)
        {
          if ([self _isCellEditableColumn: j row: row])
            {
              [self editColumn: j row: row withEvent: nil select: YES];
              return YES;
            }
        }
    }
#endif

  return NO;
}

- (void) _autosaveTableColumns
{
  if (_autosaveTableColumns && _autosaveName != nil) 
    {
      NSUserDefaults      *defaults;
      NSString            *tableKey;
      NSMutableDictionary *config;
      NSTableColumn       *column;
      id                  en;

      defaults  = [NSUserDefaults standardUserDefaults];
      tableKey = [NSString stringWithFormat: @"NSTableView Columns %@", 
			   _autosaveName];
      config = [NSMutableDictionary new];
      
      en = [[self tableColumns] objectEnumerator];
      while ((column = [en nextObject]) != nil)
	{
	  NSArray *array;
	  NSNumber *width, *identNum;
	  id ident;
	  
	  width = [NSNumber numberWithInt: [column width]];
	  ident = [column identifier];
	  identNum = [NSNumber numberWithInt: [self columnWithIdentifier: 
						      ident]];
	  array = [NSArray arrayWithObjects: width, identNum, nil];  
	  [config setObject: array  forKey: ident];      
	} 
      [defaults setObject: config  forKey: tableKey];
      [defaults synchronize];
      RELEASE (config);
    }
}

- (void) _autoloadTableColumns
{
  if (_autosaveTableColumns && _autosaveName != nil) 
    { 
      NSUserDefaults     *defaults;
      NSDictionary       *config;
      NSString           *tableKey;

      defaults  = [NSUserDefaults standardUserDefaults];
      tableKey = [NSString stringWithFormat: @"NSTableView Columns %@", 
			   _autosaveName];
      config = [defaults objectForKey: tableKey];
      if (config != nil) 
	{
	  NSEnumerator *en = [[config allKeys] objectEnumerator];
	  NSString *colKey;
	  NSArray *colDesc; 
	  NSTableColumn *col;
	  
	  while ((colKey = [en nextObject]) != nil) 
	    {
	      col = [self tableColumnWithIdentifier: colKey];
	      
	      if (col != nil)
		{
		  colDesc = [config objectForKey: colKey];
		  [col setWidth: [[colDesc objectAtIndex: 0] intValue]];
		  [self moveColumn: [self columnWithIdentifier: colKey]
			toColumn: [[colDesc objectAtIndex: 1] intValue]];
		}
	    }
	}
    }
}

- (void) superviewFrameChanged: (NSNotification*)aNotification
{
  if (_autoresizesAllColumnsToFit == YES)
    {
      CGFloat visible_width = [self convertRect: [_super_view bounds] 
				  fromView: _super_view].size.width;
      CGFloat table_width = 0;

      if (_numberOfColumns > 0)
        {
          table_width = 
            _columnOrigins[_numberOfColumns - 1] +
            [[_tableColumns objectAtIndex: _numberOfColumns - 1] width];
        }
      
      /*
	NSLog(@"columnOrigins[0] %f", _columnOrigins[0]);
	NSLog(@"superview.bounds %@", 
	      NSStringFromRect([_super_view bounds]));
	NSLog(@"superview.frame %@", 
	      NSStringFromRect([_super_view frame]));
	NSLog(@"table_width %f", table_width);
	NSLog(@"width %f", visible_width);
	NSLog(@"_superview_width %f", _superview_width);
      */

      if (table_width - _superview_width <= 0.001
	   && table_width - _superview_width >= -0.001)
	{
	  // the last column had been sized to fit
	  [self sizeToFit];
	}
      else if (table_width <= _superview_width
		&& table_width >= visible_width)
	{
	  // the tableView was too small and is now too large
	  [self sizeToFit];
	}
      else if (table_width >= _superview_width
	       && table_width <= visible_width)
	{
	  // the tableView was too large and is now too small
	  if (_numberOfColumns > 0)
	    [self scrollColumnToVisible: 0];
	  [self sizeToFit];
	}
      _superview_width = visible_width;
    }
  else
    {
      CGFloat visible_width = [self convertRect: [_super_view bounds] 
				  fromView: _super_view].size.width;
      CGFloat table_width = 0;

      if (_numberOfColumns > 0)
        {
          table_width = 
            _columnOrigins[_numberOfColumns - 1] +
            [[_tableColumns objectAtIndex: _numberOfColumns - 1] width];
        }
      
      /*
	NSLog(@"columnOrigins[0] %f", _columnOrigins[0]);
	NSLog(@"superview.bounds %@", 
	      NSStringFromRect([_super_view bounds]));
	NSLog(@"superview.frame %@", 
	      NSStringFromRect([_super_view frame]));
	NSLog(@"table_width %f", table_width);
	NSLog(@"width %f", visible_width);
	NSLog(@"_superview_width %f", _superview_width);
      */

      if (table_width - _superview_width <= 0.001
	   && table_width - _superview_width >= -0.001)
	{
	  // the last column had been sized to fit
	  [self sizeLastColumnToFit];
	}
      else if (table_width <= _superview_width
		&& table_width >= visible_width)
	{
	  // the tableView was too small and is now too large
	  [self sizeLastColumnToFit];
	}
      else if (table_width >= _superview_width
	       && table_width <= visible_width)
	{
	  // the tableView was too large and is now too small
	  if (_numberOfColumns > 0)
	    [self scrollColumnToVisible: 0];
	  [self sizeLastColumnToFit];
	}
      _superview_width = visible_width;
    }
  [self setFrame:_frame];
}


- (NSDragOperation) draggingSourceOperationMaskForLocal: (BOOL)isLocal
{
  if (isLocal)
    {
      return _draggingSourceOperationMaskForLocal;
    }
  else
    {
      return _draggingSourceOperationMaskForRemote;
    }
}

- (void) setDraggingSourceOperationMask: (NSDragOperation)mask
                               forLocal: (BOOL)isLocal
{
  if (isLocal)
    {
      _draggingSourceOperationMaskForLocal = mask;
    }
  else
    {
      _draggingSourceOperationMaskForRemote = mask;
    }
}

- (NSDragOperation) draggingEntered: (id <NSDraggingInfo>) sender
{
  currentDropRow = -1;
  currentDropOperation = -1;
  oldDropRow = -1;
  lastQuarterPosition = -1;
  oldDraggingRect = NSMakeRect(0.,0., 0., 0.);
  currentDragOperation = NSDragOperationEvery;
  return currentDragOperation;
}

- (void) draggingExited: (id <NSDraggingInfo>) sender
{
  [self setNeedsDisplayInRect: oldDraggingRect];
  [self displayIfNeeded];
}

- (void) _drawDropIndicator
{
  NSRect newRect = NSZeroRect;

  [self lockFocus];
  [self setNeedsDisplayInRect: oldDraggingRect];
  [self displayIfNeeded];

  [[NSColor darkGrayColor] set];

  if (currentDropRow == -1)
    {
	   newRect = [self bounds];
	   NSFrameRectWithWidth(newRect, 2.0);
	   oldDraggingRect = newRect;
	}
  else if (currentDropOperation == NSTableViewDropAbove)
	{
	  if (currentDropRow == 0)
		{
		  newRect = NSMakeRect([self visibleRect].origin.x,
					currentDropRow * _rowHeight,
					[self visibleRect].size.width,
					3);
		}
	  else if (currentDropRow == _numberOfRows)
		{
		  newRect = NSMakeRect([self visibleRect].origin.x,
					currentDropRow * _rowHeight - 2,
					[self visibleRect].size.width,
					3);
		}
	  else
	    {
          newRect = NSMakeRect([self visibleRect].origin.x,
				    currentDropRow * _rowHeight - 1,
				    [self visibleRect].size.width,
				    3);
	    }
	  NSRectFill(newRect);
	  oldDraggingRect = newRect;
	}
  else
	{
	  newRect = [self frameOfCellAtColumn: 0
	                                  row: currentDropRow];
	  newRect.origin.x = _bounds.origin.x;
	  newRect.size.width = _bounds.size.width + 2;
	  newRect.origin.x -= _intercellSpacing.height / 2;
	  newRect.size.height += _intercellSpacing.height;

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
	  NSFrameRectWithWidth(newRect, 2.0);

	  oldDraggingRect = newRect;
	  oldDraggingRect.origin.y -= 1;
	  oldDraggingRect.size.height += 2;
	}

	[_window flushWindow];
	[self unlockFocus];
}

/* This is a crude method of scrolling the view while dragging so the user can 
drag to any cell even if it's not visible. Unfortunately we don't receive 
events when the drag is outside the view, so the pointer must still be in the 
view to drag. */
- (void) _scrollRowAtPointToVisible: (NSPoint)p
{
  NSInteger currentRow;

  if (p.y < NSMinY([self visibleRect]) + 3)
    {
      currentRow = [self rowAtPoint: p] - 1;
      if (currentRow > 0)
        [self scrollRowToVisible: currentRow];
    }
  else if (p.y > NSMaxY([self visibleRect]) - 3)
    {
      currentRow = [self rowAtPoint: p] + 1;
      if (currentRow < _numberOfRows)
        [self scrollRowToVisible: currentRow];
    }
}

- (NSInteger) _computedRowAtPoint: (NSPoint)p
{
  return (NSInteger)(p.y - _bounds.origin.y) / (NSInteger)_rowHeight;
}

- (void) _setDropOperationAndRow: (NSInteger)row
              usingPositionInRow: (NSInteger)positionInRow 
                         atPoint: (NSPoint)p
{
  NSParameterAssert(row > -1);
  BOOL isPositionInsideMiddleQuartersOfRow = 
    (positionInRow > _rowHeight / 4 && positionInRow <= (3 * _rowHeight) / 4);
  BOOL isDropOn = (row > _numberOfRows || isPositionInsideMiddleQuartersOfRow); 

  [self setDropRow: (isDropOn ? [self _computedRowAtPoint: p] : row)
     dropOperation: (isDropOn ? NSTableViewDropOn : NSTableViewDropAbove)];
}

- (NSInteger) _dropRowFromQuarterPosition: (NSInteger)quarterPosition
{
  if ((quarterPosition - oldDropRow * 4 <= 2) &&
      (quarterPosition - oldDropRow * 4 >= -3))
    {
      return oldDropRow;
    }
  else
    {
      return (quarterPosition + 2) / 4;
    }
}

- (NSDragOperation) draggingUpdated: (id <NSDraggingInfo>) sender
{
  NSPoint p = [self convertPoint: [sender draggingLocation] fromView: nil];
  NSInteger positionInRow = (NSInteger)(p.y - _bounds.origin.y) % (NSInteger)_rowHeight;
  NSInteger quarterPosition = (NSInteger)([self _computedRowAtPoint: p] * 4.);
  NSInteger row = [self _dropRowFromQuarterPosition: quarterPosition];
  NSDragOperation dragOperation = [sender draggingSourceOperationMask];
  BOOL isSameDropTargetThanBefore = (lastQuarterPosition == quarterPosition
    && currentDragOperation == dragOperation);

  [self _scrollRowAtPointToVisible: p];

  if (isSameDropTargetThanBefore)
    return currentDragOperation;

  /* Remember current drop target */
  currentDragOperation = dragOperation;
  lastQuarterPosition = quarterPosition;
 
  /* The user can retarget this default drop using -setDropRow:dropOperation: 
     in -tableView:validateDrop:proposedRow:proposedDropOperation:. */
  [self _setDropOperationAndRow: row 
             usingPositionInRow: positionInRow 
                        atPoint: p];

  if ([_dataSource respondsToSelector: 
      @selector(tableView:validateDrop:proposedRow:proposedDropOperation:)])
    {
      currentDragOperation = [_dataSource tableView: self
                                       validateDrop: sender
                                        proposedRow: currentDropRow
                              proposedDropOperation: currentDropOperation];
    }
  
  /* -setDropRow:dropOperation: can changes both currentDropRow and 
     currentDropOperation. Whether we have to redraw the drop indicator depends 
     on this change. */
  if (currentDropRow != oldDropRow || currentDropOperation != oldDropOperation)
    {
      [self _drawDropIndicator]; 
      oldDropRow = (currentDropRow > -1 ? currentDropRow : _numberOfRows);
      oldDropOperation = currentDropOperation;
    }

  return currentDragOperation;
}

- (BOOL) performDragOperation: (id<NSDraggingInfo>)sender
{
  if ([_dataSource respondsToSelector: @selector(tableView:acceptDrop:row:dropOperation:)])
    {
      return [_dataSource tableView: self
			  acceptDrop: sender
			  row: currentDropRow
			  dropOperation: currentDropOperation];
    }
  else
    return NO;
}

- (BOOL) prepareForDragOperation: (id<NSDraggingInfo>)sender
{
  [self setNeedsDisplayInRect: oldDraggingRect];
  [self displayIfNeeded];

  return YES;
}

- (void) concludeDragOperation:(id <NSDraggingInfo>)sender
{
}

- (BOOL) canDragRowsWithIndexes: (NSIndexSet *)indexes 
                        atPoint: (NSPoint)point
{
  return YES;
}

/* 
 * sorting 
 */

/** Sets the sort descriptors used to sort the rows and delegates the sorting 
to -tableView:didChangeSortDescriptors or -outlineView:didChangeSortDescriptors:
in NSOutlineView.

The delegate methods can retrieve the new sort descriptors with 
-sortDescriptors and override them with -setSortDescriptors:.<br />
The first object in the new sort descriptor array is the sort descriptor 
prototype returned by the table column whose header was the last clicked.
See -[NSTableColumn sortDescriptorPrototype].
 
This method is called automatically when you click on a table column header, 
so you shouldn't need to call it usually.

Take note the sort descriptors are encoded by the keyed archiving (rarely used 
since neither IB or Gorm support to set these directly). */
- (void) setSortDescriptors: (NSArray *)sortDescriptors
{
  NSArray *oldSortDescriptors = [self sortDescriptors];
  NSArray *newSortDescriptors = nil;

  /* To replicate precisely the Cocoa behavior */
  if (sortDescriptors == nil)
    {
      newSortDescriptors = [NSArray array];
    }
  else
    {
      /* _sortDescriptors must remain immutable since -sortDescriptors doesn't 
         return a defensive copy */
      newSortDescriptors = [NSArray arrayWithArray: sortDescriptors];
    }

  if ([newSortDescriptors isEqual: oldSortDescriptors])
    return;

  RETAIN(oldSortDescriptors);

  ASSIGN(_sortDescriptors, newSortDescriptors);
  [self _didChangeSortDescriptors: oldSortDescriptors];

  RELEASE(oldSortDescriptors);
}

/** Returns the current sort descriptors, usually updated every time a click 
happens on a table column header.

By default, returns an empty array.

For a more detailed explanation, -setSortDescriptors:. */
- (NSArray *)sortDescriptors
{
  return _sortDescriptors;
}

/*
 * User interface validation
 */
- (BOOL) validateUserInterfaceItem: (id <NSValidatedUserInterfaceItem>)anItem
{
  // FIXME
  return YES;
}

/*
 * (NotificationRequestMethods)
 */
- (void) _postSelectionIsChangingNotification
{
  [nc postNotificationName: NSTableViewSelectionIsChangingNotification
      object: self];
}

- (void) _postSelectionDidChangeNotification
{
  [nc postNotificationName: NSTableViewSelectionDidChangeNotification
      object: self];
}

- (void) _postColumnDidMoveNotificationWithOldIndex: (NSInteger) oldIndex
					   newIndex: (NSInteger) newIndex
{
  [nc postNotificationName: NSTableViewColumnDidMoveNotification
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
	NSTableViewColumnDidResizeNotification
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
		   @selector (tableView:shouldSelectTableColumn:)] == YES) 
    {
      if ([_delegate tableView: self  shouldSelectTableColumn: tableColumn] == NO)
	{
	  return NO;
	}
    }

  return YES;
}

- (BOOL) _shouldSelectRow: (NSInteger)rowIndex
{
  if ([_delegate respondsToSelector: 
		   @selector (tableView:shouldSelectRow:)] == YES) 
    {
      if ([_delegate tableView: self shouldSelectRow: rowIndex] == NO)
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
	respondsToSelector: @selector(tableView:sortDescriptorsDidChange:)])
    {
      [_dataSource tableView: self sortDescriptorsDidChange: oldSortDescriptors];
    }
}

- (void) _didClickTableColumn: (NSTableColumn *)tc
{
  if ([_delegate 
	respondsToSelector:
	  @selector(tableView:didClickTableColumn:)])
    {
      [_delegate tableView: self
		 didClickTableColumn: tc];
    }
}

- (BOOL) _shouldEditTableColumn: (NSTableColumn *)tableColumn
			    row: (NSInteger) rowIndex
{
  if ([_delegate respondsToSelector: 
		     @selector(tableView:shouldEditTableColumn:row:)])
    {
      return [_delegate tableView: self shouldEditTableColumn: tableColumn
			row: rowIndex];
    }

  return YES;
}

- (BOOL) _isEditableColumn: (NSInteger) columnIndex
                       row: (NSInteger) rowIndex
{
  NSTableColumn *tableColumn = [_tableColumns objectAtIndex: columnIndex];

  return [tableColumn isEditable] && [self _shouldEditTableColumn: tableColumn
                                                              row: rowIndex];
}

- (BOOL) _isCellSelectableColumn: (NSInteger) columnIndex
                             row: (NSInteger) rowIndex
{
  if (![self _isEditableColumn: columnIndex row: rowIndex])
    {
      return NO;
    }
  else
    {
      NSCell *cell = [self preparedCellAtColumn: columnIndex row: rowIndex];

      return [cell isSelectable];
    }
}

- (BOOL) _isCellEditableColumn: (NSInteger) columnIndex
			   row: (NSInteger) rowIndex
{
  if (![self _isEditableColumn: columnIndex row: rowIndex])
    {
      return NO;
    }
  else
    {
      NSCell *cell = [self preparedCellAtColumn: columnIndex row: rowIndex];

      return [cell isEditable];
    }
}

- (void) _willDisplayCell: (NSCell*)cell
	   forTableColumn: (NSTableColumn *)tb
		      row: (NSInteger)index
{
  if (_del_responds)
    {
      [_delegate tableView: self   
		 willDisplayCell: cell 
		 forTableColumn: tb   
		 row: index];
    }    
}

- (id) _objectValueForTableColumn: (NSTableColumn *)tb
			      row: (NSInteger) index
{
  id result = nil;
  GSKeyValueBinding *theBinding;

  theBinding = [GSKeyValueBinding getBinding: NSValueBinding 
                                   forObject: tb];
  if (theBinding != nil)
    {
      return [(NSArray *)[theBinding destinationValue]
                 objectAtIndex: index];
    }
  else if ([_dataSource respondsToSelector:
		    @selector(tableView:objectValueForTableColumn:row:)])
    {
      result = [_dataSource tableView: self
			    objectValueForTableColumn: tb
			    row: index];
    }

  return result;
}

- (void) _setObjectValue: (id)value
	  forTableColumn: (NSTableColumn *)tb
		     row: (NSInteger) index
{
  if ([_dataSource respondsToSelector:
		    @selector(tableView:setObjectValue:forTableColumn:row:)])
    {
      [_dataSource tableView: self
		   setObjectValue: value
		   forTableColumn: tb
		   row: index];
    }
}

/* Quasi private method called on self from -noteNumberOfRowsChanged
 * implemented in NSTableView and subclasses 
 * by default returns the DataSource's -numberOfRowsInTableView:
 */
- (NSInteger) _numRows
{
  GSKeyValueBinding *theBinding;

  // If we have content binding the data source is used only
  // like a delegate
  theBinding = [GSKeyValueBinding getBinding: NSContentBinding 
                                   forObject: self];
  if (theBinding != nil)
    {
      return [(NSArray *)[theBinding destinationValue] count];
    }
  else if ([_dataSource respondsToSelector:
		    @selector(numberOfRowsInTableView:)])
    {
      return [_dataSource numberOfRowsInTableView:self];
    }
  else
    {
      NSTableColumn *tb = [_tableColumns objectAtIndex: 0];
      GSKeyValueBinding *theBinding;

      theBinding = [GSKeyValueBinding getBinding: NSValueBinding
                                       forObject: tb];
      if (theBinding != nil)
        {
          return [[theBinding destinationValue] count];
        }

      // FIXME
      return 0;
    }
}

- (BOOL) _isDraggingSource
{
  return [_dataSource respondsToSelector:
			@selector(tableView:writeRows:toPasteboard:)] 
      || [_dataSource respondsToSelector:
           @selector(tableView:writeRowsWithIndexes:toPasteboard:)];
}

- (BOOL) _writeRows: (NSIndexSet *)rows
       toPasteboard: (NSPasteboard *)pboard
{
  if ([_dataSource respondsToSelector:
		     @selector(tableView:writeRowsWithIndexes:toPasteboard:)] == YES)
    {
      return [_dataSource tableView: self
                          writeRowsWithIndexes: rows
                          toPasteboard: pboard];
    }
  else if ([_dataSource respondsToSelector:
             @selector(tableView:writeRows:toPasteboard:)] == YES)
    {
      NSArray *rowArray;

      rowArray = [self _indexSetToArray: rows];
      return [_dataSource tableView: self
                          writeRows: rowArray
                          toPasteboard: pboard];
    }
  return NO;
}

- (void) reloadDataForRowIndexes: (NSIndexSet*)rowIndexes
                   columnIndexes: (NSIndexSet*)columnIndexes
{
  [self reloadData];
}

- (void) beginUpdates
{
  _beginEndUpdates++;
}

- (void) endUpdates
{
  if (_beginEndUpdates > 0)
    {
      if (--_beginEndUpdates == 0)
        {
          // Process batched inserts/removes....
          // Just reload table for now until we get inserts/removes working...
          [self reloadData];
        }
    }
}

- (NSInteger) columnForView: (NSView*)view
{
  return NSNotFound;
}

- (void) insertRowsAtIndexes: (NSIndexSet*)indexes
               withAnimation: (NSTableViewAnimationOptions)animationOptions
{
}

- (void) removeRowsAtIndexes: (NSIndexSet*)indexes
               withAnimation: (NSTableViewAnimationOptions)animationOptions
{
}

- (NSInteger) rowForView: (NSView*)view
{
  return NSNotFound;
}

@end /* implementation of NSTableView */

@implementation NSTableView (SelectionHelper)

- (void) _setSelectingColumns: (BOOL)flag
{
  if (flag == _selectingColumns)
    return;
  
  if (flag == NO)
    {
      [self _unselectAllColumns];
      _selectingColumns = NO;
    }
  else
    {
      [self _unselectAllRows];
      _selectingColumns = YES;
    }
}

- (NSArray *) _indexSetToArray: (NSIndexSet*)indexSet
{
  NSMutableArray *array = [NSMutableArray array];
  NSUInteger index = [indexSet firstIndex];
      
  while (index != NSNotFound)
    {
      NSNumber *num  = [NSNumber numberWithUnsignedInteger: index];

      [array addObject: num]; 
      index = [indexSet indexGreaterThanIndex: index];
    }  
	    
  return array;
}

- (NSArray *) _selectedRowArray
{
  return [self _indexSetToArray: _selectedRows];
}

- (BOOL) _selectRow: (NSInteger)rowIndex
{
  if (![self _shouldSelectRow: rowIndex])
    {
      return NO;
    }
  
  [self setNeedsDisplayInRect: [self rectOfRow: rowIndex]];
  [_selectedRows addIndex: rowIndex];
  _selectedRow = rowIndex;
  return YES;
}

- (BOOL) _selectUnselectedRow: (NSInteger)rowIndex
{
  if ([_selectedRows containsIndex: rowIndex])
    {
      return NO;
    }
  
  [self setNeedsDisplayInRect: [self rectOfRow: rowIndex]];
  [_selectedRows addIndex: rowIndex];
  _selectedRow = rowIndex;
  return YES;
}

- (BOOL) _unselectRow: (NSInteger)rowIndex
{
  if (![_selectedRows containsIndex: rowIndex])
    {
      return NO;
    }

  [self setNeedsDisplayInRect: [self rectOfRow: rowIndex]];
  [_selectedRows removeIndex: rowIndex];

  if (_selectedRow == rowIndex)
    {
      _selectedRow = -1;
    }

  return YES;
}

- (void) _unselectAllRows
{
  /* Compute rect to redraw to clear the old row selection */
  NSUInteger row = [_selectedRows firstIndex];
      
  while (row != NSNotFound)
    {
      [self setNeedsDisplayInRect: [self rectOfRow: row]];
      row = [_selectedRows indexGreaterThanIndex: row];
    }
  [_selectedRows removeAllIndexes];
  _selectedRow = -1;
}

- (NSArray *) _selectedColumArray
{
  return [self _indexSetToArray: _selectedColumns];
}

- (void) _unselectAllColumns
{
  /* Compute rect to redraw to clear the old column selection */
  NSUInteger column = [_selectedColumns firstIndex];
      
  while (column != NSNotFound)
    {
      [self setNeedsDisplayInRect: [self rectOfColumn: column]];
      if (_headerView)
        {		
	  [_headerView setNeedsDisplayInRect: 
			   [_headerView headerRectOfColumn: column]];
	}
      column = [_selectedColumns indexGreaterThanIndex: column];
    }	  
  [_selectedColumns removeAllIndexes];
  _selectedColumn = -1;  
}

- (void) setValue: (id)anObject forKey: (NSString*)aKey
{
  if ([aKey isEqual: NSContentBinding])
    {
      // Reload data
      [self reloadData];
      NSDebugLLog(@"NSBinding", @"Setting table view content to %@", anObject);
    }
  else if ([aKey isEqual: NSSelectionIndexesBinding])
    {
      if (_selectingColumns)
        {
          if (nil == anObject)
            {
              [self _unselectAllColumns];
            }
          else
            {
              return [self selectColumnIndexes: anObject
                          byExtendingSelection: NO];
            }
        }
      else
        {
          if (nil == anObject)
            {
              [self _unselectAllRows];
            }
          else
            {
              return [self selectRowIndexes: anObject
                       byExtendingSelection: NO];
            }
        }
    }
  else
    {
      [super setValue: anObject forKey: aKey];
    }
}

- (id) valueForKey: (NSString*)aKey
{
  if ([aKey isEqual: NSContentBinding])
    {
      return nil;
    }
  else if ([aKey isEqual: NSSelectionIndexesBinding])
    {
      if (_selectingColumns)
        {
          return [self selectedColumnIndexes];
        }
      else
        {
          return [self selectedRowIndexes];
        }
    }
  else
    {
      return [super valueForKey: aKey];
    }
}

@end
