/** <titlt>GSTable.m</title>

   <abstract>The GSTable class (a GNU extension)</abstract>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Nicola Pero <n.pero@mi.flashnet.it>
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

#import "GNUstepGUI/GSTable.h"

@interface GSTable (Private)
-(void) _updateForNewFrameSize: (NSSize)newFrameSize;
-(void) _updateRowSize: (int)row;
-(void) _updateColumnSize: (int)column;
-(void) _updateRowOrigin: (int)row;
-(void) _updateColumnOrigin: (int)column;
-(void) _updateWholeTable;
@end

@implementation GSTable: NSView
//
// Class Methods
//
+(void) initialize
{
  if (self == [GSTable class])
    [self setVersion: 1];
}

/* It was reported that the inherited +new implementation doesn't work
 * on OSX.  Override it with a sane implementation to get portability
 * to OSX.  */
+(id) new
{
  return [[self alloc] init];
}

//
// Instance Methods
//

// Designated initializer
-(id) initWithNumberOfRows: (int)rows
	   numberOfColumns: (int)columns
{
  int i;
  
  self = [super init];
  if (nil == self)
    return nil;

  [super setAutoresizesSubviews: NO];
  if (!(rows > 0))
    {
      NSLog (@"Warning: Argument rows <= 0");
      rows = 2;
    }
  if (!(columns > 0))
    {
      NSLog (@"Warning: Argument columns <= 0");
      columns = 2;
    }

  _numberOfRows = rows;
  _numberOfColumns = columns;

  _minXBorder = 0;
  _maxXBorder = 0;
  _minYBorder = 0;  
  _maxYBorder = 0;

  _jails = NSZoneMalloc (NSDefaultMallocZone (), 
			 sizeof (NSView *) * (rows * columns));
  _expandRow = NSZoneMalloc (NSDefaultMallocZone (), 
			     sizeof (BOOL) * rows);
  _expandColumn = NSZoneMalloc (NSDefaultMallocZone (), 
				sizeof (BOOL) * columns);
  _columnDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				   sizeof (float) * columns);
  _rowDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				sizeof (float) * rows);
  _columnXOrigin = NSZoneMalloc (NSDefaultMallocZone (), 
				 sizeof (float) * columns);
  _rowYOrigin = NSZoneMalloc (NSDefaultMallocZone (), 
			      sizeof (float) * rows);
  _minColumnDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				      sizeof (float) * columns);
  _minRowDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				   sizeof (float) * rows);
  _havePrisoner = NSZoneMalloc (NSDefaultMallocZone (), 
				sizeof (BOOL) * (rows * columns));
  for (i = 0; i < (rows * columns); i++)
    {
      _jails[i] = NULL;
      _havePrisoner[i] = NO;
    }

  for (i = 0; i < rows; i++)
    {
      _expandRow[i] = YES;
      _rowDimension[i] = 0;
      _rowYOrigin[i] = 0;
      _minRowDimension[i] = 0;
    }
  _expandingRowNumber = rows;

  for (i = 0; i < columns; i++)
    {
      _expandColumn[i] = YES;
      _columnDimension[i] = 0;
      _columnXOrigin[i] = 0;
      _minColumnDimension[i] = 0;
    }
  _expandingColumnNumber = columns;

  _minimumSize = NSZeroSize;

  return self;
}

-(id) init
{
  return [self initWithNumberOfRows: 2
	       numberOfColumns: 2];
}

-(void) dealloc 
{
  NSZoneFree (NSDefaultMallocZone (), _jails);
  NSZoneFree (NSDefaultMallocZone (), _expandColumn);
  NSZoneFree (NSDefaultMallocZone (), _expandRow);
  NSZoneFree (NSDefaultMallocZone (), _columnDimension);
  NSZoneFree (NSDefaultMallocZone (), _rowDimension);
  NSZoneFree (NSDefaultMallocZone (), _columnXOrigin);
  NSZoneFree (NSDefaultMallocZone (), _rowYOrigin);
  NSZoneFree (NSDefaultMallocZone (), _minColumnDimension);
  NSZoneFree (NSDefaultMallocZone (), _minRowDimension);
  NSZoneFree (NSDefaultMallocZone (), _havePrisoner);
  [super dealloc];
}

- (void) setAutoresizesSubviews: (BOOL)flag
{
  NSLog (@"Warning: attempt to setAutoresizesSubviews for a GSTable!\n");
  return;
}

//
// Setting Border.
//
-(void) setBorder: (float)aBorder
{
  [self setMinXBorder: aBorder];
  [self setMaxXBorder: aBorder];
  [self setMinYBorder: aBorder];
  [self setMaxYBorder: aBorder];

}

-(void) setXBorder: (float)aBorder
{
  [self setMinXBorder: aBorder];
  [self setMaxXBorder: aBorder];
}

