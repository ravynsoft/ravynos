/* Implementation of NSSocketPortNameServer class for Distributed Objects
   Copyright (C) 1998,1999,2000 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1998

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

   $Date$ $Revision$
   */

/* defines to get system-v functions including inet_aton()
 * The first define is for old versions of glibc, the second for newer ones
 */
#define _SVID_SOURCE    1
#define _DEFAULT_SOURCE    1

#import "common.h"
#define	EXPOSE_NSSocketPortNameServer_IVARS	1
#import "Foundation/NSData.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSException.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSFileHandle.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSNotificationQueue.h"
#import "Foundation/NSPort.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSHost.h"
#import "Foundation/NSTask.h"
#import "GNUstepBase/NSTask+GNUstepBase.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSTimer.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSPortNameServer.h"

#import "GSPortPrivate.h"

#ifdef _WIN32
#include <winsock2.h>
#include <wininet.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

/*
 *	Protocol definition stuff for talking to gdomap process.
 */
#include        "../Tools/gdomap.h"

#define stringify_it(X) #X
#define	make_gdomap_port(X)	stringify_it(X)

/*
 * to suppress warnings about using private methods.
 */
@class	NSSocketPort;
@interface NSSocketPort (Hack)
+ (NSSocketPort*) portWithNumber: (uint16_t)number
		       onHost: (NSHost*)host
		 forceAddress: (NSString*)addr
		     listener: (BOOL)shouldListen;
- (uint16_t) portNumber;
@end

/*
 * class-wide variables.
 */
static unsigned		maxHandles = 4;
static NSTimeInterval	timeout = 20.0;
static NSString		*serverPort = @"gdomap";
static NSString		*mode = @"NSPortServerLookupMode";
static NSArray		*modes = nil;
static NSRecursiveLock	*serverLock = nil;
static NSSocketPortNameServer *defaultServer = nil;
static NSString		*launchCmd = nil;
static Class		portClass = 0;



typedef enum {
  GSPC_NONE,
  GSPC_LOPEN,
  GSPC_ROPEN,
  GSPC_RETRY,
  GSPC_WRITE,
  GSPC_READ1,
  GSPC_READ2,
  GSPC_FAIL,
  GSPC_DONE
} GSPortComState;

@interface	GSPortCom : NSObject
{
  gdo_req		msg;
  unsigned		expecting;
  NSMutableData		*data;
  NSFileHandle		*handle;
  GSPortComState	state;
  struct in_addr	addr;
}
- (struct in_addr) addr;
- (void) close;
- (NSData*) data;
- (void) didConnect: (NSNotification*)notification;
- (void) didRead: (NSNotification*)notification;
- (void) didWrite: (NSNotification*)notification;
- (void) fail;
- (BOOL) isActive;
- (void) open: (NSString*)host;
- (void) setAddr: (struct in_addr)addr;
- (GSPortComState) state;
- (void) startListNameServers;
- (void) startPortLookup: (NSString*)name onHost: (NSString*)addr;
- (void) startPortRegistration: (uint32_t)portNumber withName: (NSString*)name;
- (void) startPortUnregistration: (uint32_t)portNumber
			withName: (NSString*)name;
@end

@implementation GSPortCom

- (struct in_addr) addr
{
  return addr;
}

- (void) close
{
  if (handle != nil)
    {
      NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];

      [nc removeObserver: self
		    name: GSFileHandleConnectCompletionNotification
		  object: handle];
      [nc removeObserver: self
		    name: NSFileHandleReadCompletionNotification
		  object: handle];
      [nc removeObserver: self
		    name: GSFileHandleWriteCompletionNotification
		  object: handle];
      [handle closeFile];
      DESTROY(handle);
    }
}

- (NSData*) data
{
  return data;
}

- (void) dealloc
{
  [self close];
  TEST_RELEASE(data);
  [super dealloc];
}

