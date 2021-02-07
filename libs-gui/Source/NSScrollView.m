/** <title>NSScrollView</title>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: July 1997
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: October 1998
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: February 1999
   Table View Support: Nicola Pero <n.pero@mi.flashnet.it>
   Date: March 2000

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

#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSUserDefaults.h>

#import "AppKit/NSColor.h"
#import "AppKit/NSColorList.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSInterfaceStyle.h"
#import "AppKit/NSRulerView.h"
#import "AppKit/NSScroller.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTableHeaderView.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/PSOperators.h"
#import "GNUstepGUI/GSTheme.h"

@interface NSClipView (Private)
- (void) _scrollToPoint: (NSPoint)aPoint;
@end

//
// For nib compatibility, this is used to properly
// initialize the object from a OS X nib file in initWithCoder:.
//
typedef struct _scrollViewFlags 
{
#if GS_WORDS_BIGENDIAN == 1
  unsigned int __unused6:14;
  unsigned int __unused5:1;
  unsigned int autohidesScrollers:1;
  unsigned int __unused4:1;
  unsigned int __unused3:1;
  unsigned int __unused2:1;
  unsigned int doesNotDrawBackground:1;
  unsigned int __unused1:1;
  unsigned int hasVRuler:1;
  unsigned int hasHRuler:1;
  unsigned int showRulers:1;
  unsigned int oldRulerInstalled:1;
  unsigned int nonDynamic:1;
  unsigned int hasHScroller:1;
  unsigned int hasVScroller:1;
  unsigned int hScrollerRequired:1;
  unsigned int vScrollerRequired:1;
  NSBorderType border:2;
#else
  NSBorderType border:2;
  unsigned int vScrollerRequired:1;
  unsigned int hScrollerRequired:1;
  unsigned int hasVScroller:1;
  unsigned int hasHScroller:1;
  unsigned int nonDynamic:1;
  unsigned int oldRulerInstalled:1;
  unsigned int showRulers:1;
  unsigned int hasHRuler:1;
  unsigned int hasVRuler:1;
  unsigned int __unused1:1;
  unsigned int doesNotDrawBackground:1;
  unsigned int __unused2:1;
  unsigned int __unused3:1;
  unsigned int __unused4:1;
  unsigned int autohidesScrollers:1;
  unsigned int __unused5:1;
  unsigned int __unused6:14;
#endif  
} GSScrollViewFlags;

@interface NSScrollView (GSPrivate)
/* GNUstep private methods */
- (void) _synchronizeHeaderAndCornerView;
- (void) _themeDidActivate: (NSNotification*)notification;
@end

@implementation NSScrollView

/*
 * Class variables
 */
static Class rulerViewClass = nil;
static CGFloat scrollerWidth;

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSScrollView class])
    {
      [self setRulerViewClass: [NSRulerView class]];
      scrollerWidth = [NSScroller scrollerWidth];
      [self setVersion: 2];
    }
}

+ (void) setRulerViewClass: (Class)aClass
{
  rulerViewClass = aClass;
}

+ (Class) rulerViewClass
{
  return rulerViewClass;
}

+ (NSSize) contentSizeForFrameSize: (NSSize)frameSize
             hasHorizontalScroller: (BOOL)hFlag
               hasVerticalScroller: (BOOL)vFlag
                        borderType: (NSBorderType)borderType
{
  NSSize size = frameSize;
  NSSize border = [[GSTheme theme] sizeForBorderType: borderType];
  CGFloat innerBorderWidth = [[NSUserDefaults standardUserDefaults]
			      boolForKey: @"GSScrollViewNoInnerBorder"] ? 0.0 : 1.0;

  /*
   * Substract 1 from the width and height of
   * the line that separates the horizontal
   * and vertical scroller from the clip view
   */
  if (hFlag)
    {
      size.height -= scrollerWidth + innerBorderWidth;
    }
  if (vFlag)
    {
      size.width -= scrollerWidth + innerBorderWidth;
    }

  size.width -= 2 * border.width;
  size.height -= 2 * border.height;

  return size;
}

+ (NSSize) frameSizeForContentSize: (NSSize)contentSize
             hasHorizontalScroller: (BOOL)hFlag
               hasVerticalScroller: (BOOL)vFlag
                        borderType: (NSBorderType)borderType
{
  NSSize size = contentSize;
  NSSize border = [[GSTheme theme] sizeForBorderType: borderType];
  CGFloat innerBorderWidth = [[NSUserDefaults standardUserDefaults]
			      boolForKey: @"GSScrollViewNoInnerBorder"] ? 0.0 : 1.0;

  /*
   * Add 1 to the width and height for the line that separates the
   * horizontal and vertical scroller from the clip view.
   */
  if (hFlag)
    {
      size.height += scrollerWidth + innerBorderWidth;
    }
  if (vFlag)
    {
      size.width += scrollerWidth + innerBorderWidth;
    }

  size.width += 2 * border.width;
  size.height += 2 * border.height;

  return size;
}

/*
 * Instance methods
 */
- (id) initWithFrame: (NSRect)rect
{
  NSClipView *clipView;

  self = [super initWithFrame: rect];
  if (!self)
    return nil;

  clipView = [NSClipView new];
  [self setContentView: clipView];
  RELEASE(clipView);

  _hLineScroll = 10;
  _hPageScroll = 10;
  _vLineScroll = 10;
  _vPageScroll = 10;
  _borderType = NSNoBorder;
  _scrollsDynamically = YES;
  //_autohidesScrollers = NO;
  // FIXME: Not sure here Apple says by default all scrollers are off.
  // For compatibility the ruler should be present but not visible.
  [self setHasHorizontalRuler: YES];
  [self tile];
  _horizScrollElasticity = NSScrollElasticityAutomatic;
  _vertScrollElasticity = NSScrollElasticityAutomatic;

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_themeDidActivate:)
    name: GSThemeDidActivateNotification
    object: nil];
  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];

  DESTROY(_horizScroller);
  DESTROY(_vertScroller);
  DESTROY(_horizRuler);
  DESTROY(_vertRuler);

  [super dealloc];
}

