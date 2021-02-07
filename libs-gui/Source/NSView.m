/** <title>NSView</title>

   <abstract>The view class which encapsulates all drawing functionality</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: 1997   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: January 1999

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

#import "config.h"
#include <math.h>
#include <float.h>

#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSCalendarDate.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSSet.h>

#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSBitmapImageRep.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSDocumentController.h"
#import "AppKit/NSDocument.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSPrintInfo.h"
#import "AppKit/NSPrintOperation.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSWorkspace.h"
#import "AppKit/PSOperators.h"
#import "GNUstepGUI/GSDisplayServer.h"
#import "GNUstepGUI/GSTrackingRect.h"
#import "GNUstepGUI/GSNibLoading.h"
#import "GSToolTips.h"
#import "GSBindingHelpers.h"
#import "GSGuiPrivate.h"
#import "NSViewPrivate.h"

/*
 * We need a fast array that can store objects without retain/release ...
 */
#define GSI_ARRAY_TYPES		GSUNION_OBJ
#define GSI_ARRAY_NO_RELEASE	1
#define GSI_ARRAY_NO_RETAIN	1

#ifdef GSIArray
#undef GSIArray
#endif
#include <GNUstepBase/GSIArray.h>

#define	nKV(O)	((GSIArray)(O->_nextKeyView))
#define	pKV(O)	((GSIArray)(O->_previousKeyView))

/* Variable tells this view and subviews that we're printing. Not really
   a class variable because we want it visible to subviews also
*/
NSView *viewIsPrinting = nil;

/**
  <unit>
  <heading>NSView</heading>

  <p>NSView is an abstract class which provides facilities for drawing
  in a window and receiving events.  It is the superclass of many of
  the visual elements of the GUI.</p>

  <p>In order to display itself, a view must be placed in a window
  (represented by an NSWindow object).  Within the window is a
  hierarchy of NSViews, headed by the window's content view.  Every
  other view in a window is a descendant of this view.</p>

  <p>Subclasses can override -drawRect: in order to
  implement their appearance.  Other methods of NSView and NSResponder
  can also be overridden to handle user generated events.</p>

  </unit>
*/
  
@implementation NSView

/*
 * Class variables */
static Class	rectClass;
static Class	viewClass;

static NSAffineTransform	*flip = nil;

static NSNotificationCenter *nc = nil;

static SEL	preSel;
static SEL	invalidateSel;

static void	(*preImp)(NSAffineTransform*, SEL, NSAffineTransform*);
static void	(*invalidateImp)(NSView*, SEL);

/*
 *	Stuff to maintain a map table so we know what views are
 *	registered for drag and drop - we don't store the info in
 *	the view directly 'cot it would take up a pointer in each
 *	view and the vast majority of views wouldn't use it.
 *	Types are not registered/unregistered often enough for the
 *	performance of this mechanism to be an issue.
 */
static NSMapTable	*typesMap = 0;
static NSLock		*typesLock = nil;

/*
 * This is the only external interface to the drag types info.
 */
NSArray*
GSGetDragTypes(NSView *obj)
{
  NSArray	*t;

  [typesLock lock];
  t = (NSArray*)NSMapGet(typesMap, (void*)(gsaddr)obj);
  [typesLock unlock];
  return t;
}

static void
GSRemoveDragTypes(NSView* obj)
{
  [typesLock lock];
  NSMapRemove(typesMap, (void*)(gsaddr)obj);
  [typesLock unlock];
}

static NSArray*
GSSetDragTypes(NSView* obj, NSArray *types)
{
  NSUInteger	count = [types count];
  NSString	*strings[count];
  NSArray	*t;
  NSUInteger	i;

  /*
   * Make a new array with copies of the type strings so we don't get
   * them mutated by someone else.
   */
  [types getObjects: strings];
  for (i = 0; i < count; i++)
    {
      strings[i] = [strings[i] copy];
    }
  t = [NSArray arrayWithObjects: strings count: count];
  for (i = 0; i < count; i++)
    {
      RELEASE(strings[i]);
    }
  /*
   * Store it.
   */
  [typesLock lock];
  NSMapInsert(typesMap, (void*)(gsaddr)obj, (void*)(gsaddr)t);
  [typesLock unlock];
  return t;
}


/*
 *	Private methods.
 */

/*
 *	The [-_invalidateCoordinates] method marks the coordinate mapping
 *	matrices (matrixFromWindow and _matrixToWindow) and the cached visible
 *	rectangle as invalid.  It recursively invalidates the coordinates for
 *	all subviews as well.
 *	This method must be called whenever the size, shape or position of
 *	the view is changed in any way.
 */
- (void) _invalidateCoordinates
{
  if (_coordinates_valid == YES)
    {
      NSUInteger count;

      _coordinates_valid = NO;
      if (_rFlags.valid_rects != 0)
        {
          [_window invalidateCursorRectsForView: self];
        }
      if (_rFlags.has_subviews)
        {
          count = [_sub_views count];
          if (count > 0)
            {
              NSView*	array[count];
              NSUInteger i;
              
              [_sub_views getObjects: array];
              for (i = 0; i < count; i++)
                {
                  NSView	*sub = array[i];
                  
                  if (sub->_coordinates_valid == YES)
                    {
                      (*invalidateImp)(sub, invalidateSel);
                    }
                }
            }
        }
      [self renewGState];
    }
}

/*
 *	The [-_matrixFromWindow] method returns a matrix that can be used to
 *	map coordinates in the windows coordinate system to coordinates in the
 *	views coordinate system.  It rebuilds the mapping matrices and
 *	visible rectangle cache if necessary.
 *	All coordinate transformations use this matrix.
 */
- (NSAffineTransform*) _matrixFromWindow
{
  [self _rebuildCoordinates];
  return _matrixFromWindow;
}

/*
 *	The [-_matrixToWindow] method returns a matrix that can be used to
 *	map coordinates in the views coordinate system to coordinates in the
 *	windows coordinate system.  It rebuilds the mapping matrices and
 *	visible rectangle cache if necessary.
 *	All coordinate transformations use this matrix.
 */
- (NSAffineTransform*) _matrixToWindow
{
  [self _rebuildCoordinates];
  return _matrixToWindow;
}

/*
 *	The [-_rebuildCoordinates] method rebuilds the coordinate mapping
 *	matrices (matrixFromWindow and _matrixToWindow) and the cached visible
 *	rectangle if they have been invalidated.
 */
- (void) _rebuildCoordinates
{
  BOOL isFlipped = [self isFlipped];
  BOOL lastFlipped = _rFlags.flipped_view;

  if ((_coordinates_valid == NO) || (isFlipped != lastFlipped))
    {
      _coordinates_valid = YES;
      _rFlags.flipped_view = isFlipped;

      if (!_window && !_super_view)
        {
          _visibleRect = _bounds;
          [_matrixToWindow makeIdentityMatrix];
          [_matrixFromWindow makeIdentityMatrix];
        }
      else
        {
          NSRect		superviewsVisibleRect;
          BOOL			superFlipped;
          NSAffineTransform	*pMatrix;
          NSAffineTransformStruct     ts;
 
	  if (_super_view != nil)
	    {
	      superFlipped = [_super_view isFlipped];
	      pMatrix = [_super_view _matrixToWindow];
	    }
	  else
	    {
	      superFlipped = NO;
	      pMatrix = [NSAffineTransform transform];
           }
	      
	  ts = [pMatrix transformStruct];

          /* prepend translation */
          ts.tX = NSMinX(_frame) * ts.m11 + NSMinY(_frame) * ts.m21 + ts.tX;
          ts.tY = NSMinX(_frame) * ts.m12 + NSMinY(_frame) * ts.m22 + ts.tY;
          [_matrixToWindow setTransformStruct: ts];
 
          /* prepend rotation */
          if (_frameMatrix != nil)
            {
              (*preImp)(_matrixToWindow, preSel, _frameMatrix);
            }
 
          if (isFlipped != superFlipped)
            {
              /*
               * The flipping process must result in a coordinate system that
               * exactly overlays the original.	 To do that, we must translate
               * the origin by the height of the view.
               */
              ts = [flip transformStruct];
              ts.tY = _frame.size.height;
              [flip setTransformStruct: ts];
              (*preImp)(_matrixToWindow, preSel, flip);
            }
          if (_boundsMatrix != nil)
            {
              (*preImp)(_matrixToWindow, preSel, _boundsMatrix);
            }
          ts = [_matrixToWindow transformStruct];
          [_matrixFromWindow setTransformStruct: ts];
          [_matrixFromWindow invert];

	  if (_super_view != nil)
	    {
	      superviewsVisibleRect = [self convertRect: [_super_view visibleRect]
					       fromView: _super_view];
	      
	      _visibleRect = NSIntersectionRect(superviewsVisibleRect, _bounds);
	    }
	  else
	    {
	      _visibleRect = _bounds;
	    }
        }
    }
}

- (void) _viewDidMoveToWindow
{
  [self viewDidMoveToWindow];
  if (_rFlags.has_subviews)
    {
      NSUInteger count = [_sub_views count];

      if (count > 0)
        {
          NSUInteger i;
          NSView *array[count];

          [_sub_views getObjects: array];
          for (i = 0; i < count; ++i)
            {
              [array[i] _viewDidMoveToWindow];
            }
        }
    }
}

- (void) _viewWillMoveToWindow: (NSWindow*)newWindow
{
  BOOL old_allocate_gstate;

  [self viewWillMoveToWindow: newWindow];
  if (_coordinates_valid)
    {
      (*invalidateImp)(self, invalidateSel);
    }
  if (_rFlags.has_currects != 0)
    {
      [self discardCursorRects];
    }

  if (newWindow == _window)
    {
      return;
    }

  // This call also reset _allocate_gstate, so we have 
  // to store this value and set it again.
  // This way we keep the logic in one place.
  old_allocate_gstate = _allocate_gstate;
  [self releaseGState];
  _allocate_gstate = old_allocate_gstate;

  if (_rFlags.has_draginfo)
    {
      NSArray *t = GSGetDragTypes(self);

      if (_window != nil)
        {
          [GSDisplayServer removeDragTypes: t fromWindow: _window];
	  if ([_window autorecalculatesKeyViewLoop])
	    {
	      [_window recalculateKeyViewLoop];
	    }
        }
      if (newWindow != nil)
        {
          [GSDisplayServer addDragTypes: t toWindow: newWindow];
	  if ([newWindow autorecalculatesKeyViewLoop])
	    {
	      [newWindow recalculateKeyViewLoop];
	    }
        }
    }
  
  _window = newWindow;

  if (_rFlags.has_subviews)
    {
      NSUInteger count = [_sub_views count];

      if (count > 0)
        {
          NSUInteger i;
          NSView *array[count];
          
          [_sub_views getObjects: array];
          for (i = 0; i < count; ++i)
            {
              [array[i] _viewWillMoveToWindow: newWindow];
            }
        }
    }
}

- (void) _viewWillMoveToSuperview: (NSView*)newSuper
{
  [self viewWillMoveToSuperview: newSuper];
  _super_view = newSuper;
}

/*
 * Extend in super view covered by the frame of a view.
 * When the frame is rotated, this is different from the frame.
 */
- (NSRect) _frameExtend
{
  NSRect frame = _frame;
              
  if (_frameMatrix != nil)
    {
      NSRect r;

      r.origin = NSZeroPoint;
      r.size = frame.size;
      [_frameMatrix boundingRectFor: r result: &r];
      frame = NSOffsetRect(r, NSMinX(frame),
                           NSMinY(frame));
    }

  return frame;
}


- (NSString*) _subtreeDescriptionWithPrefix: (NSString*)prefix
{
  NSMutableString *desc = [[NSMutableString alloc] init];
  NSEnumerator *e;
  NSView *v;

  [desc appendFormat: @"%@%@\n", prefix, [self description], nil];

  prefix = [prefix stringByAppendingString: @"  "];
  e = [_sub_views objectEnumerator];
  while ((v = (NSView*)[e nextObject]) != nil)
    {
      [desc appendString: [v _subtreeDescriptionWithPrefix: prefix]];
    }

  return AUTORELEASE(desc);
}

/*
 * Unofficial Cocoa method for debugging a view hierarchy.
 */
- (NSString*) _subtreeDescription
{
  return [self _subtreeDescriptionWithPrefix: @""]; 
}

- (NSString*) _flagDescription
{
  return @"";
}

- (NSString*) _resizeDescription
{
  return [NSString stringWithFormat: @"h=%c%c%c v=%c%c%c", 
                   (_autoresizingMask & NSViewMinXMargin) ? '&' : '-',
                   (_autoresizingMask & NSViewWidthSizable) ? '&' : '-',
                   (_autoresizingMask & NSViewMaxXMargin) ? '&' : '-',
                   (_autoresizingMask & NSViewMinYMargin) ? '&' : '-',
                   (_autoresizingMask & NSViewHeightSizable) ? '&' : '-',
                   (_autoresizingMask & NSViewMaxYMargin) ? '&' : '-',
                   nil];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ %@ %@ f=%@ b=%@", 
                   [self _flagDescription], 
                   [self _resizeDescription], [super description], 
                   NSStringFromRect(_frame), NSStringFromRect(_bounds), nil];
}

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSView class])
    {
      Class	matrixClass = [NSAffineTransform class];
      NSAffineTransformStruct	ats = { 1, 0, 0, -1, 0, 1 };

      typesMap = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
                NSObjectMapValueCallBacks, 0);
      typesLock = [NSLock new];

      preSel = @selector(prependTransform:);
      invalidateSel = @selector(_invalidateCoordinates);

      preImp = (void (*)(NSAffineTransform*, SEL, NSAffineTransform*))
          [matrixClass instanceMethodForSelector: preSel];

      invalidateImp = (void (*)(NSView*, SEL))
          [self instanceMethodForSelector: invalidateSel];

      flip = [matrixClass new];
      [flip setTransformStruct: ats];

      nc = [NSNotificationCenter defaultCenter];

      viewClass = [NSView class];
      rectClass = [GSTrackingRect class];
      NSDebugLLog(@"NSView", @"Initialize NSView class\n");
      [self setVersion: 1];

      // expose bindings
      [self exposeBinding: NSToolTipBinding];
      [self exposeBinding: NSHiddenBinding];
    }
}

/**
 Return the view at the top of graphics contexts stack
 or nil if none is focused.
 */
+ (NSView*) focusView
{
  return [GSCurrentContext() focusView];
}

/*
 * Instance methods
 */
- (id) init
{
  return [self initWithFrame: NSZeroRect];
}

- (id) initWithFrame: (NSRect)frameRect
{
  self = [super init];
  if (!self)
    return self;

  if (frameRect.size.width < 0)
    {
      NSWarnMLog(@"given negative width");
      frameRect.size.width = 0;
    }
  if (frameRect.size.height < 0)
    {
      NSWarnMLog(@"given negative height");
      frameRect.size.height = 0;
    }
  _frame = frameRect;			// Set frame rectangle
  _bounds.origin = NSZeroPoint;		// Set bounds rectangle
  _bounds.size = _frame.size;

  // _frameMatrix = [NSAffineTransform new];    // Map fromsuperview to frame
  // _boundsMatrix = [NSAffineTransform new];   // Map from superview to bounds
  _matrixToWindow = [NSAffineTransform new];   // Map to window coordinates
  _matrixFromWindow = [NSAffineTransform new]; // Map from window coordinates

  _sub_views = [NSMutableArray new];
  _tracking_rects = [NSMutableArray new];
  _cursor_rects = [NSMutableArray new];

  // Some values are already set by initialisation
  //_super_view = nil;
  //_window = nil;
  //_is_rotated_from_base = NO;
  //_is_rotated_or_scaled_from_base = NO;
  _rFlags.needs_display = YES;
  _post_bounds_changes = YES;
  _post_frame_changes = YES;
  _autoresizes_subviews = YES;
  _autoresizingMask = NSViewNotSizable;
  //_coordinates_valid = NO;
  //_nextKeyView = 0;
  //_previousKeyView = 0;

  _alphaValue = 1.0;
  
  return self;
}

