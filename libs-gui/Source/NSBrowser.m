/** <title>NSBrowser</title>

   <abstract>Control to display and select from hierarchal lists</abstract>

   Copyright (C) 1996, 1997, 2002 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
   Author:  Franck Wolff <wolff@cybercable.fr>
   Date: November 1999
   Author:  Mirko Viviani <mirko.viviani@rccr.cremona.it>
   Date: September 2000
   Author:  Fred Kiefer <FredKiefer@gmx.de>
   Date: September 2002

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <math.h>                  // (float)rintf(float x)
#import "config.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSIndexPath.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/NSBrowser.h"
#import "AppKit/NSBrowserCell.h"
#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSTableHeaderCell.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSBezierPath.h"
#import "GNUstepGUI/GSTheme.h"

#import "GSGuiPrivate.h"

/* Cache */
static CGFloat scrollerWidth; // == [NSScroller scrollerWidth]
static NSTextFieldCell *titleCell;
static CGFloat browserColumnSeparation;
static CGFloat browserVerticalPadding;
static BOOL browserUseBezels;

#define NSBR_COLUMN_IS_VISIBLE(i) \
(((i)>=_firstVisibleColumn)&&((i)<=_lastVisibleColumn))

//
// Internal class for maintaining information about columns
//
@interface NSBrowserColumn : NSObject <NSCoding>
{
@public
  BOOL _isLoaded;
  NSScrollView *_columnScrollView;
  NSMatrix *_columnMatrix;
  NSString *_columnTitle;
  CGFloat _width;
}

- (void) setIsLoaded: (BOOL)flag;
- (BOOL) isLoaded;
- (void) setColumnScrollView: (NSScrollView *)aView;
- (NSScrollView *) columnScrollView;
- (void) setColumnMatrix: (NSMatrix *)aMatrix;
- (NSMatrix *) columnMatrix;
- (void) setColumnTitle: (NSString *)aString;
- (NSString *) columnTitle;
@end

@implementation NSBrowserColumn

- (id) init
{
  self = [super init];
  if (nil == self)
    return nil;

  _isLoaded = NO;

  return self;
}

- (void) dealloc
{
  TEST_RELEASE(_columnScrollView);
  TEST_RELEASE(_columnMatrix);
  TEST_RELEASE(_columnTitle);
  [super dealloc];
}

- (void) setIsLoaded: (BOOL)flag
{
  _isLoaded = flag;
}

- (BOOL) isLoaded
{
  return _isLoaded;
}

- (void) setColumnScrollView: (NSScrollView *)aView
{
  ASSIGN(_columnScrollView, aView);
}

- (NSScrollView *) columnScrollView
{
  return _columnScrollView;
}

- (void) setColumnMatrix: (NSMatrix *)aMatrix
{
  ASSIGN(_columnMatrix, aMatrix);
}

- (NSMatrix *) columnMatrix
{
  return _columnMatrix;
}

- (void) setColumnTitle: (NSString *)aString
{
  if (!aString)
    aString = @"";

  ASSIGN(_columnTitle, aString);
}

- (NSString *) columnTitle
{
  return _columnTitle;
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
    }
  else
    {
      int dummy = 0;
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isLoaded];
      [aCoder encodeObject: _columnScrollView];
      [aCoder encodeObject: _columnMatrix];
      [aCoder encodeValueOfObjCType: @encode(int) at: &dummy];
      [aCoder encodeObject: _columnTitle];
    }
}
  
- (id) initWithCoder: (NSCoder *)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
    }
  else
    {
      int dummy = 0;

      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isLoaded];
      _columnScrollView = [aDecoder decodeObject];
      if (_columnScrollView)
        RETAIN(_columnScrollView);
      _columnMatrix = [aDecoder decodeObject];
      if (_columnMatrix)
        RETAIN(_columnMatrix);
      [aDecoder decodeValueOfObjCType: @encode(int) at: &dummy];
      _columnTitle = [aDecoder decodeObject];
      if (_columnTitle)
        RETAIN(_columnTitle);
    }
  return self;
}

@end

// NB: this is used in the NSFontPanel too
@interface GSBrowserTitleCell: NSTableHeaderCell
@end

@implementation GSBrowserTitleCell

// Default appearance of GSBrowserTitleCell
- (id) initTextCell: (NSString *)aString
{
  self = [super initTextCell: aString];
  if (!self)
    return nil;

  [self setTextColor: [[GSTheme theme] browserHeaderTextColor]];
  return self;
}

- (NSRect) drawingRectForBounds: (NSRect)theRect
{
  // This adjustment must match the drawn border
  return [[GSTheme theme] browserHeaderDrawingRectForCell: self
						withFrame: theRect];
}

- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame 
                                    inView: (NSView*)controlView
{
  [[GSTheme theme] drawBrowserHeaderCell: self
			       withFrame: cellFrame
				  inView: controlView];
}

- (BOOL) isOpaque
{
  return NO;
}

@end

//
// Private NSBrowser methods
//
@interface NSBrowser (Private)
- (NSString *) _getTitleOfColumn: (NSInteger)column;
- (void) _performLoadOfColumn: (NSInteger)column;
- (void) _remapColumnSubviews: (BOOL)flag;
- (void) _setColumnTitlesNeedDisplay;
- (NSBorderType) _resolvedBorderType;
- (void) _themeDidActivate: (NSNotification*)notification;
@end

//
// NSBrowser implementation
//
@implementation NSBrowser

/** <p>Returns the NSBrowserCell class (regardless of whether a 
    -setCellClass: message has been sent to a particular instance). This
    method is not meant to be used by applications.</p>
    <p>See Also: -setCellClass: </p> */

+ (Class) cellClass
{
  return [NSBrowserCell class];
}

/** <p>Sets the class of NSCell used in the columns of the NSBrowser.</p>
 * <p>See Also: -setCellPrototype: -cellPrototype +cellClass</p> */
- (void) setCellClass: (Class)classId
{
  NSCell *aCell;

  aCell = [[classId alloc] init];
  // set the prototype for the new class
  [self setCellPrototype: aCell];
  RELEASE(aCell);
}

/** <p>Returns the NSBrowser's prototype NSCell instance.</p>
    <p>See Also: -setCellPrototype:</p>
*/
- (id) cellPrototype
{
  return _browserCellPrototype;
}

/** <p>Sets the NSCell instance copied to display items in the columns of
    NSBrowser.</p><p>See Also: -cellPrototype</p> 
*/
- (void) setCellPrototype: (NSCell *)aCell
{
  ASSIGN(_browserCellPrototype, aCell);
}

/** <p>Returns the class of NSMatrix used in the NSBrowser's columns.</p>
  <p>See Also: -setMatrixClass:</p> 
*/
- (Class) matrixClass
{
  return _browserMatrixClass;
}

/** <p>Sets the matrix class (NSMatrix or an NSMatrix subclass) used in the
    NSBrowser's columns.</p><p>See Also: -matrixClass</p> 
*/
- (void) setMatrixClass: (Class)classId
{
  _browserMatrixClass = classId;
}

/*
 * Getting matrices, cells, and rows
 */

/**<p>Returns the last (rightmost and lowest) selected NSCell. Returns nil if
   no cell is selected</p>
   <p>See Also: -selectedCells -selectedCellInColumn:</p>
*/
- (id) selectedCell
{
  NSInteger i;
  NSMatrix *matrix;

  // Nothing selected
  if ((i = [self selectedColumn]) == -1)
    {
      return nil;
    }
  
  if (!(matrix = [self matrixInColumn: i]))
    {
      return nil;
    }

  return [matrix selectedCell];
}

/** <p>Returns the last (lowest) NSCell that's selected in column. 
    Returns nil if no cell is selected</p>
    <p>See Also: -selectedCell -selectedCells</p> 
*/
- (id) selectedCellInColumn: (NSInteger)column
{
  NSMatrix *matrix;

  if (!(matrix = [self matrixInColumn: column]))
    {
      return nil;
    }

  return [matrix selectedCell];
}

/** <p>Returns a NSArray of selected cells in the rightmost column. Returns
    nil if no cell is selected.</p><p>See Also: -selectedCell 
    -selectedCellInColumn: [NSMatrix selectedCells]</p>
*/
- (NSArray *) selectedCells
{
  NSInteger i;
  NSMatrix *matrix;

  // Nothing selected
  if ((i = [self selectedColumn]) == -1)
    {
      return nil;
    }
  
  if (!(matrix = [self matrixInColumn: i]))
    {
      return nil;
    }

  return [matrix selectedCells];
}

/** <p>Selects all NSCells in the last column of the NSBrowser.</p>
    <p>See Also: [NSMatrix-selectAll:]</p>
*/
- (void) selectAll: (id)sender
{
  NSMatrix *matrix;

  if (!(matrix = [self matrixInColumn: _lastColumnLoaded]))
    {
      return;
    }

  [matrix selectAll: sender];
}

/** <p>Returns the row index of the selected cell in the column specified by
    index <var>column</var>. Returns -1 if no cell is selected</p>
   <p>See Also: -selectedCellInColumn: [NSMatrix-selectedRow]</p>
*/
- (NSInteger) selectedRowInColumn: (NSInteger)column
{
  NSMatrix *matrix;

  if (!(matrix = [self matrixInColumn: column]))
    {
      return -1;
    }

  return [matrix selectedRow];
}

/**<p>Selects the cell at index <var>row</var> in the column identified by 
   index <var>column</var>. If the delegate method 
   -browser:selectRow:inColumn: is implemented, this is its responsability
   to select the cell. This method adds a NSBrowser column if needed
   and deselects other selections if the browser does not allows multiple 
   selection.</p><p>See Also: -loadedCellAtRow:column: 
   -browser:selectRow:inColumn: [NSMatrix-selectCellAtRow:column:]</p>
 */
- (void) selectRow: (NSInteger)row inColumn: (NSInteger)column 
{
  NSMatrix *matrix;
  id cell;
  BOOL didSelect;

  if ((matrix = [self matrixInColumn: column]) == nil)
    {
      return;
    }

  if ((cell = [matrix cellAtRow: row column: 0]) == nil)
    {
      return;
    }

  [self setLastColumn: column];
  
  if (_allowsMultipleSelection == NO)
    {
      [matrix deselectAllCells];
    }

  if ([_browserDelegate respondsToSelector:
                          @selector(browser:selectRow:inColumn:)])
    {
      didSelect = [_browserDelegate browser: self
                                  selectRow: row
                                   inColumn: column];
    }
  else
    {
      [matrix selectCellAtRow: row column: 0];
      didSelect = YES;
    }

  if (didSelect && (![cell respondsToSelector: @selector(isLeaf)] 
                    || ([(NSBrowserCell*)cell isLeaf] == NO)))
    {
      [self addColumn];
    }
}

/** <p>Returns the index path of the selected item, or nil if there is
    no selection.
*/
- (NSIndexPath *) selectionIndexPath
{
  NSInteger columnNumber = 0;
  NSInteger selectedColumn = [self selectedColumn];

  if (selectedColumn > -1)
    {
      NSUInteger rowIndexes[selectedColumn + 1];
      
      for (columnNumber = 0; columnNumber <= selectedColumn; columnNumber++)
        {
          rowIndexes[columnNumber] = [self selectedRowInColumn: columnNumber];
        }

      return [[NSIndexPath alloc] initWithIndexes: rowIndexes 
                                           length: selectedColumn + 1];
    }

  return nil;
}

