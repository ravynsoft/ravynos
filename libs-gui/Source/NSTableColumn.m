/** <title>NSTableColumn</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 1999
   Completely Rewritten.

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

 <chapter>
  <heading>Overview of NSTableColumn</heading>
  <section>
    <heading>The Column Identifier</heading>
    <p>
    Each NSTableColumn object is identified by an object, called 
    the column identifier.  The reason is that, after a column has been 
    added to a table view, the user might move the columns around, so  
    there is a need to identify the columns regardless of their position 
    in the table.  
    </p>
    <p>
    The identifier is typically a string describing the column.  
    This identifier object is never displayed to the user !   
    It is only used internally by the program to identify 
    the column - so yes, you may use a funny string for it 
    and nobody will know, except people reading the code. 
    </p>
  </section>
  <section>
    <heading>Information Stored in an NSTableColumn Object</heading>
    <p>
    An NSTableColumn object mainly keeps information about the width
    of the column, its minimum and maximum width; whether the column 
    can be edited or resized; and the cells used to draw the column 
    header and the data in the column.  You can change all these 
    attributes of the column by calling the appropriate methods.  
    Please note that the table column does not hold nor has access 
    to the data to be displayed in the column; this data is maintained 
    in the table view's data source, as described in the NSTableView 
    documentation.  A last hint: to set the title of a table column, 
    ask the table column for its header cell, and set the string value 
    of this header cell to the desired title.
    </p>
  </section>
 </chapter>
*/  

#import <Foundation/NSDictionary.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSSortDescriptor.h>
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSTableHeaderCell.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTableView.h"
#import "GSBindingHelpers.h"

/**
  <p>
  NSTableColumn objects represent columns in NSTableViews.  
  </p>
 */
@implementation NSTableColumn

/*
 *
 * Class methods
 *
 */
+ (void) initialize
{
  if (self == [NSTableColumn class])
    {
      [self setVersion: 3];
      [self exposeBinding: NSValueBinding];
      [self exposeBinding: NSEnabledBinding];
    }
}

/*
 *
 * Instance methods
 *
 */

/**
  Initialize the column.  anObject is an object used to identify the
  column; it is usually a string, but might be any kind of object.
  anObject is retained.  */
- (id) initWithIdentifier: (id)anObject
{
  self = [super init];
  if (!self)
      return nil;

  _width = 100;
  _min_width = 10;
  _max_width = 100000;
  _resizing_mask = NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask;
  _is_resizable = YES;
  _is_editable = YES;
  _is_hidden = NO;
  _tableView = nil;

  _headerCell = [[NSTableHeaderCell alloc] init];
  _dataCell = [[NSTextFieldCell alloc] init];
  [_dataCell setLineBreakMode: NSLineBreakByTruncatingTail];
  [_dataCell setEditable: YES];
  _headerToolTip = nil;

  _sortDescriptorPrototype = nil;

  ASSIGN (_identifier, anObject);
  return self;
}

- (void) dealloc
{
  // Remove all key value bindings for this view.
  [GSKeyValueBinding unbindAllForObject: self];

  RELEASE(_headerCell);
  RELEASE(_headerToolTip);
  RELEASE(_dataCell);
  RELEASE(_sortDescriptorPrototype);
  TEST_RELEASE(_identifier);
  [super dealloc];
}

/*
 * Managing the Identifier
 */
/**
  Set the identifier used to identify the table.  The old identifier
  is released, and the new one is retained.  */
- (void) setIdentifier: (id)anObject
{
  ASSIGN (_identifier, anObject);
}

/**
  Return the column identifier, an object used to identify the column.
  This object is usually a string, but might be any kind of object.
  */
- (id) identifier
{
  return _identifier;
}
/*
 * Setting the NSTableView 
 */
/**
  Set the table view corresponding to this table column.  This method
  is invoked internally by the table view, and you should not call it
  directly; it is exposed because you may want to override it in
  subclasses.  To use the table column in a table view, you should use
  NSTableView's addTableColumn: instead.  */
- (void) setTableView: (NSTableView*)aTableView
{
  // We do *not* RETAIN aTableView. 
  // On the contrary, aTableView is supposed to RETAIN us.
  _tableView = aTableView;
}

/**
  Return the table view the column belongs to, or nil if the table
  column was not added to any table view.  */
