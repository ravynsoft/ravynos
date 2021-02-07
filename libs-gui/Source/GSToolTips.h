/**
   Interface of the GSToolTips class

   Copyright (C) 2006 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 2006
   
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

#import <Foundation/NSObject.h>
#import "GNUstepGUI/GSTrackingRect.h"

@class	NSTimer;
@class	NSView;
@class	NSWindow;

@interface	GSToolTips : NSObject
{
  NSView		*view;
  NSTrackingRectTag	toolTipTag;
}

/** Destroy object handling tips for aView.
 */
+ (void) removeTipsForView: (NSView*)aView;

/** Return object to handle tips for aView.
 */
+ (GSToolTips*) tipsForView: (NSView*)aView;

/** <init/> Initialiser for internal use only.
 */
- (id) initForView: (NSView*)aView;

/** Support [NSView-addToolTipRect:owner:userData:].
 */
- (NSToolTipTag) addToolTipRect: (NSRect)aRect
                          owner: (id)anObject
                       userData: (void *)data;
/**
 * Removes all of the tool tips in a given rectangle.
 */
- (void)removeToolTipsInRect: (NSRect)aRect;

/** Return the number of tooltip rectangles active.
 */
- (unsigned) count;

/** Handle mouse entry to tracking rect
 */
- (void) mouseEntered: (NSEvent *)theEvent;

/** Handle mouse exit from tracking rect
 */
- (void) mouseExited: (NSEvent *)theEvent;

/** Cancel tooltip because user clicked mouse.
 */
- (void) mouseDown: (NSEvent *)theEvent;

/** Move tooltip window with user's mouse movement.
 */
- (void) mouseMoved: (NSEvent *)theEvent;

/** Support [NSView-removeAllToolTips]
 */
- (void) removeAllToolTips;

/** Support [NSView-removeToolTip:].
 */
- (void) removeToolTip: (NSToolTipTag)tag;

/** Support [NSView-setToolTip:].
 */
- (void) setToolTip: (NSString *)string;

/** Support [NSView-toolTip].
 */
- (NSString *) toolTip;
@end