- (void) didConnect: (NSNotification*)notification
{
  NSDictionary	*userInfo = [notification userInfo];
  NSString	*e;

  e = [userInfo objectForKey: GSFileHandleNotificationError];
  if (e != nil)
    {
      NSDebugMLLog(@"NSSocketPortNameServer",
	@"failed connect to gdomap on %@:%@ - %@",
	[[notification object] socketAddress],
	[[notification object] socketService],
        e);
      /*
       * Remove our file handle, then either retry or fail.
       */
      [self close];
      if (launchCmd == nil)
	{
	  launchCmd = RETAIN([NSTask launchPathForTool: @"gdomap"]);
	}
      if (state == GSPC_LOPEN && launchCmd != nil)
	{
	  NSRunLoop	*loop = [NSRunLoop currentRunLoop];
	  NSTimer	*timer;

	  NSLog(@"NSSocketPortNameServer attempting to start gdomap on local host\n"
@"This will take a few seconds.\n"
@"Trying to launch gdomap from %@ or a machine/operating-system subdirectory.\n"
#if	!defined(GDOMAP_PORT_OVERRIDE)
@"On systems other than mswindows, this will only work if the gdomap program\n"
@"was installed setuid to root.\n"
#endif
@"It is recommended that you start up gdomap at login time or (better) when\n"
@"your computer is started instead.",
[launchCmd stringByDeletingLastPathComponent]);
	  [NSTask launchedTaskWithLaunchPath: launchCmd arguments: nil];
	  timer = [NSTimer timerWithTimeInterval: 5.0
				      invocation: nil
					 repeats: NO];
	  [loop addTimer: timer forMode: [loop currentMode]];
	  [loop runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 5.0]];
	  NSDebugMLLog(@"NSSocketPortNameServer",
	    @"retrying local connection to gdomap");
	  state = GSPC_RETRY;
	  [self open: nil];
	}
      else
	{
	  [self fail];
	}
    }
  else
    {
      [[NSNotificationCenter defaultCenter]
	removeObserver: self
		  name: GSFileHandleConnectCompletionNotification
		object: handle];
      /*
       * Now we have established a connection, we can write the request
       * to the name server.
       */
      state = GSPC_WRITE;
      [handle writeInBackgroundAndNotify: data
				forModes: modes];
      DESTROY(data);
    }
}

- (void) didRead: (NSNotification*)notification
{
  NSDictionary	*userInfo = [notification userInfo];
  NSData	*d;

  d = [userInfo objectForKey: NSFileHandleNotificationDataItem];

  if (d == nil || [d length] == 0)
    {
      [self fail];
      NSLog(@"NSSocketPortNameServer lost connection to gdomap on %@:%@",
	[[notification object] socketAddress],
	[[notification object] socketService]);
    }
  else
    {
      if (data == nil)
	{
	  data = [d mutableCopy];
	}
      else
	{
	  [data appendData: d];
	}
      if ([data length] < expecting)
	{
	  /*
	   *	Not enough data read yet - go read some more.
	   */
	  [handle readInBackgroundAndNotifyForModes: modes];
	}
      else if (state == GSPC_READ1 && msg.rtype == GDO_SERVERS)
	{
	  uint32_t	numSvrs = GSSwapBigI32ToHost(*(uint32_t*)[data bytes]);

	  if (numSvrs == 0)
	    {
	      [self fail];
	      NSLog(@"failed to get list of name servers on net");
	    }
	  else
	    {
	      /*
	       * Now read in the addresses of the servers.
	       */
	      expecting += numSvrs * sizeof(struct in_addr);
	      if ([data length] < expecting)
		{
		  state = GSPC_READ2;
		  [handle readInBackgroundAndNotifyForModes: modes];
		}
	      else
		{
		  [[NSNotificationCenter defaultCenter]
		    removeObserver: self
			      name: NSFileHandleReadCompletionNotification
			    object: handle];
		  state = GSPC_DONE;
		}
	    }
	}
      else
	{
	  [[NSNotificationCenter defaultCenter]
	    removeObserver: self
		      name: NSFileHandleReadCompletionNotification
		    object: handle];
	  state = GSPC_DONE;
	}
    }
}