- (NSArray *) selectionIndexPaths
{
  NSInteger selectedColumn = [self selectedColumn];

  if (selectedColumn == -1)
    {
      return nil;
    }
  else
    {
      NSMutableArray *paths = AUTORELEASE([[NSMutableArray alloc] init]);
      NSMatrix *matrix;
      NSArray *selectedCells;
      NSUInteger count;

      // FIXME: There should be a more efficent way to the the selected row numbers
      if (!(matrix = [self matrixInColumn: selectedColumn]))
        {
          return nil;
        }

      selectedCells = [matrix selectedCells];
      if (selectedCells == nil)
        {
          return nil;
        }

      count = [selectedCells count];
      NSInteger seletedRows[count];
      NSEnumerator *enumerator = [selectedCells objectEnumerator];
      NSCell *cell;
      int i = 0;

      while ((cell = [enumerator nextObject]) != nil)
        {
          NSInteger row;
          NSInteger column;

          [matrix getRow: &row
                  column: &column
                  ofCell: cell];
          seletedRows[i++] = row;
        }

      if (selectedColumn > 0)
        {
          NSIndexPath *indexPath;
          NSUInteger rowIndexes[selectedColumn];
          NSInteger columnNumber = 0;
          
          for (columnNumber = 0; columnNumber < selectedColumn; columnNumber++)
            {
              rowIndexes[columnNumber] = [self selectedRowInColumn: columnNumber];
            }
          
          indexPath = [[NSIndexPath alloc] initWithIndexes: rowIndexes 
                                                    length: selectedColumn];
          
          for (i = 0; i < count; i++)
            {
              [paths addObject: [indexPath indexPathByAddingIndex: seletedRows[i]]];
            }
        }
      if (selectedColumn == 0)
        {
          NSIndexPath *indexPath;

          for (i = 0; i < count; i++)
            {
              indexPath = [[NSIndexPath alloc] initWithIndex: seletedRows[i]];
              [paths addObject: indexPath];
              RELEASE(indexPath);
            }
        }
      return paths;
    }

  return nil;
}

- (void) setSelectionIndexPath: (NSIndexPath *)path
{
  NSInteger column;
  NSUInteger length;

  length = [path length];
  for (column = 0; column < length; column++)
    {
      NSInteger row = [path indexAtPosition: column];

      [self selectRow: row inColumn: column];
    }
}

- (void) setSelectionIndexPaths: (NSArray *)paths
{
  NSEnumerator *enumerator = [paths objectEnumerator];
  NSIndexPath *path;

  while ((path = [enumerator nextObject]) != nil)
    {
      // FIXME
      [self setSelectionIndexPath: path];
    }
}

/** Loads if necessary and returns the NSCell at row in column. 
    if you change this code, you may want to look at the __performLoadOfColumn:
    method in which the following code is integrated (for speed) 
*/
- (id) loadedCellAtRow: (NSInteger)row
                column: (NSInteger)column
{
  NSMatrix *matrix;
  NSCell *cell;

  if ((matrix = [self matrixInColumn: column]) == nil)
    {
      return nil;
    }

  // Get the cell
  if ((cell = [matrix cellAtRow: row column: 0]) == nil)
    {
      return nil;
    }

  // Load if not already loaded
  if (![cell respondsToSelector: @selector(isLoaded)] 
      || [(NSBrowserCell*)cell isLoaded])
    {
      return cell;
    }
  else
    {
      if (_passiveDelegate || [_browserDelegate respondsToSelector: 
                  @selector(browser:willDisplayCell:atRow:column:)])
        {
          [_browserDelegate browser: self willDisplayCell: cell
                            atRow: row  column: column];
        }
      [(NSBrowserCell*)cell setLoaded: YES];
    }

  return cell;
}

/** <p>Returns the matrix located in the column identified by index 
    <var>column</var>. Returns nil if the matrix does not exists</p> 
*/
- (NSMatrix *) matrixInColumn: (NSInteger)column
{
  NSBrowserColumn *browserColumn;

  if (column < 0 || column > _lastColumnLoaded)
    {
      return nil;
    }

  browserColumn = [_browserColumns objectAtIndex: column];
  
  if ((browserColumn == nil) || !(browserColumn->_isLoaded))
    {
      return nil;
    }

  return browserColumn->_columnMatrix;
}

/*
 * Getting and setting paths
 */

/** <p>Returns the browser's current path.</p>
    <p>See Also: -pathToColumn:</p>
*/
- (NSString *) path
{
  return [self pathToColumn: _lastColumnLoaded + 1];
}

/**
 * <p>Parses path and selects corresponding items in the NSBrowser columns.
 * </p>
 * <p>This is the primary mechanism for programmatically updating the
 * selection of a browser.  It should result in the browser cells
 * corresponding to the components being selected, and the
 * browser columns up to the end of path (and just beyond if the
 * last selected cell's [NSBrowserCell-isLeaf] returns YES).<br />
 * It does <em>not</em> result in the browsers action being sent to its
 * target, just in a change to the browser selection and display.
 * </p>
 * <p>If path begins with the -pathSeparator then it is taken to be absolute
 * and the first component in it is expected to denote a cell in column
 * zero.  Otherwise it is taken to be relative to the currently selected
 * column.
 * </p>
 * <p>Empty components (ie where a -pathSeparator occurs immediately
 * after another or at the end of path) are simply ignored.
 * </p>
 * <p>The receivers delegate is asked to select each cell in turn
 * using the -browser:selectCellWithString:inColumn: method (if it
 * implements it).  If this call to the delegate returns NO then
 * the attempt to set the path fails.<br />
 * If the delegate does not implement the method, the browser attempts
 * to locate and select the cell itself, and the method fails if it
 * is unable to locate the cell by matching its [NSCell-stringValue] with
 * the component of the path.
 * </p>
 * <p>The method returns YES if path contains no components or if a cell
 * corresponding to the path was found.  Otherwise it returns NO.
 * </p>
 */
