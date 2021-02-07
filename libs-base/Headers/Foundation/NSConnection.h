/* Interface for GNU Objective-C version of NSConnection
   Copyright (C) 1997,2000 Free Software Foundation, Inc.
   
   Original by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Version for OPENSTEP by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: August 1997, updated June 2000
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */ 

#ifndef __NSConnection_h_GNUSTEP_BASE_INCLUDE
#define __NSConnection_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>
#import	<Foundation/NSDictionary.h>
#import	<Foundation/NSString.h>
#import	<Foundation/NSTimer.h>
#import	<Foundation/NSRunLoop.h>
#import	<Foundation/NSMapTable.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSDistantObject;
@class NSPort;
@class NSPortNameServer;
@class NSData;
@class NSInvocation;

/*
 *	Keys for the NSDictionary returned by [NSConnection -statistics]
 */
/* These in OPENSTEP 4.2 */
/**
 *  Key for dictionary returned by [NSConnection-statistics]: number of
 *  messages replied to so far by the remote connection.
 */
GS_EXPORT NSString* const NSConnectionRepliesReceived;

/**
 *  Key for dictionary returned by [NSConnection-statistics]: number of
 *  messages sent so far to the remote connection.
 */
GS_EXPORT NSString* const NSConnectionRepliesSent;

/**
 *  Key for dictionary returned by [NSConnection-statistics]: number of
 *  messages received so far from the remote connection.
 */
GS_EXPORT NSString* const NSConnectionRequestsReceived;

/**
 *  Key for dictionary returned by [NSConnection-statistics]: number of
 *  messages sent so far to the remote connection.
 */
GS_EXPORT NSString* const NSConnectionRequestsSent;
/* These Are GNUstep extras */

/**
 *  GNUstep-specific key for dictionary returned by [NSConnection-statistics]:
 *  number of local objects currently in use remotely.
 */
GS_EXPORT NSString* const NSConnectionLocalCount;	/* Objects sent out */

/**
 *  GNUstep-specific key for dictionary returned by [NSConnection-statistics]:
 *  number of remote objects currently in use.
 */
GS_EXPORT NSString* const NSConnectionProxyCount;	/* Objects received */



/*
 *	NSConnection class interface.
 *
 *	A few methods are in the specification but not yet implemented.
 */
GS_EXPORT_CLASS
@interface NSConnection : NSObject
{
#if	GS_NONFRAGILE
#  if	defined(GS_NSConnection_IVARS)
@public
GS_NSConnection_IVARS;
#  endif
#else
@private id _internal;
#endif
}

+ (NSArray*) allConnections;
+ (NSConnection*) connectionWithReceivePort: (NSPort*)r
                                   sendPort: (NSPort*)s;
+ (NSConnection*) connectionWithRegisteredName: (NSString*)n
                                          host: (NSString*)h;
+ (NSConnection*) connectionWithRegisteredName: (NSString*)n
                                          host: (NSString*)h
			       usingNameServer: (NSPortNameServer*)s;
+ (id) currentConversation;
+ (NSConnection*) defaultConnection;
+ (NSDistantObject*) rootProxyForConnectionWithRegisteredName: (NSString*)n
                                                         host: (NSString*)h;
+ (NSDistantObject*) rootProxyForConnectionWithRegisteredName: (NSString*)n
  host: (NSString*)h usingNameServer: (NSPortNameServer*)s;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) 
+ (id) serviceConnectionWithName: (NSString *)name
                      rootObject: (id)root;
+ (id) serviceConnectionWithName: (NSString *)name
                      rootObject: (id)root
                 usingNameServer: (NSPortNameServer *)server;
#endif

- (void) addRequestMode: (NSString*)mode;
- (void) addRunLoop: (NSRunLoop*)loop;
- (id) delegate;
- (void) enableMultipleThreads;
- (BOOL) independentConversationQueueing;
- (id) initWithReceivePort: (NSPort*)r
		  sendPort: (NSPort*)s;
- (void) invalidate;
- (BOOL) isValid;
- (NSArray*)localObjects;
- (BOOL) multipleThreadsEnabled;
- (NSPort*) receivePort;
- (BOOL) registerName: (NSString*)name;
- (BOOL) registerName: (NSString*)name withNameServer: (NSPortNameServer*)svr;
- (NSArray*) remoteObjects;
- (void) removeRequestMode: (NSString*)mode;
- (void) removeRunLoop: (NSRunLoop *)loop;
- (NSTimeInterval) replyTimeout;
- (NSArray*) requestModes;
- (NSTimeInterval) requestTimeout;
- (id) rootObject;
- (NSDistantObject*) rootProxy;
- (void) runInNewThread;
- (NSPort*) sendPort;
- (void) setDelegate: anObj;
- (void) setIndependentConversationQueueing: (BOOL)flag;
- (void) setReplyTimeout: (NSTimeInterval)to;
- (void) setRequestMode: (NSString*)mode;
- (void) setRequestTimeout: (NSTimeInterval)to;
- (void) setRootObject: anObj;
- (NSDictionary*) statistics;
@end


