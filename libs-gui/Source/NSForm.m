/** <title>NSForm</title>

   <abstract>Form class, a matrix of text fields with labels</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: March 1997
   
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

#include "config.h"
#import <Foundation/NSNotification.h>
#import "AppKit/NSForm.h"
#import "AppKit/NSFormCell.h"

@implementation NSForm

/* Class variables */
static Class defaultCellClass = nil;

+ (void) initialize
{
  if (self == [NSForm class])
    {
      /* Set the initial version */
      [self setVersion: 1];

      /* Set the default cell class */
      defaultCellClass = [NSFormCell class];
    }
}

+ (Class) cellClass
{
  return defaultCellClass;
}

+ (void) setCellClass: (Class)classId
{
  defaultCellClass = classId;
}

- (id) initWithFrame: (NSRect)frameRect
                mode: (NSMatrixMode)aMode
           cellClass: (Class)class
        numberOfRows: (NSInteger)rowsHigh
     numberOfColumns: (NSInteger)colsWide
{
  self = [super initWithFrame: frameRect
                         mode: aMode
                    cellClass: class
                 numberOfRows: rowsHigh
              numberOfColumns: colsWide];
  if (nil == self)
    return nil;

  [self setIntercellSpacing: NSMakeSize (0, 4)];
  return self;
}

- (id) initWithFrame: (NSRect)frameRect
                mode: (NSMatrixMode)aMode
           prototype: (NSCell*)prototype
        numberOfRows: (NSInteger)rowsHigh
     numberOfColumns: (NSInteger)colsWide
{
  self = [super initWithFrame: frameRect
                         mode: aMode
                    prototype: prototype
                 numberOfRows: rowsHigh
              numberOfColumns: colsWide];
  if (nil == self)
    return nil;

  [self setIntercellSpacing: NSMakeSize (0, 4)];  
  return self;
}

/** <p>Adds a new entry with title as its title at the end of the NSForm
    and returns the NSFormCell.</p><p>See Also: -insertEntry:atIndex:
    -removeEntryAtIndex:</p>
 */
- (NSFormCell*) addEntry: (NSString*)title
{
  return [self insertEntry: title atIndex: [self numberOfRows]];
}

/** <p>Inserts a new entry with title as its title at the index index of
    the NSForm and returns the NSFormCell.</p>
    <p>See Also: -addEntry: -removeEntryAtIndex:</p>
 */
- (NSFormCell*) insertEntry: (NSString*)title
                    atIndex: (NSInteger)index
{
  NSFormCell *new_cell = [[[object_getClass(self) cellClass] alloc] initTextCell: title];

  [self insertRow: index];
  [self putCell: new_cell atRow: index column: 0];
  RELEASE (new_cell);
  
  return new_cell;
}

/** <p>Removes the entry at index index. </p>
    <p>See Also: -insertEntry:atIndex: -addEntry:</p>
 */
- (void) removeEntryAtIndex: (NSInteger)index
{
  [[NSNotificationCenter defaultCenter] 
    removeObserver: self 
    name: _NSFormCellDidChangeTitleWidthNotification
    object: [self cellAtRow: index column: 0]];
  
  [self removeRow: index];
}

/* Overriding this method allows decoding stuff to be inherited
   simpler by NSForm */
- (void) putCell: (NSCell*)newCell  atRow: (NSInteger)row  column: (NSInteger)column 
{
  if (column > 0)
    {
      NSLog (@"Warning: NSForm: tried to add a cell in a column > 0");
      return;
    }
  [super putCell: newCell  atRow: row  column: column];
  
  [self setValidateSize: YES];
  
  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_setTitleWidthNeedsUpdate:)
    name: _NSFormCellDidChangeTitleWidthNotification
    object: newCell];
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] 
    removeObserver: self 
    name: _NSFormCellDidChangeTitleWidthNotification
    object: nil];  

  [super dealloc];
}

/** <p>Sets whether then NSForm's entries have bezeled border.</p>
    <p>See Also: [NSCell-setBezeled:]</p>
 */
- (void) setBezeled: (BOOL)flag
{
  NSInteger i, count = [self numberOfRows];

  /* Set the bezeled attribute to the cell prototype */
  [[self prototype] setBezeled: flag];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setBezeled: flag];
}

/** <p>Sets whether then NSForm's entries have border</p>
    <p>See Also: [NSCell-setBordered:]</p>
 */
- (void) setBordered: (BOOL)flag
{
  NSInteger i, count = [self numberOfRows];

  /* Set the bordered attribute to the cell prototype */
  [[self prototype] setBordered: flag];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setBordered: flag];
}

/**<p>Sets the width of all entries to width</p> 
   <p>See Also: [NSMatrix-setCellSize:]</p>
 */
- (void) setEntryWidth: (float)width
{
  NSSize size = [self cellSize];

  size.width = width;
  [self setCellSize: size];
}

/**<p>Sets the size of the frame to aSize</p> 
   <p>See Also: [NSView-setFrameSize:]</p>
 */
- (void) setFrameSize: (NSSize)aSize
{
  [super setFrameSize: aSize];
  // Set the width of the entries independent of autosizesCells
  _cellSize.width = _bounds.size.width;
}

/** <p>Sets the spacing between all entries to spacing. By default
    the spacing is 4.</p><p>See Also: [NSMatrix-setIntercellSpacing:]</p>
 */
- (void) setInterlineSpacing: (CGFloat)spacing
{
  [self setIntercellSpacing: NSMakeSize(0, spacing)];
}

/* For the title attributes we use the corresponding attributes from the cell.
   For the text attributes we use instead the attributes inherited from the
   NSCell class. */

