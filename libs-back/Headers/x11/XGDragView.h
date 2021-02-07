/* -*- mode: ObjC -*-
  <title>XGDragView</title>

   <abstract>View that is dragged during drag and drop</abstract>

   Copyright (C) 2001-2005 Free Software Foundation, Inc.

   Written By: <author name="Wim Oudshoorn"><email>woudshoo@xs4all.nl</email></author>
   Date: Nov 2001
   
   This file is part of the GNU Objective C User Interface library.

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

#include <AppKit/NSCell.h>
#include <AppKit/NSEvent.h>
#include <GNUstepGUI/GSDragView.h>
#include <Foundation/NSGeometry.h>
#include "x11/xdnd.h"
#include "x11/XGServerWindow.h"

/*"
  Drag and drop support functions
  "*/

void 		GSEnsureDndIsInitialized (void);
DndClass 	xdnd (void);
Atom		GSActionForDragOperation(unsigned int op);
NSDragOperation	GSDragOperationForAction(Atom xaction);


/*"
  WRO (notes made 18 Nov 2001)
  
  The object that is dragged over the screen.  
  It hijacks the event loop and manages the complete
  drag and drop sequence.
 "*/
@interface	XGDragView : GSDragView
{
  Atom           *typelist;
}

+ (id) sharedDragView;

- (void) setupDragInfoFromXEvent: (XEvent *)xEvent;
- (void) updateDragInfoFromEvent: (NSEvent *)event;
- (void) resetDragInfo;
- (void) dragImage: (NSImage*)anImage
		at: (NSPoint)screenLocation
	    offset: (NSSize)initialOffset
	     event: (NSEvent*)event
	pasteboard: (NSPasteboard*)pboard
	    source: (id)sourceObject
	 slideBack: (BOOL)slideFlag;
- (Window) _xWindowAcceptingDnDunderX: (int) x Y: (int) y;
@end
