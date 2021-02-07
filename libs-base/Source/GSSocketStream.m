/** Implementation for GSSocketStream for GNUStep
   Copyright (C) 2006-2008 Free Software Foundation, Inc.

   Written by:  Derek Zhou <derekzhou@gmail.com>
   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2006

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

#import "Foundation/NSArray.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSHost.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSValue.h"

#import "GSPrivate.h"
#import "GSStream.h"
#import "GSSocketStream.h"

#import "GNUstepBase/GSTLS.h"

#ifndef SHUT_RD
# ifdef  SD_RECEIVE
#   define SHUT_RD      SD_RECEIVE
#   define SHUT_WR      SD_SEND
#   define SHUT_RDWR    SD_BOTH
# else
#   define SHUT_RD      0
#   define SHUT_WR      1
#   define SHUT_RDWR    2
# endif
#endif

#ifdef _WIN32
  #ifdef HAVE_WS2TCPIP_H
  #include <ws2tcpip.h>
  #endif // HAVE_WS2TCPIP_H

  #if !defined(HAVE_INET_NTOP)
  extern const char* WSAAPI inet_ntop(int, const void *, char *, size_t);
  #endif
  #if !defined(HAVE_INET_NTOP)
  extern int WSAAPI inet_pton(int , const char *, void *);
  #endif

  #define	OPTLEN	int
#else  // _WIN32
  #define	OPTLEN	socklen_t
#endif // _WIN32

unsigned
GSPrivateSockaddrLength(struct sockaddr *addr)
{
  switch (addr->sa_family) {
    case AF_INET:       return sizeof(struct sockaddr_in);
#ifdef	AF_INET6
    case AF_INET6:      return sizeof(struct sockaddr_in6);
#endif
#ifndef	_WIN32
    case AF_LOCAL:       return sizeof(struct sockaddr_un);
#endif
    default:            return 0;
  }
}

NSString *
GSPrivateSockaddrHost(struct sockaddr *addr)
{
  char		buf[40];

#if     defined(AF_INET6)
  if (AF_INET6 == addr->sa_family)
    {
      struct sockaddr_in6	*addr6 = (struct sockaddr_in6*)(void*)addr;

      inet_ntop(AF_INET, &addr6->sin6_addr, buf, sizeof(buf));
      return [NSString stringWithUTF8String: buf];
    }
#endif
  inet_ntop(AF_INET, &((struct sockaddr_in*)(void*)addr)->sin_addr,
		  buf, sizeof(buf));
  return [NSString stringWithUTF8String: buf];
}

NSString *
GSPrivateSockaddrName(struct sockaddr *addr)
{
  return [NSString stringWithFormat: @"%@:%d",
    GSPrivateSockaddrHost(addr),
    GSPrivateSockaddrPort(addr)];
}

uint16_t
GSPrivateSockaddrPort(struct sockaddr *addr)
{
  uint16_t	port;

#if     defined(AF_INET6)
  if (AF_INET6 == addr->sa_family)
    {
      struct sockaddr_in6	*addr6 = (struct sockaddr_in6*)(void*)addr;

      port = addr6->sin6_port;
      port = GSSwapBigI16ToHost(port);
      return port;
    }
#endif
  port = ((struct sockaddr_in*)(void*)addr)->sin_port;
  port = GSSwapBigI16ToHost(port);
  return port;
}

BOOL
GSPrivateSockaddrSetup(NSString *machine, uint16_t port,
  NSString *service, NSString *protocol, struct sockaddr *sin)
{
  memset(sin, '\0', sizeof(*sin));
  sin->sa_family = AF_INET;

  /* If we were given a hostname, we use any address for that host.
   * Otherwise we expect the given name to be an address unless it is
   * a null (any address).
   */
  if (0 != [machine length])
    {
      const char	*n;

      n = [machine UTF8String];
      if ((!isdigit(n[0]) || sscanf(n, "%*d.%*d.%*d.%*d") != 4)
	&& 0 == strchr(n, ':'))
	{
	  machine = [[NSHost hostWithName: machine] address];
	  n = [machine UTF8String];
	}

      if (0 == n)
	{
	  return NO;
	}
      if (0 == strchr(n, ':'))
	{
	  struct sockaddr_in	*addr = (struct sockaddr_in*)(void*)sin;

	  if (inet_pton(AF_INET, n, &addr->sin_addr) <= 0)
	    {
	      return NO;
	    }
	}
      else
	{
#if     defined(AF_INET6)
	  struct sockaddr_in6	*addr6 = (struct sockaddr_in6*)(void*)sin;

	  sin->sa_family = AF_INET6;
	  if (inet_pton(AF_INET6, n, &addr6->sin6_addr) <= 0)
	    {
	      return NO;
	    }
#else
	  return NO;
#endif
	}
    }
  else
    {
      ((struct sockaddr_in*)(void*)sin)->sin_addr.s_addr
	= GSSwapHostI32ToBig(INADDR_ANY);
    }

  /* The optional service and protocol parameters may be used to
   * look up the port
   */
  if (nil != service)
    {
      const char	*sname;
      const char	*proto;
      struct servent	*sp;

      if (nil == protocol)
	{
	  proto = "tcp";
	}
      else
	{
	  proto = [protocol UTF8String];
	}

      sname = [service UTF8String];
      if ((sp = getservbyname(sname, proto)) == 0)
	{
	  const char*     ptr = sname;
	  int             val = atoi(ptr);

	  while (isdigit(*ptr))
	    {
	      ptr++;
	    }
	  if (*ptr == '\0' && val <= 0xffff)
	    {
	      port = val;
	    }
	  else if (strcmp(ptr, "gdomap") == 0)
	    {
#ifdef	GDOMAP_PORT_OVERRIDE
	      port = GDOMAP_PORT_OVERRIDE;
#else
	      port = 538;	// IANA allocated port
#endif
	    }
	  else
	    {
	      return NO;
	    }
	}
      else
	{
	  port = GSSwapBigI16ToHost(sp->s_port);
	}
    }

#if     defined(AF_INET6)
  if (AF_INET6 == sin->sa_family)
    {
      ((struct sockaddr_in6*)(void*)sin)->sin6_port = GSSwapHostI16ToBig(port);
    }
  else
    {
      ((struct sockaddr_in*)(void*)sin)->sin_port = GSSwapHostI16ToBig(port);
    }
#else
  ((struct sockaddr_in*)sin)->sin_port = GSSwapHostI16ToBig(port);
#endif
  return YES;
}

/** The GSStreamHandler abstract class defines the methods used to
 * implement a handler object for a pair of streams.
 * The idea is that the handler is installed once the connection is
 * open, and a handshake is initiated.  During the handshake process
 * all stream events are sent to the handler rather than to the
 * stream delegate (the streams know to do this because the -handshake
 * method returns YES to tell them so).
 * While a handler is installed, the -read:maxLength: and -write:maxLength:
 * methods of the handle rare called instead of those of the streams (and
 * the handler may perform I/O using the streams by calling the private
 * -_read:maxLength: and _write:maxLength: methods instead of the public
 * methods).
 */
@interface      GSStreamHandler : NSObject
{
  GSSocketInputStream   *istream;	// Not retained
  GSSocketOutputStream  *ostream;       // Not retained
  BOOL                  initialised;
  BOOL                  handshake;
  BOOL                  active;
}
+ (void) tryInput: (GSSocketInputStream*)i output: (GSSocketOutputStream*)o;
- (id) initWithInput: (GSSocketInputStream*)i
              output: (GSSocketOutputStream*)o;
- (GSSocketInputStream*) istream;
- (GSSocketOutputStream*) ostream;

- (void) bye;           /* Close down the handled session.   */
- (BOOL) handshake;     /* A handshake/hello is in progress. */
- (void) hello;         /* Start up the session handshake.   */
- (NSInteger) read: (uint8_t *)buffer maxLength: (NSUInteger)len;
- (void) remove: (NSStream*)stream;	/* Stream no longer available */
- (void) stream: (NSStream*)stream handleEvent: (NSStreamEvent)event;
- (NSInteger) write: (const uint8_t *)buffer maxLength: (NSUInteger)len;
@end


@implementation GSStreamHandler

+ (void) initialize
{
  GSMakeWeakPointer(self, "istream");
  GSMakeWeakPointer(self, "ostream");
}

+ (void) tryInput: (GSSocketInputStream*)i output: (GSSocketOutputStream*)o
{
  [self subclassResponsibility: _cmd];
}

- (void) bye
{
  [self subclassResponsibility: _cmd];
}

- (BOOL) handshake
{
  return handshake;
}

- (void) hello
{
  [self subclassResponsibility: _cmd];
}

- (id) initWithInput: (GSSocketInputStream*)i
              output: (GSSocketOutputStream*)o
{
  istream = i;
  ostream = o;
  handshake = YES;
  return self;
}

- (GSSocketInputStream*) istream
{
  return istream;
}

- (GSSocketOutputStream*) ostream
{
  return ostream;
}

- (NSInteger) read: (uint8_t *)buffer maxLength: (NSUInteger)len
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (void) remove: (NSStream*)stream
{
  if ((id)stream == (id)istream)
    {
      istream = nil;
    }
  if ((id)stream == (id)ostream)
    {
      ostream = nil;
    }
}

- (void) stream: (NSStream*)stream handleEvent: (NSStreamEvent)event
{
  [self subclassResponsibility: _cmd];
}

- (NSInteger) write: (const uint8_t *)buffer maxLength: (NSUInteger)len
{
  [self subclassResponsibility: _cmd];
  return 0;
}

@end

#if     defined(HAVE_GNUTLS)

@interface      GSTLSHandler : GSStreamHandler
{
@public
  GSTLSSession  *session;
}

/** Populates the dictionary 'dict', copying in all the properties
 * of the supplied streams. If a property is set for both then
 * the output stream's one has precedence.
 */
+ (void) populateProperties: (NSMutableDictionary**)dict
          withSecurityLevel: (NSString*)l
            fromInputStream: (NSStream*)i
             orOutputStream: (NSStream*)o;