- (void) didWrite: (NSNotification*)notification
{
  NSDictionary    *userInfo = [notification userInfo];
  NSString        *e;

  e = [userInfo objectForKey: GSFileHandleNotificationError];
  if (e != nil)
    {
      [self fail];
      NSLog(@"NSSocketPortNameServer failed write to gdomap on %@:%@ - %@",
	[[notification object] socketAddress],
	[[notification object] socketService],
        e);
    }
  else
    {
      state = GSPC_READ1;
      data = [NSMutableData new];
      expecting = 4;
      [handle readInBackgroundAndNotifyForModes: modes];
    }
}

- (void) fail
{
  [self close];
  if (data != nil)
    {
      DESTROY(data);
    }
  msg.rtype = 0;
  state = GSPC_FAIL;
}

- (BOOL) isActive
{
  if (handle == nil)
    return NO;
  if (state == GSPC_FAIL)
    return NO;
  if (state == GSPC_NONE)
    return NO;
  if (state == GSPC_DONE)
    return NO;
  return YES;
}

- (void) open: (NSString*)hostname
{
  NSNotificationCenter	*nc;

  NSAssert(state == GSPC_NONE || state == GSPC_RETRY, @"open in bad state");

  if (state == GSPC_NONE)
    {
      state = GSPC_ROPEN;	/* Assume we are connection to remote system */
      if (hostname == nil || [hostname isEqual: @""])
	{
	  hostname = @"localhost";
	  state = GSPC_LOPEN;
	}
      else
	{
	  NSHost	*local = [NSHost localHost];
	  NSHost	*host = [NSHost hostWithName: hostname];

	  if (host == nil)
	    {
	      host = [NSHost hostWithAddress: hostname];
	    }
	  if ([local isEqual: host])
	    {
	      state = GSPC_LOPEN;
	    }
	  else
	    {
	      NSHost	*loopback = [NSHost hostWithAddress: @"127.0.0.1"];

	      if ([loopback isEqual: host])
		{
		  state = GSPC_LOPEN;
		}
	    }
	}
    }

  NS_DURING
    {
      handle = [NSFileHandle fileHandleAsClientInBackgroundAtAddress:
	hostname service: serverPort protocol: @"tcp" forModes: modes];
    }
  NS_HANDLER
    {
      NSLog(@"Exception looking up port for gdomap - %@", localException);
      if ([[localException name] isEqual: NSInvalidArgumentException])
	{
	  handle = nil;
	}
      else
	{
	  [self fail];
	}
    }
  NS_ENDHANDLER

  if (state == GSPC_FAIL)
    return;

  if (handle == nil)
    {
      if (state == GSPC_LOPEN)
	{
	  NSLog(@"Failed to find gdomap port with name '%@',\nperhaps your "
	    @"/etc/services file is not correctly set up?\n"
	    @"Retrying with default (IANA allocated) port number 538",
	    serverPort);
	  NS_DURING
	    {
	      handle = [NSFileHandle fileHandleAsClientInBackgroundAtAddress:
		hostname service: @"538" protocol: @"tcp" forModes: modes];
	    }
	  NS_HANDLER
	    {
	      NSLog(@"Exception creating handle for gdomap - %@",
		localException);
	      [self fail];
	    }
	  NS_ENDHANDLER
	  if (handle)
	    {
	      RELEASE(serverPort);
	      serverPort = @"538";
	    }
	}
      else
	{
	  [self fail];
	}
    }

  if (state == GSPC_FAIL)
    return;

  IF_NO_GC(RETAIN(handle));
  nc = [NSNotificationCenter defaultCenter];
  [nc addObserver: self
	 selector: @selector(didConnect:)
	     name: GSFileHandleConnectCompletionNotification
	   object: handle];
  [nc addObserver: self
	 selector: @selector(didRead:)
	     name: NSFileHandleReadCompletionNotification
	   object: handle];
  [nc addObserver: self
	 selector: @selector(didWrite:)
	     name: GSFileHandleWriteCompletionNotification
	   object: handle];
}

- (void) setAddr: (struct in_addr)anAddr
{
  addr = anAddr;
}

- (GSPortComState) state
{
  return state;
}

