/** <title>NSTextContainer</title>

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: 2002-11-23

   Author: Jonathan Gapen <jagapen@smithlab.chem.wisc.edu>
   Date: 1999

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/ 

#import <Foundation/NSGeometry.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSDebug.h>
#import "AppKit/NSLayoutManager.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSTextView.h"
#import "GNUstepGUI/GSLayoutManager.h"
#import "GSGuiPrivate.h"

@interface NSTextContainer (TextViewObserver)
- (void) _textViewFrameChanged: (NSNotification*)aNotification;
@end


/* TODO: rethink how this is is triggered.
use bounds rectangle instead of frame? */
@implementation NSTextContainer (TextViewObserver)

- (void) _textViewFrameChanged: (NSNotification*)aNotification
{
  if (_observingFrameChanges)
    {
      id textView;
      NSSize newTextViewSize;
      NSSize size;
      NSSize inset;

      textView = [aNotification object];
      if (textView != _textView)
        {
            NSDebugLog(@"NSTextContainer got notification for wrong View %@",
                        textView);
            return;
        }
      newTextViewSize = [textView frame].size;
      size = _containerRect.size;
      inset = [textView textContainerInset];

      if (_widthTracksTextView)
        {
          size.width = MAX(newTextViewSize.width - (inset.width * 2.0), 0.0);
        }
      if (_heightTracksTextView)
        {
          size.height = MAX(newTextViewSize.height - (inset.height * 2.0), 0.0);
        }

      [self setContainerSize: size];
    }
}

@end /* NSTextContainer (TextViewObserver) */

@implementation NSTextContainer

+ (void) initialize
{
  if (self == [NSTextContainer class])
    {
      [self setVersion: 1];
    }
}

- (id) initWithContainerSize: (NSSize)aSize
{
  NSDebugLLog(@"NSText", @"NSTextContainer initWithContainerSize");
  if (aSize.width < 0)
    {
      NSWarnMLog(@"given negative width");
      aSize.width = 0;
    }
  if (aSize.height < 0)
    {
      NSWarnMLog(@"given negative height");
      aSize.height = 0;
    }
  _layoutManager = nil;
  _textView = nil;
  _containerRect.size = aSize;
  // Tests on Cocoa indicate the default value is 5.
  _lineFragmentPadding = 5.0; 
  _observingFrameChanges = NO;
  _widthTracksTextView = NO;
  _heightTracksTextView = NO;

  return self;
}

- (id) init
{
  return [self initWithContainerSize: NSMakeSize(1e7, 1e7)];
}

- (void) dealloc
{
  if (_textView != nil)
    {
      NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
      [nc removeObserver: self
          name: NSViewFrameDidChangeNotification
          object: _textView];

      [_textView setTextContainer: nil];
      RELEASE(_textView);
    }
  [super dealloc];
}

/*
See [NSTextView -setTextContainer:] for more information about these calls.
*/
- (void) setLayoutManager: (GSLayoutManager*)aLayoutManager
{
  /* The layout manager owns us - so he retains us and we don't retain 
     him. */
  _layoutManager = aLayoutManager;
  /* Tell our text view about the change. */
  [_textView setTextContainer: self];
}

- (GSLayoutManager*) layoutManager
{
  return _layoutManager;
}

/*
Replaces the layout manager while maintaining the text object
framework intact.
*/
- (void) replaceLayoutManager: (GSLayoutManager*)aLayoutManager
{
  if (aLayoutManager != _layoutManager)
    {
      NSTextStorage *textStorage = [_layoutManager textStorage];
      NSArray *textContainers = [_layoutManager textContainers];
      NSUInteger i, count = [textContainers count];
      GSLayoutManager *oldLayoutManager = _layoutManager;

      RETAIN(oldLayoutManager);
      RETAIN(textStorage);
      [textStorage removeLayoutManager: _layoutManager];
      [textStorage addLayoutManager: aLayoutManager];

      for (i = 0; i < count; i++)
        {
          NSTextContainer *container;

          container = RETAIN([textContainers objectAtIndex: i]);
          [oldLayoutManager removeTextContainerAtIndex: i];
          /* One of these calls will result in our _layoutManager being
          changed. */
          [aLayoutManager addTextContainer: container];

          /* The textview is caching the layout manager; refresh the
           * cache with this do-nothing call.  */
          /* TODO: probably unnecessary; the call in -setLayoutManager:
          should be enough */
          [[container textView] setTextContainer: container];
          RELEASE(container);
        }
      RELEASE(textStorage);
      RELEASE(oldLayoutManager);
    }
}

