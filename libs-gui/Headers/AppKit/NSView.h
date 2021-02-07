/** <title>NSView</title>

   <abstract>Encapsulates all drawing functionality</abstract>

   Copyright <copy>(C) 1996-2015 Free Software Foundation, Inc.</copy>

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Heavily changed and extended by Ovidiu Predescu <ovidiu@net-community.com>.
   Date: 1997
   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998

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

#ifndef _GNUstep_H_NSView
#define _GNUstep_H_NSView
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSGraphicsContext.h>
#import <AppKit/NSResponder.h>
#import <AppKit/NSUserInterfaceLayout.h>

@class NSArray;
@class NSAttributedString;
@class NSData;
@class NSMutableArray;
@class NSString;

@class NSBitmapImageRep;
@class NSClipView;
@class NSCursor;
@class NSImage;
@class NSMenu;
@class NSPasteboard;
@class NSScrollView;
@class NSView;
@class NSWindow;

typedef NSInteger NSTrackingRectTag;
typedef NSInteger NSToolTipTag;

/** Describes the type of border used by an NSView.
  <list>
   <item>NSNoBorder</item>
   <item>NSLineBorder</item>
   <item>NSBezelBorder</item>
   <item>NSGrooveBorder</item>
  </list>
*/
enum _NSBorderType {
  NSNoBorder,
  NSLineBorder,
  NSBezelBorder,
  NSGrooveBorder
};
typedef NSUInteger NSBorderType;

typedef NSInteger NSViewLayerContentsRedrawPolicy;
enum {
  NSViewLayerContentsRedrawNever = 0,
  NSViewLayerContentsRedrawOnSetNeedsDisplay = 1,
  NSViewLayerContentsRedrawDuringViewResize = 2,
  NSViewLayerContentsRedrawBeforeViewResize = 3,
  NSViewLayerContentsRedrawCrossfade = 4
};

typedef NSInteger NSViewLayerContentsPlacement;
enum {
  NSViewLayerContentsPlacementScaleAxesIndependently = 0,
  NSViewLayerContentsPlacementScaleProportionallyToFit = 1,
  NSViewLayerContentsPlacementScaleProportionallyToFill = 2,
  NSViewLayerContentsPlacementCenter = 3,
  NSViewLayerContentsPlacementTop = 4,
  NSViewLayerContentsPlacementTopRight = 5,
  NSViewLayerContentsPlacementRight = 6,
  NSViewLayerContentsPlacementBottomRight = 7,
  NSViewLayerContentsPlacementBottom = 8,
  NSViewLayerContentsPlacementBottomLeft = 9,
  NSViewLayerContentsPlacementLeft = 10,
  NSViewLayerContentsPlacementTopLeft = 11
};

/*
 * autoresize constants which NSView uses in
 * determining the parts of a view which are
 * resized when the view's superview is resized
 */
enum {
  NSViewNotSizable	= 0,	// view does not resize with its superview
  NSViewMinXMargin	= 1,	// left margin between views can stretch
  NSViewWidthSizable	= 2,	// view's width can stretch
  NSViewMaxXMargin	= 4,	// right margin between views can stretch
  NSViewMinYMargin	= 8,	// bottom margin between views can stretch
  NSViewHeightSizable	= 16,	// view's height can stretch
  NSViewMaxYMargin	= 32 	// top margin between views can stretch
};

/*
 * constants defining if and how a view (or cell) should draw a focus ring
 */
typedef enum _NSFocusRingType {
  NSFocusRingTypeDefault = 0,
  NSFocusRingTypeNone = 1,
  NSFocusRingTypeExterior = 2
} NSFocusRingType;

@interface NSView : NSResponder
{
  NSRect _frame;
  NSRect _bounds;
  id _frameMatrix;
  id _boundsMatrix;
  id _matrixToWindow;
  id _matrixFromWindow;
  id _coreAnimationData;

