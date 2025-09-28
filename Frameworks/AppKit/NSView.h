/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>
                 2010 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Reviewed for API completeness against 10.6.

#import <AppKit/NSResponder.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSAnimation.h>
#import <AppKit/AppKitExport.h>
#import <ApplicationServices/ApplicationServices.h>

@class NSWindow, NSMenu, NSMenuItem, NSCursor, NSClipView, NSPasteboard, NSTextInputContext, NSImage, NSBitmapImageRep, NSScrollView, NSTrackingArea, NSShadow, NSScreen, CALayer, CIFilter, CALayerContext;

// See Cocoa Event Handling Guide : Using Tracking-Area Objects : Compatibility Issues
typedef NSTrackingArea *NSTrackingRectTag;
typedef NSTrackingArea *NSToolTipTag;

enum {
    NSViewNotSizable = 0x00,
    NSViewMinXMargin = 0x01,
    NSViewWidthSizable = 0x02,
    NSViewMaxXMargin = 0x04,
    NSViewMinYMargin = 0x08,
    NSViewHeightSizable = 0x10,
    NSViewMaxYMargin = 0x20
};

typedef enum {
    NSNoBorder,
    NSLineBorder,
    NSBezelBorder,
    NSGrooveBorder
} NSBorderType;

enum {
    NSViewLayerContentsPlacementScaleAxesIndependently = 0,
    NSViewLayerContentsPlacementScaleProportionallyToFit,
    NSViewLayerContentsPlacementScaleProportionallyToFill,
    NSViewLayerContentsPlacementCenter,
    NSViewLayerContentsPlacementTop,
    NSViewLayerContentsPlacementTopRight,
    NSViewLayerContentsPlacementRight,
    NSViewLayerContentsPlacementBottomRight,
    NSViewLayerContentsPlacementBottom,
    NSViewLayerContentsPlacementBottomLeft,
    NSViewLayerContentsPlacementLeft,
    NSViewLayerContentsPlacementTopLeft
};
typedef NSInteger NSViewLayerContentsPlacement;

enum {
    NSViewLayerContentsRedrawNever = 0,
    NSViewLayerContentsRedrawOnSetNeedsDisplay,
    NSViewLayerContentsRedrawDuringViewResize,
    NSViewLayerContentsRedrawBeforeViewResize
};
typedef NSInteger NSViewLayerContentsRedrawPolicy;

APPKIT_EXPORT NSString *const NSViewFrameDidChangeNotification;
APPKIT_EXPORT NSString *const NSViewBoundsDidChangeNotification;
APPKIT_EXPORT NSString *const NSViewFocusDidChangeNotification;

@interface NSView : NSResponder <NSAnimatablePropertyContainer> {
    NSRect _frame;
    NSRect _bounds;
    NSWindow *_window;
    NSMenu *_menu;
    NSView *_superview;
    NSMutableArray *_subviews;
    NSView *_nextKeyView;
    NSView *_previousKeyView;
    BOOL _isHidden;
    BOOL _postsNotificationOnFrameChange;
    BOOL _postsNotificationOnBoundsChange;
    BOOL _autoresizesSubviews;
    BOOL _inLiveResize;
    unsigned _autoresizingMask;
    int _tag;
    NSArray *_draggedTypes;
    NSMutableArray *_trackingAreas;
    BOOL _needsDisplay;
    NSUInteger _invalidRectCount;
    NSRect *_invalidRects;
    NSRect *_rectsBeingRedrawn;
    NSUInteger _rectsBeingRedrawnCount;
    CGFloat _frameRotation;
    CGFloat _boundsRotation;

    BOOL _validTrackingAreas;
    BOOL _validTransforms;
    CGAffineTransform _transformFromWindow;
    CGAffineTransform _transformToWindow;
    CGAffineTransform _transformToLayer;
    NSRect _visibleRect;
    NSFocusRingType _focusRingType;

    BOOL _wantsLayer;
    CALayer *_layer;
    NSArray *_contentFilters;
    CIFilter *_compositingFilter;
    NSViewLayerContentsPlacement _layerContentsPlacement;
    NSViewLayerContentsRedrawPolicy _layerContentsRedrawPolicy;
    NSShadow *_shadow;
    NSDictionary *_animations;

    CALayerContext *_layerContext;
    id __remove;
}

+ (NSView *)focusView;
+ (NSMenu *)defaultMenu;
+ (NSFocusRingType)defaultFocusRingType;

- initWithFrame:(NSRect)frame;

