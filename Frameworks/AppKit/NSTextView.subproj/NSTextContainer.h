/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/Foundation.h>

@class NSLayoutManager, NSTextView;

typedef enum {
    NSLineSweepUp,
    NSLineSweepDown,
    NSLineSweepLeft,
    NSLineSweepRight,
} NSLineSweepDirection;

typedef enum {
    NSLineDoesntMove,
    NSLineMovesUp,
    NSLineMovesDown,
    NSLineMovesLeft,
    NSLineMovesRight,
} NSLineMovementDirection;

@interface NSTextContainer : NSObject {
    NSSize _size;
    NSTextView *_textView;
    NSLayoutManager *_layoutManager;
    float _lineFragmentPadding;
    BOOL _widthTracksTextView;
    BOOL _heightTracksTextView;
}

- initWithContainerSize:(NSSize)size;

- (NSSize)containerSize;

- (NSTextView *)textView;
- (BOOL)widthTracksTextView;
- (BOOL)heightTracksTextView;

- (NSLayoutManager *)layoutManager;

- (float)lineFragmentPadding;

- (void)setContainerSize:(NSSize)size;

- (void)setTextView:(NSTextView *)textView;
- (void)setWidthTracksTextView:(BOOL)flag;
- (void)setHeightTracksTextView:(BOOL)flag;

- (void)setLayoutManager:(NSLayoutManager *)layoutManager;
- (void)replaceLayoutManager:(NSLayoutManager *)layoutManager;

- (void)setLineFragmentPadding:(float)padding;

- (BOOL)isSimpleRectangularTextContainer;

- (BOOL)containsPoint:(NSPoint)point;

- (NSRect)lineFragmentRectForProposedRect:(NSRect)proposed sweepDirection:(NSLineSweepDirection)sweep movementDirection:(NSLineMovementDirection)movement remainingRect:(NSRectPointer)remaining;

@end