- (BOOL) isFlipped
{
  return YES;
}

- (void) setContentView: (NSClipView *)aView
{
  if (aView == nil)
    [NSException raise: NSInvalidArgumentException
                format: @"Attempt to set nil content view"];
  if ([aView isKindOfClass: [NSView class]] == NO)
    [NSException raise: NSInvalidArgumentException
                format: @"Attempt to set non-view object as content view"];

  if (aView != _contentView)
    {
      NSView *docView = [aView documentView];

      [_contentView removeFromSuperview];
      [self addSubview: aView];
      // This must be done after adding it as a subview,
      // otherwise it will get unset again.
      _contentView = aView;

      if (docView != nil)
        {
          [self setDocumentView: docView];
        }
    }
  [_contentView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [self tile];
}

- (void) willRemoveSubview: (NSView *)aView
{
  if (aView == _contentView)
    {
      _contentView = nil;
    }
  if (aView == _headerClipView)
    {
      _headerClipView = nil;
    }
  if (aView == _cornerView)
    {
      _cornerView = nil;
    }
  [super willRemoveSubview: aView];
}

- (void) setHorizontalScroller: (NSScroller*)aScroller
{
  [_horizScroller removeFromSuperview];

  /*
   * Do not add the scroller view to the subviews array yet;
   * -setHasHorizontalScroller must be invoked first
   */
  ASSIGN(_horizScroller, aScroller);
  if (_horizScroller)
    {
      [_horizScroller setAutoresizingMask: NSViewWidthSizable];
      [_horizScroller setTarget: self];
      [_horizScroller setAction: @selector(_doScroll:)];
    }
}

- (void) setHasHorizontalScroller: (BOOL)flag
{
  if (_hasHorizScroller == flag)
    return;

  _hasHorizScroller = flag;

  if (_hasHorizScroller)
    {
      if (!_horizScroller)
        {
          NSScroller *scroller = [NSScroller new];

          [self setHorizontalScroller: scroller];
          RELEASE(scroller);
        }
      [self addSubview: _horizScroller];
    }
  else
    [_horizScroller removeFromSuperview];

  [self tile];
}

- (void) setVerticalScroller: (NSScroller*)aScroller
{
  [_vertScroller removeFromSuperview];

  /*
   * Do not add the scroller view to the subviews array yet;
   * -setHasVerticalScroller must be invoked first
   */
  ASSIGN(_vertScroller, aScroller);
  if (_vertScroller)
    {
      [_vertScroller setAutoresizingMask: NSViewHeightSizable];
      [_vertScroller setTarget: self];
      [_vertScroller setAction: @selector(_doScroll:)];
    }
}

- (void) setHasVerticalScroller: (BOOL)flag
{
  if (_hasVertScroller == flag)
    return;

  _hasVertScroller = flag;

  if (_hasVertScroller)
    {
      if (!_vertScroller)
        {
          NSScroller *scroller = [NSScroller new];

          [self setVerticalScroller: scroller];
          RELEASE(scroller);
          if (_contentView && ![_contentView isFlipped])
            [_vertScroller setFloatValue: 1];
        }
      [self addSubview: _vertScroller];
    }
  else
    [_vertScroller removeFromSuperview];

  [self tile];
}

/**
 <p> Return wether scroller autohiding is set or not. </p>
 <p>See Also: -setAutohidesScrollers:</p>
*/
- (BOOL) autohidesScrollers
{
  return _autohidesScrollers;
}

/**
 <p>Sets whether the view hides the scrollers (horizontal and/or vertical independendently) if they are not needed.</p>
 <p>If the content fits inside the clip view on the X or Y axis or both, the respective scroller is removed and additional space is gained.</p>
 <p>See Also: -autohidesScrollers</p>
 */
- (void) setAutohidesScrollers: (BOOL)flag
{
  _autohidesScrollers = flag;
}

- (NSScrollElasticity)horizontalScrollElasticity
{
  return _horizScrollElasticity;
}

- (void)setHorizontalScrollElasticity:(NSScrollElasticity)value
{
  _horizScrollElasticity = value;
}

- (NSScrollElasticity)verticalScrollElasticity
{
  return _vertScrollElasticity;
}


- (void)setVerticalScrollElasticity:(NSScrollElasticity)value
{
  _vertScrollElasticity = value;
}

- (void) scrollWheel: (NSEvent *)theEvent
{
  NSRect clipViewBounds;
  CGFloat deltaY = [theEvent deltaY];
  CGFloat deltaX = [theEvent deltaX];
  CGFloat amount;
  NSPoint point;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
    }
  point = clipViewBounds.origin;

  // Holding shift converts vertical scrolling to horizontal
  if (([theEvent modifierFlags] & NSShiftKeyMask) == NSShiftKeyMask)
    {
      deltaX = -deltaY;
      deltaY = 0;
    }

  // Scroll horizontally
  if (([theEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
    {
      amount = (clipViewBounds.size.width - _hPageScroll) * deltaX;
    }
  else
    {
      amount = _hLineScroll * deltaX;
    }

  NSDebugLLog (@"NSScrollView", 
    @"increment/decrement: amount = %f, horizontal", amount);

  point.x = clipViewBounds.origin.x + amount;

  // Scroll vertically
  if (([theEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask)
    {
      amount = - (clipViewBounds.size.height - _vPageScroll) * deltaY;
    }
  else
    {
      amount = - _vLineScroll * deltaY;
    }

  if (_contentView != nil && ![_contentView isFlipped])
    {
      /* If view is flipped reverse the scroll direction */
      amount = -amount;
    }
  NSDebugLLog (@"NSScrollView", 
    @"increment/decrement: amount = %f, flipped = %d",
    amount, _contentView ? [_contentView isFlipped] : 0);

  point.y = clipViewBounds.origin.y + amount;

  /* scrollToPoint: will call reflectScrolledClipView:, which will
   * update rules, headers, and scrollers.  */
  [_contentView _scrollToPoint: point];
}

- (void) keyDown: (NSEvent *)theEvent
{
  NSString *chars = [theEvent characters];
  unichar c = [chars length] == 1 ? [chars characterAtIndex: 0] : '\0';

  switch (c)
    {
      case NSUpArrowFunctionKey:
        [self scrollLineUp: self];
        break;

      case NSDownArrowFunctionKey:
        [self scrollLineDown: self];
        break;

      case NSPageUpFunctionKey:
        [self scrollPageUp: self];
        break;

      case NSPageDownFunctionKey:
        [self scrollPageDown: self];
        break;

      default:
        [super keyDown: theEvent];
        break;
    }
}

/*
 * This code is based on _doScroll: and still may need some tuning. 
 */
- (void) scrollLineUp: (id)sender
{
  NSRect  clipViewBounds;
  NSPoint point;
  CGFloat amount;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
    }
  point = clipViewBounds.origin;
  amount = _vLineScroll;
  if (_contentView != nil && ![_contentView isFlipped])
    {
      amount = -amount;
    }
  point.y = clipViewBounds.origin.y - amount;
  [_contentView _scrollToPoint: point];
}


/*
 * This code is based on _doScroll: and still may need some tuning. 
 */
- (void) scrollLineDown: (id)sender
{
  NSRect  clipViewBounds;
  NSPoint point;
  CGFloat amount;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
    }
  point = clipViewBounds.origin;
  amount = _vLineScroll;
  if (_contentView != nil && ![_contentView isFlipped])
    {
      amount = -amount;
    }
  point.y = clipViewBounds.origin.y + amount;
  [_contentView _scrollToPoint: point];
}

/**
 * Scrolls the receiver by simply invoking scrollPageUp:
 */
- (void) pageUp: (id)sender
{
  [self scrollPageUp: sender];
}

/*
 * This code is based on _doScroll: and still may need some tuning. 
 */
- (void) scrollPageUp: (id)sender
{
  NSRect  clipViewBounds;
  NSPoint point;
  CGFloat amount;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
    }
  point = clipViewBounds.origin;
  /*
   * Take verticalPageScroll into accout, but try to make sure
   * that amount is never negative (ie do not scroll backwards.)
   *
   * FIXME: It seems _doScroll and scrollWheel: should also take
   * care not to do negative scrolling.
   */
  amount = clipViewBounds.size.height - _vPageScroll;
  amount = (amount < 0) ? 0 : amount;

  if (_contentView != nil && ![_contentView isFlipped])
    {
      amount = -amount;
    }
  point.y = clipViewBounds.origin.y - amount;
  [_contentView _scrollToPoint: point];
}

/**
 * Scrolls the receiver by simply invoking scrollPageUp:
 */
- (void) pageDown: (id)sender
{
  [self scrollPageDown: sender];
}

/*
 * This code is based on _doScroll:. and still may need some tuning.
 */
- (void) scrollPageDown: (id)sender
{
  NSRect  clipViewBounds;
  NSPoint point;
  CGFloat amount;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
    }
  point = clipViewBounds.origin;
  /*
   * Take verticalPageScroll into accout, but try to make sure
   * that amount is never negativ (ie do not scroll backwards.)
   *
   * FIXME: It seems _doScroll and scrollWheel: should also take
   * care not to do negative scrolling.
   */
  amount = clipViewBounds.size.height - _vPageScroll;
  amount = (amount < 0) ? 0 : amount;
  if (_contentView != nil && ![_contentView isFlipped])
    {
      amount = -amount;
    }
  point.y = clipViewBounds.origin.y + amount;
  [_contentView _scrollToPoint: point];
}

- (void) _doScroll: (NSScroller*)scroller
{
  float floatValue = [scroller floatValue];
  NSScrollerPart hitPart = [scroller hitPart];
  NSRect clipViewBounds;
  NSRect documentRect;
  CGFloat amount = 0;
  NSPoint point;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
      documentRect = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
      documentRect = [_contentView documentRect];
    }
  point = clipViewBounds.origin;

  NSDebugLLog (@"NSScrollView", @"_doScroll: float value = %f", floatValue);

  /* do nothing if scroller is unknown */
  if (scroller != _horizScroller && scroller != _vertScroller)
    return;

  _knobMoved = NO;

  if (hitPart == NSScrollerKnob || hitPart == NSScrollerKnobSlot)
    _knobMoved = YES;
  else
    {
      if (hitPart == NSScrollerIncrementLine)
        {
          if (scroller == _horizScroller)
            amount = _hLineScroll;
          else
            amount = _vLineScroll;
        }
      else if (hitPart == NSScrollerDecrementLine)
        {
          if (scroller == _horizScroller)
            amount = -_hLineScroll;
          else
            amount = -_vLineScroll;
        }
      else if (hitPart == NSScrollerIncrementPage)
        {
          if (scroller == _horizScroller)
            amount = clipViewBounds.size.width - _hPageScroll;
          else
            amount = clipViewBounds.size.height - _vPageScroll;
        }
      else if (hitPart == NSScrollerDecrementPage)
        {
          if (scroller == _horizScroller)
            amount = _hPageScroll - clipViewBounds.size.width;
          else
            amount = _vPageScroll - clipViewBounds.size.height;
        }
      else
        {
          return;
        }
    }

  if (!_knobMoved)        /* button scrolling */
    {
      if (scroller == _horizScroller)
        {
          point.x = clipViewBounds.origin.x + amount;
        }
      else
        {
          if (_contentView != nil && ![_contentView isFlipped])
            {
              /* If view is flipped reverse the scroll direction */
              amount = -amount;
            }
          NSDebugLLog (@"NSScrollView", 
                       @"increment/decrement: amount = %f, flipped = %d",
            amount, _contentView ? [_contentView isFlipped] : 0);
          point.y = clipViewBounds.origin.y + amount;
        }
    }
  else         /* knob scolling */
    {
      if (scroller == _horizScroller)
        {
          point.x = floatValue * (documentRect.size.width
                                              - clipViewBounds.size.width);
          point.x += documentRect.origin.x;
        }
      else
        {
          if (_contentView != nil && ![_contentView isFlipped])
            floatValue = 1 - floatValue;
          point.y = floatValue * (documentRect.size.height
            - clipViewBounds.size.height);
          point.y += documentRect.origin.y;
        }
    }

  /* scrollToPoint will call reflectScrollerClipView, and that will
   * update scrollers, rulers and headers */
  [_contentView _scrollToPoint: point];
}

