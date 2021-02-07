/** <title>NSClipView</title>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: July 1997
   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
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
#import <Foundation/NSNotification.h>
#import <Foundation/NSException.h>

#import "AppKit/NSClipView.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/PSOperators.h"

#import <GNUstepGUI/GSNibLoading.h>
#import "GSGuiPrivate.h"

#include <math.h>

@interface NSClipView (Private)
- (void) _scrollToPoint: (NSPoint)aPoint;
@end

/*
 * Return the biggest integral (in device space) rect contained in rect. 
 * Conversion to/from device space is done using view.
 *
 */
static inline NSRect integralRect (NSRect rect, NSView *view)
{
  NSRect output;
  int rounded;
  
  output = [view convertRect: rect  toView: nil];

  rounded = (int)(output.origin.x);
  if ((CGFloat)rounded != output.origin.x)
    {
      output.origin.x = rounded + 1;
    }

  rounded = (int)(output.origin.y);
  if ((CGFloat)rounded != output.origin.y)
    {
      output.origin.y = rounded + 1;
    }
  
  rounded = (int)(NSMaxX (output));
  if ((CGFloat)rounded != NSMaxX (output))
    {
      output.size.width = rounded - output.origin.x;
    }
  
  rounded = (int)(NSMaxY (output));
  if ((CGFloat)rounded != NSMaxY (output))
    {
      output.size.height = rounded - output.origin.y;
    }

  return [view convertRect: output  fromView: nil];
}


/* Note that the ivar _documentView is really just a convienience
   variable. The actual document view is stored in NSClipView's
   subview array. Deallocation, coding, etc of the view is then
   handled by NSView
*/
@implementation NSClipView

- (id) initWithFrame: (NSRect)frameRect
{
  self = [super initWithFrame:frameRect];
  if (self)
    {
      [self setAutoresizesSubviews: YES];
      [self setBackgroundColor: [NSColor controlBackgroundColor]];
      _copiesOnScroll = YES;
      _drawsBackground = YES;
    }
  return self;
}

- (void) dealloc
{
  [self setDocumentView: nil];
  RELEASE(_cursor);
  RELEASE(_backgroundColor);

  [super dealloc];
}


/**<p>Sets aView the NSClipView's document view to <var>aView</var>
   </p>
   <p>See Also: -documentView</p>
 */
- (void) setDocumentView: (NSView*)aView
{
  NSNotificationCenter	*nc;
  NSView		*nextKV;

  if (_documentView == aView)
    {
      return;
    }
  
  nc = [NSNotificationCenter defaultCenter];
  if (_documentView)
    {
      nextKV = [_documentView nextKeyView];
      if ([nextKV isDescendantOf: _documentView])
	{
	  nextKV = nil;
	}

      [nc removeObserver: self
	  name: NSViewFrameDidChangeNotification
	  object: _documentView];
      [nc removeObserver: self
	  name: NSViewBoundsDidChangeNotification
	  object: _documentView];

      /* if our documentView was a tableview, unregister its
       * observers
       */
      if ([_documentView isKindOfClass: [NSTableView class]])
	{
	  [nc removeObserver: _documentView 
	      name: NSViewFrameDidChangeNotification 
	      object: self];
	}
      [_documentView removeFromSuperview];
    }
  else
    {
      nextKV = [self nextKeyView];
    }

  /* Don't retain this since it's stored in our subviews. */
  _documentView = aView;

  /* Update the view hierarchy coordinates if -isFlipped has changed.
     Call this before doing anything else! */
  [self _invalidateCoordinates];

  if (_documentView)
    {
      NSRect df;

      [self addSubview: _documentView];

      df = [_documentView frame];
      [self setBoundsOrigin: df.origin];

      /* Register for notifications sent by the document view */
      [_documentView setPostsFrameChangedNotifications: YES];
      [_documentView setPostsBoundsChangedNotifications: YES];

      [nc addObserver: self
	     selector: @selector(viewFrameChanged:)
		 name: NSViewFrameDidChangeNotification
	       object: _documentView];
      [nc addObserver: self
	     selector: @selector(viewBoundsChanged:)
		 name: NSViewBoundsDidChangeNotification
	       object: _documentView];

      /*
       *  if our document view is a tableview, let it know
       *  when we resize
       */
      if ([_documentView isKindOfClass: [NSTableView class]])
	{
	  [self setPostsFrameChangedNotifications: YES];
	  [nc addObserver: _documentView
	         selector: @selector(superviewFrameChanged:)
	             name: NSViewFrameDidChangeNotification
	           object: self];
	}

      [self setNextKeyView: _documentView];
      if (![_documentView nextKeyView])
	[_documentView setNextKeyView: nextKV];
    }
  else
    [self setNextKeyView: nextKV];

  [_super_view reflectScrolledClipView: self];
}

