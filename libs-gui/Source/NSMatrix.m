/** <title>NSMatrix</title>

   <abstract>Matrix class for grouping controls</abstract>

   Copyright (C) 1996-2015 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: March 1997
   A completely rewritten version of the original source by Pascal Forget and
   Scott Christley.
   Modified: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
   Cell handling rewritten: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: November 1999
   Implementation of Editing: Nicola Pero <n.pero@mi.flashnet.it>
   Date: November 1999
   Modified: Mirko Viviani <mirko.viviani@rccr.cremona.it>
   Date: March 2001

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

/* Mouse Tracking Notes:

   The behaviour of mouse tracking is a bit different on OS42 and MaxOSX. The 
   implementation here reflects OS42 more closely (as the original code
   in NSMatrix). Examples of differences:
   - highlighting of NSButtonCells is different;
   - OS42 makes each cell under the cursor track the mouse, MacOSX makes only
     the clicked cell track it, untilMouseUp;
   - if mouse goes up outside of a cell, OS42 sends the action, MacOSX does not
   - keys used for selection in list mode are not the same (shift and alternate
     on OS42, command and shift on MacOSX).
*/

#include "config.h"
#include <stdlib.h>

#import <Foundation/NSValue.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSException.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSFormatter.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>
#import <Foundation/NSZone.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSWindow.h"
#import "GSCodingFlags.h"

#include <math.h>

static NSNotificationCenter *nc;

#define	NSMATRIX_STRICT_CHECKING	0

#ifdef MIN
# undef MIN
#endif
#define MIN(A,B)  ({ typeof(A) __a = (A); \
		  typeof(B) __b = (B); \
		  __a < __b ? __a : __b; })

#ifdef MAX
# undef MAX
#endif
#define MAX(A,B)  ({ typeof(A) __a = (A); \
		  typeof(B) __b = (B); \
		  __a < __b ? __b : __a; })

#ifdef ABS
# undef ABS
#endif
#define ABS(A)	({ typeof(A) __a = (A); __a < 0 ? -__a : __a; })


#define SIGN(x) \
    ({typeof(x) _SIGN_x = (x); \
      _SIGN_x > 0 ? 1 : (_SIGN_x == 0 ? 0 : -1); })

#define POINT_FROM_INDEX(index) \
    ({MPoint point = { (index) % _numCols, (index) / _numCols }; point; })

#define INDEX_FROM_COORDS(x,y) \
    ((y) * _numCols + (x))
#define INDEX_FROM_POINT(point) \
    ((point).y * _numCols + (point).x)


/* Some stuff needed to compute the selection in the list mode. */
typedef struct {
  NSInteger x;
  NSInteger y;
} MPoint;

typedef struct {
  NSInteger x;
  NSInteger y;
  NSInteger width;
  NSInteger height;
} MRect;

@interface NSMatrix (PrivateMethods)
- (void) _renewRows: (NSInteger)row
	    columns: (NSInteger)col
	   rowSpace: (NSInteger)rowSpace
	   colSpace: (NSInteger)colSpace;
- (void) _setState: (NSInteger)state
	 highlight: (BOOL)highlight
	startIndex: (NSInteger)start
	  endIndex: (NSInteger)end;
- (BOOL) _selectNextSelectableCellAfterRow: (NSInteger)row
				    column: (NSInteger)column;
- (BOOL) _selectPreviousSelectableCellBeforeRow: (NSInteger)row
					 column: (NSInteger)column;
- (void) _setKeyRow: (NSInteger)row
             column: (NSInteger)column;
@end

enum {
  DEFAULT_CELL_HEIGHT = 17,
  DEFAULT_CELL_WIDTH = 100
};

/** <p>TODO documentation</p>
 */

@implementation NSMatrix

/* Class variables */
static Class defaultCellClass = nil;
static NSUInteger mouseDownFlags = 0;
static SEL copySel;
static SEL initSel;
static SEL allocSel;
static SEL getSel;

+ (void) initialize
{
  if (self == [NSMatrix class])
    {
      /* Set the initial version */
      [self setVersion: 1];

      copySel = @selector(copyWithZone:);
      initSel = @selector(init);
      allocSel = @selector(allocWithZone:);
      getSel = @selector(objectAtIndex:);

      /*
       * MacOS-X docs say default cell class is NSActionCell
       */
      defaultCellClass = [NSActionCell class];
      //
      nc = [NSNotificationCenter defaultCenter];

      [self exposeBinding: NSSelectedTagBinding];
    }
}

/**<p>Returns the cell class used to create cells. By default it is a
   NSActionCell class</p><p>See Also: +setCellClass:</p>
 */
+ (Class) cellClass
{
  return defaultCellClass;
}

/**<p>Sets the cell class used to create cells to <var>classId</var>.
   By default it is a NSActionCell class</p><p>See Also: +setCellClass:</p>
 */
+ (void) setCellClass: (Class)classId
{
  defaultCellClass = classId;
  if (defaultCellClass == nil)
    defaultCellClass = [NSActionCell class];
}

- (id) init
{
  return [self initWithFrame: NSZeroRect
		        mode: NSRadioModeMatrix
		   cellClass: [object_getClass(self) cellClass]
		numberOfRows: 0
	     numberOfColumns: 0];
}

/** <p>Initializes and returns a NSMatrix in frame frameRect.
    By default the matrix has no row and no column, the NSMatrix's mode is
    NSRadioModeMatrix and the cell class is a NSActionCell class.</p><p>See
    Also: -initWithFrame:mode:cellClass:numberOfRows:numberOfColumns:</p>
 */
- (id) initWithFrame: (NSRect)frameRect
{
  return [self initWithFrame: frameRect
		        mode: NSRadioModeMatrix
		   cellClass: [object_getClass(self) cellClass]
		numberOfRows: 0
	     numberOfColumns: 0];
}

- (void) _privateFrame: (NSRect)frameRect
                  mode: (NSMatrixMode)aMode
          numberOfRows: (NSInteger)rows
       numberOfColumns: (NSInteger)cols
{
  _myZone = [self zone];
  [self _renewRows: rows columns: cols rowSpace: 0 colSpace: 0];
  _mode = aMode;
  if ((_numCols > 0) && (_numRows > 0))
    {
      /* 
	 We must not round the _cellSize to integers here!
	 
	 Any approximation is a loss of information.  We should give
	 to the backend as much information as possible, and trust
	 that it will use that information to provide the best
	 possible rendering on that device.  Depending on the backend,
	 that might go up to using antialias or advanced graphics
	 tricks to make an advanced rendering of things not lying on
	 pixel boundaries.  Approximating here just gives less
	 information to the backend, making the rendering worse.
	 
	 Even if the backend is just approximating to pixels, it would
	 still be wrong to round _cellSize here, because rounding
	 sizes of rectangles without considering the origin of the
	 rectangles has been definitely found to be wrong and to cause
	 incorrect rendering.  The origin of the whole matrix is very
	 likely a non-integer - if not originally, as a consequence of
	 the fact that the user resized the window - so making the
	 cell size integer does not cause drawing to be done on pixel
	 boundaries anyway, and will actually make more difficult for
	 the backend to render the rectangles properly since it will
	 be drawing approximately rectangles which are already only an
	 approximate description - and this first approximation having
	 been done incorrectly too! - of what we really want to draw.
      */

      _cellSize = NSMakeSize (frameRect.size.width/_numCols,
			      frameRect.size.height/_numRows);
    }
  else
    {
      _cellSize = NSMakeSize (DEFAULT_CELL_WIDTH, DEFAULT_CELL_HEIGHT);
    }

  _intercell = NSMakeSize(1, 1);
  [self setAutosizesCells: YES];
  [self setFrame: frameRect];

  _tabKeyTraversesCells = YES;
  [self setBackgroundColor: [NSColor controlColor]];
  [self setDrawsBackground: NO];
  [self setCellBackgroundColor: [NSColor controlColor]];
  [self setDrawsCellBackground: NO];
  [self setSelectionByRect: YES];
  _dottedRow = _dottedColumn = -1;
  if (_mode == NSRadioModeMatrix && _numRows > 0 && _numCols > 0)
    {
      [self selectCellAtRow: 0 column: 0];
    }
  else
    {
      _selectedCell = nil;
      _selectedRow = _selectedColumn = -1;
    }
}

/**<p>Initializes and returns a new NSMatrix in the specified frame frameRect.
   The <ref type="type" id="NSMatrixMode">NSMatrixMode</ref> is specified
   by mode, the cell class used specified by classId and the number of rows
   and columns specified by rowsHigh and colsWide respectively</p><p>See Also:
   -initWithFrame:mode:prototype:numberOfRows:numberOfColumns:</p> 
 */
- (id) initWithFrame: (NSRect)frameRect
	        mode: (NSMatrixMode)aMode
	   cellClass: (Class)classId
        numberOfRows: (NSInteger)rowsHigh
     numberOfColumns: (NSInteger)colsWide
{
  if (!( self = [super initWithFrame: frameRect]))
    {
      return nil;
    }

  [self setCellClass: classId];
  [self _privateFrame: frameRect
                 mode: aMode
         numberOfRows: rowsHigh
      numberOfColumns: colsWide];
  return self;
}

/**<p>Initializes and returns a new NSMatrix in the specified frame frameRect.
   The <ref type="type" id="NSMatrixMode">NSMatrixMode</ref> is specified
   by mode, the cell used specified by aCell and the number of rows
   and columns specified by rowsHigh and colsWide respectively</p><p>See Also:
   -initWithFrame:mode:prototype:numberOfRows:numberOfColumns:</p> 
 */
- (id) initWithFrame: (NSRect)frameRect
	        mode: (NSMatrixMode)aMode
	   prototype: (NSCell*)aCell
        numberOfRows: (NSInteger)rowsHigh
     numberOfColumns: (NSInteger)colsWide
{
  if (!(self = [super initWithFrame: frameRect]))
    {
      return nil;
    }

  [self setPrototype: aCell];
  [self _privateFrame: frameRect
                 mode: aMode
         numberOfRows: rowsHigh
      numberOfColumns: colsWide];
  return self;
}

- (void) dealloc
{
  int i;

  if (_textObject != nil)
    {
      [_selectedCell endEditing: _textObject];
      _textObject = nil;
    }

  for (i = 0; i < _maxRows; i++)
    {
      int j;

      for (j = 0; j < _maxCols; j++)
	{
	  [_cells[i][j] release];
	}
      NSZoneFree(_myZone, _cells[i]);
      NSZoneFree(_myZone, _selectedCells[i]);
    }
  NSZoneFree(_myZone, _cells);
  NSZoneFree(_myZone, _selectedCells);

  [_cellPrototype release];
  [_backgroundColor release];
  [_cellBackgroundColor release];

  if (_delegate != nil)
    {
      [nc removeObserver: _delegate  name: nil  object: self];
      _delegate = nil;
    }

  [super dealloc];
}

/**<p>Inserts a new column after the current last column.</p>
   <p>See Also: -insertColumn:withCells: </p>
 */
- (void) addColumn
{
  [self insertColumn: _numCols withCells: nil];
}

/**<p>Inserts a new column of cells specified by cellArray after the current
   last column.</p><p>See Also: -insertColumn:withCells: </p>
 */
- (void) addColumnWithCells: (NSArray*)cellArray
{
  [self insertColumn: _numCols withCells: cellArray];
}

/**<p>Inserts a new row after the current last row.</p>
   <p>See Also: -insertRow:withCells: </p>
 */
- (void) addRow
{
  [self insertRow: _numRows withCells: nil];
}

/**<p>Inserts a new row of cells specified by cellArray
   after the current last row.</p><p>See Also: -insertRow:withCells: </p>
 */
- (void) addRowWithCells: (NSArray*)cellArray
{
  [self insertRow: _numRows withCells: cellArray];
}

/**<p>Inserts a new column at the specified column <var>column</var>.</p>
   <p>See Also: -insertColumn:withCells:</p> 
 */
- (void) insertColumn: (NSInteger)column
{
  [self insertColumn: column withCells: nil];
}

/**<p>Inserts a new column of cells ( specified by <var>cellArray</var>) 
   at the specified column <var>column</var>. This method can grows
   the matrix as necessay if needed</p>
   <p>See Also: -insertColumn:</p>
 */
- (void) insertColumn: (NSInteger)column withCells: (NSArray*)cellArray
{
  NSInteger count = [cellArray count];
  NSInteger i = _numCols + 1;

  if (column < 0)
    {
      column = 0;
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"insert negative column (%d) in matrix", (int)column);
#else
      [NSException raise: NSRangeException
		  format: @"insert negative column (%d) in matrix", (int)column];
#endif
    }

  if ((cellArray != nil) && (count != _numRows))
    {
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"Wrong number of cells (%d) in column insert in matrix", (int)count);
#else
      [NSException raise: NSRangeException
		  format: @"Wrong number of cells (%d) in column insert in matrix", (int)count];
#endif
    }

  if (column >= i)
    {
      i = column + 1;
    }

  /*
   * Use _renewRows:columns:rowSpace:colSpace: to grow the matrix as necessary.
   * MacOS-X docs say that if the matrix is empty, we make it have one column
   * and enough rows for all the elements.
   */
  if (count > 0 && (_numRows == 0 || _numCols == 0))
    {
      [self _renewRows: count columns: 1 rowSpace: 0 colSpace: count];
    }
  else
    {
      [self _renewRows: _numRows ? _numRows : 1
	       columns: i
	      rowSpace: 0
	      colSpace: count];
    }

  /*
   * Rotate the new column to the insertion point if necessary.
   */
  if (_numCols != column)
    {
      for (i = 0; i < _numRows; i++)
	{
	  int	j = _numCols;
	  id	old = _cells[i][j-1];

	  while (--j > column)
	    {
	      _cells[i][j] = _cells[i][j-1];
	      _selectedCells[i][j] = _selectedCells[i][j-1];
	    }
	  _cells[i][column] = old;
	  _selectedCells[i][column] = NO;
	}
      if (_selectedCell && (_selectedColumn >= column))
	{
	  _selectedColumn++;
	}
      if (_dottedColumn >= column)
	{
	  _dottedColumn++;
	}
    }

  /*
   * Now put the new cells from the array into the matrix.
   */
  if (count > 0)
    {
      IMP getImp = [cellArray methodForSelector: getSel];

      for (i = 0; i < _numRows && i < count; i++)
	{
	  ASSIGN(_cells[i][column], (*getImp)(cellArray, getSel, i));
	}
    }

  if (_mode == NSRadioModeMatrix && _allowsEmptySelection == NO
    && _selectedCell == nil)
    {
      [self selectCellAtRow: 0 column: 0];
    }

  [self setNeedsDisplay: YES];
}

