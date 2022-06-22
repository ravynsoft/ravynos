/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2010 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Reviewed for API completeness against 10.6.

#import <AppKit/NSResponder.h>
#import <AppKit/AppKitExport.h>
#import <AppKit/NSView.h>
#import <ApplicationServices/ApplicationServices.h>

@class NSView, NSEvent, NSColor, NSColorSpace, NSCursor, NSImage, NSScreen, NSText, NSTextView, CGWindow, NSPasteboard, NSSheetContext, NSUndoManager, NSButton, NSButtonCell, NSDrawer, NSDockTile, NSToolbar, NSWindowAnimationContext, NSTrackingArea, NSThemeFrame, NSWindowController, NSMenuItem, CARenderer;

enum {
    NSBorderlessWindowMask = 0x00,
    NSTitledWindowMask = 0x01,
    NSClosableWindowMask = 0x02,
    NSMiniaturizableWindowMask = 0x04,
    NSResizableWindowMask = 0x08,
    NSTexturedBackgroundWindowMask = 0x100,
};

// Support for wayland layer-shell & subsurface; ignored on other backends
// NSWindow type enums occupy the low 2 bytes so we'll stay above that
enum {
    WLWindowLayerShellMask = 0x80000000,
    WLWindowLayerAnchorMask = 0xF000,
    WLWindowLayerAnchorTop = 0x80001000,
    WLWindowLayerAnchorBottom = 0x80002000,
    WLWindowLayerAnchorLeft = 0x80004000,
    WLWindowLayerAnchorRight = 0x80008000,
    WLWindowLayerKeyboardMask = 0xF0000,
    WLWindowLayerKeyboardNone = 0x80000000,
    WLWindowLayerKeyboardExclusive = 0x80010000,
    WLWindowLayerKeyboardOnDemand = 0x80020000,
    WLWindowLayerMask = 0xF00000,
    WLWindowLayerBackground = 0x80000000,
    WLWindowLayerBottom = 0x80100000,
    WLWindowLayerTop = 0x80200000,
    WLWindowLayerOverlay = 0x80300000,
    WLWindowPopUp = 0x40000000
};


typedef enum {
    NSBackingStoreRetained = 0,
    NSBackingStoreNonretained = 1,
    NSBackingStoreBuffered = 2,
} NSBackingStoreType;

typedef enum {
    NSWindowCloseButton,
    NSWindowMiniaturizeButton,
    NSWindowZoomButton,
    NSWindowToolbarButton,
    NSWindowDocumentIconButton,
} NSWindowButton;

enum {
    NSWindowNumberListAllApplications = 0x01,
    NSWindowNumberListAllSpaces = 0x10
};
typedef NSUInteger NSWindowNumberListOptions;

enum {
    NSWindowBackingLocationDefault = 0x00,
    NSWindowBackingLocationVideoMemory = 0x01,
    NSWindowBackingLocationMainMemory = 0x02
};
typedef NSUInteger NSWindowBackingLocation;

enum {
    NSNormalWindowLevel = kCGNormalWindowLevel,
    NSFloatingWindowLevel = kCGFloatingWindowLevel,
    NSSubmenuWindowLevel = kCGTornOffMenuWindowLevel,
    NSTornOffMenuWindowLevel = kCGTornOffMenuWindowLevel,
    NSMainMenuWindowLevel = kCGMainMenuWindowLevel,
    NSStatusWindowLevel = kCGStatusWindowLevel,
    NSModalPanelWindowLevel = kCGModalPanelWindowLevel,
    NSPopUpMenuWindowLevel = kCGPopUpMenuWindowLevel,
    NSScreenSaverWindowLevel = kCGScreenSaverWindowLevel,
};

enum {
    NSWindowCollectionBehaviorDefault = 0x00,
    NSWindowCollectionBehaviorCanJoinAllSpaces = 0x01,
    NSWindowCollectionBehaviorMoveToActiveSpace = 0x02,
    NSWindowCollectionBehaviorManaged = 0x04,
    NSWindowCollectionBehaviorTransient = 0x08,
    NSWindowCollectionBehaviorStationary = 0x10,
    NSWindowCollectionBehaviorParticipatesInCycle = 0x20,
    NSWindowCollectionBehaviorIgnoresCycle = 0x40
};
typedef NSUInteger NSWindowCollectionBehavior;