- (void) startListNameServers
{
  msg.rtype = GDO_SERVERS;	/* Get a list of name servers.	*/
  msg.ptype = GDO_TCP_GDO;	/* Port is TCP port for GNU DO	*/
  msg.nsize = 0;
  msg.port = 0;
  TEST_RELEASE(data);
  data = [NSMutableData dataWithBytes: (void*)&msg length: sizeof(msg)];
  IF_NO_GC(RETAIN(data));
  [self open: nil];
}

- (void) startPortLookup: (NSString*)name onHost: (NSString*)host
{
  msg.rtype = GDO_LOOKUP;	/* Find the named port.		*/
  msg.ptype = GDO_TCP_GDO;	/* Port is TCP port for GNU DO	*/
  msg.port = 0;
  msg.nsize = [name cStringLength];
  [name getCString: (char*)msg.name];
  TEST_RELEASE(data);
  data = [NSMutableData dataWithBytes: (void*)&msg length: sizeof(msg)];
  IF_NO_GC(RETAIN(data));
  [self open: host];
}

- (void) startPortRegistration: (uint32_t)portNumber withName: (NSString*)name
{
  msg.rtype = GDO_REGISTER;	/* Register a port.		*/
  msg.ptype = GDO_TCP_GDO;	/* Port is TCP port for GNU DO	*/
  msg.nsize = [name cStringLength];
  [name getCString: (char*)msg.name];
  msg.port = GSSwapHostI32ToBig(portNumber);
  TEST_RELEASE(data);
  data = [NSMutableData dataWithBytes: (void*)&msg length: sizeof(msg)];
  IF_NO_GC(RETAIN(data));
  [self open: nil];
}

- (void) startPortUnregistration: (uint32_t)portNumber withName: (NSString*)name
{
  msg.rtype = GDO_UNREG;
  msg.ptype = GDO_TCP_GDO;
  if (name == nil)
    {
      msg.nsize = 0;
    }
  else
    {
      msg.nsize = [name cStringLength];
      [name getCString: (char*)msg.name];
    }
  msg.port = GSSwapHostI32ToBig(portNumber);
  TEST_RELEASE(data);
  data = [NSMutableData dataWithBytes: (void*)&msg length: sizeof(msg)];
  IF_NO_GC(RETAIN(data));
  [self open: nil];
}

@end



/**
 * This is the nameserver handling ports used for distributed objects
 * communications (see [NSConnection]) between hosts.<br />
 * Use the [NSSocketPortNameServer+sharedInstance] method to get a
 * nameserver, rather than allocating and initialising one.
 */
@implementation NSSocketPortNameServer

+ (id) allocWithZone: (NSZone*)aZone
{
  [NSException raise: NSGenericException
	      format: @"attempt to create extra port name server"];
  return nil;
}

+ (void) initialize
{
  if (self == [NSSocketPortNameServer class])
    {
      serverLock = [NSRecursiveLock new];
      [[NSObject leakAt: &serverLock] release];
      modes = [[NSArray alloc] initWithObjects: &mode count: 1];
      [[NSObject leakAt: &modes] release];
#ifdef	GDOMAP_PORT_OVERRIDE
      serverPort = RETAIN([NSString stringWithUTF8String:
	make_gdomap_port(GDOMAP_PORT_OVERRIDE)]);
      [[NSObject leakAt: &serverPort] release];
#endif
      portClass = [NSSocketPort class];
    }
}

/**
 * Returns the shared name server object for [NSSocketPort] objects.
 */
+ (id) sharedInstance
{
  if (defaultServer == nil)
    {
      NSSocketPortNameServer	*s;

      [serverLock lock];
      if (defaultServer)
	{
	  [serverLock unlock];
	  return defaultServer;
	}
      s = (NSSocketPortNameServer*)NSAllocateObject(self, 0,
	NSDefaultMallocZone());
      /* Use NSNonOwnedPointerMapKeyCallBacks for the ports used as keys
       * since we want as pointer test for equality as we may be doing
       * lookup while dealocating the port (in which case the -isEqual:
       * method could fail).
       */
      s->_portMap = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
			NSObjectMapValueCallBacks, 0);
      s->_nameMap = NSCreateMapTable(NSObjectMapKeyCallBacks,
			NSNonOwnedPointerMapValueCallBacks, 0);
      defaultServer = s;
      [serverLock unlock];
    }
  return defaultServer;
}