  NSView* _super_view;
PACKAGE_SCOPE
  NSMutableArray *_sub_views;
@protected
  NSWindow *_window;
PACKAGE_SCOPE
  NSMutableArray *_tracking_rects;
  NSMutableArray *_cursor_rects;
@protected
  NSRect _invalidRect;
  NSRect _visibleRect;
  NSInteger _gstate;
  void *_nextKeyView;
  void *_previousKeyView;
  CGFloat _alphaValue;

@public
  /*
   * Flags for internal use by NSView and it's subclasses.
   */
  struct _rFlagsType {
    unsigned	flipped_view:1;         /* Flipped state the last time we checked. */ 
    unsigned	has_subviews:1;		/* The view has subviews.	*/
    unsigned	has_currects:1;		/* The view has cursor rects.	*/
    unsigned	has_trkrects:1;		/* The view has tracking rects.	*/
    unsigned	has_draginfo:1;		/* View has drag types. 	*/
    unsigned	opaque_view:1;		/* For views whose opacity may	*/
					/* change to keep track of it.	*/
    unsigned	valid_rects:1;		/* Some cursor rects may be ok.	*/
    unsigned	needs_display:1;	/* view needs display.   	*/
    unsigned	has_tooltips:1;		/* The view has tooltips set.	*/
    unsigned	ignores_backing:1;      /* The view does not trigger    */
                                        /* backing flush when drawn     */
  } _rFlags;

  BOOL _is_rotated_from_base;
  BOOL _is_rotated_or_scaled_from_base;
  BOOL _post_frame_changes;
  BOOL _post_bounds_changes;
  BOOL _autoresizes_subviews;
  BOOL _coordinates_valid;
  BOOL _allocate_gstate;
  BOOL _renew_gstate;
  BOOL _is_hidden;
  BOOL _in_live_resize;

  NSUInteger _autoresizingMask;
  NSFocusRingType _focusRingType;
  NSRect _autoresizingFrameError;
}

/*
 * Initializing NSView Objects
 */
- (id) initWithFrame: (NSRect)frameRect;

/*
 * Managing the NSView Hierarchy
 */
- (void) addSubview: (NSView*)aView;
- (void) addSubview: (NSView*)aView
         positioned: (NSWindowOrderingMode)place
         relativeTo: (NSView*)otherView;
- (NSView*) ancestorSharedWithView: (NSView*)aView;
- (BOOL) isDescendantOf: (NSView*)aView;
- (NSView*) opaqueAncestor;
- (void) removeFromSuperviewWithoutNeedingDisplay;
- (void) removeFromSuperview;
#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
- (void) removeSubview: (NSView*)aView;
#endif
- (void) replaceSubview: (NSView*)oldView
                   with: (NSView*)newView;
- (void) sortSubviewsUsingFunction: (NSComparisonResult (*)(id ,id ,void*))compare
			   context: (void*)context;
- (NSArray*) subviews;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void) setSubviews: (NSArray *)newSubviews;
#endif
- (NSView*) superview;
- (NSWindow*) window;
- (void) viewWillMoveToSuperview: (NSView*)newSuper;
- (void) viewWillMoveToWindow: (NSWindow*)newWindow;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) didAddSubview: (NSView *)subview;
- (void) viewDidMoveToSuperview;
- (void) viewDidMoveToWindow;
- (void) willRemoveSubview: (NSView *)subview;
#endif

/*
 * Assigning a Tag
 */
- (NSInteger) tag;
- (id) viewWithTag: (NSInteger)aTag;

/*
 * Modifying the Frame Rectangle
 */
- (CGFloat) frameRotation;
- (NSRect) frame;
- (void) setFrame: (NSRect)frameRect;
- (void) setFrameOrigin: (NSPoint)newOrigin;
- (void) setFrameRotation: (CGFloat)angle;
- (void) setFrameSize: (NSSize)newSize;

/*
 * Modifying the Coordinate System
 */