- (void) scrollToBeginningOfDocument: (id)sender
{
  NSRect  clipViewBounds, documentRect;
  NSPoint point;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
      documentRect = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
      documentRect = [_contentView documentRect];
    }
  point = documentRect.origin;
  if (_contentView != nil && ![_contentView isFlipped])
    {
      point.y = NSMaxY(documentRect) - NSHeight(clipViewBounds);
      if (point.y < 0)
        point.y = 0;
    }
  [_contentView _scrollToPoint: point];
}

- (void) scrollToEndOfDocument: (id)sender
{
  NSRect  clipViewBounds, documentRect;
  NSPoint point;

  if (_contentView == nil)
    {
      clipViewBounds = NSZeroRect;
      documentRect = NSZeroRect;
    }
  else
    {
      clipViewBounds = [_contentView bounds];
      documentRect = [_contentView documentRect];
    }
  point = documentRect.origin;
  if (_contentView == nil || [_contentView isFlipped])
    {
      point.y = NSMaxY(documentRect) - NSHeight(clipViewBounds);
      if (point.y < 0)
        point.y = 0;
    }
  [_contentView _scrollToPoint: point];
}

//
// This method is here purely for nib compatibility.  This is the action
// connected to by NSScrollers in IB when building a scrollview.
//
- (void) _doScroller: (NSScroller *)scroller
{
  [self _doScroll: scroller];
}