- (void) resetCursorRects
{
  [self addCursorRect: _bounds cursor: _cursor];
}

- (void) scrollToPoint: (NSPoint)aPoint
{
  [self setBoundsOrigin: [self constrainScrollPoint: aPoint]];
  [self resetCursorRects];
}

- (void) setBounds: (NSRect)b
{
  [super setBounds: b];
  [self setNeedsDisplay: YES];
  [_super_view reflectScrolledClipView: self];
}

- (void) setBoundsSize: (NSSize)aSize
{
  [super setBoundsSize: aSize];
  [self setNeedsDisplay: YES];
  [_super_view reflectScrolledClipView: self];
}

- (void) setBoundsOrigin: (NSPoint)aPoint
{
  NSRect originalBounds = _bounds;
  NSRect newBounds = originalBounds;
  NSRect intersection;

  newBounds.origin = aPoint;

  if (NSEqualPoints(originalBounds.origin, newBounds.origin))
    {
      return;
    }

  if (_documentView == nil)
    {
      return;
    }
      
  if (_copiesOnScroll && _window && [_window gState])
    {
      /* Copy the portion of the view that is common before and after
         scrolling.  Then, document view needs to redraw the remaining
         areas. */

      /* Common part - which is a first approx of what we could
         copy... */
      intersection = NSIntersectionRect (originalBounds, newBounds);

      /* but we must make sure we only copy from visible rect - we
         can't copy bits which have been clipped (ie discarded) */
      intersection = NSIntersectionRect (intersection, [self visibleRect]);

      /* Copying is done in device space so we only can copy by
         integral rects in device space - adjust our copy rect */
      intersection = integralRect (intersection, self);

      /* At this point, intersection is the rectangle containing the
         image we can recycle from the old to the new situation.  We
         must not make any assumption on its position/size, because it
         has been intersected with visible rect, which is an arbitrary
         rectangle as far as we know. */
      if (NSEqualRects (intersection, NSZeroRect))
        {
          // no recyclable part -- docview should redraw everything
          // from scratch
          [super setBoundsOrigin: newBounds.origin];
          [_documentView setNeedsDisplayInRect: 
                             [self documentVisibleRect]];
        }
      else
        {
          /* It is assumed these dx and dy will be integer in device
             space because they are the difference of the bounds
             origins, both of which should be integers in device space
             because of the code at the end of
             constrainScrollPoint:. */
          CGFloat dx = newBounds.origin.x - originalBounds.origin.x;
          CGFloat dy = newBounds.origin.y - originalBounds.origin.y;
          NSRect redrawRect;
                    
          /* Copy the intersection to the new position */
          [self scrollRect: intersection by: NSMakeSize(-dx, -dy)];

          /* Change coordinate system to the new one */
          [super setBoundsOrigin: newBounds.origin];
        
          /* Get the rectangle representing intersection in the new
             bounds (mainly to keep code readable) */
          intersection.origin.x -= dx;
          intersection.origin.y -= dy;
          // intersection.size is the same
          
          /* Now mark everything which is outside intersection as
             needing to be redrawn by hand.  NB: During simple usage -
             scrolling in a single direction (left/rigth/up/down) -
             and a normal visible rect, only one of the following
             rects will be non-empty. */
          
          /* To the left of intersection */
          redrawRect = NSMakeRect(NSMinX(_bounds), _bounds.origin.y,
                                  NSMinX(intersection) - NSMinX(_bounds),
                                  _bounds.size.height);
          if (NSIsEmptyRect(redrawRect) == NO)
            {
              [_documentView setNeedsDisplayInRect: 
                                 [self convertRect: redrawRect 
                                       toView: _documentView]];
            }
          
          /* Right */
          redrawRect = NSMakeRect(NSMaxX(intersection), _bounds.origin.y,
                                  NSMaxX(_bounds) - NSMaxX(intersection),
                                  _bounds.size.height);
          if (NSIsEmptyRect(redrawRect) == NO)
            {
              [_documentView setNeedsDisplayInRect: 
                                 [self convertRect: redrawRect 
                                       toView: _documentView]];
            }
          
          /* Up (or Down according to whether it's flipped or not) */
          redrawRect = NSMakeRect(_bounds.origin.x, NSMinY(_bounds),
                                  _bounds.size.width, 
                                  NSMinY(intersection) - NSMinY(_bounds));
          if (NSIsEmptyRect(redrawRect) == NO)
            {
              [_documentView setNeedsDisplayInRect: 
                                 [self convertRect: redrawRect 
                                       toView: _documentView]];
            }
          
          /* Down (or Up) */
          redrawRect = NSMakeRect(_bounds.origin.x, NSMaxY(intersection),
                                  _bounds.size.width, 
                                  NSMaxY(_bounds) - NSMaxY(intersection));
          if (NSIsEmptyRect(redrawRect) == NO)
            {
              [_documentView setNeedsDisplayInRect: 
                                 [self convertRect: redrawRect 
                                       toView: _documentView]];
            }
        }
    }
  else
    {
      // dont copy anything -- docview draws it all
      [super setBoundsOrigin: newBounds.origin];
      [_documentView setNeedsDisplayInRect: [self documentVisibleRect]];
    }

  /* ?? TODO: Understand the following code - and add explanatory comment */
  /*if ([NSView focusView] == _documentView)
    {
      PStranslate (NSMinX (originalBounds) - aPoint.x, 
		   NSMinY (originalBounds) - aPoint.y);
    }*/
  
  [_super_view reflectScrolledClipView: self];
}