/**<p>Inserts a new row at index <var>row</var>.</p>
   <p>See Also: -insertRow:withCells: </p>
 */
- (void) insertRow: (NSInteger)row
{
  [self insertRow: row withCells: nil];
}

/**<p>Inserts a new row of cells ( specified by <var>cellArray</var>) 
   at the specified row <var>row</var>. This method can grows
   the matrix as necessay if needed</p>
   <p>See Also: -insertColumn:</p>
 */
- (void) insertRow: (NSInteger)row withCells: (NSArray*)cellArray
{
  NSInteger count = [cellArray count];
  NSInteger i = _numRows + 1;

  if (row < 0)
    {
      row = 0;
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"insert negative row (%d) in matrix", (int)row);
#else
      [NSException raise: NSRangeException
		  format: @"insert negative row (%d) in matrix", (int)row];
#endif
    }

  if ((cellArray != nil) && (count != _numCols))
    {
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"Wrong number of cells (%d) in row insert in matrix", (int)count);
#else
      [NSException raise: NSRangeException
		  format: @"Wrong number of cells (%d) in row insert in matrix", (int)count];
#endif
    }

  if (row >= i)
    {
      i = row + 1;
    }

  /*
   * Grow the matrix to have the new row.
   * MacOS-X docs say that if the matrix is empty, we make it have one
   * row and enough columns for all the elements.
   */
  if (count > 0 && (_numRows == 0 || _numCols == 0))
    {
      [self _renewRows: 1 columns: count rowSpace: count colSpace: 0];
    }
  else
    {
      [self _renewRows: i
	       columns: _numCols ? _numCols : 1
	      rowSpace: count
	      colSpace: 0];
    }

  /*
   * Rotate the newly created row to the insertion point if necessary.
   */
  if (_numRows != row)
    {
      id *oldr = _cells[_numRows - 1];
      BOOL *olds = _selectedCells[_numRows - 1];

      for (i = _numRows - 1; i > row; i--)
	{
	  _cells[i] = _cells[i-1];
	  _selectedCells[i] = _selectedCells[i-1];
	}
      _cells[row] = oldr;
      _selectedCells[row] = olds;
      if (_selectedCell && (_selectedRow >= row))
	_selectedRow++;

      if (_dottedRow != -1 && _dottedRow >= row)
	_dottedRow++;
    }

  /*
   * Put cells from the array into the matrix.
   */
  if (count > 0)
    {
      IMP getImp = [cellArray methodForSelector: getSel];

      for (i = 0; i < _numCols && i < count; i++)
	{
	  ASSIGN(_cells[row][i], (*getImp)(cellArray, getSel, i));
	}
    }

  if (_mode == NSRadioModeMatrix && !_allowsEmptySelection
    && _selectedCell == nil)
    {
      [self selectCellAtRow: 0 column: 0];
    }

  [self setNeedsDisplay: YES];
}

/**<p>Makes and returns new cell at row <var>row</var> and 
   column <var>column</var>.</p>
 */
- (NSCell*) makeCellAtRow: (NSInteger)row
		   column: (NSInteger)column
{
  NSCell *aCell;

  if (_cellPrototype != nil)
    {
      aCell = (*_cellNew)(_cellPrototype, copySel, _myZone);
    }
  else
    {
      aCell = (*_cellNew)(_cellClass, allocSel, _myZone);
      if (aCell != nil)
	{
	  aCell = (*_cellInit)(aCell, initSel);
	}
    }
  /*
   * This is only ever called when we are creating a new cell - so we know
   * we can simply assign a value into the matrix without releasing an old
   * value.  If someone uses this method directly (which the documentation
   * specifically says they shouldn't) they may produce a memory leak.
   */
  _cells[row][column] = aCell;
  return aCell;
}

/** <p>Returns the rectangle of the cell at row <var>row</var> and column
    <var>column</var></p>
 */
- (NSRect) cellFrameAtRow: (NSInteger)row
		   column: (NSInteger)column
{
  NSRect rect;

  rect.origin.x = column * (_cellSize.width + _intercell.width);
  rect.origin.y = row * (_cellSize.height + _intercell.height);
  rect.size = _cellSize;
  return rect;
}

/**<p>Gets the number of rows and columns of the NSMatrix</p>
   <p>See Also: -numberOfColumns -numberOfRows</p>
 */
- (void) getNumberOfRows: (NSInteger*)rowCount
		 columns: (NSInteger*)columnCount
{
  *rowCount = _numRows;
  *columnCount = _numCols;
}

/**<p>Replaces the NSMatrix's cell at row <var>row</var> and column <var>
   column</var> by <var>newCell</var> and mark for display the new cell.
   Raises a NSRangeException if the <var>row</var> or <var>column</var>
   are out of range.</p>
 */
- (void) putCell: (NSCell*)newCell
	   atRow: (NSInteger)row
	  column: (NSInteger)column
{
  if (row < 0 || row >= _numRows || column < 0 || column >= _numCols)
    {
      [NSException raise: NSRangeException
		  format: @"attempt to put cell outside matrix bounds"];
    }

  if ((row == _selectedRow) && (column == _selectedColumn) 
    && (_selectedCell != nil))
    {
      _selectedCell = newCell;
    }
  
  ASSIGN(_cells[row][column], newCell);

  [self setNeedsDisplayInRect: [self cellFrameAtRow: row column: column]];
}

/**<p>Removes the NSMatrix's column at index <var>column</var></p>
   <p>See Also: -removeRow:</p>
 */
- (void) removeColumn: (NSInteger)column
{
  if (column >= 0 && column < _numCols)
    {
      NSInteger i;

      for (i = 0; i < _maxRows; i++)
	{
	  NSInteger j;

	  AUTORELEASE(_cells[i][column]);
	  for (j = column + 1; j < _maxCols; j++)
	    {
	      _cells[i][j-1] = _cells[i][j];
	      _selectedCells[i][j-1] = _selectedCells[i][j];
	    }
	}
      _numCols--;
      _maxCols--;

      if (_maxCols == 0)
	{
	  _numRows = _maxRows = 0;
	}

      if (column == _selectedColumn)
	{
	  _selectedCell = nil;
	  [self selectCellAtRow: _selectedRow column: 0];
	}
      if (column == _dottedColumn)
	{
	  if (_numCols && [_cells[_dottedRow][0] acceptsFirstResponder])
	    _dottedColumn = 0;
	  else
	    _dottedRow = _dottedColumn = -1;
	}
    }
  else
    {
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"remove non-existent column (%d) from matrix", (int) column);
#else
      [NSException raise: NSRangeException
	format: @"remove non-existent column (%d) from matrix",  (int)column];
#endif
    }
}


/**<p>Removes the NSMatrix's row at index <var>row</var></p>
   <p>See Also: -removeColumn:</p>
 */
- (void) removeRow: (NSInteger)row
{
  if (row >= 0 && row < _numRows)
    {
      NSInteger i;

      for (i = 0; i < _maxCols; i++)
	{
	  AUTORELEASE(_cells[row][i]);
	}
      NSZoneFree(_myZone, _selectedCells[row]);
      NSZoneFree(_myZone, _cells[row]);
      for (i = row + 1; i < _maxRows; i++)
	{
	  _cells[i-1] = _cells[i];
	  _selectedCells[i-1] = _selectedCells[i];
	}
      _maxRows--;
      _numRows--;

      if (_maxRows == 0)
	{
	  _numCols = _maxCols = 0;
	}

      if (row == _selectedRow)
	{
	  _selectedCell = nil;
	  [self selectCellAtRow: 0 column: _selectedColumn];
	}
      if (row == _dottedRow)
	{
	  if (_numRows && [_cells[0][_dottedColumn] acceptsFirstResponder])
	    _dottedRow = 0;
	  else
	    _dottedRow = _dottedColumn = -1;
	}
    }
  else
    {
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"remove non-existent row (%d) from matrix", (int)row);
#else
      [NSException raise: NSRangeException
		  format: @"remove non-existent row (%d) from matrix", (int)row];
#endif
    }
}

- (void) renewRows: (NSInteger)newRows
	   columns: (NSInteger)newColumns
{
  [self _renewRows: newRows columns: newColumns rowSpace: 0 colSpace: 0];
}

- (void) setCellSize: (NSSize)aSize
{
  _cellSize = aSize;
  [self sizeToCells];
}

/** <p>Sets the space size between cells to aSize and resizes the matrix to
    fits the new cells spacing.</p>
    <p>See Also: -intercellSpacing -sizeToCells</p>
 */
- (void) setIntercellSpacing: (NSSize)aSize
{
  _intercell = aSize;
  [self sizeToCells];
}

- (void) sortUsingFunction: (NSComparisonResult (*)(id element1, id element2,
				   void *userData))comparator
		   context: (void*)context
{
  NSMutableArray *sorted;
  IMP add;
  IMP get;
  NSInteger i, j, index = 0;

  sorted = [NSMutableArray arrayWithCapacity: _numRows * _numCols];
  add = [sorted methodForSelector: @selector(addObject:)];
  get = [sorted methodForSelector: @selector(objectAtIndex:)];

  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  (*add)(sorted, @selector(addObject:), _cells[i][j]);
	}
    }

  [sorted sortUsingFunction: comparator context: context];

  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  _cells[i][j] = (*get)(sorted, @selector(objectAtIndex:), index++);
	}
    }
}

- (void) sortUsingSelector: (SEL)comparator
{
  NSMutableArray *sorted;
  IMP add;
  IMP get;
  NSInteger i, j, index = 0;

  sorted = [NSMutableArray arrayWithCapacity: _numRows * _numCols];
  add = [sorted methodForSelector: @selector(addObject:)];
  get = [sorted methodForSelector: @selector(objectAtIndex:)];

  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  (*add)(sorted, @selector(addObject:), _cells[i][j]);
	}
    }

  [sorted sortUsingSelector: comparator];

  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  _cells[i][j] = (*get)(sorted, @selector(objectAtIndex:), index++);
	}
    }
}

/** <p>Gets the row and the column of the NSMatrix correponding to the 
    specified NSPoint aPoint. Returns YES if aPoint is within the NSMatrix,
    NO otherwise</p>
 */ 
- (BOOL) getRow: (NSInteger*)row
	 column: (NSInteger*)column
       forPoint: (NSPoint)aPoint
{
  BOOL	betweenRows;
  BOOL	betweenCols;
  BOOL	beyondRows;
  BOOL	beyondCols;
  int	approxRow = aPoint.y / (_cellSize.height + _intercell.height);
  float	approxRowsHeight = approxRow * (_cellSize.height + _intercell.height);
  int	approxCol = aPoint.x / (_cellSize.width + _intercell.width);
  float	approxColsWidth = approxCol * (_cellSize.width + _intercell.width);

  /* First check the limit cases - is the point outside the matrix */
  beyondCols = (aPoint.x > _bounds.size.width  || aPoint.x < 0);
  beyondRows = (aPoint.y > _bounds.size.height || aPoint.y < 0);

  /* Determine if the point is inside a cell - note: if the point lies
     on the cell boundaries, we consider it inside the cell.  to be
     outside the cell (that is, in the intercell spacing) it must be
     completely in the intercell spacing - not on the border */
  /* The following is non zero if the point lies between rows (not inside 
     a cell) */
  betweenRows = (aPoint.y < approxRowsHeight
    || aPoint.y > approxRowsHeight + _cellSize.height);
  betweenCols = (aPoint.x < approxColsWidth
    || aPoint.x > approxColsWidth + _cellSize.width);

  if (beyondRows || betweenRows || beyondCols || betweenCols
    || (_numCols == 0) || (_numRows == 0))
    {
      if (row)
	{
	  *row = -1;
	}

      if (column)
	{
	  *column = -1;
	}
      
      return NO;
    }

  if (row)
    {
      if (approxRow < 0)
	{
	  approxRow = 0;
	}
      else if (approxRow >= _numRows)
	{
	  approxRow = _numRows - 1;
	}
      *row = approxRow;
    }

  if (column)
    {
      if (approxCol < 0)
	{
	  approxCol = 0;
	}
      else if (approxCol >= _numCols)
	{
	  approxCol = _numCols - 1;
	}
      *column = approxCol;
    }

  return YES;
}

/** <p>Gets the row and the column of the NSMatrix correponding to the 
    specified NSCell aCell. Returns YES if aCell is in the NSMatrix,
    NO otherwise</p>
 */ 
- (BOOL) getRow: (NSInteger*)row
	 column: (NSInteger*)column
	 ofCell: (NSCell*)aCell
{
  NSInteger i;

  for (i = 0; i < _numRows; i++)
    {
      NSInteger j;

      for (j = 0; j < _numCols; j++)
	{
	  if (_cells[i][j] == aCell)
	    {
	      if (row)
		*row = i;
	      if (column)
		*column = j;
	      return YES;
	    }
	}
    }

  if (row)
    *row = -1;
  if (column)
    *column = -1;

  return NO;
}

/** <p>Sets the state of the cell at row <var>row</var> and <var>column</var>
    to value. If the NSMatrix's mode is NSRadioModeMatrix it deselects
    the cell currently selected if needed.</p>
 */
- (void) setState: (NSInteger)value
	    atRow: (NSInteger)row
	   column: (NSInteger)column
{
  NSCell *aCell = [self cellAtRow: row column: column];

  if (!aCell)
    {
      return;
    }

  if (_mode == NSRadioModeMatrix)
    {
      if (value)
	{
	  if (_selectedRow > -1 && _selectedColumn > -1)
	    {
	      _selectedCells[_selectedRow][_selectedColumn] = NO;
	      [_selectedCell setState: NSOffState];
	      [self setNeedsDisplayInRect:
		     [self cellFrameAtRow: _selectedRow
				   column: _selectedColumn]];
	    }

	  _selectedCell = aCell;
	  _selectedRow = row;
	  _selectedColumn = column;

	  [_selectedCell setState: value];
	  _selectedCells[row][column] = YES;

          [self _setKeyRow: row column: column];
	}
      else if (_allowsEmptySelection)
	{
	  [self deselectSelectedCell];
	}
    }
  else
    {
      [aCell setState: value];
    }
  [self setNeedsDisplayInRect: [self cellFrameAtRow: row column: column]];
}

/**<p>Deselects all NSMatrix's cells. Does nothing if the NSMatrix's mode
   is NSRadioModeMatrix and if it does not allows empty selection.
   Except for the case, when there are no cells left at all. Then the 
   selection is always cleared.</p>
   <p>See Also: -mode -allowsEmptySelection -setNeedsDisplayInRect:</p>
 */