- (void) reflectScrolledClipView: (NSClipView *)aClipView
{
  NSRect documentFrame = NSZeroRect;
  NSRect clipViewBounds = NSZeroRect;
  float floatValue;
  CGFloat knobProportion;
  id documentView;

  if (aClipView != _contentView)
    {
      return;
    }

  NSDebugLLog (@"NSScrollView", @"reflectScrolledClipView:");

  if (_contentView)
    {
      clipViewBounds = [_contentView bounds];
    }
  if ((documentView = [_contentView documentView]))
    {
      documentFrame = [documentView frame];
    }

  // FIXME: Should we just hide the scroll bar or remove it?
  if ((_autohidesScrollers)
    && (documentFrame.size.height > clipViewBounds.size.height))
    {
      [self setHasVerticalScroller: YES];        
    } 
 
  if (_hasVertScroller)
    {
      if (documentFrame.size.height <= clipViewBounds.size.height)
        {
          if (_autohidesScrollers)
            {
              [self setHasVerticalScroller: NO];
            }
          else
            {
              [_vertScroller setEnabled: NO];
            }
        }
      else
        {
          [_vertScroller setEnabled: YES];

          knobProportion = clipViewBounds.size.height
            / documentFrame.size.height;

          floatValue = (clipViewBounds.origin.y - documentFrame.origin.y)
            / (documentFrame.size.height - clipViewBounds.size.height);

          if (![_contentView isFlipped])
            {
              floatValue = 1 - floatValue;
            }
          [_vertScroller setFloatValue: floatValue 
                         knobProportion: knobProportion];
        }
    }

  if ((_autohidesScrollers)
    && (documentFrame.size.width > clipViewBounds.size.width))
    {
      [self setHasHorizontalScroller: YES];        
    } 
 
  if (_hasHorizScroller)
    {
      if (documentFrame.size.width <= clipViewBounds.size.width)
        {
          if (_autohidesScrollers)
            {
              [self setHasHorizontalScroller: NO];
            }
          else
            {
              [_horizScroller setEnabled: NO];
            }
        }
      else
        {
          [_horizScroller setEnabled: YES];

          knobProportion = clipViewBounds.size.width
            / documentFrame.size.width;

          floatValue = (clipViewBounds.origin.x - documentFrame.origin.x)
            / (documentFrame.size.width - clipViewBounds.size.width);

          [_horizScroller setFloatValue: floatValue
                         knobProportion: knobProportion];
        }
    }

  if (_hasHeaderView)
    {
      NSPoint headerClipViewOrigin;
      
      headerClipViewOrigin = [_headerClipView bounds].origin;

      /* If needed, scroll the headerview too.  */
      if (headerClipViewOrigin.x != clipViewBounds.origin.x)
        {
          headerClipViewOrigin.x = clipViewBounds.origin.x;
          [_headerClipView scrollToPoint: headerClipViewOrigin];
        }
    }

  if (_rulersVisible == YES)
    {
      if (_hasHorizRuler)
        {
          [_horizRuler setNeedsDisplay: YES];
        }
      if (_hasVertRuler)
        {
          [_vertRuler setNeedsDisplay: YES];
       }
    }
}

- (void) setHorizontalRulerView: (NSRulerView *)aRulerView
{
  if (_rulersVisible && _horizRuler != nil)
    {
      [_horizRuler removeFromSuperview];
    }
  
  ASSIGN(_horizRuler, aRulerView);
  
  if (_horizRuler == nil)
    {
      _hasHorizRuler = NO;
    }
  else if (_rulersVisible)
    {
      [self addSubview: _horizRuler];
    }

  if (_rulersVisible)
    {
      [self tile];
    }
}

- (void) setHasHorizontalRuler: (BOOL)flag
{
  if (_hasHorizRuler == flag)
    return;

  _hasHorizRuler = flag;
  if (_hasHorizRuler && _horizRuler == nil)
    {
      _horizRuler = [[object_getClass(self) rulerViewClass] alloc];
      _horizRuler = [_horizRuler initWithScrollView: self 
                                 orientation: NSHorizontalRuler];
    }

  if (_rulersVisible)
    {
      if (_hasHorizRuler)
        {
          [self addSubview: _horizRuler];
        }
      else
        {
          [_horizRuler removeFromSuperview];
        }
      [self tile];
    }
}