- (void) dealloc
{
  [NSException raise: NSGenericException
	      format: @"attempt to deallocate default port name server"];
  GSNOSUPERDEALLOC;
}

- (BOOL) _lookupName: (NSString*)name onHost: (NSString*)host
  intoAddress: (NSString**)addr andPort: (unsigned*)port
{
  GSPortCom		*com = nil;
  NSRunLoop		*loop = [NSRunLoop currentRunLoop];
  struct in_addr	singleServer;
  struct in_addr	*svrs = &singleServer;
  unsigned		numSvrs = 1;
  unsigned		count;
  unsigned		len;
  NSMutableArray	*array;
  NSDate		*limit;

  *port = 0;
  if (name == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to lookup port with nil name"];
    }

  len = [name cStringLength];
  if (len == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to lookup port with no name"];
    }
  if (len > GDO_NAME_MAX_LEN)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"name of port is too long (max %d) bytes",
			GDO_NAME_MAX_LEN];
    }

  limit = [NSDate dateWithTimeIntervalSinceNow: timeout];

  /*
   * get one or more host addresses in network byte order.
   */
  if (host == nil || [host isEqual: @""])
    {
      /*
       *	Query a single nameserver - on the local host.
       */
#ifndef HAVE_INET_ATON
      svrs->s_addr = inet_addr("127.0.0.1");
#else
      inet_aton("127.0.0.1", (struct in_addr *)&svrs->s_addr);
#endif
    }
  else if ([host isEqual: @"*"])
    {
      GSPortCom	*com = [GSPortCom new];

      [serverLock lock];
      NS_DURING
	{
	  NSData	*dat;

	  [com startListNameServers];
	  while ([limit timeIntervalSinceNow] > 0 && [com isActive] == YES)
	    {
	      [loop runMode: mode
		 beforeDate: limit];
	    }
	  [com close];
	  if ([com state] != GSPC_DONE)
	    {
	      [NSException raise: NSPortTimeoutException
			  format: @"timed out listing name servers"];
	    }
          /*
           * Retain and autorelease the data item so the buffer won't disappear
	   * when the 'com' object is destroyed.
	   */
          dat = AUTORELEASE(RETAIN([com data]));
	  svrs = (struct in_addr*)([dat bytes] + 4);
	  numSvrs = GSSwapBigI32ToHost(*(uint32_t*)[dat bytes]);
	  if (numSvrs == 0)
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"failed to get list of name servers"];
	    }
	  RELEASE(com);
	}
      NS_HANDLER
	{
	  /*
	   *	If we had a problem - unlock before continuing.
	   */
	  RELEASE(com);
	  [serverLock unlock];
	  NSDebugMLLog(@"NSSocketPortNameServer", @"%@", localException);
	  return NO;
	}
      NS_ENDHANDLER
      [serverLock unlock];
    }
  else
    {
      NSHost	*h;

      /*
       *	Query a single nameserver - on the specified host.
       */
      numSvrs = 1;
      h = [NSHost hostWithName: host];
      if (h)
	host = [h address];
#ifndef HAVE_INET_ATON
      svrs->s_addr = inet_addr([host cString]);
#else
      inet_aton([host cString], (struct in_addr *)&svrs->s_addr);
#endif
    }

  /*
   * Ok, 'svrs'now points to one or more internet addresses in network
   * byte order, and numSvrs tells us how many there are.
   */
  array = [NSMutableArray arrayWithCapacity: maxHandles];
  [serverLock lock];
  NS_DURING
    {
      unsigned	i;

      *port = 0;
      count = 0;
      do
	{
	  /*
	   *	Make sure that all the array slots are full if possible
	   */
	  while (count < numSvrs && [array count] < maxHandles)
	    {
	      NSString	*addr;

	      com = [GSPortCom new];
	      [array addObject: com];
	      RELEASE(com);
	      [com setAddr: svrs[count]];
	      addr = [NSString stringWithUTF8String:
		(char*)inet_ntoa(svrs[count])];
	      [com startPortLookup: name onHost: addr];
	      count++;
	    }

	  /*
	   * Handle I/O on the file handles.
	   */
	  i = [array count];
	  if (i == 0)
	    {
	      break;	/* No servers left to try!	*/
	    }
	  [loop runMode: mode beforeDate: limit];

	  /*
	   * Check for completed operations.
	   */
	  while (*port == 0 && i-- > 0)
	    {
	      com = [array objectAtIndex: i];
	      if ([com isActive] == NO)
		{
		  [com close];
		  if ([com state] == GSPC_DONE)
		    {
		      *port
		        = GSSwapBigI32ToHost(*(uint32_t*)[[com data] bytes]);
		      if (*port != 0)
			{
			  singleServer = [com addr];
			}
		    }
		  [array removeObjectAtIndex: i];
		}
	    }
	}
      while (*port == 0 && [limit timeIntervalSinceNow] > 0);

      /*
       * Make sure that any outstanding lookups are cancelled.
       */
      i = [array count];
      while (i-- > 0)
	{
	  [[array objectAtIndex: i] fail];
	  [array removeObjectAtIndex: i];
	}
    }
  NS_HANDLER
    {
      /*
       *	If we had a problem - unlock before continuing.
       */
      [serverLock unlock];
      NSDebugMLLog(@"NSSocketPortNameServer", @"%@", localException);
      return NO;
    }
  NS_ENDHANDLER
  [serverLock unlock];

  if (*port)
    {
      *addr = [NSString stringWithUTF8String: inet_ntoa(singleServer)];
      return YES;
    }	
  else
    {
      return NO;
    }
}

