/* Include for GNUstep Distributed NotificationCenter
   Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1998

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYING.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   */

#define	GDNC_SERVICE	@"GDNCServer"
#define	GDNC_NETWORK	@"GDNCNetwork"

@protocol	GDNCClient
- (oneway void) postNotificationName: (NSString*)name
			      object: (NSString*)object
			    userInfo: (NSData*)info
			    selector: (NSString*)aSelector
				  to: (uint64_t)observer;
@end

@protocol	GDNCProtocol
- (void) addObserver: (uint64_t)anObserver
	    selector: (NSString*)aSelector
	        name: (NSString*)notificationname
	      object: (NSString*)anObject
  suspensionBehavior: (NSNotificationSuspensionBehavior)suspensionBehavior
		 for: (id<GDNCClient>)client;

- (oneway void) postNotificationName: (NSString*)notificationName
			      object: (NSString*)anObject
			    userInfo: (NSData*)d
		  deliverImmediately: (BOOL)deliverImmediately
			         for: (id<GDNCClient>)client;

- (void) registerClient: (id<GDNCClient>)client;

- (void) removeObserver: (uint64_t)anObserver
		   name: (NSString*)notificationname
		 object: (NSString*)anObject
		    for: (id<GDNCClient>)client;

- (void) setSuspended: (BOOL)flag
		  for: (id<GDNCClient>)client;

- (void) unregisterClient: (id<GDNCClient>)client;

@end

