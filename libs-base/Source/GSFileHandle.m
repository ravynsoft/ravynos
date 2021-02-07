/** Implementation for GSFileHandle for GNUStep
   Copyright (C) 1997-2002 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1997, 2002

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

#import "common.h"
#define	EXPOSE_NSFileHandle_IVARS	1
#define	EXPOSE_GSFileHandle_IVARS	1
#import "Foundation/NSData.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSFileHandle.h"
#import "Foundation/NSException.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSNotificationQueue.h"
#import "Foundation/NSHost.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSUserDefaults.h"
#import "GSPrivate.h"
#import "GSNetwork.h"
#import "GSFileHandle.h"

#import "../Tools/gdomap.h"

#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#if	defined(HAVE_SYS_FILE_H)
#  include	<sys/file.h>
#endif

#include <sys/stat.h>

#if	defined(HAVE_SYS_FCNTL_H)
#  include	<sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include	<fcntl.h>
#endif

#include <sys/ioctl.h>
#ifdef	__svr4__
#  ifdef HAVE_SYS_FILIO_H
#    include <sys/filio.h>
#  endif
#endif
#include <netdb.h>

/*
 *	Stuff for setting the sockets into non-blocking mode.
 */
#if defined(__POSIX_SOURCE)\
        || defined(__EXT_POSIX1_198808)\
        || defined(O_NONBLOCK)
#define NBLK_OPT     O_NONBLOCK
#else
#define NBLK_OPT     FNDELAY
#endif

#ifndef	O_BINARY
#ifdef	_O_BINARY
#define	O_BINARY	_O_BINARY
#else
#define	O_BINARY	0
#endif
#endif

#ifndef	INADDR_NONE
#define	INADDR_NONE	-1
#endif

// Maximum data in single I/O operation
#define	NETBUF_SIZE	(1024 * 16)
#define	READ_SIZE	NETBUF_SIZE*10

static GSFileHandle     *fh_stdin = nil;
static GSFileHandle     *fh_stdout = nil;
static GSFileHandle     *fh_stderr = nil;

@interface      GSTcpTune : NSObject
- (int) delay;
- (int) recvSize;
- (int) sendSize: (int)bytesToSend;
- (void) tune: (void*)handle;
@end

@implementation GSTcpTune

static int      tuneDelay = 0;
static int      tuneLinger = -1;
static int      tuneReceive = 0;
static BOOL     tuneSendAll = NO;
static int	tuneRBuf = 0;
static int	tuneSBuf = 0;

+ (void) defaultsChanged: (NSNotification*)n
{
  NSUserDefaults        *defs = (NSUserDefaults*)[n object];
  NSString              *str;

  if (nil == defs)
    {
      defs = [NSUserDefaults standardUserDefaults];
    }
  str = [defs stringForKey: @"GSTcpLinger"];
  if (nil == str)
    {
      tuneLinger = -1;
    }
  else
    {
      tuneLinger = [str intValue];
    }
  tuneRBuf = (int)[defs integerForKey: @"GSTcpRcvBuf"];
  tuneSBuf = (int)[defs integerForKey: @"GSTcpSndBuf"];
  tuneReceive = (int)[defs integerForKey: @"GSTcpReceive"];
  tuneSendAll = [defs boolForKey: @"GSTcpSendAll"];
  tuneDelay = [defs boolForKey: @"GSTcpDelay"];
}

+ (void) initialize
{
  static BOOL   beenHere = NO;

  if (NO == beenHere)
    {
      NSNotificationCenter      *nc;
      NSUserDefaults	        *defs;

      beenHere = YES;
      nc = [NSNotificationCenter defaultCenter];
      defs = [NSUserDefaults standardUserDefaults];
      [nc addObserver: self
             selector: @selector(defaultsChanged:)
                 name: NSUserDefaultsDidChangeNotification
               object: defs];
      [self defaultsChanged: nil];
    }
}

- (int) delay
{
  return tuneDelay;             // Milliseconds to delay close
}

- (int) recvSize
{
  if (tuneReceive > 0)
    {
      return tuneReceive;       // Return receive buffer size
    }
  if (tuneRBuf > 0)
    {
      return tuneRBuf;          // Return socket receive buffer size
    }
  return READ_SIZE;             // Return hard-coded default
}

- (int) sendSize: (int)bytesToSend
{
  if (YES == tuneSendAll)
    {
      return bytesToSend;       // Try to send all in one go
    }
  if (tuneSBuf > 0 && tuneSBuf <= bytesToSend)
    {
      return tuneSBuf;          // Limit to socket send buffer
    }
  if (NETBUF_SIZE <= bytesToSend)
    {
      return NETBUF_SIZE;       // Limit to hard coded default
    }
  return bytesToSend;
}

- (void) tune: (void*)handle
{
  int   desc = (int)(intptr_t)handle;
  int   value;

  /*
   * Enable tcp-level tracking of whether connection is alive.
   */
  value = 1;
  if (setsockopt(desc, SOL_SOCKET, SO_KEEPALIVE, (char *)&value, sizeof(value))
    < 0)
    {
      NSDebugMLLog(@"GSTcpTune", @"setsockopt keepalive failed");
    }

  if (tuneLinger >= 0)
    {
      struct linger     l;

      l.l_onoff = 1;
      l.l_linger = tuneLinger;
      if (setsockopt(desc, SOL_SOCKET, SO_LINGER, (char *)&l, sizeof(l)) < 0)
        {
          NSLog(@"Failed to set GSTcpLinger %d: %@",
            tuneLinger, [NSError _last]);
        }
    }
  
  if (tuneRBuf > 0)
    {
      /* Set the receive buffer for the socket.
       */
      if (setsockopt(desc, SOL_SOCKET, SO_RCVBUF,
        (char *)&tuneRBuf, sizeof(tuneRBuf)) < 0)
        {
          NSLog(@"Failed to set GSTcpRcvBuf %d: %@", tuneRBuf, [NSError _last]);
        }
      else
        {
	  NSDebugMLLog(@"GSTcpTune", @"Set GSTcpRcvBuf %d", tuneRBuf);
        }
    }
  if (tuneSBuf > 0)
    {
      /* Set the send buffer for the socket.
       */
      if (setsockopt(desc, SOL_SOCKET, SO_SNDBUF,
        (char *)&tuneSBuf, sizeof(tuneSBuf)) < 0)
        {
          NSLog(@"Failed to set GSTcpSndBuf %d: %@", tuneSBuf, [NSError _last]);
        }
      else
        {
	  NSDebugMLLog(@"GSTcpTune", @"Set GSTcpSndBuf %d", tuneSBuf);
        }
    }
}
@end

// Key to info dictionary for operation mode.
static NSString*	NotificationKey = @"NSFileHandleNotificationKey";

@interface GSFileHandle(private)
- (void) receivedEventRead;
- (void) receivedEventWrite;
@end

@implementation GSFileHandle

static GSTcpTune        *tune = nil;

+ (void) initialize
{
  if (nil == tune)
    {
      tune = [GSTcpTune new];
    }
}

/**
 * Encapsulates low level read operation to get data from the operating
 * system.
 */
- (NSInteger) read: (void*)buf length: (NSUInteger)len
{
  int	result;

  do
    {
#ifdef __ANDROID__
      if (asset)
	{
	  result = AAsset_read(asset, buf, len);
	}
      else
#endif
#if	USE_ZLIB
      if (gzDescriptor != 0)
	{
	  result = gzread(gzDescriptor, buf, len);
	}
      else
#endif
      if (isSocket)
	{
	  result = recv(descriptor, buf, len, 0);
	}
      else
	{
	  result = read(descriptor, buf, len);
	}
    }
  while (result < 0 && EINTR == errno);
  return result;
}

/**
 * Encapsulates low level write operation to send data to the operating
 * system.
 */