- (CGFloat) boundsRotation;
- (NSRect) bounds;
- (void) setBounds: (NSRect)aRect;
- (void) setBoundsOrigin: (NSPoint)newOrigin;
- (void) setBoundsRotation: (CGFloat)angle;
- (void) setBoundsSize: (NSSize)newSize;

- (void) translateOriginToPoint: (NSPoint)point;
- (void) scaleUnitSquareToSize: (NSSize)newSize;
- (void) rotateByAngle: (CGFloat)angle;

- (BOOL) isFlipped;
- (BOOL) isRotatedFromBase;
- (BOOL) isRotatedOrScaledFromBase;

/*
 * View Layers
 */

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (CGFloat) alphaValue;
- (void) setAlphaValue: (CGFloat)alpha;
- (CGFloat) frameCenterRotation;
- (void) setFrameCenterRotation:(CGFloat)rot;
#endif

/*
 * Converting Coordinates
 */
- (NSRect) centerScanRect: (NSRect)aRect;
- (NSPoint) convertPoint: (NSPoint)aPoint
		fromView: (NSView*)aView;
- (NSPoint) convertPoint: (NSPoint)aPoint
		  toView: (NSView*)aView;
- (NSRect) convertRect: (NSRect)aRect
	      fromView: (NSView*)aView;
- (NSRect) convertRect: (NSRect)aRect
		toView: (NSView*)aView;
- (NSSize) convertSize: (NSSize)aSize
	      fromView: (NSView*)aView;
- (NSSize) convertSize: (NSSize)aSize
		toView: (NSView*)aView;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (NSPoint) convertPointFromBase: (NSPoint)aPoint;
- (NSPoint) convertPointToBase: (NSPoint)aPoint;
- (NSRect) convertRectFromBase: (NSRect)aRect;
- (NSRect) convertRectToBase: (NSRect)aRect;
- (NSSize) convertSizeFromBase: (NSSize)aSize;
- (NSSize) convertSizeToBase: (NSSize)aSize;
#endif

/*
 * Notifying Ancestor Views
 */
- (void) setPostsFrameChangedNotifications: (BOOL)flag;
- (BOOL) postsFrameChangedNotifications;
- (void) setPostsBoundsChangedNotifications: (BOOL)flag;
- (BOOL) postsBoundsChangedNotifications;

/*
 * Resizing Subviews
 */
- (void) resizeSubviewsWithOldSize: (NSSize)oldSize;
- (void) setAutoresizesSubviews: (BOOL)flag;
- (BOOL) autoresizesSubviews;
- (void) setAutoresizingMask: (NSUInteger)mask;
- (NSUInteger) autoresizingMask;
- (void) resizeWithOldSuperviewSize: (NSSize)oldSize;

/*
 * Focusing
 */
+ (NSView*) focusView;
- (void) lockFocus;
- (void) unlockFocus;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL) lockFocusIfCanDraw;
- (void) lockFocusInRect: (NSRect)rect;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) lockFocusIfCanDrawInContext: (NSGraphicsContext *)context;
#endif

/*
 * Displaying
 */
- (void) display;
- (void) displayIfNeeded;
- (void) displayIfNeededIgnoringOpacity;
- (void) displayIfNeededInRect: (NSRect)aRect;
- (void) displayIfNeededInRectIgnoringOpacity: (NSRect)aRect;
- (void) displayRect: (NSRect)aRect;
- (void) displayRectIgnoringOpacity: (NSRect)aRect;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) displayRectIgnoringOpacity: (NSRect)aRect 
                          inContext: (NSGraphicsContext *)context;
#endif
- (BOOL) needsDisplay;
- (void) setNeedsDisplay: (BOOL)flag;
- (void) setNeedsDisplayInRect: (NSRect)invalidRect;
- (BOOL) isOpaque;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
+ (NSFocusRingType) defaultFocusRingType;
- (void) setKeyboardFocusRingNeedsDisplayInRect: (NSRect)rect;
- (void) setFocusRingType: (NSFocusRingType)focusRingType;
- (NSFocusRingType) focusRingType;