- (void) setVerticalRulerView: (NSRulerView *)aRulerView
{
  if (_rulersVisible && _vertRuler != nil)
    {
      [_vertRuler removeFromSuperview];
    }
  
  ASSIGN(_vertRuler, aRulerView);
  
  if (_vertRuler == nil)
    {
      _hasVertRuler = NO;
    }
  else if (_rulersVisible)
    {
      [self addSubview: _vertRuler];
    }

  if (_rulersVisible)
    {
      [self tile];
    }
}

- (void) setHasVerticalRuler: (BOOL)flag
{
  if (_hasVertRuler == flag)
    return;

  _hasVertRuler = flag;
  if (_hasVertRuler && _vertRuler == nil)
    {
      _vertRuler = [[object_getClass(self) rulerViewClass] alloc];
      _vertRuler = [_vertRuler initWithScrollView: self 
                               orientation: NSVerticalRuler];
    }

  if (_rulersVisible)
    {
      if (_hasVertRuler)
        {
          [self addSubview: _vertRuler];
        }
      else
        {
          [_vertRuler removeFromSuperview];
        }
      [self tile];
    }
}

- (void) setRulersVisible: (BOOL)flag
{
  if (_rulersVisible == flag)
    return;

  _rulersVisible = flag;
  if (flag)
    {
      if (_hasVertRuler)
        [self addSubview: _vertRuler];
      if (_hasHorizRuler)
        [self addSubview: _horizRuler];
    }
  else 
    {
      if (_hasVertRuler)
        [_vertRuler removeFromSuperview];
      if (_hasHorizRuler)
        [_horizRuler removeFromSuperview];
    }
  [self tile];
}

- (void) setFrame: (NSRect)rect
{
  [super setFrame: rect];
  [self tile];
}

- (void) setFrameSize: (NSSize)size
{
  [super setFrameSize: size];
  [self tile];
}

static NSRectEdge
GSOppositeEdge(NSRectEdge edge)
{
  return (edge == NSMinXEdge) ? NSMaxXEdge : NSMinXEdge;
}

- (void) tile
{
  NSRect headerRect, contentRect;
  NSSize border = [[GSTheme theme] sizeForBorderType: _borderType];
  NSRectEdge bottomEdge, topEdge;
  CGFloat headerViewHeight = 0;
  NSRectEdge verticalScrollerEdge = NSMinXEdge;
  NSInterfaceStyle style;
  CGFloat innerBorderWidth = [[NSUserDefaults standardUserDefaults]
			      boolForKey: @"GSScrollViewNoInnerBorder"] ? 0.0 : 1.0;

  const BOOL useBottomCorner = [[GSTheme theme] scrollViewUseBottomCorner];
  const BOOL overlapBorders = [[GSTheme theme] scrollViewScrollersOverlapBorders];

  style = NSInterfaceStyleForKey(@"NSScrollViewInterfaceStyle", nil);

  if (style == NSMacintoshInterfaceStyle
    || style == NSWindows95InterfaceStyle)
    {
      verticalScrollerEdge = NSMaxXEdge;
    }

  /* Determine edge positions.  */
  if ([self isFlipped])
    {
      topEdge = NSMinYEdge;
      bottomEdge = NSMaxYEdge;
    }
  else
    {
      topEdge = NSMaxYEdge;
      bottomEdge = NSMinYEdge;
    }

  /* Prepare the contentRect by insetting the borders.  */
  contentRect = _bounds;

  if (!overlapBorders)
    contentRect = NSInsetRect(contentRect, border.width, border.height);

  if (contentRect.size.width < 0 || contentRect.size.height < 0)
    {
      /* FIXME ... should we do something else when given
       * too small a size to tile? */
      return;
    }
  
  [self _synchronizeHeaderAndCornerView];
  
  if (overlapBorders)
    {
      if (_borderType != NSNoBorder)
	{
	  if (!(_hasHeaderView || _hasCornerView))
	    {
	      // Inset 1px on the top
	      NSDivideRect(contentRect, NULL, &contentRect, 1, topEdge);
	    }
	  if (!_hasVertScroller)
	    {
	      // Inset 1px on the edge where the vertical scroller would be
	      NSDivideRect(contentRect, NULL, &contentRect, 1, verticalScrollerEdge);
	    }
	  if (!_hasHorizScroller)
	    {
	      NSDivideRect(contentRect, NULL, &contentRect, 1, bottomEdge);
	    }
	  // The vertical edge without a scroller
	  {
	    NSDivideRect(contentRect, NULL, &contentRect, 1, 
			 GSOppositeEdge(verticalScrollerEdge));
	  }
	}
    } 

  /* First, allocate vertical space for the headerView / cornerView
     (but - NB - the headerView needs to be placed above the clipview
     later on, we can't place it now).  */

  if (_hasHeaderView == YES)
    {
      headerViewHeight = [[_headerClipView documentView] frame].size.height;
    }

  if (_hasCornerView == YES)
    {
      if (headerViewHeight == 0)
        {
          headerViewHeight = [_cornerView frame].size.height;
        }
    }

  /* Remove the vertical slice used by the header/corner view.  Save
     the height and y position of headerRect for later reuse.  */
  NSDivideRect (contentRect, &headerRect, &contentRect, headerViewHeight, 
                topEdge);

  /* Ok - now go on with drawing the actual scrollview in the
     remaining space.  Just consider contentRect to be the area in
     which we draw, ignoring header/corner view.  */

  /* Prepare the vertical scroller.  */
  if (_hasVertScroller)
    {
      NSRect vertScrollerRect;

      NSDivideRect (contentRect, &vertScrollerRect, &contentRect, 
        scrollerWidth, verticalScrollerEdge);

      /* If the theme requests it, leave a square gap in the bottom-
       * left (or bottom-right) corner where the horizontal and vertical
       * scrollers meet. */
      if (_hasHorizScroller && !useBottomCorner)
	{
	  NSDivideRect (vertScrollerRect, NULL, &vertScrollerRect,
			scrollerWidth, bottomEdge);
	}

      /** Vertically expand the scroller by 1pt on each end */
      if (overlapBorders)
        {
	  vertScrollerRect.origin.y -= 1;
	  vertScrollerRect.size.height += 2;
	}
      else if (_hasHeaderView || _hasCornerView)
        {
	  vertScrollerRect.origin.y -= 1;
	  vertScrollerRect.size.height += 1;
        }

      [_vertScroller setFrame: vertScrollerRect];

      /* Substract 1 for the line that separates the vertical scroller
       * from the clip view (and eventually the horizontal scroller),
       * unless the GSScrollViewNoInnerBorder default is set. */
      NSDivideRect (contentRect, NULL, &contentRect, innerBorderWidth, verticalScrollerEdge);
    }

  /* Prepare the horizontal scroller.  */
  if (_hasHorizScroller)
    {
      NSRect horizScrollerRect;
      
      NSDivideRect (contentRect, &horizScrollerRect, &contentRect, 
        scrollerWidth, bottomEdge);

      /** Horizontall expand the scroller by 1pt on each end */
      if (overlapBorders)
	{
	  horizScrollerRect.origin.x -= 1;
	  horizScrollerRect.size.width += 2;
	}

      [_horizScroller setFrame: horizScrollerRect];

      /* Substract 1 for the width for the line that separates the
       * horizontal scroller from the clip view,
       * unless the GSScrollViewNoInnerBorder default is set. */
      NSDivideRect (contentRect, NULL, &contentRect, innerBorderWidth, bottomEdge);
    }

  /* Now place and size the header view to be exactly above the
     resulting clipview.  */
  if (_hasHeaderView)
    {
      NSRect rect = headerRect;

      rect.origin.x = contentRect.origin.x;
      rect.size.width = contentRect.size.width;

      [_headerClipView setFrame: rect];
    }

  /* Now place the corner view.  */
  if (_hasCornerView)
    {
      NSPoint p = headerRect.origin;

      if (verticalScrollerEdge == NSMaxXEdge)
        {
          p.x += contentRect.size.width;
        }
      [_cornerView setFrameOrigin: p];
    }

  /* Now place the rulers.  */
  if (_rulersVisible)
    {
      if (_hasHorizRuler)
        {
          NSRect horizRulerRect;
          
          NSDivideRect (contentRect, &horizRulerRect, &contentRect,
            [_horizRuler requiredThickness], topEdge);
          [_horizRuler setFrame: horizRulerRect];
        }

      if (_hasVertRuler)
        {
          NSRect vertRulerRect;
          
          NSDivideRect (contentRect, &vertRulerRect, &contentRect,
            [_vertRuler requiredThickness], NSMinXEdge);
          [_vertRuler setFrame: vertRulerRect];
        }
    }

  [_contentView setFrame: contentRect];
  [self setNeedsDisplay: YES];
}