- (NSInteger) write: (const void*)buf length: (NSUInteger)len
{
  int	result;

  do
    {
#if	USE_ZLIB
    if (gzDescriptor != 0)
      {
	result = gzwrite(gzDescriptor, (char*)buf, len);
      }
    else
#endif
      if (isSocket)
	{
	  result = send(descriptor, buf, len, 0);
	}
      else
	{
	  result = write(descriptor, buf, len);
	}
    }
  while (result < 0 && EINTR == errno);
  return result;
}

+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject ([self class], 0, z);
}

- (void) dealloc
{
  if (self == fh_stdin)
    {
      RETAIN(self);
      [NSException raise: NSGenericException
                  format: @"Attempt to deallocate standard input handle"];
    }
  if (self == fh_stdout)
    {
      RETAIN(self);
      [NSException raise: NSGenericException
                  format: @"Attempt to deallocate standard output handle"];
    }
  if (self == fh_stderr)
    {
      RETAIN(self);
      [NSException raise: NSGenericException
                  format: @"Attempt to deallocate standard error handle"];
    }
  DESTROY(address);
  DESTROY(service);
  DESTROY(protocol);
  DESTROY(readInfo);
  DESTROY(writeInfo);

  /* Finalize *after* destroying readInfo and writeInfo so that, if the
   * file handle needs to be closed, we don't generate any notifications
   * containing the deallocated object.  Tnanks to david for this fix.
   */
  [self finalize];
  [super dealloc];
}

- (void) finalize
{
  [self ignoreReadDescriptor];
  [self ignoreWriteDescriptor];

#ifdef __ANDROID__
  if (asset)
    {
      AAsset_close(asset);
      asset = NULL;
    }
  else
#endif
  if (closeOnDealloc == YES && descriptor != -1)
    {
      [self closeFile];
    }
  else
    {
#if	USE_ZLIB
      /*
       * The gzDescriptor should always be closed when we have done with it.
       */
      if (gzDescriptor != 0)
        {
          gzclose(gzDescriptor);
          gzDescriptor = 0;
        }
#endif
      if (descriptor != -1)
        {
          [self setNonBlocking: wasNonBlocking];
        }
    }
}

// Initializing a GSFileHandle Object

- (id) init
{
  return [self initWithNullDevice];
}

/**
 * Initialise as a client socket connection ... do this by using
 * [-initAsClientInBackgroundAtAddress:service:protocol:forModes:]
 * and running the current run loop in NSDefaultRunLoopMode until
 * the connection attempt succeeds, fails, or times out.
 */
- (id) initAsClientAtAddress: (NSString*)a
		     service: (NSString*)s
		    protocol: (NSString*)p
{
  self = [self initAsClientInBackgroundAtAddress: a
					 service: s
					protocol: p
					forModes: nil];
  if (self != nil)
    {
      NSRunLoop	*loop;
      NSDate	*limit;

      loop = [NSRunLoop currentRunLoop];
      limit = [NSDate dateWithTimeIntervalSinceNow: 300];
      while ([limit timeIntervalSinceNow] > 0
	&& (readInfo != nil || [writeInfo count] > 0))
	{
	  [loop runMode: NSDefaultRunLoopMode
	     beforeDate: limit];
	}
      if (readInfo != nil || [writeInfo count] > 0 || readOK == NO)
	{
	  /* Must have timed out or failed */
	  DESTROY(self);
	}
      else
	{
	  [self setNonBlocking: NO];
	}
    }
  return self;
}

/*
 * States for socks connection negotiation
 */
NSString * const GSSOCKSConnect = @"GSSOCKSConnect";
NSString * const GSSOCKSSendAuth = @"GSSOCKSSendAuth";
NSString * const GSSOCKSRecvAuth = @"GSSOCKSRecvAuth";
NSString * const GSSOCKSSendConn = @"GSSOCKSSendConn";
NSString * const GSSOCKSRecvConn = @"GSSOCKSRecvConn";
NSString * const GSSOCKSRecvAddr = @"GSSOCKSRecvAddr";