/** Called on verification of the remote end's certificate to tell the
 * delegate of the input stream who the certificate issuer and owner are.
 */
- (void) stream: (NSStream*)stream issuer: (NSString*)i owner: (NSString*)o;

@end

/* Callback to allow the TLS code to pull data from the remote system.
 * If the operation fails, this sets the error number.
 */
static ssize_t
GSTLSPull(gnutls_transport_ptr_t handle, void *buffer, size_t len)
{
  ssize_t       result;
  GSTLSHandler  *tls = (GSTLSHandler*)handle;

  result = [[tls istream] _read: buffer maxLength: len];
  if (result < 0)
    {
      int       e;

      if ([[tls istream] streamStatus] == NSStreamStatusError)
        {
          e = [[[(GSTLSHandler*)handle istream] streamError] code];
        }
      else
        {
          e = EAGAIN;	// Tell GNUTLS this would block.
        }
#if	HAVE_GNUTLS_TRANSPORT_SET_ERRNO
      gnutls_transport_set_errno (tls->session->session, e);
#else
      errno = e;	// Not thread-safe
#endif
    }
  return result;
}

/* Callback to allow the TLS code to push data to the remote system.
 * If the operation fails, this sets the error number.
 */
static ssize_t
GSTLSPush(gnutls_transport_ptr_t handle, const void *buffer, size_t len)
{
  ssize_t       result;
  GSTLSHandler  *tls = (GSTLSHandler*)handle;

  result = [[tls ostream] _write: buffer maxLength: len];
  if (result < 0)
    {
      int       e;

      if ([[tls ostream] streamStatus] == NSStreamStatusError)
        {
          e = [[[tls ostream] streamError] code];
        }
      else
        {
          e = EAGAIN;	// Tell GNUTLS this would block.
        }
#if	HAVE_GNUTLS_TRANSPORT_SET_ERRNO
      gnutls_transport_set_errno (tls->session->session, e);
#else
      errno = e;	// Not thread-safe
#endif

    }
  NSDebugFLLog(@"NSStream", @"GSTLSPush write %p of %u on %u",
    [tls ostream], (unsigned)result, (unsigned)len);
  return result;
}

@implementation GSTLSHandler

static NSArray  *keys = nil;

+ (void) initialize
{
  [GSTLSObject class];
  if (nil == keys)
    {
      keys = [[NSArray alloc] initWithObjects:
        GSTLSCAFile,
        GSTLSCertificateFile,
        GSTLSCertificateKeyFile,
        GSTLSCertificateKeyPassword,
        GSTLSDebug,
        GSTLSPriority,
        GSTLSRemoteHosts,
        GSTLSRevokeFile,
        GSTLSServerName,
        GSTLSVerify,
        nil];
      [[NSObject leakAt: &keys] release];
    }
}

+ (void) populateProperties: (NSMutableDictionary**)dict
	  withSecurityLevel: (NSString*)l
	    fromInputStream: (NSStream*)i
	     orOutputStream: (NSStream*)o
{
  if (NULL != dict)
    {
      NSString                  *str;
      NSMutableDictionary       *opts = *dict;
      NSUInteger                count;
      
      if (nil != l)
	{
	  [opts setObject: l forKey: NSStreamSocketSecurityLevelKey];
	}
      count = [keys count];
      while (count-- > 0)
	{
	  NSString  *key = [keys objectAtIndex: count];

	  str = [o propertyForKey: key];
	  if (nil == str) str = [i propertyForKey: key];
	  if (nil != str) [opts setObject: str forKey: key];
	}
    }
  else
    {
      NSWarnLog(@"%@ requires not nil 'dict'", NSStringFromSelector(_cmd));
    }
}

+ (void) tryInput: (GSSocketInputStream*)i output: (GSSocketOutputStream*)o
{
  NSString      *tls;

  tls = [i propertyForKey: NSStreamSocketSecurityLevelKey];
  if (tls == nil)
    {
      tls = [o propertyForKey: NSStreamSocketSecurityLevelKey];
      if (tls != nil)
        {
          [i setProperty: tls forKey: NSStreamSocketSecurityLevelKey];
        }
    }
  else
    {
      [o setProperty: tls forKey: NSStreamSocketSecurityLevelKey];
    }

  if (tls != nil)
    {
      GSTLSHandler      *h;

      h = [[GSTLSHandler alloc] initWithInput: i output: o];
      [i _setHandler: h];
      [o _setHandler: h];
      RELEASE(h);
    }
}

- (void) bye
{
  handshake = NO;
  active = NO;
  [session disconnect: NO];
}

- (void) dealloc
{
  [self bye];
  DESTROY(session);
  [super dealloc];
}

- (BOOL) handshake
{
  return handshake;
}

- (void) hello
{
  if (active == NO)
    {
      if (handshake == NO)
        {
          /* Set flag to say we are now doing a handshake.
           */
          handshake = YES;
        }
      if ([session handshake] == YES)
        {
          handshake = NO;               // Handshake is now complete.
          active = [session active];    // Is the TLS session now active?
          if (NO == active)
            {
              NSString  *problem = [session problem];
              NSError   *theError;

              if (nil == problem)
                {
                  problem = @"TLS handshake failure";
                }
              theError = [NSError errorWithDomain: NSCocoaErrorDomain
                code: 0
                userInfo: [NSDictionary dictionaryWithObject: problem
                  forKey: NSLocalizedDescriptionKey]];
              if ([istream streamStatus] != NSStreamStatusError)
                {
                  [istream _recordError: theError];
                }
              if ([ostream streamStatus] != NSStreamStatusError)
                {
                  [ostream _recordError: theError];
                }
              [self bye];
            }
          else
            {
              NSString  *issuer = [session issuer];
              NSString  *owner = [session owner];
              id        del = [istream delegate];

              if (nil != issuer && nil != owner
                && [del respondsToSelector: @selector(stream:issuer:owner:)])
                {
                  [del stream: istream issuer: issuer owner: owner];
                }
            }
        }
    }
}

- (id) initWithInput: (GSSocketInputStream*)i
              output: (GSSocketOutputStream*)o
{
  NSString              *str;
  NSMutableDictionary   *opts;
  BOOL		        server;

  // Check whether the input stream has been accepted by a listening socket
  server = [[i propertyForKey: @"IsServer"] boolValue];

  str = [o propertyForKey: NSStreamSocketSecurityLevelKey];
  if (nil == str) str = [i propertyForKey: NSStreamSocketSecurityLevelKey];
  if ([str isEqual: NSStreamSocketSecurityLevelNone] == YES)
    {
      GSOnceMLog(@"NSStreamSocketSecurityLevelNone is insecure ..."
        @" not implemented");
      DESTROY(self);
      return nil;
    }
  else if ([str isEqual: NSStreamSocketSecurityLevelSSLv2] == YES)
    {
      GSOnceMLog(@"NSStreamSocketSecurityLevelTLSv2 is insecure ..."
        @" not implemented");
      DESTROY(self);
      return nil;
    }
  else if ([str isEqual: NSStreamSocketSecurityLevelSSLv3] == YES)
    {
      str = @"SSLv3";
    }
  else if ([str isEqual: NSStreamSocketSecurityLevelTLSv1] == YES)
    {
      str = @"TLSV1";
    }
  else
    {
      str = nil;
    }

  if ((self = [super initWithInput: i output: o]) == nil)
    {
      return nil;
    }

  /* Create the options dictionary, copying in any option from the stream
   * properties.  GSTLSPriority overrides NSStreamSocketSecurityLevelKey.
   */
  opts = [NSMutableDictionary new];
  [[self class] populateProperties: &opts
		 withSecurityLevel: str
		   fromInputStream: i
		    orOutputStream: o];
  
  session = [[GSTLSSession alloc] initWithOptions: opts
                                        direction: (server ? NO : YES)
                                        transport: (void*)self
                                             push: GSTLSPush
                                             pull: GSTLSPull];
  [opts release];
  initialised = YES;
  return self;
}

- (GSSocketInputStream*) istream
{
  return istream;
}

- (GSSocketOutputStream*) ostream
{
  return ostream;
}

- (NSInteger) read: (uint8_t *)buffer maxLength: (NSUInteger)len
{
  return [session read: buffer length: len];
}

- (void) stream: (NSStream*)stream handleEvent: (NSStreamEvent)event
{
  NSDebugMLLog(@"NSStream",
    @"GSTLSHandler got %@ on %@", [stream stringFromEvent: event], stream);

  if (handshake == YES)
    {
      switch (event)
        {
          case NSStreamEventHasSpaceAvailable:
          case NSStreamEventHasBytesAvailable:
          case NSStreamEventOpenCompleted:
            /* try to complete the handshake.
             */
            [self hello];
            break;

          case NSStreamEventErrorOccurred:
          case NSStreamEventEndEncountered:
            /* stream error or close ... handshake fails.
             */
            handshake = NO;
            break;

          default:
            break;
        }
      if (NO == handshake)
        {
          NSDebugMLLog(@"NSStream",
            @"GSTLSHandler completed on %@", stream);

          /* Make sure that, if ostream gets released as a result of
           * the event we send to istream, it doesn't get deallocated
           * and cause a crash when we try to send to it.
           */
          AUTORELEASE(RETAIN(ostream));
          if ([istream streamStatus] == NSStreamStatusOpen)
            {
              [istream _resetEvents: NSStreamEventOpenCompleted];
              [istream _sendEvent: NSStreamEventOpenCompleted];
            }
          else
            {
              [istream _resetEvents: NSStreamEventErrorOccurred];
              [istream _sendEvent: NSStreamEventErrorOccurred];
            }
          if ([ostream streamStatus] == NSStreamStatusOpen)
            {
              [ostream _resetEvents: NSStreamEventOpenCompleted
                | NSStreamEventHasSpaceAvailable];
              [ostream _sendEvent: NSStreamEventOpenCompleted];
              [ostream _sendEvent: NSStreamEventHasSpaceAvailable];
            }
          else
            {
              [ostream _resetEvents: NSStreamEventErrorOccurred];
              [ostream _sendEvent: NSStreamEventErrorOccurred];
            }
        }
    }
}

