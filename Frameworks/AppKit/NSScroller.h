/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSControl.h>

typedef enum {
    NSScrollerNoPart = 0,
    NSScrollerIncrementLine,
    NSScrollerDecrementLine,
    NSScrollerIncrementPage,
    NSScrollerDecrementPage,
    NSScrollerKnob,
    NSScrollerKnobSlot
} NSScrollerPart;

typedef enum {
    NSScrollerIncrementArrow,
    NSScrollerDecrementArrow
} NSScrollerArrow;

typedef enum {
    NSScrollerArrowsMaxEnd = 0,
    NSScrollerArrowsMinEnd = 1,
    NSScrollerArrowsNone = 2,
    NSScrollerArrowsDefaultSetting = NSScrollerArrowsMaxEnd,
} NSScrollArrowPosition;

enum {
    NSNoScrollerParts = 0,
    NSOnlyScrollerArrows = 1,
    NSAllScrollerParts = 2,
};
typedef NSUInteger NSUsableScrollerParts;

@interface NSScroller : NSControl {
    id _target;
    SEL _action;
    struct {
        unsigned isHoriz : 1;
        NSUsableScrollerParts partsUsable : 2;
    } sFlags;

    float _floatValue;
    float _knobProportion;
    NSScrollArrowPosition _arrowsPosition;
    NSControlSize _controlSize;

    NSScrollerPart _hitPart;
    BOOL _isEnabled;
    BOOL _isHighlighted;
}

+ (float)scrollerWidth;

- (float)knobProportion;
- (NSScrollArrowPosition)arrowsPosition;
- (NSControlSize)controlSize;

- (void)setFloatValue:(float)zeroToOneValue knobProportion:(float)zeroToOneKnob;
- (void)setArrowsPosition:(NSScrollArrowPosition)position;
- (void)setControlSize:(NSControlSize)value;

- (NSRect)rectForPart:(NSScrollerPart)part;
- (void)checkSpaceForParts;
- (NSUsableScrollerParts)usableParts;

- (void)highlight:(BOOL)flag;

- (void)drawKnobSlotInRect:(NSRect)rect highlight:(BOOL)flag;
- (void)drawParts;
- (void)drawArrow:(NSScrollerArrow)arrow highlight:(BOOL)flag;
- (void)drawKnob;

- (NSScrollerPart)hitPart;
- (NSScrollerPart)testPart:(NSPoint)point;
- (void)trackKnob:(NSEvent *)event;
- (void)trackScrollButtons:(NSEvent *)event;

@end