- (void) deselectAllCells
{
  NSInteger i;

  if (_numRows > 0 && _numCols > 0 &&
      !_allowsEmptySelection && _mode == NSRadioModeMatrix)
    {
      return;
    }

  for (i = 0; i < _numRows; i++)
    {
      NSInteger j;

      for (j = 0; j < _numCols; j++)
	{
	  if (_selectedCells[i][j])
	    {
	      NSCell *aCell = _cells[i][j];
	      BOOL isHighlighted = [aCell isHighlighted];

	      _selectedCells[i][j] = NO;

	      if ([aCell state] || isHighlighted)
		{
		  [aCell setState: NSOffState];

		  if (isHighlighted)
		    {
		      [aCell setHighlighted: NO];
		    }
		  [self setNeedsDisplayInRect: [self cellFrameAtRow: i
						     column: j]];
		}
	    }
	}
    }
  _selectedCell = nil;
  _selectedRow = -1;
  _selectedColumn = -1;
}

/**<p>Deselects the selected cell.Does nothing if the NSMatrix's mode
   is NSRadioModeMatrix and if it does not allows empty selection</p>
 */
- (void) deselectSelectedCell
{
  NSInteger i,j;

  if (!_selectedCell
    || (!_allowsEmptySelection && (_mode == NSRadioModeMatrix)))
    return;

  /*
   * For safety (as in macosx)
   */
  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  if (_selectedCells[i][j])
	    {
	      [_cells[i][j] setState: NSOffState];
	      _selectedCells[i][j] = NO;
	    }
	}
    }

  _selectedCell = nil;
  _selectedRow = -1;
  _selectedColumn = -1;
}

/**<p>Selects all the cells and marks self for display. Does nothing if the
   NSMatrix's mode is NSRadioModeMatrix</p><p>See Also:
   -selectCellAtRow:column: -selectCell:</p>   
 */
- (void) selectAll: (id)sender
{
  NSInteger i, j;

  /* Can't select all if only one can be selected.  */
  if (_mode == NSRadioModeMatrix)
    {
      return;
    }

  _selectedCell = nil;
  _selectedRow = -1;
  _selectedColumn = -1;

  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  if ([_cells[i][j] isEnabled] == YES
	    && [_cells[i][j] isEditable] == NO)
	    {
	      _selectedCell = _cells[i][j];
	      [_selectedCell setState: NSOnState];
	      _selectedCells[i][j] = YES;

	      _selectedRow = i;
	      _selectedColumn = j;
	    }
	  else
	    {
	      _selectedCells[i][j] = NO;
	      [_cells[i][j] setShowsFirstResponder: NO];
	    }
	}
    }

  [self setNeedsDisplay: YES];
}

- (void) _selectCell: (NSCell *)aCell atRow: (NSInteger)row column: (NSInteger)column
{
  if (aCell)
    {
      NSRect cellFrame;

      if (_selectedCell && _selectedCell != aCell)
	{
          if (_mode == NSRadioModeMatrix && _selectedRow > -1 && _selectedColumn > -1)
            {
              _selectedCells[_selectedRow][_selectedColumn] = NO;
              [_selectedCell setState: NSOffState];
            }
	  [self setNeedsDisplayInRect: [self cellFrameAtRow: _selectedRow
					     column: _selectedColumn]];
	}

      _selectedCell = aCell;
      _selectedRow = row;
      _selectedColumn = column;
      _selectedCells[row][column] = YES;

      if (_mode == NSListModeMatrix || _mode == NSRadioModeMatrix)
	{
	  [_selectedCell setState: NSOnState];
	}
      else
	{
	  [_selectedCell setNextState];
	}

      if (_mode == NSListModeMatrix)
	[aCell setHighlighted: YES];

      cellFrame = [self cellFrameAtRow: row column: column];
      if (_autoscroll)
	[self scrollRectToVisible: cellFrame];

      [self setNeedsDisplayInRect: cellFrame];

      [self _setKeyRow: row column: column];
    }
  else
    {
      _selectedCell = nil;
      _selectedRow = _selectedColumn = -1;
    }
}

- (void) selectCell: (NSCell *)aCell
{
  NSInteger row, column;

  if ([self getRow: &row column: &column ofCell: aCell] == YES)
    {
      [self _selectCell: aCell atRow: row column: column];

      // Note: we select the cell iff it is 'selectable', not 'editable' 
      // as macosx says.  This looks definitely more appropriate. 
      // [This is going to start editing only if the cell is also editable,
      // otherwise the text gets selected and that's all.]
      [self selectTextAtRow: row column: column];
    }
}

/** <p>Selects the cell and the text inside at row <var>row</var> 
    and column <var>column</var>. If row or column is -1 it deselects all
    the cells.</p>
    <p>See Also: -deselectSelectedCell -selectTextAtRow:column:</p>    
 */
- (void) selectCellAtRow: (NSInteger)row column: (NSInteger)column
{
  NSCell *aCell;

  if ((row == -1) || (column == -1))
    {
      [self deselectAllCells];
      return;
    }

  aCell = [self cellAtRow: row column: column];

  if (aCell)
    {
      [self _selectCell: aCell atRow: row column: column];
      [self selectTextAtRow: row column: column];
    }
}

/**<p>Selects the cell (and the text inside) with tag <var>anInt</var>.
   Return YES if the NSMatrix contains a cell with tag <var>anInt</var>,
   NO otherwise.</p><p>See Also: -deselectSelectedCell
   -selectTextAtRow:column:</p>
 */
- (BOOL) selectCellWithTag: (NSInteger)anInt
{
  id aCell;
  NSInteger i = _numRows;

  while (i-- > 0)
    {
      NSInteger j = _numCols;

      while (j-- > 0)
	{
	  aCell = _cells[i][j];
	  if ([aCell tag] == anInt)
	    {
	      [self _selectCell: aCell atRow: i column: j];
	      [self selectTextAtRow: i column: j];
	      return YES;
	    }
	}
    }
  return NO;
}

/**<p>Returns an array of the selected cells</p>
 */
- (NSArray*) selectedCells
{
  NSMutableArray *array = [NSMutableArray array];
  NSInteger i;

  for (i = 0; i < _numRows; i++)
    {
      NSInteger j;

      for (j = 0; j < _numCols; j++)
	{
	  if (_selectedCells[i][j] == YES)
	    {
	      [array addObject: _cells[i][j]];
	    }
	}
    }
  return array;
}

- (void) setSelectionFrom: (NSInteger)startPos
		       to: (NSInteger)endPos
		   anchor: (NSInteger)anchorPos
	        highlight: (BOOL)flag
{
  /* Cells are selected from the anchor (A) to the point where the mouse
   * went down (S) and then they are selected (if the mouse moves away from A)
   * or deselected (if the mouse moves closer to A) until the point 
   * where the mouse goes up (E).
   * This is inverted if flag is false (not sure about this though; if this is
   * changed, mouse tracking in list mode should be changed too).
   */
  /* An easy way of doing this is unselecting all cells from A to S and then
   * selecting all cells from A to E. Let's try to do it in a more optimized
   * way..
   */
  /* Linear and rectangular selections are a bit different */
  if (![self isSelectionByRect]
      || [self numberOfRows] == 1 || [self numberOfColumns] == 1)
    {
      /* Linear selection
       * There are three possibilities (ignoring direction):
       *    A    S    E 
       *    sssssssssss
       *
       *    A    E    S
       *    ssssssuuuuu
       *
       *    E    A    S
       *    ssssssuuuuu
       *
       * So, cells from A to E are selected and, if S is outside the
       * range from A to E, cells from S to its closest point are unselected
       */
      NSInteger selStart = MIN(anchorPos, endPos);
      NSInteger selEnd = MAX(anchorPos, endPos);
      [self _setState: flag ? NSOnState : NSOffState
	    highlight: flag
	    startIndex: selStart
	    endIndex: selEnd];
      if (startPos > selEnd)
        {
          [self _setState: flag ? NSOffState : NSOnState
	        highlight: !flag
	        startIndex: selEnd+1
	        endIndex: startPos];
        }
      else if (startPos < selStart)
        {
          [self _setState: flag ? NSOffState : NSOnState
	        highlight: !flag
	        startIndex: startPos
	        endIndex: selStart-1];
        }
    }
  else
    {
      /* Rectangular selection
       *
       *   A     sss
       *    S    sss
       *     E   sss
       *
       *   A     ssu
       *    E    ssu
       *     S   uuu
       *
       *   E     ss 
       *    A    ssu
       *     S    uu
       *
       *   A     ssu
       *     S   ssu
       *    E    ss
       *
       * So, cells of the rect from A to E are selected and cells of the
       * rect from A to S that are outside the first rect are unselected
       */
      MPoint anchorPoint = POINT_FROM_INDEX(anchorPos);
      MPoint endPoint = POINT_FROM_INDEX(endPos);
      MPoint startPoint = POINT_FROM_INDEX(startPos);
      NSInteger minx_AE = MIN(anchorPoint.x, endPoint.x);
      NSInteger miny_AE = MIN(anchorPoint.y, endPoint.y);
      NSInteger maxx_AE = MAX(anchorPoint.x, endPoint.x);
      NSInteger maxy_AE = MAX(anchorPoint.y, endPoint.y);
      NSInteger minx_AS = MIN(anchorPoint.x, startPoint.x);
      NSInteger miny_AS = MIN(anchorPoint.y, startPoint.y);
      NSInteger maxx_AS = MAX(anchorPoint.x, startPoint.x);
      NSInteger maxy_AS = MAX(anchorPoint.y, startPoint.y);
      
      [self _setState: flag ? NSOnState : NSOffState
	    highlight: flag
	    startIndex: INDEX_FROM_COORDS(minx_AE, miny_AE)
	    endIndex: INDEX_FROM_COORDS(maxx_AE, maxy_AE)];
      if (startPoint.x > maxx_AE)
        {
          [self _setState: flag ? NSOffState : NSOnState
	        highlight: !flag
	        startIndex: INDEX_FROM_COORDS(maxx_AE+1, miny_AS)
	        endIndex: INDEX_FROM_COORDS(startPoint.x, maxy_AS)];
        }
      else if (startPoint.x < minx_AE)
        {
          [self _setState: flag ? NSOffState : NSOnState
	        highlight: !flag
	        startIndex: INDEX_FROM_COORDS(startPoint.x, miny_AS)
	        endIndex: INDEX_FROM_COORDS(minx_AE-1, maxy_AS)];
        }
      if (startPoint.y > maxy_AE)
        {
          [self _setState: flag ? NSOffState : NSOnState
	        highlight: !flag
	        startIndex: INDEX_FROM_COORDS(minx_AS, maxy_AE+1)
	        endIndex: INDEX_FROM_COORDS(maxx_AS, startPoint.y)];
        }
      else if (startPoint.y < miny_AE)
        {
          [self _setState: flag ? NSOffState : NSOnState
	        highlight: !flag
	        startIndex: INDEX_FROM_COORDS(minx_AS, startPoint.y)
	        endIndex: INDEX_FROM_COORDS(maxx_AS, miny_AE-1)];
        }
    }

  /*
  Update the _selectedCell and related ivars. This could be optimized a lot
  in many cases, but the full search cannot be avoided in the general case,
  and being correct comes first.
  */
  {
    NSInteger i, j;
    for (i = _numRows - 1; i >= 0; i--)
      {
	for (j = _numCols - 1; j >= 0; j--)
	  {
	    if (_selectedCells[i][j])
	      {
		_selectedCell = _cells[i][j];
		_selectedRow = i;
		_selectedColumn = j;
		return;
	      }
	  }
      }
    _selectedCell = nil;
    _selectedColumn = -1;
    _selectedRow = -1;
  }
}

/**<p>Returns the cell at row <var>row</var> and column <var>column</var>
   Returns nil if the <var>row</var> or <var>column</var> are out of
   range</p>    
 */
- (id) cellAtRow: (NSInteger)row
	  column: (NSInteger)column
{
  if (row < 0 || row >= _numRows || column < 0 || column >= _numCols)
    return nil;
  return _cells[row][column];
}

/**<p>Returns the cell with tag <var>anInt</var>
   Returns nil if no cell has a tag <var>anInt</var></p>    
 */
- (id) cellWithTag: (NSInteger)anInt
{
  NSInteger i = _numRows;

  while (i-- > 0)
    {
      NSInteger j = _numCols;

      while (j-- > 0)
	{
	  id aCell = _cells[i][j];

	  if ([aCell tag] == anInt)
	    {
	      return aCell;
	    }
	}
    }
  return nil;
}

/** <p>Returns an array of the NSMatrix's cells</p>
 */
- (NSArray*) cells
{
  NSMutableArray *c;
  IMP add;
  NSInteger i;

  c = [NSMutableArray arrayWithCapacity: _numRows * _numCols];
  add = [c methodForSelector: @selector(addObject:)];
  for (i = 0; i < _numRows; i++)
    {
      NSInteger j;

      for (j = 0; j < _numCols; j++)
	{
	  (*add)(c, @selector(addObject:), _cells[i][j]);
	}
    }
  return c;
}

- (void) selectText: (id)sender
{
  // Attention, we are *not* doing what MacOS-X does.
  // But they are *not* doing what the OpenStep specification says.
  // This is a compromise -- and fully OpenStep compliant.
  NSSelectionDirection s =  NSDirectSelection;

  if (_window)
    s = [_window keyViewSelectionDirection];

  switch (s)
    {
      // _window selecting backwards
    case NSSelectingPrevious:
      [self _selectPreviousSelectableCellBeforeRow: _numRows
	    column: _numCols];
      break;
      // _Window selecting forward
    case NSSelectingNext:
      [self _selectNextSelectableCellAfterRow: -1
	    column: -1];
      break;
    case NSDirectSelection:
      // Someone else -- we have some freedom here
      if ([_selectedCell isSelectable])
	{
	  [self selectTextAtRow: _selectedRow
			 column: _selectedColumn];
	}
      else
	{
	  if (_dottedRow != -1)
	    {
              [self selectTextAtRow: _dottedRow  column: _dottedColumn];
	    }
	}
      break;
    }
}

/**<p>Select the text of the cell at row <var>row</var> and column 
   <var>column</var>. The cell is selected if and only if the cell 
   is selectable ( MacOSX select it if the cell is editable ). This 
   methods returns the selected cell if exists and selectable,
   nil otherwise</p>   
 */