- (void) stream: (NSStream*)stream issuer: (NSString*)i owner: (NSString*)o
{
  return;
}

- (NSInteger) write: (const uint8_t *)buffer maxLength: (NSUInteger)len
{
  NSInteger	offset = 0;

  /* The low level code to perform the TLS session write may return a
   * partial write even though the output stream is still writable.
   * That means we wouldn't get an event to say there's more space and
   * our overall write (for a large amount of data) could hang.  
   * To avoid that, we try writing more data as long as the stream
   * still has space available.
   */
  while ([ostream hasSpaceAvailable] && offset < len)
    {
      NSInteger	written;

      written = [session write: buffer + offset length: len - offset];
      if (written > 0)
	{
	  offset += written;
	}
    }
  return offset;
}

@end

#else   /* HAVE_GNUTLS */

/* GNUTLS not available ...
 */
@interface      GSTLSHandler : GSStreamHandler
@end
@implementation GSTLSHandler

static NSArray  *keys = nil;

+ (void) initialize
{
  if (nil == keys)
    {
      keys = [[NSArray alloc] initWithObjects:
        GSTLSCAFile,
        GSTLSCertificateFile,
        GSTLSCertificateKeyFile,
        GSTLSCertificateKeyPassword,
        GSTLSDebug,
        GSTLSPriority,
        GSTLSRemoteHosts,
        GSTLSRevokeFile,
        GSTLSVerify,
        nil];
      [[NSObject leakAt: &keys] release];
    }
}

+ (void) populateProperties: (NSMutableDictionary**)dict
	  withSecurityLevel: (NSString*)l
	    fromInputStream: (NSStream*)i
	     orOutputStream: (NSStream*)o
{
  NSString              *str;
  NSMutableDictionary   *opts = *dict;
  NSUInteger            count;
  
  if (NULL != dict)
    {
      if (nil != l)
	{
	  [opts setObject: l forKey: NSStreamSocketSecurityLevelKey];
	}
      count = [keys count];
      while (count-- > 0)
	{
	  NSString  *key = [keys objectAtIndex: count];

	  str = [o propertyForKey: key];
	  if (nil == str) str = [i propertyForKey: key];
	  if (nil != str) [opts setObject: str forKey: key];
	}
    }
  else
    {
      NSWarnLog(@"%@ requires not nil 'dict'", NSStringFromSelector(_cmd));
    }
}

+ (void) tryInput: (GSSocketInputStream*)i output: (GSSocketOutputStream*)o
{
  NSString	*tls;

  tls = [i propertyForKey: NSStreamSocketSecurityLevelKey];
  if (tls == nil)
    {
      tls = [o propertyForKey: NSStreamSocketSecurityLevelKey];
    }
  if (tls != nil
    && [tls isEqualToString: NSStreamSocketSecurityLevelNone] == NO)
    {
      NSLog(@"Attempt to use SSL/TLS without support.");
      NSLog(@"Please reconfigure gnustep-base with GNU TLS.");
    }
  return;
}
- (id) initWithInput: (GSSocketInputStream*)i
              output: (GSSocketOutputStream*)o
{
  DESTROY(self);
  return nil;
}
@end

#endif   /* HAVE_GNUTLS */



/*
 * States for socks connection negotiation
 */
static NSString * const GSSOCKSOfferAuth = @"GSSOCKSOfferAuth";
static NSString * const GSSOCKSRecvAuth = @"GSSOCKSRecvAuth";
static NSString * const GSSOCKSSendAuth = @"GSSOCKSSendAuth";
static NSString * const GSSOCKSAckAuth = @"GSSOCKSAckAuth";
static NSString * const GSSOCKSSendConn = @"GSSOCKSSendConn";
static NSString * const GSSOCKSAckConn = @"GSSOCKSAckConn";

@interface	GSSOCKS : GSStreamHandler
{
  NSString		*state;		/* Not retained */
  NSString		*address;
  NSString		*port;
  int			roffset;
  int			woffset;
  int			rwant;
  unsigned char		rbuffer[128];
}
- (void) stream: (NSStream*)stream handleEvent: (NSStreamEvent)event;
@end

@implementation	GSSOCKS
+ (void) tryInput: (GSSocketInputStream*)i output: (GSSocketOutputStream*)o
{
  NSDictionary          *conf;

  conf = [i propertyForKey: NSStreamSOCKSProxyConfigurationKey];
  if (conf == nil)
    {
      conf = [o propertyForKey: NSStreamSOCKSProxyConfigurationKey];
      if (conf != nil)
        {
          [i setProperty: conf forKey: NSStreamSOCKSProxyConfigurationKey];
        }
    }
  else
    {
      [o setProperty: conf forKey: NSStreamSOCKSProxyConfigurationKey];
    }

  if (conf != nil)
    {
      GSSOCKS           *h;
      struct sockaddr   *sa = [i _address];
      NSString          *v;
      BOOL              i6 = NO;

      v = [conf objectForKey: NSStreamSOCKSProxyVersionKey];
      if ([v isEqualToString: NSStreamSOCKSProxyVersion4] == YES)
        {
          v = NSStreamSOCKSProxyVersion4;
        }
      else
        {
          v = NSStreamSOCKSProxyVersion5;
        }

#if     defined(AF_INET6)
      if (sa->sa_family == AF_INET6)
        {
          i6 = YES;
        }
      else
#endif
      if (sa->sa_family != AF_INET)
        {
          GSOnceMLog(@"SOCKS not supported for socket type %d", sa->sa_family);
          return;
        }

      if (v == NSStreamSOCKSProxyVersion5)
        {
          GSOnceMLog(@"SOCKS 5 not supported yet");
          return;
        }
      else if (i6 == YES)
        {
          GSOnceMLog(@"INET6 not supported with SOCKS 4");
          return;
        }

      h = [[GSSOCKS alloc] initWithInput: i output: o];
      [i _setHandler: h];
      [o _setHandler: h];
      RELEASE(h);
    }
}

- (void) bye
{
  if (handshake == YES)
    {
      GSSocketInputStream	*is = RETAIN(istream);
      GSSocketOutputStream	*os = RETAIN(ostream);

      handshake = NO;

      [is _setHandler: nil];
      [os _setHandler: nil];
      [GSTLSHandler tryInput: is output: os];
      if ([is streamStatus] == NSStreamStatusOpen)
        {
	  [is _resetEvents: NSStreamEventOpenCompleted];
          [is _sendEvent: NSStreamEventOpenCompleted];
        }
      else
        {
	  [is _resetEvents: NSStreamEventErrorOccurred];
          [is _sendEvent: NSStreamEventErrorOccurred];
        }
      if ([os streamStatus]  == NSStreamStatusOpen)
        {
	  [os _resetEvents: NSStreamEventOpenCompleted
	    | NSStreamEventHasSpaceAvailable];
          [os _sendEvent: NSStreamEventOpenCompleted];
          [os _sendEvent: NSStreamEventHasSpaceAvailable];
        }
      else
        {
	  [os _resetEvents: NSStreamEventErrorOccurred];
          [os _sendEvent: NSStreamEventErrorOccurred];
        }
      RELEASE(is);
      RELEASE(os);
    }
}

- (void) dealloc
{
  RELEASE(address);
  RELEASE(port);
  [super dealloc];
}

- (void) hello
{
  if (handshake == NO)
    {
      handshake = YES;
      /* Now send self an event to say we can write, to kick off the
       * handshake with the SOCKS server.
       */
      [self stream: ostream handleEvent: NSStreamEventHasSpaceAvailable];
    }
}

- (id) initWithInput: (GSSocketInputStream*)i
              output: (GSSocketOutputStream*)o
{
  if ((self = [super initWithInput: i output: o]) != nil)
    {
      if ([istream isKindOfClass: [GSInetInputStream class]] == NO)
	{
	  NSLog(@"Attempt to use SOCKS with non-INET stream ignored");
	  DESTROY(self);
	}
#if	defined(AF_INET6)
      else if ([istream isKindOfClass: [GSInet6InputStream class]] == YES)
	{
          GSOnceMLog(@"INET6 not supported with SOCKS yet...");
	  DESTROY(self);
	}
#endif	/* AF_INET6 */
      else
	{
	  struct sockaddr_in	*addr;
          NSDictionary          *conf;
          NSString              *host;
          int                   pnum;

          /* Record the host and port that the streams are supposed to be
           * connecting to.
           */
	  addr = (struct sockaddr_in*)(void*)[istream _address];
	  address = [[NSString alloc] initWithUTF8String:
	    (char*)inet_ntoa(addr->sin_addr)];
	  port = [[NSString alloc] initWithFormat: @"%d",
	    (int)GSSwapBigI16ToHost(addr->sin_port)];

          /* Now reconfigure the streams so they will actually connect
           * to the socks proxy server.
           */
          conf = [istream propertyForKey: NSStreamSOCKSProxyConfigurationKey];
          host = [conf objectForKey: NSStreamSOCKSProxyHostKey];
          pnum = [[conf objectForKey: NSStreamSOCKSProxyPortKey] intValue];
          [istream _setSocketAddress: host port: pnum family: AF_INET];
          [ostream _setSocketAddress: host port: pnum family: AF_INET];
	}
    }
  return self;
}

- (NSInteger) read: (uint8_t *)buffer maxLength: (NSUInteger)len
{
  return [istream _read: buffer maxLength: len];
}