- (void) setTextView: (NSTextView*)aTextView
{
  NSNotificationCenter *nc;

  nc = [NSNotificationCenter defaultCenter];
          
  if (_textView)
    {
      [_textView setTextContainer: nil];
      [nc removeObserver: self  name: NSViewFrameDidChangeNotification 
          object: _textView];
      /* NB: We do not set posts frame change notifications for the
         text view to NO because there could be other observers for
         the frame change notifications. */
    }

  ASSIGN(_textView, aTextView);

  if (aTextView != nil)
    {
      [_textView setTextContainer: self];
      if (_observingFrameChanges)
        {
          [_textView setPostsFrameChangedNotifications: YES];
          [nc addObserver: self
              selector: @selector(_textViewFrameChanged:)
              name: NSViewFrameDidChangeNotification
              object: _textView];
          [self _textViewFrameChanged: 
                  [NSNotification notificationWithName: NSViewFrameDidChangeNotification
                                                object: _textView]];
        }
    }

  /* If someone's trying to set a NSTextView for us, the layout manager we
  have must be capable of handling NSTextView:s. */
  [(NSLayoutManager *)_layoutManager textContainerChangedTextView: self];
}

- (NSTextView*) textView
{
  return _textView;
}

- (void) setContainerSize: (NSSize)aSize
{
  if (NSEqualSizes(_containerRect.size, aSize))
    {
      return;
    }

  if (aSize.width < 0)
    {
      NSWarnMLog(@"given negative width");
      aSize.width = 0;
    }
  if (aSize.height < 0)
    {
      NSWarnMLog(@"given negative height");
      aSize.height = 0;
    }

  _containerRect = NSMakeRect(0, 0, aSize.width, aSize.height);

  if (_layoutManager)
    {
      [_layoutManager textContainerChangedGeometry: self];
    }
}

- (NSSize) containerSize
{
  return _containerRect.size;
}

- (void) setWidthTracksTextView: (BOOL)flag
{
  NSNotificationCenter *nc;
  BOOL old_observing = _observingFrameChanges;

  _widthTracksTextView = flag;
  _observingFrameChanges = _widthTracksTextView | _heightTracksTextView;

  if (_textView == nil)
    return;

  if (_observingFrameChanges == old_observing)
    return;

  nc = [NSNotificationCenter defaultCenter];

  if (_observingFrameChanges)
    {      
      [_textView setPostsFrameChangedNotifications: YES];
      [nc addObserver: self
          selector: @selector(_textViewFrameChanged:)
          name: NSViewFrameDidChangeNotification
          object: _textView];
    }
  else
    {
      [nc removeObserver: self name: NSViewFrameDidChangeNotification 
          object: _textView];
    }
}

- (BOOL) widthTracksTextView
{
  return _widthTracksTextView;
}

- (void) setHeightTracksTextView: (BOOL)flag
{
  NSNotificationCenter *nc;
  BOOL old_observing = _observingFrameChanges;

  _heightTracksTextView = flag;
  _observingFrameChanges = _widthTracksTextView | _heightTracksTextView;
  if (_textView == nil)
    return;

  if (_observingFrameChanges == old_observing)
    return;

  nc = [NSNotificationCenter defaultCenter];

  if (_observingFrameChanges)
    {      
      [_textView setPostsFrameChangedNotifications: YES];
      [nc addObserver: self
          selector: @selector(_textViewFrameChanged:)
          name: NSViewFrameDidChangeNotification
          object: _textView];
    }
  else
    {
      [nc removeObserver: self name: NSViewFrameDidChangeNotification 
          object: _textView];
    }
}

- (BOOL) heightTracksTextView
{
  return _heightTracksTextView;
}

- (void) setLineFragmentPadding: (CGFloat)aFloat
{
  _lineFragmentPadding = aFloat;

  if (_layoutManager)
    [_layoutManager textContainerChangedGeometry: self];
}

- (CGFloat) lineFragmentPadding
{
  return _lineFragmentPadding;
}

