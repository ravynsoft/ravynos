/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/
#import "common.h"

#if	defined(NeXT_Foundation_LIBRARY)

#import "Foundation/NSByteOrder.h"
#import "Foundation/NSHost.h"
#import "GSNetwork.h"
#import "GSPrivate.h"
#import "GNUstepBase/NSFileHandle+GNUstepBase.h"

/* Not defined on Solaris 2.7 */
#ifndef INADDR_NONE
#define INADDR_NONE     -1
#endif

@implementation NSFileHandle(GNUstepBase)
// From GSFileHandle.m

static BOOL
getAddr(NSString* name, NSString* svc, NSString* pcl, struct addrinfo **ai, struct addrinfo *hints)
{
  const char        *cHostn = NULL;
  const char        *cPortn = NULL;
  int                e = 0;

  if (!svc)
    {
      NSLog(@"service is nil.");
      
      return NO;
    }

  hints->ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
  hints->ai_protocol = IPPROTO_IP; // accept any

  if (pcl)
    {
      if ([pcl isEqualToString:@"tcp"])
	{
	  hints->ai_protocol = IPPROTO_TCP;
	  hints->ai_socktype = SOCK_STREAM;
	}
      else if ([pcl isEqualToString:@"udp"])
	{
	  hints->ai_protocol = IPPROTO_UDP;
	} 
    }

  /*
   *    If we were given a hostname, we use any address for that host.
   *    Otherwise we expect the given name to be an address unless it  is
   *    a null (any address).
   */
  if (name)
    {
      NSHost*        host = [NSHost hostWithName: name];
      
      if (host != nil)
	{
	  name = [host address];
	  NSLog(@"host address '%@'", name);
	  cHostn = [name cStringUsingEncoding:NSASCIIStringEncoding];
	}
    }
  
  cPortn = [svc cStringUsingEncoding:NSASCIIStringEncoding];
  
  // getaddrinfo() returns zero on success or one of the error codes listed in
  // gai_strerror(3) if an error occurs.
  NSLog(@"cPortn '%s'", cPortn);
                                           //&ai
  e = getaddrinfo (cHostn, cPortn, hints, ai);

  if (e != 0)
    {
      NSLog(@"getaddrinfo: %s", gai_strerror (e));
      return NO;
    }

  return YES;
}

- (id) initAsServerAtAddress: (NSString*)a
		     service: (NSString*)s
		    protocol: (NSString*)p
{
#ifndef    BROKEN_SO_REUSEADDR
  int    status = 1;
#endif
  int    net;
  struct addrinfo *ai;
  struct addrinfo hints;
  memset (&hints, '\0', sizeof (hints));

  if (getAddr(a, s, p, &ai, &hints) == NO)
    {
      DESTROY(self);
      NSLog(@"bad address-service-protocol combination");
      return  nil;
    }

  if ((net = socket (ai->ai_family, ai->ai_socktype,
                     ai->ai_protocol)) < 0)
    {
      NSLog(@"unable to create socket ai_family: %@ socktype:%@ protocol:%d - %@", (ai->ai_family == PF_INET6 ? @"PF_INET6":@"PF_INET"),
            (ai->ai_socktype == SOCK_STREAM ? @"SOCK_STREAM":@"whatever"),
            ai->ai_protocol,            
            [NSError _last]);
      DESTROY(self);
      return nil;
    }

#ifndef    BROKEN_SO_REUSEADDR
  /*
   * Under decent systems, SO_REUSEADDR means that the port can be  reused
   * immediately that this process exits.  Under some it means
   * that multiple processes can serve the same port simultaneously.
   * We don't want that broken behavior!
   */
  setsockopt(net, SOL_SOCKET, SO_REUSEADDR, (char *)&status,  sizeof(status));
#endif

  if (bind(net, ai->ai_addr, ai->ai_addrlen) != 0)
    {
      NSLog(@"unable to bind to port %@", [NSError _last]);
      goto cleanup;
    }

  if (listen(net, 5) < 0)
    {
      NSLog(@"unable to listen on port - %@",  [NSError _last]);
      goto cleanup;
    }

  // 	struct sockaddr_storeage sstore;
  // 	int slen = sizeof(ss);


//  if (getsockname(net,(struct sockaddr *)&sstore, &slen) < 0)
//    {
//      NSLog(@"unable to get socket name - %@",  [NSError _last]);
//      goto cleanup;
//    }

  freeaddrinfo (ai);
  
  self = [self initWithFileDescriptor: net closeOnDealloc: YES];

  return self;
  
cleanup:
  (void) close(net);
  freeaddrinfo (ai);
  DESTROY(self);
  
  return nil;
}

+ (id) fileHandleAsServerAtAddress: (NSString*)address
                           service: (NSString*)service
                          protocol: (NSString*)protocol
{
  id    o = [self allocWithZone: NSDefaultMallocZone()];

  return AUTORELEASE([o initAsServerAtAddress: address
                                      service: service
                                     protocol: protocol]);
}

- (NSString*) socketAddress
{
  struct sockaddr_storage    sstore;
  struct sockaddr            *sadr;
  
  socklen_t    size = sizeof(sstore);
  
  if (getsockname([self fileDescriptor], (struct sockaddr*)&sstore,  &size) < 0)
    {
      NSLog(@"unable to get socket name - %@",  [NSError _last]);
      return nil;
    }
  
  sadr = (struct sockaddr *) &sstore;
  
  switch (sadr->sa_family)
    {
      case AF_INET6:
	{
	  char straddr[INET6_ADDRSTRLEN];
	  struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&sstore;
	  
	  inet_ntop(AF_INET6, &(addr6->sin6_addr), straddr, 
		    sizeof(straddr));
	  
	  return [NSString stringWithCString: straddr 
				    encoding: NSASCIIStringEncoding];
	  break;
	}
      case AF_INET:
	{
	  
	  struct sockaddr_in * addr4 = (struct sockaddr_in*) &sstore;
	  
	  char *address = inet_ntoa(addr4->sin_addr);
	  
	  return [NSString stringWithCString: address 
				    encoding: NSASCIIStringEncoding];
	  break;
	} 
      default:
	break;
    }
  
  return nil;
}

@end

#endif	/* defined(NeXT_Foundation_LIBRARY) */