- (void) drawRect: (NSRect)rect
{
   [[GSTheme theme] drawScrollViewRect: rect
		    inView: self];
}

- (NSRect) documentVisibleRect
{
  return [_contentView documentVisibleRect];
}

- (void) setBackgroundColor: (NSColor*)aColor
{
  [_contentView setBackgroundColor: aColor];
}

- (NSColor*) backgroundColor
{
  return [_contentView backgroundColor];
}

- (void) setDrawsBackground: (BOOL)flag
{
  [_contentView setDrawsBackground: flag];
  if ((flag == NO) && 
      [_contentView respondsToSelector: @selector(setCopiesOnScroll:)])
    [_contentView setCopiesOnScroll: NO];
}

- (BOOL) drawsBackground
{
  return [_contentView drawsBackground];
}

- (void) setBorderType: (NSBorderType)borderType
{
  _borderType = borderType;
  [self tile];
}

- (void) setDocumentView: (NSView *)aView
{
  [_contentView setDocumentView: aView];

  if (_contentView && ![_contentView isFlipped])
    {
      [_vertScroller setFloatValue: 1];
    }
  [self tile];
}

- (void) resizeSubviewsWithOldSize: (NSSize)oldSize
{
  [super resizeSubviewsWithOldSize: oldSize];
  [self tile];
}

- (id) documentView
{
  return [_contentView documentView];
}

- (NSCursor*) documentCursor
{
  return [_contentView documentCursor];
}

- (void) setDocumentCursor: (NSCursor*)aCursor
{
  [_contentView setDocumentCursor: aCursor];
}

- (BOOL) isOpaque
{
  // FIXME: Only needs to be NO in a corner case,
  // when [[GSTheme theme] scrollViewUseBottomCorner] is NO
  // and the theme tile for the bottom corner is transparent.
  // So maybe cache the value of 
  // [[GSTheme theme] scrollViewUseBottomCorner] and check it here.
  return NO;
}

- (NSBorderType) borderType
{
  return _borderType;
}

/** <p>Returns whether the NSScrollView has a horizontal ruler</p>
    <p>See Also: -setHasHorizontalRuler:</p>
 */
- (BOOL) hasHorizontalRuler
{
  return _hasHorizRuler;
}

/** <p>Returns whether the NSScrollView has a horizontal scroller</p>
    <p>See Also: -setHasHorizontalScroller:</p>
 */
- (BOOL) hasHorizontalScroller
{
  return _hasHorizScroller;
}

/** <p>Returns whether the NSScrollView has a vertical ruler</p>
    <p>See Also: -setHasVerticalRuler:</p>
 */
- (BOOL) hasVerticalRuler
{
  return _hasVertRuler;
}

/** <p>Returns whether the NSScrollView has a vertical scroller</p>
    <p>See Also: -setHasVerticalScroller:</p>
 */