- (BOOL) setPath: (NSString *)path
{
  NSMutableArray *subStrings;
  NSUInteger numberOfSubStrings;
  NSUInteger indexOfSubStrings;
  NSInteger column;
  BOOL useDelegate = NO;

  if ([_browserDelegate respondsToSelector:
    @selector(browser:selectCellWithString:inColumn:)])
    {
      useDelegate = YES;
    }

  /*
   * Ensure that our starting column is loaded.
   */
  if (_lastColumnLoaded < 0)
    {
      [self loadColumnZero];
    }

  /*
   * Decompose the path.
   */
  subStrings = [[path componentsSeparatedByString: _pathSeparator] mutableCopy];
  [subStrings removeObject: @""];
  numberOfSubStrings = [subStrings count];
  
  if ([path hasPrefix: _pathSeparator])
    {
      NSUInteger i;
      /*
       * If the path begins with a separator, start at column 0.
       * Otherwise start at the currently selected column.
       */

      column = 0;
      /*
       * Optimisation. If there are columns loaded, it may be that the
       * specified path is already partially selected.  If this is the
       * case, we can avoid redrawing those columns.
       */
      for (i = 0; i <= _lastColumnLoaded; i++)
        {
          if ((i < numberOfSubStrings) &&
              [[[self selectedCellInColumn: i] stringValue]
                isEqualToString: [subStrings objectAtIndex: i]])
            {
              column = i;
            }
          else
            {
              // Actually it's always called at 0 column, when string is "/"
              [[self matrixInColumn: i] deselectAllCells];
              break;
            }
        }

      [self setLastColumn: column];
      indexOfSubStrings = column;
    }
  else
    {
      column = _lastColumnLoaded;
      indexOfSubStrings = 0;
    }

  // cycle thru str's array created from path
  while (indexOfSubStrings < numberOfSubStrings)
    {
      NSString                *aStr = [subStrings objectAtIndex: indexOfSubStrings];
      NSBrowserColumn        *bc = [_browserColumns objectAtIndex: column];
      NSMatrix                *matrix = [bc columnMatrix];
      NSBrowserCell        *selectedCell = nil;
      BOOL                     found = NO;

      if (useDelegate == YES)
        {
          if ([_browserDelegate browser: self
                   selectCellWithString: aStr
                               inColumn: column])
            {
              found = YES;
              selectedCell = [matrix selectedCell];
            }
        }
      else
        {
          NSInteger numOfRows = [matrix numberOfRows];
          NSInteger row;

          // find the cell in the browser matrix which is equal to aStr
          for (row = 0; row < numOfRows; row++)
            {
              selectedCell = [matrix cellAtRow: row column: 0];

              if ([[selectedCell stringValue] isEqualToString: aStr])
                {
                  [matrix selectCellAtRow: row column: 0];
                  found = YES;
                  break;
                }
            }
        }

      if (found)
        {
          indexOfSubStrings++;
        }
      else
        {
          // if unable to find a cell whose title matches aStr return NO
          NSDebugLLog (@"NSBrowser", 
                       @"unable to find cell '%@' in column %d\n", 
                      aStr, (int)column);
          break;
        }

      // if the cell is a leaf, we are finished setting the path
      if ([selectedCell isLeaf])
        {
          break;
        }
      
      // else, it is not a leaf: get a column in the browser for it
      [self addColumn];
      column++;
    }

  // Clean up local memory usage
  RELEASE(subStrings);

  if (indexOfSubStrings == numberOfSubStrings)
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/** <p>Returns a string representing the path from the first column up to,
    but not including, the column at index column.</p>
    <p>See Also: -path</p>*/
- (NSString *) pathToColumn: (NSInteger)column
{
  NSMutableString        *separator = [_pathSeparator mutableCopy];
  NSString *string;
  NSInteger i;
  
  /*
   * Cannot go past the number of loaded columns
   */
  if (column > _lastColumnLoaded)
    {
      column = _lastColumnLoaded + 1;
    }

  for (i = 0; i < column; ++i)
    {
      id cell = [self selectedCellInColumn: i];

      if (i != 0)
        {
          [separator appendString: _pathSeparator];
        }

      string = [cell stringValue];
      
      if (string == nil)
        {
          /* This should happen only when c == nil, in which case it
             doesn't make sense to go with the path */
          break;
        }
      else
        {
          [separator appendString: string];          
        }
    }
  /*
   * We actually return a mutable string, but that's ok since a mutable
   * string is a string and the documentation specifically says that
   * people should not depend on methods that return strings to return
   * immutable strings.
   */

  return AUTORELEASE (separator);
}

/**<p> Returns the path separator. The default is "/". </p>
   <p>See Also: -setPathSeparator:</p>
*/
- (NSString *) pathSeparator
{
  return _pathSeparator;
}

/** <p>Sets the path separator to <var>newString</var>. The default is "/".</p>
    <p>See Also: -pathSeparator</p>*/
- (void) setPathSeparator: (NSString *)aString
{
  ASSIGN(_pathSeparator, aString);
}


/*
 * Manipulating columns 
 */
- (NSBrowserColumn *) _createColumn
{
  NSBrowserColumn *bc;
  NSScrollView *sc;
  NSRect rect = {{-110, -110}, {100, 100}};

  bc = [[NSBrowserColumn alloc] init];

  // Create a scrollview
  sc = [[NSScrollView alloc] initWithFrame: rect];
  [sc setHasHorizontalScroller: NO];
  [sc setHasVerticalScroller: YES];
  [sc setBorderType: [self _resolvedBorderType]];
  
  [bc setColumnScrollView: sc];
  [self addSubview: sc];
  RELEASE(sc);

  [_browserColumns addObject: bc];
  RELEASE(bc);

  return bc;
}

/** <p>Adds a column to the right of the last column, adjusts subviews and
    scrolls to make the new column visible if needed.</p>
*/
- (void) addColumn
{
  NSInteger i;

  if ((NSUInteger)(_lastColumnLoaded + 1) >= [_browserColumns count])
    {
      i = [_browserColumns indexOfObject: [self _createColumn]];
    }
  else
    {
      i = _lastColumnLoaded + 1;
    }

  if (i < 0)
    {
      i = 0;
    }

  [self _performLoadOfColumn: i];
  [self setLastColumn: i];

  _isLoaded = YES;

  [self tile];

  if (i > 0  &&  i - 1 == _lastVisibleColumn)
    {
      [self scrollColumnsRightBy: 1];
    }
}

- (BOOL) acceptsFirstResponder
{
  return YES;
}

- (BOOL) becomeFirstResponder
{
  NSMatrix *matrix;
  NSInteger selectedColumn;

  selectedColumn = [self selectedColumn];
  if (selectedColumn == -1)
    matrix = [self matrixInColumn: 0];
  else
    matrix = [self matrixInColumn: selectedColumn];

  if (matrix)
    [_window makeFirstResponder: matrix];

  return YES;
}

/** <p>Updates the NSBrowser to display all loaded columns.</p>
    <p>See Also: -displayColumn: -tile</p>
*/
- (void) displayAllColumns
{
  [self tile];
  [self setNeedsDisplay: YES];
}

/** <p>Updates the NSBrowser to display the column with the given index.</p>
 */
- (void) displayColumn: (NSInteger)column
{
  NSBrowserColumn *bc;
  NSScrollView *sc;

  // If not visible then nothing to display
  if ((column < _firstVisibleColumn) || (column > _lastVisibleColumn))
    {
      return;
    }

  [self tile];

  // Display title of column
  if (_isTitled)
    {
      [self setNeedsDisplayInRect: [self titleFrameOfColumn: column]];
    }

  // Display column
  if (!(bc = [_browserColumns objectAtIndex: column]))
    return;
  if (!(sc = [bc columnScrollView]))
    return;

  /* FIXME: why the following ?  Are we displaying now, or marking for
   * later display ??  Given the name, I think we are displaying
   * now.  */
  [sc setNeedsDisplay: YES];
}

/** <p>Returns the column number in which <var>matrix</var> is located. 
    Returns -1 if <var>matrix</var> is not found.</p>
 */
- (NSInteger) columnOfMatrix: (NSMatrix *)matrix
{
  NSInteger i, count;

  // Loop through columns and compare matrixes
  count = [_browserColumns count];
  for (i = 0; i < count; ++i)
    {
      if (matrix == [self matrixInColumn: i])
        return i;
    }

  // Not found
  return -1;
}

/** Returns the index of the last column with a selected item. */
- (NSInteger) selectedColumn
{
  NSInteger i;
  NSMatrix *matrix;

  for (i = _lastColumnLoaded; i >= 0; i--)
    {
      if (!(matrix = [self matrixInColumn: i]))
        continue;
      if ([matrix selectedCell])
        return i;
    }
  
  return -1;
}

/** <p>Returns the index of the last column loaded.</p>
    <p>See Also: -setLastColumn:</p>
*/
- (NSInteger) lastColumn
{
  return _lastColumnLoaded;
}

/** <p>Sets the last column to <var>column</var>.</p>
    <p>See Also: -lastColumn </p>
*/
- (void) setLastColumn: (NSInteger)column
{
  NSInteger i, count;
  NSBrowserColumn *bc;
  NSScrollView *sc;

  if (column > _lastColumnLoaded)
    {
      return;
    }
    
  if (column < 0)
    {
      column = -1;
      _isLoaded = NO;
    }

  _lastColumnLoaded = column;

  // Unloads columns.
  count = [_browserColumns count];

  for (i = column + 1; i < count; ++i)
    {
      bc = [_browserColumns objectAtIndex: i];
      sc = [bc columnScrollView];

      if ([bc isLoaded])
        {
          // Make the column appear empty by removing the matrix
          if (sc)
            {
              [sc setDocumentView: nil];
            }
          [bc setIsLoaded: NO];
          [self setTitle: nil ofColumn: i];
        }

      if (!_reusesColumns && i > _lastVisibleColumn)
        {
          [sc removeFromSuperview];
          [_browserColumns removeObject: bc];
          count--;
          i--;
        }
    }
  
  [self scrollColumnToVisible:column];
}

/** Returns the index of the first visible column. */
- (NSInteger) firstVisibleColumn
{
  return _firstVisibleColumn;
}

/** <p>Returns the number of columns visible.</p>
    <p>See Also: -firstVisibleColumn -lastVisibleColumn</p>*/
- (NSInteger) numberOfVisibleColumns
{
  NSInteger num;

  num = _lastVisibleColumn - _firstVisibleColumn + 1;

  return (num > 0 ? num : 1);
}

/** Returns the index of the last visible column. */
- (NSInteger) lastVisibleColumn
{
  return _lastVisibleColumn;
}

/** Invokes delegate method -browser:isColumnValid: for visible columns. 
*/
- (void) validateVisibleColumns
{
  NSInteger i;

  // If delegate doesn't care, just return
  if (![_browserDelegate respondsToSelector: 
                           @selector(browser:isColumnValid:)])
    {
      return;
    }

  // Loop through the visible columns
  for (i = _firstVisibleColumn; i <= _lastVisibleColumn; ++i)
    {
      // Ask delegate if the column is valid and if not
      // then reload the column
      if (![_browserDelegate browser: self  isColumnValid: i])
        {
          [self reloadColumn: i];
        }
    }
}


/*
 * Loading columns
 */

/** Returns whether column zero is loaded. */
- (BOOL) isLoaded
{
  return _isLoaded;
}

/** Loads column zero; unloads previously loaded columns. */
- (void) loadColumnZero
{
  // set last column loaded
  [self setLastColumn: -1];

  // load column 0
  [self addColumn];

  [self _remapColumnSubviews: YES];
  [self _setColumnTitlesNeedDisplay];
}

/** Reloads column if it is loaded; sets it as the last column.
    Reselects previously selected cells, if they remain. */
- (void) reloadColumn: (NSInteger)column
{
  NSArray *selectedCells;
  NSEnumerator *selectedCellsEnumerator;
  NSMatrix *matrix;
  NSCell *cell;

  matrix = [self matrixInColumn: column];
  if (matrix == nil)
    {
      return;
    }
    
  // Get the previously selected cells
  selectedCells = [[matrix selectedCells] copy];
  
  // Perform the data load
  [self _performLoadOfColumn: column];
  // set last column loaded
  [self setLastColumn: column];

  // Restore the selected cells
  matrix = [self matrixInColumn: column];
  selectedCellsEnumerator = [selectedCells objectEnumerator];
  while ((cell = [selectedCellsEnumerator nextObject]) != nil)
    {
      NSInteger sRow, sColumn;

      if ([matrix getRow: &sRow  column: &sColumn  ofCell: cell])
        {
          [matrix selectCellAtRow: sRow  column: sColumn];
        }
    }
  RELEASE(selectedCells);
}


/*
 * Setting selection characteristics
 */

/**<p> Returns whether the user can select branch items when multiple selection
    is enabled. By default YES.</p>
    <p>See Also: -setAllowsBranchSelection:</p> 
*/
- (BOOL) allowsBranchSelection
{
  return _allowsBranchSelection;
}

/**<p>Sets whether the user can select branch items when multiple selection
   is enabled. By default YES.</p><p>See Also: -allowsBranchSelection</p> 
*/
- (void) setAllowsBranchSelection: (BOOL)flag
{
  _allowsBranchSelection = flag;
}

/**<p>Returns whether there can be nothing selected. By default YES.</p>
   <p>See Also: -setAllowsEmptySelection:</p> 
*/
- (BOOL) allowsEmptySelection
{
  return _allowsEmptySelection;
}

/** <p>Sets whether there can be nothing selected. By default YES.</p>
    <p>See Also: -allowsEmptySelection </p>
*/
- (void) setAllowsEmptySelection: (BOOL)flag
{
  if (_allowsEmptySelection != flag)
    {
      NSInteger i;

      _allowsEmptySelection = flag;
      for (i = 0; i <= _lastColumnLoaded; i++)
        {
          [[self matrixInColumn: i] setAllowsEmptySelection: flag];
        }
    }
}

/**<p>Returns whether the user can select multiple items. By default YES.</p>
   <p>See Also: -allowsMultipleSelection</p> 
*/
- (BOOL) allowsMultipleSelection
{
  return _allowsMultipleSelection;
}

/** <p>Sets whether the user can select multiple items. By default YES.</p>
    <p>See Also: -allowsMultipleSelection</p> 
*/
- (void) setAllowsMultipleSelection: (BOOL)flag
{
  if (_allowsMultipleSelection != flag)
    {
      NSInteger i;
      NSMatrixMode mode;

      _allowsMultipleSelection = flag;
      if (flag)
        {
          mode = NSListModeMatrix;
        }
      else
        {
          mode = NSRadioModeMatrix;
        }
      for (i = 0; i <= _lastColumnLoaded; i++)
        {
          [[self matrixInColumn: i] setMode: mode];
        }
    }
}


/*
 * Setting column characteristics
 */

/**<p>Returns YES if NSMatrix objects aren't freed when their columns
   are unloaded. By default a NSBrowser does not reuses their columns.</p>
   <p>See Also: -setReusesColumns: [NSMatrix-renewRows:columns:]</p> 
*/
- (BOOL) reusesColumns
{
  return _reusesColumns;
}

/**<p>If flag is YES, prevents NSMatrix objects from being freed when
   their columns are unloaded, so they can be reused.  By default a NSBrowser 
   does not reuses their columns.</p><p>See Also: -reusesColumns  
   [NSMatrix-renewRows:columns:]</p>
*/
- (void) setReusesColumns: (BOOL)flag
{
  _reusesColumns = flag;
}

/**<p>Returns the maximum number of visible columns. By default a NSBrowser
   has 3 visible columns.</p><p>See Also: -setMaxVisibleColumns:</p> 
*/
- (NSInteger) maxVisibleColumns
{
  return _maxVisibleColumns;
}

/** <p>Sets the maximum number of columns displayed and  adjusts the various 
    subviews. By default a NSBrowser has 3 visible columns.</p>
    <p>See Also: -maxVisibleColumns</p> 
*/
- (void) setMaxVisibleColumns: (NSInteger)columnCount
{
  if ((columnCount < 1) || (_maxVisibleColumns == columnCount))
    return;

  _maxVisibleColumns = columnCount;

  // Redisplay
  [self tile];
}

/** <p>Returns the minimum column width in pixels.</p> 
    <p>See Also: -setMinColumnWidth:</p>
*/
- (CGFloat) minColumnWidth
{
  return _minColumnWidth;
}

/** <p>Sets the minimum column width in pixels and adjusts subviews.</p>
    <p>See Also: -minColumnWidth</p>
*/
- (void) setMinColumnWidth: (CGFloat)columnWidth
{
  CGFloat sw;

  sw = scrollerWidth;
  // Take the border into account
  sw += 2 * ([[GSTheme theme] sizeForBorderType: [self _resolvedBorderType]]).width;

  // Column width cannot be less than scroller and border
  if (columnWidth < sw)
    _minColumnWidth = sw;
  else
    _minColumnWidth = columnWidth;

  [self tile];
}

/** <p>Returns whether columns are separated by bezeled borders. By default a 
    NSBrowser has separate columns.</p><p>See Also: -setSeparatesColumns:</p>
 */
- (BOOL) separatesColumns
{
  return _separatesColumns;
}

/**<p>Sets whether to separate columns with bezeled borders and marks self for 
   display. Does nothing if the NSBrowser is titled. By default a NSBrowser
   has separate columns.</p><p>See Also: -separatesColumns -isTitled</p>
*/
- (void) setSeparatesColumns: (BOOL)flag
{
  if (_separatesColumns == flag || _isTitled)
    return;

  _separatesColumns = flag;
  [self tile];
  [self setNeedsDisplay:YES];
}

- (CGFloat) columnWidthForColumnContentWidth: (CGFloat)columnContentWidth
{
  CGFloat cw;

  cw = columnContentWidth;
  if (scrollerWidth > cw)
    {
      cw = scrollerWidth;
    }

  // Take the border into account
  cw += 2 * ([[GSTheme theme] sizeForBorderType: [self _resolvedBorderType]]).width;

  return cw;
}

- (CGFloat) columnContentWidthForColumnWidth: (CGFloat)columnWidth
{
  CGFloat cw;

  cw = columnWidth;
  // Take the border into account
  cw -= 2 * ([[GSTheme theme] sizeForBorderType: [self _resolvedBorderType]]).width;

  return cw;
}

- (NSBrowserColumnResizingType) columnResizingType
{
  return _columnResizing;
}

- (void) setColumnResizingType:(NSBrowserColumnResizingType) type
{
  _columnResizing = type;
}

- (BOOL) prefersAllColumnUserResizing
{
  return _prefersAllColumnUserResizing;
}

- (void) setPrefersAllColumnUserResizing: (BOOL)flag
{
  _prefersAllColumnUserResizing = flag;
}

- (CGFloat) widthOfColumn: (NSInteger)column
{
  NSBrowserColumn *browserColumn;

  browserColumn = [_browserColumns objectAtIndex: column];

  return browserColumn->_width;
}

- (void) setWidth: (CGFloat)columnWidth ofColumn: (NSInteger)columnIndex
{
  NSBrowserColumn *browserColumn;

  browserColumn = [_browserColumns objectAtIndex: columnIndex];

  browserColumn->_width = columnWidth;
  // FIXME: Send a notifiaction
}


/**<p> Returns YES if the title of a column is set to the string value of 
    the selected NSCell in the previous column. By default YES</p>
    <p>See Also: -setTakesTitleFromPreviousColumn: -selectedCellInColumn:</p>
*/
- (BOOL) takesTitleFromPreviousColumn
{
  return _takesTitleFromPreviousColumn;
}

/** <p>Sets whether the title of a column is set to the string value of the
    selected NSCell in the previous column and marks self for display. By 
    default YES</p>
    <p>See Also: -takesTitleFromPreviousColumn -selectedCellInColumn:</p>
*/
- (void) setTakesTitleFromPreviousColumn: (BOOL)flag
{
  if (_takesTitleFromPreviousColumn != flag)
    {
      _takesTitleFromPreviousColumn = flag;
      [self setNeedsDisplay: YES];
    }
}

- (BOOL) autohidesScroller
{
  // FIXME
  return NO;
}

- (void) setAutohidesScroller: (BOOL)flag
{
  // FIXME
}

- (NSColor *) backgroundColor
{
  // FIXME
  return [NSColor controlColor];
}

- (void) setBackgroundColor: (NSColor *)backgroundColor
{
  // FIXME
}

- (BOOL) canDragRowsWithIndexes: (NSIndexSet *)rowIndexes
                       inColumn: (NSInteger)columnIndex
                      withEvent: (NSEvent *)dragEvent
{
  if ([_browserDelegate respondsToSelector: 
                         @selector(browser:canDragRowsWithIndexes:inColumn:withEvent:)])
    {
      return [_browserDelegate browser: self
                canDragRowsWithIndexes: rowIndexes
                              inColumn: columnIndex
                             withEvent: dragEvent];
    }
  else
    {
      // FIXME
      return NO;
    }
}

/*
 * Manipulating column titles
 */

/** Returns the title displayed for the column at index column.
 */
- (NSString *) titleOfColumn: (NSInteger)column
{
  NSBrowserColumn *browserColumn;

  browserColumn = [_browserColumns objectAtIndex: column];

  return browserColumn->_columnTitle;
}

/** <p>Sets the title of the column at index <var>column</var> to 
    <var>aString</var> and marks the title for dispaly if the NSBrowser
    can diplay titles or if the column <var>column</var> is visible.</p>
    <p>See Also: -isTitled -titleFrameOfColumn: -titleHeight</p>
*/
- (void) setTitle: (NSString *)aString
         ofColumn: (NSInteger)column
{
  NSBrowserColumn *bc;

  bc = [_browserColumns objectAtIndex: column];

  [bc setColumnTitle: aString];
  
  // If column is not visible then nothing to redisplay
  if (!_isTitled || !NSBR_COLUMN_IS_VISIBLE(column))
    return;
  
  [self setNeedsDisplayInRect: [self titleFrameOfColumn: column]];
}

/** <p>Returns whether columns display titles. By default a NSBrowser displays
    titles.</p><p>See Also: -setTitled:</p>
*/
- (BOOL) isTitled
{
  return _isTitled;
}

/** <p>Sets whether columns display titles and marks self for display.
    Does nothing if the NSBrowser hasn't separates columns. By default
    a NSBrowser displays titles.</p>
    <p>See Also: -isTitled -separatesColumns </p> 
*/
- (void) setTitled: (BOOL)flag
{
  if (_isTitled == flag || !_separatesColumns)
    return;
  
  _isTitled = flag;
  [self tile];
  [self setNeedsDisplay: YES];
}

/**
 */
- (void) drawTitleOfColumn: (NSInteger)column 
                    inRect: (NSRect)aRect
{
  [self drawTitle: [self titleOfColumn: column] 
        inRect: aRect 
        ofColumn: column];
}

/** Draws the title for the column at index column within the rectangle
    defined by aRect. */
- (void) drawTitle: (NSString *)title
            inRect: (NSRect)aRect
          ofColumn: (NSInteger)column
{
  if (!_isTitled || !NSBR_COLUMN_IS_VISIBLE(column))
    return;

//  [titleCell setControlView: self];
  [titleCell setStringValue: title];
  [titleCell drawWithFrame: aRect inView: self];
  [titleCell setControlView: nil];
}

/** <p>Returns the height of column titles. The Nextish look returns 21.</p>
 */
- (CGFloat) titleHeight
{
  // Nextish look requires 21 here
  return 21.0;
}

/** <p>Returns the bounds of the title frame for the column at index column.
    Returns NSZeroRect if the NSBrowser does not display its titles</p>
    <p>See Also: -isTitled</p>
*/
- (NSRect) titleFrameOfColumn: (NSInteger)column
{
  // Not titled then no frame
  if (!_isTitled)
    {
      return NSZeroRect;
    }
  else
    {
      // Number of columns over from the first
      NSInteger nbColumn = column - _firstVisibleColumn;
      CGFloat titleHeight = [self titleHeight];
      NSRect rect;

      // Calculate origin
      if (_separatesColumns)
        {
          rect.origin.x = nbColumn * (_columnSize.width + browserColumnSeparation);
        }
      else
        {
          rect.origin.x = nbColumn * _columnSize.width;
        }

      rect.origin.y = _frame.size.height - titleHeight;
      
      // Calculate size
      if (column == _lastVisibleColumn)
        {
          rect.size.width = _frame.size.width - rect.origin.x;
        }
      else
        {
          rect.size.width = _columnSize.width;
        }

      rect.size.height = titleHeight;

      return rect;
    }
}


/*
 * Scrolling an NSBrowser
 */

/** <p>Scrolls to make the column at index <var>column</var> visible.</p>
    <p>See Also: -scrollColumnsRightBy: -scrollColumnsLeftBy:</p>
 */
- (void) scrollColumnToVisible: (NSInteger)column
{
  // If its the last visible column then we are there already
  if (_lastVisibleColumn < column)
    {
      [self scrollColumnsRightBy: (column - _lastVisibleColumn)];
    } 
  else if (_firstVisibleColumn > column)
    {
      [self scrollColumnsLeftBy: (_firstVisibleColumn - column)];
    } 
}

/** <p>Scrolls columns left by <var>shiftAmount</var> columns.</p> 
    <p>See Also: -scrollColumnsRightBy: -scrollColumnToVisible:</p>
 */
- (void) scrollColumnsLeftBy: (NSInteger)shiftAmount
{
  // Cannot shift past the zero column
  if ((_firstVisibleColumn - shiftAmount) < 0)
    shiftAmount = _firstVisibleColumn;

  // No amount to shift then nothing to do
  if (shiftAmount <= 0)
    return;

  // Notify the delegate
  if ([_browserDelegate respondsToSelector: @selector(browserWillScroll:)])
    [_browserDelegate browserWillScroll: self];

  // Shift
  _firstVisibleColumn = _firstVisibleColumn - shiftAmount;
  _lastVisibleColumn = _lastVisibleColumn - shiftAmount;

  // Update the scroller
  [self updateScroller];

  // Update the scrollviews
  [self tile];
  [self _remapColumnSubviews: YES];
  [self _setColumnTitlesNeedDisplay];

  // Notify the delegate
  if ([_browserDelegate respondsToSelector: @selector(browserDidScroll:)])
    [_browserDelegate browserDidScroll: self];  
}

/** <p>Scrolls columns right by <var>shiftAmount</var> columns.</p>
    <p>See Also: -scrollColumnsLeftBy: -scrollColumnToVisible:</p>
*/
- (void) scrollColumnsRightBy: (NSInteger)shiftAmount
{
  // Cannot shift past the last loaded column
  if ((shiftAmount + _lastVisibleColumn) > _lastColumnLoaded)
    shiftAmount = _lastColumnLoaded - _lastVisibleColumn;

  // No amount to shift then nothing to do
  if (shiftAmount <= 0)
    return;

  // Notify the delegate
  if ([_browserDelegate respondsToSelector: @selector(browserWillScroll:)])
    [_browserDelegate browserWillScroll: self];

  // Shift
  _firstVisibleColumn = _firstVisibleColumn + shiftAmount;
  _lastVisibleColumn = _lastVisibleColumn + shiftAmount;

  // Update the scroller
  [self updateScroller];

  // Update the scrollviews
  [self tile];
  [self _remapColumnSubviews: NO];
  [self _setColumnTitlesNeedDisplay];

  // Notify the delegate
  if ([_browserDelegate respondsToSelector: @selector(browserDidScroll:)])
    [_browserDelegate browserDidScroll: self];
}

/** Updates the horizontal scroller to reflect column positions.
 */
- (void) updateScroller
{
  NSInteger   num = [self numberOfVisibleColumns];
  float prop = (float)num / (float)(_lastColumnLoaded + 1);
  NSInteger   uc = ((_lastColumnLoaded + 1) - num); // Unvisible columns
  float f_step = 1.0;                         // Knob moving step
  float fv = 0.0;

  if (uc > 0.0)
    {
      f_step = 1.0 / (float)uc;
    }
  fv = (float)(_firstVisibleColumn * f_step);

  if (_lastVisibleColumn > _lastColumnLoaded)
    {
      prop = (float)num / (float)(_lastVisibleColumn + 1);
    }

  [_horizontalScroller setFloatValue: fv knobProportion: prop];
}

/** Scrolls columns left or right based on an NSScroller. 
*/
- (void) scrollViaScroller: (NSScroller *)sender
{
  NSScrollerPart hit;

  if ([sender class] != [NSScroller class])
    return;
  
  hit = [sender hitPart];
  
  switch (hit)
    {
      // Scroll to the left
      case NSScrollerDecrementLine:
      case NSScrollerDecrementPage:
              [self scrollColumnsLeftBy: 1];
              break;
      
      // Scroll to the right
      case NSScrollerIncrementLine:
      case NSScrollerIncrementPage:
        [self scrollColumnsRightBy: 1];
              break;
      
      // The knob or knob slot
      case NSScrollerKnob:
      case NSScrollerKnobSlot:
        {
          float f = [sender floatValue];

          [self scrollColumnToVisible: GSRoundTowardsInfinity(f * _lastColumnLoaded)];
        }
        break;
      
      // NSScrollerNoPart ???
      default:
              break;
    }
}

- (void) scrollRowToVisible: (NSInteger)row inColumn: (NSInteger)column
{
  NSMatrix *matrix = [self matrixInColumn: column];

  [matrix scrollCellToVisibleAtRow: row
                            column: 1];
}

/*
 * Showing a horizontal scroller
 */

/**<p>Returns whether an NSScroller is used to scroll horizontally.
   By default a NSBrowser has a horizontal scroller.</p><p>See Also:
   -setHasHorizontalScroller:</p> 
*/
- (BOOL) hasHorizontalScroller
{
  return _hasHorizontalScroller;
}

/**<p>Sets whether an NSScroller is used to scroll horizontally. This method
   add the horizontal scroller, adjust the various subviews of the
   NSBrowser scroller  and marks self for display.By default a  NSBrowser
   has a horizontal scroller.</p>
   <p>See Also: -hasHorizontalScroller -tile</p>
 */
- (void) setHasHorizontalScroller: (BOOL)flag
{
  if (_hasHorizontalScroller != flag)
    {
      _hasHorizontalScroller = flag;
      if (!flag)
              [_horizontalScroller removeFromSuperview];
      else
        [self addSubview: _horizontalScroller];
      [self tile];
      [self setNeedsDisplay: YES];
    }
}


/*
 * Setting the behavior of arrow keys
 */

/** <p>Returns whether the arrow keys are enabled. By default YES.</p>
    <p>See Also: -setAcceptsArrowKeys:</p> */
- (BOOL) acceptsArrowKeys
{
  return _acceptsArrowKeys;
}

/** <p>Enables or disables the arrow keys as used for navigating within
    and between browsers. By default YES.</p>
    <p>See Also: -acceptsArrowKeys</p>
*/
- (void) setAcceptsArrowKeys: (BOOL)flag
{
  _acceptsArrowKeys = flag;
}

/** <p>Returns NO if pressing an arrow key only scrolls the browser, YES if
    it also sends the action message specified by [NSControl-setAction:].
    By default YES.</p><p>See Also: -setSendsActionOnArrowKeys:
    -acceptsArrowKeys [NSControl-setAction:] [NSControl-action] </p>
*/
- (BOOL) sendsActionOnArrowKeys
{
  return _sendsActionOnArrowKeys;
}

/** <p>Sets whether pressing an arrow key will cause the action message
    to be sent (in addition to causing scrolling). By default YES.</p>
    <p>See Also: -sendsActionOnArrowKeys -setAcceptsArrowKeys: 
    [NSControl-setAction:] [NSControl-action]</p>
*/
- (void) setSendsActionOnArrowKeys: (BOOL)flag
{
  _sendsActionOnArrowKeys = flag;
}

- (BOOL) allowsTypeSelect
{
  // FIXME
  return [self acceptsArrowKeys];
}

- (void) setAllowsTypeSelect: (BOOL)allowsTypeSelection
{
  // FIXME
  [self setAcceptsArrowKeys: allowsTypeSelection];
}

/*
 * Getting column frames
 */

/** <p>Returns the rectangle containing the column at index column.</p> */
- (NSRect) frameOfColumn: (NSInteger)column
{
  NSRect rect = NSZeroRect;
  NSSize bezelBorderSize = NSZeroSize;
  NSInteger n;

  if (browserUseBezels)
    bezelBorderSize = [[GSTheme theme] sizeForBorderType: NSBezelBorder];

  // Number of columns over from the first
  n = column - _firstVisibleColumn;

  // Calculate the frame
  rect.size = _columnSize;
  rect.origin.x = n * _columnSize.width;

  if (_separatesColumns)
    {
      rect.origin.x += n * browserColumnSeparation;
    }
  else if (!_separatesColumns && browserUseBezels)
    {
      if (column == _firstVisibleColumn)
        rect.origin.x += 2;
      else
        rect.origin.x += (n + 2);
    }

  // Adjust for horizontal scroller
  if (browserUseBezels)
    {
      if (_hasHorizontalScroller)
	{
	  if (_separatesColumns)
	    rect.origin.y = (scrollerWidth - 1) + (2 * bezelBorderSize.height) + 
	      browserVerticalPadding;
	  else
	    rect.origin.y = scrollerWidth + bezelBorderSize.width;
	}
      else if (!_separatesColumns)
        {
          rect.origin.y += bezelBorderSize.width;
        }
    }
  else
    {
      if (_hasHorizontalScroller)
	rect.origin.y = scrollerWidth;
    }

  // Padding : _columnSize.width is rounded in "tile" method
  if (column == _lastVisibleColumn)
    {
      if (_separatesColumns)
        rect.size.width = _frame.size.width - rect.origin.x;
      else 
        rect.size.width = _frame.size.width -
          (rect.origin.x + bezelBorderSize.width);

      // FIXME: Assumes left-side scrollers
      if ([[GSTheme theme] scrollViewScrollersOverlapBorders])
	{
	  rect.size.width -= 1;
	}
    }

  if (rect.size.width < 0)
    {
      rect.size.width = 0;
    }
  if (rect.size.height < 0)
    {
      rect.size.height = 0;
    }

  return rect;
}

/** Returns the rectangle containing the column at index column, */
// not including borders.
- (NSRect) frameOfInsideOfColumn: (NSInteger)column
{
  // xxx what does this one do?
  return [self frameOfColumn: column];
}

+ (void) removeSavedColumnsWithAutosaveName: (NSString *)name
{
  [[NSUserDefaults standardUserDefaults] removeObjectForKey: name];
}

- (NSString *) columnsAutosaveName
{
  return _columnsAutosaveName;
}

- (void) setColumnsAutosaveName: (NSString *)name
{
  // FIXME: More to do. The whole column width saving is missing!
  ASSIGN(_columnsAutosaveName, name);
}

/*
 * Arranging browser components
 */

/** Adjusts the various subviews of NSBrowser-scrollers, columns,
    titles, and so on-without redrawing. Your code shouldn't send this
    message.  It's invoked any time the appearance of the NSBrowser
    changes.
 */
- (void) tile
{
  NSSize bezelBorderSize = NSZeroSize;
  NSInteger i, num, columnCount, delta;
  CGFloat frameWidth;
  const BOOL overlapBorders = [[GSTheme theme] scrollViewScrollersOverlapBorders];
  const BOOL useBottomCorner = [[GSTheme theme] scrollViewUseBottomCorner];

  if (browserUseBezels)
    bezelBorderSize = [[GSTheme theme] sizeForBorderType: NSBezelBorder];

  _columnSize.height = _frame.size.height;
  
  // Titles (there is no real frames to resize)
  if (_isTitled)
    {
      _columnSize.height -= [self titleHeight] + browserVerticalPadding;
    }

  // Horizontal scroller
  if (_hasHorizontalScroller)
    {
      const CGFloat scrollerHightReduction = browserUseBezels ? 1 : 0;

      _scrollerRect.origin.x = bezelBorderSize.width;
      _scrollerRect.origin.y = bezelBorderSize.height - scrollerHightReduction;
      _scrollerRect.size.width = (_frame.size.width - 
                                  (2 * bezelBorderSize.width));
      _scrollerRect.size.height = scrollerWidth;
      
      if (_separatesColumns)
        _columnSize.height -= (scrollerWidth - scrollerHightReduction) + 
          (2 * bezelBorderSize.height) + browserVerticalPadding;
      else
        _columnSize.height -= scrollerWidth + (2 * bezelBorderSize.height);
      
      // "Bottom corner" box
      if (!browserUseBezels && !useBottomCorner)
	{
	  _scrollerRect.origin.x += scrollerWidth;
	  _scrollerRect.size.width -= scrollerWidth;
	}

      /** Horizontall expand the scroller by GSScrollerKnobOvershoot on the left */
      if (overlapBorders)
	{
	  // FIXME: Assumes left scroller
	  _scrollerRect.origin.x -= 1;
	  _scrollerRect.size.width += 1;
	}

      if (!NSEqualRects(_scrollerRect, [_horizontalScroller frame]))
        {
          [_horizontalScroller setFrame: _scrollerRect];
        }
    }
  else
    {
      _scrollerRect = NSZeroRect;
      if (!_separatesColumns)
        _columnSize.height -= 2 * bezelBorderSize.width;
    }

  if (_columnSize.height < 0)
    _columnSize.height = 0;
  
  num = _lastVisibleColumn - _firstVisibleColumn + 1;

  // Column count
  if (_minColumnWidth > 0)
    {
      CGFloat colWidth = _minColumnWidth + scrollerWidth;

      if (_separatesColumns)
        colWidth += browserColumnSeparation;

      if (_frame.size.width > colWidth)
        {
          columnCount = (int)(_frame.size.width / colWidth);
        }
      else
        columnCount = 1;
    }
  else
    columnCount = num;

  if (_maxVisibleColumns > 0 && columnCount > _maxVisibleColumns)
    columnCount = _maxVisibleColumns;

  // Create extra columns
  if (columnCount != num)
    {
      if (num > 0)
        delta = columnCount - num;
      else
        delta = columnCount - 1;

      if ((delta > 0) && (_lastVisibleColumn <= _lastColumnLoaded))
        {
          _firstVisibleColumn = (_firstVisibleColumn - delta > 0) ?
            _firstVisibleColumn - delta : 0;
        }

      for (i = [_browserColumns count]; i < columnCount; i++)
        [self _createColumn];

      _lastVisibleColumn = _firstVisibleColumn + columnCount - 1;
    }

  // Column width
  if (_separatesColumns)
    frameWidth = _frame.size.width - ((columnCount - 1) * browserColumnSeparation);
  else
    frameWidth = _frame.size.width - ((columnCount - 1) + 
                                      (2 * bezelBorderSize.width));

  _columnSize.width = (int)(frameWidth / (CGFloat)columnCount);

  for (i = _firstVisibleColumn; i <= _lastVisibleColumn; i++)
    {
      NSBrowserColumn *bc;
      NSScrollView *sc;
      NSMatrix *matrix;

      // FIXME: in some cases the column is not loaded
      while (i >= [_browserColumns count]) [self _createColumn];

      bc = [_browserColumns objectAtIndex: i];

      if (!(sc = [bc columnScrollView]))
        {
          NSLog(@"NSBrowser error, sc != [bc columnScrollView]");
          return;
        }

      [sc setBorderType: [self _resolvedBorderType]];
      [sc setFrame: [self frameOfColumn: i]];
      matrix = [bc columnMatrix];
      
      // Adjust matrix to fit in scrollview if column has been loaded
      if (matrix && [bc isLoaded])
        {
          NSSize cs, ms;
          
          cs = [sc contentSize];
          ms = [matrix cellSize];
          ms.width = cs.width;
          [matrix setCellSize: ms];
          [sc setDocumentView: matrix];
        }
    }

  if (columnCount != num)
    {
      [self updateScroller];
      [self _remapColumnSubviews: YES];
      //      [self _setColumnTitlesNeedDisplay];  
      [self setNeedsDisplay: YES];
    }
}

/** Override from NSControl. Don't do anything to change the size of the 
    browser.  */
- (void) sizeToFit
{
}

/*
 * Setting the delegate
 */

/** <p>Returns the NSBrowser's delegate.</p>
 *<p>See Also: -setDelegate:</p> */
- (id) delegate
{
  return _browserDelegate;
}

/**
 * <p>Sets the delegate of the receiver.
 * If not nil, the delegate must either be passive and respond to
 * [NSObject-browser:numberOfRowsInColumn:] or be active and respond to
 * [NSObject-browser:createRowsForColumn:inMatrix:] but not both.  
 * If the delegate is active it must also respond to
 * [NSObject-browser:willDisplayCell:atRow:column:].  
 * If the delegate is not nil but does not meet these conditions,
 * an NSBrowserIllegalDelegateException will be raised.</p>
 *<p>See Also: -delegate</p>
 */
- (void) setDelegate: (id)anObject
{
  BOOL flag = NO;

  /* Default to YES for nil delegate.  */
  _passiveDelegate = YES;

  if ([anObject respondsToSelector: 
                  @selector(browser:numberOfRowsInColumn:)])
    {
      flag = YES;
      if (![anObject respondsToSelector: 
                         @selector(browser:willDisplayCell:atRow:column:)])
        [NSException raise: NSBrowserIllegalDelegateException
                     format: @"(Passive) Delegate does not respond to %s\n",
                     GSNameFromSelector
                       (@selector(browser:willDisplayCell:atRow:column:))];
    }

  if ([anObject respondsToSelector: 
                  @selector(browser:createRowsForColumn:inMatrix:)])
    {
      _passiveDelegate = NO;

      /* If flag is already set
         then the delegate must respond to both methods.  */
      if (flag)
        {
          [NSException raise: NSBrowserIllegalDelegateException
                       format: @"Delegate responds to both %s and %s\n",
                       GSNameFromSelector
                         (@selector(browser:numberOfRowsInColumn:)),
                       GSNameFromSelector
                         (@selector(browser:createRowsForColumn:inMatrix:))];
        }

      flag = YES;
    }

  if (!flag && anObject)
    [NSException raise: NSBrowserIllegalDelegateException
                 format: @"Delegate does not respond to %s or %s\n",
                 GSNameFromSelector
                   (@selector(browser:numberOfRowsInColumn:)),
                 GSNameFromSelector
                   (@selector(browser:createRowsForColumn:inMatrix:))];

  _browserDelegate = anObject;
}


/*
 * Target and action
 */

/** <p>Returns the NSBrowser's double-click action method.</p>
 <p>See Also: -setDoubleAction: </p> 
*/
- (SEL) doubleAction
{
  return _doubleAction;
}

/**<p> Sets the NSBrowser's double-click action to aSelector.</p>
 <p>See Also: -doubleAction</p> 
*/
- (void) setDoubleAction: (SEL)aSelector
{
  _doubleAction = aSelector;
}

/** Sends the action message to the target. Returns YES upon success, 
    NO if no target for the message could be found. */
- (BOOL) sendAction
{
  return [self sendAction: [self action]  to: [self target]];
}


/*
 * Event handling
 */

/**<p> Responds to (single) mouse clicks in a column of the NSBrowser.</p>
   <p>See Also: -doDoubleClick:</p>
*/
- (void) doClick: (id)sender
{
  NSArray        *array;
  NSMutableArray *selectedCells;
  NSEnumerator   *enumerator;
  NSBrowserCell  *cell;
  NSInteger       column, aCount, selectedCellsCount;

  if ([sender class] != _browserMatrixClass)
    return;

  column = [self columnOfMatrix: sender];
  // If the matrix isn't ours then just return
  if (column < 0 || column > _lastColumnLoaded)
    return;
    
  array = [sender selectedCells];
  aCount = [array count];
  if (aCount == 0)
    return;

  selectedCells = [array mutableCopy];

  enumerator = [array objectEnumerator];
  while ((cell = [enumerator nextObject]))
    {
      if (_allowsBranchSelection == NO && [cell isLeaf] == NO)
        {
          [selectedCells removeObject: cell];
        }
    }

  if ([selectedCells count] == 0 && [sender selectedCell] != nil)
    [selectedCells addObject: [sender selectedCell]];

  selectedCellsCount = [selectedCells count];

  /* If some branch cells were selected but branch selection is not allowed
     reset the selection and select only the leaf cells. It is a pity that
     we cannot deselect cells individually. */
  if (selectedCellsCount != aCount)
    {
      BOOL autoscroll = [sender isAutoscroll];

      /* Note: Temporarily disable autoscrolling to prevent bug #18881 */
      [sender setAutoscroll: NO];
      [sender deselectAllCells];
      enumerator = [selectedCells objectEnumerator];
      while ((cell = [enumerator nextObject]))
        [sender selectCell: cell];
      [sender setAutoscroll: autoscroll];
    }

  [self setLastColumn: column];
  // Single selection
  if (selectedCellsCount == 1)
    {
      cell = [selectedCells objectAtIndex: 0];

      // If the cell is not a leaf we need to load a column
      if (![cell isLeaf])
        {
          [self addColumn];
        }

      [sender scrollCellToVisibleAtRow: [sender selectedRow] column: 0];
    }

  [self updateScroller];

  // Send the action to target
  [self sendAction];

  RELEASE(selectedCells);
}

/** <p>Responds to double-clicks in a column of the NSBrowser.</p>
    <p>See Also: -doClick: -sendAction:to:</p>
*/
- (void) doDoubleClick: (id)sender
{
  // We have already handled the single click
  // so send the double action

  [self sendAction: _doubleAction to: [self target]];
}

- (NSInteger) clickedColumn
{
  // FIXME: Return column number from doClick:
  return -1;
}

- (NSInteger) clickedRow
{
  // FIXME: Return row number from doClick:
  return -1;
}

+ (void) _themeDidActivate: (NSNotification*)n
{
  GSTheme *theme = [GSTheme theme];
  scrollerWidth = [NSScroller scrollerWidth];
  browserColumnSeparation = [theme browserColumnSeparation];
  browserVerticalPadding = [theme browserVerticalPadding];
  browserUseBezels = [theme browserUseBezels];
}

+ (void) initialize
{
  if (self == [NSBrowser class])
    {
      [[NSNotificationCenter defaultCenter] addObserver: self
	selector: @selector(_themeDidActivate:)
	name: GSThemeDidActivateNotification
	object: nil];

      // Initial version
      [self setVersion: 1];
      
      /* Create the shared titleCell if it hasn't been created already. */
      if (!titleCell)
        {
          titleCell = [GSBrowserTitleCell new];
        }
      
      [self _themeDidActivate: nil];
    }
}

/*
 * Override superclass methods
 */

/** Setups browser with frame 'rect'. */
- (id) initWithFrame: (NSRect)rect
{
  NSSize bs;

  if ((self = [super initWithFrame: rect]) == nil)
    {
      return nil;
    }

  // Class setting
  _browserCellPrototype = [[[NSBrowser cellClass] alloc] init];
  _browserMatrixClass = [NSMatrix class];
  
  // Default values
  _pathSeparator = @"/";
  _allowsBranchSelection = YES;
  _allowsEmptySelection = YES;
  _allowsMultipleSelection = YES;
  _reusesColumns = NO;
  _separatesColumns = YES;
  _isTitled = YES;
  _takesTitleFromPreviousColumn = YES;
  _hasHorizontalScroller = YES;
  _isLoaded = NO;
  _acceptsArrowKeys = YES;
  _acceptsAlphaNumericalKeys = YES;
  _lastKeyPressed = 0.;
  _charBuffer = nil;
  _sendsActionOnArrowKeys = YES;
  _sendsActionOnAlphaNumericalKeys = YES;
  _browserDelegate = nil;
  _passiveDelegate = YES;
  _doubleAction = NULL;  
  // FIXME: Seems a bit wrong to look at the current theme here
  bs = NSZeroSize;
  if (browserUseBezels)
    bs = [[GSTheme theme] sizeForBorderType: NSBezelBorder];
  _minColumnWidth = scrollerWidth + (2 * bs.width);
  if (_minColumnWidth < 100.0)
    _minColumnWidth = 100.0;

  // Horizontal scroller
  _scrollerRect.origin.x = bs.width;
  _scrollerRect.origin.y = bs.height;
  _scrollerRect.size.width = _frame.size.width - (2 * bs.width);
  _scrollerRect.size.height = scrollerWidth;
  _horizontalScroller = [[NSScroller alloc] initWithFrame: _scrollerRect];
  [_horizontalScroller setTarget: self];
  [_horizontalScroller setAction: @selector(scrollViaScroller:)];
  [self addSubview: _horizontalScroller];
  _skipUpdateScroller = NO;

  // Columns
  _browserColumns = [[NSMutableArray alloc] init];

  // Create a single column
  _lastColumnLoaded = -1;
  _firstVisibleColumn = 0;
  _lastVisibleColumn = 0;
  _maxVisibleColumns = 3;
  [self _createColumn];

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_themeDidActivate:)
    name: GSThemeDidActivateNotification
    object: nil];

  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];

  if ([titleCell controlView] == self)
    {
      [titleCell setControlView: nil];
    }

  RELEASE(_browserCellPrototype);
  RELEASE(_pathSeparator);
  RELEASE(_horizontalScroller);
  RELEASE(_browserColumns);
  TEST_RELEASE(_charBuffer);

  [super dealloc];
}



