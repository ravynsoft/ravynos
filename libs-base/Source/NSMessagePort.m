/** Implementation of network port object based on unix domain sockets
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Based on code by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>

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
#define	EXPOSE_NSPort_IVARS	1
#define	EXPOSE_NSMessagePort_IVARS	1
#import "GNUstepBase/GSLock.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSException.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSPortMessage.h"
#import "Foundation/NSPortNameServer.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSConnection.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSProcessInfo.h"

#import "GSPrivate.h"
#import "GSNetwork.h"
#import "GSPortPrivate.h"

#include <stdio.h>

#include <sys/param.h>		/* for MAXHOSTNAMELEN */
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>		/* for inet_ntoa() */
#include <ctype.h>		/* for strchr() */

#if	defined(HAVE_SYS_FCNTL_H)
#  include	<sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include	<fcntl.h>
#endif

#include <sys/time.h>
#include <sys/resource.h>
#include <netdb.h>
#include <sys/socket.h>

#if	defined(HAVE_SYS_FILE_H)
#  include	<sys/file.h>
#endif

#include <sys/stat.h>

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

#include <netinet/in.h>
#include <net/if.h>
#if	!defined(SIOCGIFCONF) || defined(__CYGWIN__)
#include <sys/ioctl.h>
#ifndef	SIOCGIFCONF
#include <sys/sockio.h>
#endif
#endif

#if	defined(__svr4__)
#  if defined(HAVE_SYS_STROPTS)
#    include <sys/stropts.h>
#  endif
#endif

@interface NSProcessInfo (private)
+ (BOOL) _exists: (int)pid;
@end

/*
 * Largest chunk of data possible in DO
 */
static uint32_t	maxDataLength = 32 * 1024 * 1024;

#if 0
#define	M_LOCK(X) {NSDebugMLLog(@"NSMessagePort",@"lock %@",X); [X lock];}
#define	M_UNLOCK(X) {NSDebugMLLog(@"NSMessagePort",@"unlock %@",X); [X unlock];}
#else
#define	M_LOCK(X) {[X lock];}
#define	M_UNLOCK(X) {[X unlock];}
#endif


/*
 * The GSPortItemType constant is used to identify the type of data in
 * each packet read.  All data transmitted is in a packet, each packet
 * has an initial packet type and packet length.
 */
typedef	enum {
  GSP_NONE,
  GSP_PORT,		/* Simple port item.			*/
  GSP_DATA,		/* Simple data item.			*/
  GSP_HEAD		/* Port message header + initial data.	*/
} GSPortItemType;

/*
 * The GSPortItemHeader structure defines the header for each item transmitted.
 * Its contents are transmitted in network byte order.
 */
typedef struct {
  uint32_t	type;	/* A GSPortItemType as a 4-byte number.		*/
  uint32_t	length;	/* The length of the item (excluding header).	*/
} GSPortItemHeader;

/*
 * The GSPortMsgHeader structure is at the start of any item of type GSP_HEAD.
 * Its contents are transmitted in network byte order.
 * Any additional data in the item is an NSData object.
 * NB. additional data counts as part of the same item.
 */
typedef struct {
  uint32_t	mId;	/* The ID for the message starting with this.	*/
  uint32_t	nItems;	/* Number of items (including this one).	*/
} GSPortMsgHeader;

typedef	struct {
  unsigned char	version;
  unsigned char	addr[0];	/* name of the port on the local host	*/
} GSPortInfo;

/*
 * Utility functions for encoding and decoding ports.
 */
static NSMessagePort*
decodePort(NSData *data)
{
  GSPortItemHeader	*pih;
  GSPortInfo		*pi;

  pih = (GSPortItemHeader*)[data bytes];
  NSCAssert(GSSwapBigI32ToHost(pih->type) == GSP_PORT,
    NSInternalInconsistencyException);
  pi = (GSPortInfo*)&pih[1];
  if (pi->version != 0)
    {
      NSLog(@"Remote version of GNUstep is more recent than this one (%i)",
	pi->version);
      return nil;
    }

  NSDebugFLLog(@"NSMessagePort", @"Decoded port as '%s'", pi->addr);

  return [NSMessagePort _portWithName: pi->addr
			     listener: NO];
}

static NSData*
newDataWithEncodedPort(NSMessagePort *port)
{
  GSPortItemHeader	*pih;
  GSPortInfo		*pi;
  NSMutableData		*data;
  unsigned		plen;
  const unsigned char	*name = [port _name];

  plen = 2 + strlen((char*)name);

  data = [[NSMutableData alloc] initWithLength: sizeof(GSPortItemHeader)+plen];
  pih = (GSPortItemHeader*)[data mutableBytes];
  pih->type = GSSwapHostI32ToBig(GSP_PORT);
  pih->length = GSSwapHostI32ToBig(plen);
  pi = (GSPortInfo*)&pih[1];
  strncpy((char*)pi->addr, (char*)name, strlen((char*)name) + 1);

  NSDebugFLLog(@"NSMessagePort", @"Encoded port as '%s'", pi->addr);

  return data;
}