- (id) selectTextAtRow: (NSInteger)row column: (NSInteger)column
{
  if (row < 0 || row >= _numRows || column < 0 || column >= _numCols)
    return self;

  // macosx doesn't select the cell if it isn't 'editable'; instead,
  // we select the cell if and only if it is 'selectable', which looks
  // more appropriate.  This is going to start editing if and only if
  // the cell is also 'editable'.
  if ([_cells[row][column] isSelectable] == NO)
    {
      return nil;
    }

  if (_textObject)
    {
      if (_selectedCell == _cells[row][column])
	{
	  [_textObject selectAll: self];
	  return _selectedCell;
	}
      else
	{
	  [self validateEditing];
	  [self abortEditing];
	}
    }

  // Now _textObject == nil
  {
    NSText *text = [_window fieldEditor: YES
			forObject: self];
    NSUInteger length;

    if (([text superview] != nil) && ([text resignFirstResponder] == NO))
      {
	return nil;
      }

    [self _selectCell: _cells[row][column] atRow: row column: column];

    /* See comment in NSTextField */
    length = [[_selectedCell stringValue] length];
    _textObject = [_selectedCell setUpFieldEditorAttributes: text];
    [_selectedCell selectWithFrame: [self cellFrameAtRow: _selectedRow
					  column: _selectedColumn]
		   inView: self
		   editor: _textObject
		   delegate: self
		   start: 0
		   length: length];
    return _selectedCell;
  }
}

- (id) keyCell
{
  if (_dottedRow == -1 || _dottedColumn == -1)
    {
      return nil;
    }
  else if (_cells != 0)
    {
      return _cells[_dottedRow][_dottedColumn];
    }

  return nil;
}

- (void) setKeyCell: (NSCell *)aCell 
{
  BOOL isValid;
  NSInteger row, column;

  isValid = [self getRow: &row  column: &column  ofCell: aCell];

  if (isValid == YES)
    {
      [self _setKeyRow: row column: column];
    }
}

/**<p>Returns the next key view</p>
   <p>See Also: -setNextText: [NSView-nextKeyView]</p>
 */
- (id) nextText
{
  return [self nextKeyView];
}

/**<p>Returns the previous key view</p>
   <p>See Also: -setPreviousText: [NSView-previousKeyView]</p>
 */
- (id) previousText
{
  return [self previousKeyView];
}

/**<p>Invokes when the text cell starts to be editing.This methods posts 
   a NSControlTextDidBeginEditingNotification with a dictionary containing 
   the NSFieldEditor as user info </p><p>See Also:
   [NSNotificationCenter-postNotificationName:object:userInfo:]</p>
*/
- (void) textDidBeginEditing: (NSNotification *)aNotification
{
  [super textDidBeginEditing: aNotification];
}

/**<p>Invokes when the text cell is changed. This methods posts a 
   NSControlTextDidChangeNotification with a dictionary containing the
   NSFieldEditor as user info </p><p>See Also: 
   [NSNotificationCenter-postNotificationName:object:userInfo:]</p>
*/
- (void) textDidChange: (NSNotification *)aNotification
{
  NSFormatter *formatter;

  // MacOS-X asks us to inform the cell if possible.
  if ((_selectedCell != nil) && [_selectedCell respondsToSelector: 
						 @selector(textDidChange:)])
    {
      [_selectedCell textDidChange: aNotification];
    }

  [super textDidChange: aNotification];

  formatter = [_selectedCell formatter];
  if (formatter != nil)
    {
      /*
       * FIXME: This part needs heavy interaction with the yet to finish 
       * text system.
       *
       */
      NSString *partialString;
      NSString *newString = nil;
      NSString *error = nil;
      BOOL wasAccepted;
      
      partialString = [_textObject string];
      wasAccepted = [formatter isPartialStringValid: partialString 
			       newEditingString: &newString 
			       errorDescription: &error];

      if (wasAccepted == NO)
	{
	  SEL sel = @selector(control:didFailToValidatePartialString:errorDescription:);
	  
	  if ([_delegate respondsToSelector: sel])
	    {
	      [_delegate control: self 
			 didFailToValidatePartialString: partialString 
			 errorDescription: error];
	    }
	}

      if (newString != nil)
	{
	  NSLog (@"Unimplemented: should set string to %@", newString);
	  // FIXME ! This would reset editing !
	  //[_textObject setString: newString];
	}
      else
	{
	  if (wasAccepted == NO)
	    {
	      // FIXME: Need to delete last typed character (?!)
	      NSLog (@"Unimplemented: should delete last typed character");
	    }
	}
      
    }
}

/**<p>Invokes when the text cell is changed.
   This methods posts a NSControlTextDidEndEditingNotification
   a dictionary containing the NSFieldEditor as user info </p><p>See Also:
   [NSNotificationCenter-postNotificationName:object:userInfo:]</p>
*/
- (void) textDidEndEditing: (NSNotification *)aNotification
{
  id textMovement;

  [super textDidEndEditing: aNotification];

  textMovement = [[aNotification userInfo] objectForKey: @"NSTextMovement"];
  if (textMovement)
    {
      switch ([(NSNumber *)textMovement intValue])
	{
	case NSReturnTextMovement:
	  if ([self sendAction] == NO)
	    {
	      NSEvent *event = [_window currentEvent];

	      if ([self performKeyEquivalent: event] == NO
		  && [_window performKeyEquivalent: event] == NO)
		[self selectText: self];
	    }
	  break;
	case NSTabTextMovement:
          if ([_selectedCell sendsActionOnEndEditing])
            [self sendAction];

	  if (_tabKeyTraversesCells)
	    {
	      if ([self _selectNextSelectableCellAfterRow: _selectedRow
		       column: _selectedColumn])
		break;
	    }
	  [_window selectKeyViewFollowingView: self];

	  if ([_window firstResponder] == _window)
	    {
	      if (_tabKeyTraversesCells)
		{
		  if ([self _selectNextSelectableCellAfterRow: -1
			   column: -1])
		    break;
		}
	      [self selectText: self];
	    }
	  break;
	case NSBacktabTextMovement:
          if ([_selectedCell sendsActionOnEndEditing])
            [self sendAction];

	  if (_tabKeyTraversesCells)
	    {
	      if ([self _selectPreviousSelectableCellBeforeRow: _selectedRow
		       column: _selectedColumn])
		break;
	    }
	  [_window selectKeyViewPrecedingView: self];

	  if ([_window firstResponder] == _window)
	    {
	      if (_tabKeyTraversesCells)
		{
		  if ([self _selectPreviousSelectableCellBeforeRow: _numRows
			   column: _numCols])
		    break;
		}
	      [self selectText: self];
	    }
	  break;
	}
    }
}

/**<p>Asks to the delegate (if it implements -control:textShouldBeginEditing: )
   if the text should be edit. Returns YES if the delegate does not implement
   this method</p>   
 */
- (BOOL) textShouldBeginEditing: (NSText*)aTextObject
{
  if (_delegate && [_delegate respondsToSelector:
    @selector(control:textShouldBeginEditing:)])
    {
      return [_delegate control: self
	 textShouldBeginEditing: aTextObject];
    }
  return YES;
}

- (BOOL) textShouldEndEditing: (NSText *)aTextObject
{
  if ([_selectedCell isEntryAcceptable: [aTextObject text]] == NO)
    {
      [self sendAction: _errorAction to: _target];
      return NO;
    }

  if ([_delegate respondsToSelector:
		   @selector(control:textShouldEndEditing:)])
    {
      if ([_delegate control: self
		     textShouldEndEditing: aTextObject] == NO)
	{
	  NSBeep ();
	  return NO;
	}
    }

  if ([_delegate respondsToSelector: 
		   @selector(control:isValidObject:)] == YES)
    {
      NSFormatter *formatter;
      id newObjectValue;
      
      formatter = [_selectedCell formatter];
      
      if ([formatter getObjectValue: &newObjectValue 
		     forString: [_textObject text] 
		     errorDescription: NULL] == YES)
	{
	  if ([_delegate control: self
			 isValidObject: newObjectValue] == NO)
            {
              return NO;
            }
	}
    }

  // In all other cases
  return YES;
}

- (BOOL) tabKeyTraversesCells
{
  return _tabKeyTraversesCells;
}

- (void) setTabKeyTraversesCells: (BOOL)flag
{
  _tabKeyTraversesCells = flag;
}

/**<p>Sets the next key view to <var>anObject</var></p>
   <p>See Also: -nextText [NSView-setNextKeyView:</p>
 */
- (void) setNextText: (id)anObject
{
  [self setNextKeyView: anObject];
}

/**<p>Sets the previous key view to <var>anObject</var></p>
   <p>See Also: -previousText [NSView-setPreviousKeyView:</p>
 */
- (void) setPreviousText: (id)anObject
{
  [self setPreviousKeyView: anObject];
}

- (void) setValidateSize: (BOOL)flag
{
  // TODO
}

- (void) sizeToCells
{
  NSSize newSize;
  NSInteger nc = _numCols;
  NSInteger nr = _numRows;

  if (!nc)
    nc = 1;
  if (!nr)
    nr = 1;
  newSize.width = nc * (_cellSize.width + _intercell.width) - _intercell.width;
  newSize.height = nr * (_cellSize.height + _intercell.height) - _intercell.height;
  [super setFrameSize: newSize];
}
  
- (void) sizeToFit
{
  /*
   * A simple explanation of the logic behind this method.
   *
   * Example of when you would like to use this method:
   * you have a matrix containing radio buttons.  Say that you have the 
   * following radio buttons - 
   *
   * * First option
   * * Second option
   * * Third option
   * * No thanks, no option for me
   *
   * this method should size the matrix so that it can comfortably
   * show all the cells it contains.  To do it, we must consider that
   * all the cells should be given the same size, yet some cells need
   * more space than the others to show their contents, so we need to
   * choose the cell size as to be enough to display every cell.  We
   * loop on all cells, call cellSize on each (which returns the
   * *minimum* comfortable size to display that cell), and choose a
   * final cellSize which is enough big to be bigger than all these
   * cellSizes.  We resize the matrix to have that cellSize, and
   * that's it.  */
  NSSize newSize = NSZeroSize;
  NSInteger i, j;

  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  NSSize tempSize = [_cells[i][j] cellSize];
	  tempSize.height = ceil(tempSize.height);
	  tempSize.width = ceil(tempSize.width);
	  if (tempSize.width > newSize.width)
	    {
	      newSize.width = tempSize.width;
	    }
	  if (tempSize.height > newSize.height)
	    {
	      newSize.height = tempSize.height;
	    }
	}
    }
  
  [self setCellSize: newSize];
}

/**<p>Scrolls the NSMatrix to make the cell at row <var>row</var> and column
   <var>column</var> visible</p>
   <p>See Also: -scrollRectToVisible: -cellFrameAtRow:column:</p>
 */
- (void) scrollCellToVisibleAtRow: (NSInteger)row
			   column: (NSInteger)column
{
  [self scrollRectToVisible: [self cellFrameAtRow: row column: column]];
}

- (void) setAutoscroll: (BOOL)flag
{
  _autoscroll = flag;
}

- (void) setScrollable: (BOOL)flag
{
  NSInteger i;

  for (i = 0; i < _numRows; i++)
    {
      NSInteger j;

      for (j = 0; j < _numCols; j++)
	{
	  [_cells[i][j] setScrollable: flag];
	}
    }
  [_cellPrototype setScrollable: flag];
}

- (void) drawRect: (NSRect)rect
{
  NSInteger i, j;
  NSInteger row1, col1;	// The cell at the upper left corner
  NSInteger row2, col2;	// The cell at the lower right corner

  if (_drawsBackground)
    {
      [_backgroundColor set];
      NSRectFill(rect);
    }

  if (!_numRows || !_numCols)
    return;

  row1 = rect.origin.y / (_cellSize.height + _intercell.height);
  col1 = rect.origin.x / (_cellSize.width + _intercell.width);
  row2 = NSMaxY(rect) / (_cellSize.height + _intercell.height);
  col2 = NSMaxX(rect) / (_cellSize.width + _intercell.width);

  if (row1 < 0)
    {
      row1 = 0;
    }
  else if (row1 >= _numRows)
    {
      row1 = _numRows - 1;
    }

  if (col1 < 0)
    {
      col1 = 0;
    }
  else if (col1 >= _numCols)
    {
      col1 = _numCols - 1;
    }

  if (row2 < 0)
    {
      row2 = 0;
    }
  else if (row2 >= _numRows)
    {
      row2 = _numRows - 1;
    }

  if (col2 < 0)
    {
      col2 = 0;
    }
  else if (col2 >= _numCols)
    {
      col2 = _numCols - 1;
    }

  /* Draw the cells within the drawing rectangle. */
  for (i = row1; i <= row2 && i < _numRows; i++)
    {
      for (j = col1; j <= col2 && j < _numCols; j++)
        {
          [self drawCellAtRow: i column: j];
        }
    }
}

- (BOOL) isOpaque
{
  return _drawsBackground;
}

- (void) drawCell: (NSCell *)aCell
{
  NSInteger row, column;

  if ([self getRow: &row  column: &column  ofCell: aCell] == YES)
    {
      [self drawCellAtRow: row  column: column];
    }
}

/**<p>Draws the cell at row <var>row</var> and column <var>column</var></p>
   <p>See Also: [NSCell-drawWithFrame:inView:] -setDrawsCellBackground:
   -drawsCellBackground</p>
 */
- (void) drawCellAtRow: (NSInteger)row column: (NSInteger)column
{
  NSCell *aCell = [self cellAtRow: row column: column];

  if (aCell)
    {
      NSRect cellFrame = [self cellFrameAtRow: row column: column];

     if (_drawsCellBackground)
        {
          [_cellBackgroundColor set];
          NSRectFill(cellFrame);
        }
      
      if (_dottedRow == row
          && _dottedColumn == column
          && [aCell acceptsFirstResponder]
          && [_window isKeyWindow]
          && [_window firstResponder] == self)
        {
          [aCell setShowsFirstResponder: YES];
          [aCell drawWithFrame: cellFrame inView: self];
          [aCell setShowsFirstResponder: NO];
        }
      else
        {
          [aCell setShowsFirstResponder: NO];
          [aCell drawWithFrame: cellFrame inView: self];
        }
    }
}

/** <p>(Un)Highlights the cell (if exists ) at row at row <var>row</var> 
    and column <var>column</var>. and maks the cell rect for display.</p>
    <p>See Also: -setNeedsDisplayInRect: [NSCell-setHighlighted:]</p>
 */
- (void) highlightCell: (BOOL)flag atRow: (NSInteger)row column: (NSInteger)column
{
  NSCell *aCell = [self cellAtRow: row column: column];

  if (aCell)
    {
      [aCell setHighlighted: flag];
      [self setNeedsDisplayInRect: [self cellFrameAtRow: row column: column]];
    }
}