/*
 * Target-actions
 */

/** Set target to 'target' */
- (void) setTarget: (id)target
{
  _target = target;
}

/** Return current target. */
- (id) target
{
  return _target;
}

/** Set action to 's'. */
- (void) setAction: (SEL)s
{
  _action = s;
}

/** Return current action. */
- (SEL) action
{
  return _action;
}



/*
 * Events handling 
 */

- (void) drawRect: (NSRect)rect
{
  [[GSTheme theme] drawBrowserRect: rect
		   inView: self
		   withScrollerRect: _scrollerRect
		   columnSize: _columnSize];
}

/* Informs the receivers's subviews that the receiver's bounds
 rectangle size has changed from oldFrameSize. */
- (void) resizeSubviewsWithOldSize: (NSSize)oldSize
{
  [self tile];
}


/* Override NSControl handler (prevents highlighting). */
- (void) mouseDown: (NSEvent *)theEvent
{
}

- (void) moveLeft: (id)sender
{
  if (_acceptsArrowKeys)
    {
      NSMatrix *matrix;
      NSInteger selectedColumn;

      matrix = (NSMatrix *)[_window firstResponder];
      selectedColumn = [self columnOfMatrix:matrix];
      if (selectedColumn == -1)
        {
          selectedColumn = [self selectedColumn];
          matrix = [self matrixInColumn: selectedColumn];
        }
      if (selectedColumn > 0)
        {
          [matrix deselectAllCells];
          [matrix scrollCellToVisibleAtRow:0 column:0];
          [self setLastColumn: selectedColumn];

          selectedColumn--;
          [self scrollColumnToVisible: selectedColumn];
          matrix = [self matrixInColumn: selectedColumn];
          [_window makeFirstResponder: matrix];

          if (_sendsActionOnArrowKeys == YES)
            {
              [super sendAction: _action to: _target];
            }
        }
    }
}

