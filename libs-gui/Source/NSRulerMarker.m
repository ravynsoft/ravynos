/** <title>NSRulerMarker</title>

   <abstract>Displays a symbol in a NSRulerView.</abstract>

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: Sept 2001
   
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

#import <Foundation/NSException.h>
#import "AppKit/NSRulerMarker.h"
#import "AppKit/NSRulerView.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSCursor.h"

@implementation NSRulerMarker

- (id)initWithRulerView:(NSRulerView *)aRulerView
         markerLocation:(CGFloat)location
		  image:(NSImage *)anImage
	    imageOrigin:(NSPoint)imageOrigin
{
  if (aRulerView == nil || anImage == nil)
    [NSException raise: NSInvalidArgumentException
		format: @"No view or image for ruler marker"];

  self = [super init];
  _isMovable = YES;
  _isRemovable = NO;
  _location = location;
  _imageOrigin = imageOrigin;
  _rulerView = aRulerView;
  ASSIGN(_image, anImage);

  return self;
}

- (void) dealloc
{
  RELEASE(_image);
  TEST_RELEASE(_representedObject);
  
  [super dealloc];
}

- (NSRulerView *)ruler
{
  return _rulerView;
}

- (void)setImage:(NSImage *)anImage
{
  ASSIGN(_image, anImage);
}

- (NSImage *)image
{
  return _image;
}

- (void)setImageOrigin:(NSPoint)aPoint
{
  _imageOrigin = aPoint;
}

- (NSPoint)imageOrigin
{
  return _imageOrigin;
}

- (NSRect)imageRectInRuler
{
  BOOL flipped = [_rulerView isFlipped];
  NSSize size = [_image size];
  NSPoint pointInRuler;
  
  pointInRuler = [[_rulerView clientView] 
                             convertPoint: NSMakePoint(_location, _location)
			     toView: _rulerView];

  if ([_rulerView orientation] == NSHorizontalRuler)
    {
      if (flipped)
        {
          return NSMakeRect(pointInRuler.x - _imageOrigin.x,
                            [_rulerView baselineLocation] - (size.height - _imageOrigin.y),
			    size.width, size.height);
        }
      else
        {
          return NSMakeRect(pointInRuler.x - _imageOrigin.x,
                            [_rulerView baselineLocation] - _imageOrigin.y,
			    size.width, size.height);
	}
    }
  else
    {
      if (flipped)
        {
          return NSMakeRect([_rulerView baselineLocation] - _imageOrigin.x,
                            pointInRuler.y - (size.height - _imageOrigin.y),
			    size.width, size.height);
        }
      else
        {
          return NSMakeRect([_rulerView baselineLocation] - _imageOrigin.x,
                            pointInRuler.y - _imageOrigin.y,
			    size.width, size.height);
        }
    }

  return NSZeroRect;
}

- (CGFloat)thicknessRequiredInRuler
{
  NSSize size = [_image size];

  if ([_rulerView orientation] == NSHorizontalRuler)
    {
      /* what is below imageOrigin is on the ruler area and is not counted */
      return size.height - _imageOrigin.y;
    }
  else
    {
      /* what is to the right of imageOrigin is on the ruler area */
      return _imageOrigin.x;
    }
}

- (void)setMovable:(BOOL)flag
{
  _isMovable = flag; 
}

- (BOOL)isMovable
{
  return _isMovable;
}

- (void)setRemovable:(BOOL)flag
{
  _isRemovable = flag;
}

- (BOOL)isRemovable
{
  return _isRemovable;
}

- (void)setMarkerLocation:(CGFloat)location
{
  _location = location;
}
 
- (CGFloat)markerLocation
{
  return _location;
}

- (void)setRepresentedObject:(id <NSCopying>)anObject
{
  ASSIGN(_representedObject, (id)anObject);
}

- (id <NSCopying>)representedObject
{
  return _representedObject;
}

- (void)drawRect:(NSRect)aRect
{
  NSPoint aPoint;
  NSRect rect;
  
  /* do not draw when dragging, all drawing is done in -trackMouse... */
  if (_isDragging)
    {
      return;
    }
    
  rect = [self imageRectInRuler];
  aPoint = rect.origin;
  if ([_rulerView isFlipped])
    {
      aPoint.y += rect.size.height;
    }
  rect = NSIntersectionRect(aRect, rect);
  if (NSIsEmptyRect(rect))
    return;

  [_image compositeToPoint: aPoint
	  operation: NSCompositeSourceOver];
}

- (BOOL)isDragging
{
  return _isDragging;
}