- (void) dealloc
{
  NSView *tmp;
  NSUInteger count;

  // Remove all key value bindings for this view.
  [GSKeyValueBinding unbindAllForObject: self];

  /*
   * Remove self from view chain.  Try to mimic MacOS-X behavior ...
   * We send setNextKeyView: messages to all view for which we are the
   * next key view, setting their next key view to nil.
   *
   * First we do the obvious stuff using the standard methods.
   */
  [self setNextKeyView: nil];
  tmp = [self previousKeyView];
  if ([tmp nextKeyView] == self)
    [tmp setNextKeyView: nil];

  /*
   * Now, we locate any remaining cases where a view has us as its next
   * view, and ask the view to change that.
   */
  if (pKV(self) != 0)
    {
      count = GSIArrayCount(pKV(self));
      while (count-- > 0)
	{
	  tmp = GSIArrayItemAtIndex(pKV(self), count).obj;
	  if ([tmp nextKeyView] == self)
	    {
	      [tmp setNextKeyView: nil];
	    }
	}
    }

  /*
   * Now we clean up the previous view array, in case subclasses have
   * overridden the default -setNextKeyView: method and broken things.
   * We also relase the memory we used.
   */
  if (pKV(self) != 0)
    {
      count = GSIArrayCount(pKV(self));
      while (count-- > 0)
	{
	  tmp = GSIArrayItemAtIndex(pKV(self), count).obj;
	  if (tmp != nil && nKV(tmp) != 0)
	    {
	      NSUInteger otherCount = GSIArrayCount(nKV(tmp));
	
	      while (otherCount-- > 1)
		{
		  if (GSIArrayItemAtIndex(nKV(tmp), otherCount).obj == self)
		    {
		      GSIArrayRemoveItemAtIndex(nKV(tmp), otherCount);
		    }
		}
	      if (GSIArrayItemAtIndex(nKV(tmp), 0).obj == self)
		{
		  GSIArraySetItemAtIndex(nKV(tmp), (GSIArrayItem)nil, 0);
		}
	    }
	}
      GSIArrayClear(pKV(self));
      NSZoneFree(NSDefaultMallocZone(), pKV(self));
      _previousKeyView = 0;
    }

  /*
   * Now we clean up all views which have us as their previous view.
   * We also release the memory we used.
   */
  if (nKV(self) != 0)
    {
      count = GSIArrayCount(nKV(self));
      while (count-- > 0)
	{
	  tmp = GSIArrayItemAtIndex(nKV(self), count).obj;
	  if (tmp != nil && pKV(tmp) != 0)
	    {
	      NSUInteger otherCount = GSIArrayCount(pKV(tmp));
	
	      while (otherCount-- > 1)
		{
		  if (GSIArrayItemAtIndex(pKV(tmp), otherCount).obj == self)
		    {
		      GSIArrayRemoveItemAtIndex(pKV(tmp), otherCount);
		    }
		}
	      if (GSIArrayItemAtIndex(pKV(tmp), 0).obj == self)
		{
		  GSIArraySetItemAtIndex(pKV(tmp), (GSIArrayItem)nil, 0);
		}
	    }
	}
      GSIArrayClear(nKV(self));
      NSZoneFree(NSDefaultMallocZone(), nKV(self));
      _nextKeyView = 0;
    }

  /*
   * Now remove our subviews, AFTER cleaning up the view chain, in case
   * any of our subviews were in the chain.
   */
  while ([_sub_views count] > 0)
    {
      [[_sub_views lastObject] removeFromSuperviewWithoutNeedingDisplay];
    }

  RELEASE(_matrixToWindow);
  RELEASE(_matrixFromWindow);
  TEST_RELEASE(_frameMatrix);
  TEST_RELEASE(_boundsMatrix);
  TEST_RELEASE(_sub_views);
  if (_rFlags.has_tooltips != 0)
    {
      [GSToolTips removeTipsForView: self];
    }
  if (_rFlags.has_currects != 0)
    {
      [self discardCursorRects];	// Handle release of cursors
    }
  TEST_RELEASE(_cursor_rects);
  TEST_RELEASE(_tracking_rects);
  [self unregisterDraggedTypes];
  [self releaseGState];

  [super dealloc];
}

/**
 * Adds aView as a subview of the receiver.
 */
- (void) addSubview: (NSView*)aView
{
  [self addSubview: aView
        positioned: NSWindowAbove
        relativeTo: nil];
}

- (void) addSubview: (NSView*)aView
	 positioned: (NSWindowOrderingMode)place
	 relativeTo: (NSView*)otherView
{
  NSUInteger index;

  if (aView == nil)
    {
      return;
    }
  if ([self isDescendantOf: aView])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"addSubview:positioned:relativeTo: creates a "
	@"loop in the views tree!"];
    }

  if (aView == otherView)
    return;

  RETAIN(aView);
  [aView removeFromSuperview];

  // Do this after the removeFromSuperview, as aView may already 
  // be a subview and the index could change.
  if (otherView == nil)
    {
      index = NSNotFound;
    }
  else
    {
      index = [_sub_views indexOfObjectIdenticalTo: otherView];
    }
  if (index == NSNotFound)
    {
      if (place == NSWindowBelow)
        index = 0;
      else
        index = [_sub_views count];
    }
  else if (place != NSWindowBelow)
    {
      index += 1;
    }

  [aView _viewWillMoveToWindow: _window];
  [aView _viewWillMoveToSuperview: self];
  [aView setNextResponder: self];
  [_sub_views insertObject: aView atIndex: index];
  _rFlags.has_subviews = 1;
  [aView resetCursorRects];
  [aView setNeedsDisplay: YES];
  [aView _viewDidMoveToWindow];
  [aView viewDidMoveToSuperview];
  [self didAddSubview: aView];
  RELEASE(aView);
}

/**
 * Returns self if aView is the receiver or aView is a subview of the receiver,
 * the ancestor view shared by aView and the receiver if any, or
 * aView if it is an ancestor of the receiver, otherwise returns nil.
 */
- (NSView*) ancestorSharedWithView: (NSView*)aView
{
  if (self == aView)
    return self;

  if ([self isDescendantOf: aView])
    return aView;

  if ([aView isDescendantOf: self])
    return self;

  /*
   * If neither are descendants of each other and either does not have a
   * superview then they cannot have a common ancestor
   */
  if (!_super_view)
    return nil;

  if (![aView superview])
    return nil;

  /* Find the common ancestor of superviews */
  return [_super_view ancestorSharedWithView: [aView superview]];
}

/**
 * Returns YES if aView is an ancestor of the receiver.
 */
- (BOOL) isDescendantOf: (NSView*)aView
{
  if (aView == self)
    return YES;

  if (!_super_view)
    return NO;

  if (_super_view == aView)
    return YES;

  return [_super_view isDescendantOf: aView];
}

- (NSView*) opaqueAncestor
{
  NSView	*next = _super_view;
  NSView	*current = self;

  while (next != nil)
    {
      if ([current isOpaque] == YES)
	{
	  break;
	}
      current = next;
      next = current->_super_view;
    }
  return current;
}

/**
 * Removes the receiver from its superviews list of subviews.
 */
- (void) removeFromSuperviewWithoutNeedingDisplay
{
  if (_super_view != nil)
    {
      [_super_view removeSubview: self];
    }
}

/**
  <p> Removes the receiver from its superviews list of subviews
  and marks the rectangle that the reciever occupied in the 
  superview as needing redisplay.  </p>

  <p> This is dangerous to use during display, since it alters the
  rectangles needing display. In this case, you can use the
  -removeFromSuperviewWithoutNeedingDisplay method instead.</p> */
- (void) removeFromSuperview
{
  if (_super_view != nil)
    {
      [_super_view setNeedsDisplayInRect: _frame];
      [self removeFromSuperviewWithoutNeedingDisplay];
    }
}

/**
  <p> Removes aSubview from the receivers list of subviews and from
  the responder chain.  </p>

  <p> Also invokes -viewWillMoveToWindow: on aView with a nil argument,
  to handle
  removal of aView (and recursively, its children) from its window -
  performing tidyup by invalidating cursor rects etc.  </p> 
*/
- (void) removeSubview: (NSView*)aView
{
  id view;
  /*
   * This must be first because it invokes -resignFirstResponder:, 
   * which assumes the view is still in the view hierarchy
   */
  for (view = [_window firstResponder];
    view != nil && [view respondsToSelector: @selector(superview)];
    view = [view superview])
    {
      if (view == aView)
	{      
	  [_window makeFirstResponder: _window];
	  break;
	}
    }
  [self willRemoveSubview: aView];
  aView->_super_view = nil;
  [aView _viewWillMoveToWindow: nil];
  [aView _viewWillMoveToSuperview: nil];
  [aView setNextResponder: nil];
  RETAIN(aView);
  [_sub_views removeObjectIdenticalTo: aView];
  [aView setNeedsDisplay: NO];
  [aView _viewDidMoveToWindow];
  [aView viewDidMoveToSuperview];
  RELEASE(aView);
  if ([_sub_views count] == 0)
    {
      _rFlags.has_subviews = 0;
    }
}

/**
 * Removes oldView, which should be a subview of the receiver, from the
 * receiver and places newView in its place. If newView is nil, just
 * removes oldView. If oldView is nil, just adds newView.
 */
- (void) replaceSubview: (NSView*)oldView with: (NSView*)newView
{
  if (newView == oldView)
    {
      return;
    }
  /*
   * NB. we implement the replacement in full rather than calling addSubview:
   * since classes like NSBox override these methods but expect to be able to
   * call [super replaceSubview:with:] safely.
   */
  if (oldView == nil)
    {
      /*
       * Strictly speaking, the docs say that if 'oldView' is not a subview
       * of the receiver then we do nothing - but here we add newView anyway.
       * So a replacement with no oldView is an addition.
       */
      RETAIN(newView);
      [newView removeFromSuperview];
      [newView _viewWillMoveToWindow: _window];
      [newView _viewWillMoveToSuperview: self];
      [newView setNextResponder: self];
      [_sub_views addObject: newView];
      _rFlags.has_subviews = 1;
      [newView resetCursorRects];
      [newView setNeedsDisplay: YES];
      [newView _viewDidMoveToWindow];
      [newView viewDidMoveToSuperview];
      [self didAddSubview: newView];
      RELEASE(newView);
    }
  else if ([_sub_views indexOfObjectIdenticalTo: oldView] != NSNotFound)
    {
      if (newView == nil)
	{
	  /*
	   * If there is no new view to add - we just remove the old one.
	   * So a replacement with no newView is a removal.
	   */
	  [oldView removeFromSuperview];
	}
      else
	{
	  NSUInteger index;

	  /*
	   * Ok - the standard case - we remove the newView from wherever it
	   * was (which may have been in this view), locate the position of
	   * the oldView (which may have changed due to the removal of the
	   * newView), remove the oldView, and insert the newView in it's
	   * place.
	   */
	  RETAIN(newView);
	  [newView removeFromSuperview];
	  index = [_sub_views indexOfObjectIdenticalTo: oldView];
	  [oldView removeFromSuperview];
	  [newView _viewWillMoveToWindow: _window];
	  [newView _viewWillMoveToSuperview: self];
	  [newView setNextResponder: self];
          [_sub_views insertObject: newView
                           atIndex: index];
	  _rFlags.has_subviews = 1;
	  [newView resetCursorRects];
	  [newView setNeedsDisplay: YES];
	  [newView _viewDidMoveToWindow];
	  [newView viewDidMoveToSuperview];
	  [self didAddSubview: newView];
	  RELEASE(newView);
	}
    }
}

- (void) setSubviews: (NSArray *)newSubviews
{
  NSEnumerator *en;
  NSView *aView;
  NSMutableArray *uniqNew = [NSMutableArray array];
  
  if (nil == newSubviews)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"Setting nil as new subviews."];
    }

  // Use a copy as we remove from the subviews array
  en = [[NSArray arrayWithArray: _sub_views] objectEnumerator];
  while ((aView = [en nextObject]))
    {
      if (NO == [newSubviews containsObject: aView])
        {
          [aView removeFromSuperview];
        }
    }
  
  en = [newSubviews objectEnumerator];
  while ((aView = [en nextObject]))
    {
      id supersub = [aView superview];

      if (supersub != nil && supersub != self)
        {
          [NSException raise: NSInvalidArgumentException
                      format: @"Superviews of new subviews must be either nil or receiver."];
        }
      
      if ([uniqNew containsObject: aView])
        {
          [NSException raise: NSInvalidArgumentException
                      format: @"Duplicated new subviews."];
        }
      
      if (NO == [_sub_views containsObject: aView])
        {
          [self addSubview: aView];
        }
      
      [uniqNew addObject: aView];
    }
  
  ASSIGN(_sub_views, uniqNew);

  // The order of the subviews may have changed
  [self setNeedsDisplay: YES];
}

- (void) sortSubviewsUsingFunction: (NSComparisonResult (*)(id ,id ,void*))compare
			   context: (void*)context
{
  [_sub_views sortUsingFunction: compare context: context];
}

/**
 * Notifies the receiver that its superview is being changed to newSuper.
 */
- (void) viewWillMoveToSuperview: (NSView*)newSuper
{
}

/**
 * Notifies the receiver that it will now be a view of newWindow.
 * Note, this method is also used when removing a view from a window
 * (in which case, newWindow is nil) to let all the subviews know
 * that they have also been removed from the window.
 */
- (void) viewWillMoveToWindow: (NSWindow*)newWindow
{
}

- (void) didAddSubview: (NSView *)subview
{}

- (void) viewDidMoveToSuperview
{}

- (void) viewDidMoveToWindow
{}

- (void) willRemoveSubview: (NSView *)subview
{}

static NSSize _computeScale(NSSize fs, NSSize bs)
{
  NSSize scale;

  if (bs.width == 0)
    {
      if (fs.width == 0)
        scale.width = 1;
      else
        scale.width = FLT_MAX;
    }
  else
    {
      scale.width = fs.width / bs.width;
    }
  if (bs.height == 0)
    {
      if (fs.height == 0)
        scale.height = 1;
      else
        scale.height = FLT_MAX;
    }
  else
    {
      scale.height = fs.height / bs.height;
    }

  return scale;
}

- (void) _setFrameAndClearAutoresizingError: (NSRect)frameRect
{
  _frame = frameRect;
  _autoresizingFrameError = NSZeroRect;
}

- (void) setFrame: (NSRect)frameRect
{
  BOOL	changedOrigin = NO;
  BOOL	changedSize = NO;
  NSSize old_size = _frame.size;

  if (frameRect.size.width < 0)
    {
      NSWarnMLog(@"given negative width");
      frameRect.size.width = 0;
    }
  if (frameRect.size.height < 0)
    {
      NSWarnMLog(@"given negative height");
      frameRect.size.height = 0;
    }

  if (NSEqualPoints(_frame.origin, frameRect.origin) == NO)
    {
      changedOrigin = YES;
    }
  if (NSEqualSizes(_frame.size, frameRect.size) == NO)
    {
      changedSize = YES;
    }
  
  if (changedSize == YES || changedOrigin == YES)
    {
      [self _setFrameAndClearAutoresizingError: frameRect];

      if (changedSize == YES)
        {
          if (_is_rotated_or_scaled_from_base == YES)
            {
              NSAffineTransform *matrix;
              NSRect frame = _frame;

              frame.origin = NSMakePoint(0, 0);
              matrix = [_boundsMatrix copy];
              [matrix invert];
              [matrix boundingRectFor: frame result: &_bounds];
              RELEASE(matrix);               
            }
          else
            {
              _bounds.size = frameRect.size;
            }
        }

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self resetCursorRects];
      [self resizeSubviewsWithOldSize: old_size];
      if (_post_frame_changes)
        {
          [nc postNotificationName: NSViewFrameDidChangeNotification
              object: self];
        }
    }
}

- (void) setFrameOrigin: (NSPoint)newOrigin
{
  if (NSEqualPoints(_frame.origin, newOrigin) == NO)
    {
      NSRect newFrame = _frame;
      newFrame.origin = newOrigin;

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self _setFrameAndClearAutoresizingError: newFrame];
      [self resetCursorRects];
      if (_post_frame_changes)
        {
          [nc postNotificationName: NSViewFrameDidChangeNotification
              object: self];
        }
    }
}

- (void) setFrameSize: (NSSize)newSize
{
  NSRect newFrame = _frame;
  if (newSize.width < 0)
    {
      NSWarnMLog(@"given negative width");
      newSize.width = 0;
    }
  if (newSize.height < 0)
    {
      NSWarnMLog(@"given negative height");
      newSize.height = 0;
    }
  if (NSEqualSizes(_frame.size, newSize) == NO)
    {
      NSSize old_size = _frame.size;

      if (_is_rotated_or_scaled_from_base)
        {
          if (_boundsMatrix == nil)
            {
              CGFloat sx = _bounds.size.width  / _frame.size.width;
              CGFloat sy = _bounds.size.height / _frame.size.height;
              
              newFrame.size = newSize;
	      [self _setFrameAndClearAutoresizingError: newFrame];
              _bounds.size.width  = _frame.size.width  * sx;
              _bounds.size.height = _frame.size.height * sy;
            }
          else
            {
              NSAffineTransform *matrix;
              NSRect frame;
              
              newFrame.size = newSize;
	      [self _setFrameAndClearAutoresizingError: newFrame];

              frame = _frame;
              frame.origin = NSMakePoint(0, 0);
              matrix = [_boundsMatrix copy];
              [matrix invert];
              [matrix boundingRectFor: frame result: &_bounds];
              RELEASE(matrix);
            }
        }
      else
        {
          newFrame.size = _bounds.size = newSize;
	  [self _setFrameAndClearAutoresizingError: newFrame];
        }

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self resetCursorRects];
      [self resizeSubviewsWithOldSize: old_size];
      if (_post_frame_changes)
        {
          [nc postNotificationName: NSViewFrameDidChangeNotification
              object: self];
        }
    }
}

- (void) setFrameRotation: (CGFloat)angle
{
  CGFloat oldAngle = [self frameRotation];

  if (oldAngle != angle)
    {
      /* no frame matrix, create one since it is needed for rotation */
      if (_frameMatrix == nil)
        {
          // Map from superview to frame
          _frameMatrix = [NSAffineTransform new];
        }

      [_frameMatrix rotateByDegrees: angle - oldAngle];
      _is_rotated_from_base = _is_rotated_or_scaled_from_base = YES;

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self resetCursorRects];
      if (_post_frame_changes)
        {
          [nc postNotificationName: NSViewFrameDidChangeNotification
              object: self];
        }
    }
}