- (void) moveRight: (id)sender
{
  if (_acceptsArrowKeys)
    {
      NSMatrix *matrix;
      NSInteger selectedColumn;

      matrix = (NSMatrix *)[_window firstResponder];
      selectedColumn = [self columnOfMatrix:matrix];
      if (selectedColumn == -1)
        {
          selectedColumn = [self selectedColumn];
          matrix = [self matrixInColumn: selectedColumn];
        }
      if (selectedColumn == -1)
        {
          matrix = [self matrixInColumn: 0];

          if ([[matrix cells] count])
            {
              [matrix selectCellAtRow: 0 column: 0];
            }
        }
      else
        {
          // if there is one selected cell and it is a leaf, move right
          // (column is already loaded)
          if (![[matrix selectedCell] isLeaf]
              && [[matrix selectedCells] count] == 1)
            {
              selectedColumn++;
              matrix = [self matrixInColumn: selectedColumn];
              if ([[matrix cells] count] && [matrix selectedCell] == nil)
                {
                  [matrix selectCellAtRow: 0 column: 0];
                }
              // if selected cell is a leaf, we need to add a column
              if (![[matrix selectedCell] isLeaf]
                  && [[matrix selectedCells] count] == 1)
                {  
                  [self addColumn];
                }
            }
        }

      [_window makeFirstResponder: matrix];

      if (_sendsActionOnArrowKeys == YES)
        {
          [super sendAction: _action to: _target];
        }
    }
}