/**
 * Concrete implementation of [NSPortNameServer-portForName:onHost:]<br />
 * Looks up and returns a port with the specified name and host.<br />
 * If host is nil or an empty string, this performs a lookup for a
 * port on the current host.<br />
 * If host is an asterisk ('*') then the lookup returns the first
 * port found with the specified name on any machine on the local
 * network.<br />
 * Returns nil if no matching port could be found.
 */
- (NSPort*) portForName: (NSString*)name
		 onHost: (NSString*)host
{
  NSString	*addr;
  unsigned	portNum = 0;

  if ([self _lookupName: name
		 onHost: host
	    intoAddress: &addr
	        andPort: &portNum] == YES)
    {
      if (portClass == [NSSocketPort class])
	{
	  NSHost	*host;

	  host = [NSHost hostWithAddress: addr];
	  return (NSPort*)[NSSocketPort portWithNumber: portNum
						onHost: host
					  forceAddress: addr
					      listener: NO];
	}
      else
	{
	  NSLog(@"Unknown port class (%@) set for new port!", portClass);
	  return nil;
	}
    }	
  else
    {
      return nil;
    }
}

/**
 * Concrete implementation of [NSPortNameServer-registerPort:forName:]<br />
 * Registers the port with the specified name such that it is available
 * on all the IP addresses of the host on which the process is running.<br />
 * Returns YES on success, NO on failure (eg the name is already in use
 * or there is a problem registering for some reason).
 */