- (BOOL) hasVerticalScroller
{
  return _hasVertScroller;
}

/**<p>Returns the size of the NSScrollView's content view</p>
 */
- (NSSize) contentSize
{
  return [_contentView bounds].size;
}

- (NSClipView *) contentView
{
  return _contentView;
}

- (NSRulerView *) horizontalRulerView
{
  return _horizRuler;
}

- (NSRulerView *) verticalRulerView
{
  return _vertRuler;
}

- (BOOL) rulersVisible
{
  return _rulersVisible;
}

- (void) setLineScroll: (CGFloat)aFloat
{
  _hLineScroll = aFloat;
  _vLineScroll = aFloat;
}

- (void) setHorizontalLineScroll: (CGFloat)aFloat
{
  _hLineScroll = aFloat;
}

- (void) setVerticalLineScroll: (CGFloat)aFloat
{
  _vLineScroll = aFloat;
}

- (CGFloat) lineScroll
{
  if (_hLineScroll != _vLineScroll)
    [NSException raise: NSInternalInconsistencyException
                format: @"horizontal and vertical values not same"];
  return _vLineScroll;
}

- (CGFloat) horizontalLineScroll
{
  return _hLineScroll;
}

- (CGFloat) verticalLineScroll
{
  return _vLineScroll;
}

- (void) setPageScroll: (CGFloat)aFloat
{
  _hPageScroll = aFloat;
  _vPageScroll = aFloat;
}

- (void) setHorizontalPageScroll: (CGFloat)aFloat
{
  _hPageScroll = aFloat;
}

- (void) setVerticalPageScroll: (CGFloat)aFloat
{
  _vPageScroll = aFloat;
}

- (CGFloat) pageScroll
{
  if (_hPageScroll != _vPageScroll)
    [NSException raise: NSInternalInconsistencyException
                format: @"horizontal and vertical values not same"];
  return _vPageScroll;
}

- (CGFloat) horizontalPageScroll
{
  return _hPageScroll;
}

- (CGFloat) verticalPageScroll
{
  return _vPageScroll;
}

- (void) setScrollsDynamically: (BOOL)flag
{
  // FIXME: This should change the behaviour of the scrollers
  _scrollsDynamically = flag;
}

- (BOOL) scrollsDynamically
{
  return _scrollsDynamically;
}

- (NSScroller*) horizontalScroller
{
  return _horizScroller;
}

- (NSScroller*) verticalScroller
{
  return _vertScroller;
}

- (BOOL)allowsMagnification
{
  //we need an ivar for this
  return NO;
}

- (void)setAllowsMagnification:(BOOL)m
{
  //we need an ivar for this
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
      
  if ([aCoder allowsKeyedCoding])
    {
      unsigned long flags = 0;

      [aCoder encodeObject: _horizScroller forKey: @"NSHScroller"];
      [aCoder encodeObject: _vertScroller forKey: @"NSVScroller"];
      [aCoder encodeObject: _contentView forKey: @"NSContentView"];

      // only encode this, if it's not null...
      if (_headerClipView != nil)
        {
          [aCoder encodeObject: _headerClipView forKey: @"NSHeaderClipView"];
        }

      flags = _borderType;
      if (_hasVertScroller)
        flags |= 16;
      if (_hasHorizScroller)
        flags |= 32;
      if (_autohidesScrollers)
        flags |= 512;

      [aCoder encodeInt: flags forKey: @"NSsFlags"];
    }
  else
    {
      [aCoder encodeObject: _contentView];
      // Was int, we need to stay compatible
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_borderType];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_scrollsDynamically];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_rulersVisible];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_hLineScroll];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_hPageScroll];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_vLineScroll];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_vPageScroll];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasHorizScroller];
      if (_hasHorizScroller)
        [aCoder encodeObject: _horizScroller];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasVertScroller];
      if (_hasVertScroller)
        [aCoder encodeObject: _vertScroller];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasHorizRuler];
      if (_hasHorizRuler)
        [aCoder encodeObject: _horizRuler];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasVertRuler];
      if (_hasVertRuler)
        [aCoder encodeObject: _vertRuler];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasHeaderView];
      if (_hasHeaderView)
        [aCoder encodeObject: _headerClipView];
      
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasCornerView];
      
      /* We do not need to encode headerview, cornerview stuff */
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      NSScroller *hScroller = [aDecoder decodeObjectForKey: @"NSHScroller"];
      NSScroller *vScroller = [aDecoder decodeObjectForKey: @"NSVScroller"];
      NSClipView *content = [aDecoder decodeObjectForKey: @"NSContentView"]; 
      NSView *docView = [content documentView];
      BOOL post_frame = [docView postsFrameChangedNotifications];
      BOOL post_bound = [docView postsBoundsChangedNotifications];

      [docView setPostsFrameChangedNotifications: NO];
      [docView setPostsBoundsChangedNotifications: NO];
      _hLineScroll = 10;
      _hPageScroll = 10;
      _vLineScroll = 10;
      _vPageScroll = 10;
      _scrollsDynamically = YES;
      /* _autohidesScroller, _rulersVisible, _hasHorizRuler and _hasVertRuler 
         implicitly set to NO */

      if ([aDecoder containsValueForKey: @"NSsFlags"])
        {
          int flags = [aDecoder decodeInt32ForKey: @"NSsFlags"];

          _borderType = flags & 3;
          _hasVertScroller = (flags & 16) == 16;
          _hasHorizScroller = (flags & 32) == 32;
          _autohidesScrollers = (flags & 512) == 512;
        }

      /* FIXME: This should only happen when we load a Mac NIB file.
         And as far as I can tell tile is handling this correctly.
      if (vScroller != nil && _hasVertScroller && content != nil)
        {
          // Move the content view since it is not moved when we retile.
          NSRect frame = [content frame];
          float w = [vScroller frame].size.width;

          //
          // Slide the content view over, since on Mac OS X the scroller is on the
          // right, the content view is not properly positioned since our scroller
          // is on the left.
          //
          frame.origin.x += w;
          [content setFrame: frame];
        }
      */

      if (hScroller != nil && _hasHorizScroller)
        {
          [self setHorizontalScroller: hScroller];
	  [hScroller setHidden: NO];
          [self addSubview: _horizScroller];
        }

      if (vScroller != nil && _hasVertScroller)
        {
          [self setVerticalScroller: vScroller];
	  [vScroller setHidden: NO];
          [self addSubview: _vertScroller];
        }

      if ([aDecoder containsValueForKey: @"NSHeaderClipView"])
        {
          _hasHeaderView = YES;
          _headerClipView = [aDecoder decodeObjectForKey: @"NSHeaderClipView"];
        }

      // set the document view into the content.
      [self setContentView: content];
      [self tile];
      // Reenable notification sending.
      [docView setPostsFrameChangedNotifications: post_frame];
      [docView setPostsBoundsChangedNotifications: post_bound];
    }
  else
    {
      int version = [aDecoder versionForClassName: @"NSScrollView"];
      NSDebugLLog(@"NSScrollView", @"NSScrollView: start decoding\n");
      _contentView = [aDecoder decodeObject];
      // Was int, we need to stay compatible
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_borderType];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_scrollsDynamically];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_rulersVisible];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_hLineScroll];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_hPageScroll];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_vLineScroll];
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_vPageScroll];
      
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasHorizScroller];
      if (_hasHorizScroller)
        [aDecoder decodeValueOfObjCType: @encode(id) at: &_horizScroller];
      
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasVertScroller];
      if (_hasVertScroller)
        [aDecoder decodeValueOfObjCType: @encode(id) at: &_vertScroller];
      
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasHorizRuler];
      if (_hasHorizRuler)
        [aDecoder decodeValueOfObjCType: @encode(id) at: &_horizRuler];
      
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasVertRuler];
      if (_hasVertRuler)
        [aDecoder decodeValueOfObjCType: @encode(id) at: &_vertRuler];
      
      if (version == 2)
        {
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasHeaderView];
          if (_hasHeaderView)
            _headerClipView = [aDecoder decodeObject];
      
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasCornerView];
        }
      else if (version == 1)
        {
          /* This recreates all the info about headerView, cornerView, etc */
          [self setDocumentView: [_contentView documentView]];
        }
      else
        {
          NSLog(@"unknown NSScrollView version (%d)", version);
          DESTROY(self);
          return nil;
        }
      [self tile];
      
      NSDebugLLog(@"NSScrollView", @"NSScrollView: finish decoding\n");
    }

  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(_themeDidActivate:)
    name: GSThemeDidActivateNotification
    object: nil];

  return self;
}