- (NSTableView *) tableView
{
  return _tableView;
}

/*
 * Controlling size 
 */
/**
  Set the width of the table column.  Before being resized, the new
  width is constrained to the table column minimum and maximum width:
  if newWidth is smaller than the table column's min width, the table
  column is simply resized to its min width.  If newWidth is bigger
  than the table column's max width, the table column is simply
  resized to its max width.  Otherwise, it is resized to newWidth.  If
  the width of the table was actually changed, the table view (if any)
  is redisplayed (by calling tile), and the
  NSTableViewColumnDidResizeNotification is posted on behalf of the
  table view.  */
- (void) setWidth: (CGFloat)newWidth
{
  CGFloat oldWidth = _width;

  if (newWidth > _max_width)
    newWidth = _max_width;
  else if (newWidth < _min_width)
    newWidth = _min_width;

  if (_width == newWidth)
    return;
  
  _width = newWidth;
  
  if (_tableView)
    {
      // Tiling also marks it as needing redisplay
      [_tableView tile];
      
      [[NSNotificationCenter defaultCenter] 
	postNotificationName: NSTableViewColumnDidResizeNotification
	object: _tableView
	userInfo: [NSDictionary dictionaryWithObjectsAndKeys:
				  self, @"NSTableColumn", 
				  [NSNumber numberWithFloat: oldWidth],
				@"NSOldWidth", nil]];
    }
}

/** Return the width of the table column. The 
    default width is 100. */
- (CGFloat) width
{
  return _width;
}

/**
  Set the min width of the table column, eventually adjusting the
  width of the column if it is smaller than the new min width.  In no
  way a table column can be made smaller than its min width.  */
- (void) setMinWidth: (CGFloat)minWidth
{
  _min_width = minWidth;
  if (_width < _min_width)
    [self setWidth: _min_width];
}

/**
  Return the column's min width.  The column can in no way be resized
  to a width smaller than this min width.  The default min width is
  10.  */
- (CGFloat) minWidth
{
  return _min_width;
}

/**
  Set the max width of the table column, eventually adjusting the
  width of the column if it is bigger than the new max width.  In no
  way a table column can be made bigger than its max width.  */
- (void) setMaxWidth: (CGFloat)maxWidth
{
  _max_width = maxWidth;
  if (_width > _max_width)
    [self setWidth: _max_width];
}

/**
  Return the column's max width.  The column can in no way be resized
  to a width bigger than this max width.  The default max width is
  100000.  */
- (CGFloat) maxWidth
{
  return _max_width;
}

/**
  Set whether the user can resize the table column by dragging the
  border of its header with the mouse.  The table column can be
  resized programmatically regardless of this setting.  */
- (void) setResizable: (BOOL)flag
{
  _is_resizable = flag;
  // TODO: To match Cocoa behavior, should be replaced by
  //if (flag)
  //  {
  //    [self setResizingMask: NSTableColumnAutoresizingMask | NSTableColumnUserResizingMask)];
  //  }
  //else
  //  {
  //    [self setResizingMask: NSTableColumnNoResizing];
  //  }
}

/**
  Return whether the column might be resized by the user by dragging
  the column header border.  */
- (BOOL) isResizable
{
  return _is_resizable;
  // TODO: To match Cocoa behavior, should be replaced by
  //return (BOOL)[self autoresizingMask];
}

/**
Return the resizing mask that describes whether the column is resizable and how 
it resizes. */
- (void) setResizingMask: (NSUInteger)resizingMask
{
  _resizing_mask = resizingMask;
}

/**
Set the resizing mask that describes whether the column is resizable and how 
it resizes. */
- (NSUInteger) resizingMask
{
  return _resizing_mask;
}
/**
  Change the width of the column to be just enough to display its
  header; change the minimum width and maximum width to allow the
  column to have this width (if the minimum width is bigger than the
  column header width, it is reduced to it; if the maximum width is
  smaller than the column header width, it is increased to it).  */
- (void) sizeToFit
{
  CGFloat new_width;

  new_width = [_headerCell cellSize].width;

  if (new_width > _max_width)
    _max_width = new_width;

  if (new_width < _min_width)
    _min_width = new_width;
  
  // For easier subclassing we dont do it directly
  [self setWidth: new_width];
}

/**
Set whether the column is invisible or not. */
- (void) setHidden: (BOOL)hidden
{
  _is_hidden = hidden;
}