/* Older systems (Solaris) compatibility */
#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#define PF_LOCAL PF_UNIX
#endif
#ifndef SUN_LEN
#define SUN_LEN(su) \
	(sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

#define	GS_CONNECTION_MSG	0
#define	NETBLOCK	8192

/*
 * Theory of operation
 *
 *
 */


/* Private interfaces */

/*
 * Here is how data is transmitted over a socket -
 * Initially the process making the connection sends an item of type
 * GSP_PORT to tell the remote end what port is connecting to it.
 * Therafter, all communication is via port messages.  Each port message
 * consists of an item of type GSP_HEAD followed by zero or more items
 * of type GSP_PORT or GSP_DATA.  The number of items in a port message
 * is encoded in the 'nItems' field of the header.
 */

typedef enum {
  GS_H_UNCON = 0,	// Currently idle and unconnected.
  GS_H_TRYCON,		// Trying connection (outgoing).
  GS_H_ACCEPT,		// Making initial connection (incoming).
  GS_H_CONNECTED	// Currently connected.
} GSHandleState;

@interface GSMessageHandle : NSObject <RunLoopEvents>
{
  int			desc;		/* File descriptor for I/O.	*/
  unsigned		wItem;		/* Index of item being written.	*/
  NSMutableData		*wData;		/* Data object being written.	*/
  unsigned		wLength;	/* Ammount written so far.	*/
  NSMutableArray	*wMsgs;		/* Message in progress.		*/
  NSMutableData		*rData;		/* Buffer for incoming data	*/
  uint32_t		rLength;	/* Amount read so far.		*/
  uint32_t		rWant;		/* Amount desired.		*/
  NSMutableArray	*rItems;	/* Message in progress.		*/
  GSPortItemType	rType;		/* Type of data being read.	*/
  uint32_t		rId;		/* Id of incoming message.	*/
  unsigned		nItems;		/* Number of items to be read.	*/
  GSHandleState		state;		/* State of the handle.		*/
  unsigned int		addrNum;	/* Address number within host.	*/
@public
  NSRecursiveLock	*myLock;	/* Lock for this handle.	*/
  BOOL			caller;		/* Did we connect to other end?	*/
  BOOL			valid;
  NSMessagePort		*recvPort;
  NSMessagePort		*sendPort;
  struct sockaddr_un 	sockAddr;	/* Far end of connection.	*/
}

+ (GSMessageHandle*) handleWithDescriptor: (int)d;
- (BOOL) connectToPort: (NSMessagePort*)aPort beforeDate: (NSDate*)when;
- (int) descriptor;
- (void) invalidate;
- (BOOL) isValid;
- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
		 extra: (void*)extra
	       forMode: (NSString*)mode;
- (NSMessagePort*) recvPort;
- (BOOL) sendMessage: (NSArray*)components beforeDate: (NSDate*)when;
- (NSMessagePort*) sendPort;
- (void) setState: (GSHandleState)s;
- (GSHandleState) state;
@end



@implementation	GSMessageHandle

static Class	mutableArrayClass;
static Class	mutableDataClass;
static Class	portMessageClass;
static Class	runLoopClass;


+ (id) allocWithZone: (NSZone*)zone
{
  [NSException raise: NSGenericException
	      format: @"attempt to alloc a GSMessageHandle!"];
  return nil;
}

+ (GSMessageHandle*) handleWithDescriptor: (int)d
{
  GSMessageHandle	*handle;
  int		e;

  if (d < 0)
    {
      NSLog(@"illegal descriptor (%d) for message handle", d);
      return nil;
    }
  if ((e = fcntl(d, F_GETFL, 0)) >= 0)
    {
      e |= NBLK_OPT;
      if (fcntl(d, F_SETFL, e) < 0)
	{
	  NSLog(@"unable to set non-blocking mode on %d - %@",
	    d, [NSError _last]);
	  return nil;
	}
    }
  else
    {
      NSLog(@"unable to get non-blocking mode on %d - %@",
	d, [NSError _last]);
      return nil;
    }
  handle = (GSMessageHandle*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  handle->desc = d;
  handle->wMsgs = [NSMutableArray new];
  handle->myLock = [NSRecursiveLock new];
  handle->valid = YES;
  return AUTORELEASE(handle);
}

+ (void) initialize
{
  if (self == [GSMessageHandle class])
    {
      mutableArrayClass = [NSMutableArray class];
      mutableDataClass = [NSMutableData class];
      portMessageClass = [NSPortMessage class];
      runLoopClass = [NSRunLoop class];
    }
}

- (void) _add: (NSRunLoop*)l
{
  [l addEvent: (void*)(uintptr_t)desc
	 type: ET_WDESC
      watcher: self
      forMode: NSConnectionReplyMode];
  [l addEvent: (void*)(uintptr_t)desc
	 type: ET_WDESC
      watcher: self
      forMode: NSDefaultRunLoopMode];
}

- (void) _rem: (NSRunLoop*)l
{
  [l removeEvent: (void*)(uintptr_t)desc
	    type: ET_WDESC
	 forMode: NSConnectionReplyMode
	     all: NO];
  [l removeEvent: (void*)(uintptr_t)desc
	    type: ET_WDESC
	 forMode: NSDefaultRunLoopMode
	     all: NO];
}

- (BOOL) connectToPort: (NSMessagePort*)aPort beforeDate: (NSDate*)when
{
  NSRunLoop		*l;
  const unsigned char *name;

  M_LOCK(myLock);
  NSDebugMLLog(@"NSMessagePort",
    @"Connecting on 0x%"PRIxPTR" before %@", (NSUInteger)self, when);
  if (state != GS_H_UNCON)
    {
      BOOL	result;

      if (state == GS_H_CONNECTED)	/* Already connected.	*/
	{
	  NSLog(@"attempting connect on connected handle");
	  result = YES;
	}
      else if (state == GS_H_ACCEPT)	/* Impossible.	*/
	{
	  NSLog(@"attempting connect with accepting handle");
	  result = NO;
	}
      else				/* Already connecting.	*/
	{
	  NSLog(@"attempting connect while connecting");
	  result = NO;
	}
      M_UNLOCK(myLock);
      return result;
    }

  if (recvPort == nil || aPort == nil)
    {
      NSLog(@"attempting connect with port(s) unset");
      M_UNLOCK(myLock);
      return NO;	/* impossible.		*/
    }

  name = [aPort _name];
  memset(&sockAddr, '\0', sizeof(sockAddr));
  sockAddr.sun_family = AF_LOCAL;
  strncpy(sockAddr.sun_path, (char*)name, sizeof(sockAddr.sun_path) - 1);

  if (connect(desc, (struct sockaddr*)&sockAddr, SUN_LEN(&sockAddr)) < 0)
    {
      if (!GSWOULDBLOCK)
	{
	  NSLog(@"unable to make connection to %s - %@",
	    sockAddr.sun_path, [NSError _last]);
	  M_UNLOCK(myLock);
	  return NO;
	}
    }

  state = GS_H_TRYCON;
  l = [NSRunLoop currentRunLoop];
  [self _add: l];

  while (valid == YES && state == GS_H_TRYCON
    && [when timeIntervalSinceNow] > 0)
    {
      [l runMode: NSConnectionReplyMode beforeDate: when];
    }

  [self _rem: l];

  if (state == GS_H_TRYCON)
    {
      state = GS_H_UNCON;
      addrNum = 0;
      M_UNLOCK(myLock);
      return NO;	/* Timed out 	*/
    }
  else if (state == GS_H_UNCON)
    {
      addrNum = 0;
      state = GS_H_UNCON;
      M_UNLOCK(myLock);
      return NO;
    }
  else
    {
      int	status = 1;

      if (setsockopt(desc, SOL_SOCKET, SO_KEEPALIVE, (char*)&status,
	sizeof(status)) < 0)
        {
          NSLog(@"failed to turn on keepalive for connected socket %d", desc);
        }
      addrNum = 0;
      caller = YES;
      [aPort addHandle: self forSend: YES];
      M_UNLOCK(myLock);
      return YES;
    }
}

- (void) dealloc
{
  [self finalize];
  DESTROY(rData);
  DESTROY(rItems);
  DESTROY(wMsgs);
  DESTROY(myLock);
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"<GSMessageHandle %p (%d) to %s>",
    self, desc, sockAddr.sun_path];
}

- (int) descriptor
{
  return desc;
}

- (void) finalize
{
  [self invalidate];
  (void)close(desc);
  desc = -1;
}

- (void) invalidate
{
  if (valid == YES)
    {
      M_LOCK(myLock);
      if (valid == YES)
	{
	  NSRunLoop	*l;

	  valid = NO;
	  l = [runLoopClass currentRunLoop];
	  [l removeEvent: (void*)(uintptr_t)desc
		    type: ET_RDESC
		 forMode: nil
		     all: YES];
	  [l removeEvent: (void*)(uintptr_t)desc
		    type: ET_WDESC
		 forMode: nil
		     all: YES];
	  NSDebugMLLog(@"NSMessagePort",
	    @"invalidated 0x%"PRIxPTR, (NSUInteger)self);
	  [[self recvPort] removeHandle: self];
	  [[self sendPort] removeHandle: self];
	}
      M_UNLOCK(myLock);
    }
}

- (BOOL) isValid
{
  return valid;
}

- (NSMessagePort*) recvPort
{
  if (recvPort == nil)
    return nil;
  else
    return recvPort;
}

- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
		 extra: (void*)extra
	       forMode: (NSString*)mode
{
  NSDebugMLLog(@"NSMessagePort_details",
    @"received %s event on 0x%"PRIxPTR,
    type != ET_WDESC ? "read" : "write", (NSUInteger)self);
  /*
   * If we have been invalidated (desc < 0) then we should ignore this
   * event and remove ourself from the runloop.
   */
  if (desc < 0)
    {
      NSRunLoop	*l = [runLoopClass currentRunLoop];

      [l removeEvent: data
		type: ET_WDESC
	     forMode: mode
		 all: YES];
      return;
    }

  M_LOCK(myLock);

  if (type != ET_WDESC)
    {
      unsigned	want;
      void	*bytes;
      int	res;

      /*
       * Make sure we have a buffer big enough to hold all the data we are
       * expecting, or NETBLOCK bytes, whichever is greater.
       */
      if (rData == nil)
	{
	  rData = [[mutableDataClass alloc] initWithLength: NETBLOCK];
	  rWant = sizeof(GSPortItemHeader);
	  rLength = 0;
	  want = NETBLOCK;
	}
      else
	{
	  want = [rData length];
	  if (want < rWant)
	    {
	      want = rWant;
	      [rData setLength: want];
	    }
	  if (want < NETBLOCK)
	    {
	      want = NETBLOCK;
	      [rData setLength: want];
	    }
	}

      /*
       * Now try to fill the buffer with data.
       */
      bytes = [rData mutableBytes];
      res = read(desc, bytes + rLength, want - rLength);
      if (res <= 0)
	{
	  if (res == 0)
	    {
	      NSDebugMLLog(@"NSMessagePort",
	        @"read eof on 0x%"PRIxPTR, (NSUInteger)self);
	      M_UNLOCK(myLock);
	      [self invalidate];
	      return;
	    }
	  else if (errno != EINTR && errno != EAGAIN)
	    {
	      NSDebugMLLog(@"NSMessagePort",
		@"read failed - %@ on 0x%p", [NSError _last], self);
	      M_UNLOCK(myLock);
	      [self invalidate];
	      return;
	    }
	  res = 0;	/* Interrupted - continue	*/
	}
      NSDebugMLLog(@"NSMessagePort_details",
	@"read %d bytes on 0x%"PRIxPTR, res, (NSUInteger)self);
      rLength += res;

      while (valid == YES && rLength >= rWant)
	{
	  BOOL	shouldDispatch = NO;

	  switch (rType)
	    {
	      case GSP_NONE:
		{
		  GSPortItemHeader	*h;
		  unsigned		l;

		  /*
		   * We have read an item header - set up to read the
		   * remainder of the item.
		   */
		  h = (GSPortItemHeader*)bytes;
		  rType = GSSwapBigI32ToHost(h->type);
		  l = GSSwapBigI32ToHost(h->length);
		  if (rType == GSP_PORT)
		    {
		      if (l > 512)
			{
			  NSLog(@"%@ - unreasonable length (%u) for port",
			    self, l);
			  M_UNLOCK(myLock);
			  [self invalidate];
			  return;
			}
		      /*
		       * For a port, we leave the item header in the data
		       * so that our decode function can check length info.
		       */
		      rWant += l;
		    }
		  else if (rType == GSP_DATA)
		    {
		      if (l == 0)
			{
			  NSData	*d;

			  /*
			   * For a zero-length data chunk, we create an empty
			   * data object and add it to the current message.
			   */
			  rType = GSP_NONE;	/* ready for a new item	*/
			  rLength -= rWant;
			  if (rLength > 0)
			    {
			      memmove(bytes, bytes + rWant, rLength);
			    }
			  rWant = sizeof(GSPortItemHeader);
			  d = [mutableDataClass new];
			  [rItems addObject: d];
			  RELEASE(d);
			  if (nItems == [rItems count])
			    {
			      shouldDispatch = YES;
			    }
			}
		      else
			{
			  if (l > maxDataLength)
			    {
			      NSLog(@"%@ - unreasonable length (%u) for data",
				self, l);
			      M_UNLOCK(myLock);
			      [self invalidate];
			      return;
			    }
			  /*
			   * If not a port or zero length data,
			   * we discard the data read so far and fill the
			   * data object with the data item from the msg.
			   */
			  rLength -= rWant;
			  if (rLength > 0)
			    {
			      memmove(bytes, bytes + rWant, rLength);
			    }
			  rWant = l;
			}
		    }
		  else if (rType == GSP_HEAD)
		    {
		      if (l > maxDataLength)
			{
			  NSLog(@"%@ - unreasonable length (%u) for data",
			    self, l);
			  M_UNLOCK(myLock);
			  [self invalidate];
			  return;
			}
		      /*
		       * If not a port or zero length data,
		       * we discard the data read so far and fill the
		       * data object with the data item from the msg.
		       */
		      rLength -= rWant;
		      if (rLength > 0)
			{
			  memmove(bytes, bytes + rWant, rLength);
			}
		      rWant = l;
		    }
		  else
		    {
		      NSLog(@"%@ - bad data received on port handle, rType=%i",
			self, rType);
		      M_UNLOCK(myLock);
		      [self invalidate];
		      return;
		    }
		}
		break;

	      case GSP_HEAD:
		{
		  GSPortMsgHeader	*h;

		  rType = GSP_NONE;	/* ready for a new item	*/
		  /*
		   * We have read a message header - set up to read the
		   * remainder of the message.
		   */
		  h = (GSPortMsgHeader*)bytes;
		  rId = GSSwapBigI32ToHost(h->mId);
		  nItems = GSSwapBigI32ToHost(h->nItems);
		  NSAssert(nItems >0, NSInternalInconsistencyException);
		  rItems
		    = [mutableArrayClass allocWithZone: NSDefaultMallocZone()];
		  rItems = [rItems initWithCapacity: nItems];
		  if (rWant > sizeof(GSPortMsgHeader))
		    {
		      NSData	*d;

		      /*
		       * The first data item of the message was included in
		       * the header - so add it to the rItems array.
		       */
		      rWant -= sizeof(GSPortMsgHeader);
		      d = [mutableDataClass alloc];
		      d = [d initWithBytes: bytes + sizeof(GSPortMsgHeader)
				    length: rWant];
		      [rItems addObject: d];
		      RELEASE(d);
		      rWant += sizeof(GSPortMsgHeader);
		      rLength -= rWant;
		      if (rLength > 0)
			{
			  memmove(bytes, bytes + rWant, rLength);
			}
		      rWant = sizeof(GSPortItemHeader);
		      if (nItems == 1)
			{
			  shouldDispatch = YES;
			}
		    }
		  else
		    {
		      /*
		       * want to read another item
		       */
		      rLength -= rWant;
		      if (rLength > 0)
			{
			  memmove(bytes, bytes + rWant, rLength);
			}
		      rWant = sizeof(GSPortItemHeader);
		    }
		}
		break;

	      case GSP_DATA:
		{
		  NSData	*d;

		  rType = GSP_NONE;	/* ready for a new item	*/
		  d = [mutableDataClass allocWithZone: NSDefaultMallocZone()];
		  d = [d initWithBytes: bytes length: rWant];
		  [rItems addObject: d];
		  RELEASE(d);
		  rLength -= rWant;
		  if (rLength > 0)
		    {
		      memmove(bytes, bytes + rWant, rLength);
		    }
		  rWant = sizeof(GSPortItemHeader);
		  if (nItems == [rItems count])
		    {
		      shouldDispatch = YES;
		    }
		}
		break;

	      case GSP_PORT:
		{
		  NSMessagePort	*p;

		  rType = GSP_NONE;	/* ready for a new item	*/
		  p = decodePort(rData);
		  if (p == nil)
		    {
		      NSLog(@"%@ - unable to decode remote port", self);
		      M_UNLOCK(myLock);
		      [self invalidate];
		      return;
		    }
		  /*
		   * Set up to read another item header.
		   */
		  rLength -= rWant;
		  if (rLength > 0)
		    {
		      memmove(bytes, bytes + rWant, rLength);
		    }
		  rWant = sizeof(GSPortItemHeader);

		  if (state == GS_H_ACCEPT)
		    {
		      /*
		       * This is the initial port information on a new
		       * connection - set up port relationships.
		       */
		      state = GS_H_CONNECTED;
		      [p addHandle: self forSend: YES];
		    }
		  else
		    {
		      /*
		       * This is a port within a port message - add
		       * it to the message components.
		       */
		      [rItems addObject: p];
		      if (nItems == [rItems count])
			{
			  shouldDispatch = YES;
			}
		    }
		}
		break;
	    }

	  if (shouldDispatch == YES)
	    {
	      NSPortMessage	*pm;
	      NSMessagePort		*rp = [self recvPort];

	      pm = [portMessageClass allocWithZone: NSDefaultMallocZone()];
	      pm = [pm initWithSendPort: [self sendPort]
			    receivePort: rp
			     components: rItems];
	      [pm setMsgid: rId];
	      rId = 0;
	      DESTROY(rItems);
	      NSDebugMLLog(@"NSMessagePort_details",
		@"got message %@ on 0x%"PRIxPTR, pm, (NSUInteger)self);
	      IF_NO_GC([rp retain];)
	      M_UNLOCK(myLock);
	      NS_DURING
		{
		  [rp handlePortMessage: pm];
		}
	      NS_HANDLER
		{
		  M_LOCK(myLock);
		  RELEASE(pm);
		  RELEASE(rp);
		  [localException raise];
		}
	      NS_ENDHANDLER
	      M_LOCK(myLock);
	      RELEASE(pm);
	      RELEASE(rp);
	      bytes = [rData mutableBytes];
	    }
	}
    }
  else
    {
      if (state == GS_H_TRYCON)	/* Connection attempt.	*/
	{
	  int	   res = 0;
	  unsigned len = sizeof(res);

/* Currently gnu hurd doesn't support getsockopt on local domain sockets
 * and fails with a 'protocol not supported' error.
 * We therefore just hope the connect was a success.
 */
#if !defined(__gnu_hurd__)
	  if (getsockopt(desc, SOL_SOCKET, SO_ERROR, (char*)&res, &len) != 0)
	    {
	      state = GS_H_UNCON;
	      NSLog(@"connect attempt failed - %@", [NSError _last]);
	    }
	  else if (res != 0)
	    {
	      state = GS_H_UNCON;
	      NSLog(@"connect attempt failed - %@",
	        [NSError _systemError: res]);
	    }
	  else
#endif
	    {
	      NSData	*d = newDataWithEncodedPort([self recvPort]);

	      len = write(desc, [d bytes], [d length]);
	      if (len == (int)[d length])
		{
		  NSDebugMLLog(@"NSMessagePort_details",
		    @"wrote %d bytes on 0x%"PRIxPTR, len, (NSUInteger)self);
		  state = GS_H_CONNECTED;
		}
	      else
		{
		  state = GS_H_UNCON;
		  NSLog(@"connect write attempt failed - %@",
		    [NSError _last]);
		}
	      RELEASE(d);
	    }
	}
      else
	{
	  int		res;
	  unsigned	l;
	  const void	*b;

	  if (wData == nil)
	    {
	      if ([wMsgs count] > 0)
		{
		  NSArray	*components = [wMsgs objectAtIndex: 0];

		  wData = [components objectAtIndex: wItem++];
		  wLength = 0;
		}
	      else
		{
// NSLog(@"No messages to write on 0x%"PRIxPTR".", (NSUInteger)self);
		  M_UNLOCK(myLock);
		  return;
		}
	    }
	  b = [wData bytes];
	  l = [wData length];
	  res = write(desc, b + wLength,  l - wLength);
	  if (res < 0)
	    {
	      if (errno != EINTR && errno != EAGAIN)
		{
		  NSLog(@"write attempt failed - %@", [NSError _last]);
		  M_UNLOCK(myLock);
		  [self invalidate];
		  return;
		}
	    }
	  else
	    {
	      NSDebugMLLog(@"NSMessagePort_details",
		@"wrote %d bytes on 0x%"PRIxPTR, res, (NSUInteger)self);
	      wLength += res;
	      if (wLength == l)
		{
		  NSArray	*components;

		  /*
		   * We have completed a data item so see what is
		   * left of the message components.
		   */
		  components = [wMsgs objectAtIndex: 0];
		  wLength = 0;
		  if ([components count] > wItem)
		    {
		      /*
		       * More to write - get next item.
		       */
		      wData = [components objectAtIndex: wItem++];
		    }
		  else
		    {
		      /*
		       * message completed - remove from list.
		       */
		      NSDebugMLLog(@"NSMessagePort_details",
			@"completed 0x%"PRIxPTR" on 0x%"PRIxPTR,
			(NSUInteger)components, (NSUInteger)self);
		      wData = nil;
		      wItem = 0;
		      [wMsgs removeObjectAtIndex: 0];
		    }
		}
	    }
	}
    }

  M_UNLOCK(myLock);
}

- (BOOL) sendMessage: (NSArray*)components beforeDate: (NSDate*)when
{
  NSRunLoop	*l;
  BOOL		sent = NO;

  NSAssert([components count] > 0, NSInternalInconsistencyException);
  NSDebugMLLog(@"NSMessagePort_details",
    @"Sending message 0x%"PRIxPTR" %@ on 0x%"PRIxPTR"(%d) before %@",
    (NSUInteger)components, components, (NSUInteger)self, desc, when);
  M_LOCK(myLock);
  [wMsgs addObject: components];

  l = [runLoopClass currentRunLoop];

  IF_NO_GC(RETAIN(self);)

  [self _add: l];

  while (valid == YES
    && [wMsgs indexOfObjectIdenticalTo: components] != NSNotFound
    && [when timeIntervalSinceNow] > 0)
    {
      M_UNLOCK(myLock);
      [l runMode: NSConnectionReplyMode beforeDate: when];
      M_LOCK(myLock);
    }

  [self _rem: l];

  if ([wMsgs indexOfObjectIdenticalTo: components] == NSNotFound)
    {
      sent = YES;
    }
  else
    {
      [wMsgs removeObjectIdenticalTo: components];
    }
  M_UNLOCK(myLock);
  NSDebugMLLog(@"NSMessagePort_details",
    @"Message send 0x%"PRIxPTR" on 0x%"PRIxPTR" status %d",
    (NSUInteger)components, (NSUInteger)self, sent);
  RELEASE(self);
  return sent;
}

- (NSMessagePort*) sendPort
{
  if (sendPort == nil)
    return nil;
  else if (caller == YES)
    return sendPort;	// We called, so port is not retained.
  else
    return sendPort;	// Retained port.
}

- (void) setState: (GSHandleState)s
{
  state = s;
}

- (GSHandleState) state
{
  return state;
}

@end



@interface NSMessagePort (RunLoop) <RunLoopEvents>
- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
		 extra: (void*)extra
	       forMode: (NSString*)mode;
@end


@implementation	NSMessagePort

static NSRecursiveLock	*messagePortLock = nil;

/*
 * Maps port name to NSMessagePort objects.
 */
static NSMapTable	*messagePortMap = 0;
static Class		messagePortClass;


+ (void) atExit
{
  NSMessagePort		*port;
  NSData		*name;
  NSMapEnumerator	mEnum;
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];

  mEnum = NSEnumerateMapTable(messagePortMap);
  while (NSNextMapEnumeratorPair(&mEnum, (void *)&name, (void *)&port))
    {
      if ([port _listener] != -1)
	unlink([name bytes]);
    }
  NSEndMapTableEnumeration(&mEnum);
  DESTROY(messagePortMap);
  DESTROY(messagePortLock);
  [arp drain];
}

