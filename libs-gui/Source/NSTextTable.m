/* NSTextTable.m

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

@implementation NSTextTable

- (BOOL) collapsesBorders
{
  return _collapsesBorders;	// if true: ???
}

- (void) setCollapsesBorders: (BOOL)flag
{
  _collapsesBorders = flag;
}

- (BOOL) hidesEmptyCells
{
  return _hidesEmptyCells;	// if true: don't draw border and fill
}

- (void) setHidesEmptyCells: (BOOL)flag
{
  _hidesEmptyCells = flag;
}

- (NSTextTableLayoutAlgorithm) layoutAlgorithm
{
  return _layoutAlgorithm;
}

- (void) setLayoutAlgorithm: (NSTextTableLayoutAlgorithm)algorithm
{
  _layoutAlgorithm = algorithm;
}

- (NSUInteger) numberOfColumns
{
  return _numberOfColumns;
}

- (void) setNumberOfColumns: (NSUInteger)numCols
{
  _numberOfColumns = numCols;
}

- (NSRect) boundsRectForBlock: (NSTextTableBlock *)block
                  contentRect: (NSRect)content
                       inRect: (NSRect)rect
                textContainer: (NSTextContainer *)container
               characterRange: (NSRange)range
{
  // FIXME
  return NSZeroRect;
}

- (NSRect) rectForBlock: (NSTextTableBlock *)block
          layoutAtPoint: (NSPoint)start
                 inRect: (NSRect)rect
          textContainer: (NSTextContainer *)container
         characterRange: (NSRange)range
{
  // FIXME
  return NSZeroRect;
}

- (void) drawBackgroundForBlock: (NSTextTableBlock *)block
                      withFrame: (NSRect)frame
                         inView: (NSView *)controlView
                 characterRange: (NSRange)range
                  layoutManager: (NSLayoutManager *)manager
{
  // FIXME
}

// are these called for the whole table?

- (NSRect) boundsRectForContentRect: (NSRect)cont
                             inRect: (NSRect)rect
                      textContainer: (NSTextContainer *)container
                     characterRange: (NSRange)range
{
  // FIXME
  return [super boundsRectForContentRect: cont 
                inRect: rect
                textContainer: container
                characterRange: range];
}

- (NSRect) rectForLayoutAtPoint: (NSPoint)point
                         inRect: (NSRect)rect
                  textContainer: (NSTextContainer *)cont
                 characterRange: (NSRange)range
{
  return [super rectForLayoutAtPoint: point
                inRect: rect
                textContainer: cont
                characterRange: range];
}

- (void) drawBackgroundWithFrame: (NSRect)rect
                          inView: (NSView *)view 
                  characterRange: (NSRange)range
                   layoutManager: (NSLayoutManager *)lm
{
  return [super drawBackgroundWithFrame: rect
                inView: view
                characterRange: range
                layoutManager: lm];
}

@end