/**
 *<p></p>
 */
- (NSPoint) constrainScrollPoint: (NSPoint)proposedNewOrigin
{
  NSRect	documentFrame;
  NSPoint	new = proposedNewOrigin;

  if (_documentView == nil)
    {
      return _bounds.origin;
    }
  
  documentFrame = [_documentView frame];
  if (documentFrame.size.width <= _bounds.size.width)
    {
      new.x = documentFrame.origin.x;
    }
  else if (proposedNewOrigin.x <= documentFrame.origin.x)
    {
      new.x = documentFrame.origin.x;
    }
  else if (proposedNewOrigin.x + _bounds.size.width >= NSMaxX(documentFrame))
    {
      new.x = NSMaxX(documentFrame) - _bounds.size.width;
    }

  if (documentFrame.size.height <= _bounds.size.height)
    {
      new.y = documentFrame.origin.y;
    }
  else if (proposedNewOrigin.y <= documentFrame.origin.y)
    {
      new.y = documentFrame.origin.y;
    }
  else if (proposedNewOrigin.y + _bounds.size.height >= NSMaxY(documentFrame))
    {
      new.y = NSMaxY(documentFrame) - _bounds.size.height;
    }

  /* Make it an integer coordinate in device space - this is to make
     sure that when the coordinates are changed and we need to copy to
     do the scrolling, the difference is an integer and so we can copy
     the image translating it by an integer in device space - and not
     by a float. */
  
  new = [self convertPoint: new  toView: nil];
  new.x = GSRoundTowardsInfinity(new.x);
  new.y = GSRoundTowardsInfinity(new.y);
  new = [self convertPoint: new  fromView: nil];
  return new;
}

/**<p>Returns the document rectangle.</p>
   <p>See Also: -documentVisibleRect </p>
 */
- (NSRect) documentRect
{
  NSRect documentFrame;
  NSRect clipViewBounds;
  NSRect rect;

  if (_documentView == nil)
    {
      return _bounds;
    }
  
  documentFrame = [_documentView frame];
  clipViewBounds = _bounds;
  rect.origin = documentFrame.origin;
  rect.size.width = MAX(documentFrame.size.width, clipViewBounds.size.width);
  rect.size.height = MAX(documentFrame.size.height, clipViewBounds.size.height);

  return rect;
}

/**<p>Returns the document visible rectangle in the document views coordinate
 * system.
 * </p>
 * <p>See Also: -documentRect [NSView-convertRect:toView:]</p>
 */
- (NSRect) documentVisibleRect
{
  return [self convertRect: _bounds toView:_documentView];
}

- (void) drawRect: (NSRect)rect
{
  if (_drawsBackground)
    {
      [_backgroundColor set];
      NSRectFill(rect);
    }
}

/**<p>Scrolls in response to mouse-dragged events. </p>
 */