typedef	struct {
  NSData                *_name;
  NSRecursiveLock       *_myLock;
  NSMapTable            *_handles;       /* Handles indexed by socket.   */
  int                   _listener;       /* Descriptor to listen on.     */
} internal;
#define	name	((internal*)_internal)->_name
#define	myLock	((internal*)_internal)->_myLock
#define	handles	((internal*)_internal)->_handles
#define	lDesc	((internal*)_internal)->_listener


+ (void) initialize
{
  if (self == [NSMessagePort class])
    {
      NSAutoreleasePool *pool = [NSAutoreleasePool new];
      NSFileManager	*mgr;
      NSString		*path;
      NSString		*pref;
      NSString		*file;
      NSEnumerator	*files;

      messagePortClass = self;
      messagePortMap = NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,
	NSNonOwnedPointerMapValueCallBacks, 0);

      messagePortLock = [NSRecursiveLock new];

      /* It's possible that an old process, with the same process ID as
       * this one, got forcibly killed or crashed so that clean_up_sockets
       * was never called.
       * To deal with that unlikely situation, we need to remove all such
       * ports which have been left over.
       */
      path = NSTemporaryDirectory();
      path = [path stringByAppendingPathComponent: @"NSMessagePort"];
      path = [path stringByAppendingPathComponent: @"ports"];
      pref = [NSString stringWithFormat: @"%i.",
	[[NSProcessInfo processInfo] processIdentifier]];
      mgr = [NSFileManager defaultManager];
      files = [[mgr directoryContentsAtPath: path] objectEnumerator];
      while ((file = [files nextObject]) != nil)
	{
          NSString	*old = [path stringByAppendingPathComponent: file];

	  if (YES == [file hasPrefix: pref])
	    {
	      NSDebugMLLog(@"NSMessagePort", @"Removing old port %@", old);
	      [mgr removeFileAtPath: old handler: nil];
	    }
	  else
	    {
	      int	pid = [file intValue];

	      if (pid > 0)
		{
		  if (NO == [NSProcessInfo _exists: pid])
		    {
		      NSDebugMLLog(@"NSMessagePort",
		        @"Removing old port %@ for process %d", old, pid);
		      [mgr removeFileAtPath: old handler: nil];
		    }
		}
	    }
	}
      [pool release];
      [self registerAtExit];
    }
}

