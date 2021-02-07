/* -*-objc-*-
   NSTextTable.h

   Copyright (C) 2008 Free Software Foundation, Inc.

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

#ifndef _GNUstep_H_NSTextTable
#define _GNUstep_H_NSTextTable
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSColor;
@class NSTextContainer;
@class NSView;
@class NSLayoutManager;
@class NSTextTableBlock;

typedef enum _NSTextBlockValueType
{
	NSTextBlockAbsoluteValueType,
	NSTextBlockPercentageValueType
} NSTextBlockValueType;

typedef enum _NSTextBlockDimension
{
	NSTextBlockWidth,
	NSTextBlockMinimumWidth,	
	NSTextBlockMaximumWidth,
	NSTextBlockHeight,
	NSTextBlockMinimumHeight,	
	NSTextBlockMaximumHeight
} NSTextBlockDimension;

typedef enum _NSTextBlockLayer
{
	NSTextBlockPadding,	
	NSTextBlockBorder,
	NSTextBlockMargin
} NSTextBlockLayer;

typedef enum _NSTextBlockVerticalAlignment
{
	NSTextBlockTopAlignment,
	NSTextBlockMiddleAlignment,
	NSTextBlockBottomAlignment,
	NSTextBlockBaselineAlignment
} NSTextBlockVerticalAlignment;

@interface NSTextBlock : NSObject <NSCoding, NSCopying>
{
  NSColor *_backgroundColor;
  NSColor *_borderColorForEdge[NSMaxYEdge + 1];
  NSTextBlockVerticalAlignment _verticalAlignment;
  // The following ivars come in pairs
  CGFloat _value[NSTextBlockMaximumHeight + 1];
  NSTextBlockValueType _valueType[NSTextBlockMaximumHeight + 1];
  CGFloat _width[NSTextBlockMargin + 1][NSMaxYEdge + 1];
  NSTextBlockValueType _widthType[NSTextBlockMargin + 1][NSMaxYEdge + 1];
}

- (NSColor *) backgroundColor;
- (NSColor *) borderColorForEdge: (NSRectEdge)edge;
- (NSRect) boundsRectForContentRect: (NSRect)cont
                             inRect: (NSRect)rect
                      textContainer: (NSTextContainer *)container
                     characterRange: (NSRange)range;
- (CGFloat) contentWidth;
- (NSTextBlockValueType) contentWidthValueType;
- (void) drawBackgroundWithFrame: (NSRect)rect
                          inView: (NSView *)view 
                  characterRange: (NSRange)range
                   layoutManager: (NSLayoutManager *)lm;
- (id) init; 
- (NSRect) rectForLayoutAtPoint: (NSPoint)point
                         inRect: (NSRect)rect
                  textContainer: (NSTextContainer *)cont
                 characterRange: (NSRange)range;
- (void) setBackgroundColor: (NSColor *)color;
- (void) setBorderColor: (NSColor *)color;
- (void) setBorderColor: (NSColor *)color forEdge: (NSRectEdge)edge;
- (void) setContentWidth: (CGFloat)val type: (NSTextBlockValueType)type;
- (void) setValue: (CGFloat)val 
             type: (NSTextBlockValueType)type
     forDimension: (NSTextBlockDimension)dimension;
- (void) setVerticalAlignment: (NSTextBlockVerticalAlignment)alignment;
- (void) setWidth: (CGFloat)val
             type: (NSTextBlockValueType)type 
         forLayer: (NSTextBlockLayer)layer;
- (void) setWidth: (CGFloat)val
             type: (NSTextBlockValueType)type
         forLayer: (NSTextBlockLayer)layer
             edge: (NSRectEdge)edge;
- (CGFloat) valueForDimension: (NSTextBlockDimension)dimension;
- (NSTextBlockValueType) valueTypeForDimension: (NSTextBlockDimension)dimension;
- (NSTextBlockVerticalAlignment) verticalAlignment;
- (CGFloat) widthForLayer: (NSTextBlockLayer)layer edge: (NSRectEdge)edge;
- (NSTextBlockValueType) widthValueTypeForLayer: (NSTextBlockLayer)layer
                                           edge: (NSRectEdge)edge;
@end

typedef enum _NSTextTableLayoutAlgorithm {
	NSTextTableAutomaticLayoutAlgorithm,
	NSTextTableFixedLayoutAlgorithm
} NSTextTableLayoutAlgorithm;

@interface NSTextTable : NSTextBlock
{
  NSTextTableLayoutAlgorithm _layoutAlgorithm;
  NSUInteger _numberOfColumns;
  BOOL _collapsesBorders;
  BOOL _hidesEmptyCells;
}

- (NSRect) boundsRectForBlock: (NSTextTableBlock *)block
                  contentRect: (NSRect)content
                       inRect: (NSRect)rect
                textContainer: (NSTextContainer *)container
               characterRange: (NSRange)range;
- (BOOL) collapsesBorders;
- (void) drawBackgroundForBlock: (NSTextTableBlock *)block
                      withFrame: (NSRect)frame
                         inView: (NSView *)controlView
                 characterRange: (NSRange)range
                  layoutManager: (NSLayoutManager *)manager;
- (BOOL) hidesEmptyCells;
- (NSTextTableLayoutAlgorithm) layoutAlgorithm;
- (NSUInteger) numberOfColumns;
- (NSRect) rectForBlock: (NSTextTableBlock *)block
          layoutAtPoint: (NSPoint)start
                 inRect: (NSRect)rect
          textContainer: (NSTextContainer *)container
         characterRange: (NSRange)range;
- (void) setCollapsesBorders: (BOOL)flag;
- (void) setHidesEmptyCells: (BOOL)flag;
- (void) setLayoutAlgorithm: (NSTextTableLayoutAlgorithm)algorithm;
- (void) setNumberOfColumns: (NSUInteger)numCols;

@end

@interface NSTextTableBlock : NSTextBlock
{
  NSTextTable *_table;
  int _row;
  int _rowSpan;
  int _col;
  int _colSpan;
}

- (id) initWithTable: (NSTextTable *)table
         startingRow: (int)row
             rowSpan: (int)rspan
      startingColumn: (int)col
          columnSpan: (int)cspan;
- (int) columnSpan;
- (int) rowSpan;
- (int) startingColumn;
- (int) startingRow;
- (NSTextTable *) table;

@end

#endif 

#endif // _GNUstep_H_NSTextTable
