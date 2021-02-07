/*
   NSTextContainer.h

   Text container for text system

   Copyright (C) 1999, 2003 Free Software Foundation, Inc.

   Author: Alexander Malmberg <alexander@malmberg.org>
   Date: April 2003

   Author: Jonathan Gapen <jagapen@smithlab.chem.wisc.edu>
   Date: 1999

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

/**
A text container defines a region in the plane. It is used by the
text system to lay out text: text is laid out inside this region. A layout
manager has a list of text containers that it lays out text in. A text
container may have one NSTextView attached to it that displays the text laid
out in the text container.

Note that the coordinate system used by NSTextContainer is the
same as in the rest of the text system classes, ie. positive y is down.

NSTextContainer itself defines a simple rectangular region as large as the
container size. In most cases, only a single, simple text container is used
with a layout manager and a text view. Examples of cases where you might want
to use several text containers, or subclasses that define more complex
regions, are:

<list>
  <item>
    Multi-page layout; one text container for each page.
  </item><item>
    Multi-column layout; one text container for each column.
  </item><item>
    Layout flowing around pictures; the text container would define a region
    that does not include the space used by the picture.
  </item>
</list>

If the region defined by a text container can change dynamically, the text
container should call [GSLayoutManager-textContainerChangedGeometry:]
whenever this happens.
*/

#ifndef _GNUstep_H_NSTextContainer
#define _GNUstep_H_NSTextContainer
#import <GNUstepBase/GSVersionMacros.h>

#if GS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class GSLayoutManager;
@class NSTextView;

enum {
  NSLineSweepLeft,
  NSLineSweepRight,
  NSLineSweepDown,
  NSLineSweepUp
};
typedef NSUInteger NSLineSweepDirection;

enum {
  NSLineDoesntMove,
  NSLineMovesLeft,
  NSLineMovesRight,
  NSLineMovesDown,
  NSLineMovesUp
};
typedef NSUInteger NSLineMovementDirection;

@interface NSTextContainer : NSObject
{
  id _layoutManager;
  id _textView;

  NSRect _containerRect;
  CGFloat _lineFragmentPadding;

  BOOL _observingFrameChanges;
  BOOL _widthTracksTextView;
  BOOL _heightTracksTextView;
}

/**
Initializes a new instance and sets the container size to aSize.
*/
- (id) initWithContainerSize: (NSSize)aSize;


/**
Querying the region
*/

/**
Returns YES if the region for this container is a rectangle
as large as the container size, otherwise NO. For simple rectangular regions,
the text system can apply certain optimizations.

NSTextContainer always returns YES. Subclasses that define more complex
regions must return NO.
*/
- (BOOL) isSimpleRectangularTextContainer;

/**
This is the main method used by the text system for querying the region
and flowing text in it. It takes a proposed line fragment rectangle and,
if possible, splits it into a valid line fragment rectangle, and a remaining
rectangle that can be used in subsequent calls to this method.

sweepDir is the direction that text moves inside the lines, and moveDir is
the direction lines move in, or NSLineDoesntMove if the line may not move.
The line sweep and line movement may not both be in the same dimension, ie.
both be vertical, or both be horizontal.

The method returns the first (according to the sweep direction) valid line
fragment rectangle in the proposed rectangle. This line fragment rectangle
is a sub-rectangle of the proposed rectangle in the sweep direction, and has
the same size in the other direction. (Ie. if the sweep direction is left,
the line fragment rectangle must have the same height as the proposed
rectangle.) If there is no valid line fragment rectangle in the proposed
rectangle, the proposed rectangle may be moved in the line movement direction
until there is one. If no valid line fragment rectangle can be returned,
the method returns NSZeroRect.

The remaining rectangle should be set to the potentially valid part of the
proposed rectangle, after moving in the line movement direction, that remains
after the first line fragment rectangle.

The proposed rectangle may be any rectangle; in particular, it may extend
outside the container size. The remaining rectangle does not have to be or
contain a valid line fragment rectangle.

Subclasses define regions by overriding this method.

Note: The TextContainerExample in the text system example collection
(TODO: link) contains an example implementation of an NSTextContainer subclass
that defiens a non-trivial region, as well as an interactive demonstration of
this method. It can also be used as a simple test of custom subclasses.

Note: Although a correct NSTextContainer implementation must handle all line
sweep and movement directions, the current standard typesetter will only call
this method with sweep right, and no movement or movement down. Relying on
this makes writing a subclass easier, but it is <em>not</em> safe.
*/
- (NSRect) lineFragmentRectForProposedRect: (NSRect)proposedRect
			    sweepDirection: (NSLineSweepDirection)sweepDir
			 movementDirection: (NSLineMovementDirection)moveDir
			     remainingRect: (NSRect *)remainingRect;

/**
Returns YES if aPoint is inside the region.

Subclasses define regions by overriding this method.
*/
- (BOOL) containsPoint: (NSPoint)aPoint;


/**
Managing the text network<br />

A text container may be attached to one layout manager and one text view.
The text container is retained by the layout manager, and retains the text
view.
*/

/**
Replaces the layout manager of this text container with aLayoutManager.
This is done without changing the rest of the text network, so
aLayoutManager will be connected to the text storage and text containers
that the current layout manager is connected to.
*/
- (void) replaceLayoutManager: (GSLayoutManager *)aLayoutManager;

/**
Returns the layout manager of this text container.
*/
- (GSLayoutManager *) layoutManager;

/**
This method should not be called directly. It is called by the layout manager
when the layout manager of a text container changes.
*/
- (void) setLayoutManager: (GSLayoutManager *)aLayoutManager;


/**
Sets the NSTextView for this text container. Note that a text view
should be attached to a text container only if the text container is attached
to a layout manager that can handle text views (eg. NSLayoutManager).

The text view is retained by the text container.
*/
- (void) setTextView: (NSTextView *)aTextView;

/**
Returns the NSTextView attached to this text container, or nil if there is
none.
*/
- (NSTextView *) textView;


/**
The container size<br />

A text container has a container size. The region defined by the text
container must be a subset of the rectangle of this size with it's top-left
corner in the origin.

Subclasses do not have to support arbitrary sizes, and may choose to ignore
-setContainerSize: completely. However, the size returned by -containerSize
must be valid.
*/
- (void) setContainerSize: (NSSize)aSize;
- (NSSize) containerSize;


/**
Automatic resizing<br />

A text container can be set to automatically track the width and/or height
of its NSTextView (TODO: frame? bounds?). For more information, see the
documentation on automatic resizing in NSTextView. (TODO: link)

When enabled, the automatic resizing is done by calling
[NSTextContainer-setContainerSize:] with the new size whenever the text view
is resized.
*/
- (void) setWidthTracksTextView: (BOOL)flag;
- (BOOL) widthTracksTextView;
- (void) setHeightTracksTextView: (BOOL)flag;
- (BOOL) heightTracksTextView;


/**
Line fragment padding<br />

The line fragment padding is an amount of space left empty at each end of
a line fragment rectangle by the standard typesetter. The default is 0.0.
*/
- (void) setLineFragmentPadding: (CGFloat)aFloat;
- (CGFloat) lineFragmentPadding;

@end

#else
#error "The OpenStep specification does not define an NSTextContainer class."
#endif

#endif /* _GNUstep_H_NSTextContainer */