- (void) _socksHandler: (NSNotification*)aNotification
{
  NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];
  NSString		*name = [aNotification name];
  NSDictionary		*info = (NSMutableDictionary*)[aNotification userInfo];
  NSArray		*modes;
  NSString		*error;
  NSMutableDictionary	*i = nil;
  NSNotification	*n = nil;

  NSDebugMLLog(@"NSFileHandle", @"%@ SOCKS connection: %@",
    self, aNotification);

  [nc removeObserver: self name: name object: self];

  modes = (NSArray*)[info objectForKey: NSFileHandleNotificationMonitorModes];
  error = [info objectForKey: GSFileHandleNotificationError];

  if (error == nil)
    {
      if (name == GSSOCKSConnect)
	{
	  NSData	*item;

	  /*
	   * Send an authorisation record to the SOCKS server.
	   */
	  i = [info mutableCopy];
	  /*
	   * Authorisation record is at least three bytes -
	   *   socks version (5)
	   *   authorisation method bytes to follow (1)
	   *   say we do no authorisation (0)
	   */
	  item = [[NSData alloc] initWithBytes: "\5\1\0"
					length: 3];
	  [i setObject: item forKey: NSFileHandleNotificationDataItem];
	  RELEASE(item);
	  [i setObject: GSSOCKSSendAuth forKey: NotificationKey];
	  [writeInfo addObject: i];
	  RELEASE(i);
	  [nc addObserver: self
		 selector: @selector(_socksHandler:)
		     name: GSSOCKSSendAuth
		   object: self];
	  [self watchWriteDescriptor];
	}
      else if (name == GSSOCKSSendAuth)
	{
	  NSMutableData	*item;

	  /*
	   * We have written the authorisation record, so we
	   * request a response from the SOCKS server.
	   */
	  readMax = 2;
	  readInfo = [info mutableCopy];
	  [readInfo setObject: GSSOCKSRecvAuth forKey: NotificationKey];
	  item = [[NSMutableData alloc] initWithCapacity: 0];
	  [readInfo setObject: item forKey: NSFileHandleNotificationDataItem];
	  RELEASE(item);
	  [nc addObserver: self
		 selector: @selector(_socksHandler:)
		     name: GSSOCKSRecvAuth
		   object: self];
	  [self watchReadDescriptorForModes: modes];
	}
      else if (name == GSSOCKSRecvAuth)
	{
	  NSData		*response;
	  const unsigned char	*bytes;

	  response = [info objectForKey: NSFileHandleNotificationDataItem];
	  bytes = (const unsigned char*)[response bytes];
	  if ([response length] != 2)
	    {
	      error = @"authorisation response from SOCKS was not two bytes";
	    }
	  else if (bytes[0] != 5)
	    {
	      error = @"authorisation response from SOCKS had wrong version";
	    }
	  else if (bytes[1] != 0)
	    {
	      error = @"authorisation response from SOCKS had wrong method";
	    }
	  else
	    {
	      NSData		*item;
	      char		buf[10];
	      const char	*ptr;
	      int		p;

	      /*
	       * Send the address information to the SOCKS server.
	       */
	      i = [info mutableCopy];
	      /*
	       * Connect command is ten bytes -
	       *   socks version
	       *   connect command
	       *   reserved byte
	       *   address type
	       *   address 4 bytes (big endian)
	       *   port 2 bytes (big endian)
	       */
	      buf[0] = 5;	// Socks version number
	      buf[1] = 1;	// Connect command
	      buf[2] = 0;	// Reserved
	      buf[3] = 1;	// Address type (IPV4)
	      ptr = [address lossyCString];
	      buf[4] = atoi(ptr);
	      while (isdigit(*ptr))
		ptr++;
	      ptr++;
	      buf[5] = atoi(ptr);
	      while (isdigit(*ptr))
		ptr++;
	      ptr++;
	      buf[6] = atoi(ptr);
	      while (isdigit(*ptr))
		ptr++;
	      ptr++;
	      buf[7] = atoi(ptr);
	      p = [service intValue];
	      buf[8] = ((p & 0xff00) >> 8);
	      buf[9] = (p & 0xff);

	      item = [[NSData alloc] initWithBytes: buf length: 10];
	      [i setObject: item forKey: NSFileHandleNotificationDataItem];
	      RELEASE(item);
	      [i setObject: GSSOCKSSendConn
		    forKey: NotificationKey];
	      [writeInfo addObject: i];
	      RELEASE(i);
	      [nc addObserver: self
		     selector: @selector(_socksHandler:)
			 name: GSSOCKSSendConn
		       object: self];
	      [self watchWriteDescriptor];
	    }
	}
      else if (name == GSSOCKSSendConn)
	{
	  NSMutableData	*item;

	  /*
	   * We have written the connect command, so we
	   * request a response from the SOCKS server.
	   */
	  readMax = 4;
	  readInfo = [info mutableCopy];
	  [readInfo setObject: GSSOCKSRecvConn forKey: NotificationKey];
	  item = [[NSMutableData alloc] initWithCapacity: 0];
	  [readInfo setObject: item forKey: NSFileHandleNotificationDataItem];
	  RELEASE(item);
	  [nc addObserver: self
		 selector: @selector(_socksHandler:)
		     name: GSSOCKSRecvConn
		   object: self];
	  [self watchReadDescriptorForModes: modes];
	}
      else if (name == GSSOCKSRecvConn)
	{
	  NSData		*response;
	  const unsigned char	*bytes;
	  unsigned		len = 0;

	  response = [info objectForKey: NSFileHandleNotificationDataItem];
	  bytes = (const unsigned char*)[response bytes];
	  if ([response length] != 4)
	    {
	      error = @"connect response from SOCKS had bad length";
	    }
	  else if (bytes[0] != 5)
	    {
	      error = @"connect response from SOCKS had wrong version";
	    }
	  else if (bytes[1] != 0)
	    {
	      switch (bytes[1])
		{
		  case 1:
		    error = @"SOCKS server general failure";
		    break;
		  case 2:
		    error = @"SOCKS server says permission denied";
		    break;
		  case 3:
		    error = @"SOCKS server says network unreachable";
		    break;
		  case 4:
		    error = @"SOCKS server says host unreachable";
		    break;
		  case 5:
		    error = @"SOCKS server says connection refused";
		    break;
		  case 6:
		    error = @"SOCKS server says connection timed out";
		    break;
		  case 7:
		    error = @"SOCKS server says command not supported";
		    break;
		  case 8:
		    error = @"SOCKS server says address type not supported";
		    break;
		  default:
		    error = @"connect response from SOCKS was failure";
		    break;
		}
	    }
	  else if (bytes[3] == 1)
	    {
	      len = 4;			// Fixed size (IPV4) address
	    }
	  else if (bytes[3] == 3)
	    {
	      len = 1 + bytes[4];	// Domain name with leading length
	    }
	  else if (bytes[3] == 4)
	    {
	      len = 16;			// Fixed size (IPV6) address
	    }
	  else
	    {
	      error = @"SOCKS server returned unknown address type";
	    }

	  if (error == nil)
	    {
	      NSMutableData	*item;

	      /*
	       * We have received a success, so we must now consume the
	       * address and port information the SOCKS server sends.
	       */
	      readMax = len + 2;
	      readInfo = [info mutableCopy];
	      [readInfo setObject: GSSOCKSRecvAddr forKey: NotificationKey];
	      item = [[NSMutableData alloc] initWithCapacity: 0];
	      [readInfo setObject: item
			   forKey: NSFileHandleNotificationDataItem];
	      RELEASE(item);
	      [nc addObserver: self
		     selector: @selector(_socksHandler:)
			 name: GSSOCKSRecvAddr
		       object: self];
	      [self watchReadDescriptorForModes: modes];
	    }
	}
      else if (name == GSSOCKSRecvAddr)
	{
	  /*
	   * Success ... We read the address from the socks server so
	   * the connection is now ready to go.
	   */
	  name = GSFileHandleConnectCompletionNotification;
	  i = [info mutableCopy];
	  [i setObject: name forKey: NotificationKey];
	  n = [NSNotification notificationWithName: name
					    object: self
					  userInfo: i];
	  RELEASE(i);
	}
      else
	{
	  /*
	   * Argh ... unexpected notification.
	   */
	  error = @"unexpected notification during SOCKS connection";
	}
    }

  /*
   * If 'error' is non-null, we set up a notification to tell people
   * the connection failed.
   */
  if (error != nil)
    {
      NSDebugMLLog(@"NSFileHandle", @"%@ SOCKS error: %@", self, error);

      /*
       * An error in the initial connection ... notify observers
       * by re-posting the notification with a new name.
       */
      name = GSFileHandleConnectCompletionNotification;
      i = [info mutableCopy];
      [i setObject: name forKey: NotificationKey];
      [i setObject: error forKey: GSFileHandleNotificationError];
      n = [NSNotification notificationWithName: name
					object: self
				      userInfo: i];
      RELEASE(i);
    }

  /*
   * If a notification has been set up, we post it as the last thing we do.
   */
  if (n != nil)
    {
      NSNotificationQueue	*q;

      q = [NSNotificationQueue defaultQueue];
      [q enqueueNotification: n
		postingStyle: NSPostASAP
		coalesceMask: NSNotificationNoCoalescing
		    forModes: modes];
    }
}

