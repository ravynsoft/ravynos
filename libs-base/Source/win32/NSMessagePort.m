/** Implementation of network port object based on windows mailboxes
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>

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
   Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02111 USA.
   */

#include "common.h"
#define	EXPOSE_NSPort_IVARS	1
#define	EXPOSE_NSMessagePort_IVARS	1
#include "GNUstepBase/GSLock.h"
#include "Foundation/NSArray.h"
#include "Foundation/NSNotification.h"
#include "Foundation/NSError.h"
#include "Foundation/NSException.h"
#include "Foundation/NSRunLoop.h"
#include "Foundation/NSByteOrder.h"
#include "Foundation/NSData.h"
#include "Foundation/NSDate.h"
#include "Foundation/NSMapTable.h"
#include "Foundation/NSPortMessage.h"
#include "Foundation/NSPortNameServer.h"
#include "Foundation/NSLock.h"
#include "Foundation/NSThread.h"
#include "Foundation/NSConnection.h"
#include "Foundation/NSDebug.h"
#include "Foundation/NSPathUtilities.h"
#include "Foundation/NSValue.h"
#include "Foundation/NSFileManager.h"
#include "Foundation/NSProcessInfo.h"

#include "../GSPrivate.h"
#include "../GSPortPrivate.h"

#include <stdio.h>

#define	UNISTR(X) \
((const unichar*)[(X) cStringUsingEncoding: NSUnicodeStringEncoding])

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
  GSP_ITEM,		/* Expecting a port item header.	*/
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
  unsigned char version;
  unsigned char	port[16];
} GSPortMsgHeader;

typedef enum {
  RS_NONE,	// Not started yet
  RS_MESG,	// Waiting to be notified of a message arriving
  RS_SIZE,	// Need to determine message size
  RS_DATA	// Need to read message data
} ReadState;

typedef	struct {
  NSString              *name;
  NSRecursiveLock       *lock;
  HANDLE                rHandle;
  HANDLE                wHandle;
  HANDLE                rEvent;
  HANDLE                wEvent;
  ReadState		rState;
  OVERLAPPED		rOv;
  OVERLAPPED		wOv;
  DWORD			rSize;
  DWORD			wSize;
  NSMutableData		*wData;		/* Data object being written.	*/
  DWORD			wLength;	/* Amount written so far.	*/
  NSMutableArray	*wMsgs;		/* Message in progress.		*/
  NSMutableData		*rData;		/* Buffer for incoming data	*/
  DWORD			rLength;	/* Amount read so far.		*/
  DWORD			rWant;		/* Amount desired.		*/
  NSMutableArray	*rMsgs;		/* Messages in progress.	*/
} internal;
#define	PORT(X)		((internal*)((NSMessagePort*)X)->_internal)

@implementation	NSMessagePort

static SECURITY_ATTRIBUTES	security;

static NSRecursiveLock	*messagePortLock = nil;

/*
 * Maps port name to NSMessagePort objects.
 */
static NSMapTable	*ports = 0;
static Class		messagePortClass = 0;

- (BOOL) _setupSendPort
{
  internal	*this = (internal*)self->_internal;
  BOOL		result;

  M_LOCK(this->lock);
  if (this->wHandle == INVALID_HANDLE_VALUE)
    {
      NSString	*path;

      path = [NSString stringWithFormat:
	@"\\\\.\\mailslot\\GNUstep\\NSMessagePort\\%@", this->name];

      this->wHandle = CreateFileW(
	UNISTR(path),
	GENERIC_WRITE,
	FILE_SHARE_READ|FILE_SHARE_WRITE,
	&security,
	OPEN_EXISTING,
	FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
	(HANDLE)0);
      if (this->wHandle == INVALID_HANDLE_VALUE)
	{
	  NSDebugMLLog(@"NSMessagePort",
	    @"unable to access mailslot '%@' for write - %@",
	    [self name], [NSError _last]);
	  result = NO;
	}
      else
	{
	  this->wEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	  this->wMsgs = [NSMutableArray new];
	  result = YES;
	}
    }
  else
    {
      result = YES;
    }
  M_UNLOCK(this->lock);
  return result;
}

