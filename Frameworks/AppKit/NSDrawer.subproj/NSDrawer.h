/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSGeometry.h>
#import <AppKit/NSResponder.h>

@class NSString, NSView, NSWindow;
@class NSDrawerWindow;

typedef enum {
    NSDrawerClosedState,
    NSDrawerOpeningState,
    NSDrawerOpenState,
    NSDrawerClosingState
} NSDrawerState;

APPKIT_EXPORT NSString *const NSDrawerWillOpenNotification;
APPKIT_EXPORT NSString *const NSDrawerDidOpenNotification;
APPKIT_EXPORT NSString *const NSDrawerWillCloseNotification;
APPKIT_EXPORT NSString *const NSDrawerDidCloseNotification;

@interface NSDrawer : NSResponder {
    NSSize _contentSize;
    NSDrawerState _state;
    NSRectEdge _edge;
    NSRectEdge _preferredEdge;
    float _leadingOffset;
    float _trailingOffset;
    NSDrawerWindow *_drawerWindow;
    NSWindow *_parentWindow;
    NSWindow *_nextParentWindow;
    id _delegate;
    NSSize _minContentSize;
    NSSize _maxContentSize;
}

// Geometry methods. Not in Apple's AppKit.
+ (NSRect)drawerFrameWithContentSize:(NSSize)contentSize parentWindow:(NSWindow *)parentWindow leadingOffset:(float)leadingOffset trailingOffset:(float)trailingOffset edge:(NSRectEdge)edge state:(NSDrawerState)state;

+ (NSRectEdge)visibleEdgeWithPreferredEdge:(NSRectEdge)preferredEdge parentWindow:(NSWindow *)parentWindow drawerWindow:(NSWindow *)drawerWindow;

- (id)initWithCoder:(NSCoder *)coder;

- (id)initWithContentSize:(NSSize)contentSize preferredEdge:(NSRectEdge)edge;

- (id)delegate;
- (NSWindow *)parentWindow;
- (NSView *)contentView;
- (NSSize)contentSize;
- (NSSize)minContentSize;
- (NSSize)maxContentSize;
- (float)leadingOffset;
- (float)trailingOffset;
- (NSRectEdge)preferredEdge;
- (int)state;
- (NSRectEdge)edge;

- (void)setDelegate:delegate;
- (void)setParentWindow:(NSWindow *)window;
- (void)setContentView:(NSView *)view;
- (void)setContentSize:(NSSize)size;
- (void)setMinContentSize:(NSSize)size;
- (void)setMaxContentSize:(NSSize)size;
- (void)setPreferredEdge:(NSRectEdge)edge;
- (void)setLeadingOffset:(float)offset;
- (void)setTrailingOffset:(float)offset;

- (void)open;
- (void)openOnEdge:(NSRectEdge)edge;
- (void)close;

- (void)open:sender;
- (void)close:sender;
- (void)toggle:sender;

- (void)parentWindowDidActivate:(NSWindow *)window;
- (void)parentWindowDidDeactivate:(NSWindow *)window;
- (void)parentWindowDidChangeFrame:(NSWindow *)window;
- (void)parentWindowDidExitMove:(NSWindow *)window;
- (void)parentWindowDidMiniaturize:(NSWindow *)window;
- (void)parentWindowDidDeminiaturize:(NSWindow *)window;
- (void)parentWindowDidClose:(NSWindow *)window;

- (void)drawerWindowDidActivate:(NSDrawerWindow *)window;
- (NSSize)drawerWindow:(NSDrawerWindow *)window constrainSize:(NSSize)size edge:(NSRectEdge)edge;
- (void)drawerWindowDidResize:(NSDrawerWindow *)window;

@end

@interface NSObject (NSDrawer_delegate)

- (NSSize)drawerWillResizeContents:(NSDrawer *)drawer toSize:(NSSize)contentSize;

- (BOOL)drawerShouldOpen:(NSDrawer *)drawer;
- (void)drawerWillOpen:(NSNotification *)note;
- (void)drawerDidOpen:(NSNotification *)note;

- (BOOL)drawerShouldClose:(NSDrawer *)drawer;
- (void)drawerWillClose:(NSNotification *)note;
- (void)drawerDidClose:(NSNotification *)note;

@end
