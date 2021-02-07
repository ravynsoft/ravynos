/*
   GSDragView - Generic Drag and Drop code.

   Copyright (C) 2004 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: May 2004

   Based on X11 specific code from:
   Created by: Wim Oudshoorn <woudshoo@xs4all.nl>
   Date: Nov 2001
   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1998

   This file is part of the GNU Objective C User Interface Library.

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
#ifndef _GSDragView_h_INCLUDE
#define _GSDragView_h_INCLUDE

#import <Foundation/NSDate.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSDragging.h>
#import <AppKit/NSView.h>

@class NSMutableDictionary;

@class NSCell;
@class NSEvent;
@class NSImage;
@class NSPasteboard;
@class NSWindow;

/*
 * used in the operation mask to indicate that the
 * user can not change the drag action by
 * pressing modifiers.
 */

#define NSDragOperationIgnoresModifiers  0xffff

@interface GSDragView : NSView <NSDraggingInfo>
{
  // the graphics that is dragged
  NSCell	*dragCell;

  // the pasteboard with the dragged data
  NSPasteboard	*dragPasteboard;

  // NSWindow in this application that is the current target
  NSWindow	*destWindow;

  // Screen coordinates of mouse pointer, only valid when destWindow != nil
  NSPoint	dragPoint;

  NSInteger	dragSequence;

  // the source of the dragging operation
  id		dragSource;

  // Operations supported by the source
  NSDragOperation dragMask;

  /* User specified operation mask (key modifiers).
   * This is either a mask of type _NSDragOperation,
   * or NSDragOperationIgnoresModifiers, which
   * is defined as 0xffff
   */
  NSDragOperation operationMask;

  // slide the image back when drag fails?
  BOOL		slideBack;

  /* The following information used in the drag and drop event loop
   */

  // offset of image w.r.t. cursor
  NSSize	offset;

  // current drag (mouse cursor) position in screen coordinates
  NSPoint	dragPosition;

  // drag (mouse cursor) position, not yet processed
  NSPoint	newPosition;

  // OS specific current window target of the drag operation
  int		targetWindowRef;

  // Operations supported by the target, only valid if targetWindowRef isn't 0
  NSDragOperation targetMask;

  // YES if target and source are in a different application
  BOOL		destExternal;

  // YES if we are currently dragging
  BOOL		isDragging;

  // Cache for cursors
  NSMutableDictionary	*cursors;
}

+ (id) sharedDragView;
- (void) dragImage: (NSImage*)anImage
		at: (NSPoint)screenLocation
	    offset: (NSSize)initialOffset
	     event: (NSEvent*)event
	pasteboard: (NSPasteboard*)pboard
	    source: (id)sourceObject
         slideBack: (BOOL)slideFlag;
- (void) postDragEvent: (NSEvent*)theEvent;
- (void) sendExternalEvent: (GSAppKitSubtype)subtype
		    action: (NSDragOperation)action
		  position: (NSPoint)eventLocation
		 timestamp: (NSTimeInterval)time
		  toWindow: (int)dWindowNumber;
- (NSWindow*) windowAcceptingDnDunder: (NSPoint)mouseLocation
			    windowRef: (int*)mouseWindowRef;

@end
#endif
