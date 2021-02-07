/*
   GSTable.h

   The GSTable class (a GNU extension)

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Nicola Pero <n.pero@mi.flashnet.it>
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

#ifndef _GNUstep_H_GSTable
#define _GNUstep_H_GSTable

#import <AppKit/NSView.h>

/** 
  <unit>
  <heading>GSTable</heading>
 
  <p>
  A GSTable object is used to control the disposition (position and
  size) of a group of NSViews.  The GSTable object offers
  two main facilities to the programmer:
  </p>
  <list>
  <item>
  with a GSTable object, you do not need to specify the exact
  position and size of each view.  You only specify the logical position,
  relative to the other views in the table.  The actual frame of each view
  is then computed by the GSTable at run time.
  </item>
  <item>
  when the GSTable is resized (for example, because the user has
  resized the window in which the GSTable is), the GSTable
  takes care of moving and resizing all its views automatically.  This is
  done in much a advanced and customizable way than in the usual standard
  NSView's autoresizing mechanism.
  </item>
  </list>

  <p>
  You create a GSTable instance with a certain number of rows and
  columns.  The GSTable object itself is invisible; it is only a
  logical device used to specify the subview position.  Then, you place
  one by one the views you want to control in the GSTable, by
  calling a method of the family -putView:atRow:column:.  Before
  placing a view in the table, you should resize it to the minimum
  comfortable size you want it to have.  The table then automatically
  places the views, organizing them in well-ordered columns and rows.
  </p>
  <p>
  The initial size of the GSTable is zero; each time you put a view
  in the GSTable, the GSTable recomputes sizes and as a
  result resizes itself so that it exactly fits the views it contains.
  You should not force a GSTable in a size different from the one
  it has automatically computed.  The only acceptable, reasonable and
  meaningful way of resizing a GSTable is through the appropriate
  [NSView-resizeWithOldSuperviewSize:] message when the GSTable is
  in the view hierarchy.
  </p>
  <p>
  When you add a view, you may specify some particular margins to be used
  for that view.  If nothing is specified, the view is added to the table
  with the margins of 0.  You should think of each view and its margins as
  a whole.  A position in the GSTable is free or filled with a view
  and its margins.
  </p>
  <p>
  The GSTable itself knows what is the minimum size it needs to
  have in order to comfortably display the views it contains.  You may get
  this size by calling the method -minimumSize.  When first filled,
  the table has this minimum size.  If in any moment you want the table to
  restore itself to this size, you should invoke the method
  -sizeToFit.
  </p>
  <p>
  When the GSTable receives a [NSView-resizeWithOldSuperviewSize:]
  message, it automatically rearranges the views it contains:
  </p>
  <list>
  <item>
  If the new width or height is equal or less than the table's minimum
  width or height, the GSTable simply arranges its views in the
  initial position.  In other words, the GSTable refuse to resize
  below its minimum width or height.  If you do that, part of the
  GSTable is clipped.
  </item>
  <item>
  If the new width or height is bigger than the table's minimum width or
  height, the space in excess is equally distributed between the columns
  or rows which have X (or Y) resizing enabled.  When a column or a row is
  resized, each view in the column or row is resized together with its
  margins.  By setting the autoresizingMask of each view, you may decide
  how the resizing operation will act on that particular view and its
  margins.  For example, setting the autoresizingMask to
  <code>NSViewWidthSizable | NSViewHeightSizable</code> will always leave the
  margins fixed to their initial dimensions, and expand/reduce only the
  view, in all directions.  Setting the autoresizingMask to
  <code>NSViewMinXMargin | NSViewMaxXMargin</code> <code>| NSViewSizable |
  NSViewHeightSizable</code> will instead expand/reduce both the margins and the
  view in the horizontal direction, but leave the margins fixed and
  expand/reduce only the view in the vertical direction.  Whatever the
  autoresizingMask and the amount of the resizing, views and margins are
  never resized below their minimum comfortable size, as explained above.
  For more information on the autoresizingMask, please refer to the
  description of the -setAutoresizingMask: method of the
  NSView class.
  </item>
  </list>

  <section>
  <heading> Advanced Description of GSTable</heading>
  
  <p>
  We call any view which is added to the GSTable a <var>prisoner</var>.
  The purpose of the GSTable is to effectively manage its
  prisoners.  To do so, the GSTable creates a special view, called
  a <var>jail</var>, for each prisoner.  The jails are subviews of the
  GSTable; each prisoner, when added to the GSTable, is made
  a subview of its jail.  The GSTable always moves and resizes
  directly the jails.  The moving is automatically transmitted to the
  prisoners, which are subviews of the jails; the resizing is transmitted
  through the usual autoresizing machinery, because the jails always have
  autoresizing of subviews turned on.  This works because if a prisoner
  sends to its superview an [NSView-frame] message, the frame of the jail
  (and <em>not</em> the frame of the GSTable) is returned, so that
  each prisoner will autoresize itself in its jail frame.  Moreover, any
  prisoner, being a subview of its jail, is clipped in its jail frame.  If
  a prisoner draws something out of its jail frame, the output is
  discarded by the usual subview/view clipping machinery.  This prevents
  the prisoners from disturbing each other.  The dimension of the jail is
  the dimension of the prisoner plus its margins.  Since the
  GSTable manages directly the jails, each prisoner is managed
  together with its margins.  When the jail is resized, the prisoner
  receives a [NSView-resizeWithOldSuperviewSize:], which makes it resize
  itself and its margins in the new jail size, according to its
  autoresizingMask.
  </p>
  </section>

  <section>
  <heading> Setting Row and Column Expand Flag</heading>
  <p>
  When the GSTable is resized, the extra space is equally divided
  between the Rows and Columns which have the X (or Y) resizing enabled.
  The following methods let you enable/disable the X (or Y) resizing of
  each row and column in the GSTable.  Note that when the
  GSTable is first created, all its columns and rows have by
  default resizing enabled.
  -setXResizingEnabled:forColumn:, -setYResizingEnabled:forRow:.
  </p>
  </section>
  </unit>
*/
@interface GSTable: NSView
{
  int _numberOfRows;
  int _numberOfColumns;
  // Border around the table.
  float _minXBorder;  
  float _maxXBorder;
  float _minYBorder;
  float _maxYBorder;
  // We control the NSView inserted in the GSTable (which we call 
  // the prisoners) by enclosing them in jails. 
  // Each prisoner is enclosed in a jail (which is a subview under 
  // our control). 
  // Each prisoner is allowed to resize only inside its jail.   
  NSView **_jails;
  // YES if the column/row should be expanded/reduced when the size 
  // of the GSTable is expanded/reduced (this BOOL is otherwhere 
  // called X/Y Resizing Enabled). 
  BOOL *_expandColumn;
  BOOL *_expandRow;
  // Cache the total number of rows/columns which have expand set to YES 
  int _expandingColumnNumber;
  int _expandingRowNumber;
  // Dimension of each column/row
  float *_columnDimension;
  float *_rowDimension;
  // Origin of each column/row
  float *_columnXOrigin;
  float *_rowYOrigin;
  // Minimum dimension each row/column is allowed to have 
  // (which is the size the jail had when first created).  
  float *_minColumnDimension;
  float *_minRowDimension;
  // Cache the minimum size the GSTable should be resized to.
  NSSize _minimumSize;
  // YES if there is a prisoner in that GSTable position. 
  // (to avoid creating a jail if there is no prisoner to control). 
  BOOL *_havePrisoner;
}
//
// Initizialing.  
// 
/** Initialize a GSTable with columns columns and rows
rows.  If columns or rows is negative or null, a warning
is issued and a default of 2 is used instead.
*/
-(id) initWithNumberOfRows: (int)rows 
           numberOfColumns: (int)columns;

