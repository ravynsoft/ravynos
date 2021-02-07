/*                                                    -*-objc-*-
   NSInputServer.h

   Copyright (C) 2001 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: August 2001

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

#ifndef _GNUstep_H_NSInputServer
#define _GNUstep_H_NSInputServer
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

@class NSString;

@protocol NSInputServerMouseTracker
- (BOOL) mouseDownOnCharacterIndex: (unsigned)index
		      atCoordinate: (NSPoint)point
		      withModifier: (unsigned int)flags
			    client: (id)sender;
- (BOOL) mouseDraggedOnCharacterIndex: (unsigned)index
			 atCoordinate: (NSPoint)point
			 withModifier: (unsigned int)flags
			       client: (id)sender;
- (void) mouseUpOnCharacterIndex: (unsigned)index
		    atCoordinate: (NSPoint)point
		    withModifier: (unsigned int)flags
			  client: (id)sender;
@end

@protocol NSInputServiceProvider
- (void) activeConversationChanged: (id)sender
		 toNewConversation: (long)newConversation;
- (void) activeConversationWillChange: (id)sender
		  fromOldConversation: (long)oldConversation;
- (BOOL) canBeDisabled;
- (void) doCommandBySelector: (SEL)aSelector
		      client: (id)sender;
- (void) inputClientBecomeActive: (id)sender;
- (void) inputClientDisabled: (id)sender;
- (void) inputClientEnabled: (id)sender;
- (void) inputClientResignActive: (id)sender;
- (void) insertText: (id)aString
	     client: (id)sender;
- (void) markedTextAbandoned: (id)sender;
- (void) markedTextSelectionChanged: (NSRange)newSelection
			     client: (id)sender;
- (void) terminate: (id)sender;
- (BOOL) wantsToDelayTextChangeNotifications;
- (BOOL) wantsToHandleMouseEvents;
- (BOOL) wantsToInterpretAllKeystrokes;
@end

@interface NSInputServer: NSObject <NSInputServerMouseTracker, NSInputServiceProvider>

- (id) initWithDelegate: (id)aDelegate
		   name: (NSString *)name;
@end

#endif //_GNUstep_H_NSInputServer