enum {
    NSWindowSharingNone = 0x00,
    NSWindowSharingReadOnly = 0x01,
    NSWindowSharingReadWrite = 0x02
};
typedef NSUInteger NSWindowSharingType;

typedef int NSSelectionDirection;

APPKIT_EXPORT NSString *const NSWindowDidBecomeKeyNotification;
APPKIT_EXPORT NSString *const NSWindowDidResignKeyNotification;
APPKIT_EXPORT NSString *const NSWindowDidBecomeMainNotification;
APPKIT_EXPORT NSString *const NSWindowDidResignMainNotification;
APPKIT_EXPORT NSString *const NSWindowWillMiniaturizeNotification;
APPKIT_EXPORT NSString *const NSWindowDidMiniaturizeNotification;
APPKIT_EXPORT NSString *const NSWindowDidDeminiaturizeNotification;
APPKIT_EXPORT NSString *const NSWindowWillMoveNotification;
APPKIT_EXPORT NSString *const NSWindowDidMoveNotification;
APPKIT_EXPORT NSString *const NSWindowDidResizeNotification;
APPKIT_EXPORT NSString *const NSWindowDidUpdateNotification;
APPKIT_EXPORT NSString *const NSWindowWillCloseNotification;
APPKIT_EXPORT NSString *const NSWindowWillStartLiveResizeNotification;
APPKIT_EXPORT NSString *const NSWindowDidEndLiveResizeNotification;

@interface NSWindow : NSResponder {
    NSRect _frame;
    NSUInteger _styleMask;
    NSBackingStoreType _backingType;
    NSInteger _level;

    NSSize _minSize;
    NSSize _maxSize;
    NSSize _contentMinSize;
    NSSize _contentMaxSize;

    NSWindow *_parentWindow;
    NSMutableArray *_childWindows;

    NSString *_representedFilename;
    NSString *_title;
    NSString *_miniwindowTitle;
    NSImage *_miniwindowImage;

    NSThemeFrame *_backgroundView;
    NSMenu *_menu;
    NSView *_menuView;
    NSView *_contentView;
    NSColor *_backgroundColor;

    id _delegate;
    NSResponder *_firstResponder;

    NSTextView *_sharedFieldEditor;
    NSTextView *_currentFieldEditor;
    NSArray *_draggedTypes;

    NSMutableArray *_trackingAreas;

    NSSheetContext *_sheetContext;

    NSSize _resizeInfo;

    int _cursorRectsDisabled;
    int _flushDisabled;

    BOOL _isOpaque;
    BOOL _isVisible;
    BOOL _isDocumentEdited;
    BOOL _makeSureIsOnAScreen;

    BOOL _acceptsMouseMovedEvents;
    BOOL _excludedFromWindowsMenu;
    BOOL _isDeferred;
    BOOL _dynamicDepthLimit;
    BOOL _canStoreColor;
    BOOL _isOneShot;
    BOOL _useOptimizedDrawing;
    BOOL _releaseWhenClosed;
    BOOL _hidesOnDeactivate;
    BOOL _hiddenForDeactivate;
    BOOL _isActive;
    BOOL _viewsNeedDisplay;
    BOOL _flushNeeded;
    BOOL _isFlushWindowDisabled;
    BOOL _isAutodisplay;
    BOOL _canHide;
    BOOL _displaysWhenScreenProfileChanges;

    CGFloat _alphaValue;
    BOOL _hasShadow;
    BOOL _showsResizeIndicator;
    BOOL _showsToolbarButton;
    BOOL _ignoresMouseEvents;
    BOOL _isMovableByWindowBackground;
    BOOL _allowsToolTipsWhenApplicationIsInactive;
    BOOL _defaultButtonCellKeyEquivalentDisabled;
    BOOL _autorecalculatesKeyViewLoop;
    BOOL _hasBeenOnScreen;