- (BOOL) autoscroll: (NSEvent*)theEvent
{
  NSPoint new;
  NSPoint delta;
  NSRect r;

  if (_documentView == nil)
    {
      return NO;
    }
  
  new = [_documentView convertPoint: [theEvent locationInWindow] 
		       fromView: nil];

  r = [self documentVisibleRect];

  if (new.x < NSMinX(r))
    delta.x = new.x - NSMinX(r);
  else if (new.x > NSMaxX(r))
    delta.x = new.x - NSMaxX(r);
  else
    delta.x = 0;

  if (new.y < NSMinY(r))
    delta.y = new.y - NSMinY(r);
  else if (new.y > NSMaxY(r))
    delta.y = new.y - NSMaxY(r);
  else
    delta.y = 0;

  new.x = _bounds.origin.x + delta.x;
  new.y = _bounds.origin.y + delta.y;

  new = [self constrainScrollPoint: new];
  if (NSEqualPoints(new, _bounds.origin))
    return NO;

  [self setBoundsOrigin: new];
  return YES;
}

- (void) viewBoundsChanged: (NSNotification*)aNotification
{
  [_super_view reflectScrolledClipView: self];
}

/**<p>Used when the document view frame notify its change.
   ( with NSViewFrameDidChangeNotification )</p>
 */
- (void) viewFrameChanged: (NSNotification*)aNotification
{
  [self _scrollToPoint: _bounds.origin];

  /* If document frame does not completely cover _bounds */
  if (NSContainsRect([_documentView frame], _bounds) == NO)
    {
      /*
       * fill the area not covered by documentView with background color
       */
      [self setNeedsDisplay: YES];
    }

  [_super_view reflectScrolledClipView: self];
}

- (void) scaleUnitSquareToSize: (NSSize)newUnitSize
{
  [super scaleUnitSquareToSize: newUnitSize];
  [_super_view reflectScrolledClipView: self];
}

- (void) setFrameSize: (NSSize)aSize
{
  [super setFrameSize: aSize];
  [self setBoundsOrigin: [self constrainScrollPoint: _bounds.origin]];
  [_super_view reflectScrolledClipView: self];
}

- (void) setFrameOrigin: (NSPoint)aPoint
{
  [super setFrameOrigin: aPoint];
  [self setBoundsOrigin: [self constrainScrollPoint: _bounds.origin]];
  [_super_view reflectScrolledClipView: self];
}

- (void) setFrame: (NSRect)rect
{
  [super setFrame: rect];
  [self setBoundsOrigin: [self constrainScrollPoint: _bounds.origin]];
  [_super_view reflectScrolledClipView: self];
}

- (void) translateOriginToPoint: (NSPoint)aPoint
{
  [super translateOriginToPoint: aPoint];
  [_super_view reflectScrolledClipView: self];
}

/**
 *<p>Returns the NSClipView's document view.</p>
 *<p>See Also: -setDocumentView: </p>
 */
- (id) documentView
{
  return _documentView;
}

/**
 */
- (void) setCopiesOnScroll: (BOOL)flag
{
  _copiesOnScroll = flag;
}

/**
 */
- (BOOL) copiesOnScroll
{
  return _copiesOnScroll;
}

/**<p>Sets the cursor for the document view to <var>aCursor</var></p>
 <p>See Also: -documentCursor</p>
 */
- (void) setDocumentCursor: (NSCursor*)aCursor
{
  ASSIGN(_cursor, aCursor);
}

/**<p>Returns the cursor of the document view</p>
   <p>See Also: -setDocumentCursor: </p>
*/
- (NSCursor*) documentCursor
{
  return _cursor;
}

/**<p>Returns the NSClipView's background color</p>
   <p>See Also: -setBackgroundColor:</p>
 */
- (NSColor*) backgroundColor
{
  return _backgroundColor;
}

/**<p>Sets the NSClipView's background color to <var>aColor</var> and marks
   self for display. Sets the opaque flag if needed ( to YES if the
   NSClipView does not draw its background, if the background color
   is nil or if the background color alpha component is less than 1.0 , NO
   otherwise) </p> 
   <p>See Also: -backgroundColor [NSView-isOpaque]</p>
 */
- (void) setBackgroundColor: (NSColor*)aColor
{
  if (![_backgroundColor isEqual: aColor])
    {
      ASSIGN (_backgroundColor, aColor);
  
      [self setNeedsDisplay: YES];
    
      if (_drawsBackground == NO || _backgroundColor == nil
	  || [_backgroundColor alphaComponent] < 1.0)
	{
	  _isOpaque = NO;
	}
      else
	{
	  _isOpaque = YES;
	}
    }
}

- (void) setDrawsBackground: (BOOL)flag
{
  if (_drawsBackground != flag)
    {
      _drawsBackground = flag; 

      [self setNeedsDisplay: YES];

      if (_drawsBackground == NO || _backgroundColor == nil
	  || [_backgroundColor alphaComponent] < 1.0)
	{
	  _isOpaque = NO;
	}
      else
	{
	  _isOpaque = YES;
	}
    }
}