/*
 * Hidding Views
 */
- (void) setHidden: (BOOL)flag;
- (BOOL) isHidden;
- (BOOL) isHiddenOrHasHiddenAncestor;
#endif

- (void) drawRect: (NSRect)rect;
- (NSRect) visibleRect;
- (BOOL) canDraw;
- (BOOL) shouldDrawColor;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSBitmapImageRep *) bitmapImageRepForCachingDisplayInRect: (NSRect)rect;
- (void) cacheDisplayInRect: (NSRect)rect 
           toBitmapImageRep: (NSBitmapImageRep *)bitmap;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL) wantsDefaultClipping;
- (BOOL) needsToDrawRect: (NSRect)aRect;
- (void) getRectsBeingDrawn: (const NSRect **)rects count: (NSInteger *)count;

/*
 * Live resize support
 */
- (BOOL) inLiveResize;
- (void) viewWillStartLiveResize;
- (void) viewDidEndLiveResize;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) preservesContentDuringLiveResize;
- (void) getRectsExposedDuringLiveResize: (NSRect[4])exposedRects count: (NSInteger *)count;
- (NSRect) rectPreservedDuringLiveResize;
#endif


/*
 * Graphics State Objects
 */
- (void) allocateGState;
- (void) releaseGState;
- (NSInteger) gState;
- (void) renewGState;
- (void) setUpGState;

- (BOOL) acceptsFirstMouse: (NSEvent*)theEvent;
- (NSView*) hitTest: (NSPoint)aPoint;
- (BOOL) mouse: (NSPoint)aPoint
	inRect: (NSRect)aRect;
- (BOOL) performKeyEquivalent: (NSEvent*)theEvent;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL) performMnemonic: (NSString *)aString;
- (BOOL) mouseDownCanMoveWindow;
#endif

/*
 * Dragging
 */
- (BOOL) dragFile: (NSString*)filename
	 fromRect: (NSRect)rect
	slideBack: (BOOL)slideFlag
	    event: (NSEvent*)event;
- (void) dragImage: (NSImage*)anImage
		at: (NSPoint)viewLocation
	    offset: (NSSize)initialOffset
	     event: (NSEvent*)event
	pasteboard: (NSPasteboard*)pboard
	    source: (id)sourceObject
	 slideBack: (BOOL)slideFlag;
- (void) registerForDraggedTypes: (NSArray*)newTypes;
- (void) unregisterDraggedTypes;
- (BOOL) shouldDelayWindowOrderingForEvent: (NSEvent*)anEvent;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (BOOL) dragPromisedFilesOfTypes: (NSArray *)typeArray
                         fromRect: (NSRect)aRect
                           source: (id)sourceObject 
                        slideBack: (BOOL)slideBack
                            event: (NSEvent *)theEvent;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSArray *) registeredDraggedTypes;
#endif

/**
 * Adds a cursor rectangle.  This provides for automatic update of the
 * cursor to be anObject while the mouse is in aRect.<br />
 * The cursor (anObject) is retained until the cursor rectangle is
 * removed or discarded.<br />
 * Generally your subviews should call this in their implementation of
 * the -resetCursorRects method.<br />
 * It is your responsibility to ensure that aRect lies within your veiw's
 * visible rectangle.
 */
- (void) addCursorRect: (NSRect)aRect
                cursor: (NSCursor*)anObject;

/**
 * Removes all the cursor rectancles which have been set up for the
 * receiver.  This is equivalent to calling -removeCursorRect:cursor:
 * for all cursor rectangles which have been set up.<br />
 * This is called automatically before the system calls -resetCursorRects
 * so you never need to call it.
 */
- (void) discardCursorRects;

/**
 * Removes the cursor rectangle which was set up for the specified
 * rectangle and cursor.
 */