    BOOL _isInLiveResize;
    BOOL _preservesContentDuringLiveResize;
    NSSize _resizeIncrements;
    NSSize _contentResizeIncrements;

    NSString *_autosaveFrameName;

    CGWindow *_platformWindow;
    NSMutableDictionary *_threadToContext;

    NSUndoManager *_undoManager;
    NSView *_initialFirstResponder;
    NSButtonCell *_defaultButtonCell;

    NSWindowController *_windowController;
    NSMutableArray *_drawers;
    NSToolbar *_toolbar;
    NSWindowAnimationContext *_animationContext;

    NSRect _savedFrame;
    NSPoint _mouseDownLocationInWindow;

    NSScreen *_preferredScreen;
}

+ (NSWindowDepth)defaultDepthLimit;

+ (NSRect)frameRectForContentRect:(NSRect)contentRect styleMask:(unsigned)styleMask;
+ (NSRect)contentRectForFrameRect:(NSRect)frameRect styleMask:(unsigned)styleMask;
+ (float)minFrameWidthWithTitle:(NSString *)title styleMask:(unsigned)styleMask;
+ (NSInteger)windowNumberAtPoint:(NSPoint)point belowWindowWithWindowNumber:(NSInteger)window;
+ (NSArray *)windowNumbersWithOptions:(NSWindowNumberListOptions)options;
+ (void)removeFrameUsingName:(NSString *)name;

+ (NSButton *)standardWindowButton:(NSWindowButton)button forStyleMask:(unsigned)styleMask;
+ (void)menuChanged:(NSMenu *)menu;

- initWithContentRect:(NSRect)contentRect styleMask:(unsigned)styleMask backing:(unsigned)backing defer:(BOOL)defer;
- initWithContentRect:(NSRect)contentRect styleMask:(unsigned)styleMask backing:(unsigned)backing defer:(BOOL)defer screen:(NSScreen *)screen;
- (NSWindow *)initWithWindowRef:(void *)carbonRef;

- (NSGraphicsContext *)graphicsContext;
- (NSDictionary *)deviceDescription;
- (void *)windowRef;
- (BOOL)allowsConcurrentViewDrawing;
- (void)setAllowsConcurrentViewDrawing:(BOOL)allows;

- (NSView *)contentView;
- (id)delegate;

- (NSString *)title;
- (NSString *)representedFilename;
- (NSURL *)representedURL;

- (NSInteger)level;
- (NSRect)frame;
- (unsigned)styleMask;
- (NSBackingStoreType)backingType;
- (NSWindowBackingLocation)preferredBackingLocation;
- (void)setPreferredBackingLocation:(NSWindowBackingLocation)location;
- (NSWindowBackingLocation)backingLocation;

- (NSSize)minSize;
- (NSSize)maxSize;
- (NSSize)contentMinSize;
- (NSSize)contentMaxSize;

- (BOOL)isOneShot;
- (BOOL)isOpaque;
- (BOOL)hasDynamicDepthLimit;
- (BOOL)isReleasedWhenClosed;
- (BOOL)preventsApplicationTerminationWhenModal;
- (void)setPreventsApplicationTerminationWhenModal:(BOOL)prevents;
- (BOOL)hidesOnDeactivate;
- (BOOL)worksWhenModal;
- (BOOL)isSheet;
- (BOOL)acceptsMouseMovedEvents;
- (BOOL)isExcludedFromWindowsMenu;
- (BOOL)isAutodisplay;
- (BOOL)isFlushWindowDisabled;
- (NSString *)frameAutosaveName;
- (BOOL)hasShadow;
- (BOOL)ignoresMouseEvents;
- (NSSize)aspectRatio;
- (NSSize)contentAspectRatio;
- (BOOL)autorecalculatesKeyViewLoop;
- (BOOL)canHide;
- (BOOL)canStoreColor;
- (BOOL)showsResizeIndicator;
- (BOOL)showsToolbarButton;
- (BOOL)displaysWhenScreenProfileChanges;
- (BOOL)isMovableByWindowBackground;
- (BOOL)allowsToolTipsWhenApplicationIsInactive;