- (BOOL)trackMouse:(NSEvent *)theEvent adding:(BOOL)adding
{
  NSView *client = [_rulerView clientView];
  NSEvent *newEvent = nil;
  NSUInteger eventMask = NSLeftMouseDraggedMask | NSLeftMouseUpMask;
  BOOL isFar = NO;
  BOOL askedCanRemove = NO;
  BOOL canRemove = NO;
  BOOL flipped;
  NSPoint mousePositionInRuler;
  NSPoint mousePositionInClient;
  NSPoint mousePositionInWindow;
  NSPoint previousMousePositionInWindow;
  NSPoint mouseOffset;
  CGFloat location;
  NSRect drawRect;
  NSRect bounds = [_rulerView bounds];
  NSPoint drawPoint;
  BOOL returnValue = NO;
  NSWindow *window;

  if (adding)
    {
      if ([client respondsToSelector: @selector(rulerView:shouldAddMarker:)]
          && [client rulerView: _rulerView shouldAddMarker: self] == NO)
	return NO;
    }
  else if (!_isMovable && !_isRemovable)
    {
      return NO;
    }
  else if (_isMovable
           && [client respondsToSelector: @selector(rulerView:shouldMoveMarker:)]
           && [client rulerView: _rulerView shouldMoveMarker: self] == NO)
    {
      return NO;
    }

  // I'm being dragged!
  _isDragging = YES;
  RETAIN(self); // make sure we're not dealloc'd if removed from ruler
  [NSCursor hide];
  [_rulerView lockFocus];

  /* cache some values */
  flipped = [_rulerView isFlipped];
  window = [_rulerView window];
  
  mousePositionInWindow = [theEvent locationInWindow];
  previousMousePositionInWindow = mousePositionInWindow;
  mousePositionInRuler = [_rulerView convertPoint: mousePositionInWindow
                                         fromView: nil];

  /* calculate offset of mouse click relative to marker's location
     (to avoid marker bumping when dragging starts) */
  if (adding)
    {
      mouseOffset = NSMakePoint(0, 0);
    }
  else
    {
      NSPoint locationInRuler;
      locationInRuler = [client convertPoint: NSMakePoint(_location, _location)
			              toView: _rulerView];
      if ([_rulerView orientation] == NSHorizontalRuler)
        {
	  mouseOffset = NSMakePoint(locationInRuler.x - mousePositionInRuler.x, 0);
	}
      else
        {
	  mouseOffset = NSMakePoint(0, locationInRuler.y - mousePositionInRuler.y);
	  if (flipped)
	    {
	      mouseOffset.y *= -1;
	    }
	}
    }

  /* cache image without marker and draw marker in position */
  if (adding)
    {
      /* marker is not drawn yet; mouse is in marker's imageOrigin */
      drawRect.size = [_image size];
      drawPoint.x = mousePositionInRuler.x - _imageOrigin.x;
      if (!flipped)
        {
          drawPoint.y = mousePositionInRuler.y - _imageOrigin.y;
          drawRect.origin = drawPoint;
        }
      else
        {
          drawPoint.y = mousePositionInRuler.y + _imageOrigin.y;
          drawRect.origin.x = drawPoint.x;
          drawRect.origin.y = drawPoint.y - drawRect.size.height;
        }
    }
  else
    {
      drawRect = [self imageRectInRuler];
      drawPoint = drawRect.origin;
      if (flipped)
        {
          drawPoint.y += drawRect.size.height;
        }
      /* make marker disappear (-drawRect: does not draw marker when dragged) */  
      [_rulerView drawRect: drawRect];
    }
  [window cacheImageInRect: [_rulerView convertRect: drawRect toView: nil]];
  [_image compositeToPoint: drawPoint
                 operation: NSCompositeSourceOver];
  [window flushWindow];

  /* loop processing events until mouse up */
  while (_isDragging)
    {
      if(newEvent == nil) 
	{
	  newEvent = theEvent;
	  previousMousePositionInWindow = NSMakePoint(0,0); 
	}
      else
	{
	  newEvent = [NSApp nextEventMatchingMask: eventMask
			    untilDate: [NSDate distantFuture]
			    inMode: NSEventTrackingRunLoopMode
			    dequeue: YES];
	}

      switch ([newEvent type]) 
	{
	  case NSLeftMouseDown:
	  case NSLeftMouseDragged:
	      /* take mouse position from outside of event stream
	         and ignore event if in same position as previous event, 
		 to reduce the number of events to process */
              mousePositionInWindow = [window mouseLocationOutsideOfEventStream];
	      if (NSEqualPoints(mousePositionInWindow, previousMousePositionInWindow))
	        {
		  break;
		}
	      previousMousePositionInWindow = mousePositionInWindow;
	      
	      /* offset mouse position to marker's location */
	      mousePositionInWindow.x += mouseOffset.x;
	      mousePositionInWindow.y += mouseOffset.y;
	      
	      /* see if mouse is far from ruler area (to remove marker) */
              mousePositionInRuler = [_rulerView convertPoint: mousePositionInWindow
                                                     fromView: nil];
	      isFar = !NSMouseInRect(mousePositionInRuler, bounds, flipped);
	      
	      /* if it is the first time it's far from the ruler area,
	         see if it can be removed */
	      if (isFar && !askedCanRemove)
	        {
		  if (adding)
		    {
		      canRemove = YES;
		    }
		  else if (!_isRemovable)
		    {
		      canRemove = NO;
		    }
		  else if ([client respondsToSelector: @selector(rulerView:shouldRemoveMarker:)])
                    {
                      canRemove = [client rulerView: _rulerView
		                 shouldRemoveMarker: self];
		    }
		  else
		    {
		      canRemove = YES;
		    }
		  askedCanRemove = YES;
		}

              /* calculate new marker location */
	      mousePositionInClient = [client convertPoint: mousePositionInWindow
                			          fromView: nil];
	      if ([_rulerView orientation] == NSHorizontalRuler)
	        {
	          location = mousePositionInClient.x;
	        }
	      else
	        {
	          location = mousePositionInClient.y;
	        }
		
	      /* give client a chance to change location */
	      if (adding && !isFar)
	        {
	          if ([client respondsToSelector: @selector(rulerView:willAddMarker:atLocation:)])
                    {
                      location = [client rulerView: _rulerView
                                     willAddMarker: self
                                        atLocation: location];
	            }
	        }
	      else if (_isMovable && !(isFar && canRemove))
	        {
	          if ([client respondsToSelector: @selector(rulerView:willMoveMarker:toLocation:)])
                    {
                      location = [client rulerView: _rulerView
                                    willMoveMarker: self
                                        toLocation: location];
	            }
	        }
	      _location = location;
	      
	      /* calculate position to draw marker */
	      drawRect = [self imageRectInRuler];
	      drawPoint = drawRect.origin;
	      if (flipped)
	        {
                  drawPoint.y += drawRect.size.height;
	        }
	      if (isFar && canRemove)
	        {
		  if ([_rulerView orientation] == NSHorizontalRuler)
		    {
		      float offset;
		      offset = mousePositionInRuler.y - [_rulerView baselineLocation];
		      drawPoint.y += offset;
		      drawRect.origin.y += offset;
		    }
                  else
		    {
		      float offset;
		      offset = mousePositionInRuler.x - [_rulerView baselineLocation];
		      drawPoint.x += offset;
		      drawRect.origin.x += offset;
		    }
	        }
		
	      /* undraw marker and redraw in new position */
	      [window restoreCachedImage];
              [window cacheImageInRect: [_rulerView convertRect: drawRect
                                                         toView: nil]];
	      [_image compositeToPoint: drawPoint
                             operation: NSCompositeSourceOver];
	      [window flushWindow];
	      break;

	  case NSLeftMouseUp:
	      if (adding)
	        {
	          if (isFar)
	            {
                      /* do nothing, it won't be added */
		      [window restoreCachedImage];
	              returnValue = NO;
	            }
	          else
	            {
                      /* marker is added to ruler view; inform client */
		      [window discardCachedImage];
	              [_rulerView addMarker:self];
	              if ([client respondsToSelector: @selector(rulerView:didAddMarker:)])
                	{
                	  [client rulerView: _rulerView didAddMarker: self];
	        	}
	              returnValue = YES;
	            }
	        }
	      else
	        {
	          if (isFar && _isRemovable && canRemove)
	            {
                      /* remove marker from ruler and inform client */
                      /* this could result in marker being dealloc'd; that's
                         why there is a retain before the loop */
		      [window restoreCachedImage];
	              [_rulerView removeMarker:self];
	              if ([client respondsToSelector: @selector(rulerView:didRemoveMarker:)])
                	{
                	  [client rulerView: _rulerView didRemoveMarker: self];
	        	}
	              returnValue = YES;
	            }
	          else if (_isMovable)
	            {
                      /* inform client that marker has been moved */
		      [window discardCachedImage];
	              if ([client respondsToSelector: @selector(rulerView:didMoveMarker:)])
                	{
                	  [client rulerView: _rulerView didMoveMarker: self];
	        	}
	              returnValue = YES;
	            }
	          else
	            {
		      [window restoreCachedImage];
	              returnValue = NO;
	            }
	        }
	      _isDragging = NO;
	      break;
	  default:
	      break;
        }
    }
  [_rulerView unlockFocus];
  [NSCursor unhide];
  [window flushWindow];
  RELEASE(self); // was retained when dragging started
  return returnValue;
}

// NSCopying protocol
- (id) copyWithZone: (NSZone*)zone
{
  NSRulerMarker *new = (NSRulerMarker*)NSCopyObject (self, 0, zone);

  new->_image = [_image copyWithZone: zone];
  new->_isDragging = NO;
  return new;
}

// NSCoding protocol
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      [aCoder encodeObject: _rulerView];
      [aCoder encodeObject: _image];
      [aCoder encodeConditionalObject: _representedObject];
      [aCoder encodePoint: _imageOrigin];
      [aCoder encodeValueOfObjCType: @encode(CGFloat) at: &_location];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isMovable];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isRemovable];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      _rulerView = [aDecoder decodeObject];
      _image = [aDecoder decodeObject];
      _representedObject = [aDecoder decodeObject];
      _imageOrigin = [aDecoder decodePoint];
      [aDecoder decodeValueOfObjCType: @encode(CGFloat) at: &_location];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isMovable];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isRemovable];
    }
  return self;
}

// NSObject protocol

@end