- (BOOL) isRotatedFromBase
{
  if (_is_rotated_from_base)
    {
      return YES;
    }
  else if (_super_view)
    {
      return [_super_view isRotatedFromBase];
    }
  else
    {
      return NO;
    }
}

- (BOOL) isRotatedOrScaledFromBase
{
  if (_is_rotated_or_scaled_from_base)
    {
      return YES;
    }
  else if (_super_view)
    {
      return [_super_view isRotatedOrScaledFromBase];
    }
  else
    {
      return NO;
    }
}

- (void) setBounds: (NSRect)aRect
{
  NSDebugLLog(@"NSView", @"setBounds %@", NSStringFromRect(aRect));
  if (aRect.size.width < 0)
    {
      NSWarnMLog(@"given negative width");
      aRect.size.width = 0;
    }
  if (aRect.size.height < 0)
    {
      NSWarnMLog(@"given negative height");
      aRect.size.height = 0;
    }

  if (_is_rotated_from_base || (NSEqualRects(_bounds, aRect) == NO))
    {
      NSAffineTransform *matrix;
      NSPoint oldOrigin;
      NSSize scale;
  
      if (_boundsMatrix == nil)
        {
          _boundsMatrix = [NSAffineTransform new]; 
        }

      // Adjust scale
      scale = _computeScale(_frame.size, aRect.size);
      if (scale.width != 1 || scale.height != 1)
        {
          _is_rotated_or_scaled_from_base = YES;
        }
      [_boundsMatrix scaleTo: scale.width : scale.height];
        {
          matrix = [_boundsMatrix copy];
          [matrix invert];
          oldOrigin = [matrix transformPoint: NSMakePoint(0, 0)];
          RELEASE(matrix);
        }
      [_boundsMatrix translateXBy: oldOrigin.x - aRect.origin.x 
                     yBy: oldOrigin.y - aRect.origin.y];      
      if (!_is_rotated_from_base)
        {
          // Adjust bounds
          _bounds = aRect;
        }
      else
        {
          // Adjust bounds
          NSRect frame = _frame;

          frame.origin = NSMakePoint(0, 0);
          matrix = [_boundsMatrix copy];
          [matrix invert];
          [matrix boundingRectFor: frame result: &_bounds];
          RELEASE(matrix);
       }

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self resetCursorRects];
      if (_post_bounds_changes)
        {
          [nc postNotificationName: NSViewBoundsDidChangeNotification
              object: self];
        }
    }
}

- (void) setBoundsOrigin: (NSPoint)newOrigin
{
  NSPoint oldOrigin;
  
  if (_boundsMatrix == nil)
    {
      oldOrigin = NSMakePoint(NSMinX(_bounds), NSMinY(_bounds));
    }
  else
    {
      NSAffineTransform *matrix = [_boundsMatrix copy];
    
      [matrix invert];
      oldOrigin = [matrix transformPoint: NSMakePoint(0, 0)];
      RELEASE(matrix);
    }
  [self translateOriginToPoint: NSMakePoint(oldOrigin.x - newOrigin.x, 
                                            oldOrigin.y - newOrigin.y)];
}

- (void) setBoundsSize: (NSSize)newSize
{
  NSSize scale;

  NSDebugLLog(@"NSView", @"%@ setBoundsSize: %@", self, 
              NSStringFromSize(newSize));

  if (newSize.width < 0)
    {
      NSWarnMLog(@"given negative width");
      newSize.width = 0;
    }
  if (newSize.height < 0)
    {
      NSWarnMLog(@"given negative height");
      newSize.height = 0;
    }

  scale = _computeScale(_frame.size, newSize);
  if (scale.width != 1 || scale.height != 1)
    {
      _is_rotated_or_scaled_from_base = YES;
    }
  
  if (_boundsMatrix == nil)
    {
      _boundsMatrix = [NSAffineTransform new]; 
    }
  [_boundsMatrix scaleTo: scale.width : scale.height];
  if (!_is_rotated_from_base)
    {
      scale = _computeScale(_bounds.size, newSize);
      _bounds.origin.x = _bounds.origin.x / scale.width;
      _bounds.origin.y = _bounds.origin.y / scale.height;
      _bounds.size = newSize;
    }
  else
    {
      NSAffineTransform *matrix;
      NSRect frame = _frame;
      
      frame.origin = NSMakePoint(0, 0);
      matrix = [_boundsMatrix copy];
      [matrix invert];
      [matrix boundingRectFor: frame result: &_bounds];
      RELEASE(matrix);               
    }

  if (_coordinates_valid)
    {
      (*invalidateImp)(self, invalidateSel);
    }
  [self resetCursorRects];
  if (_post_bounds_changes)
    {
      [nc postNotificationName: NSViewBoundsDidChangeNotification
                        object: self];
    }
}

- (void) setBoundsRotation: (CGFloat)angle
{
  [self rotateByAngle: angle - [self boundsRotation]];
}

- (void) translateOriginToPoint: (NSPoint)point
{
  NSDebugLLog(@"NSView", @"%@ translateOriginToPoint: %@", self, 
              NSStringFromPoint(point));
  if (NSEqualPoints(NSZeroPoint, point) == NO)
    {
      if (_boundsMatrix == nil)
        {
          _boundsMatrix = [NSAffineTransform new];
        }
      [_boundsMatrix translateXBy: point.x
                     yBy: point.y];
      // Adjust bounds
      _bounds.origin.x -= point.x;
      _bounds.origin.y -= point.y;

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self resetCursorRects];
      if (_post_bounds_changes)
        {
          [nc postNotificationName: NSViewBoundsDidChangeNotification
              object: self];
        }
    }
}

- (void) scaleUnitSquareToSize: (NSSize)newSize
{
  if (newSize.width != 1.0 || newSize.height != 1.0)
    {
      if (newSize.width < 0)
        {
          NSWarnMLog(@"given negative width");
          newSize.width = 0;
        }
      if (newSize.height < 0)
        {
          NSWarnMLog(@"given negative height");
          newSize.height = 0;
        }

      if (_boundsMatrix == nil)
        {
          _boundsMatrix = [NSAffineTransform new]; 
        }
      [_boundsMatrix scaleXBy: newSize.width yBy: newSize.height];
      // Adjust bounds
      _bounds.origin.x = _bounds.origin.x / newSize.width;
      _bounds.origin.y = _bounds.origin.y / newSize.height;
      _bounds.size.width  = _bounds.size.width  / newSize.width;
      _bounds.size.height = _bounds.size.height / newSize.height;

      _is_rotated_or_scaled_from_base = YES;

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self resetCursorRects];
      if (_post_bounds_changes)
        {
          [nc postNotificationName: NSViewBoundsDidChangeNotification
              object: self];
        }
    }
}

- (void) rotateByAngle: (CGFloat)angle
{
  if (angle != 0.0)
    {
      NSAffineTransform *matrix;
      NSRect frame = _frame;
      
      frame.origin = NSMakePoint(0, 0);
      if (_boundsMatrix == nil)
        {
          _boundsMatrix = [NSAffineTransform new]; 
        }
      [_boundsMatrix rotateByDegrees: angle];
      // Adjust bounds
      matrix = [_boundsMatrix copy];
      [matrix invert];
      [matrix boundingRectFor: frame result: &_bounds];
      RELEASE(matrix);

      _is_rotated_from_base = _is_rotated_or_scaled_from_base = YES;

      if (_coordinates_valid)
        {
          (*invalidateImp)(self, invalidateSel);
        }
      [self resetCursorRects];
      if (_post_bounds_changes)
        {
          [nc postNotificationName: NSViewBoundsDidChangeNotification
              object: self];
        }
    }
}


- (CGFloat) alphaValue
{
  return _alphaValue;
}

- (void)setAlphaValue: (CGFloat)alpha
{
  _alphaValue = alpha;
}

- (CGFloat) frameCenterRotation
{
  // FIXME this is dummy, we don't have layers yet
  return 0.0;
}

- (void) setFrameCenterRotation:(CGFloat)rot;
{
  // FIXME this is dummy, we don't have layers yet
  // we probably need a Matrix akin frame rotation.
}


- (NSRect) centerScanRect: (NSRect)aRect
{
  NSAffineTransform	*matrix;
  CGFloat x_org;
  CGFloat y_org;

  /*
   *	Hmm - we assume that the windows coordinate system is centered on the
   *	pixels of the screen - this may not be correct of course.
   *	Plus - this is all pretty meaningless is we are not in a window!
   */
  matrix = [self _matrixToWindow];
  aRect.origin = [matrix transformPoint: aRect.origin];
  aRect.size = [matrix transformSize: aRect.size];
  if (aRect.size.height < 0.0)
    {
      aRect.size.height = -aRect.size.height;
    }

  x_org = aRect.origin.x;
  y_org = aRect.origin.y;
  aRect.origin.x = GSRoundTowardsInfinity(aRect.origin.x);
  aRect.origin.y = [self isFlipped] ? GSRoundTowardsNegativeInfinity(aRect.origin.y) : GSRoundTowardsInfinity(aRect.origin.y);
  aRect.size.width = GSRoundTowardsInfinity(aRect.size.width + (x_org - aRect.origin.x) / 2.0);
  aRect.size.height = GSRoundTowardsInfinity(aRect.size.height + (y_org - aRect.origin.y) / 2.0);

  matrix = [self _matrixFromWindow];
  aRect.origin = [matrix transformPoint: aRect.origin];
  aRect.size = [matrix transformSize: aRect.size];
  if (aRect.size.height < 0.0)
    {
      aRect.size.height = -aRect.size.height;
    }

  return aRect;
}

- (NSPoint) convertPoint: (NSPoint)aPoint fromView: (NSView*)aView
{
  NSPoint inBase;

  if (aView == self)
    {
      return aPoint;
    }

  if (aView != nil)
    {
      NSAssert(_window == [aView window], NSInvalidArgumentException);      
      inBase = [[aView _matrixToWindow] transformPoint: aPoint];    
    }
  else
    {
      inBase = aPoint;
    }

  return [[self _matrixFromWindow] transformPoint: inBase];
}

- (NSPoint) convertPoint: (NSPoint)aPoint toView: (NSView*)aView
{
  NSPoint inBase;

  if (aView == self)
    return aPoint;

  inBase = [[self _matrixToWindow] transformPoint: aPoint];

  if (aView != nil)
    {
      NSAssert(_window == [aView window], NSInvalidArgumentException);      
      return [[aView _matrixFromWindow] transformPoint: inBase];
    }
  else
    {
      return inBase;
    }
}


/* Helper for -convertRect:fromView: and -convertRect:toView:. */
static NSRect
convert_rect_using_matrices(NSRect aRect, NSAffineTransform *matrix1,
					  NSAffineTransform *matrix2)
{
  NSRect r;
  NSPoint p[4], min, max;
  int i;

  for (i = 0; i < 4; i++)
    p[i] = aRect.origin;
  p[1].x += aRect.size.width;
  p[2].y += aRect.size.height;
  p[3].x += aRect.size.width;
  p[3].y += aRect.size.height;

  for (i = 0; i < 4; i++)
    p[i] = [matrix1 transformPoint: p[i]];

  min = max = p[0] = [matrix2 transformPoint: p[0]];
  for (i = 1; i < 4; i++)
    {
      p[i] = [matrix2 transformPoint: p[i]];
      min.x = MIN(min.x, p[i].x);
      min.y = MIN(min.y, p[i].y);
      max.x = MAX(max.x, p[i].x);
      max.y = MAX(max.y, p[i].y);
    }

  r.origin = min;
  r.size.width = max.x - min.x;
  r.size.height = max.y - min.y;

  return r;
}

/**
 * Converts aRect from the coordinate system of aView to the coordinate
 * system of the receiver, ie. returns the bounding rectangle in the
 * receiver of aRect in aView.
 * <br />
 * aView and the receiver must be in the same window. If aView is nil,
 * converts from the receiver's window's coordinate system.
 */
- (NSRect) convertRect: (NSRect)aRect fromView: (NSView*)aView
{
  NSAffineTransform *matrix1, *matrix2;

  if (aView == self || _window == nil || (aView != nil && [aView window] == nil))
    {
      return aRect;
    }

  if (aView != nil)
    {
      NSAssert(_window == [aView window], NSInvalidArgumentException); 
      matrix1 = [aView _matrixToWindow];      
    }
  else
    {
      matrix1 = [NSAffineTransform transform];
    }

  matrix2 = [self _matrixFromWindow];

  return convert_rect_using_matrices(aRect, matrix1, matrix2);
}

/**
 * Converts aRect from the coordinate system of the receiver to the
 * coordinate system of aView, ie. returns the bounding rectangle in
 * aView of aRect in the receiver.
 * <br />
 * aView and the receiver must be in the same window. If aView is nil,
 * converts to the receiver's window's coordinate system.
 */
- (NSRect) convertRect: (NSRect)aRect toView: (NSView*)aView
{
  NSAffineTransform *matrix1, *matrix2;

  if (aView == self || _window == nil || (aView != nil && [aView window] == nil))
    {
      return aRect;
    }

  matrix1 = [self _matrixToWindow];

  if (aView != nil)
    {
      NSAssert(_window == [aView window], NSInvalidArgumentException);
      matrix2 = [aView _matrixFromWindow];
    }
  else
    {
      matrix2 = [NSAffineTransform transform];
    }

  return convert_rect_using_matrices(aRect, matrix1, matrix2);
}

- (NSSize) convertSize: (NSSize)aSize fromView: (NSView*)aView
{
  NSSize inBase;
  NSSize inSelf;

  if (aView)
    {
      NSAssert(_window == [aView window], NSInvalidArgumentException);      
      inBase = [[aView _matrixToWindow] transformSize: aSize];
      if (inBase.height < 0.0)
	{
	  inBase.height = -inBase.height;
	} 
    }
  else
    {
      inBase = aSize;
    }

  inSelf = [[self _matrixFromWindow] transformSize: inBase];
  if (inSelf.height < 0.0)
    {
      inSelf.height = -inSelf.height;
    }
  return inSelf;
}

- (NSSize) convertSize: (NSSize)aSize toView: (NSView*)aView
{
  NSSize inBase = [[self _matrixToWindow] transformSize: aSize];
  if (inBase.height < 0.0)
    {
      inBase.height = -inBase.height;
    } 

  if (aView)
    {
      NSSize inOther;
      NSAssert(_window == [aView window], NSInvalidArgumentException);      
      inOther = [[aView _matrixFromWindow] transformSize: inBase];
      if (inOther.height < 0.0)
	{
	  inOther.height = -inOther.height;
	}
      return inOther;
    }
  else
    {
      return inBase;
    }
}

- (NSPoint) convertPointFromBase: (NSPoint)aPoint
{
  return [self convertPoint: aPoint fromView: nil];
}

- (NSPoint) convertPointToBase: (NSPoint)aPoint
{
  return [self convertPoint: aPoint toView: nil];
}

- (NSRect) convertRectFromBase: (NSRect)aRect
{
  return [self convertRect: aRect fromView: nil];
}

- (NSRect) convertRectToBase: (NSRect)aRect
{
  return [self convertRect: aRect toView: nil];
}

- (NSSize) convertSizeFromBase: (NSSize)aSize
{
  return [self convertSize: aSize fromView: nil];
}

- (NSSize) convertSizeToBase: (NSSize)aSize
{
  return [self convertSize: aSize toView: nil];
}

/** 
 * Sets whether the receiver should post NSViewFrameDidChangeNotification 
 * when its frame changed. 
 */
- (void) setPostsFrameChangedNotifications: (BOOL)flag
{
  _post_frame_changes = flag;
}

/** 
 * Sets whether the receiver should post NSViewBoundsDidChangeNotification 
 * when its bound changed.
 */
- (void) setPostsBoundsChangedNotifications: (BOOL)flag
{
  _post_bounds_changes = flag;
}

/*
 * resize subviews only if we are supposed to and we have never been rotated
 */
- (void) resizeSubviewsWithOldSize: (NSSize)oldSize
{
  if (_rFlags.has_subviews)
    {
      id e, o;

      if (_autoresizes_subviews == NO || _is_rotated_from_base == YES)
          return;

      e = [_sub_views objectEnumerator];
      o = [e nextObject];
      while (o)
        {
          [o resizeWithOldSuperviewSize: oldSize];
          o = [e nextObject];
        }
    }
}