- (BOOL)autorecalculatesContentBorderThicknessForEdge:(NSRectEdge)edge;
- (CGFloat)contentBorderThicknessForEdge:(NSRectEdge)edge;

- (NSImage *)miniwindowImage;
- (NSString *)miniwindowTitle;
- (NSDockTile *)dockTile;
- (NSColor *)backgroundColor;
- (CGFloat)alphaValue;
- (NSWindowDepth)depthLimit;
- (NSSize)resizeIncrements;
- (NSSize)contentResizeIncrements;
- (BOOL)preservesContentDuringLiveResize;
- (NSToolbar *)toolbar;
- (NSView *)initialFirstResponder;

- (void)setDelegate:delegate;
- (void)setFrame:(NSRect)frame display:(BOOL)display;
- (void)setFrame:(NSRect)frame display:(BOOL)display animate:(BOOL)flag;
- (void)setContentSize:(NSSize)contentSize;
- (void)setFrameOrigin:(NSPoint)point;
- (void)setFrameTopLeftPoint:(NSPoint)point;
- (void)setStyleMask:(NSUInteger)styleMask;
- (void)setMinSize:(NSSize)size;
- (void)setMaxSize:(NSSize)size;
- (void)setContentMinSize:(NSSize)value;
- (void)setContentMaxSize:(NSSize)value;
- (void)setContentBorderThickness:(CGFloat)thickness forEdge:(NSRectEdge)edge;
- (void)setMovable:(BOOL)movable;

- (void)setBackingType:(NSBackingStoreType)value;
- (void)setDynamicDepthLimit:(BOOL)value;
- (void)setOneShot:(BOOL)flag;
- (void)setReleasedWhenClosed:(BOOL)flag;
- (void)setHidesOnDeactivate:(BOOL)flag;
- (void)setAcceptsMouseMovedEvents:(BOOL)flag;
- (void)setExcludedFromWindowsMenu:(BOOL)value;
- (void)setAutodisplay:(BOOL)value;
- (void)setAutorecalculatesContentBorderThickness:(BOOL)automatic forEdge:(NSRectEdge)edge;
- (void)setTitle:(NSString *)title;
- (void)setTitleWithRepresentedFilename:(NSString *)filename;
- (void)setContentView:(NSView *)view;

- (void)setInitialFirstResponder:(NSView *)view;
- (void)setMiniwindowImage:(NSImage *)image;
- (void)setMiniwindowTitle:(NSString *)title;
- (void)setBackgroundColor:(NSColor *)color;
- (void)setAlphaValue:(CGFloat)value;
- (void)setToolbar:(NSToolbar *)toolbar;
- (void)setDefaultButtonCell:(NSButtonCell *)cell;
- (void)setWindowController:(NSWindowController *)value;
- (void)setDocumentEdited:(BOOL)flag;
- (void)setContentAspectRatio:(NSSize)value;
- (void)setHasShadow:(BOOL)value;
- (void)setIgnoresMouseEvents:(BOOL)value;
- (void)setAspectRatio:(NSSize)value;
- (void)setAutorecalculatesKeyViewLoop:(BOOL)value;
- (void)setCanHide:(BOOL)value;
- (void)setCanBecomeVisibleWithoutLogin:(BOOL)flag;
- (void)setCollectionBehavior:(NSWindowCollectionBehavior)behavior;
- (void)setLevel:(NSInteger)value;
- (void)setOpaque:(BOOL)value;
- (void)setParentWindow:(NSWindow *)value;
- (void)setPreservesContentDuringLiveResize:(BOOL)value;
- (void)setRepresentedFilename:(NSString *)value;
- (void)setRepresentedURL:(NSURL *)newURL;
- (void)setResizeIncrements:(NSSize)value;
- (void)setShowsResizeIndicator:(BOOL)value;
- (void)setShowsToolbarButton:(BOOL)value;
- (void)setContentResizeIncrements:(NSSize)value;
- (void)setDepthLimit:(NSWindowDepth)value;
- (void)setDisplaysWhenScreenProfileChanges:(BOOL)value;
- (void)setMovableByWindowBackground:(BOOL)value;
- (void)setAllowsToolTipsWhenApplicationIsInactive:(BOOL)value;