+ (id) new
{
  static int unique_index = 0;
  NSString	*path;
  NSDictionary	*attr;

  if (nil == (path = NSTemporaryDirectory()))
    {
      return nil;
    }

  attr = [NSDictionary dictionaryWithObject: [NSNumber numberWithInt: 0700]
				     forKey: NSFilePosixPermissions];

  path = [path stringByAppendingPathComponent: @"NSMessagePort"];
  [[NSFileManager defaultManager] createDirectoryAtPath: path
                            withIntermediateDirectories: YES
                                             attributes: attr
                                                  error: NULL];

  path = [path stringByAppendingPathComponent: @"ports"];
  [[NSFileManager defaultManager] createDirectoryAtPath: path
                            withIntermediateDirectories: YES
                                             attributes: attr
                                                  error: NULL];

  M_LOCK(messagePortLock);
  path = [path stringByAppendingPathComponent:
   [NSString stringWithFormat: @"%i.%i",
	[[NSProcessInfo processInfo] processIdentifier], unique_index++]];
  M_UNLOCK(messagePortLock);

  return RETAIN([self _portWithName:
    (unsigned char*)[path fileSystemRepresentation] listener: YES]);
}

/*
 * This is the preferred initialisation method for NSMessagePort
 *
 * 'socketName' is the name of the socket in the port directory
 */