+ (void) initialize
{
  if (self == [NSMessagePort class])
    {
      messagePortClass = self;
      ports = NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,
	NSNonOwnedPointerMapValueCallBacks, 0);
      [[NSObject leakAt: &ports] release];
      messagePortLock = [GSLazyRecursiveLock new];
      [[NSObject leakAt: &messagePortLock] release];
      security.nLength = sizeof(SECURITY_ATTRIBUTES);
      security.lpSecurityDescriptor = 0;	// Default
      security.bInheritHandle = FALSE;
    }
}

+ (id) newWithName: (NSString*)name
{
  NSMessagePort	*p;

  M_LOCK(messagePortLock);
  p = RETAIN((NSMessagePort*)NSMapGet(ports, (void*)name));
  if (p == nil)
    {
      p = [[self alloc] initWithName: name];
    }
  M_UNLOCK(messagePortLock);
  return p;
}

- (void) addConnection: (NSConnection*)aConnection
             toRunLoop: (NSRunLoop*)aLoop
               forMode: (NSString*)aMode
{
  NSDebugMLLog(@"NSMessagePort", @"%@ add to 0x%x in mode %@",
    self, aLoop, aMode);
  NSAssert(PORT(self)->rHandle != INVALID_HANDLE_VALUE,
    @"Attempt to listen on send port");
  [aLoop addEvent: (void*)(uintptr_t)PORT(self)->rEvent
	     type: ET_HANDLE
	  watcher: (id<RunLoopEvents>)self
	  forMode: aMode];
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

  desc = [NSString stringWithFormat: @"<NSMessagePort %p with name %@>",
    self, PORT(self)->name];
  return desc;
}

- (void) finalize
{
  internal	*this;

  NSDebugMLLog(@"NSMessagePort", @"NSMessagePort 0x%x finalized", self);
  [self invalidate];
  this = PORT(self);
  if (this != 0)
    {
      DESTROY(this->name);
      DESTROY(this->rData);
      DESTROY(this->rMsgs);
      DESTROY(this->wMsgs);
      DESTROY(this->lock);
      NSZoneFree(NSDefaultMallocZone(), _internal);
      _internal = 0;
    }
}

- (id) conversation: (NSPort*)recvPort
{
  return nil;
}

- (void) handlePortMessage: (NSPortMessage*)m
{
  id	d = [self delegate];

  if (d == nil)
    {
      NSDebugMLLog(@"NSMessagePort",
	@"No delegate to handle incoming message", 0);
      return;
    }
  if ([d respondsToSelector: @selector(handlePortMessage:)] == NO)
    {
      NSDebugMLLog(@"NSMessagePort", @"delegate doesn't handle messages", 0);
      return;
    }
  NSDebugMLLog(@"NSMessagePort", @"%@ asking %@ to handle msg", self, d);
  [d handlePortMessage: m];
}

- (NSUInteger) hash
{
  return [PORT(self)->name hash];
}

- (id) init
{
  static unsigned	sequence = 0;
  static int		ident;
  internal		*this;
  NSString		*path;

  if (sequence == 0)
    {
      ident = [[NSProcessInfo processInfo] processIdentifier];
    }
  M_LOCK(messagePortLock);
  _internal = NSZoneMalloc(NSDefaultMallocZone(), sizeof(internal));
  memset(_internal, '\0', sizeof(internal));
  this = PORT(self);
  self->_is_valid = YES;
  this->name = [[NSString alloc] initWithFormat: @"%08x%08x",
    ((unsigned)ident), sequence++];

  this->lock = [GSLazyRecursiveLock new];
  this->wHandle = INVALID_HANDLE_VALUE;
  this->wEvent = INVALID_HANDLE_VALUE;

  this->rState = RS_NONE;
  this->rEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  this->rData = [NSMutableData new];
  this->rMsgs = [NSMutableArray new];

  path = [NSString stringWithFormat:
    @"\\\\.\\mailslot\\GNUstep\\NSMessagePort\\%@", this->name];

  this->rHandle = CreateMailslotW(
    UNISTR(path),
    0,				/* No max message size.		*/
    MAILSLOT_WAIT_FOREVER,	/* No read/write timeout.	*/
    &security);

  if (this->rHandle == INVALID_HANDLE_VALUE)
    {
      NSLog(@"unable to create mailslot '%@' - %@",
	this->name, [NSError _last]);
      DESTROY(self);
    }
  else
    {
      NSMapInsert(ports, (void*)this->name, (void*)self);
      NSDebugMLLog(@"NSMessagePort", @"Created listening port: %@", self);

      /*
       * Simulate a read event to kick off the I/O for this handle.
       * If we can't start reading, we will be invalidated, and must
       * then destroy self.
       */
      [self receivedEventRead];
      if ([self isValid] == NO)
	{
	  DESTROY(self);
	}
    }

  M_UNLOCK(messagePortLock);
  return self;
}