/**
 * This category represents an informal protocol to which NSConnection
 * delegates may conform ... These methods are not actually implemented
 * by NSObject, so implementing these methods in your class has the effect
 * documented.
 */
@interface	NSObject (NSConnectionDelegate)

/**
 * <p>
 *   This is not an NSConnection method, but is a method that may
 *   be implemented by the delegate of an NSConnection object.
 * </p>
 * <p>
 *   If the delegate implements this method, the NSConnection will
 *   invoke the method for every message request or reply it receives
 *   from the remote NSConnection.  The delegate should use the
 *   authentication data to check all the NSData objects
 *   in the components array (ignoring NSPort objects),
 *   and return YES if they are valid, NO otherwise.
 * </p>
 * <p>
 *   If the method returns NO then an
 *   NSFailedAuthentication exception will be raised.
 * </p>
 * <p>
 *   In GNUstep the components array is mutable, allowing
 *   you to replace the NSData objects with your own version.
 * </p>
 */
- (BOOL) authenticateComponents: (NSMutableArray*)components
		       withData: (NSData*)authenticationData;

/**
 * <p>
 *   This is not an NSConnection method, but is a method that may
 *   be implemented by the delegate of an NSConnection object.
 * </p>
 * <p>
 *   If the delegate implements this method, the NSConnection will
 *   invoke the method for every message request to reply it sends
 *   to the remote NSConnection.  The delegate should generate
 *   authentication data by examining all the NSData objects
 *   in the components array (ignoring NSPort objects),
 *   and return the authentication data that can be used by the
 *   remote NSConnection.
 * </p>
 * <p>
 *   If the method returns nil then an
 *   NSGenericException exception will be raised.
 * </p>
 * <p>
 *   In GNUstep the components array is mutable, allowing
 *   you to replace the NSData objects with your own version.
 * </p>
 */
- (NSData*) authenticationDataForComponents: (NSMutableArray*)components;

/**
 * <p>
 *   This is not an NSConnection method, but is a method that may
 *   be implemented by the delegate of an NSConnection object.
 * </p>
 * <p>
 *   If the delegate implements this method, it will be called
 *   whenever a new NSConnection is created that has this
 *   NSConnection as its parent.  The delegate may take this
 *   opportunity to adjust the configuration of the new
 *   connection and may return a boolean value to tell the
 *   parent whether the creation of the new connection is to
 *   be permitted or not.
 * </p>
 */
 - (BOOL) connection: (NSConnection*)parent
  shouldMakeNewConnection: (NSConnection*)newConnection;

- (NSConnection*) connection: (NSConnection*)ancestorConn
		  didConnect: (NSConnection*)newConn;

/**
 * An old fashioned synonym for -connection:shouldMakeNewConnection: -
 * don't use this.
 */
- (BOOL) makeNewConnection: (NSConnection*)newConnection
                    sender: (NSConnection*)parent;
@end

/*
 *	NSRunLoop mode, NSNotification name and NSException strings.
 */

/**
 * [NSRunLoop] mode for [NSConnection] objects waiting for replies.
 * Mainly used internally by distributed objects system.
 */
GS_EXPORT NSString * const NSConnectionReplyMode;

/**
 * Posted when an [NSConnection] is deallocated or it is notified its port is
 * deactivated.  (Note, connections to remote ports don't get such a
 * notification.)  Receivers should deregister themselves for notifications
 * from the given connection.
 */
GS_EXPORT NSString * const NSConnectionDidDieNotification;

/**
 * Posted when an [NSConnection] is initialized.
 */
GS_EXPORT NSString * const NSConnectionDidInitializeNotification; /* OPENSTEP */

/**
 * Raised by an [NSConnection] on receiving a message that it or its delegate
 * cannot authenticate.
 */
GS_EXPORT NSString * const NSFailedAuthenticationException; /* MacOS-X  */

#if	defined(__cplusplus)
}
#endif

#endif /* __NSConnection_h_GNUSTEP_BASE_INCLUDE */