- (BOOL)automaticallyAdjustsContentInsets
{
  // FIXME
  return NO;
}

- (void)setAutomaticallyAdjustsContentInsets: (BOOL)adjusts
{
  static BOOL logged = NO;
  if (!logged)
    {
      NSLog(@"warning: stub no-op implementation of"
            "-[NSScrollView setAutomaticallyAdjustsContentInsets:]");
      logged = YES;
    }
}

- (NSEdgeInsets)contentInsets
{
  // FIXME
  return NSEdgeInsetsZero;
}
- (void)setContentInsets: (NSEdgeInsets)edgeInsets
{
  static BOOL logged = NO;
  if (!logged)
    {
      NSLog(@"warning: stub no-op implementation of"
            "-[NSScrollView setContentInsets:]");
      logged = YES;
    }
}

- (NSEdgeInsets)scrollerInsets
{
  // FIXME
  return NSEdgeInsetsZero;
}
- (void)setScrollerInsets: (NSEdgeInsets)insets
{
  static BOOL logged = NO;
  if (!logged)
    {
      NSLog(@"warning: stub no-op implementation of"
            "-[NSScrollView setScrollerInsets:]");
      logged = YES;
    }
}

@end

@implementation NSScrollView (GSPrivate)

/* GNUstep private method */

/* we update both of these at the same time during -tile
   so there is no reason in seperating them that'd just add
   message passing */
- (void) _synchronizeHeaderAndCornerView
{
  BOOL hadHeaderView = _hasHeaderView;
  BOOL hadCornerView = _hasCornerView;
  NSView *aView = nil;

  _hasHeaderView = ([[self documentView] 
                        respondsToSelector: @selector(headerView)]
                    && (aView=[(NSTableView *)[self documentView] headerView]));
  if (_hasHeaderView == YES)
    {
      if (hadHeaderView == NO)
        {
          _headerClipView = [NSClipView new];
          [self addSubview: _headerClipView];
          RELEASE(_headerClipView);
        }
      [_headerClipView setDocumentView: aView]; 
    }
  else if (hadHeaderView == YES)
    {
      [self removeSubview: _headerClipView];
    }
  if (_hasHeaderView == YES &&
      _hasVertScroller == YES)
    {
      aView = nil; 
      _hasCornerView =
        ([[self documentView] respondsToSelector: @selector(cornerView)]
         && (aView=[(NSTableView *)[self documentView] cornerView]));
      
      if (aView == _cornerView)
        return;
      if (_hasCornerView == YES)
        {
          if (hadCornerView == NO)
            {
               [self addSubview: aView];
            }
          else
            {
              [self replaceSubview: _cornerView with: aView];
            }
        }
      else if (hadCornerView == YES)
        {
          [self removeSubview: _cornerView];
        }
      _cornerView = aView;
    }
  else if (_cornerView != nil)
    {
      [self removeSubview: _cornerView];
      _cornerView = nil;
      _hasCornerView = NO;
    }
}

- (void) _themeDidActivate: (NSNotification*)notification
{
  // N.B. Reload cached [NSScroller scrollerWidth] since the
  // new theme may have a different scroller width.
  //
  // Since scrollerWidth is a static, it will get overwritten
  // several times; doesn't matter though.
  scrollerWidth = [NSScroller scrollerWidth];

  [self tile];
}

@end