/** Initialize with a default of 2 columns and 2 rows. */
-(id) init;
//
// Setting Border Dimension.
// Border is space around the table. 
//
/** Set the GSTable up, bottom, left and right borders to the same
  value aBorder.  The GSTable is immediately updated.  If
  aBorder is negative, the border is reset to the default, which is
  zero (0).  The border is simply unfilled space; it is measured in the
  GSTable coordinate system.
*/
-(void) setBorder: (float)aBorder;
/** Set the GSTable left and right borders to aBorder.  If
  aBorder is negative, the border is reset to zero.  The
  GSTable is immediately updated.
*/
-(void) setXBorder: (float)aBorder;
/** Same as setXBorder: but set the up and bottom borders. */
-(void) setYBorder: (float)aBorder;
/** Same as setXBorder: but set only the left border. */
-(void) setMinXBorder: (float)aBorder;
/** Same as setXBorder: but set only the right border. */
-(void) setMaxXBorder: (float)aBorder;
/** Same as setXBorder: but set only the lower border (upper 
    if the GSTable is flipped). */
-(void) setMinYBorder: (float)aBorder;
/** Same as setXBorder: but set only the upper border (lower 
    if the GSTable is flipped). */
-(void) setMaxYBorder: (float)aBorder;
//
//  Adding a View. 
//  Use these methods to put views in the GSTable. 
//  
/** Put aView in the GSTable, in the specified row and
column.  Zero (0) margins are used.  If the column column
(or the row row}) is not enough big to fully display aView
and its margins, the column (or the row) is resized (regardless of the
fact that X or Y Resizing is Enabled or not).  It is understood that
this will affect each view (and its margins) in the column (or row)
according to the autoresizing mask of each view.
*/
-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column;