+ (NSMessagePort*) _portWithName: (const unsigned char *)socketName
			listener: (BOOL)shouldListen
{
  unsigned		i;
  NSMessagePort		*port = nil;
  NSData		*theName;

  theName = [[NSData alloc] initWithBytes: socketName
				   length: strlen((char*)socketName)+1];

  M_LOCK(messagePortLock);

  /*
   * First try to find a pre-existing port.
   */
  port = (NSMessagePort*)NSMapGet(messagePortMap, theName);

  if (port == nil)
    {
      port = (NSMessagePort*)NSAllocateObject(self, 0, NSDefaultMallocZone());
      port->_internal = (internal*)NSZoneMalloc(NSDefaultMallocZone(),
	  sizeof(internal));
      ((internal*)(port->_internal))->_name = theName;
      ((internal*)(port->_internal))->_listener = -1;
      ((internal*)(port->_internal))->_handles
	= NSCreateMapTable(NSIntegerMapKeyCallBacks,
	NSObjectMapValueCallBacks, 0);
      ((internal*)(port->_internal))->_myLock = [NSRecursiveLock new];
      port->_is_valid = YES;

      if (shouldListen == YES)
	{
	  int	desc;
	  struct sockaddr_un	sockAddr;

	  /*
           * Need size of buffer for getsockbyname() later.
	   */
	  i = sizeof(sockAddr);

	  /*
	   * Creating a new port on the local host - so we must create a
	   * listener socket to accept incoming connections.
	   */
	  memset(&sockAddr, '\0', sizeof(sockAddr));
	  sockAddr.sun_family = AF_LOCAL;
	  strncpy(sockAddr.sun_path, (char*)socketName,
	    sizeof(sockAddr.sun_path) - 1);
	  if ((desc = socket(PF_LOCAL, SOCK_STREAM, PF_UNSPEC)) < 0)
	    {
	      NSLog(@"unable to create socket - %@", [NSError _last]);
	      desc = -1;
	    }
	  else if (bind(desc, (struct sockaddr *)&sockAddr,
	    SUN_LEN(&sockAddr)) < 0)
	    {
	      if (connect(desc, (struct sockaddr*)&sockAddr,
		SUN_LEN(&sockAddr)) < 0)
		{
		  NSDebugLLog(@"NSMessagePort", @"not live, resetting");
		  unlink((const char*)socketName);
		  close(desc);
		  if ((desc = socket(PF_LOCAL, SOCK_STREAM, PF_UNSPEC)) < 0)
		    {
		      NSLog(@"unable to create socket - %@",
			[NSError _last]);
		      desc = -1;
		    }
		  else if (bind(desc, (struct sockaddr *)&sockAddr,
		    SUN_LEN(&sockAddr)) < 0)
		    {
		      NSLog(@"unable to bind to %s - %@",
			sockAddr.sun_path, [NSError _last]);
		      (void) close(desc);
		      desc = -1;
		    }
		}
	      else
		{
		  NSLog(@"unable to bind to %s - %@",
		    sockAddr.sun_path, [NSError _last]);
		  (void) close(desc);
		  desc = -1;
		}
	    }

	  if (desc == -1)
	    {
              DESTROY(port);
	    }
	  else if (listen(desc, GSBACKLOG) < 0)
	    {
	      NSLog(@"unable to listen on port - %@", [NSError _last]);
	      (void) close(desc);
	      DESTROY(port);
	    }
	  else if (getsockname(desc, (struct sockaddr*)&sockAddr, &i) < 0)
	    {
	      NSLog(@"unable to get socket name - %@", [NSError _last]);
	      (void) close(desc);
	      DESTROY(port);
	    }
	  else
	    {
	      /*
	       * Set up the listening descriptor and the actual message port
	       * number (which will have been set to a real port number when
	       * we did the 'bind' call.
	       */
	      ((internal*)port->_internal)->_listener = desc;
	      /*
	       * Make sure we have the map table for this port.
	       */
	      NSMapInsert(messagePortMap, (void*)theName, (void*)port);
	      NSDebugMLLog(@"NSMessagePort", @"Created listening port: %@",
		port);
	    }
	}
      else
	{
	  /*
	   * Make sure we have the map table for this port.
	   */
	  NSMapInsert(messagePortMap, (void*)theName, (void*)port);
	  NSDebugMLLog(@"NSMessagePort", @"Created speaking port: %@", port);
	}
    }
  else
    {
      RELEASE(theName);
      IF_NO_GC([port retain];)
      NSDebugMLLog(@"NSMessagePort", @"Using pre-existing port: %@", port);
    }
  IF_NO_GC(AUTORELEASE(port));

  M_UNLOCK(messagePortLock);
  return port;
}