- (id) initAsClientInBackgroundAtAddress: (NSString*)a
				 service: (NSString*)s
			        protocol: (NSString*)p
			        forModes: (NSArray*)modes
{
  static NSString	*esocks = nil;
  static NSString	*dsocks = nil;
  static BOOL		beenHere = NO;
  int			net;
  struct sockaddr	sin;
  struct sockaddr	lsin;
  NSString		*lhost = nil;
  NSString		*shost = nil;
  NSString		*sport = nil;

  if (beenHere == NO)
    {
      NSUserDefaults	*defs;

      beenHere = YES;
      defs = [NSUserDefaults standardUserDefaults];
      dsocks = [[defs stringForKey: @"GSSOCKS"] copy];
      if (dsocks == nil)
	{
	  NSDictionary	*env;

	  env = [[NSProcessInfo processInfo] environment];
	  esocks = [env objectForKey: @"SOCKS5_SERVER"];
	  if (esocks == nil)
	    {
	      esocks = [env objectForKey: @"SOCKS_SERVER"];
	    }
	  esocks = [esocks copy];
	}
    }

  if (a == nil || [a isEqualToString: @""])
    {
      a = @"localhost";
    }
  if (s == nil)
    {
      NSLog(@"bad argument - service is nil");
      DESTROY(self);
      return nil;
    }

  if ([p hasPrefix: @"bind-"] == YES)
    {
      NSRange	r;

      lhost = [p substringFromIndex: 5];
      r = [lhost rangeOfString: @":"];
      if (r.length > 0)
	{
	  p = [lhost substringFromIndex: NSMaxRange(r)];
	  lhost = [lhost substringToIndex: r.location];
	}
      else
	{
	  p = nil;
	}
      if (GSPrivateSockaddrSetup(lhost, 0, p, @"tcp", &lsin) == NO)
	{
	  NSLog(@"bad bind address specification");
	  DESTROY(self);
	  return nil;
	}
      p = @"tcp";
    }

  /**
   * A protocol fo the form 'socks-...' controls socks operation,
   * overriding defaults and environment variables.<br />
   * If it is just 'socks-' it turns off socks for this fiel handle.<br />
   * Otherwise, the following text must be the name of the socks server
   * (optionally followed by :port).
   */
  if ([p hasPrefix: @"socks-"] == YES)
    {
      shost = [p substringFromIndex: 6];
      p = @"tcp";
    }
  else if (dsocks != nil)
    {
      shost = dsocks;	// GSSOCKS user default
    }
  else
    {
      shost = esocks;	// SOCKS_SERVER environment variable.
    }

  if (shost != nil && [shost length] > 0)
    {
      NSRange	r;

      r = [shost rangeOfString: @":"];
      if (r.length > 0)
	{
	  sport = [shost substringFromIndex: NSMaxRange(r)];
	  shost = [shost substringToIndex: r.location];
	}
      else
	{
	  sport = @"1080";
	}
      p = @"tcp";
    }

  if (GSPrivateSockaddrSetup(a, 0, s, p, &sin) == NO)
    {
      DESTROY(self);
      NSLog(@"bad address-service-protocol combination");
      return nil;
    }
  [self setAddr: &sin];		// Store the address of the remote end.

  /*
   * Don't use SOCKS if we are contacting the local host.
   */
  if (shost != nil)
    {
      NSHost	*remote = [NSHost hostWithAddress: [self socketAddress]];
      NSHost	*local = [NSHost currentHost];

      if ([remote isEqual: local] || [remote isEqual: [NSHost localHost]])
        {
	  shost = nil;
	}
    }
  if (shost != nil)
    {
      if (GSPrivateSockaddrSetup(shost, 0, sport, p, &sin) == NO)
	{
	  NSLog(@"bad SOCKS host-port combination");
	  DESTROY(self);
	  return nil;
	}
    }

  if ((net = socket(sin.sa_family, SOCK_STREAM, PF_UNSPEC)) == -1)
    {
      NSLog(@"unable to create socket - %@", [NSError _last]);
      DESTROY(self);
      return nil;
    }

  [tune tune: (void*)(intptr_t)net];

  if (lhost != nil)
    {
      if (bind(net, &lsin, GSPrivateSockaddrLength(&lsin)) == -1)
	{
	  NSLog(@"unable to bind to port %@ - %@",
	    GSPrivateSockaddrName(&lsin), [NSError _last]);
	  (void) close(net);
	  DESTROY(self);
	  return nil;
	}
    }

  self = [self initWithFileDescriptor: net closeOnDealloc: YES];
  if (self)
    {
      NSMutableDictionary*	info;

      isSocket = YES;
      [self setNonBlocking: YES];
      if (connect(net, &sin, GSPrivateSockaddrLength(&sin)) == -1)
	{
	  if (!GSWOULDBLOCK)
	    {
	      NSError	*e = [NSError _last];

	      NSLog(@"unable to make socket connection to %@ - %@ (%d)",
		GSPrivateSockaddrName(&sin), e, (int)[e code]);
	      DESTROY(self);
	      return nil;
	    }
	}

      info = [[NSMutableDictionary alloc] initWithCapacity: 4];
      [info setObject: address forKey: NSFileHandleNotificationDataItem];
      if (shost == nil)
	{
	  [info setObject: GSFileHandleConnectCompletionNotification
		   forKey: NotificationKey];
	}
      else
	{
	  NSNotificationCenter	*nc;

	  /*
	   * If we are making a socks connection, register self as an
	   * observer of notifications and ensure we will manage this.
	   */
	  nc = [NSNotificationCenter defaultCenter];
	  [nc addObserver: self
		 selector: @selector(_socksHandler:)
		     name: GSSOCKSConnect
		   object: self];
	  [info setObject: GSSOCKSConnect
		   forKey: NotificationKey];
	}
      if (modes)
	{
	  [info setObject: modes forKey: NSFileHandleNotificationMonitorModes];
	}
      [writeInfo addObject: info];
      RELEASE(info);
      [self watchWriteDescriptor];
      connectOK = YES;
      acceptOK = NO;
      readOK = NO;
      writeOK = NO;
    }
  return self;
}

- (id) initAsServerAtAddress: (NSString*)a
		     service: (NSString*)s
		    protocol: (NSString*)p
{
#ifndef	BROKEN_SO_REUSEADDR
  int			status = 1;
#endif
  int			net;
  struct sockaddr	sin;
  unsigned int		size = sizeof(sin);

  if (GSPrivateSockaddrSetup(a, 0, s, p, &sin) == NO)
    {
      DESTROY(self);
      NSLog(@"bad address-service-protocol combination");
      return  nil;
    }

  if ((net = socket(sin.sa_family, SOCK_STREAM, PF_UNSPEC)) == -1)
    {
      NSLog(@"unable to create socket - %@", [NSError _last]);
      DESTROY(self);
      return nil;
    }

#ifndef	BROKEN_SO_REUSEADDR
  /*
   * Under decent systems, SO_REUSEADDR means that the port can be reused
   * immediately that this process exits.  Under some it means
   * that multiple processes can serve the same port simultaneously.
   * We don't want that broken behavior!
   */
  if (setsockopt(net, SOL_SOCKET, SO_REUSEADDR, (char*)&status, sizeof(status))
    < 0)
    {
      NSDebugMLLog(@"GSTcpTune", @"setsockopt reuseaddr failed");
    }
#endif

  if (bind(net, &sin, GSPrivateSockaddrLength(&sin)) == -1)
    {
      NSError	*e = [NSError _last];

      NSLog(@"unable to bind to port %@ - %@",
	GSPrivateSockaddrName(&sin), e);
      (void) close(net);
      DESTROY(self);
      return nil;
    }

  /* We try to allow a large number of connections.
   */
  if (listen(net, GSBACKLOG) == -1)
    {
      NSLog(@"unable to listen on port - %@", [NSError _last]);
      (void) close(net);
      DESTROY(self);
      return nil;
    }

  if (getsockname(net, &sin, &size) == -1)
    {
      NSLog(@"unable to get socket name - %@", [NSError _last]);
      (void) close(net);
      DESTROY(self);
      return nil;
    }

  self = [self initWithFileDescriptor: net closeOnDealloc: YES];
  if (self)
    {
      isSocket = YES;
      connectOK = NO;
      acceptOK = YES;
      readOK = NO;
      writeOK = NO;
      [self setAddr: &sin];
    }
  return self;
}

- (id) initForReadingAtPath: (NSString*)path
{
  int	d = open([path fileSystemRepresentation], O_RDONLY|O_BINARY);

  if (d < 0)
    {
#ifdef __ANDROID__
      asset = [NSBundle assetForPath:path withMode:AASSET_MODE_RANDOM];
      if (asset)
	{
	  readOK = YES;
	  return self;
	}
#endif
      
      DESTROY(self);
      return nil;
    }
  else
    {
      self = [self initWithFileDescriptor: d closeOnDealloc: YES];
      if (self)
	{
	  connectOK = NO;
	  acceptOK = NO;
	  writeOK = NO;
	}
      return self;
    }
}

- (id) initForWritingAtPath: (NSString*)path
{
  int	d = open([path fileSystemRepresentation], O_WRONLY|O_BINARY);

  if (d < 0)
    {
      DESTROY(self);
      return nil;
    }
  else
    {
      self = [self initWithFileDescriptor: d closeOnDealloc: YES];
      if (self)
	{
	  connectOK = NO;
	  acceptOK = NO;
	  readOK = NO;
	}
      return self;
    }
}

- (id) initForUpdatingAtPath: (NSString*)path
{
  int	d = open([path fileSystemRepresentation], O_RDWR|O_BINARY);

  if (d < 0)
    {
      DESTROY(self);
      return nil;
    }
  else
    {
      self = [self initWithFileDescriptor: d closeOnDealloc: YES];
      if (self != nil)
	{
	  connectOK = NO;
	  acceptOK = NO;
	}
      return self;
    }
}