- (void) stream: (NSStream*)stream handleEvent: (NSStreamEvent)event
{
  NSString		*error = nil;
  NSDictionary		*conf;
  NSString		*user;
  NSString		*pass;

  if (event == NSStreamEventErrorOccurred
    || [stream streamStatus] == NSStreamStatusError
    || [stream streamStatus] == NSStreamStatusClosed)
    {
      [self bye];
      return;
    }

  conf = [stream propertyForKey: NSStreamSOCKSProxyConfigurationKey];
  user = [conf objectForKey: NSStreamSOCKSProxyUserKey];
  pass = [conf objectForKey: NSStreamSOCKSProxyPasswordKey];
  if ([[conf objectForKey: NSStreamSOCKSProxyVersionKey]
    isEqual: NSStreamSOCKSProxyVersion4] == YES)
    {
    }
  else
    {
      again:

      if (state == GSSOCKSOfferAuth)
	{
	  int		result;
	  int		want;
	  unsigned char	buf[4];

	  /*
	   * Authorisation record is at least three bytes -
	   *   socks version (5)
	   *   authorisation method bytes to follow (1)
	   *   say we do no authorisation (0)
	   *   say we do user/pass authorisation (2)
	   */
	  buf[0] = 5;
	  if (user && pass)
	    {
	      buf[1] = 2;
	      buf[2] = 2;
	      buf[3] = 0;
	      want = 4;
	    }
	  else
	    {
	      buf[1] = 1;
	      buf[2] = 0;
	      want = 3;
	    }

	  result = [ostream _write: buf + woffset maxLength: 4 - woffset];
	  if (result > 0)
	    {
	      woffset += result;
	      if (woffset == want)
		{
		  woffset = 0;
		  state = GSSOCKSRecvAuth;
		  goto again;
		}
	    }
	}
      else if (state == GSSOCKSRecvAuth)
	{
	  int	result;

	  result = [istream _read: rbuffer + roffset maxLength: 2 - roffset];
	  if (result == 0)
	    {
	      error = @"SOCKS end-of-file during negotiation";
	    }
	  else if (result > 0)
	    {
	      roffset += result;
	      if (roffset == 2)
		{
		  roffset = 0;
		  if (rbuffer[0] != 5)
		    {
		      error = @"SOCKS authorisation response had wrong version";
		    }
		  else if (rbuffer[1] == 0)
		    {
		      state = GSSOCKSSendConn;
		      goto again;
		    }
		  else if (rbuffer[1] == 2)
		    {
		      state = GSSOCKSSendAuth;
		      goto again;
		    }
		  else
		    {
		      error = @"SOCKS authorisation response had wrong method";
		    }
		}
	    }
	}
      else if (state == GSSOCKSSendAuth)
	{
	  NSData	*u = [user dataUsingEncoding: NSUTF8StringEncoding];
	  unsigned	ul = [u length];
	  NSData	*p = [pass dataUsingEncoding: NSUTF8StringEncoding];
	  unsigned	pl = [p length];

	  if (ul < 1 || ul > 255)
	    {
	      error = @"NSStreamSOCKSProxyUserKey value too long";
	    }
	  else if (pl < 1 || pl > 255)
	    {
	      error = @"NSStreamSOCKSProxyPasswordKey value too long";
	    }
	  else
	    {
	      int		want = ul + pl + 3;
	      unsigned char	buf[want];
	      int		result;

	      buf[0] = 5;
	      buf[1] = ul;
	      memcpy(buf + 2, [u bytes], ul);
	      buf[ul + 2] = pl;
	      memcpy(buf + ul + 3, [p bytes], pl);
	      result = [ostream _write: buf + woffset
			     maxLength: want - woffset];
	      if (result == 0)
		{
		  error = @"SOCKS end-of-file during negotiation";
		}
	      else if (result > 0)
		{
		  woffset += result;
		  if (woffset == want)
		    {
		      state = GSSOCKSAckAuth;
		      goto again;
		    }
		}
	    }
	}
      else if (state == GSSOCKSAckAuth)
	{
	  int	result;

	  result = [istream _read: rbuffer + roffset maxLength: 2 - roffset];
	  if (result == 0)
	    {
	      error = @"SOCKS end-of-file during negotiation";
	    }
	  else if (result > 0)
	    {
	      roffset += result;
	      if (roffset == 2)
		{
		  roffset = 0;
		  if (rbuffer[0] != 5)
		    {
		      error = @"SOCKS authorisation response had wrong version";
		    }
		  else if (rbuffer[1] == 0)
		    {
		      state = GSSOCKSSendConn;
		      goto again;
		    }
		  else if (rbuffer[1] == 2)
		    {
		      error = @"SOCKS authorisation failed";
		    }
		}
	    }
	}
      else if (state == GSSOCKSSendConn)
	{
	  unsigned char	buf[10];
	  int		want = 10;
	  int		result;
	  const char	*ptr;

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
	  ptr = [address UTF8String];
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
	  result = [port intValue];
	  buf[8] = ((result & 0xff00) >> 8);
	  buf[9] = (result & 0xff);

	  result = [ostream _write: buf + woffset maxLength: want - woffset];
	  if (result == 0)
	    {
	      error = @"SOCKS end-of-file during negotiation";
	    }
	  else if (result > 0)
	    {
	      woffset += result;
	      if (woffset == want)
		{
		  rwant = 5;
		  state = GSSOCKSAckConn;
		  goto again;
		}
	    }
	}
      else if (state == GSSOCKSAckConn)
	{
	  int	result;

	  result = [istream _read: rbuffer + roffset
                        maxLength: rwant - roffset];
	  if (result == 0)
	    {
	      error = @"SOCKS end-of-file during negotiation";
	    }
	  else if (result > 0)
	    {
	      roffset += result;
	      if (roffset == rwant)
		{
		  if (rbuffer[0] != 5)
		    {
		      error = @"connect response from SOCKS had wrong version";
		    }
		  else if (rbuffer[1] != 0)
		    {
		      switch (rbuffer[1])
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
			    error = @"SOCKS server says address not supported";
			    break;
			  default:
			    error = @"connect response from SOCKS was failure";
			    break;
			}
		    }
		  else if (rbuffer[3] == 1)
		    {
		      rwant = 10;		// Fixed size (IPV4) address
		    }
		  else if (rbuffer[3] == 3)
		    {
		      rwant = 7 + rbuffer[4];	// Domain name leading length
		    }
		  else if (rbuffer[3] == 4)
		    {
		      rwant = 22;		// Fixed size (IPV6) address
		    }
		  else
		    {
		      error = @"SOCKS server returned unknown address type";
		    }
		  if (error == nil)
		    {
		      if (roffset < rwant)
			{
			  goto again;	// Need address/port bytes
			}
		      else
			{
			  NSString	*a;

			  if (rbuffer[3] == 1)
			    {
			      a = [NSString stringWithFormat: @"%d.%d.%d.%d",
			        rbuffer[4], rbuffer[5], rbuffer[6], rbuffer[7]];
			    }
			  else if (rbuffer[3] == 3)
			    {
			      rbuffer[rwant] = '\0';
			      a = [NSString stringWithUTF8String:
			        (const char*)rbuffer];
			    }
			  else
			    {
			      unsigned char	buf[40];
			      int		i = 4;
			      int		j = 0;

			      while (i < rwant)
			        {
				  int	val;

				  val = rbuffer[i++];
				  val = val * 256 + rbuffer[i++];
				  if (i > 4)
				    {
				      buf[j++] = ':';
				    }
				  snprintf((char*)&buf[j], 5, "%04x", val);
				  j += 4;
				}
			      a = [NSString stringWithUTF8String:
			        (const char*)buf];
			    }

			  [istream setProperty: a
					forKey: GSStreamRemoteAddressKey];
			  [ostream setProperty: a
					forKey: GSStreamRemoteAddressKey];
			  a = [NSString stringWithFormat: @"%d",
			    rbuffer[rwant-1] * 256 * rbuffer[rwant-2]];
			  [istream setProperty: a
					forKey: GSStreamRemotePortKey];
			  [ostream setProperty: a
					forKey: GSStreamRemotePortKey];
			  /* Return immediately after calling -bye as it
			   * will cause this instance to be deallocated.
			   */
			  [self bye];
			  return;
			}
		    }
		}
	    }
	}
    }

  if ([error length] > 0)
    {
      NSError *theError;

      theError = [NSError errorWithDomain: NSCocoaErrorDomain
	code: 0
	userInfo: [NSDictionary dictionaryWithObject: error
	  forKey: NSLocalizedDescriptionKey]];
      if ([istream streamStatus] != NSStreamStatusError)
	{
	  [istream _recordError: theError];
	}
      if ([ostream streamStatus] != NSStreamStatusError)
	{
	  [ostream _recordError: theError];
	}
      [self bye];
    }
}

- (NSInteger) write: (const uint8_t *)buffer maxLength: (NSUInteger)len
{
  return [ostream _write: buffer maxLength: len];
}

@end


static inline BOOL
socketError(int result)
{
#if	defined(_WIN32)
  return (result == SOCKET_ERROR) ? YES : NO;
#else
  return (result < 0) ? YES : NO;
#endif
}

static inline BOOL
socketWouldBlock()
{
  return GSWOULDBLOCK ? YES : NO;
}


static void
setNonBlocking(SOCKET fd)
{
#if	defined(_WIN32)
  unsigned long dummy = 1;

  if (ioctlsocket(fd, FIONBIO, &dummy) == SOCKET_ERROR)
    {
      NSLog(@"unable to set non-blocking mode - %@", [NSError _last]);
    }
#else
  int flags = fcntl(fd, F_GETFL, 0);

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      NSLog(@"unable to set non-blocking mode - %@",
        [NSError _last]);
    }
#endif
}

@implementation GSSocketStream

- (void) dealloc
{
  if (_sock != INVALID_SOCKET)
    {
      [self close];
    }
  [_sibling _setSibling: nil];
  _sibling = nil;
  [_handler remove: self];
  DESTROY(_handler);
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ sock %d loopID %p",
    [super description], _sock, _loopID];
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      // so that unopened access will fail
      _sibling = nil;
      _closing = NO;
      _passive = NO;
#if	defined(_WIN32)
      _loopID = WSA_INVALID_EVENT;
#else
      _loopID = (void*)(intptr_t)-1;
#endif
      _sock = INVALID_SOCKET;
      _handler = nil;
      _address.s.sa_family = AF_UNSPEC;
    }
  return self;
}

- (struct sockaddr*) _address
{
  return &_address.s;
}