-(void) setYBorder: (float)aBorder
{
  [self setMinYBorder: aBorder];
  [self setMaxYBorder: aBorder];
}

-(void) setMinXBorder: (float)aBorder
{  
  float borderChange;
  NSSize tableSize = [self frame].size;
  int i;
 
  if (aBorder < 0)
    aBorder = 0;
  
  borderChange = aBorder - _minXBorder; 
  
  for (i = 0; i < _numberOfColumns; i++)
    {
      _columnXOrigin[i] += borderChange;
      [self _updateColumnOrigin: i];
    }
  
  _minimumSize.width += borderChange;
  tableSize.width += borderChange; 
  [super setFrameSize: tableSize];

  _minXBorder = aBorder;
}

-(void) setMaxXBorder: (float)aBorder
{
  float borderChange;
  NSSize tableSize = [self frame].size;
  
  if (aBorder < 0)
    aBorder = 0;
  
  borderChange = aBorder - _maxXBorder; 
  
  _minimumSize.width += borderChange;
  tableSize.width += borderChange; 
  [super setFrameSize: tableSize];

  _maxXBorder = aBorder;
}

-(void) setMinYBorder: (float)aBorder
{
  float borderChange;
  NSSize tableSize = [self frame].size;
  int i;
  
  if (aBorder < 0)
    aBorder = 0;
  
  borderChange = aBorder - _minYBorder; 
  
  for (i = 0; i < _numberOfRows; i++)
    {
      _rowYOrigin[i] += borderChange;
      [self _updateRowOrigin: i];
    }
  
  _minimumSize.height += borderChange;
  tableSize.height += borderChange; 
  [super setFrameSize: tableSize];

  _minYBorder = aBorder;
}

-(void) setMaxYBorder: (float)aBorder
{
  float borderChange;
  NSSize tableSize = [self frame].size;
  
  if (aBorder < 0)
    aBorder = 0;
  
  borderChange = aBorder - _maxYBorder; 
  
  _minimumSize.height += borderChange;
  tableSize.height += borderChange; 
  [super setFrameSize: tableSize];

  _maxYBorder = aBorder;
}

// 
// Adding Views 
// 
-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column
{
  [self putView: aView
	atRow: row
	column: column
	withMinXMargin: 0
	maxXMargin: 0
	minYMargin: 0
	maxYMargin: 0];  	    
}

-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column
    withMargins: (float)margins
{
  [self putView: aView
	atRow: row
	column: column
	withMinXMargin: margins
	maxXMargin: margins	 
	minYMargin: margins	 
	maxYMargin: margins];  	    
}

-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column
   withXMargins: (float)xMargins
       yMargins: (float)yMargins
{
  [self putView: aView
	atRow: row
	column: column
	withMinXMargin: xMargins
	maxXMargin: xMargins	 
	minYMargin: yMargins	 
	maxYMargin: yMargins];  	    
}