- (BOOL)setFrameUsingName:(NSString *)name;
- (BOOL)setFrameUsingName:(NSString *)name force:(BOOL)force;
- (BOOL)setFrameAutosaveName:(NSString *)name;
- (void)saveFrameUsingName:(NSString *)name;
- (void)setFrameFromString:(NSString *)value;
- (NSString *)stringWithSavedFrame;

- (int)resizeFlags;
- (float)userSpaceScaleFactor;
- (NSResponder *)firstResponder;

- (NSButton *)standardWindowButton:(NSWindowButton)value;
- (NSButtonCell *)defaultButtonCell;
- (NSWindow *)attachedSheet;

- (id)windowController;
- (NSArray *)drawers;

- (int)windowNumber;
- (int)gState;
- (NSScreen *)screen;
- (NSScreen *)deepestScreen;
- (NSColorSpace *)colorSpace;
- (void)setColorSpace:(NSColorSpace *)newColorSpace;
- (BOOL)isOnActiveSpace;
- (NSWindowSharingType)sharingType;
- (void)setSharingType:(NSWindowSharingType)type;

- (BOOL)isDocumentEdited;
- (BOOL)isZoomed;
- (BOOL)isVisible;
- (BOOL)isKeyWindow;
- (BOOL)isMainWindow;
- (BOOL)isMiniaturized;
- (BOOL)isMovable;
- (BOOL)inLiveResize;
- (BOOL)canBecomeKeyWindow;
- (BOOL)canBecomeMainWindow;
- (BOOL)canBecomeVisibleWithoutLogin;
- (NSWindowCollectionBehavior)collectionBehavior;

- (NSPoint)convertBaseToScreen:(NSPoint)point;
- (NSPoint)convertScreenToBase:(NSPoint)point;

- (NSRect)frameRectForContentRect:(NSRect)rect;
- (NSRect)contentRectForFrameRect:(NSRect)rect;
- (NSRect)constrainFrameRect:(NSRect)rect toScreen:(NSScreen *)screen;

- (NSWindow *)parentWindow;
- (NSArray *)childWindows;
- (void)addChildWindow:(NSWindow *)child ordered:(NSWindowOrderingMode)ordered;
- (void)removeChildWindow:(NSWindow *)child;

- (BOOL)makeFirstResponder:(NSResponder *)responder;

- (void)makeKeyWindow;
- (void)makeMainWindow;

- (void)becomeKeyWindow;
- (void)resignKeyWindow;
- (void)becomeMainWindow;
- (void)resignMainWindow;

- (NSTimeInterval)animationResizeTime:(NSRect)frame;

- (void)selectNextKeyView:sender;
- (void)selectPreviousKeyView:sender;
- (void)selectKeyViewFollowingView:(NSView *)view;
- (void)selectKeyViewPrecedingView:(NSView *)view;
- (void)recalculateKeyViewLoop;
- (NSSelectionDirection)keyViewSelectionDirection;

- (void)disableKeyEquivalentForDefaultButtonCell;
- (void)enableKeyEquivalentForDefaultButtonCell;

- (NSText *)fieldEditor:(BOOL)create forObject:object;
- (void)endEditingFor:object;

- (void)disableScreenUpdatesUntilFlush;
- (void)useOptimizedDrawing:(BOOL)flag;
- (BOOL)viewsNeedDisplay;
- (void)setViewsNeedDisplay:(BOOL)flag;
- (void)disableFlushWindow;
- (void)enableFlushWindow;
- (void)flushWindow;
- (void)flushWindowIfNeeded;
- (void)displayIfNeeded;
- (void)display;

- (void)invalidateShadow;

- (void)cacheImageInRect:(NSRect)rect;
- (void)restoreCachedImage;
- (void)discardCachedImage;

- (BOOL)areCursorRectsEnabled;
- (void)disableCursorRects;
- (void)enableCursorRects;
- (void)discardCursorRects;
- (void)resetCursorRects;
- (void)invalidateCursorRectsForView:(NSView *)view;