- (id) propertyForKey: (NSString *)key
{
  id	result = [super propertyForKey: key];

  if (result == nil && _address.s.sa_family != AF_UNSPEC)
    {
      SOCKET    	s = [self _sock];
      struct sockaddr	sin;
      socklen_t	        size = sizeof(sin);

      memset(&sin, '\0', size);
      if ([key isEqualToString: GSStreamLocalAddressKey])
	{
	  if (getsockname(s, (struct sockaddr*)&sin, (OPTLEN*)&size) != -1)
	    {
	      result = GSPrivateSockaddrHost(&sin);
	    }
	}
      else if ([key isEqualToString: GSStreamLocalPortKey])
	{
	  if (getsockname(s, (struct sockaddr*)&sin, (OPTLEN*)&size) != -1)
	    {
	      result = [NSString stringWithFormat: @"%d",
		(int)GSPrivateSockaddrPort(&sin)];
	    }
	}
      else if ([key isEqualToString: GSStreamRemoteAddressKey])
	{
	  if (getpeername(s, (struct sockaddr*)&sin, (OPTLEN*)&size) != -1)
	    {
	      result = GSPrivateSockaddrHost(&sin);
	    }
	}
      else if ([key isEqualToString: GSStreamRemotePortKey])
	{
	  if (getpeername(s, (struct sockaddr*)&sin, (OPTLEN*)&size) != -1)
	    {
	      result = [NSString stringWithFormat: @"%d",
		(int)GSPrivateSockaddrPort(&sin)];
	    }
	}
    }
  return result;
}

- (NSInteger) _read: (uint8_t *)buffer maxLength: (NSUInteger)len
{
  [self subclassResponsibility: _cmd];
  return -1;
}

- (void) _sendEvent: (NSStreamEvent)event
{
  /* If the receiver has a TLS handshake in progress,
   * we must send events to the TLS handler rather than
   * the stream delegate.
   */
  if (_handler != nil && [_handler handshake] == YES)
    {
      /* Must retain self here to avoid premature deallocation of input
       * and/or output stream in case of an error during TLS handshake.
       */
      RETAIN(self);
      [super _sendEvent: event delegate: _handler];
      RELEASE(self);
    }
  else
    {
      [super _sendEvent: event];
    }
}

- (BOOL) _setSocketAddress: (NSString*)address
                      port: (NSInteger)port
                    family: (NSInteger)family
{
  uint16_t	p = (uint16_t)port;

  switch (family)
    {
      case AF_INET:
        {
          int           ptonReturn;
          const char    *addr_c;
          struct	sockaddr_in	peer;

          addr_c = [address cStringUsingEncoding: NSUTF8StringEncoding];
          memset(&peer, '\0', sizeof(peer));
          peer.sin_family = AF_INET;
          peer.sin_port = GSSwapHostI16ToBig(p);
          ptonReturn = inet_pton(AF_INET, addr_c, &peer.sin_addr);
          if (ptonReturn <= 0)   // error
            {
              return NO;
            }
          else
            {
              [self _setAddress: (struct sockaddr*)&peer];
              return YES;
            }
        }

#if	defined(AF_INET6)
      case AF_INET6:
        {
          int           ptonReturn;
          const char    *addr_c;
          struct	sockaddr_in6	peer;

          addr_c = [address cStringUsingEncoding: NSUTF8StringEncoding];
          memset(&peer, '\0', sizeof(peer));
          peer.sin6_family = AF_INET6;
          peer.sin6_port = GSSwapHostI16ToBig(p);
          ptonReturn = inet_pton(AF_INET6, addr_c, &peer.sin6_addr);
          if (ptonReturn <= 0)   // error
            {
              return NO;
            }
          else
            {
              [self _setAddress: (struct sockaddr*)&peer];
              return YES;
            }
        }
#endif

#ifndef	_WIN32
      case AF_LOCAL:
	{
	  struct sockaddr_un	peer;
	  const char                *c_addr;

	  c_addr = [address fileSystemRepresentation];
	  memset(&peer, '\0', sizeof(peer));
	  peer.sun_family = AF_LOCAL;
	  if (strlen(c_addr) > sizeof(peer.sun_path)-1) // too long
	    {
	      return NO;
	    }
	  else
	    {
	      strncpy(peer.sun_path, c_addr, sizeof(peer.sun_path)-1);
	      [self _setAddress: (struct sockaddr*)&peer];
	      return YES;
	    }
	}
#endif

      default:
        return NO;
    }
}

- (void) _setAddress: (struct sockaddr*)address
{
  memcpy(&_address.s, address, GSPrivateSockaddrLength(address));
}

- (void) _setLoopID: (void *)ref
{
#if	!defined(_WIN32)
  _sock = (SOCKET)(intptr_t)ref;        // On gnu/linux _sock is _loopID
#endif
  _loopID = ref;
}

- (void) _setClosing: (BOOL)closing
{
  _closing = closing;
}

- (void) _setPassive: (BOOL)passive
{
  _passive = passive;
}

- (void) _setSibling: (GSSocketStream*)sibling
{
  _sibling = sibling;
}

- (void) _setSock: (SOCKET)sock
{
  setNonBlocking(sock);
  _sock = sock;

  /* As well as recording the socket, we set up the stream for monitoring it.
   * On unix style systems we set the socket descriptor as the _loopID to be
   * monitored, and on mswindows systems we create an event object to be
   * monitored (the socket events are assoociated with this object later).
   */
#if	defined(_WIN32)
  _loopID = CreateEvent(NULL, NO, NO, NULL);
#else
  _loopID = (void*)(intptr_t)sock;      // On gnu/linux _sock is _loopID
#endif
}

- (void) _setHandler: (id)h
{
  ASSIGN(_handler, h);
}

- (SOCKET) _sock
{
  return _sock;
}

- (NSInteger) _write: (const uint8_t *)buffer maxLength: (NSUInteger)len
{
  [self subclassResponsibility: _cmd];
  return -1;
}

@end


@implementation GSSocketInputStream

+ (void) initialize
{
  GSMakeWeakPointer(self, "_sibling");
  if (self == [GSSocketInputStream class])
    {
      GSObjCAddClassBehavior(self, [GSSocketStream class]);
    }
}

- (void) open
{
  // could be opened because of sibling
  if ([self _isOpened])
    return;
  if (_sibling && [_sibling streamStatus] == NSStreamStatusError)
    {
      [self _setStatus: NSStreamStatusError];
      return;
    }
  if (_passive || (_sibling && [_sibling _isOpened]))
    goto open_ok;
  // check sibling status, avoid double connect
  if (_sibling && [_sibling streamStatus] == NSStreamStatusOpening)
    {
      [self _setStatus: NSStreamStatusOpening];
      return;
    }
  else
    {
      int result;

      if ([self _sock] == INVALID_SOCKET)
        {
          SOCKET        s;

          if (_handler == nil)
            {
              [GSSOCKS tryInput: self output: _sibling];
            }
          s = socket(_address.s.sa_family, SOCK_STREAM, 0);
          if (BADSOCKET(s))
            {
              [self _recordError];
              return;
            }
          else
            {
              [self _setSock: s];
              [_sibling _setSock: s];
            }
        }

      if (nil == _handler)
        {
          [GSTLSHandler tryInput: self output: _sibling];
        }

      result = connect([self _sock], &_address.s,
        GSPrivateSockaddrLength(&_address.s));
      if (socketError(result))
        {
          if (socketWouldBlock())
            {
              /* Need to set the status first, so that the run loop can tell
               * it needs to add the stream as waiting on writable, as an
               * indication of opened
               */
              [self _setStatus: NSStreamStatusOpening];
            }
          else
            {
              /* Had an immediate connect error.
               */
              [self _recordError];
              [_sibling _recordError];
            }
#if	defined(_WIN32)
          WSAEventSelect(_sock, _loopID, FD_ALL_EVENTS);
#endif
	  if (NSCountMapTable(_loops) > 0)
	    {
	      [self _schedule];
	      return;
	    }
          else if (NSStreamStatusOpening == _currentStatus)
            {
              NSRunLoop *r;
              NSDate    *d;

              /* The stream was not scheduled in any run loop, so we
               * implement a blocking connect by running in the default
               * run loop mode.
               */
              r = [NSRunLoop currentRunLoop];
              d = [NSDate distantFuture];
              [r addStream: self mode: NSDefaultRunLoopMode];
              while ([r runMode: NSDefaultRunLoopMode beforeDate: d] == YES)
                {
                  if (_currentStatus != NSStreamStatusOpening)
                    {
                      break;
                    }
                }
              [r removeStream: self mode: NSDefaultRunLoopMode];
              return;
            }
        }
    }

 open_ok:
#if	defined(_WIN32)
  WSAEventSelect(_sock, _loopID, FD_ALL_EVENTS);
#endif
  [super open];
}

- (void) close
{
  /* If the socket descriptor is still present, we need to close it to
   * avoid a leak no matter what the nominal state of the stream is.
   * The descriptor is created before the stream is formally opened.
   */
  if (INVALID_SOCKET == _sock)
    {
      if (_currentStatus == NSStreamStatusNotOpen)
        {
          NSDebugMLLog(@"NSStream",
            @"Attempt to close unopened stream %@", self);
          return;
        }
      if (_currentStatus == NSStreamStatusClosed)
        {
          NSDebugMLLog(@"NSStream",
            @"Attempt to close already closed stream %@", self);
          return;
        }
    }
  [_handler bye];
#if	defined(_WIN32)
  [super close];
  if (_sibling && [_sibling streamStatus] != NSStreamStatusClosed)
    {
      /*
       * Windows only permits a single event to be associated with a socket
       * at any time, but the runloop system only allows an event handle to
       * be added to the loop once, and we have two streams for each socket.
       * So we use two events, one for each stream, and when one stream is
       * closed, we must call WSAEventSelect to ensure that the event handle
       * of the sibling is used to signal events from now on.
       */
      shutdown(_sock, SHUT_RD);
      [_sibling _unschedule];
      if (WSAEventSelect(_sock, [_sibling _loopID], FD_ALL_EVENTS)
	== SOCKET_ERROR)
	{
          NSDebugMLLog(@"NSStream", @"%@ Error %d transferring to %@",
	    self, WSAGetLastError(), _sibling);
	}
      [_sibling _schedule];
    }
  else
    {
      closesocket(_sock);
    }
  WSACloseEvent(_loopID);
  _loopID = WSA_INVALID_EVENT;
#else
  [super close];
  // read shutdown is ignored, because the other side may shutdown first.
  if (!_sibling || [_sibling streamStatus] == NSStreamStatusClosed)
    close((intptr_t)_loopID);
  else
    shutdown((intptr_t)_loopID, SHUT_RD);
  _loopID = (void*)(intptr_t)-1;
#endif
  _sock = INVALID_SOCKET;
}