/**<p>Sends the cell action, if a NSMatrix's cell is selected 
   and enabled, sends the NSMatrix action otherwise. Returns YES if
   the action is succesfully sent. NO if a cell is selected but not enabled
   or if an action can not be sent.</p>
   <p>See Also: -sendAction:to: -selectedCell</p>   
 */
- (BOOL) sendAction
{
  if (_selectedCell)
    {
      if ([_selectedCell isEnabled] == NO)
        {
          return NO;
        }

      return [self sendAction: [_selectedCell action] 
		   to:         [_selectedCell target]]; 
    }

  // _selectedCell == nil
  return [super sendAction: _action to: _target];
}

- (BOOL) sendAction: (SEL)theAction
		 to: (id)theTarget
{
  if (theAction)
    {
      if (theTarget)
	{
	  return [super sendAction: theAction to: theTarget];
	}
      else
	{
	  return [super sendAction: theAction to: _target];
	}
    }
  else
    {
      return [super sendAction: _action to: _target];
    }
}

- (void) sendAction: (SEL)aSelector
		 to: (id)anObject
        forAllCells: (BOOL)flag
{
  NSInteger i;

  if (flag)
    {
      for (i = 0; i < _numRows; i++)
	{
	  NSInteger j;

	  for (j = 0; j < _numCols; j++)
	    {
	      if (![anObject performSelector: aSelector
				  withObject: _cells[i][j]])
		{
		  return;
		}
	    }
	}
    }
  else
    {
      for (i = 0; i < _numRows; i++)
	{
	  NSInteger j;

	  for (j = 0; j < _numCols; j++)
	    {
	      if (_selectedCells[i][j])
		{
		  if (![anObject performSelector: aSelector
				      withObject: _cells[i][j]])
		    {
		      return;
		    }
		}
	    }
	}
    }
}

/** 
 */
- (void) sendDoubleAction
{
  if ([_selectedCell isEnabled] == NO)
    return;

  if (_doubleAction)
    [self sendAction: _doubleAction to: _target];
  else
    [self sendAction];
}

/**<p>Returns NO if the NSMatrix's mode is <ref type="type" id="NSMatrixMode">
   NSListModeMatrix</ref>,  YES otherwise.</p>
   <p>See Also: -setMode: -mode</p>
 */
- (BOOL) acceptsFirstMouse: (NSEvent*)theEvent
{
  if (_mode == NSListModeMatrix)
    return NO;
  else
    return YES;
}

- (void) _mouseDownNonListMode: (NSEvent *)theEvent
{
  BOOL mouseUpInCell = NO, onCell, scrolling = NO, mouseUp = NO;
  NSCell *mouseCell;
  NSInteger mouseRow;
  NSInteger mouseColumn;
  NSPoint mouseLocation;
  NSRect mouseCellFrame;
  NSCell *originallySelectedCell = _selectedCell;
  NSUInteger eventMask = NSLeftMouseUpMask | NSLeftMouseDownMask
                     | NSMouseMovedMask  | NSLeftMouseDraggedMask;

  while (!mouseUp)
    {
      mouseLocation = [self convertPoint: [theEvent locationInWindow]
                                fromView: nil];

      onCell = [self getRow: &mouseRow
		     column: &mouseColumn
		     forPoint: mouseLocation];

      if (onCell)
	{
	  mouseCellFrame = [self cellFrameAtRow: mouseRow column: mouseColumn];
	  mouseCell = [self cellAtRow: mouseRow column: mouseColumn];

	  if (_autoscroll)
            {
	      scrolling = [self scrollRectToVisible: mouseCellFrame];
            }

	  if ([mouseCell isEnabled])
	    {	      
              int old_state;
              
              /* Select the cell before tracking. The cell can send its action
               * during tracking, and the target discovers which cell was
               * clicked calling selectedCell.
               * The cell calls -nextState before sending the action, so its
               * state should not be changed here (except in radio mode).
               */
              old_state = [mouseCell state];
              [self _selectCell: mouseCell atRow: mouseRow column: mouseColumn];
              if (_mode == NSRadioModeMatrix && !_allowsEmptySelection)
                {
                  [mouseCell setState: NSOffState];
                }
              else
                {
                  [mouseCell setState: old_state];
                }
              
	      if (_mode != NSTrackModeMatrix)
		{
		  [self highlightCell: YES
			atRow: mouseRow
			column: mouseColumn];
		}

	      mouseUpInCell = [mouseCell trackMouse: theEvent
                                         inRect: mouseCellFrame
                                         ofView: self
					 untilMouseUp:
					   [[mouseCell class]
					     prefersTrackingUntilMouseUp]];

	      if (_mode != NSTrackModeMatrix)
		{
		  [self highlightCell: NO
			atRow: mouseRow
			column: mouseColumn];
		}
              else
                {
                  if ([mouseCell state] != old_state)
                    {
                      [self setNeedsDisplayInRect: mouseCellFrame];
                    }
                }
                
	      mouseUp = mouseUpInCell
                      || ([[NSApp currentEvent] type] == NSLeftMouseUp);

	      if (!mouseUpInCell)
		{
	          _selectedCells[_selectedRow][_selectedColumn] = NO;
	          _selectedCell = nil;
	          _selectedRow = _selectedColumn = -1;
		}
	    }
	}

      // if mouse didn't go up, take next event
      if (!mouseUp)
	{
          NSEvent *newEvent;
	  newEvent = [NSApp nextEventMatchingMask: eventMask
			    untilDate: !scrolling
			    ? [NSDate distantFuture]
			    : [NSDate dateWithTimeIntervalSinceNow: 0.05]
			    inMode: NSEventTrackingRunLoopMode
			    dequeue: YES];

	  if (newEvent != nil)
            {
              theEvent = newEvent;
              mouseUp = ([theEvent type] == NSLeftMouseUp);
            }
	}
    }

  if (!mouseUpInCell)
    {
      if (_mode == NSRadioModeMatrix && !_allowsEmptySelection)
        {
          [self selectCell: originallySelectedCell];
        }
      [self sendAction]; /* like OPENSTEP, unlike MacOSX */  
    }
}

- (void) _mouseDownListMode: (NSEvent *) theEvent
{
  NSPoint locationInWindow, mouseLocation;
  NSInteger mouseRow, mouseColumn;
  NSInteger mouseIndex, previousIndex = 0, anchor = 0;
  id mouseCell, previousCell = nil;
  BOOL onCell;
  BOOL isSelecting = YES;
  NSUInteger eventMask = NSLeftMouseUpMask | NSLeftMouseDownMask
                     | NSMouseMovedMask | NSLeftMouseDraggedMask
                     | NSPeriodicMask;

  // List mode
  // multiple cells can be selected, dragging the mouse
  // cells do not track the mouse
  // shift key makes expands selection noncontiguously
  // alternate key expands selection contiguously
  // implementation based on OS 4.2 behaviour, that is different from MacOS X

  if (_autoscroll)
    {
      [NSEvent startPeriodicEventsAfterDelay: 0.05 withPeriod: 0.05];
    }

  locationInWindow = [theEvent locationInWindow];

  while ([theEvent type] != NSLeftMouseUp)
    {
      // must convert location each time or periodic events won't work well
      mouseLocation = [self convertPoint: locationInWindow fromView: nil];
      onCell = [self getRow: &mouseRow
		     column: &mouseColumn
		     forPoint: mouseLocation];

      if (onCell)
	{
	  mouseCell = [self cellAtRow: mouseRow column: mouseColumn];
          mouseIndex = INDEX_FROM_COORDS(mouseColumn, mouseRow);

	  if (_autoscroll)
            {
              NSRect mouseRect;
              mouseRect = [self cellFrameAtRow: mouseRow column: mouseColumn];
	      [self scrollRectToVisible: mouseRect];
            }
                         

	  if (mouseCell != previousCell && [mouseCell isEnabled] == YES)
	    {
	      if (!previousCell)
		{
                  // When the user first clicks on a cell
                  // we clear the existing selection
                  // unless the Alternate or Shift keys have been pressed.
		  if (!(mouseDownFlags & NSShiftKeyMask)
		    && !(mouseDownFlags & NSAlternateKeyMask))
		    {
		      [self deselectAllCells];
		    }
                    
                  /* The clicked cell is the anchor of the selection, unless
                   * the Alternate key is pressed, when the anchor is made
                   * the key cell, from which the selection will be 
                   * extended (this is probably not the best cell when
                   * selection is by rect)
                   */
		  if (!(mouseDownFlags & NSAlternateKeyMask))
                    {
		      anchor = INDEX_FROM_COORDS(mouseColumn, mouseRow);
                    }
                  else
                    {
                      if (_dottedColumn != -1)
		        anchor = INDEX_FROM_COORDS(_dottedColumn, _dottedRow);
                      else
                        anchor = INDEX_FROM_COORDS(0, 0);
                    }
                    
                  /* With the shift key pressed, clicking on a selected cell
                   * deselects it (and inverts the selection on mouse dragging).
                   */
		  if (mouseDownFlags & NSShiftKeyMask)
                    {
                      isSelecting = ([mouseCell state] == NSOffState);
                    }
                  else
                    {
                      isSelecting = YES;
                    }

                  previousIndex = mouseIndex;
		}
                
	      [self setSelectionFrom: previousIndex
	            to: mouseIndex
	            anchor: anchor
	            highlight: isSelecting];
              [self _setKeyRow: mouseRow column: mouseColumn];
              
              previousIndex = mouseIndex;
	      previousCell = mouseCell;
	    }
	}

      theEvent = [NSApp nextEventMatchingMask: eventMask
                                    untilDate: [NSDate distantFuture]
                                       inMode: NSEventTrackingRunLoopMode
                                      dequeue: YES];

      NSDebugLLog(@"NSMatrix", @"matrix: got event of type: %d\n",
                  (int)[theEvent type]);

      if ([theEvent type] != NSPeriodic)
        {
          locationInWindow = [theEvent locationInWindow];
        }
    }

  if (_autoscroll)
    {
      [NSEvent stopPeriodicEvents];
    }

  [self sendAction];
}

- (void) mouseDown: (NSEvent*)theEvent
{
  NSInteger row, column;
  NSPoint lastLocation = [theEvent locationInWindow];
  NSInteger clickCount;

  /*
   * Pathological case -- ignore mouse down
   */
  if ((_numRows == 0) || (_numCols == 0))
    {
      [super mouseDown: theEvent];
      return; 
    }

  // Manage multi-click events
  clickCount = [theEvent clickCount];

  if (clickCount > 2)
    return;

  if (clickCount == 2 && (_ignoresMultiClick == NO))
    {
      [self sendDoubleAction];
      return;
    }

  // From now on, code to manage simple-click events

  lastLocation = [self convertPoint: lastLocation
		       fromView: nil];

  // If mouse down was on a selectable cell, start editing/selecting.
  if ([self getRow: &row
	    column: &column
	    forPoint: lastLocation])
    {
      if ([_cells[row][column] isEnabled])
	{
	  if ([_cells[row][column] isSelectable])
	    {
	      NSText *t = [_window fieldEditor: YES forObject: self];

	      if ([t superview] != nil)
		{
		  if ([t resignFirstResponder] == NO)
		    {
		      if ([_window makeFirstResponder: _window] == NO)
			return;
		    }
		}
	      // During editing, the selected cell is the cell being edited
              [self _selectCell: _cells[row][column] atRow: row column: column];
	      _textObject = [_selectedCell setUpFieldEditorAttributes: t];
	      [_selectedCell editWithFrame: [self cellFrameAtRow: row
						  column: column]
			     inView: self
			     editor: _textObject
			     delegate: self
			     event: theEvent];
	      return;
	    }
	}
    }

  // Paranoia check -- _textObject should already be nil, since we
  // accept first responder, so NSWindow should have already given
  // us first responder status (thus already ending editing with _textObject).
  if (_textObject)
    {
      NSLog (@"Hi, I am a bug.");
      [self validateEditing];
      [self abortEditing];
    }

  mouseDownFlags = [theEvent modifierFlags];

  if (_mode != NSListModeMatrix)
    {
      [self _mouseDownNonListMode: theEvent];
    }
  else
    {
      [self _mouseDownListMode: theEvent];
    }
}


- (void) updateCell: (NSCell*)aCell
{
  NSInteger row, col;
  NSRect rect;

  if ([self getRow: &row column: &col ofCell: aCell] == NO)
    {
      return;	// Not a cell in this matrix - we can't update it.
    }

  rect = [self cellFrameAtRow: row column: col];
  [self setNeedsDisplayInRect: rect];
}

/**<p>Simulates a mouse click for the first cell with the corresponding
   key Equivalent.</p>
   <p>See Also: [NSCell-keyEquivalent]</p>
 */
- (BOOL) performKeyEquivalent: (NSEvent*)theEvent
{
  NSString *keyEquivalent = [theEvent charactersIgnoringModifiers];
  NSUInteger modifiers = [theEvent modifierFlags];
  int i;
  NSUInteger relevantModifiersMask = NSCommandKeyMask | NSAlternateKeyMask | NSControlKeyMask;

  /* Take shift key into account only for control keys and arrow and function keys */
  if ((modifiers & NSFunctionKeyMask)
      || ([keyEquivalent length] > 0 && [[NSCharacterSet controlCharacterSet] characterIsMember:[keyEquivalent characterAtIndex:0]]))
    relevantModifiersMask |= NSShiftKeyMask;

  if ([keyEquivalent length] == 0)
    return NO; // don't respond to zero-length string (such as the Windows key)

  for (i = 0; i < _numRows; i++)
    {
      int j;

      for (j = 0; j < _numCols; j++)
	{
	  NSCell *aCell = _cells[i][j];
          NSUInteger mask = 0;

          if ([aCell respondsToSelector:@selector(keyEquivalentModifierMask)])
            mask = [(NSButtonCell *)aCell keyEquivalentModifierMask];

	  if ([aCell isEnabled]
	    && [[aCell keyEquivalent] isEqualToString: keyEquivalent]
              && (mask & relevantModifiersMask) == (modifiers & relevantModifiersMask))
	    {
	      NSCell *oldSelectedCell = _selectedCell;
	      int     oldSelectedRow = _selectedRow; 
	      int     oldSelectedColumn = _selectedColumn;

	      _selectedCell = aCell;
	      [self lockFocus];
	      [self highlightCell: YES atRow: i column: j];
	      [_window flushWindow];
	      [aCell setNextState];
	      [self sendAction];
	      [self highlightCell: NO atRow: i column: j];
	      [self unlockFocus];
	      _selectedCell = oldSelectedCell;
	      _selectedRow = oldSelectedRow;
	      _selectedColumn = oldSelectedColumn;

	      return YES;
	    }
	}
    }

  return NO;
}