static void autoresize(CGFloat oldContainerSize,
		       CGFloat newContainerSize,
		       CGFloat *contentPositionInOut,
		       CGFloat *contentSizeInOut,
		       BOOL minMarginFlexible,
		       BOOL sizeFlexible,
		       BOOL maxMarginFlexible)
{
  const CGFloat change = newContainerSize - oldContainerSize;
  const CGFloat oldContentSize = *contentSizeInOut;
  const CGFloat oldContentPosition = *contentPositionInOut;
  CGFloat flexibleSpace = 0.0;

  // See how much flexible space we have to distrube the change over

  if (sizeFlexible)
    flexibleSpace += oldContentSize;

  if (minMarginFlexible)
    flexibleSpace += oldContentPosition;

  if (maxMarginFlexible)
    flexibleSpace += oldContainerSize - oldContentPosition - oldContentSize;


  if (flexibleSpace <= 0.0)
    {
      /**
       * In this code path there is no flexible space so we divide 
       * the available space equally among the flexible portions of the view
       */
      int subdivisions = (sizeFlexible ? 1 : 0) +
	(minMarginFlexible ? 1 : 0) +
	(maxMarginFlexible ? 1 : 0);

      if (subdivisions > 0)
	{
	  const CGFloat changePerOption = change / subdivisions;
	  
	  if (sizeFlexible)
	    { 
	      *contentSizeInOut += changePerOption;
	    }
	  if (minMarginFlexible)
	    {
	      *contentPositionInOut += changePerOption;
	    }
	}
    }
  else
    {
      /**
       * In this code path we distribute the change proportionately
       * over the flexible spaces
       */
      const CGFloat changePerPoint = change / flexibleSpace;

      if (sizeFlexible)
	{ 
          *contentSizeInOut += changePerPoint * oldContentSize;
	}
      if (minMarginFlexible)
	{
	  *contentPositionInOut += changePerPoint * oldContentPosition;
	}
    }
}

- (void) resizeWithOldSuperviewSize: (NSSize)oldSize
{
  NSSize superViewFrameSize;
  NSRect newFrame = _frame;
  NSRect newFrameRounded;

  if (_autoresizingMask == NSViewNotSizable)
    return;

  if (!NSEqualRects(NSZeroRect, _autoresizingFrameError))
    {
      newFrame.origin.x -= _autoresizingFrameError.origin.x;
      newFrame.origin.y -= _autoresizingFrameError.origin.y;
      newFrame.size.width -= _autoresizingFrameError.size.width;
      newFrame.size.height -= _autoresizingFrameError.size.height;
    }

  superViewFrameSize = NSMakeSize(0,0);
  if (_super_view)
    superViewFrameSize = [_super_view frame].size;

  autoresize(oldSize.width,
	     superViewFrameSize.width,
	     &newFrame.origin.x,
	     &newFrame.size.width,
	     (_autoresizingMask & NSViewMinXMargin),	     
	     (_autoresizingMask & NSViewWidthSizable),
	     (_autoresizingMask & NSViewMaxXMargin));

  {
    const BOOL flipped = (_super_view && [_super_view isFlipped]);

    autoresize(oldSize.height,
	       superViewFrameSize.height,
	       &newFrame.origin.y,
	       &newFrame.size.height,
	       flipped ? (_autoresizingMask & NSViewMaxYMargin) : (_autoresizingMask & NSViewMinYMargin),
	       (_autoresizingMask & NSViewHeightSizable),
	       flipped ? (_autoresizingMask & NSViewMinYMargin) : (_autoresizingMask & NSViewMaxYMargin));
  }

  newFrameRounded = newFrame;

  /**
   * Perform rounding to pixel-align the frame if we are not rotated
   */
  if (![self isRotatedFromBase] && [self superview] != nil)
    {
      newFrameRounded = [[self superview] centerScanRect: newFrameRounded];
    }

  [self setFrame: newFrameRounded];

  _autoresizingFrameError.origin.x = (newFrameRounded.origin.x - newFrame.origin.x);
  _autoresizingFrameError.origin.y = (newFrameRounded.origin.y - newFrame.origin.y);
  _autoresizingFrameError.size.width = (newFrameRounded.size.width - newFrame.size.width);
  _autoresizingFrameError.size.height = (newFrameRounded.size.height - newFrame.size.height);
}

- (void) _lockFocusInContext: (NSGraphicsContext *)ctxt inRect: (NSRect)rect
{
  NSRect wrect;
  NSInteger window_gstate = 0;

  if (viewIsPrinting == nil)
    {
      NSAssert(_window != nil, NSInternalInconsistencyException);
      /* Check for deferred window */
      if ((window_gstate = [_window gState]) == 0)
        {
          return;
        }
    }

  if (ctxt == nil)
    {
      if (viewIsPrinting != nil)
        {
          NSPrintOperation *printOp = [NSPrintOperation currentOperation];

          ctxt = [printOp context];
        }
      else
        {
          ctxt = [_window graphicsContext];
        }
    }

  // Set current context
  [NSGraphicsContext saveGraphicsState];
  [NSGraphicsContext setCurrentContext: ctxt];

  [ctxt lockFocusView: self inRect: rect];
  wrect = [self convertRect: rect toView: nil];
  NSDebugLLog(@"NSView", @"-lockFocusInRect: %@\n"
	      @"\t for view %@ in window %p (%@)\n"
	      @"\t frame %@, flip %d",
	      NSStringFromRect(wrect),
	      self, _window, NSStringFromRect([_window frame]),
	      NSStringFromRect(_frame), [self isFlipped]);
  if (viewIsPrinting == nil)
    {
      [_window->_rectsBeingDrawn addObject: [NSValue valueWithRect: wrect]];
    }

  /* Make sure we don't modify superview's gstate */
  DPSgsave(ctxt);

  if (viewIsPrinting != nil)
    {
      if (viewIsPrinting == self)
        {
          /* Make sure coordinates are valid, then fake that we don't have
             a superview so we get printed correctly */
          [self _matrixToWindow];
          [_matrixToWindow makeIdentityMatrix];
        }
      else
        {
          [[self _matrixToWindow] concat];
        }

      /* Allow subclases to make other modifications */
      [self setUpGState];
    }
  else
    {
      if (_gstate && !_renew_gstate)
        {
          DPSsetgstate(ctxt, _gstate);
          DPSgsave(ctxt);
        }
      else
        {
          // This only works, when the context comes from the window
          DPSsetgstate(ctxt, window_gstate);
          DPSgsave(ctxt);
          [[self _matrixToWindow] concat];
          
          /* Allow subclases to make other modifications */
          [self setUpGState];
          _renew_gstate = NO;
          if (_allocate_gstate)
            {
              if (_gstate)
                {
                  GSReplaceGState(ctxt, _gstate);
                }
              else
                {
                  _gstate = GSDefineGState(ctxt);
                }
              /* Balance the previous gsave and install our own gstate */
              DPSgrestore(ctxt);
              DPSsetgstate(ctxt, _gstate);
              DPSgsave(ctxt);
            }
        }
    }

  if ([self wantsDefaultClipping])
    {
      /* 
       * Clip to the visible rectangle - which will never be greater
       * than the bounds of the view. This prevents drawing outside
       * our bounds.
       */
      // Normally the second test is not needed, it can differ only
      // when the view is loaded from a NIB file.
      if (_is_rotated_from_base && (_boundsMatrix != nil))
        {
          // When the view is rotated, we clip to the frame.
          NSAffineTransform *matrix;
          NSRect frame = _frame;
          NSBezierPath *bp;

          frame.origin = NSMakePoint(0, 0);
          bp = [NSBezierPath bezierPathWithRect: frame];
          
          matrix = [_boundsMatrix copy];
          [matrix invert];
          [bp transformUsingAffineTransform: matrix];
          [bp addClip];
          RELEASE(matrix);
        }
      else
        { 
          // FIXME: Should we use _bounds or visibleRect here?
          DPSrectclip(ctxt, NSMinX(rect), NSMinY(rect),
                      NSWidth(rect), NSHeight(rect));
        }
    }

  /* Tell backends that images are drawn upside down. Obsolete?
     This is needed when a backend is able to handle full image transformation. */
  GSWSetViewIsFlipped(ctxt, [self isFlipped]);
}

- (void) _setIgnoresBacking: (BOOL) flag
{
  _rFlags.ignores_backing = flag;
}

- (BOOL) _ignoresBacking
{
  return _rFlags.ignores_backing;
}

- (void) unlockFocusNeedsFlush: (BOOL)flush
{
  NSGraphicsContext *ctxt = GSCurrentContext();

  NSDebugLLog(@"NSView_details", @"-unlockFocusNeedsFlush: %i for view %@\n",
	      flush, self);

  if (viewIsPrinting == nil)
    {
      NSAssert(_window != nil, NSInternalInconsistencyException);
      /* Check for deferred window */
      if ([_window gState] == 0)
        return;

      /* Restore our original gstate */
      DPSgrestore(ctxt);
    }

  /* Restore state of nesting lockFocus */
  DPSgrestore(ctxt);
  if (!_allocate_gstate)
    _gstate = 0;

  if (viewIsPrinting == nil)
    {
      NSRect        rect;
      if (flush && !_rFlags.ignores_backing)
        {
          rect = [[_window->_rectsBeingDrawn lastObject] rectValue];
          _window->_rectNeedingFlush =
              NSUnionRect(_window->_rectNeedingFlush, rect);
          _window->_f.needs_flush = YES;
        }
      [_window->_rectsBeingDrawn removeLastObject];
    }
  [ctxt unlockFocusView: self needsFlush: YES ];
  [NSGraphicsContext restoreGraphicsState];
}

/**
  <p> Tell the view to maintain a private gstate object which
  encapsulates all the information about drawing, such as coordinate
  transforms, line widths, etc. If you do not invoke this method, a
  gstate object is constructed each time the view is lockFocused.
  Allocating a private gstate may improve the performance of views
  that are focused a lot and have a lot of customized drawing
  parameters.  </p> 

  <p> View subclasses should override the
  setUpGstate method to set these custom parameters.
  </p> 
*/
- (void) allocateGState
{
  _allocate_gstate = YES;
  _renew_gstate = YES;
}

/**
  Frees the gstate object, if there is one. 
*/
- (void) releaseGState
{
  if (_allocate_gstate && _gstate &&
      _window && ([_window graphicsContext] != nil))
    {
      GSUndefineGState([_window graphicsContext], _gstate);
    }
  _gstate = 0;
  _allocate_gstate = NO;
}

/**
  Returns an identifier that represents the view's gstate object,
  which is used to encapsulate drawing information about the view.
  Most of the time a gstate object is created from scratch when the
  view is focused, so if the view is not currently focused or
  allocateGState has not been called, then this method will return 0.
  FIXME: The above is what the OpenStep and Cocoa specification say, but 
  gState is 0 unless allocateGState has been called. 
*/
- (NSInteger) gState
{
  if (_allocate_gstate && (!_gstate || _renew_gstate))
    {
      // Set the gstate by locking and unlocking focus.
      [self lockFocus];
      [self unlockFocusNeedsFlush: NO];
    }

  return _gstate;
}

/** 
  Invalidates the view's gstate object so it will be set up again
  using setUpGState the next time the view is focused.  */
- (void) renewGState
{
  _renew_gstate = YES;
  /* Note that the next time we lock focus, we'll realloc a gstate (if
     _allocate_gstate). This seems to make sense, and also allows us
     to call this method each time we invalidate the coordinates */
}

/* Overridden by subclasses to setup custom gstate */
- (void) setUpGState
{
}

- (void) lockFocusInRect: (NSRect)rect
{
  [self _lockFocusInContext: nil inRect: rect];
}

- (void) lockFocus
{
  [self lockFocusInRect: [self visibleRect]];
}

- (void) unlockFocus
{
  [self unlockFocusNeedsFlush: YES];
}

- (BOOL) lockFocusIfCanDraw
{
  return [self lockFocusIfCanDrawInContext: nil];
}

- (BOOL) lockFocusIfCanDrawInContext: (NSGraphicsContext *)context
{
  if ([self canDraw])
    {
      [self _lockFocusInContext: context inRect: [self visibleRect]];
      return YES;
    }
  else
    {
      return NO;
    }
}

- (BOOL) canDraw
{
  if (((viewIsPrinting != nil) && [self isDescendantOf: viewIsPrinting]) || 
      ((_window != nil) && ([_window windowNumber] != 0) && 
       ![self isHiddenOrHasHiddenAncestor]))
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/*
 * The following display* methods work based on these invariants:
 * - When a view is marked as needing display, all views above it 
 *   in the hierarchy are marked as well.
 * - When a view has an invalid rectangle, all views above it up 
 *   to the next opaque view also include this invalid rectangle.
 *
 * After drawing an area in a view give, subviews a chance to draw 
 * there too.
 * When drawing a non-opaque subview we need to make sure any area
 * we draw in has been drawn by the opaque superview as well.
 *
 * When drawing the invalid area of a view, we need to make sure 
 * that invalid areas in opaque subviews get drawn as well. These 
 * areas will not be included in the invalid area of the view.
 *
 * IfNeeded means we only draw if the view is marked as needing display
 * and will only draw in the _invalidRect of this view and that of all 
 * the opaque subviews. For non-opaque subviews we need to draw where 
 * ever a superview has already drawn.
 * 
 * InRect means we will only draw in this rectangle. If non is given the
 * visibleRect gets used.
 *
 * IgnoringOpacity means we start drawing at the current view. Otherwise 
 * we go up to the next opaque view.
 *
 */

- (void) display
{
  [self displayRect: [self visibleRect]];
}

- (void) displayIfNeeded
{
  if (_rFlags.needs_display == YES)
    {
      [self displayIfNeededInRect: [self visibleRect]];
    }
}

- (void) displayIfNeededIgnoringOpacity
{
  if (_rFlags.needs_display == YES)
    {
      [self displayIfNeededInRectIgnoringOpacity: [self visibleRect]];
    }
}

- (void) displayIfNeededInRect: (NSRect)aRect
{
  if (_rFlags.needs_display == YES)
    {
      if ([self isOpaque] == YES)
        {
          [self displayIfNeededInRectIgnoringOpacity: aRect];
        }
      else
        {
          NSView *firstOpaque = [self opaqueAncestor];

          aRect = [firstOpaque convertRect: aRect fromView: self];
          [firstOpaque displayIfNeededInRectIgnoringOpacity: aRect];
        }
    }
}

- (void) displayIfNeededInRectIgnoringOpacity: (NSRect)aRect
{
  if (_rFlags.needs_display == YES)
    {
      NSRect rect;
        
      /*
       * Restrict the drawing of self onto the invalid rectangle.
       */
      rect = NSIntersectionRect(aRect, _invalidRect);
      [self displayRectIgnoringOpacity: rect];

      /*
       * If we still need display after displaying the invalid rectangle,
       * this means that some subviews still need to display.
       * For opaque subviews their invalid rectangle may even overlap the
       * original aRect.
       * Display any subview that need display.
       */ 
      if (_rFlags.needs_display == YES)
        {
          NSEnumerator *enumerator = [_sub_views objectEnumerator];
          NSView *subview;
          BOOL subviewNeedsDisplay = NO;
         
          while ((subview = [enumerator nextObject]) != nil)
            {
              if (subview->_rFlags.needs_display)
                {
                  NSRect subviewFrame = [subview _frameExtend];
                  NSRect isect;
              
                  isect = NSIntersectionRect(aRect, subviewFrame);
                  if (NSIsEmptyRect(isect) == NO)
                    {
                      isect = [subview convertRect: isect fromView: self];
                      [subview displayIfNeededInRectIgnoringOpacity: isect];
                    }

                  if (subview->_rFlags.needs_display)
                    {
                      subviewNeedsDisplay = YES;
                    }
                }
            }
          /*
           * Make sure our needs_display flag matches that of the subviews.
           * Only set to NO when there is no _invalidRect.
           */
          if (NSIsEmptyRect(_invalidRect))
            {
              _rFlags.needs_display = subviewNeedsDisplay;
            }
        }
    }
}

/**
 * Causes the area of the view specified by aRect to be displayed.
 * This is done by moving up the view hierarchy until an opaque view
 * is found, then asking that view to update the appropriate area.
 */
- (void) displayRect: (NSRect)aRect
{
  if ([self isOpaque] == YES)
    {
      [self displayRectIgnoringOpacity: aRect];
    }
  else
    {
      NSView *firstOpaque = [self opaqueAncestor];

      aRect = [firstOpaque convertRect: aRect fromView: self];
      [firstOpaque displayRectIgnoringOpacity: aRect];
    }
}

- (void) displayRectIgnoringOpacity: (NSRect)aRect
{
  [self displayRectIgnoringOpacity: aRect inContext: nil];
}

- (void) displayRectIgnoringOpacity: (NSRect)aRect 
                          inContext: (NSGraphicsContext *)context
{
  NSGraphicsContext *wContext;
  BOOL flush = NO;
  BOOL subviewNeedsDisplay = NO;

  if (![self canDraw])
    {
      return;
    }

  wContext = [_window graphicsContext];
  if (context == nil)
    {
      context = wContext;
    }

  if (context == wContext)
    {
      NSRect neededRect;
      NSRect visibleRect = [self visibleRect];

      flush = YES;
      [_window disableFlushWindow];
      aRect = NSIntersectionRect(aRect, visibleRect);
      neededRect = NSIntersectionRect(_invalidRect, visibleRect);
  
      /*
       * If the rect we are going to display contains the _invalidRect
       * then we can empty _invalidRect. Do this before the drawing,
       * as drawRect: may change this value.
       * FIXME: If the drawn rectangle cuts of a complete part of the
       * _invalidRect, we should try to reduce this.
       */
      if (NSEqualRects(aRect, NSUnionRect(neededRect, aRect)) == YES)
        {
          _invalidRect = NSZeroRect;
          _rFlags.needs_display = NO;
        }
    }
  
  if (NSIsEmptyRect(aRect) == NO)
    {
      /*
       * Now we draw this view.
       */
      [self _lockFocusInContext: context inRect: aRect];
      [self drawRect: aRect];
      [self unlockFocusNeedsFlush: flush];
    }

  /*
   * Even when aRect is empty we need to loop over the subviews to see, 
   * if there is anything left to draw.
   */
  if (_rFlags.has_subviews == YES)
    {
      NSUInteger count = [_sub_views count];

      if (count > 0)
        {
          NSView *array[count];
          NSUInteger i;
          
          [_sub_views getObjects: array];

          for (i = 0; i < count; ++i)
            {
              NSView *subview = array[i];
              NSRect subviewFrame = [subview _frameExtend];
              NSRect isect;
              
              /*
               * Having drawn ourself into the rect, we must make sure that
               * subviews overlapping the area are redrawn.
               */
              isect = NSIntersectionRect(aRect, subviewFrame);
              if (NSIsEmptyRect(isect) == NO)
                {
                  isect = [subview convertRect: isect fromView: self];
                  [subview displayRectIgnoringOpacity: isect
                                            inContext: context];
                }
              /*
               * Is there still something to draw in the subview?
               * This keeps the invariant that views further up are marked
               * for redraw when ever a view further down needs to redraw.
               */
              if (subview->_rFlags.needs_display == YES)
                {
                  subviewNeedsDisplay = YES;
                }
            }
        }
    }

  if (context == wContext)
    {
      if (subviewNeedsDisplay)
        {
          /*
           * If not all subviews have been fully displayed, we cannot turn off
           * the 'needs_display' flag. This is to keep the invariant that when 
           * a view is marked as needing to display, all its ancestors will be 
           * marked too.
           */
          _rFlags.needs_display = YES;
        }
      [_window enableFlushWindow];
      [_window flushWindowIfNeeded];
    }
}

/**
  This method is invoked to handle drawing inside the view.  The
  default NSView's implementation does nothing; subclasses might
  override it to draw something inside the view.  Since NSView's
  implementation is guaranteed to be empty, you should not call
  super's implementation when you override it in subclasses.
  drawRect: is invoked when the focus has already been locked on the
  view; you can use arbitrary postscript functions in drawRect: to
  draw inside your view; the coordinate system in which you draw is
  the view's own coordinate system (this means for example that you
  should refer to the rectangle covered by the view using its bounds,
  and not its frame).  The argument of drawRect: is the rectangle
  which needs to be redrawn.  In a lossy implementation, you can
  ignore the argument and redraw the whole view; if you are aiming at
  performance, you may want to redraw only what is inside the
  rectangle which needs to be redrawn; this usually improves drawing
  performance considerably.  */
- (void) drawRect: (NSRect)rect
{}

- (NSRect) visibleRect
{
  if ([self isHiddenOrHasHiddenAncestor])
    {
      return NSZeroRect;
    }

  if (_coordinates_valid == NO)
    {
      [self _rebuildCoordinates];
    }
  return _visibleRect;
}

- (BOOL) wantsDefaultClipping
{
  return YES;
}

- (BOOL) needsToDrawRect: (NSRect)aRect
{
  const NSRect *rects;
  NSInteger i, count;

  [self getRectsBeingDrawn: &rects count: &count];
  for (i = 0; i < count; i++)
    {
      if (NSIntersectsRect(aRect, rects[i]))
	return YES;
    }
  return NO;
}

- (void) getRectsBeingDrawn: (const NSRect **)rects count: (NSInteger *)count
{
  // FIXME
  static NSRect rect;

  rect = [[_window->_rectsBeingDrawn lastObject] rectValue];
  rect = [self convertRect: rect fromView: nil];

  if (rects != NULL)
    {
      *rects = &rect;
    }

  if (count != NULL)
    {
      *count = 1;
    }
}

- (NSBitmapImageRep *) bitmapImageRepForCachingDisplayInRect: (NSRect)rect
{
  NSBitmapImageRep *bitmap;

  [self lockFocus];
  bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect: rect];
  [self unlockFocus];

  return AUTORELEASE(bitmap);
}