- (BOOL) registerPort: (NSPort*)port
	      forName: (NSString*)name
{
  NSRunLoop	*loop = [NSRunLoop currentRunLoop];
  GSPortCom	*com = nil;
  unsigned	len;
  NSDate	*limit;

  if (name == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to register port with nil name"];
    }
  if (port == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to register nil port"];
    }
  if ([port isKindOfClass: portClass] == NO)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to register port of unexpected class (%@)",
	port];
    }
  len = [name cStringLength];
  if (len == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to register port with no name"];
    }
  if (len > GDO_NAME_MAX_LEN)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"name of port is too long (max %d) bytes",
			GDO_NAME_MAX_LEN];
    }

  limit = [NSDate dateWithTimeIntervalSinceNow: timeout];
  /*
   *	Lock out other threads while doing I/O to gdomap
   */
  [serverLock lock];

  NS_DURING
    {
      NSMutableSet	*known = NSMapGet(_portMap, port);

      /*
       *	If there is no set of names for this port - create one.
       */
      if (known == nil)
	{
	  known = [NSMutableSet new];
	  NSMapInsert(_portMap, port, known);
	  RELEASE(known);
	}

      /*
       *	If this port has never been registered under any name, first
       *	send an unregister message to gdomap to ensure that any old
       *	names for the port (perhaps from a server that crashed without
       *	unregistering its ports) are no longer around.
       */
      if ([known count] == 0)
	{
	  com = [GSPortCom new];
	  [com startPortUnregistration: [(NSSocketPort*)port portNumber]
			      withName: nil];
	  while ([limit timeIntervalSinceNow] > 0 && [com isActive] == YES)
	    {
	      [loop runMode: mode
		 beforeDate: limit];
	    }
	  [com close];
	  if ([com state] != GSPC_DONE)
	    {
	      [NSException raise: NSPortTimeoutException
			  format: @"timed out unregistering port"];
	    }
	  DESTROY(com);
	}

      com = [GSPortCom new];
      [com startPortRegistration: [(NSSocketPort*)port portNumber]
			withName: name];
      while ([limit timeIntervalSinceNow] > 0 && [com isActive] == YES)
	{
	  [loop runMode: mode beforeDate: limit];
	}
      [com close];
      if ([com state] != GSPC_DONE)
	{
	  [NSException raise: NSPortTimeoutException
		      format: @"timed out registering port %@", name];
	}
      else
	{
	  unsigned	result;

	  result = GSSwapBigI32ToHost(*(uint32_t*)[[com data] bytes]);
	  if (result == 0)
	    {
	      unsigned int	portNum;
	      NSString		*addr;
	      BOOL		found;

	      NS_DURING
	        {
		  found = [self _lookupName: name
				     onHost: @""
				intoAddress: &addr
				    andPort: &portNum];

		}
	      NS_HANDLER
	        {
		  found = NO;
		}
	      NS_ENDHANDLER

	      if (found == YES)
		{
		  [NSException raise: NSGenericException
		    format: @"Unable to register name '%@' for the port -\n%@\n"
@"It appears that a process is already registered with this name at port\n"
@"'%d' IP address %@\n"
@"Perhaps this program ran before and was shut down without unregistering,\n"
@"so another process may be running using the same network port.  If this is\n"
@"the case, you can use -\n"
@"gdomap -U '%@'\n"
@"to remove the registration so that you can attempt this operation again.",
		    name, port, portNum, addr, name];
		}
	      else
	        {
		  [NSException raise: NSGenericException
		    format: @"Unable to register name '%@' for the port -\n%@\n"
@"Typically, this might mean that a process is already running with the name\n"
@"'%@' ...\n"
@"Try the command -\n"
@"  gdomap -M localhost -L '%@'\n"
@"to find its network details.\n"
@"Alternatively, it may have been shut down without unregistering, and\n"
@"another process may be running using the same network port.  If this is\n"
@"the case, you can use -\n"
@"gdomap -U '%@'\n"
@"to remove the registration so that you can attempt this operation again.",
		    name, port, name, name, name];
		}
	    }
	  else
	    {
	      /*
	       *	Add this name to the set of names that the port
	       *	is known by and to the name map.
	       */
	      [known addObject: name];
	      NSMapInsert(_nameMap, name, port);
	    }
	}
      DESTROY(com);
    }
  NS_HANDLER
    {
      /*
       *	If we had a problem - close and unlock before continuing.
       */
      DESTROY(com);
      [serverLock unlock];
      NSDebugMLLog(@"NSSocketPortNameServer", @"%@", localException);
      return NO;
    }
  NS_ENDHANDLER
  [serverLock unlock];
  return YES;
}

/**
 * Concrete implementation of [NSPortNameServer-removePortForName:]<br />
 * Unregisters the specified name from any associated port on the
 * local host.<br />
 * Returns YES on success, NO on failure.
 */