- (void) keyDown: (NSEvent *)theEvent
{
  NSString *characters = [theEvent characters];
  unichar character = 0;

  if ([characters length] > 0)
    {
      character = [characters characterAtIndex: 0];
    }

  if (_acceptsArrowKeys)
    {
      switch (character)
        {
        case NSUpArrowFunctionKey:
        case NSDownArrowFunctionKey:
          return;
        case NSLeftArrowFunctionKey:
          [self moveLeft:self];
          return;
        case NSRightArrowFunctionKey:
          [self moveRight:self];
          return;
        case NSBackTabCharacter:
          [_window selectKeyViewPrecedingView: self];
          return;
        case NSTabCharacter:
          {
            if ([theEvent modifierFlags] & NSShiftKeyMask)
              {
                [_window selectKeyViewPrecedingView: self];
              }
            else
              {
                [_window selectKeyViewFollowingView: self];
              }
          }
          return;
        }
    }

  if (_acceptsAlphaNumericalKeys && (character < 0xF700)
       && ([characters length] > 0))
    {
      NSMatrix *matrix;
      NSString *sv;
      NSInteger i, n, s;
      NSInteger match;
      NSInteger selectedColumn;
      SEL lcarcSel = @selector(loadedCellAtRow:column:);
      IMP lcarc = [self methodForSelector: lcarcSel];

      selectedColumn = [self selectedColumn];
      if (selectedColumn != -1)
        {
          matrix = [self matrixInColumn: selectedColumn];
          n = [matrix numberOfRows];
          s = [matrix selectedRow];
          
          if (!_charBuffer)
            {
              _charBuffer = [characters substringToIndex: 1];
              RETAIN(_charBuffer);
            }
          else
            {
              if (([theEvent timestamp] - _lastKeyPressed < 2000.0)
                  && (_alphaNumericalLastColumn == selectedColumn))
                {
                  NSString *transition;
                  transition = [_charBuffer 
                                 stringByAppendingString:
                                   [characters substringToIndex: 1]];
                  RELEASE(_charBuffer);
                  _charBuffer = transition;
                  RETAIN(_charBuffer);
                }
              else
                {
                  RELEASE(_charBuffer);
                  _charBuffer = [characters substringToIndex: 1];
                  RETAIN(_charBuffer);
                }
            }

          _alphaNumericalLastColumn = selectedColumn;
          _lastKeyPressed = [theEvent timestamp];
          
          sv = [((*lcarc)(self, lcarcSel, s, selectedColumn))
                 stringValue];

          if (([sv length] > 0)
              && ([sv hasPrefix: _charBuffer]))
            return;

          match = -1;
          for (i = s + 1; i < n; i++)
            {
              sv = [((*lcarc)(self, lcarcSel, i, selectedColumn))
                     stringValue];
              if (([sv length] > 0)
                  && ([sv hasPrefix: _charBuffer]))
                {
                  match = i;
                  break;
                }
            }
          if (i == n)
            {
              for (i = 0; i < s; i++)
                {
                  sv = [((*lcarc)(self, lcarcSel, i, selectedColumn))
                         stringValue];
                  if (([sv length] > 0)
                      && ([sv hasPrefix: _charBuffer]))
                    {
                      match = i;
                      break;
                    }
                }
            }
          if (match != -1)
            {
              [matrix deselectAllCells];
              [self selectRow: match
                inColumn: selectedColumn];
              [matrix scrollCellToVisibleAtRow: match column: 0];
              [matrix performClick: self];
              return;
            }
        }
      _lastKeyPressed = 0.;
    }

  [super keyDown: theEvent];
}