- (id) initWithName: (NSString*)name
{
  NSMessagePort	*p;
  BOOL		found = NO;

  M_LOCK(messagePortLock);
  p = RETAIN((NSMessagePort*)NSMapGet(ports, (void*)name));
  if (p == nil)
    {
      internal	*this;

      _internal = NSZoneMalloc(NSDefaultMallocZone(), sizeof(internal));
      memset(_internal, '\0', sizeof(internal));
      this = PORT(self);
      self->_is_valid = YES;
      this->name = [name copy];

      this->lock = [GSLazyRecursiveLock new];

      this->rState = RS_NONE;

      this->rHandle = INVALID_HANDLE_VALUE;
      this->rEvent = INVALID_HANDLE_VALUE;
      this->wHandle = INVALID_HANDLE_VALUE;
      this->wEvent = INVALID_HANDLE_VALUE;
    }
  else
    {
      found = YES;
      DESTROY(self);
      self = p;
    }

  /* This is a 'speaking' port ... set it up for write operation
   * if necessary.
   * NB. This must be done (to create the mailbox) before the port
   * is added to the nameserver mapping ... or nother process might
   * try to access the mailbox before it exists.
   */
  if ([self _setupSendPort] == NO)
    {
      DESTROY(self);
    }

  if (self != nil && found == NO)
    {
      /* This was newly created ... add to map so that it can be found.
       */
      NSMapInsert(ports, (void*)[self name], (void*)self);
      NSDebugMLLog(@"NSMessagePort", @"Created speaking port: %@", self);
    }
  M_UNLOCK(messagePortLock);
  return self;
}

- (void) invalidate
{
  RETAIN(self);
  if ([self isValid] == YES)
    {
      internal	*this;

      this = PORT(self);
      M_LOCK(this->lock);
      if ([self isValid] == YES)
	{
	  M_LOCK(messagePortLock);
	  if (this->rHandle != INVALID_HANDLE_VALUE)
	    {
	      (void) CancelIo(this->rHandle);
	    }
	  if (this->wHandle != INVALID_HANDLE_VALUE)
	    {
	      (void) CancelIo(this->wHandle);
	    }
	  if (this->rEvent != INVALID_HANDLE_VALUE)
	    {
	      (void) CloseHandle(this->rEvent);
	      this->rEvent = INVALID_HANDLE_VALUE;
	    }
	  if (this->wEvent != INVALID_HANDLE_VALUE)
	    {
	      (void) CloseHandle(this->wEvent);
	      this->wEvent = INVALID_HANDLE_VALUE;
	    }
	  if (this->rHandle != INVALID_HANDLE_VALUE)
	    {
	      (void) CloseHandle(this->rHandle);
	      this->rHandle = INVALID_HANDLE_VALUE;
	    }
	  if (this->wHandle != INVALID_HANDLE_VALUE)
	    {
	      (void) CloseHandle(this->wHandle);
	      this->wHandle = INVALID_HANDLE_VALUE;
	    }
	  NSMapRemove(ports, (void*)this->name);
	  M_UNLOCK(messagePortLock);

	  [[NSMessagePortNameServer sharedInstance] removePort: self];
	  [super invalidate];
	}
      M_UNLOCK(this->lock);
    }
  RELEASE(self);
}