- (BOOL) removePortForName: (NSString*)name
{
  NSRunLoop	*loop = [NSRunLoop currentRunLoop];
  GSPortCom	*com = nil;
  unsigned	len;
  NSDate	*limit = [NSDate dateWithTimeIntervalSinceNow: timeout];
  BOOL		val = NO;

  if (name == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to remove port with nil name"];
    }

  len = [name cStringLength];
  if (len == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to remove port with no name"];
    }
  if (len > GDO_NAME_MAX_LEN)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"name of port is too long (max %d) bytes",
			GDO_NAME_MAX_LEN];
    }

  /*
   *	Lock out other threads while doing I/O to gdomap
   */
  [serverLock lock];

  NS_DURING
    {
      com = [GSPortCom new];
      [com startPortUnregistration: 0 withName: name];
      while ([limit timeIntervalSinceNow] > 0 && [com isActive] == YES)
	{
	  [loop runMode: mode
	     beforeDate: limit];
	}
      [com close];
      if ([com state] != GSPC_DONE)
	{
	  [NSException raise: NSPortTimeoutException
		      format: @"timed out unregistering port"];
	}
      else
	{
	  NSPort	*port;
	  unsigned	result;

	  result = GSSwapBigI32ToHost(*(uint32_t*)[[com data] bytes]);
	  if (result == 0)
	    {
	      NSLog(@"NSSocketPortNameServer unable to unregister '%@'", name);
	      val = NO;
	    }
	  else
	    {
	      val = YES;
	    }
	  /*
	   *	Find the port that was registered for this name and
	   *	remove the mapping table entries.
	   */
	  port = NSMapGet(_nameMap, name);
	  if (port)
	    {
	      NSMutableSet	*known;

	      NSMapRemove(_nameMap, name);
	      known = NSMapGet(_portMap, port);
	      if (known)
		{
		  [known removeObject: name];
		  if ([known count] == 0)
		    {
		      NSMapRemove(_portMap, port);
		    }
		}
	    }
	}
      RELEASE(com);
    }
  NS_HANDLER
    {
      /*
       *	If we had a problem - unlock before continuing.
       */
      RELEASE(com);
      NSDebugMLLog(@"NSSocketPortNameServer", @"%@", localException);
      val = NO;
    }
  NS_ENDHANDLER
  [serverLock unlock];
  return val;
}
@end

@implementation	NSSocketPortNameServer (GNUstep)

+ (Class) setPortClass: (Class)c
{
  Class	old = portClass;

  portClass = c;
  return old;
}

/**
 * Return the names under which the port is currently registered by
 * this process.
 */
- (NSArray*) namesForPort: (NSPort*)port
{
  NSArray	*names;

  if (port == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"attempt to get names for nil port"];
    }
  /*
   *	Lock out other threads while grabbing port names.
   */
  [serverLock lock];
  names = [(NSSet*)NSMapGet(_portMap, port) allObjects];
  [serverLock unlock];
  return names;
}

/**
 * Remove all names for a particular port - used when a port is
 * invalidated.
 */
- (BOOL) removePort: (NSPort*)port
{
  BOOL	ok = YES;
  [serverLock lock];
  NS_DURING
    {
      NSMutableSet	*known = (NSMutableSet*)NSMapGet(_portMap, port);
      NSString		*name;

      IF_NO_GC(RETAIN(known);)
      while ((name = [known anyObject]) != nil)
	{
	  if ([self removePortForName: name] == NO)
	    {
	      ok = NO;
	    }
	}
      RELEASE(known);
    }
  NS_HANDLER
    {
      [serverLock unlock];
      NSDebugMLLog(@"NSSocketPortNameServer", @"%@", localException);
      return NO;
    }
  NS_ENDHANDLER
  [serverLock unlock];
  return ok;
}

/**
 * Remove name for port iff it is registered by this process.
 */
- (BOOL) removePort: (NSPort*)port forName: (NSString*)name
{
  BOOL	ok = YES;

  [serverLock lock];
  NS_DURING
    {
      NSMutableSet	*known = (NSMutableSet*)NSMapGet(_portMap, port);

      if ([known member: name] != nil)
	{
	  if ([self removePortForName: name] == NO)
	    {
	      ok = NO;
	    }
	}
    }
  NS_HANDLER
    {
      [serverLock unlock];
      [localException raise];
    }
  NS_ENDHANDLER
  [serverLock unlock];
  return ok;
}
@end