- (void) resetCursorRects
{
  NSInteger i;

  for (i = 0; i < _numRows; i++)
    {
      NSInteger j;

      for (j = 0; j < _numCols; j++)
	{
	  NSCell *aCell = _cells[i][j];

	  [aCell resetCursorRect: [self cellFrameAtRow: i column: j]
		 inView: self];
	}
    }
}

- (NSString*) toolTipForCell: (NSCell*)cell
{
  // FIXME
  return @"";
}

- (void) setToolTip: (NSString*)toolTipString forCell: (NSCell*)cell
{
  // FIXME
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      GSMatrixFlags matrixFlags;
      unsigned int mFlags = 0;

      [aCoder encodeObject: [self backgroundColor] forKey: @"NSBackgroundColor"];
      [aCoder encodeObject: [self cellBackgroundColor] forKey: @"NSCellBackgroundColor"];
      [aCoder encodeObject: [self prototype] forKey: @"NSProtoCell"];
      [aCoder encodeObject: NSStringFromClass([self cellClass]) forKey: @"NSCellClass"];
      [aCoder encodeSize: _cellSize forKey: @"NSCellSize"];
      [aCoder encodeSize: _intercell forKey: @"NSIntercellSpacing"];

      /// set the flags...
      matrixFlags.isRadio = ([self mode] == NSRadioModeMatrix);
      matrixFlags.isList = ([self mode] == NSListModeMatrix);
      matrixFlags.isHighlight = ([self mode] == NSHighlightModeMatrix);      
      matrixFlags.allowsEmptySelection = [self allowsEmptySelection];
      matrixFlags.selectionByRect = [self isSelectionByRect];
      matrixFlags.drawCellBackground = [self drawsCellBackground];
      matrixFlags.drawBackground = [self drawsBackground];
      matrixFlags.tabKeyTraversesCells = _tabKeyTraversesCells;
      matrixFlags.autosizesCells = _autosizesCells;

      // clear unused...
      matrixFlags.autoScroll = 0;
      matrixFlags.drawingAncestor = 0;
      matrixFlags.tabKeyTraversesCellsExplicitly = 0;
      matrixFlags.canSearchIncrementally = 0;
      matrixFlags.unused = 0;

      memcpy((void *)&mFlags,(void *)&matrixFlags,sizeof(unsigned int));
      [aCoder encodeInt: mFlags forKey: @"NSMatrixFlags"];

      [aCoder encodeInt: _numCols forKey: @"NSNumCols"];
      [aCoder encodeInt: _numRows forKey: @"NSNumRows"];
      [aCoder encodeObject: [self cells] forKey: @"NSCells"];
      [aCoder encodeInt: _selectedColumn forKey: @"NSSelectedCol"];
      [aCoder encodeInt: _selectedRow forKey: @"NSSelectedRow"];
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode (int) at: &_mode];
      [aCoder encodeValueOfObjCType: @encode (BOOL) at: &_allowsEmptySelection];
      [aCoder encodeValueOfObjCType: @encode (BOOL) at: &_selectionByRect];
      [aCoder encodeValueOfObjCType: @encode (BOOL) at: &_autosizesCells];
      [aCoder encodeValueOfObjCType: @encode (BOOL) at: &_autoscroll];
      [aCoder encodeSize: _cellSize];
      [aCoder encodeSize: _intercell];
      [aCoder encodeObject: _backgroundColor];
      [aCoder encodeObject: _cellBackgroundColor];
      [aCoder encodeValueOfObjCType: @encode (BOOL) at: &_drawsBackground];
      [aCoder encodeValueOfObjCType: @encode (BOOL) at: &_drawsCellBackground];
      [aCoder encodeObject: NSStringFromClass (_cellClass)];
      [aCoder encodeObject: _cellPrototype];
      [aCoder encodeValueOfObjCType: @encode (int) at: &_numRows];
      [aCoder encodeValueOfObjCType: @encode (int) at: &_numCols];
      
      /* This is slower, but does not expose NSMatrix internals and will work 
     with subclasses */
      [aCoder encodeObject: [self cells]];
      
      [aCoder encodeConditionalObject: _delegate];
      [aCoder encodeConditionalObject: _target];
      [aCoder encodeValueOfObjCType: @encode (SEL) at: &_action];
      [aCoder encodeValueOfObjCType: @encode (SEL) at: &_doubleAction];
      [aCoder encodeValueOfObjCType: @encode (SEL) at: &_errorAction];
      [aCoder encodeValueOfObjCType: @encode (BOOL) at: &_tabKeyTraversesCells];
      [aCoder encodeObject: [self keyCell]];
      /* We do not encode information on selected cells, because this is saved 
	 with the cells themselves */
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  Class class;
  id cell;
  int rows = 0, columns = 0;
  NSArray *array;
  NSInteger i = 0, count = 0;

  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSBackgroundColor"])
        {
          [self setBackgroundColor: [aDecoder decodeObjectForKey: @"NSBackgroundColor"]];
        }
      if ([aDecoder containsValueForKey: @"NSCellBackgroundColor"])
        {
          [self setCellBackgroundColor: [aDecoder decodeObjectForKey: @"NSCellBackgroundColor"]];
        }
      if ([aDecoder containsValueForKey: @"NSProtoCell"])
        {
          [self setPrototype: [aDecoder decodeObjectForKey: @"NSProtoCell"]];
        }
      if ([aDecoder containsValueForKey: @"NSCellClass"])
        {
          class = NSClassFromString((NSString *)[aDecoder decodeObjectForKey: @"NSCellClass"]);
          if (class != Nil)
            {
              [self setCellClass: class]; 
            }
        }
      if ([aDecoder containsValueForKey: @"NSCellSize"])
        {
          // Don't use method here as this would change the frame
          _cellSize = [aDecoder decodeSizeForKey: @"NSCellSize"];
        }
      if ([aDecoder containsValueForKey: @"NSIntercellSpacing"])
        {
          // Don't use method here as this would change the frame
          _intercell = [aDecoder decodeSizeForKey: @"NSIntercellSpacing"];
        }
      if ([aDecoder containsValueForKey: @"NSMatrixFlags"])
        {
          int mFlags = [aDecoder decodeIntForKey: @"NSMatrixFlags"];
          GSMatrixFlags matrixFlags;

          memcpy((void *)&matrixFlags,(void *)&mFlags,sizeof(struct _GSMatrixFlags));
          
          if (matrixFlags.isRadio)
            {
              [self setMode: NSRadioModeMatrix];
            }
          else if (matrixFlags.isList)
            {
              [self setMode: NSListModeMatrix];
            }
          else if (matrixFlags.isHighlight)
            {
              [self setMode: NSHighlightModeMatrix];
            }

          [self setAllowsEmptySelection: matrixFlags.allowsEmptySelection];
          [self setSelectionByRect: matrixFlags.selectionByRect];
          [self setDrawsCellBackground: matrixFlags.drawCellBackground];
          [self setDrawsBackground: matrixFlags.drawBackground];
          _autosizesCells = matrixFlags.autosizesCells;
          _tabKeyTraversesCells = matrixFlags.tabKeyTraversesCells;
        }
      if ([aDecoder containsValueForKey: @"NSNumCols"])
        {
          columns = [aDecoder decodeIntForKey: @"NSNumCols"];
        }
      if ([aDecoder containsValueForKey: @"NSNumRows"])
        {
          rows = [aDecoder decodeIntForKey: @"NSNumRows"];
        }

      array = [aDecoder decodeObjectForKey: @"NSCells"];
      [self renewRows: rows columns: columns];
      count = [array count];
      if (count != rows * columns)
        {
          NSLog (@"Trying to decode an invalid NSMatrix: cell number does not fit matrix dimension");
          // Quick fix to do what we can
          if (count > rows * columns)
            {
              count = rows * columns;
            }
        }

      _selectedRow = _selectedColumn = -1;

      for (i = 0; i < count; i++)
        {
          NSInteger row, column;
          
          cell = [array objectAtIndex: i];
          row = i / columns;
          column = i % columns;
          
          [self putCell: cell atRow: row column: column];
          if ([cell state])
            {
              [self selectCellAtRow: row column: column];
            }
        }

      // mis-use these variables for selection
      rows = -1;
      columns = -1;
      if ([aDecoder containsValueForKey: @"NSSelectedCol"])
        {
          columns = [aDecoder decodeIntForKey: @"NSSelectedCol"];
        }
      if ([aDecoder containsValueForKey: @"NSSelectedRow"])
        {
          rows = [aDecoder decodeIntForKey: @"NSSelectedRow"];
        }
      if ((rows != -1) && (columns != -1))
        [self selectCellAtRow: rows column: columns];
    }
  else
    {
      _myZone = [self zone];
      [aDecoder decodeValueOfObjCType: @encode (int) at: &_mode];
      [aDecoder decodeValueOfObjCType: @encode (BOOL) at: &_allowsEmptySelection];
      [aDecoder decodeValueOfObjCType: @encode (BOOL) at: &_selectionByRect];
      [aDecoder decodeValueOfObjCType: @encode (BOOL) at: &_autosizesCells];
      [aDecoder decodeValueOfObjCType: @encode (BOOL) at: &_autoscroll];
      _cellSize = [aDecoder decodeSize];
      _intercell = [aDecoder decodeSize];
      [aDecoder decodeValueOfObjCType: @encode (id) at: &_backgroundColor];
      [aDecoder decodeValueOfObjCType: @encode (id) at: &_cellBackgroundColor];
      [aDecoder decodeValueOfObjCType: @encode (BOOL) at: &_drawsBackground];
      [aDecoder decodeValueOfObjCType: @encode (BOOL) at: &_drawsCellBackground];
      
      class = NSClassFromString ((NSString *)[aDecoder decodeObject]);
      if (class != Nil)
        {
          [self setCellClass: class]; 
        }
      
      cell = [aDecoder decodeObject];
      if (cell != nil)
        {
          [self setPrototype: cell];
        }
      
      if (_cellPrototype == nil)
        {
          [self setCellClass: [object_getClass(self) cellClass]];
        }
      
      [aDecoder decodeValueOfObjCType: @encode (int) at: &rows];
      [aDecoder decodeValueOfObjCType: @encode (int) at: &columns];

      /* NB: This works without changes for NSForm */
      array = [aDecoder decodeObject];
      [self renewRows: rows  columns: columns];
      count = [array count];
      if (count != rows * columns)
        {
          NSLog (@"Trying to decode an invalid NSMatrix: cell number does not fit matrix dimension");
          // Quick fix to do what we can
          if (count > rows * columns)
            {
              count = rows * columns;
            }
        }
      
      _selectedRow = _selectedColumn = -1;

      for (i = 0; i < count; i++)
        {
          NSInteger row, column;

          cell = [array objectAtIndex: i];
          row = i / columns;
          column = i % columns;
          
          [self putCell: cell atRow: row column: column];
          if ([cell state])
            {
              [self selectCellAtRow: row column: column];
            }
        }
      
      [aDecoder decodeValueOfObjCType: @encode (id) at: &_delegate];
      [aDecoder decodeValueOfObjCType: @encode (id) at: &_target];
      [aDecoder decodeValueOfObjCType: @encode (SEL) at: &_action];
      [aDecoder decodeValueOfObjCType: @encode (SEL) at: &_doubleAction];
      [aDecoder decodeValueOfObjCType: @encode (SEL) at: &_errorAction];
      [aDecoder decodeValueOfObjCType: @encode (BOOL) at: &_tabKeyTraversesCells];
      [self setKeyCell: [aDecoder decodeObject]];
    }
  
  return self;
}

/** <p>Sets the NSMatrix's mode to aMode. See <ref type="type" 
    id="NSMatrixMode">NSMatrixMode</ref> for more informations. By default
    the mode is <ref type="type" id="NSMatrixMode">NSRadioModeMatrix</ref>.
    </p><p>See Also: -setMode:</p>
 */
- (void) setMode: (NSMatrixMode)aMode
{
  _mode = aMode;
}

/** <p>Returns the NSMatrix's mode. See <ref type="type" id="NSMatrixMode">
    NSMatrixMode</ref> for more informations. By default the mode
    is <ref type="type" id="NSMatrixMode">NSRadioModeMatrix</ref>.</p>
    <p>See Also: -setMode:</p>
 */
- (NSMatrixMode) mode
{
  return _mode;
}

/** <p>Sets the cell class used by the NSMatrix when it creates new cells 
    to classId. The default cell class is a NSActionCell class</p>
    <p>See Also: -cellClass -setPrototype: -prototype</p>
 */
- (void) setCellClass: (Class)classId
{
  _cellClass = classId;
  if (_cellClass == nil)
    {
      _cellClass = defaultCellClass;
    }
  _cellNew = [_cellClass methodForSelector: allocSel];
  _cellInit = [_cellClass instanceMethodForSelector: initSel];
  DESTROY(_cellPrototype);
}

/** <p>Returns the cell class used by the NSMatrix when it creates new cells. 
    The default cell class  is a NSActionCell class</p>
    <p>See Also: -setCellClass: -setPrototype: -prototype</p>
 */
- (Class) cellClass
{
  return _cellClass;
}

/**<p>Sets the  prototype cell used by the NSMatrix when it creates new cells
   to aCell. The default cell is  NSActionCell</p>
    <p>See Also: -cellClass -setPrototype: -prototype</p>
 */
- (void) setPrototype: (NSCell*)aCell
{
  ASSIGN(_cellPrototype, aCell);
  if (_cellPrototype == nil)
    {
      [self setCellClass: defaultCellClass];
    }
  else
    {
      _cellNew = [_cellPrototype methodForSelector: copySel];
      _cellInit = 0;
      _cellClass = [aCell class];
    }
}

/**<p>Returns the  prototype cell used by the NSMatrix when it creates new
   cells. The default cell is  NSActionCell</p>
    <p>See Also: -cellClass -setPrototype: -prototype</p>
 */
- (id) prototype
{
  return _cellPrototype;
}

/**<p>Returns the size of the NSMatrix's cells</p>
   <p>See Also: -setCellSize:</p>
 */
- (NSSize) cellSize
{
  return _cellSize;
}

/** <p>Returns the space size between cells.</p>
    <p>See Also: -setIntercellSpacing:</p>
 */
- (NSSize) intercellSpacing
{
  return _intercell;
}

/** <p>Sets the background color to <var>aColor</var> and marks self for
    display. The background color is used to display the NSMatrix color 
    ( the space between the cells), not the cells ( uses 
    -setCellBackgroundColor: for that)</p><p>See Also: -backgroundColor
    -setCellBackgroundColor: -cellBackgroundColor -drawsBackground 
    -setDrawsBackground:</p>
 */