- (BOOL) isEqual: (id)anObject
{
  if (anObject == self)
    {
      return YES;
    }
  if ([anObject class] == [self class])
    {
      NSMessagePort	*o = (NSMessagePort*)anObject;

      return [PORT(o)->name isEqual: PORT(self)->name];
    }
  return NO;
}

- (NSString*) name
{
  return PORT(self)->name;
}


/*
 * Called when an event occurs on a listener port
 * ALSO called when the port is created, to start reading.
 */
- (void) receivedEventRead
{
  internal	*this = PORT(self);

  M_LOCK(this->lock);

  NSDebugMLLog(@"NSMessagePort", @"entered with rWant=%d", this->rWant);

  if (this->rState == RS_MESG)
    {
      /*
       * Have we read something?
       */
      if (GetOverlappedResult(
	this->rHandle,
	&this->rOv,
	&this->rSize,
	TRUE) == 0)
	{
	  errno = GetLastError();
	  NSDebugMLLog(@"NSMessagePort", @"overlapped result=%d", errno);
	  /*
	   * Our overlapped read attempt should fail ... because mailslots
	   * insist we read an entire message in one go, and we asked it
	   * to read zero bytes.  The error we are expecting is 
	   * ERROR_INSUFFICIENT_BUFFER ... indicating that there is a
	   * message to be read ... so we can ask for its size and read it
	   * synchronously.
	   */
	  if (errno == ERROR_INSUFFICIENT_BUFFER)
	    {
	      this->rState = RS_SIZE;
	    }
	  else
	    {
	      NSLog(@"GetOverlappedResult failed ... %@", [NSError _last]);
	      this->rState = RS_NONE;
	      this->rLength = 0;
	    }
	}
      else
	{
	  NSLog(@"GetOverlappedResult success ... %u", this->rSize);
	  this->rState = RS_NONE;
	  this->rLength = 0;
	}
    }

  if (this->rState == RS_SIZE)
    {
      if (GetMailslotInfo(
	this->rHandle,
	0,
	&this->rWant,
	0,
	0) == 0)
	{
	  NSLog(@"unable to get info from mailslot '%@' - %@",
	    this->name, [NSError _last]);
	  [self invalidate];
	  return;
	}
      else
	{
	  this->rState = RS_DATA;
	  NSDebugMLLog(@"NSMessagePort", @"mailslot size=%d",
	    this->rWant);
	  [this->rData setLength: this->rWant];
	  if (ReadFile(this->rHandle,
	    [this->rData mutableBytes],	// Store results here
	    this->rWant,
	    &this->rSize,
	    NULL) == 0)
	    {
	      NSLog(@"unable to read from mailslot '%@' - %@",
		this->name, [NSError _last]);
	      [self invalidate];
	      return;
	    }
	  if (this->rSize != this->rWant)
	    {
	      NSLog(@"only read %d of %d bytes from mailslot '%@' - %@",
		this->rSize, this->rWant, this->name, [NSError _last]);
	      [self invalidate];
	      return;
	    }
	  else
	    {
	      NSDebugMLLog(@"NSMessagePort", @"Read complete on %@",
		self);
	    }
	  this->rLength = this->rSize;
	  this->rSize = 0;
	  this->rState = RS_NONE;
	}
    }

  /*
   * Do next part only if we have completed a read.
   */
  if (this->rLength > 0 && this->rLength == this->rWant)
    {
      unsigned char	*buf = [this->rData mutableBytes];
      GSPortItemType	rType;
      GSPortItemHeader	*pih;
      unsigned		off = 0;
      unsigned		len;
      unsigned		rId = 0;
      unsigned		nItems = 0;
      NSMessagePort	*rPort = nil;
      NSMutableArray	*rItems = nil;

      while (off + sizeof(GSPortItemHeader) <= this->rLength)
	{
	  pih = (GSPortItemHeader*)(buf + off);
	  off += sizeof(GSPortItemHeader);
	  rType = GSSwapBigI32ToHost(pih->type);
	  len = GSSwapBigI32ToHost(pih->length);
	  if (len + off > this->rLength)
	    {
	      NSLog(@"%@ - unreasonable length (%u) for data", self, len);
	      break;
	    }
	  if (rType != GSP_HEAD && rItems == nil)
	    {
	      NSLog(@"%@ - initial part of message had bad type");
	      break;
	    }

	  if (rType == GSP_HEAD)
	    {
	      GSPortMsgHeader	*pmh;
	      NSString		*n;
	      NSMutableData		*d;

	      if (len < sizeof(GSPortMsgHeader))
		{
		  NSLog(@"%@ - bad length for header", self);
		  break;
		}
	      pmh = (GSPortMsgHeader*)(buf + off);
	      off += sizeof(GSPortMsgHeader);
	      len -= sizeof(GSPortMsgHeader);
	      rId = GSSwapBigI32ToHost(pmh->mId);
	      nItems = GSSwapBigI32ToHost(pmh->nItems);
	      if (nItems == 0)
		{
		  NSLog(@"%@ - unable to decode item count", self);
		  break;
		}
	      n = [[NSString alloc] initWithBytes: pmh->port
					   length: 16
					 encoding: NSASCIIStringEncoding];
	      NSDebugFLLog(@"NSMessagePort", @"Decoded port as '%@'", n);
	      rPort = [messagePortClass newWithName: n];
	      RELEASE(n);
	      if (rPort == nil)
		{
		  NSLog(@"%@ - unable to decode remote port", self);
		  break;
		}
	      rItems = [NSMutableArray alloc];
	      rItems = [rItems initWithCapacity: nItems];
	      d = [[NSMutableData alloc] initWithBytes: buf + off
						length: len];
	      [rItems addObject: d];
	      RELEASE(d);
	    }
	  else if (rType == GSP_DATA)
	    {
	      NSMutableData	*d;

	      d = [[NSMutableData alloc] initWithBytes: buf + off
						length: len];
	      [rItems addObject: d];
	      RELEASE(d);
	    }
	  else if (rType == GSP_PORT)
	    {
	      NSMessagePort	*p;
	      NSString	*n;

	      if (len != 16)
		{
		  NSLog(@"%@ - bad length for port item", self);
		  break;
		}
	      n = [[NSString alloc] initWithBytes: buf + off
					   length: 16
					 encoding: NSASCIIStringEncoding];
	      NSDebugFLLog(@"NSMessagePort", @"Decoded port as '%@'", n);
	      p = [messagePortClass newWithName: n];
	      RELEASE(n);
	      if (p == nil)
		{
		  NSLog(@"%@ - unable to decode remote port", self);
		  break;
		}
	      [rItems addObject: p];
	      RELEASE(p);
	    }
	  off += len;
	  if (nItems == [rItems count])
	    {
	      NSPortMessage	*pm;

	      pm = [NSPortMessage allocWithZone: NSDefaultMallocZone()];
	      pm = [pm initWithSendPort: rPort
			    receivePort: self
			     components: rItems];
	      DESTROY(rPort);
	      DESTROY(rItems);
	      [pm setMsgid: rId];
	      [this->rMsgs addObject: pm];
	      RELEASE(pm);
	      break;
	    }
	}
      DESTROY(rPort);
      DESTROY(rItems);
    }

  /*
   * Got something ... is it all we want? If not, ask to read more.
   */
  if ([self isValid] == YES
    && (this->rState == RS_NONE || this->rLength < this->rWant))
    {
      int	rc;

      this->rOv.Offset = 0;
      this->rOv.OffsetHigh = 0;
      this->rOv.hEvent = this->rEvent;
      if (this->rState == RS_NONE)
	{
	  this->rLength = 0;
	  this->rWant = 1;
	}
      if ([this->rData length] < (this->rWant - this->rLength))
	{
	  [this->rData setLength: this->rWant - this->rLength];
	}
      rc = ReadFile(this->rHandle,
	[this->rData mutableBytes],	// Store results here
	this->rWant - this->rLength,
	&this->rSize,
	&this->rOv);

      if (rc > 0)
	{
	  NSDebugMLLog(@"NSMessagePort", @"Read immediate on %@", self);
	  if (this->rState == RS_NONE)
	    {
	      this->rState = RS_SIZE;
	    }
	  SetEvent(this->rEvent);
	}
      else if ((errno = GetLastError()) == ERROR_IO_PENDING)
	{
	  NSDebugMLLog(@"NSMessagePort", @"Read queued on %@", self);
	  if (this->rState == RS_NONE)
	    {
	      this->rState = RS_MESG;
	    }
	}
      else if (errno == ERROR_INSUFFICIENT_BUFFER)
	{
	  NSDebugMLLog(@"NSMessagePort", @"Read retry on %@", self);
	  if (this->rState == RS_NONE)
	    {
	      this->rState = RS_SIZE;
	    }
	  SetEvent(this->rEvent);
	}
      else
	{
	  NSLog(@"unable to read from mailslot '%@' - %@",
	    this->name, [NSError _last]);
	  [self invalidate];
	}
    }

  while ([this->rMsgs count] > 0)
    {
      NSPortMessage	*pm;

      pm = RETAIN([this->rMsgs objectAtIndex: 0]);
      [this->rMsgs removeObjectAtIndex: 0];

      NSDebugMLLog(@"NSMessagePort", @"got message %@ on 0x%x", pm, self);
      M_UNLOCK(this->lock);
      NS_DURING
	{
	  [self handlePortMessage: pm];
	}
      NS_HANDLER
	{
	  M_LOCK(this->lock);
	  RELEASE(pm);
	  [localException raise];
	}
      NS_ENDHANDLER
      M_LOCK(this->lock);
      RELEASE(pm);
    }

  M_UNLOCK(this->lock);
}