// The other methods are only wrappers for this one.
-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column
 withMinXMargin: (float)minXMargin
     maxXMargin: (float)maxXMargin
     minYMargin: (float)minYMargin
     maxYMargin: (float)maxYMargin
{
  int jailNumber;
  NSRect oldFrame;
  NSRect theFrame;
  NSRect tableFrame = [self frame];
  int i;
  // YES if the GSTable needs to be resized
  BOOL tableNeedResize = NO;
  // YES if the {prisoner + margins} needs to be resized to fill the jail. 
  // This is accomplished by creating the jail with the old size (of 
  // {prisoner + margins}), putting the prisoner inside and the resizing 
  // the jail to the new size. 
  BOOL prisonerNeedResize = NO;

  if (row > (_numberOfRows - 1)) 
    {
      NSLog (@"Warning: argument row is > (numberOfRows - 1)\n");
      return;
    }
  if (row < 0)
    {
      NSLog (@"Warning: argument row is < 0\n");
      return;
    }
  if (column > (_numberOfColumns - 1))
    {
      NSLog (@"Warning: argument column is > (numberOfColumns - 1)\n");
      return;
    }
  if (column < 0)
    {
      NSLog (@"Warning: argument column is < 0\n");
      return;
    }

  oldFrame = [aView frame];
  oldFrame.size.width += (minXMargin + maxXMargin);
  oldFrame.size.height += (minYMargin + maxYMargin);
  theFrame = oldFrame;
  
  jailNumber = row * (_numberOfColumns) + column;

  //
  //
  // Manage Column/Row and aView Sizes
  //
  //

  //
  // Column
  //
  if (theFrame.size.width > _columnDimension[column])
    {
      float xShift = theFrame.size.width - _columnDimension[column];

      // Compute new size tableFrame.size
      tableFrame.size.width += xShift;
      tableNeedResize = YES;
      
      // Resize the column
      _columnDimension[column] = theFrame.size.width;
      [self _updateColumnSize: column];
      
      // Shift the columns on the right 
      for (i = column + 1; i < _numberOfColumns; i++)
	{
	  _columnXOrigin[i] += xShift;
	  [self _updateColumnOrigin: i];
	}
    }
  else // theFrame.size.width <= _columnDimension[column]
    {
      theFrame.size.width = _columnDimension[column];
      prisonerNeedResize = YES;
    }
  //
  // Row
  //
  if (theFrame.size.height > _rowDimension[row])
    {
      float yShift = theFrame.size.height - _rowDimension[row];

      // Compute new size
      tableFrame.size.height += yShift;
      tableNeedResize = YES;

      // Resize the row
      _rowDimension[row] = theFrame.size.height;
      [self _updateRowSize: row];

      // Shift the rows on the top 
      for (i = row + 1; i < _numberOfRows; i++)
	{
	  _rowYOrigin[i] += yShift;
	  [self _updateRowOrigin: i];
	}
    }
  else // theFrame.size.height <= _rowDimension[row]
    {
      theFrame.size.height = _rowDimension[row];
      prisonerNeedResize = YES;
    }
  
  if (tableNeedResize)
    {
      [super setFrameSize: tableFrame.size];
    }
  
      
  if (_minColumnDimension[column] < theFrame.size.width)
    {
      _minimumSize.width += (theFrame.size.width - _minColumnDimension[column]);
      _minColumnDimension[column] = theFrame.size.width;
    }

  if (_minRowDimension[row] < theFrame.size.height)
    {
      _minimumSize.height += (theFrame.size.height - _minRowDimension[row]);
      _minRowDimension[row] = theFrame.size.height;
    }
  //
  //
  // Put the jail in the GSTable
  //
  //
  theFrame.origin = NSMakePoint (tableFrame.origin.x + _columnXOrigin[column], 
				 tableFrame.origin.y + _rowYOrigin[row]);
  if (_havePrisoner[jailNumber])
    {
      if (prisonerNeedResize)
	[_jails[jailNumber] setFrame: oldFrame];
      else // !prisonerNeedResize
	[_jails[jailNumber] setFrame: theFrame];
    }
  else // !_havePrisoner
    {
      if (prisonerNeedResize)
	_jails[jailNumber] = [[NSView alloc] initWithFrame: oldFrame];
      else // !prisonerNeedResize
	_jails[jailNumber] = [[NSView alloc] initWithFrame: theFrame];

      [_jails[jailNumber] setAutoresizingMask: NSViewNotSizable];
      [_jails[jailNumber] setAutoresizesSubviews: YES];
      [self addSubview: _jails[jailNumber]];
      [_jails[jailNumber] release];
    }
  //
  //
  // Put the prisoner in the jail
  //
  //
  if (!_havePrisoner[jailNumber])
    {
      [_jails[jailNumber] addSubview: aView];
    }
  else // _havePrisoner[jailNumber]
    {
      [_jails[jailNumber] 
	     replaceSubview: [[_jails[jailNumber] subviews] objectAtIndex: 0]
	     with: aView];
    }
  [aView setFrameOrigin: NSMakePoint (minXMargin, minYMargin)];
  if (prisonerNeedResize)
    [_jails[jailNumber] setFrame: theFrame];
  _havePrisoner[jailNumber] = YES;
} 

/* resizeWithOldSuperviewSize: automatically calls setFrame: */

- (void) setFrame: (NSRect)frame
{
  [self _updateForNewFrameSize: frame.size];
  [super setFrame: frame];
}

- (void) setFrameSize: (NSSize)newFrameSize
{
  [self _updateForNewFrameSize: newFrameSize];
  [super setFrameSize: newFrameSize];
}

//
// Minimum Size
//
-(NSSize) minimumSize
{
  return _minimumSize;
} 

//
// Resizing 
//
-(void) sizeToFit
{
  int i;

  // This should never happen but anyway. 
  if ((_numberOfColumns == 0) || (_numberOfRows == 0))
    {
      [super setFrameSize: NSZeroSize];
      return;
    }
  
  _columnXOrigin[0] = _minXBorder;
  _columnDimension[0] = _minColumnDimension[0];
  _rowYOrigin[0] = _minYBorder;
  _rowDimension[0] = _minRowDimension[0];
  
  for (i = 1; i < _numberOfColumns; i++)
    {
      _columnXOrigin[i] = _columnXOrigin[i - 1] + _columnDimension[i - 1];
      _columnDimension[i] = _minColumnDimension[i];
    }

  for (i = 1; i < _numberOfRows; i++)
    {
      _rowYOrigin[i] = _rowYOrigin[i - 1] + _rowDimension[i - 1];
      _rowDimension[i] = _minRowDimension[i];
    }
  [self _updateWholeTable];
  [super setFrameSize: _minimumSize];
}