- (id) initWithStandardError
{
  if (fh_stderr != nil)
    {
      ASSIGN(self, fh_stderr);
    }
  else
    {
      self = [self initWithFileDescriptor: 2 closeOnDealloc: NO];
      ASSIGN(fh_stderr, self);
      if (self)
	{
	  readOK = NO;
	}
    }
  return self;
}

- (id) initWithStandardInput
{
  if (fh_stdin != nil)
    {
      ASSIGN(self, fh_stdin);
    }
  else
    {
      self = [self initWithFileDescriptor: 0 closeOnDealloc: NO];
      ASSIGN(fh_stdin, self);
      if (self)
	{
	  writeOK = NO;
	}
    }
  return self;
}

- (id) initWithStandardOutput
{
  if (fh_stdout != nil)
    {
      ASSIGN(self, fh_stdout);
    }
  else
    {
      self = [self initWithFileDescriptor: 1 closeOnDealloc: NO];
      ASSIGN(fh_stdout, self);
      if (self)
	{
	  readOK = NO;
	}
    }
  return self;
}

- (id) initWithNullDevice
{
  self = [self initWithFileDescriptor: open("/dev/null", O_RDWR|O_BINARY)
		       closeOnDealloc: YES];
  if (self)
    {
      isNullDevice = YES;
    }
  return self;
}

- (id) initWithFileDescriptor: (int)desc closeOnDealloc: (BOOL)flag
{
  self = [super init];
  if (nil == self)
    {
      if (YES == flag)
	{
	  close(desc);
	}
    }
  else
    {
      struct stat	sbuf;
      int		e;

      if (fstat(desc, &sbuf) < 0)
	{
#if	defined(_WIN32)
	  /* On windows, an fstat will fail if the descriptor is a pipe
	   * or socket, so we simply mark the descriptor as not being a
	   * standard file.
	   */
	  isStandardFile = NO;
#else
	  /* This should never happen on unix.  If it does, we have somehow
	   * ended up with a bad descriptor.
	   */
          NSLog(@"unable to get status of descriptor %d - %@",
	    desc, [NSError _last]);
	  isStandardFile = NO;
#endif
	}
      else
	{
	  if (S_ISREG(sbuf.st_mode))
	    {
	      isStandardFile = YES;
	    }
	  else
	    {
	      isStandardFile = NO;
	    }
	}

      if ((e = fcntl(desc, F_GETFL, 0)) >= 0)
	{
	  if (e & NBLK_OPT)
	    {
	      wasNonBlocking = YES;
	    }
	  else
	    {
	      wasNonBlocking = NO;
	    }
	}

      isNonBlocking = wasNonBlocking;
      descriptor = desc;
      closeOnDealloc = flag;
      readInfo = nil;
      writeInfo = [NSMutableArray new];
      readMax = 0;
      writePos = 0;
      readOK = YES;
      writeOK = YES;
      acceptOK = YES;
      connectOK = YES;
    }
  return self;
}

- (id) initWithNativeHandle: (void*)hdl
{
  return [self initWithFileDescriptor: (uintptr_t)hdl closeOnDealloc: NO];
}

- (id) initWithNativeHandle: (void*)hdl closeOnDealloc: (BOOL)flag
{
  return [self initWithFileDescriptor: (uintptr_t)hdl closeOnDealloc: flag];
}

- (void) checkAccept
{
  if (acceptOK == NO)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"accept not permitted in this file handle"];
    }
  if (readInfo)
    {
      id	operation = [readInfo objectForKey: NotificationKey];

      if (operation == NSFileHandleConnectionAcceptedNotification)
        {
          [NSException raise: NSFileHandleOperationException
                      format: @"accept already in progress"];
	}
      else
	{
          [NSException raise: NSFileHandleOperationException
                      format: @"read already in progress"];
	}
    }
}

- (void) checkConnect
{
  if (connectOK == NO)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"connect not permitted in this file handle"];
    }
  if ([writeInfo count] > 0)
    {
      NSDictionary	*info = [writeInfo objectAtIndex: 0];
      id		operation = [info objectForKey: NotificationKey];

      if (operation == GSFileHandleConnectCompletionNotification)
	{
          [NSException raise: NSFileHandleOperationException
                      format: @"connect already in progress"];
	}
      else
	{
          [NSException raise: NSFileHandleOperationException
                      format: @"write already in progress"];
	}
    }
}

- (void) checkRead
{
  if (readOK == NO)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"read not permitted on this file handle"];
    }
  if (readInfo)
    {
      id	operation = [readInfo objectForKey: NotificationKey];

      if (operation == NSFileHandleConnectionAcceptedNotification)
        {
          [NSException raise: NSFileHandleOperationException
                      format: @"accept already in progress"];
	}
      else
	{
          [NSException raise: NSFileHandleOperationException
                      format: @"read already in progress"];
	}
    }
}

- (void) checkWrite
{
  if (writeOK == NO)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"write not permitted in this file handle"];
    }
  if ([writeInfo count] > 0)
    {
      NSDictionary	*info = [writeInfo objectAtIndex: 0];
      id		operation = [info objectForKey: NotificationKey];

      if (operation != GSFileHandleWriteCompletionNotification)
	{
          [NSException raise: NSFileHandleOperationException
                      format: @"connect in progress"];
	}
    }
}

// Returning file handles

- (int) fileDescriptor
{
  return descriptor;
}

- (void*) nativeHandle
{
  return (void*)(intptr_t)descriptor;
}

// Synchronous I/O operations

- (NSData*) availableData
{
  int			rmax = [tune recvSize];
  char			buf[rmax];
  NSMutableData*	d;
  int			len;

  [self checkRead];
  d = [NSMutableData dataWithCapacity: 0];
  if (isStandardFile)
    {
      if (isNonBlocking == YES)
	{
	  [self setNonBlocking: NO];
	}
      while ((len = [self read: buf length: sizeof(buf)]) > 0)
        {
	  [d appendBytes: buf length: len];
        }
    }
  else
    {
      if (isNonBlocking == NO)
	{
	  [self setNonBlocking: YES];
	}
      len = [self read: buf length: sizeof(buf)];

      if (len <= 0)
	{
	  if (errno == EAGAIN || errno == EINTR)
	    {
	      /*
	       * Read would have blocked ... so try to get a single character
	       * in non-blocking mode (to ensure we wait until data arrives)
	       * and then try again.
	       * This ensures that we block for *some* data as we should.
	       */
	      [self setNonBlocking: NO];
	      len = [self read: buf length: 1];
	      [self setNonBlocking: YES];
	      if (len == 1)
		{
		  len = [self read: &buf[1] length: sizeof(buf) - 1];
		  if (len <= 0)
		    {
		      len = 1;
		    }
		  else
		    {
		      len = len + 1;
		    }
		}
	    }
	}

      if (len > 0)
	{
	  [d appendBytes: buf length: len];
	}
    }
  if (len < 0)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"unable to read from descriptor - %@",
                  [NSError _last]];
    }
  return d;
}

- (NSData*) readDataToEndOfFile
{
  int			rmax = [tune recvSize];
  char			buf[rmax];
  NSMutableData*	d;
  int			len;

  [self checkRead];
  if (isNonBlocking == YES)
    {
      [self setNonBlocking: NO];
    }
  d = [NSMutableData dataWithCapacity: 0];
  while ((len = [self read: buf length: sizeof(buf)]) > 0)
    {
      [d appendBytes: buf length: len];
    }
  if (len < 0)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"unable to read from descriptor - %@",
                  [NSError _last]];
    }
  return d;
}