/*
 * Called when an event occurs on a speaker port
 * ALSO called when we start trying to write a new message and there
 * wasn't one in progress.
 */
- (void) receivedEventWrite
{
  internal	*this = PORT(self);

  M_LOCK(this->lock);

  if (this->wData != nil)
    {
      /*
       * Have we written something?
       */
      if (GetOverlappedResult(
	this->wHandle,
	&this->wOv,
	&this->wSize,
	TRUE) == 0)
	{
	  NSLog(@"GetOverlappedResult failed ... %@", [NSError _last]);
	}
      else
	{
	  this->wLength += this->wSize;
	  this->wSize = 0;
	}
    }

again:

  /*
   * Handle start of next data item if we have completed one,
   * or if we are called without a write in progress.
   */
  if (this->wData == nil || this->wLength == [this->wData length])
    {
      if (this->wData != nil)
	{
	  NSDebugMLLog(@"NSMessagePort",
	    @"completed write on 0x%x", self);
	  [this->wMsgs removeObjectIdenticalTo: this->wData];
	  this->wData = nil;
	}
      if ([this->wMsgs count] > 0)
	{
	  this->wData = [this->wMsgs objectAtIndex: 0];
	}
      this->wLength = 0;	// Nothing written yet.
    }

  if (this->wData != nil)
    {
      int	rc;

      this->wOv.Offset = 0;
      this->wOv.OffsetHigh = 0;
      this->wOv.hEvent = this->wEvent;
      rc = WriteFile(this->wHandle,
	[this->wData bytes],			// Output from here
	[this->wData length] - this->wLength,
	&this->wSize,				// Store number of bytes written
	&this->wOv);
      if (rc > 0)
	{
	  NSDebugMLLog(@"NSMessagePort", @"Write of %d performs %d",
	    [this->wData length] - this->wLength, this->wSize);
	  this->wLength += this->wSize;
	  goto again;
	}
      else if ((errno = GetLastError()) != ERROR_IO_PENDING)
	{
	  /* This is probably an end of file
	   * eg. when the process at the other end has terminated.
	   */
	  NSDebugMLog(@"NSMessagePort",
	    @"unable to write to mailslot '%@' - %@",
	    this->name, [NSError _last]);
	  [self invalidate];
	}
      else
	{
	  NSDebugMLLog(@"NSMessagePort", @"Write of %d queued",
	    [this->wData length] - this->wLength);
	}
    }
  M_UNLOCK(this->lock);
}