- (void) cacheDisplayInRect: (NSRect)rect 
           toBitmapImageRep: (NSBitmapImageRep *)bitmap
{
  NSDictionary *dict;
  NSData *imageData;

  [self lockFocus];
  dict = [GSCurrentContext() GSReadRect: rect];
  [self unlockFocus];
  imageData = [dict objectForKey: @"Data"];

  if (imageData != nil)
    {
      // Copy the image data to the bitmap
      memcpy([bitmap bitmapData], [imageData bytes], [imageData length]);
    }
}


extern NSThread *GSAppKitThread; /* TODO */

/*
For -setNeedsDisplay*, the real work is done in the ..._real methods, and
the actual public method simply calls it, but makes sure that the call is
in the main thread.
*/

- (void) _setNeedsDisplay_real: (NSNumber *)n
{
  BOOL flag = [n boolValue];

  if (flag)
    {
      [self setNeedsDisplayInRect: _bounds];
    }
  else
    {
      _rFlags.needs_display = NO;
      _invalidRect = NSZeroRect;
    }
}

/**
 * As an exception to the general rules for threads and gui, this
 * method is thread-safe and may be called from any thread. Display
 * will always be done in the main thread. (Note that other methods are
 * in general not thread-safe; if you want to access other properties of
 * views from multiple threads, you need to provide the synchronization.)
 */
- (void) setNeedsDisplay: (BOOL)flag
{
  NSNumber *n = [[NSNumber alloc] initWithBool: flag];
  if (GSCurrentThread() != GSAppKitThread)
    {
      NSDebugMLLog (@"MacOSXCompatibility", 
                    @"setNeedsDisplay: called on secondary thread");
      [self performSelectorOnMainThread: @selector(_setNeedsDisplay_real:)
            withObject: n
            waitUntilDone: NO];
    }
  else
    {
      [self _setNeedsDisplay_real: n];
    }
  DESTROY(n);
}


- (void) _setNeedsDisplayInRect_real: (NSValue *)v
{
  NSRect invalidRect = [v rectValue];
  NSView *currentView = _super_view;

  /*
   *	Limit to bounds, combine with old _invalidRect, and then check to see
   *	if the result is the same as the old _invalidRect - if it isn't then
   *	set the new _invalidRect.
   */
  invalidRect = NSIntersectionRect(invalidRect, _bounds);
  invalidRect = NSUnionRect(_invalidRect, invalidRect);
  if (NSEqualRects(invalidRect, _invalidRect) == NO)
    {
      NSView	*firstOpaque = [self opaqueAncestor];

      _rFlags.needs_display = YES;
      _invalidRect = invalidRect;
      if (firstOpaque == self)
        {
	  /**
	   * Enlarge (if necessary) _invalidRect so it lies on integral device pixels 
	   */
	  const NSRect inBase =  [self convertRectToBase: _invalidRect];
	  const NSRect inBaseRounded = NSIntegralRect(inBase);
	  _invalidRect = [self convertRectFromBase: inBaseRounded];

          [_window setViewsNeedDisplay: YES];
        }
      else
        {
          invalidRect = [firstOpaque convertRect: _invalidRect fromView: self];
          [firstOpaque setNeedsDisplayInRect: invalidRect];
        }
    }

 /*
   * Must make sure that superviews know that we need display.
   * NB. we may have been marked as needing display and then moved to another
   * parent, so we can't assume that our parent is marked simply because we are.
   */
  while (currentView)
    {
      currentView->_rFlags.needs_display = YES;
      currentView = currentView->_super_view;
    }
  // Also mark the window, as this may not happen above
  [_window setViewsNeedDisplay: YES];
}

/**
 * Inform the view system that the specified rectangle is invalid and
 * requires updating.  This automatically informs any superviews of
 * any updating they need to do.
 *
 * As an exception to the general rules for threads and gui, this
 * method is thread-safe and may be called from any thread. Display
 * will always be done in the main thread. (Note that other methods are
 * in general not thread-safe; if you want to access other properties of
 * views from multiple threads, you need to provide the synchronization.)
 */
- (void) setNeedsDisplayInRect: (NSRect)invalidRect
{
  NSValue *v;

  if (NSIsEmptyRect(invalidRect))
    return; // avoid unnecessary work when rectangle is empty
	
  v = [[NSValue alloc]
		 initWithBytes: &invalidRect
		 objCType: @encode(NSRect)];

  if (GSCurrentThread() != GSAppKitThread)
    {
      NSDebugMLLog (@"MacOSXCompatibility", 
                    @"setNeedsDisplayInRect: called on secondary thread");
      [self performSelectorOnMainThread: @selector(_setNeedsDisplayInRect_real:)
            withObject: v
            waitUntilDone: NO];
    }
  else
    {
      [self _setNeedsDisplayInRect_real: v];
    }
  DESTROY(v);
}

+ (NSFocusRingType) defaultFocusRingType
{
  return NSFocusRingTypeDefault;
}

- (void) setKeyboardFocusRingNeedsDisplayInRect: (NSRect)rect
{
  // FIXME For external type special handling is needed
  [self setNeedsDisplayInRect: rect];
}

- (void) setFocusRingType: (NSFocusRingType)focusRingType
{
  _focusRingType = focusRingType;
}

- (NSFocusRingType) focusRingType
{
  return _focusRingType;
}

/*
 * Hidding Views
 */
- (void) setHidden: (BOOL)flag
{
  id view;

  if (_is_hidden == flag)
      return;

  _is_hidden = flag;

  if (_is_hidden)
    {
      for (view = [_window firstResponder];
           view != nil && [view respondsToSelector: @selector(superview)];
           view = [view superview])
        {
          if (view == self)
            {
              [_window makeFirstResponder: [self nextValidKeyView]];
              break;
            }
        }
      if (_rFlags.has_draginfo)
        {
          if (_window != nil)
            {
              NSArray *t = GSGetDragTypes(self);
              
              [GSDisplayServer removeDragTypes: t fromWindow: _window];
            }
        }
      [[self superview] setNeedsDisplay: YES];
    }
  else
    {
      if (_rFlags.has_draginfo)
        {
          if (_window != nil)
            {
              NSArray *t = GSGetDragTypes(self);
              
              [GSDisplayServer addDragTypes: t toWindow: _window];
            }
        }
      if (_rFlags.has_subviews)
        {
          // The _visibleRect of subviews will be NSZeroRect, because when they
          // were calculated in -[_rebuildCoordinates], they were intersected
          // with the result of calling -[visibleRect] on the hidden superview,
          // which returns NSZeroRect for hidden views.
          //
          // So, recalculate the subview coordinates now to make them correct.

          [_sub_views makeObjectsPerformSelector: 
            @selector(_invalidateCoordinates)];
        }
      [self setNeedsDisplay: YES];
    }
}

- (BOOL) isHidden
{
  return _is_hidden;
}

- (BOOL) isHiddenOrHasHiddenAncestor
{
  return ([self isHidden] || [_super_view isHiddenOrHasHiddenAncestor]);
}

/*
 * Live resize support
 */
- (BOOL) inLiveResize
{
  return _in_live_resize;
}

- (void) viewWillStartLiveResize
{
  // FIXME
  _in_live_resize = YES; 
}

- (void) viewDidEndLiveResize
{
  // FIXME
  _in_live_resize = NO; 
}

- (BOOL) preservesContentDuringLiveResize
{
  return NO;
}

- (void) getRectsExposedDuringLiveResize: (NSRect[4])exposedRects count: (NSInteger *)count
{
  // FIXME
  if (count != NULL)
    {
      *count = 1;
    }
  exposedRects[0] = _bounds;
}

- (NSRect) rectPreservedDuringLiveResize
{
  return NSZeroRect;
}

/*
 * Scrolling
 */
- (NSRect) adjustScroll: (NSRect)newVisible
{
  return newVisible;
}

/**
 * Finds the nearest enclosing NSClipView and, if the location of the event
 * is outside it, scrolls the NSClipView in the direction of the event. The
 * amount scrolled is proportional to how far outside the NSClipView the
 * event's location is.
 *
 * This method is suitable for calling periodically from a modal event
 * tracking loop when the mouse is dragged outside the tracking view. The
 * suggested period of the calls is 0.1 seconds.
 */
- (BOOL) autoscroll: (NSEvent*)theEvent
{
  if (_super_view)
    return [_super_view autoscroll: theEvent];

  return NO;
}

- (void) reflectScrolledClipView: (NSClipView*)aClipView
{
}

- (void) scrollClipView: (NSClipView*)aClipView toPoint: (NSPoint)aPoint
{
  [aClipView scrollToPoint: aPoint];
}

- (NSClipView*) _enclosingClipView
{
  static Class clipViewClass;
  id aView = [self superview];

  if (!clipViewClass)
    {
      clipViewClass = [NSClipView class];
    }

  while (aView != nil)
    {
      if ([aView isKindOfClass: clipViewClass])
	{
	  break;
	}
      aView = [aView superview];
    }

  return aView;
}

- (void) scrollPoint: (NSPoint)aPoint
{
  NSClipView *s = [self _enclosingClipView];

  if (s == nil)
    return;

  aPoint = [self convertPoint: aPoint toView: s];
  if (NSEqualPoints(aPoint, [s bounds].origin) == NO)
    {
      [s scrollToPoint: aPoint];
    }
}

/**
   Copy on scroll method, should be called from [NSClipView setBoundsOrigin].
 */
- (void) scrollRect: (NSRect)aRect by: (NSSize)delta
{
  NSPoint destPoint;

  aRect = NSIntersectionRect(aRect, _bounds);   // Don't copy stuff outside.
  destPoint = aRect.origin;
  destPoint.x += delta.width;
  destPoint.y += delta.height;
  if ([self isFlipped])
    {
      destPoint.y += aRect.size.height;
    }

  //NSLog(@"destPoint %@ in %@", NSStringFromPoint(destPoint), NSStringFromRect(_bounds));

  [self lockFocus];
  //NSCopyBits(0, aRect, destPoint);
  NSCopyBits([[self window] gState], [self convertRect: aRect toView: nil], destPoint);
  [self unlockFocus];
}

/**
Scrolls the nearest enclosing clip view the minimum required distance
necessary to make aRect (or as much of it possible) in the receiver visible.
Returns YES iff any scrolling was done.
*/
- (BOOL) scrollRectToVisible: (NSRect)aRect
{
  NSClipView *s = [self _enclosingClipView];

  if (s != nil)
    {
      NSRect	vRect = [s documentVisibleRect];
      NSPoint	aPoint = vRect.origin;
      // Ok we assume that the rectangle is origined at the bottom left
      // and goes to the top and right as it grows in size for the naming
      // of these variables
      CGFloat ldiff, rdiff, tdiff, bdiff;

      if (vRect.size.width == 0 && vRect.size.height == 0)
	return NO;

      aRect = [self convertRect: aRect toView: [s documentView]];

      // Find the differences on each side.
      ldiff = NSMinX(vRect) - NSMinX(aRect);
      rdiff = NSMaxX(aRect) - NSMaxX(vRect);
      bdiff = NSMinY(vRect) - NSMinY(aRect);
      tdiff = NSMaxY(aRect) - NSMaxY(vRect);

      // If the diff's have the same sign then nothing needs to be scrolled
      if ((ldiff * rdiff) >= 0.0) ldiff = rdiff = 0.0;
      if ((bdiff * tdiff) >= 0.0) bdiff = tdiff = 0.0;

      // Move the smallest difference
      aPoint.x += (fabs(ldiff) < fabs(rdiff)) ? (-ldiff) : rdiff;
      aPoint.y += (fabs(bdiff) < fabs(tdiff)) ? (-bdiff) : tdiff;

      if (aPoint.x != vRect.origin.x || aPoint.y != vRect.origin.y)
	{
	  aPoint = [[s documentView] convertPoint: aPoint toView: s];
	  [s scrollToPoint: aPoint];
	  return YES;
	}
    }
  return NO;
}

- (NSScrollView*) enclosingScrollView
{
  static Class scrollViewClass;
  id	aView = [self superview];

  if (!scrollViewClass)
    {
      scrollViewClass = [NSScrollView class];
    }

  while (aView != nil)
    {
      if ([aView isKindOfClass: scrollViewClass])
	{
	  break;
	}
      aView = [aView superview];
    }

  return aView;
}

/*
 * Managing the Cursor
 *
 * We use the tracking rectangle class to maintain the cursor rects
 */
- (void) addCursorRect: (NSRect)aRect cursor: (NSCursor*)anObject
{
  if (_window != nil)
    {
      GSTrackingRect	*m;

      aRect = [self convertRect: aRect toView: nil];
      m = [rectClass allocWithZone: NSDefaultMallocZone()];
      m = [m initWithRect: aRect
		      tag: 0
		    owner: RETAIN(anObject)
		 userData: NULL
		   inside: YES];
      [_cursor_rects addObject: m];
      RELEASE(m);
      _rFlags.has_currects = 1;
      _rFlags.valid_rects = 1;
    }
}

- (void) discardCursorRects
{
  if (_rFlags.has_currects != 0)
    {
      NSUInteger count = [_cursor_rects count];

      if (count > 0)
        {
	  GSTrackingRect *rects[count];

	  [_cursor_rects getObjects: rects];
	  if (_rFlags.valid_rects != 0)
	    {
	      NSPoint loc = _window->_lastPoint;
	      NSUInteger i;

	      for (i = 0; i < count; ++i)
		{
		  GSTrackingRect *r = rects[i];
		  if (NSMouseInRect(loc, r->rectangle, NO))
		    {
		      [r->owner mouseExited: nil];
		    }
		  [r invalidate];
		}
	      _rFlags.valid_rects = 0;
	    }
	  while (count-- > 0)
	    {
	      RELEASE([rects[count] owner]);
	    }
	  [_cursor_rects removeAllObjects];
	}
      _rFlags.has_currects = 0;
    }
}