- (NSRect) lineFragmentRectForProposedRect: (NSRect)proposedRect
                            sweepDirection: (NSLineSweepDirection)sweepDir
                         movementDirection: (NSLineMovementDirection)moveDir
                             remainingRect: (NSRect *)remainingRect
{
  CGFloat minx, maxx, miny, maxy;
  CGFloat cminx, cmaxx, cminy, cmaxy;

  minx = NSMinX(proposedRect);
  maxx = NSMaxX(proposedRect);
  miny = NSMinY(proposedRect);
  maxy = NSMaxY(proposedRect);

  cminx = NSMinX(_containerRect) + _lineFragmentPadding;
  cmaxx = NSMaxX(_containerRect) - _lineFragmentPadding;
  cminy = NSMinY(_containerRect);
  cmaxy = NSMaxY(_containerRect);

  *remainingRect = NSZeroRect;

  if (minx >= cminx && maxx <= cmaxx && miny >= cminy && maxy <= cmaxy)
    {
      return proposedRect;
    }

  switch (moveDir)
    {
      case NSLineMovesLeft:
        if (maxx < cminx)
          return NSZeroRect;
        if (maxx > cmaxx)
          {
            minx -= maxx-cmaxx;
            maxx = cmaxx;
          }
        break;

      case NSLineMovesRight:
        if (minx > cmaxx)
          return NSZeroRect;
        if (minx < cminx)
          {
            maxx += cminx-minx;
            minx = cminx;
          }
        break;

      case NSLineMovesDown:
        if (miny > cmaxy)
          return NSZeroRect;
        if (miny < cminy)
          {
            maxy += cminy - miny;
            miny = cminy;
          }
        break;

      case NSLineMovesUp:
        if (maxy < cminy)
          return NSZeroRect;
        if (maxy > cmaxy)
          {
            miny -= maxy - cmaxy;
            maxy = cmaxy;
          }
        break;

      case NSLineDoesntMove:
        break;
    }

  switch (sweepDir)
    {
      case NSLineSweepLeft:
      case NSLineSweepRight:
        if (minx < cminx)
          minx = cminx;
        if (maxx > cmaxx)
          maxx = cmaxx;
        break;

      case NSLineSweepDown:
      case NSLineSweepUp:
        if (miny < cminy)
          miny = cminy;
        if (maxy > cmaxy)
          maxy = cmaxy;
        break;
    }

  if (minx < cminx || maxx > cmaxx ||
      miny < cminy || maxy > cmaxy)
    {
      return NSZeroRect;
    }

  return NSMakeRect(minx, miny,
                    (maxx > minx) ? maxx - minx : 0.0,
                    (maxy > miny) ? maxy - miny : 0.0);
}

- (BOOL) isSimpleRectangularTextContainer
{
  // sub-classes may say no; this class always says yes
  return YES;
}

- (BOOL) containsPoint: (NSPoint)aPoint
{
  return NSPointInRect(aPoint, _containerRect);
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      NSSize size = NSMakeSize(1e7, 1e7);

      if ([aDecoder containsValueForKey: @"NSWidth"])
        {
          size.width = [aDecoder decodeFloatForKey: @"NSWidth"];
        }
      self = [self initWithContainerSize: size];
      if ([aDecoder containsValueForKey: @"NSTCFlags"])
        {
          int flags = [aDecoder decodeIntForKey: @"NSTCFlags"];
          
          // decode the flags.
          _widthTracksTextView = (flags & 1) != 0;
          _heightTracksTextView = (flags & 2) != 0;
	  // Mac OS X doesn't seem to save this flag
          _observingFrameChanges = _widthTracksTextView | _heightTracksTextView;
        }

      // decoding the manager adds this text container automatically...
      if ([aDecoder containsValueForKey: @"NSLayoutManager"])
	{
	  _layoutManager = [aDecoder decodeObjectForKey: @"NSLayoutManager"];
	}

      return self;
    }
  else
    {
      return self;
    }
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      NSSize size = _containerRect.size;
      int flags = ((_widthTracksTextView)?1:0) | 
        ((_heightTracksTextView)?2:0) |
        ((_observingFrameChanges)?4:0);

      [coder encodeObject: _textView forKey: @"NSTextView"];
      [coder encodeObject: _layoutManager forKey: @"NSLayoutManager"];
      [coder encodeFloat: size.width forKey: @"NSWidth"];
      [coder encodeInt: flags forKey: @"NSTCFlags"];
    }
}

@end /* NSTextContainer */