- (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
		 extra: (void*)extra
	       forMode: (NSString*)mode
{
  RETAIN(self);
  if ([self isValid] == YES)
    {
      internal	*this = PORT(self);

      NSDebugMLLog(@"NSMessagePort", @"got event on %@ in mode %@", self, mode);
      if (this->rEvent == (HANDLE)data)
	{
	  [self receivedEventRead];
	}
      else
	{
	  [self receivedEventWrite];
	}
    }
  else
    {
      NSDebugMLLog(@"NSMessagePort",
	@"got event on invalidated port 0x%x in mode %@", self, mode);
    }
  RELEASE(self);
}


- (void) removeConnection: (NSConnection*)aConnection
              fromRunLoop: (NSRunLoop*)aLoop
                  forMode: (NSString*)aMode
{
  NSDebugMLLog(@"NSMessagePort", @"%@ remove from 0x%x in mode %@",
    self, aLoop, aMode);
  [aLoop removeEvent: (void*)(uintptr_t)PORT(self)->rEvent
		type: ET_HANDLE
	     forMode: aMode
		 all: NO];
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

- (void) release
{
  /* We lock the port table while checking, to prevent
   * another thread from grabbing this port while we are
   * checking it.
   * If we are going to deallocate the object, we first remove
   * it from the table so that no other thread will find it
   * and try to use it while it is being deallocated.
   */
  M_LOCK(messagePortLock);
  if (NSDecrementExtraRefCountWasZero(self))
    {
      NSMapRemove(ports, (void*)[self name]);
      M_UNLOCK(messagePortLock);
      [self dealloc];
    }
  else
    {
      M_UNLOCK(messagePortLock);
    }
}

- (BOOL) sendBeforeDate: (NSDate*)when
		  msgid: (int)msgId
             components: (NSMutableArray*)components
                   from: (NSPort*)receivingPort
               reserved: (NSUInteger)length
{
  NSMutableData		*h = nil;
  NSMutableData		*first;
  BOOL			sent = NO;
  unsigned		c;
  unsigned		i;
  internal		*this;
  GSPortItemHeader	*pih;
  GSPortMsgHeader	*pmh;

  NSDebugMLLog(@"NSMessagePort",
    @"send message\n  Send: %@\n  Recv: %@\n  Components -\n%@",
    self, receivingPort, components);

  if ([self isValid] == NO)
    {
      return NO;
    }
  this = PORT(self);

  if (PORT(self)->wHandle == INVALID_HANDLE_VALUE
    && [self _setupSendPort] == NO)
    {
      NSLog(@"Attempt to send through recv port");
    }

  c = (unsigned)[components count];
  if (c == 0)
    {
      NSLog(@"empty components sent");
      return NO;
    }
  /*
   * If the reserved length in the first data object is wrong - we have to
   * fail.
   */
  if (length != [self reservedSpaceLength])
    {
      NSLog(@"bad reserved length - %u", length);
      return NO;
    }
  NSAssert([receivingPort isKindOfClass: messagePortClass] == YES,
    @"Receiving port is not the correct type");
  NSAssert([receivingPort isValid] == YES,
    @"Receiving port is not valid");
  NSAssert(PORT(receivingPort)->rHandle != INVALID_HANDLE_VALUE,
    @"Attempt to send to send port");

  first = [components objectAtIndex: 0];
  if (c == 1)
    {
      //h = RETAIN(first);
      h = [first mutableCopy];
    }
  if (c > 1)
    {
      unsigned	l = 0;
      id	o;

      for (i = 1; i < c; i++)
	{
	  o = [components objectAtIndex: i];
	  if ([o isKindOfClass: [NSData class]] == YES)
	    {
	      l += sizeof(GSPortItemHeader) + [o length];
	    }
	  else
	    {
	      l += sizeof(GSPortItemHeader) + 16;	// A port
	    }
	}
      h = [[NSMutableData alloc] initWithCapacity: [first length] + l];
      [h appendData: first];
  
      for (i = 1; i < c; i++)
	{
	  GSPortItemHeader	ih;
	      
	  o = [components objectAtIndex: i];
	  if ([o isKindOfClass: [NSData class]] == YES)
	    {
	      ih.type = GSSwapHostI32ToBig(GSP_DATA);
	      ih.length = GSSwapHostI32ToBig([o length]);
	      [h appendBytes: &ih length: sizeof(ih)];
	      [h appendData: o];
	    }
	  else
	    {
	      ih.type = GSSwapHostI32ToBig(GSP_PORT);
	      ih.length = GSSwapHostI32ToBig(16);
	      [h appendBytes: &ih length: sizeof(ih)];
	      [h appendBytes: [o UTF8String] length: 16];
	    }
	}
    }

  pih = (GSPortItemHeader*)[h mutableBytes];
  pih->type = GSSwapHostI32ToBig(GSP_HEAD);
  pih->length = GSSwapHostI32ToBig([first length] - sizeof(GSPortItemHeader));
  pmh = (GSPortMsgHeader*)&pih[1];
  pmh->mId = GSSwapHostI32ToBig(msgId);
  pmh->nItems = GSSwapHostI32ToBig(c);
  pmh->version = 0;
  memcpy(pmh->port, [[(NSMessagePort*)receivingPort name] UTF8String], 16);
 
  /*
   * Now send the message.
   */

  RETAIN(self);
  M_LOCK(this->lock);
  [this->wMsgs addObject: h];
  if (this->wData == nil)
    {
      [self receivedEventWrite];	// Start async write.
    }

  if ([this->wMsgs indexOfObjectIdenticalTo: h] == NSNotFound)
    {
      sent = YES;			// Write completed synchronously
    }
  else
    {
      NSRunLoop		*loop = [NSRunLoop currentRunLoop];

      [loop addEvent: (void*)(uintptr_t)this->wEvent
		type: ET_HANDLE
	     watcher: (id<RunLoopEvents>)self
	     forMode: NSConnectionReplyMode];
      [loop addEvent: (void*)(uintptr_t)this->wEvent
		type: ET_HANDLE
	     watcher: (id<RunLoopEvents>)self
	     forMode: NSDefaultRunLoopMode];

      while ([self isValid] == YES
	&& [this->wMsgs indexOfObjectIdenticalTo: h] != NSNotFound
	&& [when timeIntervalSinceNow] > 0)
	{
	  M_UNLOCK(this->lock);
          NS_DURING
	    [loop runMode: NSConnectionReplyMode beforeDate: when];
          NS_HANDLER
            M_LOCK(this->lock);
            [loop removeEvent: (void*)(uintptr_t)this->wEvent
                         type: ET_HANDLE
                      forMode: NSConnectionReplyMode
                          all: NO];
            [loop removeEvent: (void*)(uintptr_t)this->wEvent
                         type: ET_HANDLE
                      forMode: NSDefaultRunLoopMode
                          all: NO];
            M_UNLOCK(this->lock);
            [localException raise];
          NS_ENDHANDLER
	  M_LOCK(this->lock);
	}

      [loop removeEvent: (void*)(uintptr_t)this->wEvent
		   type: ET_HANDLE
		forMode: NSConnectionReplyMode
		    all: NO];
      [loop removeEvent: (void*)(uintptr_t)this->wEvent
		   type: ET_HANDLE
		forMode: NSDefaultRunLoopMode
		    all: NO];

      if ([this->wMsgs indexOfObjectIdenticalTo: h] == NSNotFound)
	{
	  sent = YES;
	}
    }
  M_UNLOCK(this->lock);
  RELEASE(h);
  RELEASE(self);

  return sent;
}

- (NSDate*) timedOutEvent: (void*)data
		     type: (RunLoopEventType)type
		  forMode: (NSString*)mode
{
  return nil;
}

@end