- (void) removeCursorRect: (NSRect)aRect cursor: (NSCursor*)anObject
{
  id e = [_cursor_rects objectEnumerator];
  GSTrackingRect	*o;
  NSCursor		*c;
  NSPoint loc = [_window mouseLocationOutsideOfEventStream];

  /* Base remove test upon cursor object */
  o = [e nextObject];
  while (o)
    {
      c = [o owner];
      if (c == anObject)
	{
	  if (NSMouseInRect(loc, o->rectangle, NO))
	    {
	      [c mouseExited: nil];
	    }
	  [o invalidate];
	  [_cursor_rects removeObject: o];
	  if ([_cursor_rects count] == 0)
	    {
	      _rFlags.has_currects = 0;
	      _rFlags.valid_rects = 0;
	    }
	  RELEASE(c);
	  break;
	}
      else
	{
	  o = [e nextObject];
	}
    }
}

- (void) resetCursorRects
{
}

static NSView* findByTag(NSView *view, NSInteger aTag, NSUInteger *level)
{
  NSUInteger i, count;
  NSArray *sub = [view subviews];

  count = [sub count];
  if (count > 0)
    {
      NSView	*array[count];

      [sub getObjects: array];

      for (i = 0; i < count; i++)
	{
	  if ([array[i] tag] == aTag)
	    return array[i];
	}
      *level += 1;
      for (i = 0; i < count; i++)
	{
	  NSView	*v;

	  v = findByTag(array[i], aTag, level);
	  if (v != nil)
	    return v;
	}
      *level -= 1;
    }
  return nil;
}

- (id) viewWithTag: (NSInteger)aTag
{
  NSView	*view = nil;

  /*
   * If we have the specified tag - return self.
   */
  if ([self tag] == aTag)
    {
      view = self;
    }
  else if (_rFlags.has_subviews)
    {
      NSUInteger count = [_sub_views count];

      if (count > 0)
	{
	  NSView *array[count];
	  NSUInteger i;

	  [_sub_views getObjects: array];

	  /*
	   * Quick check to see if any of our direct descendents has the tag.
	   */
	  for (i = 0; i < count; i++)
	    {
	      NSView *subView = array[i];

	      if ([subView tag] == aTag)
	        {
		  view = subView;
		  break;
		}
	    }

	  if (view == nil)
	    {
	      NSUInteger level = 0xffffffff;

	      /*
	       * Ok - do it the long way - search the whole tree for each of
	       * our descendents and see which has the closest view matching
	       * the tag.
	       */
	      for (i = 0; i < count; i++)
		{
		  NSUInteger l = 0;
		  NSView *v;

		  v = findByTag(array[i], aTag, &l);

		  if (v != nil && l < level)
		    {
		      view = v;
		      level = l;
		    }
		}
	    }
	}
    }
  return view;
}

/*
 * Aiding Event Handling
 */

/**
 * Returns YES if the view object will accept the first
 * click received when in an inactive window, and NO
 * otherwise.
 */
- (BOOL) acceptsFirstMouse: (NSEvent*)theEvent
{
  return NO;
}

/**
 * Returns the subview, lowest in the receiver's hierarchy, which
 * contains aPoint, or nil if there is no such view.
 */
- (NSView*) hitTest: (NSPoint)aPoint
{
  NSPoint p;
  NSView *v = nil, *w;

  /* If not within our frame then it can't be a hit.

  As a special case, always assume that it's a hit if our _super_view is nil,
  ie. if we're the top-level view in a window.
  */

  if ([self isHidden])
    {
      return nil;
    }

  if (_is_rotated_or_scaled_from_base)
    {
      p = [self convertPoint: aPoint fromView: _super_view];
      if (!NSPointInRect (p, _bounds))
        {
          return nil;
        }
    }
  else if (_super_view && ![_super_view mouse: aPoint inRect: _frame])
    {
      return nil;
    }
  else
    {
      p = [self convertPoint: aPoint fromView: _super_view];
    }

  if (_rFlags.has_subviews)
    {
      NSUInteger count;

      count = [_sub_views count];
      if (count > 0)
        {
          NSView *array[count];

          [_sub_views getObjects: array];
          
          while (count > 0)
            {
              w = array[--count];
              v = [w hitTest: p];
              if (v)
                break;
            }
        }
    }
  /*
   * mouse is either in the subview or within self
   */
  if (v)
    return v;
  else
    return self;
}

/**
 * Returns whether or not aPoint lies within aRect.
 */
- (BOOL) mouse: (NSPoint)aPoint  inRect: (NSRect)aRect
{
  return NSMouseInRect (aPoint, aRect, [self isFlipped]);
}

- (BOOL) performKeyEquivalent: (NSEvent*)theEvent
{
  NSUInteger i;

  for (i = 0; i < [_sub_views count]; i++)
    if ([[_sub_views objectAtIndex: i] performKeyEquivalent: theEvent] == YES)
      return YES;
  return NO;
}

- (BOOL) performMnemonic: (NSString *)aString
{
  NSUInteger i;

  for (i = 0; i < [_sub_views count]; i++)
    if ([[_sub_views objectAtIndex: i] performMnemonic: aString] == YES)
      return YES;
  return NO;
}

- (BOOL) mouseDownCanMoveWindow
{
  return ![self isOpaque];
}

- (void) removeTrackingRect: (NSTrackingRectTag)tag
{
  NSUInteger i, j;
  GSTrackingRect	*m;

  j = [_tracking_rects count];
  for (i = 0;i < j; ++i)
    {
      m = (GSTrackingRect*)[_tracking_rects objectAtIndex: i];
      if ([m tag] == tag)
	{
	  [m invalidate];
	  [_tracking_rects removeObjectAtIndex: i];
	  if ([_tracking_rects count] == 0)
	    {
	      _rFlags.has_trkrects = 0;
	    }
	  return;
	}
    }
}

- (BOOL) shouldDelayWindowOrderingForEvent: (NSEvent*)anEvent
{
  return NO;
}

- (NSTrackingRectTag) addTrackingRect: (NSRect)aRect
				owner: (id)anObject
			     userData: (void*)data
			 assumeInside: (BOOL)flag
{
  NSTrackingRectTag	t;
  NSUInteger		i, j;
  GSTrackingRect	*m;

  t = 0;
  j = [_tracking_rects count];
  for (i = 0; i < j; ++i)
    {
      m = (GSTrackingRect*)[_tracking_rects objectAtIndex: i];
      if ([m tag] > t)
	t = [m tag];
    }
  ++t;

  m = [[rectClass alloc] initWithRect: aRect
				  tag: t
				owner: anObject
			     userData: data
			       inside: flag];
  [_tracking_rects addObject: m];
  RELEASE(m);
  _rFlags.has_trkrects = 1;
  return t;
}

-(BOOL) needsPanelToBecomeKey
{
  return NO;
}


/**
 * <p>The effect of the -setNextKeyView: method is to set aView to be the
 * value returned by subsequent calls to the receivers -nextKeyView method.
 * This also has the effect of setting the previous key view of aView,
 * so that subsequent calls to its -previousKeyView method will return
 * the receiver.
 * </p>
 * <p>As a special case, if you pass nil as aView then the -previousKeyView
 * of the receivers current -nextKeyView is set to nil as well as the
 * receivers -nextKeyView being set to nil.<br />
 * This behavior provides MacOS-X compatibility.
 * </p>
 * <p>If you pass a non-view object other than nil, an
 * NSInternaInconsistencyException is raised.
 * </p>
 * <p><strong>NB</strong> This method does <em>NOT</em> cause aView to be
 * retained, and if aView is deallocated, the [NSView-dealloc] method will
 * automatically remove it from the key view chain it is in.
 * </p>
 * <p>For keyboard navigation, views are linked together in a chain, so that
 * the current first responder view can be changed by stepping backward
 * and forward in that chain.  This is the method for building and modifying
 * that chain.
 * </p>
 * <p>The MacOS-X documentation refers to this chain as a <em>loop</em>, but
 * the actual implementation is not a loop at all (except as a special case
 * when you make the chain into a loop).  In fact, while each view may have
 * only zero or one <em>next</em> view, and zero or one <em>previous</em>
 * view, several views may have their <em>next</em> view set to a single
 * view and/or their <em>previous</em> views set to a single view.  So the
 * actual setup is a directed graph rather than a loop.
 * </p>
 * <p>While a directed graph is a very powerful and flexible way of managing
 * the way views get keyboard focus in response to  tabs etc, it can be
 * confusing if misused.  It is probably best therefore, to set your views
 * up as a single loop within each window.
 * </p>
 * <example>
 *   [a setNextKeyView: b];
 *   [b setNextKeyView: c];
 *   [c setNextKeyView: d];
 *   [d setNextKeyView: a];
 * </example>
 */
- (void) setNextKeyView: (NSView *)aView
{
  NSView	*tmp;
  NSUInteger	count;

  if (aView != nil && [aView isKindOfClass: viewClass] == NO)
    {
      [NSException raise: NSInternalInconsistencyException
	format: @"[NSView -setNextKeyView:] passed non-view object %@", aView];
    }

  if (aView == nil)
    {
      if (nKV(self) != 0)
	{
	  tmp = GSIArrayItemAtIndex(nKV(self), 0).obj;
	  if (tmp != nil)
	    {
	      /*
	       * Remove all reference to self from our next key view.
	       */
	      if (pKV(tmp) != 0)
		{
		  count = GSIArrayCount(pKV(tmp));
		  while (count-- > 1)
		    {
		      if (GSIArrayItemAtIndex(pKV(tmp), count).obj == self)
			{
			  GSIArrayRemoveItemAtIndex(pKV(tmp), count);
			}
		    }
		  if (GSIArrayItemAtIndex(pKV(tmp), 0).obj == self)
		    {
		      GSIArraySetItemAtIndex(pKV(tmp), (GSIArrayItem)nil, 0);
		    }
		}
	      /*
	       * Clear link to the next key view.
	       */
	      GSIArraySetItemAtIndex(nKV(self), (GSIArrayItem)nil, 0);
	    }
	}
      return;
    }

  if (nKV(self) == 0)
    {
      /*
       * Create array and ensure that it has a nil item at index 0 ...
       * so we always have room for the pointer to the next view.
       */
      _nextKeyView = NSZoneMalloc(NSDefaultMallocZone(), sizeof(GSIArray_t));
      GSIArrayInitWithZoneAndCapacity(nKV(self), NSDefaultMallocZone(), 1);
      GSIArrayAddItem(nKV(self), (GSIArrayItem)nil);
    }
  else
    {
      /* A safety measure against recursion.  */
      tmp = GSIArrayItemAtIndex(nKV(self), 0).obj;
      if (tmp == aView)
	{
	  return;
	}
    }

  if (pKV(aView) == 0)
    {
      /*
       * Create array and ensure that it has a nil item at index 0 ...
       * so we always have room for the pointer to the previous view.
       */
      aView->_previousKeyView = NSZoneMalloc(NSDefaultMallocZone(), sizeof(GSIArray_t));
      GSIArrayInitWithZoneAndCapacity(pKV(aView), NSDefaultMallocZone(), 1);
      GSIArrayAddItem(pKV(aView), (GSIArrayItem)nil);
    }

  /*
   * Tell the old previous view of aView that aView no longer points to it.
   */
  tmp = GSIArrayItemAtIndex(pKV(aView), 0).obj;
  if (tmp != nil)
    {
      count = GSIArrayCount(nKV(tmp));
      while (count-- > 1)
	{
	  if (GSIArrayItemAtIndex(nKV(tmp), count).obj == aView)
	    {
	      GSIArrayRemoveItemAtIndex(nKV(tmp), count);
	    }
	}
      /*
       * If the view still points to aView, make a note of it in the
       * 'previous' array of aView while making space for the new link.
       */
      if (GSIArrayItemAtIndex(nKV(tmp), 0).obj == aView)
	{
	  GSIArrayInsertItem(pKV(aView), (GSIArrayItem)nil, 0);
	}
    }

  /*
   * Set up 'previous' link in aView to point to us.
   */
  GSIArraySetItemAtIndex(pKV(aView), (GSIArrayItem)((id)self), 0);

  /*
   * Tell our current 'next' view that we are no longer pointing to it.
   */
  tmp = GSIArrayItemAtIndex(nKV(self), 0).obj;
  if (tmp != nil)
    {
      count = GSIArrayCount(pKV(tmp));
      while (count-- > 1)
	{
	  if (GSIArrayItemAtIndex(pKV(tmp), count).obj == self)
	    {
	      GSIArrayRemoveItemAtIndex(pKV(tmp), count);
	    }
	}
      if (GSIArrayItemAtIndex(pKV(tmp), 0).obj == self)
	{
	  GSIArraySetItemAtIndex(pKV(tmp), (GSIArrayItem)nil, 0);
	}
    }

  /*
   * Set up 'next' link to point to aView.
   */
  GSIArraySetItemAtIndex(nKV(self), (GSIArrayItem)((id)aView), 0);
}

/**
 * Returns the next view after the receiver in the key view chain.<br />
 * Returns nil if there is no view after the receiver.<br />
 * The next view is set up using the -setNextKeyView: method.<br />
 * The key view chain is used to determine the order in which views become
 * first responder when using keyboard navigation.
 */
- (NSView *) nextKeyView
{
  if (nKV(self) == 0)
    {
      return nil;
    }
  return GSIArrayItemAtIndex(nKV(self), 0).obj;
}

/**
 * Returns the first available view after the receiver which is
 * actually able to become first responder. See -nextKeyView and
 * [NSResponder-acceptsFirstResponder]
 */
- (NSView *) nextValidKeyView
{
  NSView *theView;

  theView = [self nextKeyView];
  while (1)
    {
      if ((theView == nil) || (theView == self) || 
	  [theView canBecomeKeyView])
	{
	  return theView;
	}
      theView = [theView nextKeyView];
    }
}

/**
 * GNUstep addition ... a conveninece method to insert a view in the
 * key view chain before the receiver, using the -previousKeyView and
 * -setNextKeyView: methods.
 */
- (void) setPreviousKeyView: (NSView *)aView
{
  NSView	*p = [self previousKeyView];

  if (aView == p || aView == self)
    {
      return;
    }
  [p setNextKeyView: aView];
  [aView setNextKeyView: self];
}

/**
 * Returns the view before the receiver in the key view chain.<br />
 * Returns nil if there is no view before the receiver in the chain.<br />
 * The previous view of the receiver was set up by passing it as the
 * argument to a call of -setNextKeyView: on that view.<br />
 * The key view chain is used to determine the order in which views become
 * first responder when using keyboard navigation.
 */
- (NSView *) previousKeyView
{
  if (pKV(self) == 0)
    {
      return nil;
    }
  return GSIArrayItemAtIndex(pKV(self), 0).obj;
}

/**
 * Returns the first available view before the receiver which is
 * actually able to become first responder. See -nextKeyView and
 * [NSResponder-acceptsFirstResponder]
 */
- (NSView *) previousValidKeyView
{
  NSView *theView;

  theView = [self previousKeyView];
  while (1)
    {
      if ((theView == nil) || (theView == self) || 
	  [theView canBecomeKeyView])
	{
	  return theView;
	}
      theView = [theView previousKeyView];
    }
}

- (BOOL) canBecomeKeyView
{
  // FIXME
  return [self acceptsFirstResponder] && ![self isHiddenOrHasHiddenAncestor];
}

/*
 * Dragging
 */
- (BOOL) dragFile: (NSString*)filename
	 fromRect: (NSRect)rect
	slideBack: (BOOL)slideFlag
	    event: (NSEvent*)event
{
  NSImage *anImage = [[NSWorkspace sharedWorkspace] iconForFile: filename];
  NSPasteboard *pboard = [NSPasteboard pasteboardWithName: NSDragPboard];

  if (anImage == nil)
    return NO;

  [pboard declareTypes: [NSArray arrayWithObject: NSFilenamesPboardType] 
	  owner: self];
  if (![pboard setPropertyList: [NSArray arrayWithObject: filename]
	       forType: NSFilenamesPboardType])
    return NO;

  [self dragImage: anImage
	at: rect.origin
	offset: NSMakeSize(0, 0)
	event: event
	pasteboard: pboard
	source: self
	slideBack: slideFlag];
  return YES;
}

- (void) dragImage: (NSImage*)anImage
		at: (NSPoint)viewLocation
	    offset: (NSSize)initialOffset
	     event: (NSEvent*)event
	pasteboard: (NSPasteboard*)pboard
	    source: (id)sourceObject
	 slideBack: (BOOL)slideFlag
{
  [_window dragImage: anImage
	   at: [self convertPoint: viewLocation toView: nil]
	   offset: initialOffset
	   event: event
	   pasteboard: pboard
	   source: sourceObject
	   slideBack: slideFlag];
}

/**
 * Registers the fact that the receiver should accept dragged data
 * of any of the specified types.  You need to do this if you want
 * your view to support drag and drop.
 */