- (NSData*) readDataOfLength: (unsigned)len
{
  NSMutableData	*d;
  int		got;
  int		rmax = [tune recvSize];
  char		buf[rmax];

  [self checkRead];
  if (isNonBlocking == YES)
    {
      [self setNonBlocking: NO];
    }

  d = [NSMutableData dataWithCapacity: len < sizeof(buf) ? len : sizeof(buf)];
  do
    {
      int	chunk = len > sizeof(buf) ? sizeof(buf) : len;

      got = [self read: buf length: chunk];
      if (got > 0)
	{
	  [d appendBytes: buf length: got];
	  len -= got;
	}
      else if (got < 0)
	{
	  [NSException raise: NSFileHandleOperationException
		      format: @"unable to read from descriptor - %@",
		      [NSError _last]];
	}
    }
  while (len > 0 && got > 0);

  return d;
}

- (void) writeData: (NSData*)item
{
  int		rval = 0;
  const void*	ptr = [item bytes];
  unsigned int	len = [item length];
  unsigned int	pos = 0;

  [self checkWrite];
  if (isNonBlocking == YES)
    {
      [self setNonBlocking: NO];
    }
  while (pos < len)
    {
      int	toWrite = len - pos;

      toWrite = [tune sendSize: toWrite];
      rval = [self write: (char*)ptr+pos length: toWrite];
      if (rval < 0)
	{
	  if (errno == EAGAIN || errno == EINTR)
	    {
	      rval = 0;
	    }
	  else
	    {
	      break;
	    }
	}
      pos += rval;
    }
  if (rval < 0)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"unable to write to descriptor - %@",
                  [NSError _last]];
    }
}


// Asynchronous I/O operations

- (void) acceptConnectionInBackgroundAndNotifyForModes: (NSArray*)modes
{
  [self checkAccept];
  readMax = 0;
  RELEASE(readInfo);
  readInfo = [[NSMutableDictionary alloc] initWithCapacity: 4];
  [readInfo setObject: NSFileHandleConnectionAcceptedNotification
	       forKey: NotificationKey];
  [self watchReadDescriptorForModes: modes];
}

- (void) readDataInBackgroundAndNotifyLength: (unsigned)len
				    forModes: (NSArray*)modes
{
  NSMutableData	*d;

  [self checkRead];
  if (len > 0x7fffffff)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"length (%u) too large", len];
    }
  readMax = len;
  RELEASE(readInfo);
  readInfo = [[NSMutableDictionary alloc] initWithCapacity: 4];
  [readInfo setObject: NSFileHandleReadCompletionNotification
	       forKey: NotificationKey];
  d = [[NSMutableData alloc] initWithCapacity: readMax];
  [readInfo setObject: d forKey: NSFileHandleNotificationDataItem];
  RELEASE(d);
  [self watchReadDescriptorForModes: modes];
}

- (void) readInBackgroundAndNotifyForModes: (NSArray*)modes
{
  NSMutableData	*d;

  [self checkRead];
  readMax = -1;		// Accept any quantity of data.
  RELEASE(readInfo);
  readInfo = [[NSMutableDictionary alloc] initWithCapacity: 4];
  [readInfo setObject: NSFileHandleReadCompletionNotification
	       forKey: NotificationKey];
  d = [[NSMutableData alloc] initWithCapacity: 0];
  [readInfo setObject: d forKey: NSFileHandleNotificationDataItem];
  RELEASE(d);
  [self watchReadDescriptorForModes: modes];
}

- (void) readToEndOfFileInBackgroundAndNotifyForModes: (NSArray*)modes
{
  NSMutableData	*d;

  [self checkRead];
  readMax = 0;
  RELEASE(readInfo);
  readInfo = [[NSMutableDictionary alloc] initWithCapacity: 4];
  [readInfo setObject: NSFileHandleReadToEndOfFileCompletionNotification
	       forKey: NotificationKey];
  d = [[NSMutableData alloc] initWithCapacity: 0];
  [readInfo setObject: d forKey: NSFileHandleNotificationDataItem];
  RELEASE(d);
  [self watchReadDescriptorForModes: modes];
}

- (void) waitForDataInBackgroundAndNotifyForModes: (NSArray*)modes
{
  [self checkRead];
  readMax = 0;
  RELEASE(readInfo);
  readInfo = [[NSMutableDictionary alloc] initWithCapacity: 4];
  [readInfo setObject: NSFileHandleDataAvailableNotification
	       forKey: NotificationKey];
  [readInfo setObject: [NSMutableData dataWithCapacity: 0]
	       forKey: NSFileHandleNotificationDataItem];
  [self watchReadDescriptorForModes: modes];
}

// Seeking within a file

- (unsigned long long) offsetInFile
{
  off_t	result = -1;

#ifdef __ANDROID__
  if (asset)
    {
      result = AAsset_seek(asset, 0, SEEK_CUR);
    }
  else
#endif
  if (isStandardFile && descriptor >= 0)
    {
#if	USE_ZLIB
      if (gzDescriptor != 0)
	{
	  result = gzseek(gzDescriptor, 0, SEEK_CUR);
	}
      else
#endif
      result = lseek(descriptor, 0, SEEK_CUR);
    }
  if (result < 0)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"failed to move to offset in file - %@",
                  [NSError _last]];
    }
  return (unsigned long long)result;
}

- (unsigned long long) seekToEndOfFile
{
  off_t	result = -1;

#ifdef __ANDROID__
  if (asset)
    {
      result = AAsset_seek(asset, 0, SEEK_END);
    }
  else
#endif
  if (isStandardFile && descriptor >= 0)
    {
#if	USE_ZLIB
      if (gzDescriptor != 0)
	{
	  result = gzseek(gzDescriptor, 0, SEEK_END);
	}
      else
#endif
      result = lseek(descriptor, 0, SEEK_END);
    }
  if (result < 0)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"failed to move to offset in file - %@",
                  [NSError _last]];
    }
  return (unsigned long long)result;
}

- (void) seekToFileOffset: (unsigned long long)pos
{
  off_t	result = -1;

#ifdef __ANDROID__
  if (asset)
    {
      result = AAsset_seek(asset, (off_t)pos, SEEK_SET);
    }
  else
#endif
  if (isStandardFile && descriptor >= 0)
    {
#if	USE_ZLIB
      if (gzDescriptor != 0)
	{
	  result = gzseek(gzDescriptor, (off_t)pos, SEEK_SET);
	}
      else
#endif
      result = lseek(descriptor, (off_t)pos, SEEK_SET);
    }
  if (result < 0)
    {
      [NSException raise: NSFileHandleOperationException
                  format: @"failed to move to offset in file - %@",
                  [NSError _last]];
    }
}


// Operations on file

- (void) closeFile
{
  if (descriptor < 0)
    {
      [NSException raise: NSFileHandleOperationException
		  format: @"attempt to close closed file"];
    }
  [self ignoreReadDescriptor];
  [self ignoreWriteDescriptor];

  [self setNonBlocking: NO];
  
#ifdef __ANDROID__
  if (asset)
    {
      AAsset_close(asset);
      asset = NULL;
    }
  else
#endif
#if	USE_ZLIB
  if (gzDescriptor != 0)
    {
      gzclose(gzDescriptor);
      gzDescriptor = 0;
    }
#endif
  if (YES == isSocket)
    {
      int       milli = [tune delay];

      shutdown(descriptor, SHUT_WR);
      if (milli > 0)
        {
          NSTimeInterval        until;

          until = [NSDate timeIntervalSinceReferenceDate];
          until += ((double)milli) / 1000.0;

          [self setNonBlocking: YES];
          while ([NSDate timeIntervalSinceReferenceDate] < until)
            {
              int       result;
              char      buffer[4096];

              result = read(descriptor, buffer, sizeof(buffer));
              if (result <= 0)
                {
                  if (result < 0)
                    {
                      if (EAGAIN == errno || EINTR == errno)
                        {
                          continue;
                        }
                      NSLog(@"%@ read fail on socket shutdown: %@",
                        self, [NSError _last]);
                    }
                  break;
                }
            }
          [self setNonBlocking: YES];
        }
    }
  (void)close(descriptor);
  descriptor = -1;
  acceptOK = NO;
  connectOK = NO;
  readOK = NO;
  writeOK = NO;

  /*
   *    Clear any pending operations on the file handle, sending
   *    notifications if necessary.
   */
  if (readInfo)
    {
      [readInfo setObject: @"File handle closed locally"
                   forKey: GSFileHandleNotificationError];
      [self postReadNotification];
    }

  if ([writeInfo count])
    {
      NSMutableDictionary       *info = [writeInfo objectAtIndex: 0];

      [info setObject: @"File handle closed locally"
               forKey: GSFileHandleNotificationError];
      [self postWriteNotification];
      [writeInfo removeAllObjects];
    }
}