- (void) addHandle: (GSMessageHandle*)handle forSend: (BOOL)send
{
  M_LOCK(myLock);
  if (send == YES)
    {
      if (handle->caller == YES)
	handle->sendPort = self;
      else
	ASSIGN(handle->sendPort, self);
    }
  else
    {
      handle->recvPort = self;
    }
  NSMapInsert(handles, (void*)(uintptr_t)[handle descriptor], (void*)handle);
  M_UNLOCK(myLock);
}

- (id) copyWithZone: (NSZone*)zone
{
  return RETAIN(self);
}

- (void) dealloc
{
  [self finalize];
  [super dealloc];
}

- (NSString*) description
{
  NSString	*desc;

  desc = [NSString stringWithFormat:
    @"<%s %p file name %s>",
    GSClassNameFromObject(self), self, (char*)[name bytes]];
  return desc;
}

- (void) finalize
{
  NSDebugMLLog(@"NSMessagePort",
    @"NSMessagePort 0x%"PRIxPTR" finalized", (NSUInteger)self);
  [self invalidate];
  if (_internal != 0)
    {
      DESTROY(name);
      NSFreeMapTable(handles);
      RELEASE(myLock);
      NSZoneFree(NSDefaultMallocZone(), _internal);
    }
}

/*
 * This is a callback method used by the NSRunLoop class to determine which
 * descriptors to watch for the port.
 */
- (void) getFds: (NSInteger*)fds count: (NSInteger*)count
{
  NSInteger             limit = *count;
  NSInteger             pos = 0;
  NSMapEnumerator	me;
  void			*sock;
  GSMessageHandle	*handle;
  id			recvSelf;

  M_LOCK(myLock);

  /*
   * Put in our listening socket.
   */
  if (lDesc >= 0)
    {
      if (pos < limit)
        {
          fds[pos] = lDesc;
        }
      pos++;
    }

  /*
   * Enumerate all our socket handles, and put them in as long as they
   * are to be used for receiving.
   */
  recvSelf = self;
  me = NSEnumerateMapTable(handles);
  while (NSNextMapEnumeratorPair(&me, &sock, (void**)&handle))
    {
      if (handle->recvPort == recvSelf)
        {
          if (pos < limit)
            {
              fds[pos] = (int)(intptr_t)sock;
            }
          pos++;
        }
    }
  NSEndMapTableEnumeration(&me);
  M_UNLOCK(myLock);
  *count = pos;
}