- (void) registerForDraggedTypes: (NSArray*)newTypes
{
  NSArray	*o;
  NSArray	*t;

  if (newTypes == nil || [newTypes count] == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"Types information missing"];

  /*
   * Get the old drag types for this view if we need to tell the context
   * to change the registered types for the window.
   */
  if (_rFlags.has_draginfo == 1 && _window != nil)
    {
      o = TEST_RETAIN(GSGetDragTypes(self));
    }
  else
    {
      o = nil;
    }

  t = GSSetDragTypes(self, newTypes);
  _rFlags.has_draginfo = 1;
  if (_window != nil)
    {
      // Remove the old types first, that way overlapping types stay assigned.
      if (o != nil)
	{
	  [GSDisplayServer removeDragTypes: o fromWindow: _window];
	}
      [GSDisplayServer addDragTypes: t toWindow: _window];
    }
  TEST_RELEASE(o);
}

- (void) unregisterDraggedTypes
{
  if (_rFlags.has_draginfo)
    {
      if (_window != nil)
	{
	  NSArray		*t = GSGetDragTypes(self);

	  [GSDisplayServer removeDragTypes: t fromWindow: _window];
	}
      GSRemoveDragTypes(self);
      _rFlags.has_draginfo = 0;
    }
}

- (NSArray *) registeredDraggedTypes
{
  return GSGetDragTypes(self);
}

- (BOOL) dragPromisedFilesOfTypes: (NSArray *)typeArray
                         fromRect: (NSRect)aRect
                           source: (id)sourceObject 
                        slideBack: (BOOL)slideBack
                            event: (NSEvent *)theEvent
{
  // FIXME: Where to get the image from?
  NSImage *anImage = nil;
  NSPasteboard *pboard = [NSPasteboard pasteboardWithName: NSDragPboard];

  if (anImage == nil)
    return NO;

  [pboard declareTypes: [NSArray arrayWithObject: NSFilesPromisePboardType] 
	  owner: sourceObject];
  // FIXME: Not sure if this is correct.
  if (![pboard setPropertyList: typeArray
	       forType: NSFilesPromisePboardType])
    return NO;

  [self dragImage: anImage
	at: aRect.origin
	offset: NSMakeSize(0, 0)
	event: theEvent
	pasteboard: pboard
	source: sourceObject
	slideBack: slideBack];
  return YES;
}

/*
 * Printing
 */
- (void) fax: (id)sender
{
  NSPrintInfo *aPrintInfo = [NSPrintInfo sharedPrintInfo];

  [aPrintInfo setJobDisposition: NSPrintFaxJob];
  [[NSPrintOperation printOperationWithView: self
		     printInfo: aPrintInfo] runOperation];
}

- (void) print: (id)sender
{
  [[NSPrintOperation printOperationWithView: self] runOperation];
}

- (NSData*) dataWithEPSInsideRect: (NSRect)aRect
{
  NSMutableData *data = [NSMutableData data];

  if ([[NSPrintOperation EPSOperationWithView: self
			 insideRect: aRect
			 toData: data] runOperation])
    {
      return data;
    }
  else
    {
      return nil;
    }
}

- (void) writeEPSInsideRect: (NSRect)rect
	       toPasteboard: (NSPasteboard*)pasteboard
{
  NSData *data = [self dataWithEPSInsideRect: rect];

  if (data != nil)
    [pasteboard setData: data
		forType: NSPostScriptPboardType];
}

- (NSData *) dataWithPDFInsideRect: (NSRect)aRect
{
  NSMutableData *data = [NSMutableData data];
  
  if ([[NSPrintOperation PDFOperationWithView: self
			 insideRect: aRect
			 toData: data] runOperation])
    {
      return data;
    }
  else
    {
      return nil;
    }
}

- (void) writePDFInsideRect: (NSRect)aRect 
	       toPasteboard: (NSPasteboard *)pboard
{
  NSData *data = [self dataWithPDFInsideRect: aRect];

  if (data != nil)
    [pboard setData: data
	    forType: NSPDFPboardType];
}

- (NSString *) printJobTitle
{
  id doc;
  NSString *title;
  doc = [[NSDocumentController sharedDocumentController] documentForWindow:
							   [self window]];
  if (doc)
    title = [doc displayName];
  else
    title = [[self window] title];
  return title;
}

/*
 * Pagination
 */
- (void) adjustPageHeightNew: (CGFloat*)newBottom
			 top: (CGFloat)oldTop
		      bottom: (CGFloat)oldBottom
		       limit: (CGFloat)bottomLimit
{
  CGFloat bottom = oldBottom;

  if (_rFlags.has_subviews)
    {
      id e, o;

      e = [_sub_views objectEnumerator];
      while ((o = [e nextObject]) != nil)
	{
          // FIXME: We have to convert this values for the subclass

	  CGFloat oTop, oBottom, oLimit;
	  /* Don't ask me why, but gcc-2.91.66 crashes if we use
	     NSMakePoint in the following expressions.  We avoid this
	     compiler internal bug by using an auxiliary aPoint
	     variable, and setting it manually to the NSPoints we
	     need.  */
	  {
	    NSPoint aPoint = {0, oldTop};
	    oTop = ([self convertPoint: aPoint  toView: o]).y;
	  }
	  
	  {
	    NSPoint aPoint = {0, bottom};
	    oBottom = ([self convertPoint: aPoint  toView: o]).y;
	  }

	  {
	    NSPoint aPoint = {0, bottomLimit};
	    oLimit = ([self convertPoint: aPoint  toView: o]).y;
	  }

	  [o adjustPageHeightNew: &oBottom
	     top: oTop
	     bottom: oBottom
	     limit: oLimit];

	  {
	    NSPoint aPoint = {0, oBottom};
	    bottom = ([self convertPoint: aPoint  fromView: o]).y; 
	  }	    
	}
    }

  *newBottom = bottom;
}

- (void) adjustPageWidthNew: (CGFloat*)newRight
		       left: (CGFloat)oldLeft
		      right: (CGFloat)oldRight
		      limit: (CGFloat)rightLimit
{
  CGFloat right = oldRight;

  if (_rFlags.has_subviews)
    {
      id e, o;

      e = [_sub_views objectEnumerator];
      while ((o = [e nextObject]) != nil)
	{
          // FIXME: We have to convert this values for the subclass

	  /* See comments in adjustPageHeightNew:top:bottom:limit:
	     about why code is structured in this funny way.  */
	  CGFloat oLeft, oRight, oLimit;
	  /* Don't ask me why, but gcc-2.91.66 crashes if we use
	     NSMakePoint in the following expressions.  We avoid this
	     compiler internal bug by using an auxiliary aPoint
	     variable, and setting it manually to the NSPoints we
	     need.  */
	  {
	    NSPoint aPoint = {oldLeft, 0};
	    oLeft = ([self convertPoint: aPoint  toView: o]).x;
	  }
	  
	  {
	    NSPoint aPoint = {right, 0};
	    oRight = ([self convertPoint: aPoint  toView: o]).x;
	  }

	  {
	    NSPoint aPoint = {rightLimit, 0};
	    oLimit = ([self convertPoint: aPoint  toView: o]).x;
	  }

	  [o adjustPageHeightNew: &oRight
	     top: oLeft
	     bottom: oRight
	     limit: oLimit];

	  {
	    NSPoint aPoint = {oRight, 0};
	    right = ([self convertPoint: aPoint  fromView: o]).x; 
	  }	    
	}
    }

  *newRight = right;
}

- (CGFloat) heightAdjustLimit
{
  return 0.0;
}

- (BOOL) knowsPagesFirst: (int*)firstPageNum last: (int*)lastPageNum
{
  return NO;
}

- (BOOL) knowsPageRange: (NSRange*)range
{
  return NO;
}

- (NSPoint) locationOfPrintRect: (NSRect)aRect
{
  int pages;
  NSPoint location;
  NSRect bounds;
  NSMutableDictionary *dict;
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSPrintInfo *printInfo = [printOp printInfo];
  dict = [printInfo dictionary];

  pages = [[dict objectForKey: @"NSPrintTotalPages"] intValue];
  if ([dict objectForKey: @"NSPrintPaperBounds"])
    bounds = [[dict objectForKey: @"NSPrintPaperBounds"] rectValue];
  else
    bounds = aRect;
  location = NSMakePoint(0, NSHeight(bounds)-NSHeight(aRect));
  /* FIXME:  I can't figure out how the location for a multi-page document
     is computed. Just ignore centering? */
  if (pages == 1)
    {
      if ([printInfo isHorizontallyCentered])
        location.x = (NSWidth(bounds) - NSWidth(aRect))/2;
      if ([printInfo isVerticallyCentered])
        location.y = (NSHeight(bounds) - NSHeight(aRect))/2;
    }

  return location;
}

- (NSRect) rectForPage: (NSInteger)page
{
  return NSZeroRect;
}

- (CGFloat) widthAdjustLimit
{
  return 0.0;
}

/*
 * Writing Conforming PostScript
 */
- (void) beginPage: (int)ordinalNum
             label: (NSString*)aString
              bBox: (NSRect)pageRect
             fonts: (NSString*)fontNames
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt  beginPage: ordinalNum
         label: aString
         bBox: pageRect
         fonts: fontNames];
}

- (void) beginPageSetupRect: (NSRect)aRect placement: (NSPoint)location
{
  [self beginPageInRect: aRect atPlacement: location];
}

- (void) beginPrologueBBox: (NSRect)boundingBox
              creationDate: (NSString*)dateCreated
                 createdBy: (NSString*)anApplication
                     fonts: (NSString*)fontNames
                   forWhom: (NSString*)user
                     pages: (int)numPages
                     title: (NSString*)aTitle
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt beginPrologueBBox: boundingBox
	      creationDate: dateCreated
        createdBy: anApplication
        fonts: fontNames
        forWhom: user
        pages: numPages
        title: aTitle];
}

- (void) addToPageSetup
{
}

- (void) beginSetup
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt beginSetup];
}

- (void) beginTrailer
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt beginTrailer];
}

- (void) drawPageBorderWithSize: (NSSize)borderSize
{
}

- (void) drawSheetBorderWithSize: (NSSize)borderSize
{
}

- (void) endHeaderComments
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt endHeaderComments];
}

- (void) endPrologue
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt endPrologue];
}

- (void) endSetup
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt endSetup];
}

- (void) endPageSetup
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt endPageSetup];
}

- (void) endPage
{
  int nup;
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];
  NSDictionary *dict = [[printOp printInfo] dictionary];

  // Balance gsave in beginPageInRect:
  DPSgrestore(ctxt);

  nup = [[dict objectForKey: NSPrintPagesPerSheet] intValue];
  if (nup > 1)
    {
      DPSPrintf(ctxt, "__GSpagesaveobject restore\n\n");
    }

  // [self unlockFocus];
}

- (void) endTrailer
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt endTrailer];
}

- (NSAttributedString *) pageFooter
{
  return [[[NSAttributedString alloc] initWithString:
		  [NSString stringWithFormat:@"Page %d", 
			    [[NSPrintOperation currentOperation] currentPage]]] 
	     autorelease];
}

- (NSAttributedString *) pageHeader
{
  return [[[NSAttributedString alloc] initWithString: 
		  [NSString stringWithFormat:@"%@ %@", [self printJobTitle], 
			    [[NSCalendarDate calendarDate] description]]] autorelease];
}


/** 
    Writes header and job information for the PostScript document. This
    includes at a minimum, PostScript header information. It may also 
    include job setup information if the output is intended for a printer
    (i.e. not an EPS file). Most of the information for writing the
    header comes from the NSPrintOperation and NSPrintInfo objects 
    associated with the current print operation.

    There isn't normally anything that the program needs to override
    at the beginning of a document, although if there is additional
    setup that needs to be done, you can override the NSView's methods
    endHeaderComments, endPrologue, beginSetup, and/or endSetup.

    This method calls the above methods in the listed order before
    or after writing the required information. For an EPS operation, the
    beginSetup and endSetup methods aren't used.  */
- (void)beginDocument
{
  int first, last, pages, nup;
  NSRect bbox;
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];
  NSDictionary *dict = [[printOp printInfo] dictionary];

  if (printOp == nil)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"beginDocument called without a current print op"];
    }
  /* Inform ourselves and subviews that we're printing so we adjust
     the PostScript accordingly. Perhaps this could be in the thread
     dictionary, but that's probably overkill and slow */
  viewIsPrinting = self;

  /* Get pagination information */
  nup = [[dict objectForKey: NSPrintPagesPerSheet] intValue];
  bbox = NSZeroRect;
  if ([dict objectForKey: @"NSPrintSheetBounds"])
    bbox = [[dict objectForKey: @"NSPrintSheetBounds"] rectValue];
  first = [[dict objectForKey: NSPrintFirstPage] intValue];
  last  = [[dict objectForKey: NSPrintLastPage] intValue];
  pages = last - first + 1;
  if (nup > 1)
    pages = ceil((float)pages / nup);

  /* Begin document structure */
  [self beginPrologueBBox: bbox
	     creationDate: [[NSCalendarDate calendarDate] description]
	        createdBy: [[NSProcessInfo processInfo] processName]
		    fonts: nil
	          forWhom: NSUserName()
		    pages: pages
	            title: [self printJobTitle]];
  [self endHeaderComments];

  [ctxt printerProlog];
  [self endPrologue];
  if ([printOp isEPSOperation] == NO)
    {
      [self beginSetup];
      // Setup goes here !
      [self endSetup];
    }

  [ctxt resetUsedFonts];
  /* Make sure we set the visible rect so everything is printed. */
  [self _invalidateCoordinates];
  _visibleRect = _bounds;
}

- (void) beginPageInRect: (NSRect)aRect 
             atPlacement: (NSPoint)location
{
  int nup;
  NSRect bounds;
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];
  NSDictionary *dict = [[printOp printInfo] dictionary];

  if (NSIsEmptyRect(aRect))
    {
      if ([dict objectForKey: @"NSPrintPaperBounds"])
        {
          bounds = [[dict objectForKey: @"NSPrintPaperBounds"] rectValue];
        }
      else
        {
          // FIXME: What should we use here?
          bounds = aRect;
        }
    }
  else
    {
      bounds = aRect;
    }

  nup = [[dict objectForKey: NSPrintPagesPerSheet] intValue];
  if (nup > 1)
    {
      int page;
      float xoff, yoff;
      float scale;

      DPSPrintf(ctxt, "/__GSpagesaveobject save def\n");

      scale = [[dict objectForKey: @"NSNupScale"] floatValue];
      page = [printOp currentPage] 
          - [[dict objectForKey: NSPrintFirstPage] intValue];
      page = page % nup;
      if (nup == 2)
        xoff = page;
      else
        xoff = (page % (nup/2));
      xoff *= NSWidth(bounds) * scale;
      if (nup == 2)
        yoff = 0;
      else
        yoff = (int)((nup-page-1) / (nup/2));
      yoff *= NSHeight(bounds) * scale;
      DPStranslate(ctxt, xoff, yoff);
      DPSgsave(ctxt);
      DPSscale(ctxt, scale, scale);
    }
  else
    {
      DPSgsave(ctxt);
    }

  /* Translate to placement */
  if (location.x != 0 || location.y != 0)
    {
      DPStranslate(ctxt, location.x, location.y);
    }
}

- (void) _endSheet
{
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];

  [ctxt endSheet];
}

- (void) endDocument
{
  int first, last, current, pages;
  NSPrintOperation *printOp = [NSPrintOperation currentOperation];
  NSGraphicsContext *ctxt = [printOp context];
  NSDictionary *dict = [[printOp printInfo] dictionary];

  first = [[dict objectForKey: NSPrintFirstPage] intValue];
  last  = [[dict objectForKey: NSPrintLastPage] intValue];
  pages = last - first + 1;
  [self beginTrailer];

  if (pages == 0)
    {
      int nup = [[dict objectForKey: NSPrintPagesPerSheet] intValue];
      current = [printOp currentPage];
      pages = current - first; // Current is 1 more than the last page
      if (nup > 1)
        pages = ceil((float)pages / nup);
    }
  else
    {
      // Already reported at start of document
      pages = 0;
    }
  [ctxt endDocumentPages: pages documentFonts: [ctxt usedFonts]];

  [self endTrailer];
  [self _invalidateCoordinates];
  viewIsPrinting = nil;
}