- (NSInteger) read: (uint8_t *)buffer maxLength: (NSUInteger)len
{
  if (buffer == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"null pointer for buffer"];
    }
  if (len == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"zero byte read requested"];
    }

  if (_handler == nil)
    return [self _read: buffer maxLength: len];
  else
    return [_handler read: buffer maxLength: len];
}

- (NSInteger) _read: (uint8_t *)buffer maxLength: (NSUInteger)len
{
  int readLen;

  _events &= ~NSStreamEventHasBytesAvailable;

  if ([self streamStatus] == NSStreamStatusClosed)
    {
      return 0;
    }
  if ([self streamStatus] == NSStreamStatusAtEnd)
    {
      readLen = 0;
    }
  else
    {
#if	defined(_WIN32)
      readLen = recv([self _sock], (char*) buffer, (socklen_t) len, 0);
#else
      readLen = read([self _sock], buffer, len);
#endif
    }
  if (socketError(readLen))
    {
      if (_closing == YES)
        {
          /* If a read fails on a closing socket,
           * we have reached the end of all data sent by
           * the remote end before it shut down.
           */
          [self _setClosing: NO];
          [self _setStatus: NSStreamStatusAtEnd];
          [self _sendEvent: NSStreamEventEndEncountered];
          readLen = 0;
        }
      else
        {
          if (socketWouldBlock())
            {
              /* We need an event from the operating system
               * to tell us we can start reading again.
               */
              [self _setStatus: NSStreamStatusReading];
            }
          else
            {
              [self _recordError];
            }
          readLen = -1;
        }
    }
  else if (readLen == 0)
    {
      [self _setStatus: NSStreamStatusAtEnd];
      [self _sendEvent: NSStreamEventEndEncountered];
    }
  else
    {
      [self _setStatus: NSStreamStatusOpen];
    }
  return readLen;
}

- (BOOL) getBuffer: (uint8_t **)buffer length: (NSUInteger *)len
{
  return NO;
}

- (void) _dispatch
{
#if	defined(_WIN32)
  AUTORELEASE(RETAIN(self));
  /*
   * Windows only permits a single event to be associated with a socket
   * at any time, but the runloop system only allows an event handle to
   * be added to the loop once, and we have two streams for each socket.
   * So we use two events, one for each stream, and the _dispatch method
   * must handle things for both streams.
   */
  if ([self streamStatus] == NSStreamStatusClosed)
    {
      /*
       * It is possible the stream is closed yet recieving event because
       * of not closed sibling
       */
      NSAssert([_sibling streamStatus] != NSStreamStatusClosed,
	@"Received event for closed stream");
      [_sibling _dispatch];
    }
  else if ([self streamStatus] == NSStreamStatusError)
    {
      [self _sendEvent: NSStreamEventErrorOccurred];
    }
  else
    {
      WSANETWORKEVENTS events;
      int error = 0;
      int getReturn = -1;

      if (WSAEnumNetworkEvents(_sock, _loopID, &events) == SOCKET_ERROR)
	{
	  error = WSAGetLastError();
          NSDebugMLLog(@"NSStream", @"%@ Error %d", self, error);
	}
#ifndef	NDEBUG
      else
	{
	  NSDebugMLLog(@"NSStream", @"%@ EVENTS 0x%x",
	    self, events.lNetworkEvents);
	}
#endif

      if ([self streamStatus] == NSStreamStatusOpening)
	{
	  [self _unschedule];
	  if (error == 0)
	    {
	      socklen_t len = sizeof(error);

	      getReturn = getsockopt(_sock, SOL_SOCKET, SO_ERROR,
		(char*)&error, (OPTLEN*)&len);
	    }

	  if (getReturn >= 0 && error == 0
	    && (events.lNetworkEvents & FD_CONNECT))
	    { // finish up the opening
	      _passive = YES;
	      [self open];
	      // notify sibling
	      if (_sibling)
		{
		  [_sibling open];
		  [_sibling _sendEvent: NSStreamEventOpenCompleted];
		}
	      [self _sendEvent: NSStreamEventOpenCompleted];
	    }
	}

      if (error != 0)
	{
	  errno = error;
	  [self _recordError];
	  [self _sendEvent: NSStreamEventErrorOccurred];
	  if ([_sibling streamStatus] == NSStreamStatusOpening)
	    {
	      [_sibling _recordError];
	      [_sibling _sendEvent: NSStreamEventErrorOccurred];
	    }
	}
      else
	{
	  if (events.lNetworkEvents & FD_WRITE)
	    {
	      NSAssert([_sibling _isOpened], NSInternalInconsistencyException);
	      /* Clear NSStreamStatusWriting if it was set */
	      [_sibling _setStatus: NSStreamStatusOpen];
	    }

	  /* On winsock a socket is always writable unless it has had
	   * failure/closure or a write blocked and we have not been
	   * signalled again.
	   */
	  while ([_sibling _unhandledData] == NO
	    && [_sibling hasSpaceAvailable])
	    {
	      [_sibling _sendEvent: NSStreamEventHasSpaceAvailable];
	    }

	  if (events.lNetworkEvents & FD_READ)
	    {
	      [self _setStatus: NSStreamStatusOpen];
	      while ([self hasBytesAvailable]
		&& [self _unhandledData] == NO)
		{
	          [self _sendEvent: NSStreamEventHasBytesAvailable];
		}
	    }

	  if (events.lNetworkEvents & FD_CLOSE)
	    {
	      [self _setClosing: YES];
	      [_sibling _setClosing: YES];
	      while ([self hasBytesAvailable]
		&& [self _unhandledData] == NO)
		{
		  [self _sendEvent: NSStreamEventHasBytesAvailable];
		}
	    }
	  if (events.lNetworkEvents == 0)
	    {
	      [self _sendEvent: NSStreamEventHasBytesAvailable];
	    }
	}
    }
#else
  NSStreamEvent myEvent;

  if ([self streamStatus] == NSStreamStatusOpening)
    {
      int error;
      int result;
      socklen_t len = sizeof(error);

      IF_NO_GC([[self retain] autorelease];)
      [self _unschedule];
      result = getsockopt([self _sock], SOL_SOCKET, SO_ERROR,
	&error, (OPTLEN*)&len);

      if (result >= 0 && !error)
        { // finish up the opening
          myEvent = NSStreamEventOpenCompleted;
          _passive = YES;
          [self open];
          // notify sibling
          [_sibling open];
          [_sibling _sendEvent: myEvent];
        }
      else // must be an error
        {
          if (error)
            errno = error;
          [self _recordError];
          myEvent = NSStreamEventErrorOccurred;
          [_sibling _recordError];
          [_sibling _sendEvent: myEvent];
        }
    }
  else if ([self streamStatus] == NSStreamStatusAtEnd)
    {
      myEvent = NSStreamEventEndEncountered;
    }
  else if ([self streamStatus] == NSStreamStatusError)
    {
      myEvent = NSStreamEventErrorOccurred;
    }
  else
    {
      [self _setStatus: NSStreamStatusOpen];
      myEvent = NSStreamEventHasBytesAvailable;
    }
  [self _sendEvent: myEvent];
#endif
}

#if	defined(_WIN32)
- (BOOL) runLoopShouldBlock: (BOOL*)trigger
{
  *trigger = YES;
  return YES;
}
#endif

@end


@implementation GSSocketOutputStream

+ (void) initialize
{
  GSMakeWeakPointer(self, "_sibling");
  if (self == [GSSocketOutputStream class])
    {
      GSObjCAddClassBehavior(self, [GSSocketStream class]);
    }
}

- (NSInteger) _write: (const uint8_t *)buffer maxLength: (NSUInteger)len
{
  int writeLen;

  _events &= ~NSStreamEventHasSpaceAvailable;

  if ([self streamStatus] == NSStreamStatusClosed)
    {
      return 0;
    }
  if ([self streamStatus] == NSStreamStatusAtEnd)
    {
      [self _sendEvent: NSStreamEventEndEncountered];
      return 0;
    }

#if	defined(_WIN32)
  writeLen = send([self _sock], (char*) buffer, (socklen_t) len, 0);
#else
  writeLen = write([self _sock], buffer, (socklen_t) len);
#endif

  if (socketError(writeLen))
    {
      if (_closing == YES)
        {
          /* If a write fails on a closing socket,
           * we know the other end is no longer reading.
           */
          [self _setClosing: NO];
          [self _setStatus: NSStreamStatusAtEnd];
          [self _sendEvent: NSStreamEventEndEncountered];
          writeLen = 0;
        }
      else
        {
          if (socketWouldBlock())
            {
              /* We need an event from the operating system
               * to tell us we can start writing again.
               */
              [self _setStatus: NSStreamStatusWriting];
            }
          else
            {
              [self _recordError];
            }
          writeLen = -1;
        }
    }
  else
    {
      [self _setStatus: NSStreamStatusOpen];
    }
  return writeLen;
}