- (BOOL) drawsBackground
{
  return _drawsBackground;
}

- (BOOL) isOpaque
{
  return _isOpaque;
}

- (BOOL) isFlipped
{
  return (_documentView != nil) ? [_documentView isFlipped] : NO;
}

/* Disable rotation of clip view */
- (void) rotateByAngle: (CGFloat)angle
{
}

- (void) setBoundsRotation: (CGFloat)angle
{
}

- (void) setFrameRotation: (CGFloat)angle
{
}

/* Managing responder chain */
- (BOOL) acceptsFirstResponder
{
  if (_documentView == nil)
    {
      return NO;
    }
  else
    {
      return [_documentView acceptsFirstResponder];
    }
}

- (BOOL) becomeFirstResponder
{
  if (_documentView == nil)
    {
      return NO;
    }
  else
    {
      return [_window makeFirstResponder: _documentView];
    }
}

- (void) setNextKeyView: (NSView *)aView
{
  if (_documentView && aView != _documentView)
    [_documentView setNextKeyView: aView];
  else
    [super setNextKeyView: aView];
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      unsigned int flags = 0;
      [aCoder encodeObject: [self backgroundColor] forKey: @"NSBGColor"];
      [aCoder encodeObject: [self documentCursor] forKey: @"NSCursor"];
      [aCoder encodeObject: [self documentView] forKey: @"NSDocView"];
      
      if ([self drawsBackground])
	flags |= 4;
      if ([self copiesOnScroll] == NO)
	flags |= 2;

      [aCoder encodeInt: flags forKey: @"NScvFlags"];
    }
  else
    {
      [aCoder encodeObject: _backgroundColor];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_copiesOnScroll];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_drawsBackground];
      [aCoder encodeObject: _cursor];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (self == nil)
    {
      return nil;
    }

  if ([aDecoder allowsKeyedCoding])
    {
      [self setAutoresizesSubviews: YES];

      [self setBackgroundColor: [aDecoder decodeObjectForKey: @"NSBGColor"]];
      [self setDocumentCursor: [aDecoder decodeObjectForKey: @"NSCursor"]];

      if ([aDecoder containsValueForKey: @"NScvFlags"])
        {
	  int flags = [aDecoder decodeIntForKey: @"NScvFlags"];
	  BOOL drawsBackground = ((4 & flags) > 0);
	  BOOL noCopyOnScroll =  ((2 & flags) > 0); // ??? Not sure...

	  [self setCopiesOnScroll: (noCopyOnScroll == NO)];
	  [self setDrawsBackground: drawsBackground];
	}

      if ([[self subviews] count] > 0)
        {
          NSRect rect;
	  id document = [aDecoder decodeObjectForKey: @"NSDocView"];

	  NSAssert([document class] != [NSCustomView class],
		   NSInvalidArgumentException);
	  rect = [document frame];
	  rect.origin = NSZeroPoint;
	  [document setFrame: rect];
	  RETAIN(document); // prevent it from being released.
	  [document removeFromSuperview];
	  [self setDocumentView: document];
	  RELEASE(document);
	}
    }
  else
    {
      BOOL temp;
      
      [self setAutoresizesSubviews: YES];
      
      [self setBackgroundColor: [aDecoder decodeObject]];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_copiesOnScroll];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &temp];
      [self setDrawsBackground: temp];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_cursor];
      
      if ([[self subviews] count] > 0)
        {
          NSView *document = [[self subviews] objectAtIndex: 0];
	  RETAIN(document); // prevent it from being released.
	  [document removeFromSuperview];
	  [self setDocumentView: document];
	  RELEASE(document);
	}
    }
  return self;
}
@end

@implementation NSClipView (Private)

- (void) _scrollToPoint: (NSPoint)aPoint
{ 
  NSRect proposedBounds; 
  NSRect proposedVisibleRect; 
  NSRect newVisibleRect; 
  NSRect newBounds; 
  
  // give documentView a chance to adjust its visible rectangle 
  proposedBounds = _bounds; 
  proposedBounds.origin = aPoint; 
  proposedVisibleRect = [self convertRect: proposedBounds 
                              toView: _documentView]; 
  newVisibleRect = [_documentView adjustScroll: proposedVisibleRect]; 
  newBounds = [self convertRect: newVisibleRect fromView: _documentView]; 
  
  [self scrollToPoint: newBounds.origin]; 
}

@end