- (id) conversation: (NSPort*)recvPort
{
  NSMapEnumerator	me;
  void			*dummy;
  GSMessageHandle	*handle = nil;

  M_LOCK(myLock);
  /*
   * Enumerate all our socket handles, and look for one with port.
   */
  me = NSEnumerateMapTable(handles);
  while (NSNextMapEnumeratorPair(&me, &dummy, (void**)&handle))
    {
      if ((NSPort*) [handle recvPort] == recvPort)
	{
	  IF_NO_GC([handle retain];)
	  NSEndMapTableEnumeration(&me);
	  M_UNLOCK(myLock);
	  return AUTORELEASE(handle);
	}
    }
  NSEndMapTableEnumeration(&me);
  M_UNLOCK(myLock);
  return nil;
}

- (GSMessageHandle*) handleForPort: (NSMessagePort*)recvPort
			beforeDate: (NSDate*)when
{
  NSMapEnumerator	me;
  int			sock;
  void			*dummy;
  GSMessageHandle	*handle = nil;

  M_LOCK(myLock);
  /*
   * Enumerate all our socket handles, and look for one with port.
   */
  me = NSEnumerateMapTable(handles);
  while (NSNextMapEnumeratorPair(&me, &dummy, (void**)&handle))
    {
      if ([handle recvPort] == recvPort)
	{
	  M_UNLOCK(myLock);
	  NSEndMapTableEnumeration(&me);
	  return handle;
	}
    }
  NSEndMapTableEnumeration(&me);

  /*
   * Not found ... create a new handle.
   */
  handle = nil;
  sock = socket(PF_LOCAL, SOCK_STREAM, PF_UNSPEC);
  if (sock < 0)
    {
      NSLog(@"unable to create socket - %@", [NSError _last]);
    }
  else if ((handle = [GSMessageHandle handleWithDescriptor: sock]) == nil)
    {
      (void)close(sock);
      NSLog(@"unable to create GSMessageHandle - %@", [NSError _last]);
    }
  else
    {
      [recvPort addHandle: handle forSend: NO];
    }
  M_UNLOCK(myLock);
  /*
   * If we succeeded in creating a new handle - connect to remote host.
   */
  if (handle != nil)
    {
      if ([handle connectToPort: self beforeDate: when] == NO)
	{
	  [handle invalidate];
	  handle = nil;
	}
    }
  return handle;
}

- (void) handlePortMessage: (NSPortMessage*)m
{
  id	d = [self delegate];

  if (d == nil)
    {
      NSDebugMLLog(@"NSMessagePort",
	@"%@", @"No delegate to handle incoming message");
      return;
    }
  if ([d respondsToSelector: @selector(handlePortMessage:)] == NO)
    {
      NSDebugMLLog(@"NSMessagePort",
	@"%@", @"delegate doesn't handle messages");
      return;
    }
  [d handlePortMessage: m];
}

- (NSUInteger) hash
{
  return [name hash];
}

- (id) init
{
  DESTROY(self);
  self = [messagePortClass new];
  return self;
}

- (void) invalidate
{
  if ([self isValid] == YES)
    {
      IF_NO_GC(RETAIN(self);)
      M_LOCK(myLock);

      if ([self isValid] == YES)
	{
	  NSArray	*handleArray;
	  unsigned	i;

	  M_LOCK(messagePortLock);
	  NSMapRemove(messagePortMap, (void*)name);
	  M_UNLOCK(messagePortLock);
	  if (lDesc >= 0)
	    {
	      (void) close(lDesc);
	      unlink([name bytes]);
	      lDesc = -1;
	    }

	  handleArray = NSAllMapTableValues(handles);
	  i = [handleArray count];
	  while (i-- > 0)
	    {
	      GSMessageHandle	*handle;

	      handle = [handleArray objectAtIndex: i];
	      [handle invalidate];
	    }

	  [[NSMessagePortNameServer sharedInstance] removePort: self];
	  [super invalidate];
	}
      M_UNLOCK(myLock);
      RELEASE(self);
    }
}

- (BOOL) isEqual: (id)anObject
{
  if (anObject == self)
    {
      return YES;
    }
  if ([anObject class] == [self class] && [self isValid] && [anObject isValid])
    {
      NSMessagePort	*o = (NSMessagePort*)anObject;

      return [((internal*)o->_internal)->_name isEqual: name];
    }
  return NO;
}

- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
		 extra: (void*)extra
	       forMode: (NSString*)mode
{
  int		desc = (int)(uintptr_t)extra;
  GSMessageHandle	*handle;

  if (desc == lDesc)
    {
      struct sockaddr_un	sockAddr;
      unsigned			size = sizeof(sockAddr);

      desc = accept(lDesc, (struct sockaddr*)&sockAddr, &size);
      if (desc < 0)
        {
	  NSDebugMLLog(@"NSMessagePort",
	    @"accept failed - handled in other thread?");
        }
      else
	{
	  int	status = 1;

	  if (setsockopt(desc, SOL_SOCKET, SO_KEEPALIVE, (char*)&status,
	    sizeof(status)) < 0)
            {
              NSLog(@"failed to turn on keepalive for accepted socket %d",
                desc);
            }
	  /*
	   * Create a handle for the socket and set it up so we are its
	   * receiving port, and it's waiting to get the port name from
	   * the other end.
	   */
	  handle = [GSMessageHandle handleWithDescriptor: desc];
	  memcpy(&handle->sockAddr, &sockAddr, sizeof(sockAddr));

	  [handle setState: GS_H_ACCEPT];
	  [self addHandle: handle forSend: NO];
	}
    }
  else
    {
      M_LOCK(myLock);
      handle = (GSMessageHandle*)NSMapGet(handles, (void*)(uintptr_t)desc);
      IF_NO_GC(AUTORELEASE(RETAIN(handle)));
      M_UNLOCK(myLock);
      if (handle == nil)
	{
	  const char	*t;

	  if (type == ET_RDESC) t = "rdesc";
	  else if (type == ET_WDESC) t = "wdesc";
	  else if (type == ET_RPORT) t = "rport";
	  else t = "unknown";
	  NSLog(@"No handle for event %s on descriptor %d", t, desc);
	}
      else
	{
	  [handle receivedEvent: data type: type extra: extra forMode: mode];
	}
    }
}

- (oneway void) release
{
  M_LOCK(messagePortLock);
  if (NSDecrementExtraRefCountWasZero(self))
    {
      if (_internal != 0)
        {
          NSMapRemove(messagePortMap, (void*)name);
	}
      M_UNLOCK(messagePortLock);
      [self dealloc];
    }
  else
    {
      M_UNLOCK(messagePortLock);
    }
}