- (void) synchronizeFile
{
  if (isStandardFile)
    {
      (void)sync();
    }
}

- (void) truncateFileAtOffset: (unsigned long long)pos
{
  if (isStandardFile && descriptor >= 0)
    {
      (void)ftruncate(descriptor, pos);
    }
  [self seekToFileOffset: pos];
}

- (void) writeInBackgroundAndNotify: (NSData*)item forModes: (NSArray*)modes
{
  NSMutableDictionary*	info;

  [self checkWrite];

  info = [[NSMutableDictionary alloc] initWithCapacity: 4];
  [info setObject: item forKey: NSFileHandleNotificationDataItem];
  [info setObject: GSFileHandleWriteCompletionNotification
		forKey: NotificationKey];
  if (modes != nil)
    {
      [info setObject: modes forKey: NSFileHandleNotificationMonitorModes];
    }
  [writeInfo addObject: info];
  RELEASE(info);
  [self watchWriteDescriptor];
}

- (void) writeInBackgroundAndNotify: (NSData*)item;
{
  [self writeInBackgroundAndNotify: item forModes: nil];
}

- (void) postReadNotification
{
  NSMutableDictionary	*info = readInfo;
  NSNotification	*n;
  NSNotificationQueue	*q;
  NSArray		*modes;
  NSString		*name;

  [self ignoreReadDescriptor];
  readInfo = nil;
  readMax = 0;
  modes = (NSArray*)[info objectForKey: NSFileHandleNotificationMonitorModes];
  name = (NSString*)[info objectForKey: NotificationKey];

  if (name == nil)
    {
      return;
    }
  n = [NSNotification notificationWithName: name object: self userInfo: info];

  RELEASE(info);	/* Retained by the notification.	*/

  q = [NSNotificationQueue defaultQueue];
  [q enqueueNotification: n
	    postingStyle: NSPostASAP
	    coalesceMask: NSNotificationNoCoalescing
		forModes: modes];
}

- (void) postWriteNotification
{
  NSMutableDictionary	*info = [writeInfo objectAtIndex: 0];
  NSNotificationQueue	*q;
  NSNotification	*n;
  NSArray		*modes;
  NSString		*name;

  [self ignoreWriteDescriptor];
  modes = (NSArray*)[info objectForKey: NSFileHandleNotificationMonitorModes];
  name = (NSString*)[info objectForKey: NotificationKey];

  n = [NSNotification notificationWithName: name object: self userInfo: info];

  writePos = 0;
  [writeInfo removeObjectAtIndex: 0];	/* Retained by notification.	*/

  q = [NSNotificationQueue defaultQueue];
  [q enqueueNotification: n
	    postingStyle: NSPostASAP
	    coalesceMask: NSNotificationNoCoalescing
		forModes: modes];
  if ((writeOK || connectOK) && [writeInfo count] > 0)
    {
      [self watchWriteDescriptor];	/* In case of queued writes.	*/
    }
}

- (BOOL) readInProgress
{
  if (readInfo)
    {
      return YES;
    }
  return NO;
}

- (BOOL) writeInProgress
{
  if ([writeInfo count] > 0)
    {
      return YES;
    }
  return NO;
}

- (void) ignoreReadDescriptor
{
  NSRunLoop	*l;
  NSArray	*modes;

  if (descriptor < 0)
    {
      return;
    }
  l = [NSRunLoop currentRunLoop];
  modes = nil;

  if (readInfo)
    {
      modes = (NSArray*)[readInfo objectForKey:
	NSFileHandleNotificationMonitorModes];
    }

  if (modes && [modes count])
    {
      unsigned int	i;

      for (i = 0; i < [modes count]; i++)
	{
	  [l removeEvent: (void*)(uintptr_t)descriptor
		    type: ET_RDESC
		 forMode: [modes objectAtIndex: i]
		     all: YES];
        }
    }
  else
    {
      [l removeEvent: (void*)(uintptr_t)descriptor
		type: ET_RDESC
	     forMode: NSDefaultRunLoopMode
		 all: YES];
    }
}

- (void) ignoreWriteDescriptor
{
  NSRunLoop	*l;
  NSArray	*modes;

  if (descriptor < 0)
    {
      return;
    }
  l = [NSRunLoop currentRunLoop];
  modes = nil;

  if ([writeInfo count] > 0)
    {
      NSMutableDictionary	*info = [writeInfo objectAtIndex: 0];

      modes = [info objectForKey: NSFileHandleNotificationMonitorModes];
    }

  if (modes && [modes count])
    {
      unsigned int	i;

      for (i = 0; i < [modes count]; i++)
	{
	  [l removeEvent: (void*)(uintptr_t)descriptor
		    type: ET_WDESC
		 forMode: [modes objectAtIndex: i]
		     all: YES];
        }
    }
  else
    {
      [l removeEvent: (void*)(uintptr_t)descriptor
		type: ET_WDESC
	     forMode: NSDefaultRunLoopMode
		 all: YES];
    }
}

- (void) watchReadDescriptorForModes: (NSArray*)modes;
{
  NSRunLoop	*l;

  if (descriptor < 0)
    {
      return;
    }

  l = [NSRunLoop currentRunLoop];
  [self setNonBlocking: YES];
  if (modes && [modes count])
    {
      unsigned int	i;

      for (i = 0; i < [modes count]; i++)
	{
	  [l addEvent: (void*)(uintptr_t)descriptor
		 type: ET_RDESC
	      watcher: self
	      forMode: [modes objectAtIndex: i]];
        }
      [readInfo setObject: modes forKey: NSFileHandleNotificationMonitorModes];
    }
  else
    {
      [l addEvent: (void*)(uintptr_t)descriptor
	     type: ET_RDESC
	  watcher: self
	  forMode: NSDefaultRunLoopMode];
    }
}

- (void) watchWriteDescriptor
{
  if (descriptor < 0)
    {
      return;
    }
  if ([writeInfo count] > 0)
    {
      NSMutableDictionary	*info = [writeInfo objectAtIndex: 0];
      NSRunLoop			*l = [NSRunLoop currentRunLoop];
      NSArray			*modes = nil;

      modes = [info objectForKey: NSFileHandleNotificationMonitorModes];

      [self setNonBlocking: YES];
      if (modes && [modes count])
	{
	  unsigned int	i;

	  for (i = 0; i < [modes count]; i++)
	    {
	      [l addEvent: (void*)(uintptr_t)descriptor
		     type: ET_WDESC
		  watcher: self
		  forMode: [modes objectAtIndex: i]];
	    }
	}
      else
	{
	  [l addEvent: (void*)(uintptr_t)descriptor
		 type: ET_WDESC
	      watcher: self
	      forMode: NSDefaultRunLoopMode];
	}
    }
}