/**
Return whether the column is invisible or not.

When the column is hidden, it remains present in the column array returned 
by -[NSTableView tableColumns]. */
- (BOOL) isHidden
{
  return _is_hidden;
}

/*
 * Controlling editability 
 */
/**
  Set whether data in the column might be edited by the user by
  double-cliking on them.  */
- (void) setEditable: (BOOL)flag
{
  _is_editable = flag;
}

/**
  Return whether data displayed in the column can be edited by the
  user by double-cliking on them.  */
- (BOOL) isEditable
{
  return _is_editable;
}

/*
 * Setting component cells 
 */
/**
  Set the cell used to display the column header.  aCell can't be nil,
  otherwise a warning will be generated and the method call ignored.
  The old cell is released, the new one is retained.  */
- (void) setHeaderCell: (NSCell*)aCell
{
  if (aCell == nil)
    {
      NSLog (@"Attempt to set a nil headerCell for NSTableColumn");
      return;
    }
  ASSIGN (_headerCell, aCell);
}

/** 
  Return the cell used to display the column title.  The default
  header cell is an NSTableHeaderCell.  */
- (NSCell*) headerCell
{
  return _headerCell;
}

/**
Set the tool tip text displayed when the pointer is in the header area. */
- (void) setHeaderToolTip: (NSString *)aString
{
  ASSIGN(_headerToolTip, aString);
}

/**
Return the toop tip text displayed when the pointer is in the header area. */
- (NSString *) headerToolTip
{
  return _headerToolTip;
}

/**
  Set the cell used to display data in the column.  aCell can't be
  nil, otherwise a warning will be generated and the method ignored.
  The old cell is released, the new one is retained.  If you want to
  change the attributes in which a single row in a column is
  displayed, you should better use a delegate for your NSTableView
  implementing tableView:willDisplayCell:forTableColumn:row:.  */
- (void) setDataCell: (NSCell*)aCell
{
  if (aCell == nil)
    {
      NSLog (@"Attempt to set a nil dataCell for NSTableColumn");
      return;
    }
  ASSIGN (_dataCell, aCell);
}

/** 
  Return the cell used to display data in the column.  The default
  data cell is an NSTextFieldCell.  */
- (NSCell*) dataCell
{
  return _dataCell;
}

- (NSCell*) dataCellForRow: (NSInteger)row
{
  return [self dataCell];
}

/*
 * Sorting
 */

/** 
Return the sort descriptor bound to the column. */
- (NSSortDescriptor *) sortDescriptorPrototype
{
  return _sortDescriptorPrototype;
}

/** Return the sort descriptor bound to the column. 

This sort descriptor will be added to -[NSTableView sortDescriptors] when you 
bind a column to another object and NSCreateSortDescriptorBindingOption is set 
to YES. */
- (void) setSortDescriptorPrototype: (NSSortDescriptor *)aSortDescriptor
{
  ASSIGN(_sortDescriptorPrototype, aSortDescriptor);
}