//
// Adding Rows and Columns
// These should be used to add more rows and columns to the GSTable. 
// Of course it is faster to create a GSTable with the right number of rows 
// and columns from the beginning.
//
-(void) addRow
{
  int j;

  _numberOfRows++;
  _havePrisoner = NSZoneRealloc (NSDefaultMallocZone (), _havePrisoner, 
				 (_numberOfRows * _numberOfColumns) 
				 * (sizeof (BOOL)));
  _jails = NSZoneRealloc (NSDefaultMallocZone (), _jails, 
			 (_numberOfRows * _numberOfColumns) 
			  * sizeof (NSView *));

  for (j = (_numberOfRows - 1) * _numberOfColumns; 
       j < (_numberOfRows * _numberOfColumns); j++)
    {
      _jails[j] = NULL;
      _havePrisoner[j] = NO;
    }
  
  _expandRow = NSZoneRealloc (NSDefaultMallocZone (), _expandRow, 
			      (_numberOfRows) * (sizeof (BOOL)));
  _expandRow[_numberOfRows - 1] = YES;
  
  _expandingRowNumber++;
  
  _rowDimension = NSZoneRealloc (NSDefaultMallocZone (), _rowDimension, 
				 (_numberOfRows) * (sizeof (float)));
  _rowDimension[_numberOfRows - 1] = 0;
  
  _rowYOrigin = NSZoneRealloc (NSDefaultMallocZone (), _rowYOrigin, 
			       (_numberOfRows) * (sizeof (float)));
  _rowYOrigin[_numberOfRows - 1] = (_rowYOrigin[_numberOfRows - 2] 
                                   + _rowDimension[_numberOfRows - 2]);
  
  _minRowDimension = NSZoneRealloc (NSDefaultMallocZone (), _minRowDimension, 
				    (_numberOfRows) * (sizeof (float)));
  _minRowDimension[_numberOfRows - 1] = 0;
  
}

// TODO: -(void) insertRow: (int)row;
// TODO: -(void) removeRow: (int)row;
-(void) addColumn
{
  int i, j;

  _numberOfColumns++;

  _havePrisoner = NSZoneRealloc (NSDefaultMallocZone (), _havePrisoner, 
				 (_numberOfRows * _numberOfColumns) 
				 * (sizeof (BOOL)));
  _jails = NSZoneRealloc (NSDefaultMallocZone (), _jails, 
			  (_numberOfRows * _numberOfColumns) 
			  * sizeof (NSView *));
  // Reorder the jails
  for (j = (_numberOfRows - 1); j >= 0; j--)
    {
      _jails[(_numberOfColumns * (j + 1)) - 1] = NULL;
      _havePrisoner[(_numberOfColumns *  (j + 1)) - 1] = NO;
      for (i = (_numberOfColumns - 2); i >= 0; i--)
	{
	  _jails[(_numberOfColumns * j) + i] 
	    = _jails[((_numberOfColumns - 1) * j) + i];
	  _havePrisoner[(_numberOfColumns * j) + i] 
	    = _havePrisoner[((_numberOfColumns - 1) * j) + i];
	}
    }
  
  _expandColumn = NSZoneRealloc (NSDefaultMallocZone (), _expandColumn, 
				 (_numberOfColumns) * (sizeof (BOOL)));
  _expandColumn[_numberOfColumns - 1] = YES;
  
  _expandingColumnNumber++;

  _columnDimension = NSZoneRealloc (NSDefaultMallocZone (), _columnDimension, 
				   (_numberOfColumns) * (sizeof (float)));
  _columnDimension[_numberOfColumns - 1] = 0;
  
  _columnXOrigin = NSZoneRealloc (NSDefaultMallocZone (), _columnXOrigin, 
				  (_numberOfColumns) * (sizeof (float)));
  _columnXOrigin[_numberOfColumns - 1] = (_columnXOrigin[_numberOfColumns - 2] 
                                     + _columnDimension[_numberOfColumns - 2]);
  
  _minColumnDimension = NSZoneRealloc (NSDefaultMallocZone (), 
				       _minColumnDimension, 
				       (_numberOfColumns) * (sizeof (float)));
  _minColumnDimension[_numberOfColumns - 1] = 0;
}
// TODO: -(void) insertColumn: (int)column;
// TODO: -(void) removeColumn: (int)column;