- (void) setBackgroundColor: (NSColor*)aColor
{
  ASSIGN(_backgroundColor, aColor);
  [self setNeedsDisplay: YES];
}

/** <p>Returns the background color The background color is used to display 
    the NSMatrix color ( the space between the cells), not the cells ( uses 
    -setCellBackgroundColor: for that)</p> <p>See Also: -setBackgroundColor:
    -setCellBackgroundColor: -cellBackgroundColor -drawsBackground 
    -setDrawsBackground:</p>
 */
- (NSColor*) backgroundColor
{
  return _backgroundColor;
}

/** <p>Sets the background color of the NSMatrix's cells to <var>aColor</var> 
    and marks self for display. </p><p>See Also: -cellBackgroundColor
    -backgroundColor -setBackgroundColor: -setDrawsCellBackground: 
    -drawsCellBackground</p>
 */
- (void) setCellBackgroundColor: (NSColor*)aColor
{
  ASSIGN(_cellBackgroundColor, aColor);
  [self setNeedsDisplay: YES];
}

/** <p>Returns the background color of the NSMatrix's cells.</p><p>See Also:
    -setCellBackgroundColor:  -backgroundColor -setBackgroundColor: </p>
 */
- (NSColor*) cellBackgroundColor
{
  return _cellBackgroundColor;
}

/**<p>Sets the delegate to <var>anObject</var>. The delegate is used 
   when editing a cell</p><p>See Also: -delegate -textDidEndEditing:
   -textDidBeginEditing: -textDidChange:</p>
 */
