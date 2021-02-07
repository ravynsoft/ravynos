/* NSTextTableBlock.m

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author:  H. Nikolaus Schaller
   Date: 2007
   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: January 2008
   
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

#import <Foundation/NSCoder.h>
#import <Foundation/NSString.h>

#import "AppKit/NSTextTable.h"

@implementation NSTextTableBlock

- (id) initWithTable: (NSTextTable *)table
         startingRow: (int)row
             rowSpan: (int)rspan
      startingColumn: (int)col
          columnSpan: (int)cspan;
{
  self = [super init];
  if (self == nil)
    return nil;

  ASSIGN(_table, table);
  _row = row;
  _rowSpan = rspan;
  _col = col;
  _colSpan = cspan;

  return self;
}

- (void) dealloc
{
  RELEASE(_table);
  [super dealloc];
}

- (int) columnSpan
{
  return _colSpan;
}

- (int) rowSpan
{
  return _rowSpan;
}

- (int) startingColumn
{
  return _col;
}

- (int) startingRow
{
  return _row;
}

- (NSTextTable *) table
{
  return _table;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSTextTableBlock *t = [super copyWithZone: zone];

  _table = TEST_RETAIN(_table);

  return t;
}

- (NSRect) boundsRectForContentRect: (NSRect)content
                             inRect: (NSRect)rect
                      textContainer: (NSTextContainer *)container
                     characterRange: (NSRange)range
{
  return [_table boundsRectForBlock: self
                 contentRect: content
                 inRect: rect
                 textContainer: container
                 characterRange: range];
}

- (NSRect) rectForLayoutAtPoint: (NSPoint)point
                         inRect: (NSRect)rect
                  textContainer: (NSTextContainer *)container
                 characterRange: (NSRange)range
{
  return [_table rectForBlock: self
                 layoutAtPoint: point
                 inRect: rect
                 textContainer: container
                 characterRange: range];
}

- (void) drawBackgroundWithFrame: (NSRect)rect
                          inView: (NSView *)view 
                  characterRange: (NSRange)range
                   layoutManager: (NSLayoutManager *)lm
{
  // override to handle span
  // we can ask [_table numberOfColumns] for sizing purposes
  return [super drawBackgroundWithFrame: rect
                inView: view
                characterRange: range
                layoutManager: lm];
}

@end