//
// Setting Row and Column Expand Flag
// 
-(void) setXResizingEnabled: (BOOL)aFlag
		  forColumn: (int)aColumn
{
   if (aColumn > (_numberOfColumns - 1)) 
    {
      NSLog (@"Warning: argument column is > (numberOfColumns - 1)\n");
      return;
    }
  if (aColumn < 0)
    {
      NSLog (@"Warning: argument column is < 0\n");
      return;
    }
  if ((_expandColumn[aColumn] == YES) && (aFlag == NO))
    {    
      _expandingColumnNumber--;
      _expandColumn[aColumn] = aFlag;
    }
  else if ((_expandColumn[aColumn] == NO) && (aFlag == YES))
    {
      _expandingColumnNumber++;
      _expandColumn[aColumn] = aFlag;
    }
}

-(BOOL) isXResizingEnabledForColumn: (int)aColumn
{
  if (aColumn > (_numberOfColumns - 1)) 
    {
      NSLog (@"Warning: argument column is > (numberOfColumns - 1)\n");
      return NO;
    }  
  if (aColumn < 0)
    {
      NSLog (@"Warning: argument column is < 0\n");
      return NO;
    }
  return _expandColumn[aColumn];
}

-(void) setYResizingEnabled: (BOOL)aFlag
		     forRow: (int)aRow
{
  if (aRow > (_numberOfRows - 1)) 
    {
      NSLog (@"Warning: argument row is > (numberOfRows - 1)\n");
      return;
    }
  if (aRow < 0)
    {
      NSLog (@"Warning: argument row is < 0\n");
      return;
    }
  if ((_expandRow[aRow] == YES) && (aFlag == NO))
    {
      _expandingRowNumber--;
      _expandRow[aRow] = aFlag;
    }
  else if ((_expandRow[aRow] == NO) && (aFlag == YES))
    {
      _expandingRowNumber++;
      _expandRow[aRow] = aFlag;
    }
}

-(BOOL) isYResizingEnabledForRow: (int)aRow
{
  if (aRow > (_numberOfRows - 1)) 
    {
      NSLog (@"Warning: argument row is > (numberOfRows - 1)\n");
      return NO;
    }
  if (aRow < 0)
    {
      NSLog (@"Warning: argument row is < 0\n");
      return NO;
    }
  return _expandRow[aRow];
}

//
// Getting Row and Column Number
//
-(int) numberOfRows
{
  return _numberOfRows;
}

-(int) numberOfColumns
{
  return _numberOfColumns;
}

//
// NSCoding protocol
//
-(void) encodeWithCoder: (NSCoder*)aCoder
{
  int i;
  
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeInt: _numberOfRows forKey: @"GSNumberOfRows"];
      [aCoder encodeInt: _numberOfColumns forKey: @"GSNumberOfColumns"];
      for (i = 0; i < _numberOfRows * _numberOfColumns; i++)
	{
	  [aCoder encodeObject: _jails[i] forKey: 
		    [NSString stringWithFormat: @"GSJail%d",i]];
	  [aCoder encodeBool: _havePrisoner[i] forKey: 
		    [NSString stringWithFormat: @"GSHavePrisoner%d",i]];
	}
      [aCoder encodeFloat: _minXBorder forKey: @"GSMinXBorder"];
      [aCoder encodeFloat: _maxXBorder forKey: @"GSMaxXBorder"];
      [aCoder encodeFloat: _minYBorder forKey: @"GSMinYBorder"];
      [aCoder encodeFloat: _maxYBorder forKey: @"GSMaxYBorder"];
      for (i = 0; i < _numberOfColumns; i++)
	{
	  [aCoder encodeBool: _expandColumn[i] forKey: 
		    [NSString stringWithFormat: @"GSExpandColumn%d",i]];
	  [aCoder encodeFloat: _columnDimension[i] forKey:
		    [NSString stringWithFormat: @"GSColumnDimension%d",i]];	  
	  [aCoder encodeFloat: _minColumnDimension[i] forKey:
		    [NSString stringWithFormat: @"GSMinColumnDimension%d",i]];
	}
      for (i = 0; i < _numberOfRows; i++)
	{
	  [aCoder encodeBool: _expandRow[i] forKey: 
		    [NSString stringWithFormat: @"GSExpandRow%d",i]];
	  [aCoder encodeFloat: _rowDimension[i] forKey: 
		    [NSString stringWithFormat: @"GSRowDimension%d",i]];
	  [aCoder encodeFloat: _minRowDimension[i] forKey: 
		    [NSString stringWithFormat: @"GSMinRowDimension%d",i]];
	}
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(int) at: &_numberOfRows];
      [aCoder encodeValueOfObjCType: @encode(int) at: &_numberOfColumns];
      for (i = 0; i < _numberOfRows * _numberOfColumns; i++)
	{
	  [aCoder encodeObject: _jails[i]];
	  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_havePrisoner[i]];
	}
      [aCoder encodeValueOfObjCType: @encode(float) at: &_minXBorder];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_maxXBorder];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_minYBorder];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_maxYBorder];
      for (i = 0; i < _numberOfColumns; i++)
	{
	  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_expandColumn[i]];
	  [aCoder encodeValueOfObjCType: @encode(float) at: &_columnDimension[i]];
	  [aCoder encodeValueOfObjCType: @encode(float) 
		  at: &_minColumnDimension[i]];
	}
      for (i = 0; i < _numberOfRows; i++)
	{
	  [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_expandRow[i]];
	  [aCoder encodeValueOfObjCType: @encode(float) at: &_rowDimension[i]];
	  [aCoder encodeValueOfObjCType: @encode(float) at: &_minRowDimension[i]];
	}
    }
}