- (void) removeCursorRect: (NSRect)aRect
                   cursor: (NSCursor*)anObject;

/** <override-subclass/>
 * This is called to establish a new set of cursor rectangles whenever
 * the receiver needs to do so (eg the view has been resized).  The default
 * implementation does nothing, but subclasses should use it to make
 * calls to -addCursorRect:cursor: to establish their new cursor rectangles.
 */
- (void) resetCursorRects;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/*
 * Tool Tips
 */
- (NSToolTipTag) addToolTipRect: (NSRect)aRect 
                          owner: (id)anObject 
                       userData: (void *)data;
- (void) removeAllToolTips;
- (void) removeToolTip: (NSToolTipTag)tag;
- (void) setToolTip: (NSString *)string;
- (NSString *) toolTip;
#endif

/**
 * Removes a tracking rectangle which was previously established using the
 * -addTrackingRect:owner:userData:assumeInside: method.<br />
 * The value of tag must be the value returned by the method used to add
 * the rectangle.
 */
- (void) removeTrackingRect: (NSTrackingRectTag)tag;

/**
 * Adds a tracking rectangle to monitor mouse movement and generate
 * mouse-entered and mouse-exited events.<br />
 * The event messages are sent to anObject, which is <em>not</em>
 * retained and must therefore be sure to remove any tracking rectangles
 * using it before it is deallocated.<br />
 * The value of data is supplied as the user data in the event objects
 * generated.<br />
 * If flag is YES then the mouse is assumed to be inside the tracking
 * rectangle and the first event generated will therefore be a mouse exit,
 * if it is NO then the first event generated will be a mouse entry.
 */
- (NSTrackingRectTag) addTrackingRect: (NSRect)aRect
                                owner: (id)anObject
                             userData: (void*)data
                         assumeInside: (BOOL)flag;

/*
 * Scrolling
 */
- (NSRect) adjustScroll: (NSRect)newVisible;
- (BOOL) autoscroll: (NSEvent*)theEvent;
- (NSScrollView*) enclosingScrollView;
- (void) scrollPoint: (NSPoint)aPoint;
- (void) scrollRect: (NSRect)aRect
                 by: (NSSize)delta;
- (BOOL) scrollRectToVisible: (NSRect)aRect;

- (void) reflectScrolledClipView: (NSClipView*)aClipView;
- (void) scrollClipView: (NSClipView*)aClipView
                toPoint: (NSPoint)aPoint;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/*
 * Menu operations
 */
+ (NSMenu *) defaultMenu;
- (NSMenu *) menuForEvent: (NSEvent *)theEvent;
#endif

/*
 * Aiding Event Handling
 */
- (BOOL) needsPanelToBecomeKey;
- (void) setNextKeyView: (NSView*)aView;
- (NSView*) nextKeyView;
- (NSView*) nextValidKeyView;
- (void) setPreviousKeyView: (NSView*)aView;
- (NSView*) previousKeyView;
- (NSView*) previousValidKeyView;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL) canBecomeKeyView;
#endif

/*
 * Printing
 */
- (void) fax: (id)sender;
- (void) print: (id)sender;
- (NSData*) dataWithEPSInsideRect: (NSRect)aRect;
- (void) writeEPSInsideRect: (NSRect)rect
               toPasteboard: (NSPasteboard*)pasteboard;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSData *) dataWithPDFInsideRect: (NSRect)aRect;
- (void) writePDFInsideRect: (NSRect)aRect 
               toPasteboard: (NSPasteboard *)pboard;
- (NSString *) printJobTitle;
#endif

/*
 * Pagination
 */
- (void) adjustPageHeightNew: (CGFloat*)newBottom
                         top: (CGFloat)oldTop
                      bottom: (CGFloat)oldBottom
                       limit: (CGFloat)bottomLimit;
- (void) adjustPageWidthNew: (CGFloat*)newRight
                       left: (CGFloat)oldLeft
                      right: (CGFloat)oldRight
                      limit: (CGFloat)rightLimit;