/*
 * NSCoding protocol
 *
 * We do not encode most of the instance variables except the Browser columns
 * because they are internal objects (though not transportable). So we just
 * encode enoguh information to rebuild identical columns on the decoder
 * side. Same for the Horizontal Scroller
 */

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      long flags = 0;

      //
      // NOTE: The browserview under GS uses an NSMatrix subview, the one under
      // Cocoa does not.   This will cause IB to issue an "inconsistency" alert
      // which is minor and nothing to worry about.
      //
      [aCoder encodeObject: _browserCellPrototype forKey: @"NSCellPrototype"];
      [aCoder encodeObject: [self _getTitleOfColumn: 0] forKey: @"NSFirstColumnTitle"];
      [aCoder encodeObject: _pathSeparator forKey: @"NSPathSeparator"];

      flags |= [self hasHorizontalScroller] ? 0x10000 : 0;
      flags |= ([self allowsEmptySelection] == NO) ? 0x20000 : 0;
      flags |= [self sendsActionOnArrowKeys] ? 0x40000 : 0;
      flags |= [self acceptsArrowKeys] ? 0x100000 : 0;
      flags |= [self separatesColumns] ? 0x4000000 : 0;
      flags |= [self takesTitleFromPreviousColumn] ? 0x8000000 : 0;
      flags |= [self isTitled] ? 0x10000000 : 0;
      flags |= [self reusesColumns] ? 0x20000000 : 0;
      flags |= [self allowsBranchSelection] ? 0x40000000 : 0;
      flags |= [self allowsMultipleSelection] ? 0x80000000 : 0;
      [aCoder encodeInt: flags forKey: @"NSBrFlags"];

      [aCoder encodeInt: _maxVisibleColumns forKey: @"NSNumberOfVisibleColumns"];
      [aCoder encodeInt: _minColumnWidth forKey: @"NSMinColumnWidth"];

      [aCoder encodeInt: _columnResizing forKey: @"NSColumnResizingType"];
      //[aCoder encodeInt: prefWidth forKey: @"NSPreferedColumnWidth"];
      if (nil != [self columnsAutosaveName])
        {
          [aCoder encodeObject: [self columnsAutosaveName] forKey: @"NSColumnsAutosaveName"];
        }
    }
  else
    {
      // Here to keep compatibility with old version
      [aCoder encodeObject: nil];
      [aCoder encodeObject:_browserCellPrototype];
      [aCoder encodeObject: NSStringFromClass (_browserMatrixClass)];
      
      [aCoder encodeObject:_pathSeparator];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isLoaded];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsBranchSelection];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsEmptySelection];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_allowsMultipleSelection];
      [aCoder encodeValueOfObjCType: @encode(int) at: &_maxVisibleColumns];
      [aCoder encodeValueOfObjCType: @encode(CGFloat) at: &_minColumnWidth];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_reusesColumns];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_separatesColumns];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_takesTitleFromPreviousColumn];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isTitled];
      
      
      [aCoder encodeObject:_horizontalScroller];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasHorizontalScroller];
      [aCoder encodeRect: _scrollerRect];
      [aCoder encodeSize: _columnSize];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_acceptsArrowKeys];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_sendsActionOnArrowKeys];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_acceptsAlphaNumericalKeys];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_sendsActionOnAlphaNumericalKeys];
      
      [aCoder encodeConditionalObject:_browserDelegate];
      
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_doubleAction];
      [aCoder encodeConditionalObject: _target];
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_action];
      
      [aCoder encodeObject: _browserColumns];
      
      // Just encode the number of columns and the first visible
      // and rebuild the browser columns on the decoding side
      {
        int colCount = [_browserColumns count];  
        [aCoder encodeValueOfObjCType: @encode(int) at: &colCount];
        [aCoder encodeValueOfObjCType: @encode(int) at: &_firstVisibleColumn];
      }
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if ([aDecoder allowsKeyedCoding])
    {
      NSCell *proto = [aDecoder decodeObjectForKey: @"NSCellPrototype"];
      NSString *title = [aDecoder decodeObjectForKey: @"NSFirstColumnTitle"];
      NSString *sep = [aDecoder decodeObjectForKey: @"NSPathSeparator"];
      long flags;
      NSSize bs;
      
      // Class setting
      _browserCellPrototype = [[[NSBrowser cellClass] alloc] init];
      _browserMatrixClass = [NSMatrix class];
      
      // Default values
      _pathSeparator = @"/";
      _allowsBranchSelection = YES;
      _allowsEmptySelection = YES;
      _allowsMultipleSelection = YES;
      _reusesColumns = NO;
      _separatesColumns = YES;
      _isTitled = YES;
      _takesTitleFromPreviousColumn = YES;
      _hasHorizontalScroller = YES;
      _isLoaded = NO;
      _acceptsArrowKeys = YES;
      _acceptsAlphaNumericalKeys = YES;
      _lastKeyPressed = 0.;
      _charBuffer = nil;
      _sendsActionOnArrowKeys = YES;
      _sendsActionOnAlphaNumericalKeys = YES;
      _browserDelegate = nil;
      _passiveDelegate = YES;
      _doubleAction = NULL;  
      // FIXME: Seems a bit wrong to look at the current theme here
      bs = NSZeroSize;
      if (browserUseBezels)
	bs = [[GSTheme theme] sizeForBorderType: NSBezelBorder];
      _minColumnWidth = scrollerWidth + (2 * bs.width);
      if (_minColumnWidth < 100.0)
        _minColumnWidth = 100.0;
      
      // Horizontal scroller
      _scrollerRect.origin.x = bs.width;
      _scrollerRect.origin.y = bs.height;
      _scrollerRect.size.width = _frame.size.width - (2 * bs.width);
      _scrollerRect.size.height = scrollerWidth;
      _horizontalScroller = [[NSScroller alloc] initWithFrame: _scrollerRect];
      [_horizontalScroller setTarget: self];
      [_horizontalScroller setAction: @selector(scrollViaScroller:)];
      [self addSubview: _horizontalScroller];
      _skipUpdateScroller = NO;
      
      // Columns
      _browserColumns = [[NSMutableArray alloc] init];
      
      // Create a single column
      _lastColumnLoaded = -1;
      _firstVisibleColumn = 0;
      _lastVisibleColumn = 0;
      _maxVisibleColumns = 3;
      [self _createColumn];
      // end //            
      
      [self setCellPrototype: proto];
      [self setPathSeparator: sep];
      [self setTitle: title ofColumn: 0];

      if ([aDecoder containsValueForKey: @"NSBrFlags"])
        {
          flags = [aDecoder decodeIntForKey: @"NSBrFlags"];

          [self setHasHorizontalScroller: ((flags & 0x10000) == 0x10000)];
          [self setAllowsEmptySelection: !((flags & 0x20000) == 0x20000)];
          [self setSendsActionOnArrowKeys: ((flags & 0x40000) == 0x40000)];
          [self setAcceptsArrowKeys: ((flags & 0x100000) == 0x100000)];
          [self setSeparatesColumns: ((flags & 0x4000000) == 0x4000000)];
          [self setTakesTitleFromPreviousColumn: ((flags & 0x8000000) == 0x8000000)];
          [self setTitled: ((flags & 0x10000000) == 0x10000000)];
          [self setReusesColumns: ((flags & 0x20000000) == 0x20000000)];
          [self setAllowsBranchSelection: ((flags & 0x40000000) == 0x40000000)];
          [self setAllowsMultipleSelection: ((flags & 0x80000000) == 0x80000000)];
        }

      if ([aDecoder containsValueForKey: @"NSNumberOfVisibleColumns"])
        {
          [self setMaxVisibleColumns: [aDecoder decodeIntForKey: 
                                                  @"NSNumberOfVisibleColumns"]];
        }

      if ([aDecoder containsValueForKey: @"NSMinColumnWidth"])
        {
          [self setMinColumnWidth: [aDecoder decodeIntForKey: @"NSMinColumnWidth"]];
        }

      if ([aDecoder containsValueForKey: @"NSColumnResizingType"])
        {
          [self setColumnResizingType: [aDecoder decodeIntForKey: @"NSColumnResizingType"]];
        }

      if ([aDecoder containsValueForKey: @"NSPreferedColumnWidth"])
        {
          //int prefWidth = [aDecoder decodeIntForKey: @"NSPreferedColumnWidth"];
        }

      if ([aDecoder containsValueForKey: @"NSColumnsAutosaveName"])
        {
          [self setColumnsAutosaveName: [aDecoder decodeObjectForKey:
                                                    @"NSColumnsAutosaveName"]];
        }
    }
  else
    {
      int colCount;
      
      // Here to keep compatibility with old version
      [aDecoder decodeObject];
      _browserCellPrototype = RETAIN([aDecoder decodeObject]);
      _browserMatrixClass   = NSClassFromString ((NSString *)[aDecoder decodeObject]);
      
      [self setPathSeparator: [aDecoder decodeObject]];
      
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isLoaded];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsBranchSelection];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsEmptySelection];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_allowsMultipleSelection];
      [aDecoder decodeValueOfObjCType: @encode(int) at: &_maxVisibleColumns];
      [aDecoder decodeValueOfObjCType: @encode(CGFloat) at: &_minColumnWidth];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_reusesColumns];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_separatesColumns];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_takesTitleFromPreviousColumn];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isTitled];
      
      //NSBox *_horizontalScrollerBox;
      _horizontalScroller = RETAIN([aDecoder decodeObject]);
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasHorizontalScroller];
      _scrollerRect = [aDecoder decodeRect];
      _columnSize = [aDecoder decodeSize];

      _skipUpdateScroller = NO;
      /*
        _horizontalScroller = [[NSScroller alloc] initWithFrame: _scrollerRect];
        [_horizontalScroller setTarget: self];
        [_horizontalScroller setAction: @selector(scrollViaScroller:)];
      */
      [self setHasHorizontalScroller: _hasHorizontalScroller];

      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_acceptsArrowKeys];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_sendsActionOnArrowKeys];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_acceptsAlphaNumericalKeys];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_sendsActionOnAlphaNumericalKeys];
      _lastKeyPressed = 0;
      _charBuffer = nil;
      // Skip: int _alphaNumericalLastColumn;

      _browserDelegate = [aDecoder decodeObject];
      if (_browserDelegate != nil)
        [self setDelegate:_browserDelegate];
      else
        _passiveDelegate = YES;
      
      [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_doubleAction];
      _target = [aDecoder decodeObject];
      [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_action];
      
      // Do the minimal thing to initiate the browser...
      /*
        _lastColumnLoaded = -1;
        _firstVisibleColumn = 0;
        _lastVisibleColumn = 0;
        [self _createColumn];
      */
      _browserColumns = RETAIN([aDecoder decodeObject]);
      // ..and rebuild any existing browser columns
      [aDecoder decodeValueOfObjCType: @encode(int) at: &colCount];
      [aDecoder decodeValueOfObjCType: @encode(int) at: &_firstVisibleColumn];
    }
      
  // Display even if there isn't any column
  _isLoaded = NO;
  [self tile];

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_themeDidActivate:)
    name: GSThemeDidActivateNotification
    object: nil];

  return self;
}



