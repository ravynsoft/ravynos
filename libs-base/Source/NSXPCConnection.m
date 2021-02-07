/* Implementation of class NSXPCConnection
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

#import "Foundation/NSXPCConnection.h"
#import "GNUstepBase/NSObject+GNUstepBase.h"

@implementation NSXPCConnection

- (instancetype) initWithServiceName:(NSString *)serviceName
{
  return [self notImplemented: _cmd];
}

- (NSString *) serviceName
{
  return [self notImplemented: _cmd];
}

- (void) setServiceName: (NSString *)serviceName
{
  [self notImplemented: _cmd];
}

- (instancetype) initWithMachServiceName: (NSString *)name
				 options: (NSXPCConnectionOptions)options
{
  return [self notImplemented: _cmd];
}

- (instancetype) initWithListenerEndpoint: (NSXPCListenerEndpoint *)endpoint
{
  return [self notImplemented: _cmd];
}


- (NSXPCListenerEndpoint *) endpoint
{
  return [self notImplemented: _cmd];
}

- (void) setEndpoint: (NSXPCListenerEndpoint *) endpoint
{
  [self notImplemented: _cmd];
}

- (NSXPCInterface *) exportedInterface
{
  return [self notImplemented: _cmd];
}

- (void) setExportInterface: (NSXPCInterface *)exportedInterface
{
  [self notImplemented: _cmd];
}

- (NSXPCInterface *) remoteObjectInterface
{
  return [self notImplemented: _cmd];
}

- (void) setRemoteObjectInterface: (NSXPCInterface *)remoteObjectInterface
{
  [self notImplemented: _cmd];
}

- (id) remoteObjectProxy
{
  return [self notImplemented: _cmd];
}

- (void) setRemoteObjectProxy: (id)remoteObjectProxy
{
  [self notImplemented: _cmd];
}

- (id) remoteObjectProxyWithErrorHandler:(GSXPCProxyErrorHandler)handler
{
  return [self notImplemented: _cmd];
}

- (id) synchronousRemoteObjectProxyWithErrorHandler:
  (GSXPCProxyErrorHandler)handler
{
  return [self notImplemented: _cmd];
}

- (GSXPCInterruptionHandler) interruptionHandler 
{
  return NULL;
}

- (void) setInterruptionHandler: (GSXPCInterruptionHandler)handler
{
  [self notImplemented: _cmd];
}

- (GSXPCInvalidationHandler) invalidationHandler 
{
  return NULL;
}

- (void) setInvalidationHandler: (GSXPCInvalidationHandler)handler
{
  [self notImplemented: _cmd];
}

- (void) resume
{
  [self notImplemented: _cmd];
}

- (void) suspend
{
  [self notImplemented: _cmd];
}

- (void) invalidate
{
  [self notImplemented: _cmd];
}

- (NSUInteger) auditSessionIdentifier
{
  return (NSUInteger)[self notImplemented: _cmd];
}
- (pid_t) processIdentifier
{
  return (pid_t)(uintptr_t)[self notImplemented: _cmd];
}
- (uid_t) effectiveUserIdentifier
{
  return (uid_t)(uintptr_t)[self notImplemented: _cmd];
}
- (gid_t) effectiveGroupIdentifier
{
  return (gid_t)(uintptr_t)[self notImplemented: _cmd];
}
@end

@implementation NSXPCListener

+ (NSXPCListener *) serviceListener
{
  return [self notImplemented: _cmd];
}

+ (NSXPCListener *) anonymousListener
{
  return [self notImplemented: _cmd];
}

- (instancetype) initWithMachServiceName:(NSString *)name
{
  return [self notImplemented: _cmd];
}

- (id <NSXPCListenerDelegate>) delegate
{
  return [self notImplemented: _cmd];
}

- (void) setDelegate: (id <NSXPCListenerDelegate>) delegate
{
  [self notImplemented: _cmd];
}

- (NSXPCListenerEndpoint *) endpoint
{
  return [self notImplemented: _cmd];
}

- (void) setEndpoint: (NSXPCListenerEndpoint *)endpoint
{
  [self notImplemented: _cmd];
}

- (void) resume
{
  [self notImplemented: _cmd];
}

- (void) suspend
{
  [self notImplemented: _cmd];
}

- (void) invalidate
{
  [self notImplemented: _cmd];
}

@end

@implementation NSXPCInterface

+ (NSXPCInterface *) interfaceWithProtocol: (Protocol *)protocol
{
  return [self notImplemented: _cmd];
}

- (Protocol *) protocol
{
  return [self notImplemented: _cmd];
}

- (void) setProtocol: (Protocol *)protocol
{
  [self notImplemented: _cmd];
}

- (void) setClasses: (NSSet *)classes
	forSelector: (SEL)sel
      argumentIndex: (NSUInteger)arg
	    ofReply: (BOOL)ofReply
{
  [self notImplemented: _cmd];
}

- (NSSet *) classesForSelector: (SEL)sel
		 argumentIndex: (NSUInteger)arg
		       ofReply: (BOOL)ofReply
{
  return [self notImplemented: _cmd];
}

- (void) setInterface: (NSXPCInterface *)ifc
	  forSelector: (SEL)sel
	argumentIndex: (NSUInteger)arg
	      ofReply: (BOOL)ofReply
{
  [self notImplemented: _cmd];
}

- (NSXPCInterface *) interfaceForSelector: (SEL)sel
			    argumentIndex: (NSUInteger)arg
				  ofReply: (BOOL)ofReply
{
  return [self notImplemented: _cmd];
}

@end

@implementation NSXPCListenerEndpoint

- (instancetype) initWithCoder: (NSCoder *)coder
{
  return [self notImplemented: _cmd];
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [self notImplemented: _cmd];
}

@end

