/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSView.h>

APPKIT_EXPORT NSString *const NSSplitViewDidResizeSubviewsNotification;
APPKIT_EXPORT NSString *const NSSplitViewWillResizeSubviewsNotification;

typedef enum {
    NSSplitViewDividerStyleThick = 1,
    NSSplitViewDividerStyleThin,
    NSSplitViewDividerStylePaneSplitter,
} NSSplitViewDividerStyle;

@interface NSSplitView : NSView {
    id _delegate;
    BOOL _isVertical;
    NSSplitViewDividerStyle _dividerStyle;
}

- (id)delegate;
- (BOOL)isVertical;

- (void)setDelegate:(id)delegate;
- (void)setVertical:(BOOL)flag;

- (void)adjustSubviews;

- (float)dividerThickness;
- (void)drawDividerInRect:(NSRect)rect;

- (BOOL)isSubviewCollapsed:(NSView *)subview;

- (void)setDividerStyle:(NSSplitViewDividerStyle)style;
- (NSSplitViewDividerStyle)dividerStyle;

/* Get the minimum or maximum possible position of a divider */
- (float)minPossiblePositionOfDividerAtIndex:(int)index;
- (float)maxPossiblePositionOfDividerAtIndex:(int)index;

/** Set the position of a divider */
- (void)setPosition:(float)position ofDividerAtIndex:(int)index;

@end

// nb. it looks like the API for this is changed in MacOS X, replacing splitView:constrainMix:max:ofSubviewAt
// with two individual methods for each.
@interface NSObject (NSSplitView_delegate)
- (BOOL)splitView:(NSSplitView *)splitView canCollapseSubview:(NSView *)subview;
- (BOOL)splitView:(NSSplitView *)splitView shouldCollapseSubview:(NSView *)subview forDoubleClickOnDividerAtIndex:(int)index;
- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMinimumPosition ofSubviewAt:(int)index;
- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(int)index;

- (void)splitView:(NSSplitView *)splitView constrainMinCoordinate:(float *)min maxCoordinate:(float *)max ofSubviewAt:(int)index;
- (float)splitView:(NSSplitView *)splitView constrainSplitPosition:(float)pos ofSubviewAt:(int)index;
- (void)splitView:(NSSplitView *)splitView resizeSubviewsWithOldSize:(NSSize)size;

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view;
- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(int)index;

- (NSRect)splitView:(NSSplitView *)splitView effectiveRect:(NSRect)proposedEffectiveRect forDrawnRect:(NSRect)drawnRect ofDividerAtIndex:(int)index;
- (NSRect)splitView:(NSSplitView *)splitView additionalEffectiveRectOfDividerAtIndex:(NSInteger)index;

- (void)splitViewDidResizeSubviews:(NSNotification *)note;
- (void)splitViewWillResizeSubviews:(NSNotification *)note;
@end
