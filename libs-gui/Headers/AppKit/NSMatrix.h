/* 
   NSMatrix.h

   Copyright (C) 1996-2015 Free Software Foundation, Inc.

   Author:  Ovidiu Predescu <ovidiu@net-community.com>
   Date: March 1997
   A completely rewritten version of the original source by Pascal Forget and
   Scott Christley.
   
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

#ifndef _GNUstep_H_NSMatrix
#define _GNUstep_H_NSMatrix
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSControl.h>

@class NSArray;
@class NSMutableArray;
@class NSNotification;

@class NSCell;
@class NSColor;
@class NSText;
@class NSEvent;

typedef enum _NSMatrixMode {
  NSRadioModeMatrix,
  NSHighlightModeMatrix,
  NSListModeMatrix,
  NSTrackModeMatrix 
} NSMatrixMode;

@protocol NSMatrixDelegate <NSControlTextEditingDelegate>
@end

@interface NSMatrix : NSControl <NSCoding>
{
  __strong id		**_cells;
  BOOL		**_selectedCells;
  NSInteger	_maxRows;
  NSInteger	_maxCols;
  NSInteger	_numRows;
  NSInteger	_numCols;
  NSZone	*_myZone;
  Class		_cellClass;
  id		_cellPrototype;
  IMP		_cellNew;
  IMP		_cellInit;
  NSMatrixMode	_mode;
  NSSize	_cellSize;
  NSSize	_intercell;
  NSColor	*_backgroundColor;
  NSColor	*_cellBackgroundColor;
  id		_delegate;
  NSText*       _textObject;        
  BOOL          _tabKeyTraversesCells;
  id		_target;
  SEL		_action;
  SEL		_doubleAction;
  SEL		_errorAction;
  id		_selectedCell;
  NSInteger	_selectedRow;
  NSInteger	_selectedColumn;
  BOOL		_allowsEmptySelection;
  BOOL		_selectionByRect;
  BOOL		_drawsBackground;
  BOOL		_drawsCellBackground;
  BOOL		_autosizesCells;
  BOOL		_autoscroll;
  id            _reserved1;
  NSInteger	_dottedRow;
  NSInteger	_dottedColumn;
}

/*
 * Initializing the NSMatrix Class 
 */
+ (Class) cellClass;
+ (void) setCellClass: (Class)classId;

/*
 * Initializing an NSMatrix Object
 */
- (id) initWithFrame: (NSRect)frameRect;
- (id) initWithFrame: (NSRect)frameRect
		mode: (NSMatrixMode)aMode
	   cellClass: (Class)classId
	numberOfRows: (NSInteger)rowsHigh
     numberOfColumns: (NSInteger)colsWide;
- (id) initWithFrame: (NSRect)frameRect
		mode: (NSMatrixMode)aMode
	   prototype: (NSCell *)aCell
	numberOfRows: (NSInteger)rowsHigh
     numberOfColumns: (NSInteger)colsWide;

/*
 * Setting the Selection Mode 
 */
- (NSMatrixMode) mode;
- (void) setMode: (NSMatrixMode)aMode;

/*
 * Configuring the NSMatrix 
 */
- (BOOL) allowsEmptySelection;
- (BOOL) isSelectionByRect;
- (void) setAllowsEmptySelection: (BOOL)flag;
- (void) setSelectionByRect: (BOOL)flag;

/*
 * Setting the Cell Class 
 */
- (Class) cellClass;
- (id) prototype;
- (void) setCellClass: (Class)classId;
- (void) setPrototype: (NSCell *)aCell;

/*
 * Laying Out the NSMatrix 
 */
- (void) addColumn;
- (void) addColumnWithCells: (NSArray *)cellArray;
- (void) addRow;
- (void) addRowWithCells: (NSArray *)cellArray;
- (NSRect) cellFrameAtRow: (NSInteger)row
		   column: (NSInteger)column;
- (NSSize) cellSize;
- (void) getNumberOfRows: (NSInteger *)rowCount
		 columns: (NSInteger *)columnCount;
- (void) insertColumn: (NSInteger)column;
- (void) insertColumn: (NSInteger)column withCells: (NSArray *)cellArray;
- (void) insertRow: (NSInteger)row;
- (void) insertRow: (NSInteger)row withCells: (NSArray *)cellArray;
- (NSSize) intercellSpacing;
- (NSCell *) makeCellAtRow: (NSInteger)row
		    column: (NSInteger)column;
- (void) putCell: (NSCell *)newCell
	   atRow: (NSInteger)row
	  column: (NSInteger)column;
- (void) removeColumn: (NSInteger)column;
- (void) removeRow: (NSInteger)row;
- (void) renewRows: (NSInteger)newRows
	   columns: (NSInteger)newColumns;
- (void) setCellSize: (NSSize)aSize;
- (void) setIntercellSpacing: (NSSize)aSize;
- (void) sortUsingFunction: (NSComparisonResult (*)(id element1, id element2, void *userData))comparator
		   context: (void *)context;
