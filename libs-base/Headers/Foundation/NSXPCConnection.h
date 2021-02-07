/* Definition of class NSXPCConnection
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Tue Nov 12 23:50:29 EST 2019

   This file is part of the GNUstep Library.
   
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

#ifndef _NSXPCConnection_h_GNUSTEP_BASE_INCLUDE
#define _NSXPCConnection_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>
#include <Foundation/NSError.h>

#include <sys/types.h> // for gid_t and uid_t

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)

#if defined(_WIN32)
#if defined(_MSC_VER)
typedef unsigned pid_t;
#endif
typedef unsigned gid_t;
typedef unsigned uid_t;
#endif

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSMutableDictionary, NSString, NSOperationQueue, NSSet, NSLock, NSError;
@class NSXPCConnection, NSXPCListener, NSXPCInterface, NSXPCListenerEndpoint;
@protocol NSXPCListenerDelegate;

DEFINE_BLOCK_TYPE(GSXPCProxyErrorHandler, void, NSError *);
DEFINE_BLOCK_TYPE_NO_ARGS(GSXPCInterruptionHandler, void);
DEFINE_BLOCK_TYPE_NO_ARGS(GSXPCInvalidationHandler, void);


@protocol NSXPCProxyCreating

- (id) remoteObjectProxy;

- (id) remoteObjectProxyWithErrorHandler: (GSXPCProxyErrorHandler)handler;

- (id) synchronousRemoteObjectProxyWithErrorHandler: (GSXPCProxyErrorHandler)handler;

@end

enum
{
    NSXPCConnectionPrivileged = (1 << 12UL)
};
typedef NSUInteger NSXPCConnectionOptions; 
  
GS_EXPORT_CLASS
@interface NSXPCConnection : NSObject <NSXPCProxyCreating>

- (instancetype) initWithListenerEndpoint: (NSXPCListenerEndpoint *)endpoint;

- (instancetype) initWithMachServiceName: (NSString *)name
				 options: (NSXPCConnectionOptions)options;

- (instancetype) initWithServiceName:(NSString *)serviceName;
  
- (NSXPCListenerEndpoint *) endpoint;
- (void) setEndpoint: (NSXPCListenerEndpoint *) endpoint;
  
- (NSXPCInterface *) exportedInterface;
- (void) setExportInterface: (NSXPCInterface *)exportedInterface;
  
- (NSXPCInterface *) remoteObjectInterface;
- (void) setRemoteObjectInterface: (NSXPCInterface *)remoteObjectInterface;


- (id) remoteObjectProxy;
- (void) setRemoteObjectProxy: (id)remoteObjectProxy;

- (id) remoteObjectProxyWithErrorHandler:(GSXPCProxyErrorHandler)handler;

- (NSString *) serviceName;
- (void) setServiceName: (NSString *)serviceName;
  
- (id) synchronousRemoteObjectProxyWithErrorHandler:
  (GSXPCProxyErrorHandler)handler;

- (GSXPCInterruptionHandler) interruptionHandler; 
- (void) setInterruptionHandler: (GSXPCInterruptionHandler)handler;
  
- (GSXPCInvalidationHandler) invalidationHandler; 
- (void) setInvalidationHandler: (GSXPCInvalidationHandler)handler;
  
- (void) resume;

- (void) suspend;

- (void) invalidate;

- (NSUInteger) auditSessionIdentifier;
- (pid_t) processIdentifier;
- (uid_t) effectiveUserIdentifier;
- (gid_t) effectiveGroupIdentifier;

@end


@interface NSXPCListener : NSObject

+ (NSXPCListener *) serviceListener;

+ (NSXPCListener *) anonymousListener;

- (instancetype) initWithMachServiceName:(NSString *)name;

- (id <NSXPCListenerDelegate>) delegate;
- (void) setDelegate: (id <NSXPCListenerDelegate>) delegate;

- (NSXPCListenerEndpoint *) endpoint;
- (void) setEndpoint: (NSXPCListenerEndpoint *)endpoint;
  
- (void) resume;

- (void) suspend;

- (void) invalidate;

@end

@protocol NSXPCListenerDelegate <NSObject>

- (BOOL) listener: (NSXPCListener *)listener
  shouldAcceptNewConnection: (NSXPCConnection *)newConnection;

@end

@interface NSXPCInterface : NSObject

+ (NSXPCInterface *) interfaceWithProtocol: (Protocol *)protocol;

- (Protocol *) protocol;
- (void) setProtocol: (Protocol *)protocol;

- (void) setClasses: (NSSet *)classes
	forSelector: (SEL)sel
      argumentIndex: (NSUInteger)arg
	    ofReply: (BOOL)ofReply;

- (NSSet *) classesForSelector: (SEL)sel
		 argumentIndex: (NSUInteger)arg
		       ofReply: (BOOL)ofReply;

- (void) setInterface: (NSXPCInterface *)ifc
	  forSelector: (SEL)sel
	argumentIndex: (NSUInteger)arg
	      ofReply: (BOOL)ofReply;

- (NSXPCInterface *) interfaceForSelector: (SEL)sel
			    argumentIndex: (NSUInteger)arg
				  ofReply: (BOOL)ofReply;

@end

GS_EXPORT_CLASS
@interface NSXPCListenerEndpoint : NSObject <NSCoding>  // NSSecureCoding
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSXPCConnection_h_GNUSTEP_BASE_INCLUDE */