- (void)close;
- (void)center;
- (void)orderWindow:(NSWindowOrderingMode)place relativeTo:(int)relativeTo;
- (void)orderFrontRegardless;

- (NSPoint)mouseLocationOutsideOfEventStream;

- (NSEvent *)currentEvent;
- (NSEvent *)nextEventMatchingMask:(unsigned)mask;
- (NSEvent *)nextEventMatchingMask:(unsigned)mask untilDate:(NSDate *)untilDate inMode:(NSString *)mode dequeue:(BOOL)dequeue;
- (void)discardEventsMatchingMask:(unsigned)mask beforeEvent:(NSEvent *)event;

- (void)sendEvent:(NSEvent *)event;
- (void)postEvent:(NSEvent *)event atStart:(BOOL)atStart;

- (BOOL)tryToPerform:(SEL)selector with:object;
- (void)keyDown:(NSEvent *)event;

- (NSPoint)cascadeTopLeftFromPoint:(NSPoint)topLeftPoint;

- (NSData *)dataWithEPSInsideRect:(NSRect)rect;
- (NSData *)dataWithPDFInsideRect:(NSRect)rect;

- (void)registerForDraggedTypes:(NSArray *)types;
- (void)unregisterDraggedTypes;

- (void)dragImage:(NSImage *)image at:(NSPoint)location offset:(NSSize)offset event:(NSEvent *)event pasteboard:(NSPasteboard *)pasteboard source:source slideBack:(BOOL)slideBack;

- validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType;

- (void)update;

- (void)makeKeyAndOrderFront:sender;
- (void)orderFront:sender;
- (void)orderBack:sender;
- (void)orderOut:sender;

- (void)performClose:sender;
- (void)performMiniaturize:sender;
- (void)performZoom:sender;

- (void)zoom:sender;
- (void)miniaturize:sender;
- (void)deminiaturize:sender;
- (void)print:sender;

- (void)toggleToolbarShown:sender;
- (void)runToolbarCustomizationPalette:sender;

// semi-private platform support for layer-shell
- (void)setKeyboardInteractivity:(uint32_t)keyboardStyle;

@end

@interface NSObject (NSWindow_delegate)

- (void)windowWillBeginSheet:(NSNotification *)note;
- (void)windowDidEndSheet:(NSNotification *)note;
- (NSRect)window:(NSWindow *)window willPositionSheet:(NSWindow *)sheet usingRect:(NSRect)rect;

- (void)windowDidChangeScreen:(NSNotification *)note;
- (void)windowDidChangeScreenProfile:(NSNotification *)note;
- (void)windowDidExpose:(NSNotification *)note;
- (BOOL)windowShouldZoom:(NSWindow *)sender toFrame:(NSRect)frame;

- windowWillReturnFieldEditor:(NSWindow *)sender toObject:object;

- (NSRect)windowWillUseStandardFrame:(NSWindow *)sender defaultFrame:(NSRect)frame;

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window;
- (void)windowDidBecomeKey:(NSNotification *)note;
- (void)windowDidResignKey:(NSNotification *)note;
- (void)windowDidBecomeMain:(NSNotification *)note;
- (void)windowDidResignMain:(NSNotification *)note;
- (void)windowWillMiniaturize:(NSNotification *)note;
- (void)windowDidMiniaturize:(NSNotification *)note;
- (void)windowDidDeminiaturize:(NSNotification *)note;
- (void)windowWillMove:(NSNotification *)note;
- (void)windowDidMove:(NSNotification *)note;
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)size;
- (void)windowDidResize:(NSNotification *)note;
- (void)windowDidUpdate:(NSNotification *)note;
- (BOOL)windowShouldClose:sender;
- (void)windowWillClose:(NSNotification *)note;

@end

//private
APPKIT_EXPORT NSString *const NSWindowWillAnimateNotification;
APPKIT_EXPORT NSString *const NSWindowAnimatingNotification;
APPKIT_EXPORT NSString *const NSWindowDidAnimateNotification;