- (void) open
{
  // could be opened because of sibling
  if ([self _isOpened])
    return;
  if (_sibling && [_sibling streamStatus] == NSStreamStatusError)
    {
      [self _setStatus: NSStreamStatusError];
      return;
    }
  if (_passive || (_sibling && [_sibling _isOpened]))
    goto open_ok;
  // check sibling status, avoid double connect
  if (_sibling && [_sibling streamStatus] == NSStreamStatusOpening)
    {
      [self _setStatus: NSStreamStatusOpening];
      return;
    }
  else
    {
      int result;

      if ([self _sock] == INVALID_SOCKET)
        {
          SOCKET        s;

          if (_handler == nil)
            {
              [GSSOCKS tryInput: _sibling output: self];
            }
          s = socket(_address.s.sa_family, SOCK_STREAM, 0);
          if (BADSOCKET(s))
            {
              [self _recordError];
              return;
            }
          else
            {
              [self _setSock: s];
              [_sibling _setSock: s];
            }
        }

      if (nil == _handler)
        {
          [GSTLSHandler tryInput: _sibling output: self];
        }

      result = connect([self _sock], &_address.s,
        GSPrivateSockaddrLength(&_address.s));
      if (socketError(result))
        {
          if (socketWouldBlock())
            {
              /*
               * Need to set the status first, so that the run loop can tell
               * it needs to add the stream as waiting on writable, as an
               * indication of opened
               */
              [self _setStatus: NSStreamStatusOpening];
            }
          else
            {
              /* Had an immediate connect error.
               */
              [self _recordError];
              [_sibling _recordError];
            }
#if	defined(_WIN32)
          WSAEventSelect(_sock, _loopID, FD_ALL_EVENTS);
#endif
	  if (NSCountMapTable(_loops) > 0)
	    {
	      [self _schedule];
	      return;
	    }
          else if (NSStreamStatusOpening == _currentStatus)
            {
              NSRunLoop *r;
              NSDate    *d;

              /* The stream was not scheduled in any run loop, so we
               * implement a blocking connect by running in the default
               * run loop mode.
               */
              r = [NSRunLoop currentRunLoop];
              d = [NSDate distantFuture];
              [r addStream: self mode: NSDefaultRunLoopMode];
              while ([r runMode: NSDefaultRunLoopMode beforeDate: d] == YES)
                {
                  if (_currentStatus != NSStreamStatusOpening)
                    {
                      break;
                    }
                }
              [r removeStream: self mode: NSDefaultRunLoopMode];
              return;
            }
        }
    }

 open_ok:
#if	defined(_WIN32)
  WSAEventSelect(_sock, _loopID, FD_ALL_EVENTS);
#endif
  [super open];
}


- (void) close
{
  /* If the socket descriptor is still present, we need to close it to
   * avoid a leak no matter what the nominal state of the stream is.
   * The descriptor is created before the stream is formally opened.
   */
  if (INVALID_SOCKET == _sock)
    {
      if (_currentStatus == NSStreamStatusNotOpen)
        {
          NSDebugMLLog(@"NSStream",
            @"Attempt to close unopened stream %@", self);
          return;
        }
      if (_currentStatus == NSStreamStatusClosed)
        {
          NSDebugMLLog(@"NSStream",
            @"Attempt to close already closed stream %@", self);
          return;
        }
    }
  [_handler bye];
#if	defined(_WIN32)
  if (_sibling && [_sibling streamStatus] != NSStreamStatusClosed)
    {
      /*
       * Windows only permits a single event to be associated with a socket
       * at any time, but the runloop system only allows an event handle to
       * be added to the loop once, and we have two streams for each socket.
       * So we use two events, one for each stream, and when one stream is
       * closed, we must call WSAEventSelect to ensure that the event handle
       * of the sibling is used to signal events from now on.
       */
      shutdown(_sock, SHUT_WR);
      _sock = INVALID_SOCKET;
      [_sibling _unschedule];
      if (WSAEventSelect([_sibling _sock], [_sibling _loopID], FD_ALL_EVENTS)
	== SOCKET_ERROR)
	{
          NSDebugMLLog(@"NSStream", @"%@ Error %d transferring to %@",
	    self, WSAGetLastError(), _sibling);
	}
      [_sibling _schedule];
    }
  else
    {
      closesocket(_sock);
    }
  WSACloseEvent(_loopID);
  [super close];
  _loopID = WSA_INVALID_EVENT;
#else
  // read shutdown is ignored, because the other side may shutdown first.
  if (!_sibling || [_sibling streamStatus] == NSStreamStatusClosed)
    close((intptr_t)_loopID);
  else
    shutdown((intptr_t)_loopID, SHUT_WR);
  [super close];
  _loopID = (void*)(intptr_t)-1;
#endif
  _sock = INVALID_SOCKET;
}

- (NSInteger) write: (const uint8_t *)buffer maxLength: (NSUInteger)len
{
  if (len == 0)
    {
      /*
       *  The method allows the 'len' equal to 0. In this case the 'buffer'
       *  is ignored. This can be useful if there is a necessity to postpone
       *  actual writing (for no data are ready for example) without leaving
       *  the stream in the state of unhandled NSStreamEventHasSpaceAvailable
       *  (to keep receiving of that event from a runloop).
       *  The delegate's -[stream:handleEvent:] would keep calling of
       *  -[write: NULL maxLength: 0] until the delegate's state allows it
       *  to write actual bytes.
       *  The downside of that is that it produces a busy wait ... with the
       *  run loop immediately notifying the stream that it has space to
       *  write, so care should be taken to ensure that the delegate has a
       *  near constant supply of data to write, or has some mechanism to
       *  detect that no more data is arriving, and shut down.
       */ 
      _events &= ~NSStreamEventHasSpaceAvailable;
      return 0;
    }

  if (buffer == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"null pointer for buffer"];
    }

  if (_handler == nil)
    return [self _write: buffer maxLength: len];
  else
    return [_handler write: buffer maxLength: len];
}

- (void) _dispatch
{
#if	defined(_WIN32)
  AUTORELEASE(RETAIN(self));
  /*
   * Windows only permits a single event to be associated with a socket
   * at any time, but the runloop system only allows an event handle to
   * be added to the loop once, and we have two streams for each socket.
   * So we use two events, one for each stream, and the _dispatch method
   * must handle things for both streams.
   */
  if ([self streamStatus] == NSStreamStatusClosed)
    {
      /*
       * It is possible the stream is closed yet recieving event because
       * of not closed sibling
       */
      NSAssert([_sibling streamStatus] != NSStreamStatusClosed,
	@"Received event for closed stream");
      [_sibling _dispatch];
    }
  else if ([self streamStatus] == NSStreamStatusError)
    {
      [self _sendEvent: NSStreamEventErrorOccurred];
    }
  else
    {
      WSANETWORKEVENTS events;
      int error = 0;
      int getReturn = -1;

      if (WSAEnumNetworkEvents(_sock, _loopID, &events) == SOCKET_ERROR)
	{
	  error = WSAGetLastError();
          NSDebugMLLog(@"NSStream", @"%@ Error %d", self, error);
	}
#ifndef	NDEBUG
      else
	{
	  NSDebugMLLog(@"NSStream", @"%@ EVENTS 0x%x",
	    self, events.lNetworkEvents);
	}
#endif

      if ([self streamStatus] == NSStreamStatusOpening)
	{
	  [self _unschedule];
	  if (error == 0)
	    {
	      socklen_t len = sizeof(error);

	      getReturn = getsockopt(_sock, SOL_SOCKET, SO_ERROR,
		(char*)&error, (OPTLEN*)&len);
	    }

	  if (getReturn >= 0 && error == 0
	    && (events.lNetworkEvents & FD_CONNECT))
	    { // finish up the opening
	      events.lNetworkEvents ^= FD_CONNECT;
	      _passive = YES;
	      [self open];
	      // notify sibling
	      if (_sibling)
		{
		  [_sibling open];
		  [_sibling _sendEvent: NSStreamEventOpenCompleted];
		}
	      [self _sendEvent: NSStreamEventOpenCompleted];
	    }
	}

      if (error != 0)
	{
	  errno = error;
	  [self _recordError];
	  [self _sendEvent: NSStreamEventErrorOccurred];
	  if ([_sibling streamStatus] == NSStreamStatusOpening)
	    {
	      [_sibling _recordError];
	      [_sibling _sendEvent: NSStreamEventErrorOccurred];
	    }
	}
      else
	{
	  if (events.lNetworkEvents & FD_WRITE)
	    {
	      /* Clear NSStreamStatusWriting if it was set */
	      [self _setStatus: NSStreamStatusOpen];
	    }

	  /* On winsock a socket is always writable unless it has had
	   * failure/closure or a write blocked and we have not been
	   * signalled again.
	   */
	  while ([self _unhandledData] == NO && [self hasSpaceAvailable])
	    {
	      [self _sendEvent: NSStreamEventHasSpaceAvailable];
	    }

	  if (events.lNetworkEvents & FD_READ)
	    {
	      [_sibling _setStatus: NSStreamStatusOpen];
	      while ([_sibling hasBytesAvailable]
		&& [_sibling _unhandledData] == NO)
		{
	          [_sibling _sendEvent: NSStreamEventHasBytesAvailable];
		}
	    }
	  if (events.lNetworkEvents & FD_CLOSE)
	    {
	      [self _setClosing: YES];
	      [_sibling _setClosing: YES];
	      while ([_sibling hasBytesAvailable]
		&& [_sibling _unhandledData] == NO)
		{
		  [_sibling _sendEvent: NSStreamEventHasBytesAvailable];
		}
	    }
	  if (events.lNetworkEvents == 0)
	    {
	      [self _sendEvent: NSStreamEventHasSpaceAvailable];
	    }
	}
    }
#else
  NSStreamEvent myEvent;

  if ([self streamStatus] == NSStreamStatusOpening)
    {
      int error;
      socklen_t len = sizeof(error);
      int result;

      IF_NO_GC([[self retain] autorelease];)
      [self _schedule];
      result = getsockopt((intptr_t)_loopID, SOL_SOCKET, SO_ERROR,
	&error, (OPTLEN*)&len);
      if (result >= 0 && !error)
        { // finish up the opening
          myEvent = NSStreamEventOpenCompleted;
          _passive = YES;
          [self open];
          // notify sibling
          [_sibling open];
          [_sibling _sendEvent: myEvent];
        }
      else // must be an error
        {
          if (error)
            errno = error;
          [self _recordError];
          myEvent = NSStreamEventErrorOccurred;
          [_sibling _recordError];
          [_sibling _sendEvent: myEvent];
        }
    }
  else if ([self streamStatus] == NSStreamStatusAtEnd)
    {
      myEvent = NSStreamEventEndEncountered;
    }
  else if ([self streamStatus] == NSStreamStatusError)
    {
      myEvent = NSStreamEventErrorOccurred;
    }
  else
    {
      [self _setStatus: NSStreamStatusOpen];
      myEvent = NSStreamEventHasSpaceAvailable;
    }
  [self _sendEvent: myEvent];
#endif
}