-(id) initWithCoder: (NSCoder*)aDecoder
{
  int i;
      
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return self;

  [super setAutoresizesSubviews: NO];

  if ([aDecoder allowsKeyedCoding])
    {
      _numberOfRows = [aDecoder decodeIntForKey: @"GSNumberOfRows"];
      _numberOfColumns = [aDecoder decodeIntForKey: @"GSNumberOfColumns"];

      // create the jails...
      _jails = NSZoneMalloc (NSDefaultMallocZone (), 
			     sizeof (NSView *) 
			     * (_numberOfRows * _numberOfColumns));
      
      _havePrisoner = NSZoneMalloc (NSDefaultMallocZone (), 
				    sizeof (BOOL) 
				    * (_numberOfRows * _numberOfColumns));
      
      
      for (i = 0; i < _numberOfRows * _numberOfColumns; i++)
	{
	  _jails[i] = [aDecoder decodeObjectForKey: 
				  [NSString stringWithFormat: @"GSJail%d",i]];
	  _havePrisoner[i] = [aDecoder decodeBoolForKey: 
					 [NSString stringWithFormat: @"GSHavePrisoner%d",i]];
	}
      
      _minXBorder = [aDecoder decodeFloatForKey: @"GSMinXBorder"];
      _maxXBorder = [aDecoder decodeFloatForKey: @"GSMaxXBorder"];
      _minYBorder = [aDecoder decodeFloatForKey: @"GSMinYBorder"];
      _maxYBorder = [aDecoder decodeFloatForKey: @"GSMaxYBorder"];

                  // We compute _minimumSize, _expandingRowNumber 
      // and _expandingColumnNumber during deconding.
      _minimumSize = NSZeroSize;
      _expandingRowNumber = 0;
      _expandingColumnNumber = 0;
      
      // Columns
      _expandColumn = NSZoneMalloc (NSDefaultMallocZone (), 
				    sizeof (BOOL) * _numberOfColumns);
      _columnDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				       sizeof (float) * _numberOfColumns);
      _minColumnDimension = NSZoneMalloc (NSDefaultMallocZone (), 
					  sizeof (float) * _numberOfColumns);
      _minimumSize.width += _minXBorder;
      for (i = 0; i < _numberOfColumns; i++)
	{
	  _expandColumn[i] = [aDecoder decodeBoolForKey: 
					 [NSString stringWithFormat: @"GSExpandColumn%d",i]];
	  if (_expandColumn[i])
	    _expandingColumnNumber++;
	  _columnDimension[i] = [aDecoder decodeFloatForKey: 
					    [NSString stringWithFormat: @"GSColumnDimension%d",i]];
	  _minColumnDimension[i] = [aDecoder decodeFloatForKey: 
					       [NSString stringWithFormat: @"GSMinColumnDimension%d",i]];
	  _minimumSize.width += _minColumnDimension[i];
	}
      _minimumSize.width += _maxXBorder;
      // Calculate column origins
      _columnXOrigin = NSZoneMalloc (NSDefaultMallocZone (), 
				     sizeof (float) * _numberOfColumns);
      _columnXOrigin[0] = _minXBorder;
      for (i = 1; i < _numberOfColumns; i++)
	_columnXOrigin[i] = _columnXOrigin[i - 1] + _columnDimension[i - 1];
      
      // Rows
      _expandRow = NSZoneMalloc (NSDefaultMallocZone (), 
				 sizeof (BOOL) * _numberOfRows);
      _rowDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				    sizeof (float) * _numberOfRows);
      _minRowDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				       sizeof (float) * _numberOfRows);
      _minimumSize.height += _minYBorder;
      for (i = 0; i < _numberOfRows; i++)
	{
	  _expandRow[i] = [aDecoder decodeBoolForKey:
				      [NSString stringWithFormat: @"GSExpandRow%d",i]];
	  if (_expandRow[i])
	    _expandingRowNumber++;
	  _rowDimension[i] = [aDecoder decodeFloatForKey: 
					 [NSString stringWithFormat: @"GSRowDimension%d",i]];
	  _minRowDimension[i] = [aDecoder decodeFloatForKey: 
					    [NSString stringWithFormat: @"GSMinRowDimension%d",i]];
	  _minimumSize.height += _minRowDimension[i];
	}
      _minimumSize.height += _maxYBorder;
      // Calculate row origins
      _rowYOrigin = NSZoneMalloc (NSDefaultMallocZone (), 
				  sizeof (float) * _numberOfRows);
      _rowYOrigin[0] = _minYBorder;
      for (i = 1; i < _numberOfRows; i++)
	_rowYOrigin[i] = _rowYOrigin[i - 1] + _rowDimension[i - 1];
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(int) at: &_numberOfRows];
      [aDecoder decodeValueOfObjCType: @encode(int) at: &_numberOfColumns];
      
      // 
      _jails = NSZoneMalloc (NSDefaultMallocZone (), 
			     sizeof (NSView *) 
			     * (_numberOfRows * _numberOfColumns));
      
      _havePrisoner = NSZoneMalloc (NSDefaultMallocZone (), 
				    sizeof (BOOL) 
				    * (_numberOfRows * _numberOfColumns));
      
      for (i = 0; i < _numberOfRows * _numberOfColumns; i++)
	{
	  _jails[i] = [aDecoder decodeObject];
	  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_havePrisoner[i]];
	}
      
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_minXBorder];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_maxXBorder];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_minYBorder];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_maxYBorder];
      
      // We compute _minimumSize, _expandingRowNumber 
      // and _expandingColumnNumber during deconding.
      _minimumSize = NSZeroSize;
      _expandingRowNumber = 0;
      _expandingColumnNumber = 0;
      
      // Columns
      _expandColumn = NSZoneMalloc (NSDefaultMallocZone (), 
				    sizeof (BOOL) * _numberOfColumns);
      _columnDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				       sizeof (float) * _numberOfColumns);
      _minColumnDimension = NSZoneMalloc (NSDefaultMallocZone (), 
					  sizeof (float) * _numberOfColumns);
      _minimumSize.width += _minXBorder;
      for (i = 0; i < _numberOfColumns; i++)
	{
	  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_expandColumn[i]];
	  if (_expandColumn[i])
	    _expandingColumnNumber++;
	  [aDecoder decodeValueOfObjCType: @encode(float) at: &_columnDimension[i]];
	  [aDecoder decodeValueOfObjCType: @encode(float) 
		    at: &_minColumnDimension[i]];
	  _minimumSize.width += _minColumnDimension[i];
	}
      _minimumSize.width += _maxXBorder;
      // Calculate column origins
      _columnXOrigin = NSZoneMalloc (NSDefaultMallocZone (), 
				     sizeof (float) * _numberOfColumns);
      _columnXOrigin[0] = _minXBorder;
      for (i = 1; i < _numberOfColumns; i++)
	_columnXOrigin[i] = _columnXOrigin[i - 1] + _columnDimension[i - 1];
      
      // Rows
      _expandRow = NSZoneMalloc (NSDefaultMallocZone (), 
				 sizeof (BOOL) * _numberOfRows);
      _rowDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				    sizeof (float) * _numberOfRows);
      _minRowDimension = NSZoneMalloc (NSDefaultMallocZone (), 
				       sizeof (float) * _numberOfRows);
      _minimumSize.height += _minYBorder;
      for (i = 0; i < _numberOfRows; i++)
	{
	  [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_expandRow[i]];
	  if (_expandRow[i])
	    _expandingRowNumber++;
	  [aDecoder decodeValueOfObjCType: @encode(float) at: &_rowDimension[i]];
	  [aDecoder decodeValueOfObjCType: @encode(float) at: &_minRowDimension[i]];
	  _minimumSize.height += _minRowDimension[i];
	}
      _minimumSize.height += _maxYBorder;
      // Calculate row origins
      _rowYOrigin = NSZoneMalloc (NSDefaultMallocZone (), 
				  sizeof (float) * _numberOfRows);
      _rowYOrigin[0] = _minYBorder;
      for (i = 1; i < _numberOfRows; i++)
	_rowYOrigin[i] = _rowYOrigin[i - 1] + _rowDimension[i - 1];
    }

  return self;
}
@end