- (NSRect)frame;
- (CGFloat)frameRotation;
- (CGFloat)frameCenterRotation;
- (NSRect)bounds;
- (CGFloat)boundsRotation;
- (BOOL)isRotatedFromBase;
- (BOOL)isRotatedOrScaledFromBase;
- (void)translateOriginToPoint:(NSPoint)point;
- (void)rotateByAngle:(CGFloat)angle;
- (BOOL)postsFrameChangedNotifications;
- (BOOL)postsBoundsChangedNotifications;

- (void)scaleUnitSquareToSize:(NSSize)size;

- (NSWindow *)window;
- (NSView *)superview;
- (BOOL)isDescendantOf:(NSView *)other;
- (NSView *)ancestorSharedWithView:(NSView *)view;
- (NSScrollView *)enclosingScrollView;
- (NSRect)adjustScroll:(NSRect)toRect;

- (NSArray *)subviews;
- (BOOL)autoresizesSubviews;
- (unsigned)autoresizingMask;
- (NSFocusRingType)focusRingType;

- (int)tag;
- (BOOL)isFlipped;
- (BOOL)isOpaque;
- (CGFloat)alphaValue;
- (void)setAlphaValue:(CGFloat)alpha;
- (int)gState;
- (NSRect)visibleRect;
- (BOOL)wantsDefaultClipping;

- (NSBitmapImageRep *)bitmapImageRepForCachingDisplayInRect:(NSRect)rect;
- (void)cacheDisplayInRect:(NSRect)rect toBitmapImageRep:(NSBitmapImageRep *)imageRep;

- (BOOL)isHidden;
- (BOOL)isHiddenOrHasHiddenAncestor;
- (void)setHidden:(BOOL)flag;
- (void)viewDidHide;
- (void)viewDidUnhide;

- (BOOL)canBecomeKeyView;
- (BOOL)needsPanelToBecomeKey;

- (NSView *)nextKeyView;
- (NSView *)nextValidKeyView;

- (NSView *)previousKeyView;
- (NSView *)previousValidKeyView;

- (NSMenu *)menuForEvent:(NSEvent *)event;
- (NSMenuItem *)enclosingMenuItem;
- (NSString *)toolTip;

- viewWithTag:(int)tag;
- (NSView *)hitTest:(NSPoint)point;

- (NSPoint)convertPoint:(NSPoint)point fromView:(NSView *)viewOrNil;
- (NSPoint)convertPoint:(NSPoint)point toView:(NSView *)viewOrNil;
- (NSSize)convertSize:(NSSize)size fromView:(NSView *)viewOrNil;
- (NSSize)convertSize:(NSSize)size toView:(NSView *)viewOrNil;
- (NSRect)convertRect:(NSRect)rect fromView:(NSView *)viewOrNil;
- (NSRect)convertRect:(NSRect)rect toView:(NSView *)viewOrNil;
- (NSRect)centerScanRect:(NSRect)rect;

- (void)setFrame:(NSRect)frame;
- (void)setFrameSize:(NSSize)size;
- (void)setFrameOrigin:(NSPoint)origin;
- (void)setFrameRotation:(CGFloat)angle;
- (void)setFrameCenterRotation:(CGFloat)angle;

- (void)setBounds:(NSRect)bounds;
- (void)setBoundsSize:(NSSize)size;
- (void)setBoundsOrigin:(NSPoint)origin;
- (void)setBoundsRotation:(CGFloat)angle;

- (CGFloat)frameRotation;
- (CGFloat)boundsRotation;

- (void)setFrameRotation:(CGFloat)degrees;
- (void)setBoundsRotation:(CGFloat)degrees;
- (void)rotateByAngle:(CGFloat)degrees;

- (void)setPostsFrameChangedNotifications:(BOOL)flag;
- (void)setPostsBoundsChangedNotifications:(BOOL)flag;

- (void)addSubview:(NSView *)view;
- (void)addSubview:(NSView *)view positioned:(NSWindowOrderingMode)ordering relativeTo:(NSView *)relativeTo;
- (void)replaceSubview:(NSView *)oldView with:(NSView *)newView;
- (void)setSubviews:(NSArray *)newSubviews;
- (void)sortSubviewsUsingFunction:(NSComparisonResult (*)(id, id, void *))compareFunction context:(void *)context;
- (void)didAddSubview:(NSView *)subview;
- (void)willRemoveSubview:(NSView *)subview;
- (void)setAutoresizesSubviews:(BOOL)flag;
- (void)setAutoresizingMask:(unsigned int)mask;
- (void)setFocusRingType:(NSFocusRingType)value;