- (void) sortUsingSelector: (SEL)comparator;
- (NSInteger) numberOfColumns;
- (NSInteger) numberOfRows;

/*
 * Finding Matrix Coordinates 
 */
- (BOOL) getRow: (NSInteger *)row
	 column: (NSInteger *)column
       forPoint: (NSPoint)aPoint;
- (BOOL) getRow: (NSInteger *)row
	 column: (NSInteger *)column
	 ofCell: (NSCell *)aCell;

/*
 * Modifying Individual Cells 
 */
- (void) setState: (NSInteger)value
	    atRow: (NSInteger)row
	   column: (NSInteger)column;

/*
 * Selecting Cells 
 */
- (void) deselectAllCells;
- (void) deselectSelectedCell;
- (void) selectAll: (id)sender;
- (void) selectCellAtRow: (NSInteger)row
		  column: (NSInteger)column;
- (BOOL) selectCellWithTag: (NSInteger)anInt;
- (id) selectedCell;
- (NSArray *) selectedCells;
- (NSInteger) selectedColumn;
- (NSInteger) selectedRow;
- (void) setSelectionFrom: (NSInteger)startPos
		       to: (NSInteger)endPos
		   anchor: (NSInteger)anchorPos
		highlight: (BOOL)flag;

/*
 * Finding Cells 
 */
- (id) cellAtRow: (NSInteger)row
	  column: (NSInteger)column;
- (id) cellWithTag: (NSInteger)anInt;
- (NSArray *) cells;

/*
 * Modifying Graphic Attributes 
 */
- (NSColor *) backgroundColor;
- (NSColor *) cellBackgroundColor;
- (BOOL) drawsBackground;
- (BOOL) drawsCellBackground;
- (void) setBackgroundColor: (NSColor *)aColor;
- (void) setCellBackgroundColor: (NSColor *)aColor;
- (void) setDrawsBackground: (BOOL)flag;
- (void) setDrawsCellBackground: (BOOL)flag;

/*
 * Editing Text in Cells 
 */
- (void) selectText: (id)sender;
- (id) selectTextAtRow: (NSInteger)row
		column: (NSInteger)column;
- (void) textDidBeginEditing: (NSNotification *)aNotification;
- (void) textDidChange: (NSNotification *)aNotification;
- (void) textDidEndEditing: (NSNotification *)aNotification;
- (BOOL) textShouldBeginEditing: (NSText *)aTextObject;
- (BOOL) textShouldEndEditing: (NSText *)aTextObject;

/*
 * Setting Tab Key Behavior 
 */
- (id) keyCell;
- (void) setKeyCell: (NSCell *)aCell;
- (id) nextText;
- (id) previousText;
- (void) setNextText: (id)anObject;
- (void) setPreviousText: (id)anObject;
- (BOOL) tabKeyTraversesCells;
- (void) setTabKeyTraversesCells: (BOOL)flag;

/*
 * Assigning a Delegate 
 */
- (void) setDelegate: (id)anObject;
- (id) delegate;

/*
 * Resizing the Matrix and Cells 
 */
- (BOOL) autosizesCells;
- (void) setAutosizesCells: (BOOL)flag;
- (void) setValidateSize: (BOOL)flag;
- (void) sizeToCells;

/*
 * Scrolling 
 */
- (BOOL) isAutoscroll;
- (void) scrollCellToVisibleAtRow: (NSInteger)row
			   column: (NSInteger)column;
- (void) setAutoscroll: (BOOL)flag;
- (void) setScrollable: (BOOL)flag;

/*
 * Displaying 
 */
- (void) drawCellAtRow: (NSInteger)row
		column: (NSInteger)column;
- (void) highlightCell: (BOOL)flag
		 atRow: (NSInteger)row
		column: (NSInteger)column;

/*
 *Target and Action 
 */
- (void) setAction: (SEL)aSelector;
- (SEL) action;
- (void) setDoubleAction: (SEL)aSelector;
- (SEL) doubleAction;
- (void) setErrorAction: (SEL)aSelector;
- (SEL) errorAction;
- (BOOL) sendAction;
- (void) sendAction: (SEL)aSelector
		 to: (id)anObject
	forAllCells: (BOOL)flag;
- (void) sendDoubleAction;

/*
 * Handling Event and Action Messages 
 */
- (BOOL) acceptsFirstMouse: (NSEvent *)theEvent;
- (void) mouseDown: (NSEvent *)theEvent;
- (NSInteger) mouseDownFlags;
- (BOOL) performKeyEquivalent: (NSEvent *)theEvent;

/*
 * Managing the Cursor 
 */
- (void) resetCursorRects;

/*
 * Handling tool tips
 */
- (NSString *) toolTipForCell: (NSCell *)cell;
- (void) setToolTip: (NSString *)toolTipString forCell: (NSCell *)cell;

@end

#endif /* _GNUstep_H_NSMatrix */
