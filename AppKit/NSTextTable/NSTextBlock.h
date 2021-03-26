/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSRange.h>

@class NSColor, NSMutableArray, NSTextContainer, NSView, NSLayoutManager;

typedef enum {
    NSTextBlockTopAlignment,
    NSTextBlockMiddleAlignment,
    NSTextBlockBottomAlignment,
    NSTextBlockBaselineAlignment
} NSTextBlockVerticalAlignment;

typedef enum {
    NSTextBlockAbsoluteValueType,
    NSTextBlockPercentageValueType,
} NSTextBlockValueType;

typedef enum {
    NSTextBlockWidth,
    NSTextBlockMinimumWidth,
    NSTextBlockMaximumWidth,
    NSTextBlockHeight,
    NSTextBlockMinimumHeight,
    NSTextBlockMaximumHeight,
} NSTextBlockDimension;

typedef enum {
    NSTextBlockPadding,
    NSTextBlockBorder,
    NSTextBlockMargin,
} NSTextBlockLayer;

@interface NSTextBlock : NSObject {
    NSColor *_backgroundColor;
    NSMutableArray *_borderColors;
    NSTextBlockVerticalAlignment _verticalAlignment;
    float _contentWidth;
    NSTextBlockValueType _contentWidthValueType;
    float _dimensionValues[NSTextBlockMaximumHeight + 1];
    float _dimensionValueTypes[NSTextBlockMaximumHeight + 1];
    float _layerWidths[NSTextBlockMargin + 1][NSMaxYEdge + 1];
    float _layerValueTypes[NSTextBlockMargin + 1][NSMaxYEdge + 1];
}

- init;

- (NSColor *)backgroundColor;
- (NSColor *)borderColorForEdge:(NSRectEdge)edge;
- (NSTextBlockVerticalAlignment)verticalAlignment;
- (float)contentWidth;
- (NSTextBlockValueType)contentWidthValueType;
- (float)valueForDimension:(NSTextBlockDimension)dimension;
- (NSTextBlockValueType)valueTypeForDimension:(NSTextBlockDimension)dimension;
- (float)widthForLayer:(NSTextBlockLayer)layer edge:(NSRectEdge)edge;
- (NSTextBlockValueType)widthValueTypeForLayer:(NSTextBlockLayer)layer edge:(NSRectEdge)edge;

- (void)setBackgroundColor:(NSColor *)color;
- (void)setBorderColor:(NSColor *)color;
- (void)setBorderColor:(NSColor *)color forEdge:(NSRectEdge)edge;
- (void)setVerticalAlignment:(NSTextBlockVerticalAlignment)alignment;
- (void)setContentWidth:(float)width type:(NSTextBlockValueType)type;
- (void)setValue:(float)value type:(NSTextBlockValueType)type forDimension:(NSTextBlockDimension)dimension;
- (void)setWidth:(float)value type:(NSTextBlockValueType)type forLayer:(NSTextBlockLayer)layer;
- (void)setWidth:(float)value type:(NSTextBlockValueType)type forLayer:(NSTextBlockLayer)layer edge:(NSRectEdge)edge;

- (NSRect)rectForLayoutAtPoint:(NSPoint)point inRect:(NSRect)rect textContainer:(NSTextContainer *)textContainer characterRange:(NSRange)range;
- (NSRect)boundsRectForContentRect:(NSRect)contentRect inRect:(NSRect)rect textContainer:(NSTextContainer *)textContainer characterRange:(NSRange)range;

- (void)drawBackgroundWithFrame:(NSRect)frame inView:(NSView *)view characterRange:(NSRange)range layoutManager:(NSLayoutManager *)layoutManager;

@end