- (void)setNextKeyView:(NSView *)next;
- (BOOL)acceptsFirstMouse:(NSEvent *)event;
- (BOOL)acceptsTouchEvents;
- (void)setAcceptsTouchEvents:(BOOL)accepts;
- (BOOL)wantsRestingTouches;
- (void)setWantsRestingTouches:(BOOL)wants;

- (BOOL)performKeyEquivalent:(NSEvent *)event;
- (BOOL)performMnemonic:(NSString *)string;

- (void)setToolTip:(NSString *)string;
- (NSToolTipTag)addToolTipRect:(NSRect)rect owner:object userData:(void *)userData;
- (void)removeToolTip:(NSToolTipTag)tag;
- (void)removeAllToolTips;

- (void)addCursorRect:(NSRect)rect cursor:(NSCursor *)cursor;
- (void)removeCursorRect:(NSRect)rect cursor:(NSCursor *)cursor;
- (void)discardCursorRects;
- (void)resetCursorRects;

- (NSArray *)trackingAreas;
- (void)addTrackingArea:(NSTrackingArea *)trackingArea;
- (void)removeTrackingArea:(NSTrackingArea *)trackingArea;
- (void)updateTrackingAreas;

- (NSTrackingRectTag)addTrackingRect:(NSRect)rect owner:object userData:(void *)userData assumeInside:(BOOL)assumeInside;
- (void)removeTrackingRect:(NSTrackingRectTag)tag;

- (NSTextInputContext *)inputContext;

- (void)registerForDraggedTypes:(NSArray *)types;
- (void)unregisterDraggedTypes;
- (NSArray *)registeredDraggedTypes;

- (void)removeFromSuperview;
- (void)removeFromSuperviewWithoutNeedingDisplay;
- (void)viewWillMoveToSuperview:(NSView *)view;
- (void)viewDidMoveToSuperview;

- (void)viewWillMoveToWindow:(NSWindow *)window;
- (void)viewDidMoveToWindow;

- (BOOL)shouldDelayWindowOrderingForEvent:(NSEvent *)event;

- (void)resizeSubviewsWithOldSize:(NSSize)oldSize;
- (void)resizeWithOldSuperviewSize:(NSSize)oldSize;

- (BOOL)inLiveResize;
- (BOOL)preservesContentDuringLiveResize;
- (NSRect)rectPreservedDuringLiveResize;
- (void)viewWillStartLiveResize;
- (void)viewDidEndLiveResize;

- (BOOL)enterFullScreenMode:(NSScreen *)screen withOptions:(NSDictionary *)options;
- (BOOL)isInFullScreenMode;
- (void)exitFullScreenModeWithOptions:(NSDictionary *)options;

- (void)scrollPoint:(NSPoint)point;
- (BOOL)scrollRectToVisible:(NSRect)rect;
- (void)scrollClipView:(NSClipView *)clipView toPoint:(NSPoint)newOrigin;
- (BOOL)mouse:(NSPoint)point inRect:(NSRect)rect;
- (void)reflectScrolledClipView:(NSClipView *)view;

- (void)allocateGState;
- (void)releaseGState;
- (void)setUpGState;
- (void)renewGState;

- (CALayer *)layer;
- (void)setLayer:(CALayer *)newLayer;
- (BOOL)wantsLayer;
- (void)setWantsLayer:(BOOL)wantsLayer;
- (NSViewLayerContentsPlacement)layerContentsPlacement;
- (void)setLayerContentsPlacement:(NSViewLayerContentsPlacement)newPlacement;
- (NSViewLayerContentsRedrawPolicy)layerContentsRedrawPolicy;
- (void)setLayerContentsRedrawPolicy:(NSViewLayerContentsRedrawPolicy)newPolicy;
- (CALayer *)makeBackingLayer;

- (NSArray *)backgroundFilters;
- (void)setBackgroundFilters:(NSArray *)filters;
- (NSArray *)contentFilters;
- (void)setContentFilters:(NSArray *)filters;
- (CIFilter *)compositingFilter;
- (void)setCompositingFilter:(CIFilter *)filter;
- (NSShadow *)shadow;
- (void)setShadow:(NSShadow *)shadow;

- (BOOL)needsDisplay;
- (void)setNeedsDisplay:(BOOL)flag;
- (void)setNeedsDisplayInRect:(NSRect)rect;
- (void)setKeyboardFocusRingNeedsDisplayInRect:(NSRect)rect;
- (void)translateRectsNeedingDisplayInRect:(NSRect)rect by:(NSSize)delta;