@implementation GSTable (Private)
/* Updates the subviews and locations for a new frame size.  */
- (void) _updateForNewFrameSize: (NSSize)newFrameSize
{
  NSSize oldFrameSize = [self frame].size;
  float originShift;
  float dimensionIncrement;
  int i;
  // YES if the whole GSTable needs an update
  BOOL tableNeedUpdate = NO;

  //
  // Width
  //
  if (newFrameSize.width <= _minimumSize.width)
    {
      if (oldFrameSize.width > _minimumSize.width)
	{
	  originShift = _minXBorder;
	  for (i = 0; i < _numberOfColumns; i++)
	    {  
	      _columnDimension[i] = _minColumnDimension[i];
	      _columnXOrigin[i] = originShift;
	      originShift += _minColumnDimension[i];
	    }
	  tableNeedUpdate = YES;
	}
    }
    else // newFrameSize.width > _minimumSize.width
    {
      if (oldFrameSize.width < _minimumSize.width)
	oldFrameSize.width = _minimumSize.width;

      if ((newFrameSize.width != oldFrameSize.width) && _expandingColumnNumber)
	{  
	  originShift = 0;
	  dimensionIncrement = newFrameSize.width - oldFrameSize.width;
	  dimensionIncrement = dimensionIncrement / _expandingColumnNumber; 
	  for (i = 0; i < _numberOfColumns; i++)
	    {
	      _columnXOrigin[i] += originShift;
	      if (_expandColumn[i])
		{
		  _columnDimension[i] += dimensionIncrement;
		  originShift += dimensionIncrement;
		}
	    }
	  tableNeedUpdate = YES;
	}
    }
  //
  // Height 
  //
  if (newFrameSize.height <= _minimumSize.height)
    {
      if (oldFrameSize.height > _minimumSize.height)
	{
	  originShift = _minYBorder;
	  for (i = 0; i < _numberOfRows; i++)
	    {  
	      _rowDimension[i] = _minRowDimension[i];
	      _rowYOrigin[i] = originShift;
	      originShift += _minRowDimension[i];
	    }
	  tableNeedUpdate = YES;
	}
    }
    else // newFrameSize.height > _minimumSize.height
    {
      if (oldFrameSize.height < _minimumSize.height)
	oldFrameSize.height = _minimumSize.height;

      if ((newFrameSize.height != oldFrameSize.height) && _expandingRowNumber)
	{  
	  originShift = 0;
	  dimensionIncrement = newFrameSize.height - oldFrameSize.height;
	  dimensionIncrement = dimensionIncrement / _expandingRowNumber; 
	  for (i = 0; i < _numberOfRows; i++)
	    {
	      _rowYOrigin[i] += originShift;
	      if (_expandRow[i])
		{
		  _rowDimension[i] += dimensionIncrement;
		  originShift += dimensionIncrement;
		}
	    }
	  tableNeedUpdate = YES;
	}
    }

  if (tableNeedUpdate)
    {
      [self _updateWholeTable];
    }
}