/** <p>Sets the text alignment of the title to aMode for all entries.
    See <ref type="type" id="NSTextAlignment">NSTextAlignment</ref> for more
    informations. The default title alignment is NSLeftTextAlignment</p> 
    <p>See Also:  [NSFormCell-setTitleAlignment:] -setTextAlignment: </p>
 */
- (void) setTitleAlignment: (NSTextAlignment)aMode
{
  NSInteger i, count = [self numberOfRows];

  /* Set the title alignment attribute to the cell prototype */
  [[self prototype] setTitleAlignment: aMode];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setTitleAlignment: aMode];
}

/** <p>Sets the text alignment to aMode for all entries. See
    <ref type="type" id="NSTextAlignment">NSTextAlignment</ref> for more
    informations. The default text alignment is NSRightTextAlignment</p> 
    <p>See Also: -setTitleAlignment: [NSCell-setAlignment:]</p>
 */
- (void) setTextAlignment: (NSTextAlignment)aMode
{
  NSInteger i, count = [self numberOfRows];

  /* Set the text alignment attribute to the cell prototype */
  [[self prototype] setAlignment: aMode];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setAlignment: aMode];
}

/** <p>Sets the text font of the title to fontObject for all entries</p>
    <p>See Also: [NSFormCell-setTitleFont:] -setTextFont:</p>
 */
- (void) setTitleFont: (NSFont*)fontObject
{
  NSInteger i, count = [self numberOfRows];

  /* Set the title font attribute to the cell prototype */
  [[self prototype] setTitleFont: fontObject];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setTitleFont: fontObject];
}

/** <p>Sets the text font to fontObject for all entries</p>
    <p>See Also: [NSCell-setFont:] -setTitleFont:</p>
 */
- (void) setTextFont: (NSFont*)fontObject
{
  NSInteger i, count = [self numberOfRows];

  /* Set the text font attribute to the cell prototype */
  [[self prototype] setFont: fontObject];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setFont: fontObject];
}

/** <p>Sets the title writing direction to direction for all entries</p>
    <p>See Also: [NSFormCell-setTitleBaseWritingDirection:] -setTextBaseWritingDirection:</p>
 */
- (void) setTitleBaseWritingDirection: (NSWritingDirection)direction
{
  NSInteger i, count = [self numberOfRows];

  /* Set the writing direction attribute to the cell prototype */
  [[self prototype] setTitleBaseWritingDirection: direction];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setTitleBaseWritingDirection: direction];
}

/** <p>Sets the contents writing direction to direction for all entries</p>
    <p>See Also: [NSCell-setBaseWritingDirection:] -setTitleBaseWritingDirection:</p>
 */
- (void) setTextBaseWritingDirection: (NSWritingDirection)direction
{
  NSInteger i, count = [self numberOfRows];

  /* Set the writing direction attribute to the cell prototype */
  [[self prototype] setBaseWritingDirection: direction];

  for (i = 0; i < count; i++)
    [[self cellAtRow: i column: 0] setBaseWritingDirection: direction];
}

/**<p>Returns the index of the entry specified by aTag or -1 if aTag is not 
   found in entries.</p><p>See Also: [NSMatrix-cellAtRow:column:]</p>
 */
- (NSInteger) indexOfCellWithTag: (NSInteger)aTag
{
  NSInteger i, count = [self numberOfRows];

  for (i = 0; i < count; i++)
    if ([[self cellAtRow: i column: 0] tag] == aTag)
      return i;
  return -1;
}

/**<p>Returns the index of the current selected entry.</p>
   <p>[NSMatrix-selectedRow]</p>
 */
- (NSInteger) indexOfSelectedItem
{
  return [self selectedRow];
}

/**<p>Returns the NSFormCell at index <var>index</var></p>
   <p>See Also: [NSMatrix-cellAtRow:column:]</p>
 */
- (id) cellAtIndex: (NSInteger)index
{
  return [self cellAtRow: index column: 0];
}

-(void) _setTitleWidthNeedsUpdate: (NSNotification*)notification
{
  [self setValidateSize: YES];
}

- (void) setValidateSize: (BOOL)flag
{
  _title_width_needs_update = flag;
  // TODO: Think about reducing redisplaying
  if (flag)
    [self setNeedsDisplay];
}

- (void) calcSize
{
  NSInteger i, count = [self numberOfRows];
  CGFloat new_title_width = 0;
  CGFloat candidate_title_width = 0;
  NSRect rect;

  // Compute max of title width in the cells
  for (i = 0; i < count; i++)
    {
      candidate_title_width = [_cells[i][0] titleWidth];
      if (candidate_title_width > new_title_width)  
        new_title_width = candidate_title_width;
    }

  // Suggest this max as title width to all cells 
  rect = NSMakeRect (0, 0, new_title_width, 0);
  for (i = 0; i < count; i++)
    {
      [_cells[i][0] calcDrawInfo: rect];
    }
  _title_width_needs_update = NO;
}

- (void) drawRect: (NSRect)rect
{
  if (_title_width_needs_update)
    [self calcSize];

  [super drawRect: rect];
}

/** <p>Draws the NSFormCell at the specified index</p>
    <p>See Also: -cellAtIndex: [NSCell-drawWithFrame:inView:]
    [NSMatrix-cellFrameAtRow:column:]</p>
 */
- (void) drawCellAtIndex: (NSInteger)index
{
  id theCell = [self cellAtIndex: index];

  [theCell drawWithFrame: [self cellFrameAtRow: index column: 0]
                  inView: self];
}

- (void) drawCellAtRow: (NSInteger)row column: (NSInteger)column
{
  [self drawCellAtIndex: row];
}

/** <p>Selects the text in the entry specified by index.</p>
    <p>[NSMatrix-selectTextAtRow:column:]</p>
 */
- (void) selectTextAtIndex: (NSInteger)index
{
  [self selectTextAtRow: index column: 0];
}

@end