#if	defined(_WIN32)
- (BOOL) runLoopShouldBlock: (BOOL*)trigger
{
  *trigger = YES;
  if ([self _unhandledData] == YES && [self streamStatus] == NSStreamStatusOpen)
    {
      /* In winsock, a writable status is only signalled if an earlier
       * write failed (because it would block), so we must simulate the
       * writable event by having the run loop trigger without blocking.
       */
      return NO;
    }
  return YES;
}
#endif

@end

@implementation GSSocketServerStream

+ (void) initialize
{
  GSMakeWeakPointer(self, "_sibling");
  if (self == [GSSocketServerStream class])
    {
      GSObjCAddClassBehavior(self, [GSSocketStream class]);
    }
}

- (Class) _inputStreamClass
{
  [self subclassResponsibility: _cmd];
  return Nil;
}

- (Class) _outputStreamClass
{
  [self subclassResponsibility: _cmd];
  return Nil;
}

- (void) open
{
  int bindReturn;
  int listenReturn;
  SOCKET s;

  if (_currentStatus != NSStreamStatusNotOpen)
    {
      NSDebugMLLog(@"NSStream",
        @"Attempt to re-open stream %@", self);
      return;
    }

  s = socket(_address.s.sa_family, SOCK_STREAM, 0);
  if (BADSOCKET(s))
    {
      [self _recordError];
      [self _sendEvent: NSStreamEventErrorOccurred];
      return;
    }
  else
    {
      [(GSSocketStream*)self _setSock: s];
    }

#ifndef	BROKEN_SO_REUSEADDR
  if (_address.s.sa_family == AF_INET
#ifdef  AF_INET6
    || _address.s.sa_family == AF_INET6
#endif
  )
    {
      /*
       * Under decent systems, SO_REUSEADDR means that the port can be reused
       * immediately that this process exits.  Under some it means
       * that multiple processes can serve the same port simultaneously.
       * We don't want that broken behavior!
       */
      int	status = 1;

      if (setsockopt([self _sock], SOL_SOCKET, SO_REUSEADDR,
        (char *)&status, (OPTLEN)sizeof(status)) < 0)
        {
          NSDebugMLLog(@"GSTcpTune", @"setsockopt reuseaddr failed");
        }
    }
#endif

  bindReturn = bind([self _sock],
    &_address.s, GSPrivateSockaddrLength(&_address.s));
  if (socketError(bindReturn))
    {
      [self _recordError];
      [self _sendEvent: NSStreamEventErrorOccurred];
      return;
    }
  listenReturn = listen([self _sock], GSBACKLOG);
  if (socketError(listenReturn))
    {
      [self _recordError];
      [self _sendEvent: NSStreamEventErrorOccurred];
      return;
    }
#if	defined(_WIN32)
  if (_loopID != WSA_INVALID_EVENT)
    {
      WSAEventSelect(_sock, _loopID, FD_ALL_EVENTS);
    }
#endif
  [super open];
}

- (void) close
{
#if	defined(_WIN32)
  if (_loopID != WSA_INVALID_EVENT)
    {
      WSACloseEvent(_loopID);
    }
  if (_sock != INVALID_SOCKET)
    {
      closesocket(_sock);
      [super close];
      _loopID = WSA_INVALID_EVENT;
    }
#else
  if (_loopID != (void*)(intptr_t)-1)
    {
      close((intptr_t)_loopID);
      [super close];
      _loopID = (void*)(intptr_t)-1;
    }
#endif
  _sock = INVALID_SOCKET;
}

- (void) acceptWithInputStream: (NSInputStream **)inputStream
                  outputStream: (NSOutputStream **)outputStream
{
  NSArray *keys;
  NSUInteger count;
  NSMutableDictionary *opts;
  NSString *str;

  GSSocketStream *ins = AUTORELEASE([[self _inputStreamClass] new]);
  GSSocketStream *outs = AUTORELEASE([[self _outputStreamClass] new]);
  /* Align on a 2 byte boundary for a 16bit port number in the sockaddr
   */
  struct {
    uint8_t bytes[BUFSIZ];
  } __attribute__((aligned(2)))buf;
  struct sockaddr       *addr = (struct sockaddr*)&buf;
  socklen_t		len = sizeof(buf);
  int			acceptReturn;

  acceptReturn = accept([self _sock], addr, (OPTLEN*)&len);
  _events &= ~NSStreamEventHasBytesAvailable;
  if (socketError(acceptReturn))
    { // test for real error
      if (!socketWouldBlock())
	{
          [self _recordError];
	}
      ins = nil;
      outs = nil;
    }
  else
    {
      // no need to connect again
      [ins _setPassive: YES];
      [outs _setPassive: YES];

      // copy the addr to outs
      [ins _setAddress: addr];
      [outs _setAddress: addr];
      [ins _setSock: acceptReturn];
      [outs _setSock: acceptReturn];

      /* Set property to indicate that the input stream was accepted by
       * a listening socket (server) rather than produced by an outgoing
       * connection (client).
       */
      [ins setProperty: @"YES" forKey: @"IsServer"];

      /* At this point, we can insert the handler to deal with TLS
       */
      str = [self propertyForKey: NSStreamSocketSecurityLevelKey];
      if (nil != str)
	{
	  opts = [NSMutableDictionary new];
	  [opts setObject: str forKey: NSStreamSocketSecurityLevelKey];
	  // copy the properties in the 'opts'
	  [GSTLSHandler populateProperties: &opts
			 withSecurityLevel: str
			   fromInputStream: self
			    orOutputStream: nil];
	  // and set the input/output streams's properties from the 'opts'
	  keys = [opts allKeys];
	  count = [keys count];
	  while(count-- > 0)
	    {
	      NSString *key = [keys objectAtIndex: count];
	      str = [opts objectForKey: key];
	      [ins setProperty: str forKey: key];
	      [outs setProperty: str forKey: key];
	    }

          /* Set the streams to be 'open' in order to have the TLS
           * handshake done.  On completion the state will be reset.
           */
          [ins _setStatus: NSStreamStatusOpen];
          [outs _setStatus: NSStreamStatusOpen];
	  [GSTLSHandler tryInput: (GSSocketInputStream *)ins
			  output: (GSSocketOutputStream *)outs];
	  DESTROY(opts);
	}
    }
  if (inputStream)
    {
      [ins _setSibling: outs];
      *inputStream = (NSInputStream*)ins;
    }
  if (outputStream)
    {
      [outs _setSibling: ins];
      *outputStream = (NSOutputStream*)outs;
    }
  /* Now the streams are redy to be opened.
   */
}

- (void) _dispatch
{
#if	defined(_WIN32)
  WSANETWORKEVENTS events;

  if (WSAEnumNetworkEvents(_sock, _loopID, &events) == SOCKET_ERROR)
    {
      errno = WSAGetLastError();
      [self _recordError];
      [self _sendEvent: NSStreamEventErrorOccurred];
    }
  else if (events.lNetworkEvents & FD_ACCEPT)
    {
      events.lNetworkEvents ^= FD_ACCEPT;
      [self _setStatus: NSStreamStatusReading];
      [self _sendEvent: NSStreamEventHasBytesAvailable];
    }
#else
  NSStreamEvent myEvent;

  [self _setStatus: NSStreamStatusOpen];
  myEvent = NSStreamEventHasBytesAvailable;
  [self _sendEvent: myEvent];
#endif
}

@end



@implementation GSInetInputStream

- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  if ((self = [super init]) != nil)
    {
      if ([self _setSocketAddress: addr port: port family: AF_INET] == NO)
        {
          DESTROY(self);
        }
    }
  return self;
}

@end

@implementation GSInet6InputStream
#if	defined(AF_INET6)

- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  if ((self = [super init]) != nil)
    {
      if ([self _setSocketAddress: addr port: port family: AF_INET6] == NO)
        {
          DESTROY(self);
        }
    }
  return self;
}

#else
- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  DESTROY(self);
  return nil;
}
#endif
@end

@implementation GSInetOutputStream

- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  if ((self = [super init]) != nil)
    {
      if ([self _setSocketAddress: addr port: port family: AF_INET] == NO)
        {
          DESTROY(self);
        }
    }
  return self;
}

@end

@implementation GSInet6OutputStream
#if	defined(AF_INET6)

- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  if ((self = [super init]) != nil)
    {
      if ([self _setSocketAddress: addr port: port family: AF_INET6] == NO)
        {
          DESTROY(self);
        }
    }
  return self;
}

#else
- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  DESTROY(self);
  return nil;
}
#endif
@end

@implementation GSInetServerStream

- (Class) _inputStreamClass
{
  return [GSInetInputStream class];
}

- (Class) _outputStreamClass
{
  return [GSInetOutputStream class];
}

- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  if ((self = [super init]) != nil)
    {
      if ([addr length] == 0)
        {
          addr = @"0.0.0.0";
        }
      if ([self _setSocketAddress: addr port: port family: AF_INET] == NO)
        {
          DESTROY(self);
        }
    }
  return self;
}

@end

@implementation GSInet6ServerStream
#if	defined(AF_INET6)
- (Class) _inputStreamClass
{
  return [GSInet6InputStream class];
}

- (Class) _outputStreamClass
{
  return [GSInet6OutputStream class];
}

- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  if ([super init] != nil)
    {
      if ([addr length] == 0)
        {
          addr = @"0:0:0:0:0:0:0:0";   /* Bind on all addresses */
        }
      if ([self _setSocketAddress: addr port: port family: AF_INET6] == NO)
        {
          DESTROY(self);
        }
    }
  return self;
}
#else
- (id) initToAddr: (NSString*)addr port: (NSInteger)port
{
  DESTROY(self);
  return nil;
}
#endif
@end