- (BOOL)canDraw;
- (BOOL)canDrawConcurrently;
- (void)viewWillDraw;
- (void)setCanDrawConcurrently:(BOOL)canDraw;
- (void)lockFocus;
- (BOOL)lockFocusIfCanDraw;
- (BOOL)lockFocusIfCanDrawInContext:(NSGraphicsContext *)context;
- (void)unlockFocus;

- (BOOL)needsToDrawRect:(NSRect)rect;
- (void)getRectsBeingDrawn:(const NSRect **)rects count:(NSInteger *)count;
- (void)getRectsExposedDuringLiveResize:(NSRect)rects count:(NSInteger *)count;

- (BOOL)shouldDrawColor;

- (NSView *)opaqueAncestor;
- (void)display;
- (void)displayIfNeeded;
- (void)displayIfNeededIgnoringOpacity;
- (void)displayIfNeededInRect:(NSRect)rect;
- (void)displayIfNeededInRectIgnoringOpacity:(NSRect)rect;
- (void)displayRect:(NSRect)rect;
- (void)displayRectIgnoringOpacity:(NSRect)rect;
- (void)displayRectIgnoringOpacity:(NSRect)rect inContext:(NSGraphicsContext *)context;
- (void)drawRect:(NSRect)rect;

- (BOOL)autoscroll:(NSEvent *)event;
- (void)scrollRect:(NSRect)rect by:(NSSize)delta;
- (BOOL)mouseDownCanMoveWindow;

- (void)print:sender;

- (void)beginDocument;
- (void)endDocument;

- (void)beginPageInRect:(NSRect)rect atPlacement:(NSPoint)placement;
- (void)endPage;

- (NSAttributedString *)pageHeader;
- (NSAttributedString *)pageFooter;
- (NSString *)printJobTitle;
- (void)drawSheetBorderWithSize:(NSSize)size;
- (void)drawPageBorderWithSize:(NSSize)size;

- (float)widthAdjustLimit;
- (float)heightAdjustLimit;
- (void)adjustPageWidthNew:(float *)adjustedRight left:(float)left right:(float)right limit:(float)limit;
- (void)adjustPageHeightNew:(float *)adjustedBottom top:(float)top bottom:(float)bottom limit:(float)limit;

- (BOOL)knowsPageRange:(NSRange *)range;
- (NSPoint)locationOfPrintRect:(NSRect)rect;
- (NSRect)rectForPage:(int)page;

- (NSData *)dataWithEPSInsideRect:(NSRect)rect;
- (NSData *)dataWithPDFInsideRect:(NSRect)rect;

- (void)writeEPSInsideRect:(NSRect)rect toPasteboard:(NSPasteboard *)pasteboard;
- (void)writePDFInsideRect:(NSRect)rect toPasteboard:(NSPasteboard *)pasteboard;

- (void)dragImage:(NSImage *)image at:(NSPoint)location offset:(NSSize)offset event:(NSEvent *)event pasteboard:(NSPasteboard *)pasteboard source:source slideBack:(BOOL)slideBack;
- (BOOL)dragFile:(NSString *)path fromRect:(NSRect)rect slideBack:(BOOL)slideBack event:(NSEvent *)event;
- (BOOL)dragPromisedFilesOfTypes:(NSArray *)types fromRect:(NSRect)rect source:(id)source slideBack:(BOOL)slideBack event:(NSEvent *)event;

- (NSPoint)convertPointFromBase:(NSPoint)aPoint;
- (NSPoint)convertPointToBase:(NSPoint)aPoint;
- (NSSize)convertSizeFromBase:(NSSize)aSize;
- (NSSize)convertSizeToBase:(NSSize)aSize;
- (NSRect)convertRectFromBase:(NSRect)aRect;
- (NSRect)convertRectToBase:(NSRect)aRect;

- (void)showDefinitionForAttributedString:(NSAttributedString *)string atPoint:(NSPoint)origin;
// Blocks aren't supported by the compiler yet.
//-(void)showDefinitionForAttributedString:(NSAttributedString *)string range:(NSRange)range options:(NSDictionary *)options baselineOriginProvider:(NSPoint (^)(NSRange adjustedRange))originProvider;

// private,move
- (NSArray *)_draggedTypes;
- (void)_setWindow:(NSWindow *)window;
- (void)_collectTrackingAreasForWindowInto:(NSMutableArray *)collector;

@end

@interface NSObject (NSView_toolTipOwner)
- (NSString *)view:(NSView *)view stringForToolTip:(NSToolTipTag)tag point:(NSPoint)point userData:(void *)data;
@end