- (void) receivedEventRead
{
  NSString	*operation;

  operation = [readInfo objectForKey: NotificationKey];
  if (operation == NSFileHandleConnectionAcceptedNotification)
    {
      struct sockaddr	buf;
      int			desc;
      unsigned int		blen = sizeof(buf);

      desc = accept(descriptor, &buf, &blen);
      if (desc == -1)
	{
	  NSString	*s;

	  s = [NSString stringWithFormat: @"Accept attempt failed - %@",
	    [NSError _last]];
	  [readInfo setObject: s forKey: GSFileHandleNotificationError];
	}
      else
	{ // Accept attempt completed.
	  GSFileHandle		*h;
	  struct sockaddr	sin;
	  unsigned int		size = sizeof(sin);

          [tune tune: (void*)(intptr_t)desc];
        
	  h = [[[self class] alloc] initWithFileDescriptor: desc
					    closeOnDealloc: YES];
	  h->isSocket = YES;
	  if (getpeername(desc, &sin, &size) >= 0)
            {
              [h setAddr: &sin];
            }
	  [readInfo setObject: h
                       forKey: NSFileHandleNotificationFileHandleItem];
	  RELEASE(h);
	}
      [self postReadNotification];
    }
  else if (operation == NSFileHandleDataAvailableNotification)
    {
      [self postReadNotification];
    }
  else
    {
      NSMutableData	*item;
      int		length;
      int		received = 0;
      int		rmax = [tune recvSize];
      char		buf[rmax];

      item = [readInfo objectForKey: NSFileHandleNotificationDataItem];
      /*
       * We may have a maximum data size set...
       */
      if (readMax > 0)
        {
          length = (unsigned int)readMax - [item length];
          if (length > (int)sizeof(buf))
            {
	      length = sizeof(buf);
	    }
	}
      else
	{
	  length = sizeof(buf);
	}

      received = [self read: buf length: length];
      if (received == 0)
        { // Read up to end of file.
          [self postReadNotification];
        }
      else if (received < 0)
        {
          if (errno != EAGAIN && errno != EINTR)
            {
	      NSString	*s;

	      s = [NSString stringWithFormat: @"Read attempt failed - %@",
		[NSError _last]];
	      [readInfo setObject: s forKey: GSFileHandleNotificationError];
	      [self postReadNotification];
	    }
	}
      else
	{
	  [item appendBytes: buf length: received];
	  if (readMax < 0 || (readMax > 0 && (int)[item length] == readMax))
	    {
	      // Read a single chunk of data
	      [self postReadNotification];
	    }
	}
    }
}

- (void) receivedEventWrite
{
  NSString	*operation;
  NSMutableDictionary	*info;

  info = [writeInfo objectAtIndex: 0];
  operation = [info objectForKey: NotificationKey];
  if (operation == GSFileHandleConnectCompletionNotification
    || operation == GSSOCKSConnect)
    { // Connection attempt completed.
      int	result;
      int	rval;
      unsigned	len = sizeof(result);

      rval = getsockopt(descriptor, SOL_SOCKET, SO_ERROR, (char*)&result, &len);
      if (rval != 0)
        {
          NSString	*s;

          s = [NSString stringWithFormat: @"Connect attempt failed - %@",
	    [NSError _last]];
          [info setObject: s forKey: GSFileHandleNotificationError];
	}
      else if (result != 0)
        {
          NSString	*s;

          s = [NSString stringWithFormat: @"Connect attempt failed - %@",
	    [NSError _systemError: result]];
          [info setObject: s forKey: GSFileHandleNotificationError];
        }
      else
        {
          readOK = YES;
          writeOK = YES;
        }
      connectOK = NO;
      [self postWriteNotification];
    }
  else
    {
      NSData	*item;
      int		length;
      const void	*ptr;

      item = [info objectForKey: NSFileHandleNotificationDataItem];
      length = [item length];
      ptr = [item bytes];
      if (writePos < length)
        {
          int	written;

          written = [self write: (char*)ptr+writePos
                         length: length-writePos];
          if (written <= 0)
            {
	      if (written < 0 && errno != EAGAIN && errno != EINTR)
	        {
	          NSString	*s;

	          s = [NSString stringWithFormat:
		    @"Write attempt failed - %@", [NSError _last]];
	          [info setObject: s forKey: GSFileHandleNotificationError];
	          [self postWriteNotification];
	        }
	    }
	  else
            {
	      writePos += written;
	    }
	}
      if (writePos >= length)
        { // Write operation completed.
          [self postWriteNotification];
        }
    }
}

- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
		 extra: (void*)extra
	       forMode: (NSString*)mode
{
  NSDebugMLLog(@"NSFileHandle", @"%@ event: %d", self, type);

  if (isNonBlocking == NO)
    {
      [self setNonBlocking: YES];
    }
  if (type == ET_RDESC)
    {
      [self receivedEventRead];
    }
  else
    {
      [self receivedEventWrite];
    }
}

- (void) setAddr: (struct sockaddr *)sin
{
  NSString	*s;

  ASSIGN(address, GSPrivateSockaddrHost(sin));
  s = [NSString stringWithFormat: @"%d", GSPrivateSockaddrPort(sin)];
  ASSIGN(service, s);
  protocol = @"tcp";
}

- (void) setNonBlocking: (BOOL)flag
{
  if (descriptor < 0)
    {
      return;
    }
  else if (isStandardFile == YES)
    {
      return;
    }
  else if (isNonBlocking == flag)
    {
      return;
    }
  else
    {
      int	e;

      if ((e = fcntl(descriptor, F_GETFL, 0)) >= 0)
	{
	  if (flag == YES)
	    {
	      e |= NBLK_OPT;
	    }
	  else
	    {
	      e &= ~NBLK_OPT;
	    }
	  if (fcntl(descriptor, F_SETFL, e) < 0)
	    {
	      NSLog(@"unable to set non-blocking mode for %d - %@",
		descriptor, [NSError _last]);
	    }
	  else
	    {
	      isNonBlocking = flag;
	    }
	}
      else
	{
	  NSLog(@"unable to get non-blocking mode for %d - %@",
	    descriptor, [NSError _last]);
	}
    }
}

- (NSString*) socketAddress
{
  return address;
}

- (NSString*) socketLocalAddress
{
  NSString	*str = nil;
  struct sockaddr sin;
  unsigned	size = sizeof(sin);

  if (getsockname(descriptor, &sin, &size) == -1)
    {
      NSLog(@"unable to get socket name - %@", [NSError _last]);
    }
  else
    {
      str = GSPrivateSockaddrHost(&sin);
    }
  return str;
}

- (NSString*) socketLocalService
{
  NSString	*str = nil;
  struct sockaddr sin;
  unsigned	size = sizeof(sin);

  if (getsockname(descriptor, &sin, &size) == -1)
    {
      NSLog(@"unable to get socket name - %@", [NSError _last]);
    }
  else
    {
      str = [NSString stringWithFormat: @"%d", GSPrivateSockaddrPort(&sin)];
    }
  return str;
}

- (NSString*) socketProtocol
{
  return protocol;
}

- (NSString*) socketService
{
  return service;
}

- (BOOL) useCompression
{
#if	USE_ZLIB
  int	d;

  if (gzDescriptor != 0)
    {
      return YES;	// Already open
    }
  if (descriptor < 0)
    {
      return NO;	// No descriptor available.
    }
  if (readOK == YES && writeOK == YES)
    {
      return NO;	// Can't both read and write.
    }
  d = dup(descriptor);  // d is closed by gzclose() later.
  if (d < 0)
    {
      return NO;	// No descriptor available.
    }
  if (readOK == YES)
    {
      gzDescriptor = gzdopen(d, "rb");
    }
  else
    {
      gzDescriptor = gzdopen(d, "wb");
    }
  if (gzDescriptor == 0)
    {
      close(d);
      return NO;	// Open attempt failed.
    }
  return YES;
#endif
  return NO;
}
@end