- (void) setDelegate: (id)anObject
{
  if (_delegate)
    [nc removeObserver: _delegate name: nil object: self];
  _delegate = anObject;

#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(controlText##notif_name:)]) \
    [nc addObserver: _delegate \
      selector: @selector(controlText##notif_name:) \
      name: NSControlText##notif_name##Notification object: self]

  if (_delegate)
    {
      SET_DELEGATE_NOTIFICATION(DidBeginEditing);
      SET_DELEGATE_NOTIFICATION(DidEndEditing);
      SET_DELEGATE_NOTIFICATION(DidChange);
    }
}


/**<p>Returns the NSMatrix's delegate. delegate is used when editing a cell</p>
   <p>See Also: -setDelegate: -textDidEndEditing:  -textDidBeginEditing: 
   -textDidChange:</p>
 */
- (id) delegate
{
  return _delegate;
}

- (void) setTarget: anObject
{
  _target = anObject;
}

- (id) target
{
  return _target;
}

/**
 * Sets the message to send when a single click occurs.<br />
 */
- (void) setAction: (SEL)aSelector
{
  _action = aSelector;
}

- (SEL) action
{
  return _action;
}

/** <p>Sets the message to send when a double click occurs.
    NB: In GNUstep the following method does *not* set 
    ignoresMultiClick to NO as in the MacOS-X spec.
    It simply sets the doubleAction, as in OpenStep spec.</p>
    <p>-doubleAction</p> 
 */
- (void) setDoubleAction: (SEL)aSelector
{
  _doubleAction = aSelector;
}

/** <p>Returns the action method, used when the user double clicks</p>
    <p>See Also: -setDoubleAction:</p>
 */
- (SEL) doubleAction
{
  return _doubleAction;
}

/**<p>Sets the error action method to <var>aSelector</var>. This error method
   is used when in -textShouldEndEditing: if the selected cell doe not
   have a valid text object</p>
   <p>See Also: -errorAction</p>
 */
- (void) setErrorAction: (SEL)aSelector
{
  _errorAction = aSelector;
}

/**<p>Returns the error action method to <var>aSelector</var>This error method
   is used when in -textShouldEndEditing: if the selected cell doe not
   have a valid text object</p>
   <p>See Also: -setErrorAction:</p>
 */
- (SEL) errorAction
{
  return _errorAction;
}

/**<p> Enables or disables all cells of the receiver. </p>
 */
- (void) setEnabled: (BOOL)flag
{
  NSInteger i, j;

  for (i = 0; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  [_cells[i][j] setEnabled: flag];
	}
    }
}

/**<p> Sets a flag to indicate whether the matrix should permit empty
   selections or should force one or mor cells to be selected at all times.
   </p><p>See Also: -allowsEmptySelection</p>
 */
- (void) setAllowsEmptySelection: (BOOL)flag
{
  _allowsEmptySelection = flag;
}

/**<p>Returns whether the matrix should permit empty selections or should
   force one or mor cells to be selected at all times.</p>
   <p>See Also: -setAllowsEmptySelection:</p>
 */
- (BOOL) allowsEmptySelection
{
  return _allowsEmptySelection;
}

- (void) setSelectionByRect: (BOOL)flag
{
  _selectionByRect = flag;
}

/** 
 */
- (BOOL) isSelectionByRect
{
  return _selectionByRect;
}

/** <p>Sets whether the NSMatrix draws its background and marks self for
    display.</p>
    <p>See Also: -drawsBackground -setDrawsCellBackground:</p>
 */
- (void) setDrawsBackground: (BOOL)flag
{
  _drawsBackground = flag;
  [self setNeedsDisplay: YES];
}

/** <p>Returns whether the NSMatrix draws its background</p>
    <p>See Also: -setDrawsBackground: -drawsCellBackground</p>
 */
- (BOOL) drawsBackground
{
  return _drawsBackground;
}

/**<p>Sets whether the NSMatrix draws cells backgrounds and marks self for
   display</p><p>See Also: -drawsCellBackground -setDrawsBackground:</p>
 */
- (void) setDrawsCellBackground: (BOOL)flag
{
  _drawsCellBackground = flag;
  [self setNeedsDisplay: YES];
}

/**<p>Returns whether the NSMatrix draws cells backgrounds</p>
   <p>See Also: -setDrawsCellBackground: -drawsBackground</p>
 */
- (BOOL) drawsCellBackground
{
  return _drawsCellBackground;
}

/** <p>Sets whether the NSMatrix resizes its cells automatically</p>
    <p>See Also: -autosizesCells</p>
 */
- (void) setAutosizesCells: (BOOL)flag
{
  _autosizesCells = flag;
}

/** <p>Returns whether the NSMatrix resizes its cells automatically</p>
    <p>See Also: -autosizesCells</p>
 */
- (BOOL) autosizesCells
{
  return _autosizesCells;
}

- (BOOL) isAutoscroll
{
  return _autoscroll;
}

/**<p>Returns the number of rows of the NSMatrix</p>
   <p>See Also: -numberOfColumns</p>
 */
- (NSInteger) numberOfRows
{
  return _numRows;
}

/**<p>Returns the number of columns of the NSMatrix</p>
   <p>See Also: -numberOfRows</p>
 */
- (NSInteger) numberOfColumns
{
  return _numCols;
}

- (id) selectedCell
{
  return _selectedCell;
}

/**<p>Returns the column number of the selected cell or -1 
   if no cell is selected</p><p>See Also: -selectedRow -selectedCell</p>
 */
- (NSInteger) selectedColumn
{
  return _selectedColumn;
}


/**<p>Returns the row number of the selected cell or -1 
   if no cell is selected</p><p>See Also: -selectedColumn -selectedCell</p>
 */
- (NSInteger) selectedRow
{
  return _selectedRow;
}

- (NSInteger) mouseDownFlags
{
  return mouseDownFlags;
}

- (BOOL) isFlipped
{
  return YES;
}

- (void) _rebuildLayoutAfterResizing
{
  if (_autosizesCells)
    {
      /* Keep the intercell as it is, and adjust the cell size to fit.  */
      if (_numRows > 1)
	{
	  _cellSize.height = _bounds.size.height - ((_numRows - 1) * _intercell.height);
	  _cellSize.height = _cellSize.height / _numRows;
	  if (_cellSize.height < 0)
	    {
	      _cellSize.height = 0;
	    }
	}
      else
	{
	  _cellSize.height = _bounds.size.height;
	}
      
      if (_numCols > 1)
	{
	  _cellSize.width = _bounds.size.width - ((_numCols - 1) * _intercell.width);
	  _cellSize.width = _cellSize.width / _numCols;
	  if (_cellSize.width < 0)
	    {
	      _cellSize.width = 0;
	    }
	}
      else
	{
	  _cellSize.width = _bounds.size.width;
	}
    }
}

- (void) setFrame: (NSRect)aFrame
{
  [super setFrame: aFrame];
  [self _rebuildLayoutAfterResizing];
}

- (void) setFrameSize: (NSSize)aSize
{
  [super setFrameSize: aSize];
  [self _rebuildLayoutAfterResizing];
}

- (void) _move: (unichar)pos
{
  BOOL selectCell = NO;
  NSInteger h, i, lastDottedRow, lastDottedColumn;

  if (_mode == NSRadioModeMatrix || _mode == NSListModeMatrix)
    selectCell = YES;

  if (_dottedRow == -1 || _dottedColumn == -1)
    {
      if (pos == NSUpArrowFunctionKey || pos == NSDownArrowFunctionKey)
	{
	  for (h = 0; h < _numCols; h++)
	    {
	      for (i = 0; i < _numRows; i++)
		{
		  if ([_cells[i][h] acceptsFirstResponder])
		    {
		      _dottedRow = i;
		      _dottedColumn = h;
		      break;
		    }
		}

	      if (i == _dottedRow)
		break;
	    }
	}
      else
	{
	  for (i = 0; i < _numRows; i++)
	    {
	      for (h = 0; h < _numCols; h++)
		{
		  if ([_cells[i][h] acceptsFirstResponder])
		    {
		      _dottedRow = i;
		      _dottedColumn = h;
		      break;
		    }
		}

	      if (h == _dottedColumn)
		break;
	    }
	}

      if (_dottedRow == -1 || _dottedColumn == -1)
	return;

      if (selectCell)
	{
	  if (_selectedCell)
	    {
	      [self deselectAllCells];
	    }

	  [self selectCellAtRow: _dottedRow column: _dottedColumn];
	}
      else
	[self setNeedsDisplayInRect: [self cellFrameAtRow: _dottedRow
					   column: _dottedColumn]];
    }
  else
    {
      lastDottedRow = _dottedRow;
      lastDottedColumn = _dottedColumn;

      if (pos == NSUpArrowFunctionKey)
	{
	  if (_dottedRow <= 0)
	    return;

	  for (i = _dottedRow-1; i >= 0; i--)
	    {
	      if ([_cells[i][_dottedColumn] acceptsFirstResponder])
		{
		  _dottedRow = i;
		  break;
		}
	    }
	}
      else if (pos == NSDownArrowFunctionKey)
	{
	  if (_dottedRow >= _numRows-1)
	    return;

	  for (i = _dottedRow+1; i < _numRows; i++)
	    {
	      if ([_cells[i][_dottedColumn] acceptsFirstResponder])
		{
		  _dottedRow = i;
		  break;
		}
	    }
	}
      else if (pos == NSLeftArrowFunctionKey)
	{
	  if (_dottedColumn <= 0)
	    return;

	  for (i = _dottedColumn-1; i >= 0; i--)
	    {
	      if ([_cells[_dottedRow][i] acceptsFirstResponder])
		{
		  _dottedColumn = i;
		  break;
		}
	    }
	}
      else
	{
	  if (_dottedColumn >= _numCols-1)
	    return;

	  for (i = _dottedColumn+1; i < _numCols; i++)
	    {
	      if ([_cells[_dottedRow][i] acceptsFirstResponder])
		{
		  _dottedColumn = i;
		  break;
		}
	    }
	}

      if ((pos == NSUpArrowFunctionKey || pos == NSDownArrowFunctionKey)
	  && _dottedRow != i)
	return;

      if ((pos == NSLeftArrowFunctionKey || pos == NSRightArrowFunctionKey)
	  && _dottedColumn != i)
	return;

      if (selectCell)
	{
	  if (_mode == NSRadioModeMatrix)
	    {
	      /* FIXME */
	      /*
	      NSCell *aCell = _cells[lastDottedRow][lastDottedColumn];
	      BOOL    isHighlighted = [aCell isHighlighted];

	      if ([aCell state] || isHighlighted)
		{
		  [aCell setState: NSOffState];
		  _selectedCells[lastDottedRow][lastDottedColumn] = NO;
		  _selectedRow = _selectedColumn = -1;
		  _selectedCell = nil;

		  if (isHighlighted)
		    [self highlightCell: NO
			  atRow: lastDottedRow
			  column: lastDottedColumn];
		  else
		    [self drawCell: aCell];
		}
	      */
	    }
	  else
	    [self deselectAllCells];

	  [self selectCellAtRow: _dottedRow column: _dottedColumn];
	}
      else
	{
	  [self setNeedsDisplayInRect: [self cellFrameAtRow: lastDottedRow
					     column: lastDottedColumn]];
	  [self setNeedsDisplayInRect: [self cellFrameAtRow: _dottedRow
					     column: _dottedColumn]];
	}
    }

  if (selectCell)
    {
      [self displayIfNeeded];
      [self performClick: self];
    }
}

- (void) moveUp: (id)sender
{
  [self _move: NSUpArrowFunctionKey];
}

- (void) moveDown: (id)sender
{
  [self _move: NSDownArrowFunctionKey];
}

- (void) moveLeft: (id)sender
{
  [self _move: NSLeftArrowFunctionKey];
}

- (void) moveRight: (id)sender
{
  [self _move: NSRightArrowFunctionKey];
}

- (void) _shiftModifier: (unichar)character
{
  int i, lastDottedRow, lastDottedColumn;

  lastDottedRow = _dottedRow;
  lastDottedColumn = _dottedColumn;

  if (character == NSUpArrowFunctionKey)
    {
      if (_dottedRow <= 0)
	return;

      for (i = _dottedRow-1; i >= 0; i--)
	{
	  if ([_cells[i][_dottedColumn] acceptsFirstResponder])
	    {
	      _dottedRow = i;
	      break;
	    }
	}

      if (_dottedRow != i)
	return;
    }
  else if (character == NSDownArrowFunctionKey)
    {
      if (_dottedRow < 0 || _dottedRow >= _numRows-1)
	return;

      for (i = _dottedRow+1; i < _numRows; i++)
	{
	  if ([_cells[i][_dottedColumn] acceptsFirstResponder])
	    {
	      _dottedRow = i;
	      break;
	    }
	}
    }
  else if (character == NSLeftArrowFunctionKey)
    {
      if (_dottedColumn <= 0)
	return;

      for (i = _dottedColumn-1; i >= 0; i--)
	{
	  if ([_cells[_dottedRow][i] acceptsFirstResponder])
	    {
	      _dottedColumn = i;
	      break;
	    }
	}
    }
  else
    {
      if (_dottedColumn < 0 || _dottedColumn >= _numCols-1)
	return;

      for (i = _dottedColumn+1; i < _numCols; i++)
	{
	  if ([_cells[_dottedRow][i] acceptsFirstResponder])
	    {
	      _dottedColumn = i;
	      break;
	    }
	}
    }

  [self lockFocus];
  [self drawCell: _cells[lastDottedRow][lastDottedColumn]];
  [self drawCell: _cells[_dottedRow][_dottedColumn]];
  [self unlockFocus];
  [_window flushWindow];

  [self performClick: self];
}

- (void) _altModifier: (unichar)character
{
  switch (character)
    {
    case NSUpArrowFunctionKey:
      if (_dottedRow <= 0)
	return;

      _dottedRow--;
      break;

    case NSDownArrowFunctionKey:
      if (_dottedRow < 0 || _dottedRow >= _numRows-1)
	return;

      _dottedRow++;
      break;

    case NSLeftArrowFunctionKey:
      if (_dottedColumn <= 0)
	return;

      _dottedColumn--;
      break;

    case NSRightArrowFunctionKey:
      if (_dottedColumn < 0 || _dottedColumn >= _numCols-1)
	return;

      _dottedColumn++;
      break;
    }

  [self setSelectionFrom: INDEX_FROM_COORDS(_selectedColumn, _selectedRow)
	to: INDEX_FROM_COORDS(_dottedColumn, _dottedRow)
	anchor: INDEX_FROM_COORDS(_selectedColumn, _selectedRow)
	highlight: YES];

  [self displayIfNeeded];
  [self performClick: self];
}

- (void) keyDown: (NSEvent *)theEvent
{
  NSString *characters = [theEvent characters];
  NSUInteger modifiers = [theEvent modifierFlags];
  unichar  character = 0;

  if ([characters length] > 0)
    {
      character = [characters characterAtIndex: 0];
    }

  switch (character)
    {
    case NSCarriageReturnCharacter:
    case NSNewlineCharacter:
    case NSEnterCharacter: 
      [self selectText: self];
      break;

    case ' ':
      if (_dottedRow != -1 && _dottedColumn != -1)
	{
	  if (modifiers & NSAlternateKeyMask)
	    [self _altModifier: character];
	  else
	    {
	      switch (_mode)
		{
		case NSTrackModeMatrix:
		case NSHighlightModeMatrix:
		case NSRadioModeMatrix:
		  [self selectCellAtRow: _dottedRow column: _dottedColumn];
		  break;

		case NSListModeMatrix:
		  if (!(modifiers & NSShiftKeyMask))
		    [self deselectAllCells];
		  break;
		}

	      [self displayIfNeeded];
	      [self performClick: self];
	    }
	  return;
	}
      break;

    case NSLeftArrowFunctionKey:
    case NSRightArrowFunctionKey:
      if (_numCols <= 1)
	break;

    case NSUpArrowFunctionKey:
    case NSDownArrowFunctionKey:
      if (modifiers & NSShiftKeyMask)
	[self _shiftModifier: character];
      else if (modifiers & NSAlternateKeyMask)
	[self _altModifier: character];
      else
	{
	  if (character == NSUpArrowFunctionKey)
	    [self moveUp: self];
	  else if (character == NSDownArrowFunctionKey)
	    [self moveDown: self];
	  else if (character == NSLeftArrowFunctionKey)
	    [self moveLeft: self];
	  else
	    [self moveRight: self];
	}
      return;

    case NSBackTabCharacter:
      if (_tabKeyTraversesCells)
	{
          if ([self _selectNextSelectableCellAfterRow: _selectedRow
                                               column: _selectedColumn])
            return;
        }
      break;

    case NSTabCharacter:
      if (_tabKeyTraversesCells)
	{
	  if ([theEvent modifierFlags] & NSShiftKeyMask)
	    {
	      if ([self _selectNextSelectableCellAfterRow: _selectedRow
		       column: _selectedColumn])
		return;
	    }
	  else
	    {
	      if ([self _selectPreviousSelectableCellBeforeRow: _selectedRow
		       column: _selectedColumn])
		return;
	    }
	}
      break;

    default:
      break;
    }

  [super keyDown: theEvent];
}

- (void) performClick: (id)sender
{
  [super sendAction: _action to: _target];
}

- (BOOL) acceptsFirstResponder
{
  // We gratefully accept keyboard events.
  return YES;
}

- (void) _setNeedsDisplayDottedCell
{
  if (_dottedRow != -1 && _dottedColumn != -1)
    {
      [self setNeedsDisplayInRect: [self cellFrameAtRow: _dottedRow
					 column: _dottedColumn]];
    }
}

- (BOOL) becomeFirstResponder
{
  [self _setNeedsDisplayDottedCell];

  return YES;
}

- (BOOL) resignFirstResponder
{
  [self _setNeedsDisplayDottedCell];

  return YES;
}

- (void) becomeKeyWindow
{
  [self _setNeedsDisplayDottedCell];
}

- (void) resignKeyWindow
{
  [self _setNeedsDisplayDottedCell];
}

- (BOOL) abortEditing
{
  if (_textObject)
    {
      [_selectedCell endEditing: _textObject];
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
  if (_textObject)
    {
      NSFormatter *formatter;
      NSString *string;

      formatter = [_selectedCell formatter];
      string = AUTORELEASE ([[_textObject text] copy]);

      if (formatter == nil)
	{
	  [_selectedCell setStringValue: string];
	}
      else
	{
	  id newObjectValue;
	  NSString *error;
 
	  if ([formatter getObjectValue: &newObjectValue 
			 forString: string 
			 errorDescription: &error] == YES)
	    {
	      [_selectedCell setObjectValue: newObjectValue];
	    }
	  else
	    {
	      if ([_delegate control: self 
			     didFailToFormatString: string 
			     errorDescription: error] == YES)
		{
		  [_selectedCell setStringValue: string];
		}
	      
	    }
	}
    }
}

- (void) setValue: (id)anObject forKey: (NSString*)aKey
{
  if ([aKey isEqual: NSSelectedTagBinding])
    {
      [self selectCellWithTag: [anObject integerValue]];
    }
  else
    {
      [super setValue: anObject forKey: aKey];
    }
}

- (id) valueForKey: (NSString*)aKey
{
  if ([aKey isEqual: NSSelectedTagBinding])
    {
      return [NSNumber numberWithInteger: [self selectedTag]];
    }
  else
    {
      return [super valueForKey: aKey];
    }
}

@end


@implementation NSMatrix (PrivateMethods)

/*
 * Renew rows and columns, but when expanding the matrix, refrain from
 * creating rowSpace  items in the last row and colSpace items in the
 * last column.  When inserting the contents of an array into the matrix,
 * this avoids creation of new cless which would immediately be replaced
 * by those from the array.
 * NB. new spaces in the matrix are pre-initialised with nil values so
 * that replacing them doesn't cause attempts to release random memory.
 */
- (void) _renewRows: (NSInteger)row
	    columns: (NSInteger)col
	   rowSpace: (NSInteger)rowSpace
	   colSpace: (NSInteger)colSpace
{
  NSInteger i, j;
  NSInteger oldMaxC;
  NSInteger oldMaxR;
  SEL mkSel = @selector(makeCellAtRow:column:);
  IMP mkImp = [self methodForSelector: mkSel];

//NSLog(@"%x - mr: %d mc:%d nr:%d nc:%d r:%d c:%d", (unsigned)self, _maxRows, _maxCols, _numRows, _numCols, row, col);
  if (row < 0)
    {
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"renew negative row (%d) in matrix", (int)row);
#else
      [NSException raise: NSRangeException
		  format: @"renew negative row (%d) in matrix", (int)row];
#endif
      row = 0;
    }
  if (col < 0)
    {
#if	NSMATRIX_STRICT_CHECKING == 0
      NSLog(@"renew negative column (%d) in matrix", (int)col);
#else
      [NSException raise: NSRangeException
		  format: @"renew negative column (%d) in matrix", (int)col];
#endif
      col = 0;
    }

  /*
   * Update matrix dimension before we actually change it - so that
   * makeCellAtRow:column: doesn't think we are trying to make a cell
   * outside the array bounds.
   * Our implementation doesn't care, but a subclass might use
   * putCell:atRow:column: to implement it, and that checks bounds.
   */
  oldMaxC = _maxCols;
  _numCols = col;
  if (col > _maxCols)
    _maxCols = col;
  oldMaxR = _maxRows;
  _numRows = row;
  if (row > _maxRows)
    _maxRows = row;

  if (col > oldMaxC)
    {
      NSInteger end = col - 1;

      for (i = 0; i < oldMaxR; i++)
	{
	  _cells[i] = NSZoneRealloc(_myZone, _cells[i], col * sizeof(id));
	  _selectedCells[i] = NSZoneRealloc(_myZone, _selectedCells[i], 
                                            col * sizeof(BOOL));

	  for (j = oldMaxC; j < col; j++)
	    {
	      _cells[i][j] = nil;
	      _selectedCells[i][j] = NO;
	      if (j == end && colSpace > 0)
		{
		  colSpace--;
		}
	      else
		{
		  (*mkImp)(self, mkSel, i, j);
		}
	    }
	}
    }

  if (row > oldMaxR)
    {
      NSInteger end = row - 1;

      _cells = NSZoneRealloc(_myZone, _cells, row * sizeof(id*));
      _selectedCells
	= NSZoneRealloc(_myZone, _selectedCells, row * sizeof(BOOL*));

      /* Allocate the new rows and fill them */
      for (i = oldMaxR; i < row; i++)
	{
	  _cells[i] = NSZoneMalloc(_myZone, _maxCols * sizeof(id));
	  _selectedCells[i] = NSZoneMalloc(_myZone, _maxCols * sizeof(BOOL));

	  if (i == end)
	    {
	      for (j = 0; j < _maxCols; j++)
		{
		  _cells[i][j] = nil;
		  _selectedCells[i][j] = NO;
		  if (rowSpace > 0)
		    {
		      rowSpace--;
		    }
		  else
		    {
		      (*mkImp)(self, mkSel, i, j);
		    }
		}
	    }
	  else
	    {
	      for (j = 0; j < _maxCols; j++)
		{
		  _cells[i][j] = nil;
		  _selectedCells[i][j] = NO;
		  (*mkImp)(self, mkSel, i, j);
		}
	    }
	}
    }

  [self deselectAllCells];
//NSLog(@"%x - end mr: %d mc:%d nr:%d nc:%d r:%d c:%d", (unsigned)self, _maxRows, _maxCols, _numRows, _numCols, row, col);
}

- (void) _setState: (NSInteger)state
	 highlight: (BOOL)highlight
	startIndex: (NSInteger)start
	  endIndex: (NSInteger)end
{
  NSInteger i;
  MPoint startPoint = POINT_FROM_INDEX(start);
  MPoint endPoint = POINT_FROM_INDEX(end);

  for (i = startPoint.y; i <= endPoint.y; i++)
    {
      NSInteger j;
      NSInteger colLimit;

      if (_selectionByRect || i == startPoint.y)
	{
	  j = startPoint.x;
	}
      else
	{
	  j = 0;
	}

      if (_selectionByRect || i == endPoint.y)
	colLimit = endPoint.x;
      else
	colLimit = _numCols - 1;

      for (; j <= colLimit; j++)
	{
	  NSCell *aCell = _cells[i][j];

          if ([aCell isEnabled]
	    && ([aCell state] != state || [aCell isHighlighted] != highlight
	      || (state == NSOffState && _selectedCells[i][j] != NO)
	      || (state != NSOffState && _selectedCells[i][j] == NO)))
            {
	      [aCell setState: state];

	      if (state == NSOffState)
	        _selectedCells[i][j] = NO;
	      else
	        _selectedCells[i][j] = YES;

	      [aCell setHighlighted: highlight];
	      [self setNeedsDisplayInRect: [self cellFrameAtRow: i column: j]];
            }
	}
    }
}

// Return YES on success; NO if no selectable cell found.
-(BOOL) _selectNextSelectableCellAfterRow: (NSInteger)row
				   column: (NSInteger)column
{
  NSInteger i, j;

  if (row > -1)
    {
      // First look for cells in the same row
      for (j = column + 1; j < _numCols; j++)
	{
	  if ([_cells[row][j] isEnabled] && [_cells[row][j] isSelectable])
	    {
	      _selectedCell = [self selectTextAtRow: row column: j];
	      _selectedRow = row;
	      _selectedColumn = j;
	      _selectedCells[row][j] = YES;
	      return YES;
	    }
	}
    }
  // Otherwise, make the big cycle.
  for (i = row + 1; i < _numRows; i++)
    {
      for (j = 0; j < _numCols; j++)
	{
	  if ([_cells[i][j] isEnabled] && [_cells[i][j] isSelectable])
	    {
	      _selectedCell = [self selectTextAtRow: i column: j];
	      _selectedRow = i;
	      _selectedColumn = j;
	      _selectedCells[i][j] = YES;
	      return YES;
	    }
	}
    }
  return NO;
}

-(BOOL) _selectPreviousSelectableCellBeforeRow: (NSInteger)row
					column: (NSInteger)column
{
  NSInteger i,j;

  if (row < _numRows)
    {
      // First look for cells in the same row
      for (j = column - 1; j > -1; j--)
	{
	  if ([_cells[row][j] isEnabled] && [_cells[row][j] isSelectable])
	    {
	      _selectedCell = [self selectTextAtRow: row column: j];
	      _selectedRow = row;
	      _selectedColumn = j;
	      _selectedCells[row][j] = YES;
	      return YES;
	    }
	}
    }
  // Otherwise, make the big cycle.
  for (i = row - 1; i > -1; i--)
    {
      for (j = _numCols - 1; j > -1; j--)
	{
	  if ([_cells[i][j] isEnabled] && [_cells[i][j] isSelectable])
	    {
	      _selectedCell = [self selectTextAtRow: i column: j];
	      _selectedRow = i;
	      _selectedColumn = j;
	      _selectedCells[i][j] = YES;
	      return YES;
	    }
	}
    }
  return NO;
}

- (void) _setKeyRow: (NSInteger)row column: (NSInteger)column
{
  if (_dottedRow == row && _dottedColumn == column)
    {
      return;
    }
  if ([_cells[row][column] acceptsFirstResponder])
    {
      if (_dottedRow != -1 && _dottedColumn != -1)
        {
          [self setNeedsDisplayInRect: [self cellFrameAtRow: _dottedRow
                                             column: _dottedColumn]];
	}
      _dottedRow = row;
      _dottedColumn = column;
      [self setNeedsDisplayInRect: [self cellFrameAtRow: _dottedRow
                                         column: _dottedColumn]];
    }
}

@end