/** Put aView in the GSTable, using margins as margin
in all directions: left, right, top, bottom.
*/
-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column
    withMargins: (float)margins;

/** Put aView in the GSTable, using xMargins as the
left and right margins, and yMargins as the top and bottom
margins. */
-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column
   withXMargins: (float)xMargins
       yMargins: (float)yMargins;

/** <p>Put aView in the GSTable, using the specified margins.
  The names for the margins are chosen as to be as close as possible to
  the autoresizingMask convention.  The margins are to be interpreted as
  follows:</p>
  <deflist>
  <term> minXMargin </term>
  <desc>Left Margin</desc>
  <term> maxXMargin </term>
  <desc>Right Margin</desc>
  <term> minYMargin </term>
  <desc>Lower Margin (Upper if view is flipped)</desc>
  <term> maxYMargin </term>
  <desc>Upper Margin (Lower if view is flipped)</desc>
  </deflist>

  <p>
  Each view which is added to the GSTable can have some margins
  set.  The GSTable treats the view and its margins as a whole.
  They are given (as a whole) some space, which is reduced or
  increased (but only if X or Y Resizing is Enabled for the column
  or the row in which the view resides) when the GSTable is
  resized.  When this happens, the space is added (or subtracted)
  to the view or to the margins according to the autoResizeMask of
  the view.
  </p>
*/
-(void) putView: (NSView *)aView
	  atRow: (int)row
	 column: (int)column
 withMinXMargin: (float)minXMargin   // Left Margin 
     maxXMargin: (float)maxXMargin   // Right Margin
     minYMargin: (float)minYMargin   // Lower Margin (Upper if flipped)
     maxYMargin: (float)maxYMargin;  // Upper Margin (Lower if flipped)
//
// Minimum Size. 
/** This returns the minimum size the GSTable should be resized to. 
  Trying to resize the GSTable below this size will only result in clipping 
  (ie, making it disappear) part of the GSTable.
*/
-(NSSize) minimumSize;
//
// Resizing. 
/** If for any reason you need the GSTable to be redrawn (with minimum size), 
    invoke the following. */
-(void) sizeToFit;
//
// Setting Row and Column Expand Flag
//
/** Enable/disable X Resizing for the column aColumn}
according to aFlag.  Note: at present, enabling/disabling 
X resizing after the table has been put in the view hierarchy 
is not supported.  
*/
-(void) setXResizingEnabled: (BOOL)aFlag 
		  forColumn: (int)aColumn;

/** Return whether X resizing is enabled for the column aColumn. */
-(BOOL) isXResizingEnabledForColumn: (int)aColumn;

/** Enable/disable Y Resizing for the row aRow 
according to aFlag.  Note: at present, enabling/disabling 
Y resizing after the table has been put in the view hierarchy 
is not supported.
*/
-(void) setYResizingEnabled: (BOOL)aFlag 
		     forRow: (int)aRow;

/** Return whether Y resizing is enabled for the row aRow. */
-(BOOL) isYResizingEnabledForRow: (int)aRow;
//
// Adding Rows and Columns
// These should be used to add more rows and columns to the GSTable. 
// Of course it is faster to create a GSTable with the right number of rows 
// and columns from the beginning.
//
/** Add a row to the GSTable.  The row is added void, with zero
    height and Y Resizing enabled. */
-(void) addRow;
// TODO: -(void) insertRow: (int)row;
// TODO: -(void) removeRow: (int)row;

/** Add a column to the GSTable.  The column is added void,
    with zero width and X Resizing enabled. */
-(void) addColumn;
// TODO: -(void) insertColumn: (int)column;
// TODO: -(void) removeColumn: (int)column;
//
// Getting Row and Column Number
//
/** Return the number of rows in the GSTable.  */
-(int) numberOfRows;

/** Return the number of columns in the GSTable.  */
-(int) numberOfColumns;
@end

#endif /* _GNUstep_H_GSTable */