/*
 * Encoding/Decoding
 */

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: _identifier forKey: @"NSIdentifier"];
      [aCoder encodeObject: _dataCell forKey: @"NSDataCell"];
      [aCoder encodeObject: _headerCell forKey: @"NSHeaderCell"];
      [aCoder encodeBool: _is_resizable forKey: @"NSIsResizable"];
      [aCoder encodeBool: _is_editable forKey: @"NSIsEditable"];
      [aCoder encodeFloat: _max_width forKey: @"NSMaxWidth"];
      [aCoder encodeFloat: _min_width forKey: @"NSMinWidth"];
      [aCoder encodeFloat: _width forKey: @"NSWidth"];
      [aCoder encodeObject: _sortDescriptorPrototype 
              forKey: @"NSSortDescriptorPrototype"];
      [aCoder encodeInt: _resizing_mask forKey: @"NSResizingMask"];
      [aCoder encodeObject: _headerToolTip forKey: @"NSHeaderToolTip"];
      [aCoder encodeBool: _is_hidden forKey: @"NSHidden"];
      [aCoder encodeObject: _tableView forKey: @"NSTableView"];
    }
  else
    {
      [aCoder encodeObject: _identifier];
      
      [aCoder encodeValueOfObjCType: @encode(float) at: &_width];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_min_width];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_max_width];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_is_resizable];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_is_editable];
      
      [aCoder encodeObject: _headerCell];
      [aCoder encodeObject: _dataCell];

      [aCoder encodeObject: _sortDescriptorPrototype];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      id identifier = [aDecoder decodeObjectForKey: @"NSIdentifier"];

      self = [self initWithIdentifier: identifier];
      if (!self)
        return nil;

      if ([aDecoder containsValueForKey: @"NSDataCell"])
        {
          [self setDataCell: [aDecoder decodeObjectForKey: @"NSDataCell"]];
        }
      if ([aDecoder containsValueForKey: @"NSHeaderCell"])
        {
          [self setHeaderCell: [aDecoder decodeObjectForKey: @"NSHeaderCell"]];
        }
      if ([aDecoder containsValueForKey: @"NSIsResizeable"])
        {
          [self setResizable: [aDecoder decodeBoolForKey: @"NSIsResizeable"]];
        }
      if ([aDecoder containsValueForKey: @"NSIsEditable"])
        {
          [self setEditable: [aDecoder decodeBoolForKey: @"NSIsEditable"]];
        }
      else
        {
          [self setEditable: NO];
        }
      if ([aDecoder containsValueForKey: @"NSWidth"])
        {
          [self setWidth: [aDecoder decodeFloatForKey: @"NSWidth"]]; 
        }
      if ([aDecoder containsValueForKey: @"NSMinWidth"])
        {
          [self setMinWidth: [aDecoder decodeFloatForKey: @"NSMinWidth"]];
        }
      if ([aDecoder containsValueForKey: @"NSMaxWidth"])
        {
          [self setMaxWidth: [aDecoder decodeFloatForKey: @"NSMaxWidth"]];
        }
      if ([aDecoder containsValueForKey: @"NSSortDescriptorPrototype"])
        {
          [self setSortDescriptorPrototype: 
                  [aDecoder decodeObjectForKey: @"NSSortDescriptorPrototype"]];
        }
      if ([aDecoder containsValueForKey: @"NSResizingMask"])
        {
          [self setResizingMask: [aDecoder decodeIntForKey: @"NSResizingMask"]];
        }
      if ([aDecoder containsValueForKey: @"NSHeaderToolTip"])
        {
          [self setHeaderToolTip: [aDecoder decodeObjectForKey: @"NSHeaderToolTip"]];
        }
      if ([aDecoder containsValueForKey: @"NSHidden"])
        {
          [self setHidden: [aDecoder decodeBoolForKey: @"NSHidden"]];
        }
      if ([aDecoder containsValueForKey: @"NSTableView"])
        {
          [self setTableView: [aDecoder decodeObjectForKey: @"NSTableView"]];
        }
    }
  else
    {
      int version = [aDecoder versionForClassName: 
				  @"NSTableColumn"];
      
      self = [super init];
      if (!self)
        return nil;

      if (version >= 2)
        {
          _identifier = RETAIN([aDecoder decodeObject]);
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_width];
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_min_width];
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_max_width];
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_is_resizable];
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_is_editable];
          _headerCell = RETAIN([aDecoder decodeObject]);
          _dataCell   = RETAIN([aDecoder decodeObject]);

          if (version >= 3)
            {
              _sortDescriptorPrototype = RETAIN([aDecoder decodeObject]);
            }
        }
      else
        {
          _identifier = RETAIN([aDecoder decodeObject]);
          _headerCell = RETAIN([aDecoder decodeObject]);
          _dataCell   = RETAIN([aDecoder decodeObject]);
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_width];
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_min_width];
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_max_width];
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_is_resizable];
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_is_editable];
        }
    }
  return self;
}

- (void) setValue: (id)anObject forKey: (NSString*)aKey
{
  if ([aKey isEqual: NSValueBinding])
    {
      // Reload data
      [_tableView reloadData];
    }
  else if ([aKey isEqual: NSEnabledBinding])
    {
      // FIXME
    }
  else
    {
      [super setValue: anObject forKey: aKey];
    }
}

- (id) valueForKey: (NSString*)aKey
{
  if ([aKey isEqual: NSValueBinding])
    {
      return nil;
    }
  else if ([aKey isEqual: NSEnabledBinding])
    {
      // FIXME
      return [NSNumber numberWithBool: YES];
    }
  else
    {
      return [super valueForKey: aKey];
    }
}
@end
