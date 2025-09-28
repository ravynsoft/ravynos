/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSView.h>

@class NSClipView, NSScroller, NSColor, NSRulerView;

@interface NSScrollView : NSView {
    NSClipView *_clipView;
    NSClipView *_headerClipView;
    NSView *_cornerView;
    NSScroller *_verticalScroller;
    NSScroller *_horizontalScroller;
    NSRulerView *_horizontalRuler;
    NSRulerView *_verticalRuler;
    NSColor *_backgroundColor;
    float _verticalLineScroll;
    float _verticalPageScroll;
    float _horizontalLineScroll;
    float _horizontalPageScroll;
    int _borderType;
    BOOL _drawsBackground;
    BOOL _hasVerticalScroller;
    BOOL _hasHorizontalScroller;
    BOOL _hasHorizontalRuler;
    BOOL _hasVerticalRuler;
    BOOL _rulersVisible;
    BOOL _scrollsDynamically;
    BOOL _autohidesScrollers;
    NSCursor *_documentCursor;
}

+ (NSSize)frameSizeForContentSize:(NSSize)contentSize hasHorizontalScroller:(BOOL)hasHorizontalScroller hasVerticalScroller:(BOOL)hasVerticalScroller borderType:(NSBorderType)borderType;
+ (NSSize)contentSizeForFrameSize:(NSSize)fSize hasHorizontalScroller:(BOOL)hasHorizontalScroller hasVerticalScroller:(BOOL)hasVerticalScroller borderType:(NSBorderType)borderType;

+ (void)setRulerViewClass:(Class)aClass;
+ (Class)rulerViewClass;

- (NSSize)contentSize;

- documentView;
- (NSClipView *)contentView;
- (NSRect)documentVisibleRect;

- (BOOL)drawsBackground;
- (NSColor *)backgroundColor;
- (NSBorderType)borderType;
- (NSScroller *)verticalScroller;
- (NSScroller *)horizontalScroller;
- (void)setVerticalRulerView:(NSRulerView *)ruler;
- (NSRulerView *)verticalRulerView;
- (void)setHorizontalRulerView:(NSRulerView *)ruler;
- (NSRulerView *)horizontalRulerView;
- (BOOL)hasVerticalScroller;
- (BOOL)hasHorizontalScroller;
- (BOOL)hasVerticalRuler;
- (BOOL)hasHorizontalRuler;
- (BOOL)rulersVisible;
- (float)verticalLineScroll;
- (float)horizontalLineScroll;
- (float)verticalPageScroll;
- (float)horizontalPageScroll;
- (float)lineScroll;
- (float)pageScroll;
- (BOOL)scrollsDynamically;
- (BOOL)autohidesScrollers;

- (NSCursor *)documentCursor;

- (void)setDocumentView:(NSView *)view;
- (void)setContentView:(NSClipView *)clipView;
- (void)setDrawsBackground:(BOOL)value;
- (void)setBackgroundColor:(NSColor *)color;
- (void)setBorderType:(NSBorderType)borderType;
- (void)setVerticalScroller:(NSScroller *)scroller;
- (void)setHorizontalScroller:(NSScroller *)scroller;
- (void)setHasVerticalScroller:(BOOL)flag;
- (void)setHasHorizontalScroller:(BOOL)flag;
- (void)setHasVerticalRuler:(BOOL)flag;
- (void)setHasHorizontalRuler:(BOOL)flag;
- (void)setRulersVisible:(BOOL)flag;
- (void)setVerticalLineScroll:(float)value;
- (void)setHorizontalLineScroll:(float)value;
- (void)setVerticalPageScroll:(float)value;
- (void)setHorizontalPageScroll:(float)value;
- (void)setLineScroll:(float)value;
- (void)setPageScroll:(float)value;
- (void)setScrollsDynamically:(BOOL)flag;
- (void)setDocumentCursor:(NSCursor *)cursor;
- (void)setAutohidesScrollers:(BOOL)value;

- (void)tile;
- (void)reflectScrolledClipView:(NSClipView *)clipView;

@end