/* An exception occurred while printing. Clean up */
- (void) _cleanupPrinting
{
  [self _invalidateCoordinates];
  viewIsPrinting = nil;
}  

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      NSUInteger vFlags = 0;

      // encoding
      [aCoder encodeConditionalObject: [self nextKeyView] 
	      forKey: @"NSNextKeyView"];
      [aCoder encodeConditionalObject: [self previousKeyView] 
	      forKey: @"NSPreviousKeyView"];
      [aCoder encodeObject: _sub_views 
	      forKey: @"NSSubviews"];
      [aCoder encodeRect: _frame 
	      forKey: @"NSFrame"];

      // autosizing masks.
      vFlags = _autoresizingMask;

      // add the autoresize flag.
      if (_autoresizes_subviews)
        {
          vFlags |= 0x100;
        }

      // add the hidden flag
      if (_is_hidden)
        {
          vFlags |= 0x80000000;
        }
      
      [aCoder encodeInt: vFlags 
	      forKey: @"NSvFlags"];

      //
      // Don't attempt to archive the superview of a view which is the
      // content view for a window.
      //
      if (([[self window] contentView] != self) && _super_view != nil)
        {
          [aCoder encodeConditionalObject: _super_view forKey: @"NSSuperview"];
        }
    }
  else
    {
      NSDebugLLog(@"NSView", @"NSView: start encoding\n");
      [super encodeWithCoder: aCoder];

      [aCoder encodeRect: _frame];
      [aCoder encodeRect: _bounds];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_is_rotated_from_base];
      [aCoder encodeValueOfObjCType: @encode(BOOL)
	      at: &_is_rotated_or_scaled_from_base];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_post_frame_changes];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_autoresizes_subviews];
      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &_autoresizingMask];
      [aCoder encodeConditionalObject: [self nextKeyView]];
      [aCoder encodeConditionalObject: [self previousKeyView]];
      [aCoder encodeObject: _sub_views];
      NSDebugLLog(@"NSView", @"NSView: finish encoding\n");
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  NSEnumerator *e;
  NSView	*sub;
  NSArray	*subs;

  // decode the superclass...
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  // initialize these here, since they're needed in either case.
  // _frameMatrix = [NSAffineTransform new];    // Map fromsuperview to frame
  // _boundsMatrix = [NSAffineTransform new];   // Map from superview to bounds
  _matrixToWindow = [NSAffineTransform new];  // Map to window coordinates
  _matrixFromWindow = [NSAffineTransform new];// Map from window coordinates
 
  if ([aDecoder allowsKeyedCoding])
    {
      NSView *prevKeyView = nil;
      NSView *nextKeyView = nil;

      if ([aDecoder containsValueForKey: @"NSFrame"])
        {
          _frame = [aDecoder decodeRectForKey: @"NSFrame"];
        }
      else
        {
          _frame = NSZeroRect;
          if ([aDecoder containsValueForKey: @"NSFrameSize"])
            {
              _frame.size = [aDecoder decodeSizeForKey: @"NSFrameSize"];
            }
        }

      // Set bounds rectangle
      _bounds.origin = NSZeroPoint;
      _bounds.size = _frame.size;
      if ([aDecoder containsValueForKey: @"NSBounds"])
        {
          [self setBounds: [aDecoder decodeRectForKey: @"NSBounds"]];
        }
      
      _sub_views = [NSMutableArray new];
      _tracking_rects = [NSMutableArray new];
      _cursor_rects = [NSMutableArray new];
      
      _is_rotated_from_base = NO;
      _is_rotated_or_scaled_from_base = NO;
      _rFlags.needs_display = YES;
      _post_bounds_changes = YES;
      _post_frame_changes = YES;
      _autoresizes_subviews = YES;
      _autoresizingMask = NSViewNotSizable;
      _coordinates_valid = NO;
      /*
       * Note: don't zero _nextKeyView and _previousKeyView, as the key view
       * chain may already have been established by super's initWithCoder:
       *
       * _nextKeyView = 0;
       * _previousKeyView = 0;
       */
      
      // previous and next key views...
      prevKeyView = [aDecoder decodeObjectForKey: @"NSPreviousKeyView"];
      nextKeyView = [aDecoder decodeObjectForKey: @"NSNextKeyView"];
      if (nextKeyView != nil)
        {
          [self setNextKeyView: nextKeyView];
        }
      if (prevKeyView != nil)
        {
          [self setPreviousKeyView: prevKeyView];
        }
      if ([aDecoder containsValueForKey: @"NSvFlags"])
        {
          NSUInteger vFlags = [aDecoder decodeIntForKey: @"NSvFlags"];
	  
          // We are lucky here, Apple use the same constants
          // in the lower bits of the flags
          [self setAutoresizingMask: vFlags & 0x3F];
          [self setAutoresizesSubviews: ((vFlags & 0x100) == 0x100)];
          [self setHidden: ((vFlags & 0x80000000) == 0x80000000)];
        }

      // iterate over subviews and put them into the view...
      subs = [aDecoder decodeObjectForKey: @"NSSubviews"];
      e = [subs objectEnumerator];
      while ((sub = [e nextObject]) != nil)
	{
	  NSAssert([sub class] != [NSCustomView class],
		   NSInternalInconsistencyException);
	  NSAssert([sub window] == nil,
		   NSInternalInconsistencyException);
          NSAssert([sub superview] == nil,
                   NSInternalInconsistencyException);
	  [sub _viewWillMoveToWindow: _window];
	  [sub _viewWillMoveToSuperview: self];
	  [sub setNextResponder: self];
	  [_sub_views addObject: sub];
	  _rFlags.has_subviews = 1;
	  [sub resetCursorRects];
	  [sub setNeedsDisplay: YES];
	  [sub _viewDidMoveToWindow];
	  [sub viewDidMoveToSuperview];
	  [self didAddSubview: sub];
	}

      // the superview...
      //[aDecoder decodeObjectForKey: @"NSSuperview"];
    }
  else
    {
      NSRect	rect;
      
      NSDebugLLog(@"NSView", @"NSView: start decoding\n");

      _frame = [aDecoder decodeRect];
      
      _bounds.origin = NSZeroPoint;
      _bounds.size = _frame.size;
      
      rect = [aDecoder decodeRect];
      [self setBounds: rect];
      
      _sub_views = [NSMutableArray new];
      _tracking_rects = [NSMutableArray new];
      _cursor_rects = [NSMutableArray new];
      
      _super_view = nil;
      _window = nil;
      _rFlags.needs_display = YES;
      [aDecoder decodeValueOfObjCType: @encode(BOOL)
				   at: &_is_rotated_from_base];
      [aDecoder decodeValueOfObjCType: @encode(BOOL)
				   at: &_is_rotated_or_scaled_from_base];
      _post_bounds_changes = YES;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_post_frame_changes];
      [aDecoder decodeValueOfObjCType: @encode(BOOL)
				   at: &_autoresizes_subviews];
      [aDecoder decodeValueOfObjCType: @encode(NSUInteger)
				   at: &_autoresizingMask];
      _coordinates_valid = NO;
      [self setNextKeyView: [aDecoder decodeObject]];
      [[aDecoder decodeObject] setNextKeyView: self];

      [aDecoder decodeValueOfObjCType: @encode(id) at: &subs];
      NSDebugLLog(@"NSView", @"NSView: finish decoding\n");

      // iterate over subviews and put them into the view...
      e = [subs objectEnumerator];
      while ((sub = [e nextObject]) != nil)
	{
	  NSAssert([sub window] == nil,
	    NSInternalInconsistencyException);
	  NSAssert([sub superview] == nil,
	    NSInternalInconsistencyException);
	  [sub _viewWillMoveToWindow: _window];
	  [sub _viewWillMoveToSuperview: self];
	  [sub setNextResponder: self];
	  [_sub_views addObject: sub];
	  _rFlags.has_subviews = 1;
	  [sub resetCursorRects];
	  [sub setNeedsDisplay: YES];
	  [sub _viewDidMoveToWindow];
	  [sub viewDidMoveToSuperview];
	  [self didAddSubview: sub];
	}
      RELEASE(subs);
    }

  return self;
}

/*
 * Accessor methods
 */
- (void) setAutoresizesSubviews: (BOOL)flag
{
  _autoresizes_subviews = flag;
}

- (void) setAutoresizingMask: (NSUInteger)mask
{
  _autoresizingMask = mask;
}

/** Returns the window in which the receiver resides. */
- (NSWindow*) window
{
  return _window;
}

- (BOOL) autoresizesSubviews
{
  return _autoresizes_subviews;
}

- (NSUInteger) autoresizingMask
{
  return _autoresizingMask;
}

- (NSArray*) subviews
{
  /*
   * Return a mutable copy 'cos we know that a mutable copy of an array or
   * a mutable array does a shallow copy - which is what we want to give
   * away - we don't want people to mess with our actual subviews array.
   */
  return AUTORELEASE([_sub_views mutableCopyWithZone: NSDefaultMallocZone()]);
}

- (NSView*) superview
{
  return _super_view;
}

- (BOOL) shouldDrawColor
{
  return YES;
}

- (BOOL) isOpaque
{
  return NO;
}

- (BOOL) needsDisplay
{
  return _rFlags.needs_display;
}

- (NSInteger) tag
{
  return -1;
}

- (BOOL) isFlipped
{
  return NO;
}

- (NSRect) bounds
{
  return _bounds;
}

- (NSRect) frame
{
  return _frame;
}

- (CGFloat) boundsRotation
{
  if (_boundsMatrix != nil)
    {
      return [_boundsMatrix rotationAngle];
    }

  return 0.0;
}

- (CGFloat) frameRotation
{
  if (_frameMatrix != nil)
    {
      return [_frameMatrix rotationAngle];
    }

  return 0.0;
}

/** 
 * Returns whether the receiver posts NSViewFrameDidChangeNotification when 
 * its frame changed.
 *
 * Returns YES by default (as documented in Cocoa View Programming Guide). 
 */
- (BOOL) postsFrameChangedNotifications
{
  return _post_frame_changes;
}

/** 
 * Returns whether the receiver posts NSViewBoundsDidChangeNotification when 
 * its bound changed. 
 *
 * Returns YES by default (as documented in Cocoa View Programming Guide). 
 */
- (BOOL) postsBoundsChangedNotifications
{
  return _post_bounds_changes;
}


/**
 * <p>Returns the default menu to be used for instances of the 
 *    current class; if no menu has been set through setMenu:
 *    this default menu will be used.
 * </p>
 * <p>NSView's implementation returns nil. You should override
 *    this method if you want all instances of your custom view
 *    to use the same menu.
 * </p>
 */
+ (NSMenu *)defaultMenu
{
  return nil;
}

/**
 * <p>NSResponder's method, overriden by NSView.</p>
 * <p>If no menu has been set through the use of setMenu:, or 
 *    if a nil value has been set through setMenu:, then the 
 *    value returned by defaultMenu is used. Otherwise this
 *    method returns the menu set through NSResponder.
 * <p>
 * <p> see [NSResponder -menu], [NSResponder -setMenu:],
 *     [NSView +defaultMenu] and [NSView -menuForEvent:].
 * </p>
 */
- (NSMenu *)menu
{
  NSMenu *m = [super menu];
  if (m)
    {
      return m;
    }
  else
    {
      return [[self class] defaultMenu];
    }
}

/**
 * <p>Returns the menu that it appropriates for the given 
 *    event. NSView's implementation returns the default menu of
 *    the view.</p>
 * <p>This methods is intended to be overriden so that it can
 *    return a context-sensitive for appropriate mouse's events. (
 *    (although it seems it can be used for any kind of event)</p>
 * <p>This method is used by NSView's rightMouseDown: method, 
 *    and the returned NSMenu is displayed as a context menu</p> 
 * <p>Use of this method is discouraged in GNUstep as it breaks many
 *    user interface guidelines. At the very least, menu items that appear
 *    in a context sensitive menu should also always appear in a normal
 *    menu. Otherwise, users are faced with an inconsistant interface where
 *    the menu items they want are only available in certain (possibly
 *    unknown) cases, making it difficult for the user to understand how
 *    the application operates</p>
 * <p> see [NSResponder -menu], [NSResponder -setMenu:],
 *     [NSView +defaultMenu] and [NSView -menu].
 * </p>
 */
- (NSMenu *)menuForEvent: (NSEvent *)theEvent
{
  return [self menu];
}

/*
 * Tool Tips
 */

- (NSToolTipTag) addToolTipRect: (NSRect)aRect 
			  owner: (id)anObject 
		       userData: (void *)data
{
  GSToolTips	*tt = [GSToolTips tipsForView: self];

  _rFlags.has_tooltips = 1;
  return [tt addToolTipRect: aRect owner: anObject userData: data];
}

- (void) removeAllToolTips
{
  if (_rFlags.has_tooltips == 1)
    {
      GSToolTips	*tt = [GSToolTips tipsForView: self];

      [tt removeAllToolTips];
    }
}

- (void) removeToolTip: (NSToolTipTag)tag
{
  if (_rFlags.has_tooltips == 1)
    {
      GSToolTips	*tt = [GSToolTips tipsForView: self];

      [tt removeToolTip: tag];
    }
}

- (void) setToolTip: (NSString *)string
{
  if (_rFlags.has_tooltips == 1 || [string length] > 0)
    {
      GSToolTips	*tt = [GSToolTips tipsForView: self];

      _rFlags.has_tooltips = 1;
      [tt setToolTip: string];
    }
}

- (NSString *) toolTip
{
  if (_rFlags.has_tooltips == 1)
    {
      GSToolTips	*tt = [GSToolTips tipsForView: self];

      return [tt toolTip];
    }
  return nil;
}

- (void) rightMouseDown: (NSEvent *) theEvent
{
  NSMenu *m;
  m = [self menuForEvent: theEvent];
  if (m)
    {
      [NSMenu popUpContextMenu: m
	      withEvent: theEvent
	      forView: self];
    }
  else
    {
      [super rightMouseDown: theEvent];
    }
}

- (BOOL) shouldBeTreatedAsInkEvent: (NSEvent *)theEvent
{
  return YES;
}

- (void) bind: (NSString *)binding
     toObject: (id)anObject
  withKeyPath: (NSString *)keyPath
      options: (NSDictionary *)options
{
  if ([binding hasPrefix: NSHiddenBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueOrBinding alloc] initWithBinding: NSHiddenBinding 
                                   withName: binding 
                                   toObject: anObject
                                   withKeyPath: keyPath
                                   options: options
                                   fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else
    {
      [super bind: binding
             toObject: anObject
             withKeyPath: keyPath
             options: options];
    }
}

- (NSViewLayerContentsPlacement) layerContentsPlacement
{
  // FIXME (when views have CALayer support)
  return NSViewLayerContentsPlacementScaleAxesIndependently;
}

- (void) setLayerContentsPlacement: (NSViewLayerContentsPlacement)placement
{
  // FIXME (when views have CALayer support)
  static BOOL logged = NO;
  if (!logged)
    {
      NSLog(@"warning: stub no-op implementation of -[NSView setLayerContentsPlacement:]");
      logged = YES;
    }
}

- (NSViewLayerContentsRedrawPolicy) layerContentsRedrawPolicy
{
  // FIXME (when views have CALayer support)
  return NSViewLayerContentsRedrawNever;
}

- (void) setLayerContentsRedrawPolicy: (NSViewLayerContentsRedrawPolicy) pol
{
  // FIXME (when views have CALayer support)
  static BOOL logged = NO;
  if (!logged)
    {
      NSLog(@"warning: stub no-op implementation of -[NSView setLayerContentsRedrawPolicy:]");
      logged = YES;
    }
}

- (NSUserInterfaceLayoutDirection) userInterfaceLayoutDirection
{
  // FIXME
  return NSUserInterfaceLayoutDirectionLeftToRight;
}

- (void) setUserInterfaceLayoutDirection: (NSUserInterfaceLayoutDirection)dir
{
  // FIXME: implement this
  return;
}

@end

@implementation NSView (__NSViewPrivateMethods__)

/*
 * This method inserts a view at a given place in the view hierarchy.
 */
- (void) _insertSubview: (NSView *)sv atIndex: (NSUInteger)idx
{
  [sv _viewWillMoveToWindow: _window];
  [sv _viewWillMoveToSuperview: self];
  [sv setNextResponder: self];
  [_sub_views insertObject: sv atIndex: idx];
  _rFlags.has_subviews = 1;
  [sv resetCursorRects];
  [sv setNeedsDisplay: YES];
  [sv _viewDidMoveToWindow];
  [sv viewDidMoveToSuperview];
  [self didAddSubview: sv];
}

@end


@implementation NSView(KeyViewLoop)

static NSComparisonResult
cmpFrame(id view1, id view2, void *context)
{
  BOOL flippedSuperView = [(NSView *)context isFlipped];
  NSRect frame1 = [view1 frame];
  NSRect frame2 = [view2 frame];

  if (NSMinY(frame1) < NSMinY(frame2))
    return flippedSuperView ? NSOrderedAscending : NSOrderedDescending;
  if (NSMaxY(frame1) > NSMaxY(frame2))
    return flippedSuperView ? NSOrderedDescending : NSOrderedAscending;

  // FIXME Should use NSMaxX in a Hebrew or Arabic locale
  if (NSMinX(frame1) < NSMinX(frame2))
    return NSOrderedAscending;
  if (NSMinX(frame1) > NSMinX(frame2))
    return NSOrderedDescending;
  return NSOrderedSame;
}

- (void) _setUpKeyViewLoopWithNextKeyView: (NSView *)nextKeyView
{
  if (_rFlags.has_subviews)
    {
      [self _recursiveSetUpKeyViewLoopWithNextKeyView: nextKeyView];
    }
  else
    {
      [self setNextKeyView: nextKeyView];
    }
}

- (void) _recursiveSetUpKeyViewLoopWithNextKeyView: (NSView *)nextKeyView
{
  NSArray *sortedViews;
  NSView *aView;
  NSEnumerator *e;

  sortedViews = [_sub_views sortedArrayUsingFunction: cmpFrame context: self];
  e = [sortedViews reverseObjectEnumerator];
  while ((aView = [e nextObject]) != nil)
    {
      [aView _setUpKeyViewLoopWithNextKeyView: nextKeyView];
      nextKeyView = aView;
    }
  [self setNextKeyView: nextKeyView];
}

@end