- (CGFloat) heightAdjustLimit;
- (BOOL) knowsPagesFirst: (int*)firstPageNum
                    last: (int*)lastPageNum;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL) knowsPageRange: (NSRange*)range;
#endif
- (NSPoint) locationOfPrintRect: (NSRect)aRect;
- (NSRect) rectForPage: (NSInteger)page;
- (CGFloat) widthAdjustLimit;

/*
 * Writing Conforming PostScript
 */
- (void) addToPageSetup;
- (void) beginPage: (int)ordinalNum
	     label: (NSString*)aString
	      bBox: (NSRect)pageRect
	     fonts: (NSString*)fontNames;
- (void) beginPageSetupRect: (NSRect)aRect
		  placement: (NSPoint)location;
- (void) beginPrologueBBox: (NSRect)boundingBox
	      creationDate: (NSString*)dateCreated
		 createdBy: (NSString*)anApplication
		     fonts: (NSString*)fontNames
		   forWhom: (NSString*)user
		     pages: (int)numPages
		     title: (NSString*)aTitle;
- (void) beginSetup;
- (void) beginTrailer;
- (void) drawPageBorderWithSize: (NSSize)borderSize;
- (void) drawSheetBorderWithSize: (NSSize)borderSize;
- (void) endHeaderComments;
- (void) endPrologue;
- (void) endSetup;
- (void) endPageSetup;
- (void) endPage;
- (void) endTrailer;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)beginDocument;
- (void)beginPageInRect:(NSRect)aRect 
	    atPlacement:(NSPoint)location;
- (void)endDocument;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSAttributedString *)pageFooter;
- (NSAttributedString *)pageHeader;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property (nonatomic) NSViewLayerContentsPlacement layerContentsPlacement;
@property (nonatomic) NSViewLayerContentsRedrawPolicy layerContentsRedrawPolicy;
#else
- (NSViewLayerContentsPlacement) layerContentsPlacement;
- (void) setLayerContentsPlacement: (NSViewLayerContentsPlacement)placement;

- (NSViewLayerContentsRedrawPolicy) layerContentsRedrawPolicy;
- (void) setLayerContentsRedrawPolicy: (NSViewLayerContentsRedrawPolicy) pol;
#endif
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property NSUserInterfaceLayoutDirection userInterfaceLayoutDirection;
#else
- (NSUserInterfaceLayoutDirection) userInterfaceLayoutDirection;
- (void) setUserInterfaceLayoutDirection: (NSUserInterfaceLayoutDirection)dir;
#endif
#endif

@end


@class NSAffineTransform;

/*
 * GNUstep extensions
 * Methods whose names begin with an underscore must NOT be overridden.
 */
#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
@interface NSView (PrivateMethods)

/*
 * The [-_invalidateCoordinates] method marks the cached visible rectangles
 * of the view and it's subview as being invalid.  NSViews methods call this
 * whenever the coordinate system of the view is changed in any way - thus
 * forcing recalculation of cached values next time they are needed.
 */
- (void) _invalidateCoordinates;
- (void) _rebuildCoordinates;

- (NSAffineTransform*) _matrixToWindow;
- (NSAffineTransform*) _matrixFromWindow;

- (void) _setIgnoresBacking: (BOOL) flag;
- (BOOL) _ignoresBacking;

@end
#endif

/*
 * GNUstep specific function to determine the drag types registered for a view.
 */
APPKIT_EXPORT NSArray *GSGetDragTypes(NSView* aView);

/* Notifications */
APPKIT_EXPORT NSString *NSViewFrameDidChangeNotification;
APPKIT_EXPORT NSString *NSViewBoundsDidChangeNotification;
APPKIT_EXPORT NSString *NSViewFocusDidChangeNotification;
APPKIT_EXPORT NSString *NSViewGlobalFrameDidChangeNotification;

#endif // _GNUstep_H_NSView
