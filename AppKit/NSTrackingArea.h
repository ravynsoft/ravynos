/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009-2010 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Reviewed for API completeness against 10.6.

#import <Foundation/Foundation.h>

@class NSView;

enum {
    NSTrackingMouseEnteredAndExited = (1 << 0),
    NSTrackingMouseMoved = (1 << 1),
    NSTrackingCursorUpdate = (1 << 2),

    NSTrackingActiveWhenFirstResponder = (1 << 4),
    NSTrackingActiveInKeyWindow = (1 << 5),
    NSTrackingActiveInActiveApp = (1 << 6),
    NSTrackingActiveAlways = (1 << 7),

    NSTrackingAssumeInside = (1 << 8),
    NSTrackingInVisibleRect = (1 << 9),
    NSTrackingEnabledDuringMouseDrag = (1 << 10),
};

typedef NSUInteger NSTrackingAreaOptions;

@interface NSTrackingArea : NSObject {
    NSRect _rect;
    NSTrackingAreaOptions _options;
    id _owner;
    void *_userData;
    BOOL _retainUserData;

    // NSWindow needs this. It's maintained when areas are collected for the window.
    NSView *_view;
    // NSWindow needs this. It's maintained when areas are collected for the window.
    NSRect _rectInWindow;
    // _mouseInside is a marker handled by NSWindow.
    BOOL _mouseInside;
    // Instead of sending events, show the NSToolTipWindow.
    // The text for the tooltip is fetched from owner.
    BOOL _isToolTip;
    // Needed for compatibility with legacy cursorRects. If YES, this area will be
    // discarded by -[NSView discardCursorRects] (and -[NSWindow discardCursorRects]).
    BOOL _legacy;
}

// Apple documented
- (id)initWithRect:(NSRect)rect options:(NSTrackingAreaOptions)options owner:(id)owner userInfo:(NSDictionary *)userInfo;

- (NSRect)rect;
- (NSTrackingAreaOptions)options;
- (id)owner;
- (NSDictionary *)userInfo;

// undocumented
- (id)_initWithRect:(NSRect)rect options:(NSTrackingAreaOptions)options owner:(id)owner userData:(void *)userData retainUserData:(BOOL)retainUserData isToolTip:(BOOL)isToolTip isLegacy:(BOOL)legacy;

//-(void)_setRect:(NSRect)rect;
- (NSRect)_rectInWindow;
- (void)_setRectInWindow:(NSRect)rectInWindow;
- (NSView *)_view;
- (void)_setView:(NSView *)newView;
- (BOOL)_isToolTip;
- (BOOL)_isLegacy;
- (BOOL)_mouseInside;
- (void)_setMouseInside:(BOOL)mouseInside;

@end