//
// After computing new theoretical sizes/positions,
// use the following methods to update the real table view
// to the new sizes/positions.
//
-(void) _updateRowSize: (int)row
{ 
  // NB: This (and the following) is for private use, 
  //     so we do not check that row exists. 
  int i;
  int startIndex = row * _numberOfColumns;
  
  for (i = 0; i < _numberOfColumns; i++)
    {
      if (_havePrisoner[startIndex + i])
	{
	  [_jails[startIndex + i] 
		 setFrameSize: NSMakeSize (_columnDimension[i],
					   _rowDimension[row])];
	}
    }
}

-(void) _updateColumnSize: (int)column
{
  int i;
  
  for (i = 0; i < _numberOfRows; i++)
    {
      if (_havePrisoner[(i * _numberOfColumns) + column])
	{
	  [_jails[(i * _numberOfColumns) + column] 
		 setFrameSize: NSMakeSize (_columnDimension[column],
					   _rowDimension[i])];
	}
    }
}

-(void) _updateRowOrigin: (int)row
{ 
  int i;
  int startIndex = row * _numberOfColumns;
  
  for (i = 0; i < _numberOfColumns; i++)
    {
      if (_havePrisoner[startIndex + i])
	{
	  [_jails[startIndex + i]
		 setFrameOrigin: NSMakePoint (_columnXOrigin[i],
					      _rowYOrigin[row])];
	}
    }
}

-(void) _updateColumnOrigin: (int)column
{
  int i;
  
  for (i = 0; i < _numberOfRows; i++)
    {
      if (_havePrisoner[(i * _numberOfColumns) + column])
	{
	  [_jails[(i * _numberOfColumns) + column]
		 setFrameOrigin: NSMakePoint (_columnXOrigin[column],
					      _rowYOrigin[i])];
	}
    }
}

-(void) _updateWholeTable
{
  int i,j;
  
  for (j = 0; j < _numberOfColumns; j++)
    for (i = 0; i < _numberOfRows; i++)
      {
	if (_havePrisoner[(i * _numberOfColumns) + j])
	  {
	    [_jails[(i * _numberOfColumns) + j] 
		   setFrameOrigin: NSMakePoint (_columnXOrigin[j],
						_rowYOrigin[i])];
	    [_jails[(i * _numberOfColumns) + j]
		   setFrameSize: NSMakeSize (_columnDimension[j],
					     _rowDimension[i])];
	  }
      }
}
@end