/*
 * Div.
 */

- (BOOL) isOpaque
{
  // NSBrowser used to be opaque but may not be due to themes;
  // e.g. if the header tile images are not opaque.
  return NO;
}

@end

@implementation NSBrowser (GNUstepExtensions)
/*
 * Setting the behavior of arrow keys
 */

/** Returns YES if the alphanumerical keys are enabled. */
- (BOOL) acceptsAlphaNumericalKeys
{
  return _acceptsAlphaNumericalKeys;
}

/** Enables or disables the arrow keys as used for navigating within
    and between browsers. */
- (void) setAcceptsAlphaNumericalKeys: (BOOL)flag
{
  _acceptsAlphaNumericalKeys = flag;
}

/** Returns NO if pressing an arrow key only scrolls the browser, YES if 
    it also sends the action message specified by setAction:. */
- (BOOL) sendsActionOnAlphaNumericalKeys
{
  return _sendsActionOnAlphaNumericalKeys;
}

/** Sets whether pressing an arrow key will cause the action message 
    to be sent (in addition to causing scrolling). */
- (void) setSendsActionOnAlphaNumericalKeys: (BOOL)flag
{
  _sendsActionOnAlphaNumericalKeys = flag;
}

@end


/*
 *
 *  PRIVATE METHODS
 *
 */
@implementation NSBrowser (Private)

- (void) _remapColumnSubviews: (BOOL)fromFirst
{
  NSBrowserColumn *bc;
  NSScrollView *sc;
  NSUInteger i, count;
  id firstResponder = nil;
  BOOL setFirstResponder = NO;

  // Removes all column subviews.
  count = [_browserColumns count];
  for (i = 0; i < count; i++)
    {
      bc = [_browserColumns objectAtIndex: i];
      sc = [bc columnScrollView];

      if (!firstResponder && [bc columnMatrix] == [_window firstResponder])
        {
          firstResponder = [bc columnMatrix];
        }
      if (sc)
        {
          [sc removeFromSuperviewWithoutNeedingDisplay];
        }
    }

  if (_firstVisibleColumn > _lastVisibleColumn)
    return;

  // Sets columns subviews order according to fromFirst (display order...).
  // All added subviews are automaticaly marked as needing display (->
  // NSView).
  if (fromFirst)
    {
      for (i = _firstVisibleColumn; i <= _lastVisibleColumn; i++)
        {
          bc = [_browserColumns objectAtIndex: i];
          sc = [bc columnScrollView];
          [self addSubview: sc];

          if ([bc columnMatrix] == firstResponder)
            {
              [_window makeFirstResponder: firstResponder];
              setFirstResponder = YES;
            }
        }

      if (firstResponder && setFirstResponder == NO)
        {
          [_window makeFirstResponder:
                     [[_browserColumns objectAtIndex: _firstVisibleColumn]
                       columnMatrix]];
        }
    }
  else
    {
      for (i = _lastVisibleColumn; i >= _firstVisibleColumn; i--)
        {
          bc = [_browserColumns objectAtIndex: i];
          sc = [bc columnScrollView];
          [self addSubview: sc];

          if ([bc columnMatrix] == firstResponder)
            {
              [_window makeFirstResponder: firstResponder];
              setFirstResponder = YES;
            }
        }

      if (firstResponder && setFirstResponder == NO)
        {
          [_window makeFirstResponder:
                     [[_browserColumns objectAtIndex: _lastVisibleColumn]
                       columnMatrix]];
        }
    }
}

/* Loads column 'column' (asking the delegate). */
- (void) _performLoadOfColumn: (NSInteger)column
{
  NSBrowserColumn *bc;
  NSScrollView *sc;
  NSMatrix *matrix;
  NSInteger i, rows, cols;

  if (_passiveDelegate)
    {
      // Ask the delegate for the number of rows
      rows = [_browserDelegate browser: self numberOfRowsInColumn: column];
      cols = 1;
    }
  else
    {
      rows = 0;
      cols = 0;
    }

  bc = [_browserColumns objectAtIndex: column];

  if (!(sc = [bc columnScrollView]))
    return;

  matrix = [bc columnMatrix];

  if (_reusesColumns && matrix)
    {
      [matrix renewRows: rows columns: cols];

      // Mark all the cells as unloaded
      for (i = 0; i < rows; i++)
        {
          [[matrix cellAtRow: i column: 0] setLoaded: NO];
        }
    }
  else
    {
      NSRect matrixRect = {{0, 0}, {100, 100}};
      NSSize matrixIntercellSpace = {0, 0};

      // create a new col matrix
      matrix = [[_browserMatrixClass alloc]
                   initWithFrame: matrixRect
                   mode: NSListModeMatrix
                   prototype: _browserCellPrototype
                   numberOfRows: rows
                   numberOfColumns: cols];
      [matrix setIntercellSpacing: matrixIntercellSpace];
      [matrix setAllowsEmptySelection: _allowsEmptySelection];
      [matrix setAutoscroll: YES];

      // Set up background colors.
      [matrix setBackgroundColor: [self backgroundColor]];
      [matrix setDrawsBackground: YES];

      if (!_allowsMultipleSelection)
        {
          [matrix setMode: NSRadioModeMatrix];
        }
      [matrix setTarget: self];
      [matrix setAction: @selector(doClick:)];
      [matrix setDoubleAction: @selector(doDoubleClick:)];
      
      // set new col matrix and release old
      [bc setColumnMatrix: matrix];
      RELEASE (matrix);
    }
  [sc setDocumentView: matrix];

  // Loading is different based upon passive/active delegate
  if (_passiveDelegate)
    {
      // Now loop through the cells and load each one
      id aCell;
      SEL sel1 = @selector(browser:willDisplayCell:atRow:column:);
      IMP imp1 = [_browserDelegate methodForSelector: sel1];
      SEL sel2 = @selector(cellAtRow:column:);
      IMP imp2 = [matrix methodForSelector: sel2];
      
      for (i = 0; i < rows; i++)
        {
          aCell = (*imp2)(matrix, sel2, i, 0);
          if (![aCell isLoaded])
            {
              (*imp1)(_browserDelegate, sel1, self, aCell, i, 
                      column);
              [aCell setLoaded: YES];
            }
        }
    }
  else
    {
      // Tell the delegate to create the rows
      [_browserDelegate browser: self
                        createRowsForColumn: column
                        inMatrix: matrix];
    }

  [bc setIsLoaded: YES];
  
  if (column > _lastColumnLoaded)
    {
      _lastColumnLoaded = column;
    }

  /* Determine the height of a cell in the matrix, and set that as the 
     cellSize of the matrix.  */
  {
    NSSize cs, ms;
    NSBrowserCell *b = [matrix cellAtRow: 0  column: 0]; 

    if (b != nil)
      {
        ms = [b cellSize];
      }
    else
      {
        ms = [matrix cellSize];
      }
    cs = [sc contentSize];
    ms.width = cs.width;
    [matrix setCellSize: ms];
  }

  // Get the title even when untitled, as this may change later.
  [self setTitle: [self _getTitleOfColumn: column] ofColumn: column];
  // Mark for redisplay
  [self displayColumn: column];
}

/* Get the title of a column. */
- (NSString *) _getTitleOfColumn: (NSInteger)column
{
  // Ask the delegate for the column title
  if ([_browserDelegate respondsToSelector: 
                          @selector(browser:titleOfColumn:)])
    {
      return [_browserDelegate browser: self titleOfColumn: column];
    }
  

  // Check if we take title from previous column
  if (_takesTitleFromPreviousColumn)
    {
      id c;
      
      // If first column then use the path separator
      if (column == 0)
        {
          return _pathSeparator;
        }
      
      // Get the selected cell
      // Use its string value as the title
      // Only if it is not a leaf
      if (_allowsMultipleSelection == NO)
        {
          c = [self selectedCellInColumn: column - 1];
        }
      else
        {
          NSMatrix *matrix;
          NSArray  *selectedCells;

          if (!(matrix = [self matrixInColumn: column - 1]))
            return @"";

          selectedCells = [matrix selectedCells];

          if ([selectedCells count] == 1)
            {
              c = [selectedCells objectAtIndex:0];
            }
          else
            {
              return @"";
            }
        }

      if ([c isLeaf])
        {
          return @"";
        }
      else
        { 
          NSString *value = [c stringValue];

          if (value != nil)
            {
              return value;
            }
          else
            {
              return @"";
            }
        }
    }
  return @"";
}

/* Marks all titles as needing to be redrawn. */
- (void) _setColumnTitlesNeedDisplay
{
  if (_isTitled)
    {
      NSRect r = [self titleFrameOfColumn: _firstVisibleColumn];

      r.size.width = _frame.size.width;
      [self setNeedsDisplayInRect: r];
    }
}

- (void) setNeedsDisplayInRect: (NSRect)invalidRect
{
  [super setNeedsDisplayInRect: invalidRect];
}

- (NSBorderType) _resolvedBorderType
{
  if (browserUseBezels && _separatesColumns)
    {
      return NSBezelBorder;
    }
  return NSNoBorder;
}

- (void) _themeDidActivate: (NSNotification*)notification
{
  [self tile];
}

@end