- (void) removeHandle: (GSMessageHandle*)handle
{
  IF_NO_GC(RETAIN(self);)
  M_LOCK(myLock);
  if ([handle sendPort] == self)
    {
      if (handle->caller != YES)
	{
	  /*
	   * This is a handle for a send port, and the handle was not formed
	   * by calling the remote process, so this port object must have
	   * been created to deal with an incoming connection and will have
	   * been retained - we must therefore release this port since the
	   * handle no longer uses it.
	   */
	  IF_NO_GC([self autorelease];)
	}
      handle->sendPort = nil;
    }
  if ([handle recvPort] == self)
    {
      handle->recvPort = nil;
    }
  NSMapRemove(handles, (void*)(uintptr_t)[handle descriptor]);
  if (lDesc < 0 && NSCountMapTable(handles) == 0)
    {
      [self invalidate];
    }
  M_UNLOCK(myLock);
  RELEASE(self);
}

/*
 * This returns the amount of space that a port coder should reserve at the
 * start of its encoded data so that the NSMessagePort can insert header info
 * into the data.
 * The idea is that a message consisting of a single data item with space at
 * the start can be written directly without having to copy data to another
 * buffer etc.
 */
- (NSUInteger) reservedSpaceLength
{
  return sizeof(GSPortItemHeader) + sizeof(GSPortMsgHeader);
}

- (BOOL) sendBeforeDate: (NSDate*)when
		  msgid: (NSInteger)msgId
             components: (NSMutableArray*)components
                   from: (NSPort*)receivingPort
               reserved: (NSUInteger)length
{
  BOOL		sent = NO;
  GSMessageHandle	*h;
  unsigned	rl;

  if ([self isValid] == NO)
    {
      return NO;
    }
  if ([components count] == 0)
    {
      NSLog(@"empty components sent");
      return NO;
    }
  /*
   * If the reserved length in the first data object is wrong - we have to
   * fail, unless it's zero, in which case we can insert a data object for
   * the header.
   */
  rl = [self reservedSpaceLength];
  if (length != 0 && length != rl)
    {
      NSLog(@"bad reserved length - %"PRIuPTR, length);
      return NO;
    }
  if ([receivingPort isKindOfClass: messagePortClass] == NO)
    {
      NSLog(@"woah there - receiving port is not the correct type");
      return NO;
    }

  h = [self handleForPort: (NSMessagePort*)receivingPort beforeDate: when];
  if (h != nil)
    {
      NSMutableData	*header;
      unsigned		hLength;
      unsigned		l;
      GSPortItemHeader	*pih;
      GSPortMsgHeader	*pmh;
      unsigned		c = [components count];
      unsigned		i;
      BOOL		pack = YES;

      /*
       * Ok - ensure we have space to insert header info.
       */
      if (length == 0 && rl != 0)
	{
	  header = [[mutableDataClass alloc] initWithCapacity: NETBLOCK];

	  [header setLength: rl];
	  [components insertObject: header atIndex: 0];
	  RELEASE(header);
	}

      header = [components objectAtIndex: 0];
      /*
       * The Item header contains the item type and the length of the
       * data in the item (excluding the item header itself).
       */
      hLength = [header length];
      l = hLength - sizeof(GSPortItemHeader);
      pih = (GSPortItemHeader*)[header mutableBytes];
      pih->type = GSSwapHostI32ToBig(GSP_HEAD);
      pih->length = GSSwapHostI32ToBig(l);

      /*
       * The message header contains the message Id and the original count
       * of components in the message (excluding any extra component added
       * simply to hold the header).
       */
      pmh = (GSPortMsgHeader*)&pih[1];
      pmh->mId = GSSwapHostI32ToBig(msgId);
      pmh->nItems = GSSwapHostI32ToBig(c);

      /*
       * Now insert item header information as required.
       * Pack as many items into the initial data object as possible, up to
       * a maximum of NETBLOCK bytes.  This is to try to get a single,
       * efficient write operation if possible.
       */
      c = [components count];
      for (i = 1; i < c; i++)
	{
	  id	o = [components objectAtIndex: i];

	  if ([o isKindOfClass: [NSData class]])
	    {
	      GSPortItemHeader	*pih;
	      unsigned		h = sizeof(GSPortItemHeader);
	      unsigned		l = [o length];
	      void		*b;

	      if (pack == YES && hLength + l + h <= NETBLOCK)
		{
		  [header setLength: hLength + l + h];
		  b = [header mutableBytes];
		  b += hLength;
#if NEED_WORD_ALIGNMENT
		  /*
		   * When packing data, an item may not be aligned on a
		   * word boundary, so we work with an aligned buffer
		   * and use memcmpy()
		   */
		  if ((hLength % __alignof__(uint32_t)) != 0)
		    {
		      GSPortItemHeader	itemHeader;

		      pih = (GSPortItemHeader*)&itemHeader;
		      pih->type = GSSwapHostI32ToBig(GSP_DATA);
		      pih->length = GSSwapHostI32ToBig(l);
		      memcpy(b, (void*)pih, h);
		    }
		  else
		    {
		      pih = (GSPortItemHeader*)b;
		      pih->type = GSSwapHostI32ToBig(GSP_DATA);
		      pih->length = GSSwapHostI32ToBig(l);
		    }
#else
		  pih = (GSPortItemHeader*)b;
		  pih->type = GSSwapHostI32ToBig(GSP_DATA);
		  pih->length = GSSwapHostI32ToBig(l);
#endif
		  memcpy(b+h, [o bytes], l);
		  [components removeObjectAtIndex: i--];
		  c--;
		  hLength += l + h;
		}
	      else
		{
		  NSMutableData	*d;

		  pack = NO;
		  d = [[NSMutableData alloc] initWithLength: l + h];
		  b = [d mutableBytes];
		  pih = (GSPortItemHeader*)b;
		  memcpy(b+h, [o bytes], l);
		  pih->type = GSSwapHostI32ToBig(GSP_DATA);
		  pih->length = GSSwapHostI32ToBig(l);
		  [components replaceObjectAtIndex: i
					withObject: d];
		  RELEASE(d);
		}
	    }
	  else if ([o isKindOfClass: messagePortClass])
	    {
	      NSData	*d = newDataWithEncodedPort(o);
	      unsigned	dLength = [d length];

	      if (pack == YES && hLength + dLength <= NETBLOCK)
		{
		  void	*b;

		  [header setLength: hLength + dLength];
		  b = [header mutableBytes];
		  b += hLength;
		  hLength += dLength;
		  memcpy(b, [d bytes], dLength);
		  [components removeObjectAtIndex: i--];
		  c--;
		}
	      else
		{
		  pack = NO;
		  [components replaceObjectAtIndex: i withObject: d];
		}
	      RELEASE(d);
	    }
	}

      /*
       * Now send the message.
       */
      sent = [h sendMessage: components beforeDate: when];
    }
  return sent;
}

- (const unsigned char *) _name
{
  return [name bytes];
}

- (int) _listener
{
  return lDesc;
}

@end

