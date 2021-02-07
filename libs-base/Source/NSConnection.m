/** Implementation of connection object for remote object messaging
   Copyright (C) 1994-2017 Free Software Foundation, Inc.

   Created by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: July 1994
   Minor rewrite for OPENSTEP by: Richard Frith-Macdonald <rfm@gnu.org>
   Date: August 1997
   Major rewrite for MACOSX by: Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2000

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

   <title>NSConnection class reference</title>
   $Date$ $Revision$
   */

#import "common.h"

#if !defined (__GNU_LIBOBJC__)
#  include <objc/encoding.h>
#endif

#define	GS_NSConnection_IVARS \
  BOOL			_isValid; \
  BOOL			_independentQueueing; \
  BOOL			_authenticateIn; \
  BOOL			_authenticateOut; \
  BOOL			_multipleThreads; \
  BOOL			_shuttingDown; \
  BOOL			_useKeepalive; \
  BOOL			_keepaliveWait; \
  NSPort		*_receivePort; \
  NSPort		*_sendPort; \
  unsigned		_requestDepth; \
  unsigned		_messageCount; \
  unsigned		_reqOutCount; \
  unsigned		_reqInCount; \
  unsigned		_repOutCount; \
  unsigned		_repInCount; \
  GSIMapTable		_localObjects; \
  GSIMapTable		_localTargets; \
  GSIMapTable		_remoteProxies; \
  GSIMapTable		_replyMap; \
  NSTimeInterval	_replyTimeout; \
  NSTimeInterval	_requestTimeout; \
  NSMutableArray	*_requestModes; \
  NSMutableArray	*_runLoops; \
  NSMutableArray	*_requestQueue; \
  id			_delegate; \
  NSRecursiveLock	*_refGate; \
  NSMutableArray	*_cachedDecoders; \
  NSMutableArray	*_cachedEncoders; \
  NSString		*_remoteName; \
  NSString		*_registeredName; \
  NSPortNameServer	*_nameServer; \
  int			_lastKeepalive

#define	EXPOSE_NSDistantObject_IVARS	1

#ifdef HAVE_MALLOC_H
#if !defined(__OpenBSD__)
#include <malloc.h>
#endif
#endif
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#import "Foundation/NSEnumerator.h"
#import "GNUstepBase/GSLock.h"

/* Skip past an argument and also any offset information before the next.
 */
static inline const char *
skip_argspec(const char *ptr)
{
  if (ptr != NULL)
    {
      ptr = NSGetSizeAndAlignment(ptr, NULL, NULL);
      if (*ptr == '+') ptr++;
      while (isdigit(*ptr)) ptr++;
    }
  return ptr;
}

/*
 *	Setup for inline operation of pointer map tables.
 */
#define	GSI_MAP_KTYPES	GSUNION_PTR | GSUNION_OBJ | GSUNION_NSINT
#define	GSI_MAP_VTYPES	GSUNION_PTR | GSUNION_OBJ
#define	GSI_MAP_RETAIN_KEY(M, X)
#define	GSI_MAP_RELEASE_KEY(M, X)
#define	GSI_MAP_RETAIN_VAL(M, X)
#define	GSI_MAP_RELEASE_VAL(M, X)
#define	GSI_MAP_HASH(M, X)	((X).nsu ^ ((X).nsu >> 3))
#define	GSI_MAP_EQUAL(M, X,Y)	((X).ptr == (Y).ptr)
#define	GSI_MAP_NOCLEAN	1


#include "GNUstepBase/GSIMap.h"

#define	_IN_CONNECTION_M
#import "Foundation/NSConnection.h"
#undef	_IN_CONNECTION_M

#import "Foundation/NSPortCoder.h"
#import "GNUstepBase/DistributedObjects.h"

#import "Foundation/NSHashTable.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSData.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSException.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSThread.h"
#import "Foundation/NSPort.h"
#import "Foundation/NSPortMessage.h"
#import "Foundation/NSPortNameServer.h"
#import "Foundation/NSNotification.h"
#import "GSInvocation.h"
#import "GSPortPrivate.h"
#import "GSPrivate.h"


static inline NSRunLoop *
GSRunLoopForThread(NSThread *aThread)
{
  GSRunLoopThreadInfo   *info = GSRunLoopInfoForThread(aThread);

  if (info == nil || info->loop == nil)
    {
      if (aThread == nil || aThread == GSCurrentThread())
        {
          return [NSRunLoop currentRunLoop];
        }
      return nil;
    }
  return info->loop;
}


@interface	NSPortCoder (Private)
- (NSMutableArray*) _components;
@end
@interface	NSPortMessage (Private)
- (NSMutableArray*) _components;
@end

@interface NSConnection (GNUstepExtensions)
- (void) finalize;
- (void) forwardInvocation: (NSInvocation *)inv
		  forProxy: (NSDistantObject*)object;
- (const char *) typeForSelector: (SEL)sel remoteTarget: (unsigned)target;
@end

#define GS_F_LOCK(X) \
{NSDebugFLLog(@"GSConnection",@"Lock %@",X);[X lock];}
#define GS_F_UNLOCK(X) \
{NSDebugFLLog(@"GSConnection",@"Unlock %@",X);[X unlock];}
#define GS_M_LOCK(X) \
{NSDebugMLLog(@"GSConnection",@"Lock %@",X);[X lock];}
#define GSM_UNLOCK(X) \
{NSDebugMLLog(@"GSConnection",@"Unlock %@",X);[X unlock];}

NSString * const NSDestinationInvalidException =
  @"NSDestinationInvalidException";
NSString * const NSFailedAuthenticationException =
  @"NSFailedAuthenticationExceptions";
NSString * const NSObjectInaccessibleException =
  @"NSObjectInaccessibleException";
NSString * const NSObjectNotAvailableException =
  @"NSObjectNotAvailableException";

/*
 * Cache various class pointers.
 */
static id	dummyObject;
static Class	connectionClass;
static Class	dateClass;
static Class	distantObjectClass;
static Class	sendCoderClass;
static Class	recvCoderClass;
static Class	runLoopClass;

static NSString*
stringFromMsgType(int type)
{
  switch (type)
    {
      case METHOD_REQUEST:
	return @"method request";
      case METHOD_REPLY:
	return @"method reply";
      case ROOTPROXY_REQUEST:
	return @"root proxy request";
      case ROOTPROXY_REPLY:
	return @"root proxy reply";
      case CONNECTION_SHUTDOWN:
	return @"connection shutdown";
      case METHODTYPE_REQUEST:
	return @"methodtype request";
      case METHODTYPE_REPLY:
	return @"methodtype reply";
      case PROXY_RELEASE:
	return @"proxy release";
      case PROXY_RETAIN:
	return @"proxy retain";
      case RETAIN_REPLY:
	return @"retain replay";
      default:
	return @"unknown operation type!";
    }
}



/*
 * CachedLocalObject is a trivial class to keep track of local
 * proxies which have been removed from their connections and
 * need to persist a while in case another process needs them.
 */
@interface	CachedLocalObject : NSObject
{
  NSDistantObject	*obj;
  int			time;
}
- (BOOL) countdown;
- (NSDistantObject*) obj;
+ (id) newWithObject: (NSDistantObject*)o time: (int)t;
@end

@implementation	CachedLocalObject

+ (id) newWithObject: (NSDistantObject*)o time: (int)t
{
  CachedLocalObject	*item;

  item = (CachedLocalObject*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  item->obj = RETAIN(o);
  item->time = t;
  return item;
}

- (void) dealloc
{
  RELEASE(obj);
  [super dealloc];
}

- (BOOL) countdown
{
  if (time-- > 0)
    return YES;
  return NO;
}

- (NSDistantObject*) obj
{
  return obj;
}

@end



/** <ignore> */

#define	GSInternal	NSConnectionInternal
#include	"GSInternal.h"
GS_PRIVATE_INTERNAL(NSConnection)

#define	IisValid		(internal->_isValid)
#define	IindependentQueueing	(internal->_independentQueueing)
#define	IauthenticateIn		(internal->_authenticateIn)
#define	IauthenticateOut	(internal->_authenticateOut)
#define	ImultipleThreads	(internal->_multipleThreads)
#define	IshuttingDown		(internal->_shuttingDown)
#define	IuseKeepalive		(internal->_useKeepalive)
#define	IkeepaliveWait		(internal->_keepaliveWait)
#define	IreceivePort		(internal->_receivePort)
#define	IsendPort		(internal->_sendPort)
#define	IrequestDepth		(internal->_requestDepth)
#define	ImessageCount		(internal->_messageCount)
#define	IreqOutCount		(internal->_reqOutCount)
#define	IreqInCount		(internal->_reqInCount)
#define	IrepOutCount		(internal->_repOutCount)
#define	IrepInCount		(internal->_repInCount)
#define	IlocalObjects		(internal->_localObjects)
#define	IlocalTargets		(internal->_localTargets)
#define	IremoteProxies		(internal->_remoteProxies)
#define	IreplyMap		(internal->_replyMap)
#define	IreplyTimeout		(internal->_replyTimeout)
#define	IrequestTimeout		(internal->_requestTimeout)
#define	IrequestModes		(internal->_requestModes)
#define	IrunLoops		(internal->_runLoops)
#define	IrequestQueue		(internal->_requestQueue)
#define	Idelegate		(internal->_delegate)
#define	IrefGate		(internal->_refGate)
#define	IcachedDecoders		(internal->_cachedDecoders)
#define	IcachedEncoders		(internal->_cachedEncoders)
#define	IremoteName		(internal->_remoteName)
#define	IregisteredName		(internal->_registeredName)
#define	InameServer		(internal->_nameServer)
#define	IlastKeepalive		(internal->_lastKeepalive)

/** </ignore> */

@interface NSConnection(Private)

- (void) handlePortMessage: (NSPortMessage*)msg;
- (void) _runInNewThread;
+ (int) setDebug: (int)val;
- (void) _enableKeepalive;

- (void) addLocalObject: (NSDistantObject*)anObj;
- (void) removeLocalObject: (NSDistantObject*)anObj;

- (void) _doneInReply: (NSPortCoder*)c;
- (void) _doneInRmc: (NSPortCoder*) NS_CONSUMED c;
- (void) _failInRmc: (NSPortCoder*)c;
- (void) _failOutRmc: (NSPortCoder*)c;
- (NSPortCoder*) _getReplyRmc: (int)sn for: (const char*)request;
- (NSPortCoder*) _newInRmc: (NSMutableArray*)components;
- (NSPortCoder*) _newOutRmc: (int)sequence generate: (int*)sno reply: (BOOL)f;
- (void) _portIsInvalid: (NSNotification*)notification;
- (void) _sendOutRmc: (NSPortCoder*) NS_CONSUMED c
                type: (int)msgid
            sequence: (int)sno;

- (void) _service_forwardForProxy: (NSPortCoder*)rmc;
- (void) _service_release: (NSPortCoder*)rmc;
- (void) _service_retain: (NSPortCoder*)rmc;
- (void) _service_rootObject: (NSPortCoder*)rmc;
- (void) _service_shutdown: (NSPortCoder*)rmc;
- (void) _service_typeForSelector: (NSPortCoder*)rmc;
- (void) _shutdown;
+ (void) _threadWillExit: (NSNotification*)notification;
@end



/* class defaults */
static NSTimer		*timer = nil;

static BOOL cacheCoders = NO;
static int debug_connection = 0;

static NSHashTable	*connection_table;
static NSRecursiveLock		*connection_table_gate = nil;

/*
 * Locate an existing connection with the specified send and receive ports.
 * nil ports act as wildcards and return the first match.
 */
static NSConnection*
existingConnection(NSPort *receivePort, NSPort *sendPort)
{
  NSHashEnumerator	enumerator;
  NSConnection		*c;

  GS_F_LOCK(connection_table_gate);
  enumerator = NSEnumerateHashTable(connection_table);
  while ((c = (NSConnection*)NSNextHashEnumeratorItem(&enumerator)) != nil)
    {
      if ((sendPort == nil || [sendPort isEqual: [c sendPort]])
        && (receivePort == nil || [receivePort isEqual: [c receivePort]]))
	{
	  /*
	   * We don't want this connection to be destroyed by another thread
	   * between now and when it's returned from this function and used!
	   */
	  IF_NO_GC([[c retain] autorelease];)
	  break;
	}
    }
  NSEndHashTableEnumeration(&enumerator);
  GS_F_UNLOCK(connection_table_gate);
  return c;
}

static NSMapTable *root_object_map;
static NSLock *root_object_map_gate = nil;

static id
rootObjectForInPort(NSPort *aPort)
{
  id	rootObject;

  GS_F_LOCK(root_object_map_gate);
  rootObject = (id)NSMapGet(root_object_map, (void*)(uintptr_t)aPort);
  GS_F_UNLOCK(root_object_map_gate);
  return rootObject;
}

/* Pass nil to remove any reference keyed by aPort. */
static void
setRootObjectForInPort(id anObj, NSPort *aPort)
{
  id	oldRootObject;

  GS_F_LOCK(root_object_map_gate);
  oldRootObject = (id)NSMapGet(root_object_map, (void*)(uintptr_t)aPort);
  if (oldRootObject != anObj)
    {
      if (anObj != nil)
	{
	  NSMapInsert(root_object_map, (void*)(uintptr_t)aPort,
	    (void*)(uintptr_t)anObj);
	}
      else /* anObj == nil && oldRootObject != nil */
	{
	  NSMapRemove(root_object_map, (void*)(uintptr_t)aPort);
	}
    }
  GS_F_UNLOCK(root_object_map_gate);
}

static NSMapTable *targetToCached = NULL;
static NSLock	*cached_proxies_gate = nil;




/**
 * NSConnection objects are used to manage communications between
 * objects in different processes, in different machines, or in
 * different threads.
 */
@implementation NSConnection

/**
 * Returns an array containing all the NSConnection objects known to
 * the system. These connections will be valid at the time that the
 * array was created, but may be invalidated by other threads
 * before you get to examine the array.
 */
+ (NSArray*) allConnections
{
  NSArray	*a;

  GS_M_LOCK(connection_table_gate);
  a = NSAllHashTableObjects(connection_table);
  GSM_UNLOCK(connection_table_gate);
  return a;
}

/**
 * Returns a connection initialised using -initWithReceivePort:sendPort:<br />
 * Both ports must be of the same type.
 */
+ (NSConnection*) connectionWithReceivePort: (NSPort*)r
				   sendPort: (NSPort*)s
{
  NSConnection	*c = existingConnection(r, s);

  if (c == nil)
    {
      c = [self allocWithZone: NSDefaultMallocZone()];
      c = [c initWithReceivePort: r sendPort: s];
      IF_NO_GC([c autorelease];)
    }
  return c;
}

/**
 * <p>Returns an NSConnection object whose send port is that of the
 * NSConnection registered under the name n on the host h
 * </p>
 * <p>This method calls +connectionWithRegisteredName:host:usingNameServer:
 * using the default system name server.
 * </p>
 * <p>Use [NSSocketPortNameServer] for connections to remote hosts.
 * </p>
 */
+ (NSConnection*) connectionWithRegisteredName: (NSString*)n
					  host: (NSString*)h
{
  NSPortNameServer	*s;

  s = [NSPortNameServer systemDefaultPortNameServer];
  return [self connectionWithRegisteredName: n
				       host: h
			    usingNameServer: s];
}

/**
 * <p>
 *   Returns an NSConnection object whose send port is that of the
 *   NSConnection registered under <em>name</em> on <em>host</em>.
 * </p>
 * <p>
 *   The nameserver <em>server</em> is used to look up the send
 *   port to be used for the connection.<br />
 *   Use [NSSocketPortNameServer+sharedInstance]
 *   for connections to remote hosts.
 * </p>
 * <p>
 *   If <em>host</em> is <code>nil</code> or an empty string,
 *   the host is taken to be the local machine.<br />
 *   If it is an asterisk ('*') then the nameserver checks all
 *   hosts on the local subnet (unless the nameserver is one
 *   that only manages local ports).<br />
 *   In the GNUstep implementation, the local host is searched before
 *   any other hosts.<br />
 *   NB. if the nameserver does not support connections to remote hosts
 *   (the default situation) the host argeument should be omitted.
 * </p>
 * <p>
 *   If no NSConnection can be found for <em>name</em> and
 *   <em>host</em>host, the method returns <code>nil</code>.
 * </p>
 * <p>
 *   The returned object has the default NSConnection of the
 *   current thread as its parent (it has the same receive port
 *   as the default connection).
 * </p>
 */
+ (NSConnection*) connectionWithRegisteredName: (NSString*)n
					  host: (NSString*)h
			       usingNameServer: (NSPortNameServer*)s
{
  NSConnection		*con = nil;

  if (s != nil)
    {
      NSPort	*sendPort = [s portForName: n onHost: h];

      if (sendPort != nil)
	{
	  NSPort	*recvPort;

	  recvPort = [[self defaultConnection] receivePort];
	  if (recvPort == sendPort)
	    {
	      /*
	       * If the receive and send port are the same, the server
	       * must be in this process - so we need to create a new
	       * connection to talk to it.
	       */
	      recvPort = [NSPort port];
	    }
	  else if (![recvPort isMemberOfClass: [sendPort class]])
	    {
	      /*
	      We can only use the port of the default connection for
	      connections using the same port class. For other port classes,
	      we must use a receiving port of the same class as the sending
	      port, so we allocate one here.
	      */
	      recvPort = [[sendPort class] port];
	    }

	  con = existingConnection(recvPort, sendPort);
	  if (con == nil)
	    {
	      con = [self connectionWithReceivePort: recvPort
					   sendPort: sendPort];
	    }
	  ASSIGNCOPY(GSIVar(con, _remoteName), n);
	}
    }
  return con;
}

/**
 * Return the current conversation ... not implemented in GNUstep
 */
+ (id) currentConversation
{
  return nil;
}

/**
 * Returns the default connection for a thread.<br />
 * Creates a new instance if necessary.<br />
 * The default connection has a single NSPort object used for
 * both sending and receiving - this it can't be used to
 * connect to a remote process, but can be used to vend objects.<br />
 * Possible problem - if the connection is invalidated, it won't be
 * cleaned up until this thread calls this method again.  The connection
 * and it's ports could hang around for a very long time.
 */
+ (NSConnection*) defaultConnection
{
  static NSString	*tkey = @"NSConnectionThreadKey";
  NSConnection		*c;
  NSMutableDictionary	*d;

  d = GSCurrentThreadDictionary();
  c = (NSConnection*)[d objectForKey: tkey];
  if (c != nil && [c isValid] == NO)
    {
      /*
       * If the default connection for this thread has been invalidated -
       * release it and create a new one.
       */
      [d removeObjectForKey: tkey];
      c = nil;
    }
  if (c == nil)
    {
      NSPort	*port;

      c = [self alloc];
      port = [NSPort port];
      c = [c initWithReceivePort: port sendPort: nil];
      if (c != nil)
	{
	  [d setObject: c forKey: tkey];
	  RELEASE(c);
	}
    }
  return c;
}

+ (void) initialize
{
  if (connectionClass == nil)
    {
      NSNotificationCenter	*nc;

      GSMakeWeakPointer(self, "delegate");
      connectionClass = self;
      dateClass = [NSDate class];
      distantObjectClass = [NSDistantObject class];
      sendCoderClass = [NSPortCoder class];
      recvCoderClass = [NSPortCoder class];
      runLoopClass = [NSRunLoop class];

      dummyObject = [NSObject new];
      [[NSObject leakAt: &dummyObject] release];

      connection_table =
	NSCreateHashTable(NSNonRetainedObjectHashCallBacks, 0);
      [[NSObject leakAt: &connection_table] release];

      targetToCached =
	NSCreateMapTable(NSIntegerMapKeyCallBacks,
	  NSObjectMapValueCallBacks, 0);
      [[NSObject leakAt: &targetToCached] release];

      root_object_map =
	NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
	  NSObjectMapValueCallBacks, 0);
      [[NSObject leakAt: &root_object_map] release];

      if (connection_table_gate == nil)
	{
	  connection_table_gate = [NSRecursiveLock new];
          [[NSObject leakAt: &connection_table_gate] release];
	}
      if (cached_proxies_gate == nil)
	{
	  cached_proxies_gate = [NSLock new];
          [[NSObject leakAt: &cached_proxies_gate] release];
	}
      if (root_object_map_gate == nil)
	{
	  root_object_map_gate = [NSLock new];
          [[NSObject leakAt: &root_object_map_gate] release];
	}

      /*
       * When any thread exits, we must check to see if we are using its
       * runloop, and remove ourselves from it if necessary.
       */
      nc = [NSNotificationCenter defaultCenter];
      [nc addObserver: self
	     selector: @selector(_threadWillExit:)
		 name: NSThreadWillExitNotification
	       object: nil];
    }
}

/**
 * Undocumented feature for compatibility with OPENSTEP/MacOS-X
 * +new returns the default connection.
 */
+ (id) new
{
  return RETAIN([self defaultConnection]);
}

/**
 * This method calls
 * +rootProxyForConnectionWithRegisteredName:host:usingNameServer:
 * to return a proxy for a root object on the remote connection with
 * the send port registered under name n on host h.
 */
+ (NSDistantObject*) rootProxyForConnectionWithRegisteredName: (NSString*)n
						         host: (NSString*)h
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSConnection		*connection;
  NSDistantObject	*proxy = nil;

  connection = [self connectionWithRegisteredName: n host: h];
  if (connection != nil)
    {
      proxy = [[connection rootProxy] retain];
    }
  [arp drain];
  return [proxy autorelease];
}

/**
 * This method calls
 * +connectionWithRegisteredName:host:usingNameServer:
 * to get a connection, then sends it a -rootProxy message to get
 * a proxy for the root object being vended by the remote connection.
 * Returns the proxy or nil if it couldn't find a connection or if
 * the root object for the connection has not been set.<br />
 * Use [NSSocketPortNameServer+sharedInstance]
 * for connections to remote hosts.
 */
+ (NSDistantObject*) rootProxyForConnectionWithRegisteredName: (NSString*)n
  host: (NSString*)h usingNameServer: (NSPortNameServer*)s
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSConnection		*connection;
  NSDistantObject	*proxy = nil;

  connection = [self connectionWithRegisteredName: n
					     host: h
				  usingNameServer: s];
  if (connection != nil)
    {
      proxy = RETAIN([connection rootProxy]);
    }
  [arp drain];
  return AUTORELEASE(proxy);
}

+ (id) serviceConnectionWithName: (NSString *)name
                      rootObject: (id)root
{
  return [self serviceConnectionWithName: name
    rootObject: root
    usingNameServer: [NSPortNameServer systemDefaultPortNameServer]];
}

+ (id) serviceConnectionWithName: (NSString *)name
                      rootObject: (id)root
                 usingNameServer: (NSPortNameServer *)server
{
  NSConnection  *c;
  NSPort        *p;

  if ([server isKindOfClass: [NSMessagePortNameServer class]] == YES)
    {
      p = [NSMessagePort port];
    }
  else if ([server isKindOfClass: [NSSocketPortNameServer class]] == YES)
    {
      p = [NSSocketPort port];
    }
  else
    {
      p = nil;
    }

  c = [[NSConnection alloc] initWithReceivePort: p sendPort: nil];
  [c setRootObject: root];
  if ([c registerName: name withNameServer: server] == NO)
    {
      DESTROY(c);
    }
  return AUTORELEASE(c);
}

+ (void) _timeout: (NSTimer*)t
{
  NSArray	*cached_locals;
  int	i;

  GS_M_LOCK(cached_proxies_gate);
  cached_locals = NSAllMapTableValues(targetToCached);
  for (i = [cached_locals count]; i > 0; i--)
    {
      CachedLocalObject *item = [cached_locals objectAtIndex: i-1];

      if ([item countdown] == NO)
	{
	  NSDistantObject	*obj = [item obj];

	  NSMapRemove(targetToCached,
	    (void*)(uintptr_t)obj->_handle);
	}
    }
  if ([cached_locals count] == 0)
    {
      [t invalidate];
      timer = nil;
    }
  GSM_UNLOCK(cached_proxies_gate);
}

/**
 * Adds mode to the run loop modes that the NSConnection
 * will listen to for incoming messages.
 */
- (void) addRequestMode: (NSString*)mode
{
  GS_M_LOCK(IrefGate);
  if ([self isValid] == YES)
    {
      if ([IrequestModes containsObject: mode] == NO)
	{
	  NSUInteger	c = [IrunLoops count];

	  while (c-- > 0)
	    {
	      NSRunLoop	*loop = [IrunLoops objectAtIndex: c];

	      [IreceivePort addConnection: self toRunLoop: loop forMode: mode];
	    }
	  [IrequestModes addObject: mode];
	}
    }
  GSM_UNLOCK(IrefGate);
}

/**
 * Adds loop to the set of run loops that the NSConnection
 * will listen to for incoming messages.
 */
- (void) addRunLoop: (NSRunLoop*)loop
{
  GS_M_LOCK(IrefGate);
  if ([self isValid] == YES)
    {
      if ([IrunLoops indexOfObjectIdenticalTo: loop] == NSNotFound)
	{
	  NSUInteger		c = [IrequestModes count];

	  while (c-- > 0)
	    {
	      NSString	*mode = [IrequestModes objectAtIndex: c];

	      [IreceivePort addConnection: self toRunLoop: loop forMode: mode];
	    }
	  [IrunLoops addObject: loop];
	}
    }
  GSM_UNLOCK(IrefGate);
}

- (void) dealloc
{
  if (debug_connection)
    NSLog(@"deallocating %@", self);
  [self finalize];
  if (internal != nil)
    {
      GS_DESTROY_INTERNAL(NSConnection);
    }
  [super dealloc];
}

/**
 * Returns the delegate of the NSConnection.
 */
- (id) delegate
{
  return Idelegate;
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ local: '%@',%@ remote '%@',%@",
    [super description],
    IregisteredName ? (id)IregisteredName : (id)@"", [self receivePort],
    IremoteName ? (id)IremoteName : (id)@"", [self sendPort]];
}

/**
 * Sets the NSConnection configuration so that multiple threads may
 * use the connection to send requests to the remote connection.<br />
 * This option is inherited by child connections.<br />
 * NB. A connection with multiple threads enabled will run slower than
 * a normal connection.
 */
- (void) enableMultipleThreads
{
  ImultipleThreads = YES;
}

/**
 * Returns YES if the NSConnection is configured to
 * handle remote messages atomically, NO otherwise.<br />
 * This option is inherited by child connections.
 */
- (BOOL) independentConversationQueueing
{
  return IindependentQueueing;
}

/**
 * Return a connection able to act as a server receive incoming requests.
 */
- (id) init
{
  NSPort	*port = [NSPort port];

  self = [self initWithReceivePort: port sendPort: nil];
  return self;
}

/** <init />
 * Initialises an NSConnection with the receive port r and the
 * send port s.<br />
 * Behavior varies with the port values as follows -
 * <deflist>
 *   <term>r is <code>nil</code></term>
 *   <desc>
 *     The NSConnection is released and the method returns
 *     <code>nil</code>.
 *   </desc>
 *   <term>s is <code>nil</code></term>
 *   <desc>
 *     The NSConnection uses r as the send port as
 *     well as the receive port.
 *   </desc>
 *   <term>s is the same as r</term>
 *   <desc>
 *     The NSConnection is usable only for vending objects.
 *   </desc>
 *   <term>A connection with the same ports exists</term>
 *   <desc>
 *     The new connection is released and the old connection
 *     is retained and returned.
 *   </desc>
 *   <term>A connection with the same ports (swapped) exists</term>
 *   <desc>
 *     The new connection is initialised as normal, and will
 *     communicate with the old connection.
 *   </desc>
 * </deflist>
 * <p>
 *   If a connection exists whose send and receive ports are
 *   both the same as the new connections receive port, that
 *   existing connection is deemed to be the parent of the
 *   new connection.  The new connection inherits configuration
 *   information from the parent, and the delegate of the
 *   parent has a chance to adjust the configuration of the
 *   new connection or veto its creation.
 *   <br/>
 *   NSConnectionDidInitializeNotification is posted once a new
 *   connection is initialised.
 * </p>
 */
- (id) initWithReceivePort: (NSPort*)r
		  sendPort: (NSPort*)s
{
  NSNotificationCenter	*nCenter;
  NSConnection		*parent;
  NSConnection		*conn;
  NSRunLoop		*loop;
  id			del;
  NSZone		*z;

  z = NSDefaultMallocZone();
  /*
   * If the receive port is nil, deallocate connection and return nil.
   */
  if (r == nil)
    {
      if (debug_connection > 2)
	{
	  NSLog(@"Asked to create connection with nil receive port");
	}
      DESTROY(self);
      return self;
    }

  /*
   * If the send port is nil, set it to the same as the receive port
   * This connection will then only be useful to act as a server.
   */
  if (s == nil)
    {
      s = r;
    }

  conn = existingConnection(r, s);

  /*
   * If the send and receive ports match an existing connection
   * deallocate the new one and retain and return the old one.
   */
  if (conn != nil)
    {
      DESTROY(self);
      self = RETAIN(conn);
      if (debug_connection > 2)
	{
	  NSLog(@"Found existing connection (%@) for \n\t%@\n\t%@",
	    conn, r, s);
	}
      return self;
    }

  /* Create our private data structure.
   */
  GS_CREATE_INTERNAL(NSConnection);

  /*
   * The parent connection is the one whose send and receive ports are
   * both the same as our receive port.
   */
  parent = existingConnection(r, r);

  if (debug_connection)
    {
      NSLog(@"Initialising new connection with parent %@, %@\n  "
	@"Send: %@\n  Recv: %@", parent, self, s, r);
    }

  GS_M_LOCK(connection_table_gate);

  IisValid = YES;
  IreceivePort = RETAIN(r);
  IsendPort = RETAIN(s);
  ImessageCount = 0;
  IrepOutCount = 0;
  IreqOutCount = 0;
  IrepInCount = 0;
  IreqInCount = 0;

  /*
   * These arrays cache NSPortCoder objects
   */
  if (cacheCoders == YES)
    {
      IcachedDecoders = [NSMutableArray new];
      IcachedEncoders = [NSMutableArray new];
    }

  /*
   * This is used to queue up incoming NSPortMessages representing requests
   * that can't immediately be dealt with.
   */
  IrequestQueue = [NSMutableArray new];

  /*
   * This maps request sequence numbers to the NSPortCoder objects representing
   * replies arriving from the remote connection.
   */
  IreplyMap = (GSIMapTable)NSZoneMalloc(z, sizeof(GSIMapTable_t));
  GSIMapInitWithZoneAndCapacity(IreplyMap, z, 4);

  /*
   * This maps (void*)obj to (id)obj.  The obj's are retained.
   * We use this instead of an NSHashTable because we only care about
   * the object's address, and don't want to send the -hash message to it.
   */
  IlocalObjects
    = (GSIMapTable)NSZoneMalloc(z, sizeof(GSIMapTable_t));
  GSIMapInitWithZoneAndCapacity(IlocalObjects, z, 4);

  /*
   * This maps handles for local objects to their local proxies.
   */
  IlocalTargets
    = (GSIMapTable)NSZoneMalloc(z, sizeof(GSIMapTable_t));
  GSIMapInitWithZoneAndCapacity(IlocalTargets, z, 4);

  /*
   * This maps targets to remote proxies.
   */
  IremoteProxies
    = (GSIMapTable)NSZoneMalloc(z, sizeof(GSIMapTable_t));
  GSIMapInitWithZoneAndCapacity(IremoteProxies, z, 4);

  IrequestDepth = 0;
  Idelegate = nil;
  IrefGate = [NSRecursiveLock new];

  /*
   * Some attributes are inherited from the parent if possible.
   */
  if (parent != nil)
    {
      NSUInteger	count;

      ImultipleThreads = GSIVar(parent, _multipleThreads);
      IindependentQueueing = GSIVar(parent, _independentQueueing);
      IreplyTimeout = GSIVar(parent, _replyTimeout);
      IrequestTimeout = GSIVar(parent, _requestTimeout);
      IrunLoops = [GSIVar(parent, _runLoops) mutableCopy];
      count = [GSIVar(parent, _requestModes) count];
      IrequestModes
	= [[NSMutableArray alloc] initWithCapacity: count];
      while (count-- > 0)
	{
	  [self addRequestMode:
	    [GSIVar(parent, _requestModes) objectAtIndex: count]];
	}
      if (GSIVar(parent, _useKeepalive) == YES)
	{
	  [self _enableKeepalive];
	}
    }
  else
    {
      ImultipleThreads = NO;
      IindependentQueueing = NO;
      IreplyTimeout = 1.0E12;
      IrequestTimeout = 1.0E12;
      /*
       * Set up request modes array and make sure the receiving port
       * is added to the run loop to get data.
       */
      loop = GSRunLoopForThread(nil);
      IrunLoops = [[NSMutableArray alloc] initWithObjects: &loop count: 1];
      IrequestModes = [[NSMutableArray alloc] initWithCapacity: 2];
      [self addRequestMode: NSDefaultRunLoopMode];
      [self addRequestMode: NSConnectionReplyMode];
      IuseKeepalive = NO;

      /*
       * If we have no parent, we must handle incoming packets on our
       * receive port ourself - so we set ourself up as the port delegate.
       */
      [IreceivePort setDelegate: self];
    }

  /* Ask the delegate for permission, (OpenStep-style and GNUstep-style). */

  /* Preferred MacOS-X version, which just allows the returning of BOOL */
  del = [parent delegate];
  if ([del respondsToSelector: @selector(connection:shouldMakeNewConnection:)])
    {
      if ([del connection: parent shouldMakeNewConnection: self] == NO)
	{
	  GSM_UNLOCK(connection_table_gate);
	  DESTROY(self);
	  return nil;
	}
    }
  /* Deprecated OpenStep version, which just allows the returning of BOOL */
  if ([del respondsToSelector: @selector(makeNewConnection:sender:)])
    {
      if (![del makeNewConnection: self sender: parent])
	{
	  GSM_UNLOCK(connection_table_gate);
	  DESTROY(self);
	  return nil;
	}
    }
  /* Here is the GNUstep version, which allows the delegate to specify
     a substitute.  Note: The delegate is responsible for freeing
     newConn if it returns something different. */
  if ([del respondsToSelector: @selector(connection:didConnect:)])
    {
      self = [del connection: parent didConnect: self];
    }

  nCenter = [NSNotificationCenter defaultCenter];
  /*
   * Register ourselves for invalidation notification when the
   * ports become invalid.
   */
  [nCenter addObserver: self
	      selector: @selector(_portIsInvalid:)
		  name: NSPortDidBecomeInvalidNotification
		object: r];
  if (s != nil)
    {
      [nCenter addObserver: self
		  selector: @selector(_portIsInvalid:)
		      name: NSPortDidBecomeInvalidNotification
		    object: s];
    }

  /* In order that connections may be deallocated - there is an
     implementation of [-release] to automatically remove the connection
     from this array when it is the only thing retaining it. */
  NSHashInsert(connection_table, (void*)self);
  GSM_UNLOCK(connection_table_gate);

  [nCenter postNotificationName: NSConnectionDidInitializeNotification
			 object: self];

  return self;
}

/**
 * Marks the receiving NSConnection as invalid.
 * <br />
 * Removes the NSConnections ports from any run loops.
 * <br />
 * Posts an NSConnectionDidDieNotification.
 * <br />
 * Invalidates all remote objects and local proxies.
 */
- (void) invalidate
{
  GS_M_LOCK(IrefGate);
  if (IisValid == NO)
    {
      GSM_UNLOCK(IrefGate);
      return;
    }
  if (IshuttingDown == NO)
    {
      IshuttingDown = YES;
      /*
       * Not invalidated as a result of a shutdown from the other end,
       * so tell the other end it must shut down.
       */
      //[self _shutdown];
    }
  IisValid = NO;
  GS_M_LOCK(connection_table_gate);
  NSHashRemove(connection_table, self);
  GSM_UNLOCK(connection_table_gate);

  GSM_UNLOCK(IrefGate);

  /*
   * Don't need notifications any more - so remove self as observer.
   */
  [[NSNotificationCenter defaultCenter] removeObserver: self];

  /*
   * Make sure we are not registered.
   */
#if	!defined(_WIN32)
  if ([IreceivePort isKindOfClass: [NSMessagePort class]])
    {
      [self registerName: nil
	  withNameServer: [NSMessagePortNameServer sharedInstance]];
    }
  else
#endif
  if ([IreceivePort isKindOfClass: [NSSocketPort class]])
    {
      [self registerName: nil
	  withNameServer: [NSSocketPortNameServer sharedInstance]];
    }
  else
    {
      [self registerName: nil];
    }

  /*
   * Withdraw from run loops.
   */
  [self setRequestMode: nil];

  IF_NO_GC(RETAIN(self);)

  if (debug_connection)
    {
      NSLog(@"Invalidating connection %@", self);
    }
  /*
   * We need to notify any watchers of our death - but if we are already
   * in the deallocation process, we can't have a notification retaining
   * and autoreleasing us later once we are deallocated - so we do the
   * notification with a local autorelease pool to ensure that any release
   * is done before the deallocation completes.
   */
  {
    NSAutoreleasePool	*arp = [NSAutoreleasePool new];

    [[NSNotificationCenter defaultCenter]
      postNotificationName: NSConnectionDidDieNotification
		    object: self];
    [arp drain];
  }

  /*
   *	If we have been invalidated, we don't need to retain proxies
   *	for local objects any more.  In fact, we want to get rid of
   *	these proxies in case they are keeping us retained when we
   *	might otherwise de deallocated.
   */
  GS_M_LOCK(IrefGate);
  if (IlocalTargets != 0)
    {
      NSMutableArray		*targets;
      NSUInteger		i = IlocalTargets->nodeCount;
      GSIMapEnumerator_t	enumerator;
      GSIMapNode 		node;

      targets = [[NSMutableArray alloc] initWithCapacity: i];
      enumerator = GSIMapEnumeratorForMap(IlocalTargets);
      node = GSIMapEnumeratorNextNode(&enumerator);
      while (node != 0)
	{
	  [targets addObject: node->value.obj];
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      while (i-- > 0)
	{
	  [self removeLocalObject: [targets objectAtIndex: i]];
	}
      RELEASE(targets);
      GSIMapEmptyMap(IlocalTargets);
      NSZoneFree(IlocalTargets->zone, (void*)IlocalTargets);
      IlocalTargets = 0;
    }
  if (IremoteProxies != 0)
    {
      GSIMapEmptyMap(IremoteProxies);
      NSZoneFree(IremoteProxies->zone, (void*)IremoteProxies);
      IremoteProxies = 0;
    }
  if (IlocalObjects != 0)
    {
      GSIMapEnumerator_t	enumerator;
      GSIMapNode 		node;

      enumerator = GSIMapEnumeratorForMap(IlocalObjects);
      node = GSIMapEnumeratorNextNode(&enumerator);

      while (node != 0)
	{
	  RELEASE(node->key.obj);
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      GSIMapEmptyMap(IlocalObjects);
      NSZoneFree(IlocalObjects->zone, (void*)IlocalObjects);
      IlocalObjects = 0;
    }
  GSM_UNLOCK(IrefGate);

  /*
   * If we are invalidated, we shouldn't be receiving any event and
   * should not need to be in any run loops.
   */
  while ([IrunLoops count] > 0)
    {
      [self removeRunLoop: [IrunLoops lastObject]];
    }

  /*
   * Invalidate the current conversation so we don't leak.
   */
  if ([IsendPort isValid] == YES)
    {
      [[IsendPort conversation: IreceivePort] invalidate];
    }

  RELEASE(self);
}

/**
 * Returns YES if the connection is valid, NO otherwise.
 * A connection is valid until it has been sent an -invalidate message.
 */
- (BOOL) isValid
{
  return IisValid;
}

/**
 * Returns an array of all the local objects that have proxies at the
 * remote end of the connection because they have been sent over the
 * connection and not yet released by the far end.
 */
- (NSArray*) localObjects
{
  NSArray	*a;

  /* Don't assert (IisValid); */
  GS_M_LOCK(IrefGate);
  if (IlocalObjects != 0)
    {

      GSIMapEnumerator_t	enumerator;
      GSIMapNode 		node;
      NSMutableArray            *c;

      enumerator = GSIMapEnumeratorForMap(IlocalObjects);
      node = GSIMapEnumeratorNextNode(&enumerator);

      c = [NSMutableArray arrayWithCapacity: IlocalObjects->nodeCount];
      while (node != 0)
	{
	  [c addObject: node->key.obj];
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      a = c;
    }
  else
    {
      a = [NSArray array];
    }
  GSM_UNLOCK(IrefGate);
  return a;
}

/**
 * Returns YES if the connection permits multiple threads to use it to
 * send requests, NO otherwise.<br />
 * See the -enableMultipleThreads method.
 */
- (BOOL) multipleThreadsEnabled
{
  return ImultipleThreads;
}

/**
 * Returns the NSPort object on which incoming messages are received.
 */
- (NSPort*) receivePort
{
  return IreceivePort;
}

/**
 * Simply invokes -registerName:withNameServer:
 * passing it the default system nameserver.
 */
- (BOOL) registerName: (NSString*)name
{
  NSPortNameServer	*svr = [NSPortNameServer systemDefaultPortNameServer];

  return [self registerName: name withNameServer: svr];
}

/**
 * Registers the receive port of the NSConnection as name and
 * unregisters the previous value (if any).<br />
 * Returns YES on success, NO on failure.<br />
 * On failure, the connection remains registered under the
 * previous name.<br />
 * Supply nil as name to unregister the NSConnection.
 */
- (BOOL) registerName: (NSString*)name withNameServer: (NSPortNameServer*)svr
{
  BOOL			result = YES;

  if (name != nil)
    {
      result = [svr registerPort: IreceivePort forName: name];
    }
  if (result == YES)
    {
      if (IregisteredName != nil)
	{
	  [InameServer removePort: IreceivePort forName: IregisteredName];
	}
      ASSIGN(IregisteredName, name);
      ASSIGN(InameServer, svr);
    }
  return result;
}

- (oneway void) release
{
  /* We lock the connection table while checking, to prevent
   * another thread from grabbing this connection while we are
   * checking it.
   * If we are going to deallocate the object, we first remove
   * it from the table so that no other thread will find it
   * and try to use it while it is being deallocated.
   */
  GS_M_LOCK(connection_table_gate);
  if (NSDecrementExtraRefCountWasZero(self))
    {
      NSHashRemove(connection_table, self);
      GSM_UNLOCK(connection_table_gate);
      [self dealloc];
    }
  else
    {
      GSM_UNLOCK(connection_table_gate);
    }
}

/**
 * Returns an array of proxies to all the remote objects known to
 * the NSConnection.
 */
- (NSArray *) remoteObjects
{
  NSMutableArray	*c;

  /* Don't assert (IisValid); */
  GS_M_LOCK(IrefGate);
  if (IremoteProxies != 0)
    {
      GSIMapEnumerator_t	enumerator;
      GSIMapNode 		node;

      enumerator = GSIMapEnumeratorForMap(IremoteProxies);
      node = GSIMapEnumeratorNextNode(&enumerator);

      c = [NSMutableArray arrayWithCapacity: IremoteProxies->nodeCount];
      while (node != 0)
	{
	  [c addObject: node->key.obj];
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
    }
  else
    {
      c = [NSMutableArray array];
    }
  GSM_UNLOCK(IrefGate);
  return c;
}

/**
 * Removes mode from the run loop modes used to receive incoming messages.
 */
- (void) removeRequestMode: (NSString*)mode
{
  GS_M_LOCK(IrefGate);
  if (IrequestModes != nil && [IrequestModes containsObject: mode])
    {
      NSUInteger	c = [IrunLoops count];

      while (c-- > 0)
	{
	  NSRunLoop	*loop = [IrunLoops objectAtIndex: c];

	  [IreceivePort removeConnection: self
			     fromRunLoop: loop
				 forMode: mode];
	}
      [IrequestModes removeObject: mode];
    }
  GSM_UNLOCK(IrefGate);
}

/**
 * Removes loop from the run loops used to receive incoming messages.
 */
- (void) removeRunLoop: (NSRunLoop*)loop
{
  GS_M_LOCK(IrefGate);
  if (IrunLoops != nil)
    {
      NSUInteger	pos = [IrunLoops indexOfObjectIdenticalTo: loop];

      if (pos != NSNotFound)
	{
	  NSUInteger	c = [IrequestModes count];

	  while (c-- > 0)
	    {
	      NSString	*mode = [IrequestModes objectAtIndex: c];

	      [IreceivePort removeConnection: self
				 fromRunLoop: [IrunLoops objectAtIndex: pos]
				     forMode: mode];
	    }
	  [IrunLoops removeObjectAtIndex: pos];
	}
    }
  GSM_UNLOCK(IrefGate);
}

/**
 * Returns the timeout interval used when waiting for a reply to
 * a request sent on the NSConnection.  This value is inherited
 * from the parent connection or may be set using the -setReplyTimeout:
 * method.<br />
 * The default value is the maximum delay (effectively infinite).
 */
- (NSTimeInterval) replyTimeout
{
  return IreplyTimeout;
}

/**
 * Returns an array of all the run loop modes that the NSConnection
 * uses when waiting for an incoming request.
 */
- (NSArray*) requestModes
{
  NSArray	*c;

  GS_M_LOCK(IrefGate);
  c = AUTORELEASE([IrequestModes copy]);
  GSM_UNLOCK(IrefGate);
  return c;
}

/**
 * Returns the timeout interval used when trying to send a request
 * on the NSConnection.  This value is inherited from the parent
 * connection or may be set using the -setRequestTimeout: method.<br />
 * The default value is the maximum delay (effectively infinite).
 */
- (NSTimeInterval) requestTimeout
{
  return IrequestTimeout;
}

/**
 * Returns the object that is made available by this connection
 * or by its parent (the object is associated with the receive port).<br />
 * Returns nil if no root object has been set.
 */
- (id) rootObject
{
  return rootObjectForInPort(IreceivePort);
}

/**
 * Returns the proxy for the root object of the remote NSConnection.<br />
 * Generally you will wish to call [NSDistantObject-setProtocolForProxy:]
 * immediately after obtaining such a root proxy.
 */
- (NSDistantObject*) rootProxy
{
  NSPortCoder		*op;
  NSPortCoder		*ip;
  NSDistantObject	*newProxy = nil;
  int			seq_num;

  NSParameterAssert(IreceivePort);
  NSParameterAssert(IisValid);

  NS_DURING
    {
      /*
       * If this is a server connection without a remote end, its root proxy
       * is the same as its root object.
       */
      if (IreceivePort == IsendPort)
        {
          return [self rootObject];
        }
      op = [self _newOutRmc: 0 generate: &seq_num reply: YES];
      [self _sendOutRmc: op type: ROOTPROXY_REQUEST sequence: seq_num];

      ip = [self _getReplyRmc: seq_num for: "rootproxy"];
      [ip decodeValueOfObjCType: @encode(id) at: &newProxy];
      [self _doneInRmc: ip];
    }
  NS_HANDLER
    {
      /* The ports/connection may have been invalidated while getting the
       * root proxy ... if so we should return nil.
       */
      newProxy = nil;
    }
  NS_ENDHANDLER

  return AUTORELEASE(newProxy);
}

/**
 * Removes the NSConnection from the current threads default
 * run loop, then creates a new thread and runs the NSConnection in it.
 */
- (void) runInNewThread
{
  [self removeRunLoop: GSRunLoopForThread(nil)];
  [NSThread detachNewThreadSelector: @selector(_runInNewThread)
			   toTarget: self
			 withObject: nil];
}

/**
 * Returns the port on which the NSConnection sends messages.
 */
- (NSPort*) sendPort
{
  return IsendPort;
}

/**
 * Sets the NSConnection's delegate (without retaining it).<br />
 * The delegate is able to control some of the NSConnection's
 * behavior by implementing methods in an informal protocol.
 */
- (void) setDelegate: (id)anObj
{
  Idelegate = anObj;
  IauthenticateIn =
    [anObj respondsToSelector: @selector(authenticateComponents:withData:)];
  IauthenticateOut =
    [anObj respondsToSelector: @selector(authenticationDataForComponents:)];
}

/**
 * Sets whether or not the NSConnection should handle requests
 * arriving from the remote NSConnection atomically.<br />
 * By default, this is set to NO ... if set to YES then any messages
 * arriving while one message is being dealt with, will be queued.<br />
 * NB. careful - use of this option can cause deadlocks.
 */
- (void) setIndependentConversationQueueing: (BOOL)flag
{
  IindependentQueueing = flag;
}

/**
 * Sets the time interval that the NSConnection will wait for a
 * reply for one of its requests before raising an
 * NSPortTimeoutException.<br />
 * NB. In GNUstep you may also get such an exception if the connection
 * becomes invalidated while waiting for a reply to a request.
 */
- (void) setReplyTimeout: (NSTimeInterval)to
{
  if (to <= 0.0 || to > 1.0E12) to = 1.0E12;
  IreplyTimeout = to;
}

/**
 * Sets the runloop mode in which requests will be sent to the remote
 * end of the connection.  Normally this is NSDefaultRunloopMode
 */
- (void) setRequestMode: (NSString*)mode
{
  GS_M_LOCK(IrefGate);
  if (IrequestModes != nil)
    {
      while ([IrequestModes count] > 0
	&& [IrequestModes objectAtIndex: 0] != mode)
	{
	  [self removeRequestMode: [IrequestModes objectAtIndex: 0]];
	}
      while ([IrequestModes count] > 1)
	{
	  [self removeRequestMode: [IrequestModes objectAtIndex: 1]];
	}
      if (mode != nil && [IrequestModes count] == 0)
	{
	  [self addRequestMode: mode];
	}
    }
  GSM_UNLOCK(IrefGate);
}

/**
 * Sets the time interval that the NSConnection will wait to send
 * one of its requests before raising an NSPortTimeoutException.
 */
- (void) setRequestTimeout: (NSTimeInterval)to
{
  if (to <= 0.0 || to > 1.0E12) to = 1.0E12;
  IrequestTimeout = to;
}

/**
 * Sets the root object that is vended by the connection.
 */
- (void) setRootObject: (id)anObj
{
  setRootObjectForInPort(anObj, IreceivePort);
#if	defined(_WIN32)
  /* On ms-windows, the operating system does not inform us when the remote
   * client of a message port goes away ... so we need to enable keepalive
   * to detect that condition.
   */
  if ([IreceivePort isKindOfClass: [NSMessagePort class]])
    {
      [self _enableKeepalive];
    }
#endif
}

/**
 * Returns an object containing various statistics for the
 * NSConnection.
 * <br />
 * On GNUstep the dictionary contains -
 * <deflist>
 *   <term>NSConnectionRepliesReceived</term>
 *   <desc>
 *     The number of messages replied to by the remote NSConnection.
 *   </desc>
 *   <term>NSConnectionRepliesSent</term>
 *   <desc>
 *     The number of replies sent to the remote NSConnection.
 *   </desc>
 *   <term>NSConnectionRequestsReceived</term>
 *   <desc>
 *     The number of messages received from the remote NSConnection.
 *   </desc>
 *   <term>NSConnectionRequestsSent</term>
 *   <desc>
 *     The number of messages sent to the remote NSConnection.
 *   </desc>
 *   <term>NSConnectionLocalCount</term>
 *   <desc>
 *     The number of local objects currently vended.
 *   </desc>
 *   <term>NSConnectionProxyCount</term>
 *   <desc>
 *     The number of remote objects currently in use.
 *   </desc>
 * </deflist>
 */
- (NSDictionary*) statistics
{
  NSMutableDictionary	*d;
  id			o;

  d = [NSMutableDictionary dictionaryWithCapacity: 8];

  GS_M_LOCK(IrefGate);

  /*
   *	These are in OPENSTEP 4.2
   */
  o = [NSNumber numberWithUnsignedInt: IrepInCount];
  [d setObject: o forKey: NSConnectionRepliesReceived];
  o = [NSNumber numberWithUnsignedInt: IrepOutCount];
  [d setObject: o forKey: NSConnectionRepliesSent];
  o = [NSNumber numberWithUnsignedInt: IreqInCount];
  [d setObject: o forKey: NSConnectionRequestsReceived];
  o = [NSNumber numberWithUnsignedInt: IreqOutCount];
  [d setObject: o forKey: NSConnectionRequestsSent];

  /*
   *	These are GNUstep extras
   */
  o = [NSNumber numberWithUnsignedInt:
    IlocalTargets ? IlocalTargets->nodeCount : 0];
  [d setObject: o forKey: NSConnectionLocalCount];
  o = [NSNumber numberWithUnsignedInt:
    IremoteProxies ? IremoteProxies->nodeCount : 0];
  [d setObject: o forKey: NSConnectionProxyCount];
  o = [NSNumber numberWithUnsignedInt:
    IreplyMap ? IreplyMap->nodeCount : 0];
  [d setObject: o forKey: @"NSConnectionReplyQueue"];
  o = [NSNumber numberWithUnsignedInt: [IrequestQueue count]];
  [d setObject: o forKey: @"NSConnectionRequestQueue"];

  GSM_UNLOCK(IrefGate);

  return d;
}

@end



@implementation	NSConnection (GNUstepExtensions)

+ (NSConnection*) newRegisteringAtName: (NSString*)name
			withRootObject: (id)anObject
{
  NSConnection	*conn;

  GSOnceMLog(@"This method is deprecated, use standard initialisation");

  conn = [[self alloc] initWithReceivePort: [NSPort port]
				  sendPort: nil];
  [conn setRootObject: anObject];
  if ([conn registerName: name] == NO)
    {
      DESTROY(conn);
    }
  return conn;
}

- (void) finalize
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];

  if (debug_connection)
    NSLog(@"finalising %@", self);

  [self invalidate];

  /* Remove rootObject from root_object_map if this is last connection */
  if (IreceivePort != nil && existingConnection(IreceivePort, nil) == nil)
    {
      setRootObjectForInPort(nil, IreceivePort);
    }

  /* Remove receive port from run loop. */
  [self setRequestMode: nil];

  DESTROY(IrequestModes);
  DESTROY(IrunLoops);

  /*
   * Finished with ports - releasing them may generate a notification
   * If we are the receive port delagate, try to shift responsibility.
   */
  if ([IreceivePort delegate] == self)
    {
      NSConnection	*root = existingConnection(IreceivePort, IreceivePort);

      if (root == nil)
	{
	  root =  existingConnection(IreceivePort, nil);
	}
      [IreceivePort setDelegate: root];
    }
  DESTROY(IreceivePort);
  DESTROY(IsendPort);

  DESTROY(IrequestQueue);
  if (IreplyMap != 0)
    {
      GSIMapEnumerator_t	enumerator;
      GSIMapNode 		node;

      enumerator = GSIMapEnumeratorForMap(IreplyMap);
      node = GSIMapEnumeratorNextNode(&enumerator);

      while (node != 0)
	{
	  if (node->value.obj != dummyObject)
	    {
	      RELEASE(node->value.obj);
	    }
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      GSIMapEmptyMap(IreplyMap);
      NSZoneFree(IreplyMap->zone, (void*)IreplyMap);
      IreplyMap = 0;
    }

  DESTROY(IcachedDecoders);
  DESTROY(IcachedEncoders);

  DESTROY(IremoteName);

  DESTROY(IrefGate);

  [arp drain];
}

/*
 * NSDistantObject's -forwardInvocation: method calls this to send the message
 * over the wire.
 */
- (void) forwardInvocation: (NSInvocation*)inv
		  forProxy: (NSDistantObject*)object
{
  NSPortCoder	*op;
  BOOL		outParams;
  BOOL		needsResponse;
  const char	*name;
  const char	*type;
  unsigned	seq;
  NSRunLoop	*runLoop = GSRunLoopForThread(nil);

  if ([IrunLoops indexOfObjectIdenticalTo: runLoop] == NSNotFound)
    {
      if (ImultipleThreads == NO)
	{
	  [NSException raise: NSObjectInaccessibleException
            format: @"Forwarding message for %p in wrong thread - %@",
            object, inv];
	}
      else
	{
	  [self addRunLoop: runLoop];
	}
    }

  /* Encode the method on an RMC, and send it. */

  NSParameterAssert (IisValid);

  /* get the method types from the selector */
  type = [[inv methodSignature] methodType];
  if (type == 0 || *type == '\0')
    {
      type = [[object methodSignatureForSelector: [inv selector]] methodType];
      if (type)
	{
	  GSSelectorFromNameAndTypes(sel_getName([inv selector]), type);
	}
    }
  NSParameterAssert(type);
  NSParameterAssert(*type);

  op = [self _newOutRmc: 0 generate: (int*)&seq reply: YES];

  if (debug_connection > 4)
    NSLog(@"building packet seq %d", seq);

  [inv setTarget: object];
  outParams = [inv encodeWithDistantCoder: op passPointers: NO];

  if (outParams == YES)
    {
      needsResponse = YES;
    }
  else
    {
      int		flags;

      needsResponse = NO;
      flags = objc_get_type_qualifiers(type);
      if ((flags & _F_ONEWAY) == 0)
	{
	  needsResponse = YES;
	}
      else
	{
	  const char	*tmptype = objc_skip_type_qualifiers(type);

	  if (*tmptype != _C_VOID)
	    {
	      needsResponse = YES;
	    }
	}
    }

  [self _sendOutRmc: op type: METHOD_REQUEST sequence: seq];
  name = sel_getName([inv selector]);
  NSDebugMLLog(@"NSConnection", @"Sent message %s RMC %d to 0x%"PRIxPTR,
    name, seq, (NSUInteger)self);

  if (needsResponse == NO)
    {
      GSIMapNode	node;

      /*
       * Since we don't need a response, we can remove the placeholder from
       * the IreplyMap.  However, in case the other end has already sent us
       * a response, we must check for it and scrap it if necessary.
       */
      GS_M_LOCK(IrefGate);
      node = GSIMapNodeForKey(IreplyMap, (GSIMapKey)(NSUInteger)seq);
      if (node != 0 && node->value.obj != dummyObject)
	{
	  BOOL	is_exception = NO;

	  [node->value.obj decodeValueOfObjCType: @encode(BOOL)
					      at: &is_exception];
	  if (is_exception == YES)
	    NSLog(@"Got exception with %s", name);
	  else
	    NSLog(@"Got response with %s", name);
	  [self _doneInRmc: node->value.obj];
	}
      GSIMapRemoveKey(IreplyMap, (GSIMapKey)(NSUInteger)seq);
      GSM_UNLOCK(IrefGate);
    }
  else
    {
      int		argnum;
      int		flags;
      const char	*tmptype;
      void		*datum;
      NSPortCoder	*aRmc;
      BOOL		is_exception;

      if ([self isValid] == NO)
	{
	  [NSException raise: NSGenericException
	    format: @"connection waiting for request was shut down"];
	}
      aRmc = [self _getReplyRmc: seq for: name];

      /*
       * Find out if the server is returning an exception instead
       * of the return values.
       */
      [aRmc decodeValueOfObjCType: @encode(BOOL) at: &is_exception];
      if (is_exception == YES)
	{
	  /* Decode the exception object, and raise it. */
	  id exc = [aRmc decodeObject];

	  [self _doneInReply: aRmc];
	  [exc raise];
	}

      /* Get the return type qualifier flags, and the return type. */
      flags = objc_get_type_qualifiers(type);
      tmptype = objc_skip_type_qualifiers(type);

      /* Decode the return value and pass-by-reference values, if there
	 are any.  OUT_PARAMETERS should be the value returned by
	 cifframe_dissect_call(). */
      if (outParams || *tmptype != _C_VOID || (flags & _F_ONEWAY) == 0)
	/* xxx What happens with method declared "- (oneway) foo: (out int*)ip;" */
	/* xxx What happens with method declared "- (in char *) bar;" */
	/* xxx Is this right?  Do we also have to check _F_ONEWAY? */
	{
	  id	obj;

	  /* If there is a return value, decode it, and put it in datum. */
	  if (*tmptype != _C_VOID || (flags & _F_ONEWAY) == 0)
	    {
	      switch (*tmptype)
		{
		  case _C_ID:
		    datum = &obj;
		    [aRmc decodeValueOfObjCType: tmptype at: datum];
		    [obj autorelease];
		    break;
		  case _C_PTR:
		    /* We are returning a pointer to something. */
		    tmptype++;
		    datum = alloca (objc_sizeof_type (tmptype));
		    [aRmc decodeValueOfObjCType: tmptype at: datum];
		    break;

		  case _C_VOID:
		    datum = alloca (sizeof (int));
		    [aRmc decodeValueOfObjCType: @encode(int) at: datum];
		    break;

		  default:
		    datum = alloca (objc_sizeof_type (tmptype));
		    [aRmc decodeValueOfObjCType: tmptype at: datum];
		    break;
		}
	    }
	  else
	    {
	      datum = 0;
	    }
	  [inv setReturnValue: datum];

	  /* Decode the values returned by reference.  Note: this logic
	     must match exactly the code in _service_forwardForProxy:
	     */
	  if (outParams)
	    {
	      /* Step through all the arguments, finding the ones that were
		 passed by reference. */
	      for (tmptype = skip_argspec (tmptype), argnum = 0;
	        *tmptype != '\0';
	        tmptype = skip_argspec (tmptype), argnum++)
		{
		  /* Get the type qualifiers, like IN, OUT, INOUT, ONEWAY. */
		  flags = objc_get_type_qualifiers(tmptype);
		  /* Skip over the type qualifiers, so now TYPE is
		     pointing directly at the char corresponding to the
		     argument's type. */
		  tmptype = objc_skip_type_qualifiers(tmptype);

		  if (*tmptype == _C_PTR
		    && ((flags & _F_OUT) || !(flags & _F_IN)))
		    {
		      /* If the arg was byref, we obtain its address
		       * and decode the data directly to it.
		       */
		      tmptype++;
		      [inv getArgument: &datum atIndex: argnum];
		      [aRmc decodeValueOfObjCType: tmptype at: datum];
		      if (*tmptype == _C_ID)
			{
			  [*(id*)datum autorelease];
			}
		    }
		  else if (*tmptype == _C_CHARPTR
		    && ((flags & _F_OUT) || !(flags & _F_IN)))
		    {
		      [aRmc decodeValueOfObjCType: tmptype at: &datum];
		      [inv setArgument: datum atIndex: argnum];
		    }
		}
	    }
	}
      [self _doneInReply: aRmc];
    }
}

- (const char *) typeForSelector: (SEL)sel remoteTarget: (unsigned)target
{
  id op, ip;
  char	*type = 0;
  int	seq_num;
  NSData *data;

  NSParameterAssert(IreceivePort);
  NSParameterAssert (IisValid);
  op = [self _newOutRmc: 0 generate: &seq_num reply: YES];
  [op encodeValueOfObjCType: ":" at: &sel];
  [op encodeValueOfObjCType: @encode(unsigned) at: &target];
  [self _sendOutRmc: op type: METHODTYPE_REQUEST sequence: seq_num];
  ip = [self _getReplyRmc: seq_num for: "methodtype"];
  [ip decodeValueOfObjCType: @encode(char*) at: &type];
  data = type ? [NSData dataWithBytes: type length: strlen(type)+1] : nil;
  [self _doneInRmc: ip];
  return (const char*)[data bytes];
}


/* Class-wide stats and collections. */

+ (unsigned) connectionsCount
{
  unsigned	result;

  GS_M_LOCK(connection_table_gate);
  result = NSCountHashTable(connection_table);
  GSM_UNLOCK(connection_table_gate);
  return result;
}

+ (unsigned) connectionsCountWithInPort: (NSPort*)aPort
{
  unsigned		count = 0;
  NSHashEnumerator	enumerator;
  NSConnection		*o;

  GS_M_LOCK(connection_table_gate);
  enumerator = NSEnumerateHashTable(connection_table);
  while ((o = (NSConnection*)NSNextHashEnumeratorItem(&enumerator)) != nil)
    {
      if ([aPort isEqual: [o receivePort]])
	{
	  count++;
	}
    }
  NSEndHashTableEnumeration(&enumerator);
  GSM_UNLOCK(connection_table_gate);

  return count;
}

@end





@implementation	NSConnection (Private)

- (void) handlePortMessage: (NSPortMessage*)msg
{
  NSPortCoder		*rmc;
  int			type = [msg msgid];
  NSMutableArray	*components = [msg _components];
  NSPort		*rp = [msg receivePort];
  NSPort		*sp = [msg sendPort];
  NSConnection		*conn;

  if (debug_connection > 4)
    {
      NSLog(@"handling packet of type %d (%@)", type, stringFromMsgType(type));
    }
  conn = [connectionClass connectionWithReceivePort: rp sendPort: sp];
  if (conn == nil)
    {
      NSLog(@"Received port message for unknown connection - %@", msg);
      NSLog(@"All connections: %@", [NSConnection allConnections]);
      return;
    }
  else if ([conn isValid] == NO)
    {
      if (debug_connection)
	{
	  NSLog(@"received port message for invalid connection - %@", msg);
	}
      return;
    }
  if (debug_connection > 4)
    {
      NSLog(@"  connection is %@", conn);
    }

  if (GSIVar(conn, _authenticateIn) == YES
    && (type == METHOD_REQUEST || type == METHOD_REPLY))
    {
      NSData	        *d;
      NSUInteger	count = [components count];

      d = RETAIN([components objectAtIndex: --count]);
      [components removeObjectAtIndex: count];
      if ([[conn delegate] authenticateComponents: components
					 withData: d] == NO)
	{
	  RELEASE(d);
	  [NSException raise: NSFailedAuthenticationException
		      format: @"message not authenticated by delegate"];
	}
      RELEASE(d);
    }

  rmc = [conn _newInRmc: components];
  if (debug_connection > 5)
    {
      NSLog(@"made rmc %p for %d", rmc, type);
    }

  switch (type)
    {
      case ROOTPROXY_REQUEST:
	/* It won't take much time to handle this, so go ahead and service
	   it, even if we are waiting for a reply. */
	[conn _service_rootObject: rmc];
	break;

      case METHODTYPE_REQUEST:
	/* It won't take much time to handle this, so go ahead and service
	   it, even if we are waiting for a reply. */
	[conn _service_typeForSelector: rmc];
	break;

      case METHOD_REQUEST:
	/*
	 * We just got a new request; we need to decide whether to queue
	 * it or service it now.
	 * If the REPLY_DEPTH is 0, then we aren't in the middle of waiting
	 * for a reply, we are waiting for requests---so service it now.
	 * If REPLY_DEPTH is non-zero, we may still want to service it now
	 * if independent_queuing is NO.
	 */
	GS_M_LOCK(GSIVar(conn, _refGate));
	if (GSIVar(conn, _requestDepth) == 0
	  || GSIVar(conn, _independentQueueing) == NO)
	  {
	    GSIVar(conn, _requestDepth)++;
	    GSM_UNLOCK(GSIVar(conn, _refGate));
	    [conn _service_forwardForProxy: rmc];	// Catches exceptions
	    GS_M_LOCK(GSIVar(conn, _refGate));
	    GSIVar(conn, _requestDepth)--;
	  }
	else
	  {
	    [GSIVar(conn, _requestQueue) addObject: rmc];
	  }
	/*
	 * Service any requests that were queued while we
	 * were waiting for replies.
	 */
	while (GSIVar(conn, _requestDepth) == 0
	  && [GSIVar(conn, _requestQueue) count] > 0)
	  {
	    rmc = [GSIVar(conn, _requestQueue) objectAtIndex: 0];
	    [GSIVar(conn, _requestQueue) removeObjectAtIndex: 0];
	    GSM_UNLOCK(GSIVar(conn, _refGate));
	    [conn _service_forwardForProxy: rmc];	// Catches exceptions
	    GS_M_LOCK(GSIVar(conn, _refGate));
	  }
	GSM_UNLOCK(GSIVar(conn, _refGate));
	break;

      /*
       * For replies, we read the sequence number from the reply object and
       * store it in a map using thee sequence number as the key.  That way
       * it's easy for the connection to find replies by their numbers.
       */
      case ROOTPROXY_REPLY:
      case METHOD_REPLY:
      case METHODTYPE_REPLY:
      case RETAIN_REPLY:
	{
	  int		sequence;
	  GSIMapNode	node;

	  [rmc decodeValueOfObjCType: @encode(int) at: &sequence];
	  if (type == ROOTPROXY_REPLY && GSIVar(conn, _keepaliveWait) == YES
	    && sequence == GSIVar(conn, _lastKeepalive))
	    {
	      GSIVar(conn, _keepaliveWait) = NO;
	      NSDebugMLLog(@"NSConnection", @"Handled keepalive %d on %@",
		sequence, conn);
	      [self _doneInRmc: rmc];
	      break;
	    }
	  GS_M_LOCK(GSIVar(conn, _refGate));
	  node = GSIMapNodeForKey(GSIVar(conn, _replyMap),
	    (GSIMapKey)(NSUInteger)sequence);
	  if (node == 0)
	    {
	      NSDebugMLLog(@"NSConnection", @"Ignoring reply RMC %d on %@",
		sequence, conn);
	      [self _doneInRmc: rmc];
	    }
	  else if (node->value.obj == dummyObject)
	    {
	      NSDebugMLLog(@"NSConnection", @"Saving reply RMC %d on %@",
		sequence, conn);
	      node->value.obj = rmc;
	    }
	  else
	    {
	      NSDebugMLLog(@"NSConnection", @"Replace reply RMC %d on %@",
		sequence, conn);
	      [self _doneInRmc: node->value.obj];
	      node->value.obj = rmc;
	    }
	  GSM_UNLOCK(GSIVar(conn, _refGate));
	}
	break;

      case CONNECTION_SHUTDOWN:
	{
	  [conn _service_shutdown: rmc];
	  break;
	}
      case PROXY_RELEASE:
	{
	  [conn _service_release: rmc];
	  break;
	}
      case PROXY_RETAIN:
	{
	  [conn _service_retain: rmc];
	  break;
	}
      default:
	[NSException raise: NSGenericException
		    format: @"unrecognized NSPortCoder identifier"];
    }
}

- (void) _runInNewThread
{
  NSRunLoop	*loop = GSRunLoopForThread(nil);

  [self addRunLoop: loop];
  [loop run];
}

+ (int) setDebug: (int)val
{
  int   old = debug_connection;

  debug_connection = val;
  return old;
}

- (void) _keepalive: (NSNotification*)n
{
  if ([self isValid])
    {
      if (IkeepaliveWait == NO)
	{
	  NSPortCoder	*op;

	  /* Send out a root proxy request to ping the other end.
	   */
	  op = [self _newOutRmc: 0 generate: &IlastKeepalive reply: NO];
	  IkeepaliveWait = YES;
	  [self _sendOutRmc: op
                       type: ROOTPROXY_REQUEST
                   sequence: IlastKeepalive];
	}
      else
	{
	  /* keepalive timeout outstanding still.
	   */
	  [self invalidate];
	}
    }
}

/**
 */
- (void) _enableKeepalive
{
  IuseKeepalive = YES;	/* Set so that child connections will inherit. */
  IkeepaliveWait = NO;
  if (IreceivePort !=IsendPort)
    {
      /* If this is not a listening connection, we actually enable the
       * keepalive timing (usng the regular housekeeping notifications)
       * and must also enable multiple thread support as the keepalive
       * notification may arrive in a different thread from the one we
       * are running in.
       */
      [self enableMultipleThreads];
      [[NSNotificationCenter defaultCenter] addObserver: self
	selector: @selector(_keepalive:)
	name: @"GSHousekeeping" object: nil];
    }
}


/* NSConnection calls this to service the incoming method request. */
- (void) _service_forwardForProxy: (NSPortCoder*)aRmc
{
  char		*forward_type = 0;
  NSPortCoder	*decoder = nil;
  NSPortCoder	*encoder = nil;
  NSInvocation	*inv = nil;
  unsigned	seq;

  /* Save this for later */
  [aRmc decodeValueOfObjCType: @encode(int) at: &seq];

  /*
   * Make sure don't let exceptions caused by servicing the client's
   * request cause us to crash.
   */
  NS_DURING
    {
      NSRunLoop		*runLoop = GSRunLoopForThread(nil);
      const char	*type;
      const char	*tmptype;
      const char	*etmptype;
      id		tmp;
      id		target;
      SEL		selector;
      BOOL		is_exception = NO;
      BOOL		is_async = NO;
      BOOL		is_void = NO;
      unsigned		flags;
      int		argnum;
      unsigned		in_parameters = 0;
      unsigned		out_parameters = 0;
      NSMethodSignature	*sig;
      const char	*encoded_types = forward_type;

      NSParameterAssert (IisValid);
      if ([IrunLoops indexOfObjectIdenticalTo: runLoop] == NSNotFound)
	{
	  if (ImultipleThreads == YES)
	    {
	      [self addRunLoop: runLoop];
	    }
	  else
	    {
	      [NSException raise: NSObjectInaccessibleException
			  format: @"Message received in wrong thread"];
	    }
	}

      /*
       * Get the types that we're using, so that we know
       * exactly what qualifiers the forwarder used.
       * If all selectors included qualifiers and I could make
       * sel_types_match() work the way I wanted, we wouldn't need
       * to do this.
       */
      [aRmc decodeValueOfObjCType: @encode(char*) at: &forward_type];

      if (debug_connection > 1)
        {
          NSLog(@"Handling message (sig %s) RMC %d from %p",
	    forward_type, seq, self);
	}

      IreqInCount++;	/* Handling an incoming request. */

      encoded_types = forward_type;
      etmptype = encoded_types;

      decoder = aRmc;

      /* Decode the object, (which is always the first argument to a method).
       * Use the -decodeObject method to ensure that the target of the
       * invocation is autoreleased and will be deallocated when we finish.
       */
      target = [decoder decodeObject];

      /* Decode the selector, (which is the second argument to a method). */
      /* xxx @encode(SEL) produces "^v" in gcc 2.5.8.  It should be ":" */
      [decoder decodeValueOfObjCType: @encode(SEL) at: &selector];

      /* Get the "selector type" for this method.  The "selector type" is
	 a string that lists the return and argument types, and also
	 indicates in which registers and where on the stack the arguments
	 should be placed before the method call.  The selector type
	 string we get here should have the same argument and return types
	 as the ENCODED_TYPES string, but it will have different register
	 and stack locations if the ENCODED_TYPES came from a machine of a
	 different architecture. */
      sig = [target methodSignatureForSelector: selector];
      if (nil == sig)
	{
	  [NSException raise: NSInvalidArgumentException
		       format: @"decoded object %p doesn't handle %s",
	    target, sel_getName(selector)];
	}
      type = [sig methodType];

      /* Make sure we successfully got the method type, and that its
	 types match the ENCODED_TYPES. */
      NSCParameterAssert (type);
      if (GSSelectorTypesMatch(encoded_types, type) == NO)
	{
	  [NSException raise: NSInvalidArgumentException
	    format: @"NSConection types (%s / %s) mismatch for %s",
	    encoded_types, type, sel_getName(selector)];
	}

      inv = [[NSInvocation alloc] initWithMethodSignature: sig];

      tmptype = skip_argspec (type);
      etmptype = skip_argspec (etmptype);
      [inv setTarget: target];

      tmptype = skip_argspec (tmptype);
      etmptype = skip_argspec (etmptype);
      [inv setSelector: selector];


      /* Step TMPTYPE and ETMPTYPE in lock-step through their
	 method type strings. */

      for (tmptype = skip_argspec (tmptype),
	   etmptype = skip_argspec (etmptype), argnum = 2;
	   *tmptype != '\0';
	   tmptype = skip_argspec (tmptype),
	   etmptype = skip_argspec (etmptype), argnum++)
	{
	  void	*datum;

	  /* Get the type qualifiers, like IN, OUT, INOUT, ONEWAY. */
	  flags = objc_get_type_qualifiers (etmptype);
	  /* Skip over the type qualifiers, so now TYPE is pointing directly
	     at the char corresponding to the argument's type.  */
	  tmptype = objc_skip_type_qualifiers(tmptype);

	  /* Decide how, (or whether or not), to decode the argument
	     depending on its FLAGS and TMPTYPE.  Only the first two cases
	     involve parameters that may potentially be passed by
	     reference, and thus only the first two may change the value
	     of OUT_PARAMETERS.  *** Note: This logic must match exactly
	     the code in cifframe_dissect_call(); that function should
	     encode exactly what we decode here. *** */

	  switch (*tmptype)
	    {
	      case _C_CHARPTR:
		/* Handle a (char*) argument. */
		/* If the char* is qualified as an OUT parameter, or if it
		   not explicitly qualified as an IN parameter, then we will
		   have to get this char* again after the method is run,
		   because the method may have changed it.  Set
		   OUT_PARAMETERS accordingly. */
		if ((flags & _F_OUT) || !(flags & _F_IN))
		  out_parameters++;
		else
		  in_parameters++;
		/* If the char* is qualified as an IN parameter, or not
		   explicity qualified as an OUT parameter, then decode it.
		   Note: the decoder allocates memory for holding the
		   string, and it is also responsible for making sure that
		   the memory gets freed eventually, (usually through the
		   autorelease of NSData object). */
		if ((flags & _F_IN) || !(flags & _F_OUT))
		  {
		    datum = alloca (sizeof(char*));
		    [decoder decodeValueOfObjCType: tmptype at: datum];
		    [inv setArgument: datum atIndex: argnum];
		  }
		break;

	      case _C_PTR:
		/* If the pointer's value is qualified as an OUT parameter,
		   or if it not explicitly qualified as an IN parameter,
		   then we will have to get the value pointed to again after
		   the method is run, because the method may have changed
		   it.  Set OUT_PARAMETERS accordingly. */
		if ((flags & _F_OUT) || !(flags & _F_IN))
		  out_parameters++;
		else
		  in_parameters++;

		/* Handle an argument that is a pointer to a non-char.  But
		   (void*) and (anything**) is not allowed. */
		/* The argument is a pointer to something; increment TYPE
		     so we can see what it is a pointer to. */
		tmptype++;
		/* If the pointer's value is qualified as an IN parameter,
		   or not explicity qualified as an OUT parameter, then
		   decode it. */
		if ((flags & _F_IN) || !(flags & _F_OUT))
		  {
		    datum = alloca (objc_sizeof_type (tmptype));
		    [decoder decodeValueOfObjCType: tmptype at: datum];
		    [inv setArgument: &datum atIndex: argnum];
		  }
		break;

	      default:
		in_parameters++;
		datum = alloca (objc_sizeof_type (tmptype));
		if (*tmptype == _C_ID)
		  {
		    *(id*)datum = [decoder decodeObject];
		  }
		else
		  {
		    [decoder decodeValueOfObjCType: tmptype at: datum];
		  }
		[inv setArgument: datum atIndex: argnum];
	    }
	}

      /* Stop using the decoder.
       */
      tmp = decoder;
      decoder = nil;
      [self _doneInRmc: tmp];

      /* Get the qualifier type of the return value. */
      flags = objc_get_type_qualifiers (encoded_types);
      /* Get the return type; store it our two temporary char*'s. */
      etmptype = objc_skip_type_qualifiers (encoded_types);
      tmptype = objc_skip_type_qualifiers (type);

      if (_C_VOID == *tmptype)
	{
	  is_void = YES;
	}

      /* If this is a oneway void with no out parameters, we don't need to
       * send back any response.
       */
      if (YES == is_void && (flags & _F_ONEWAY) && !out_parameters)
        {
	  is_async = YES;
	}

      NSDebugMLLog(@"RMC", @"RMC %d %s method '%s' on %p(%s)", seq,
	(YES == is_async) ? "async" : "invoke",
	selector ? sel_getName(selector) : "nil",
	target, target ? class_getName([target class]) : "nil");

      /* Invoke the method! */
      [inv invoke];

      if (YES == is_async)
        {
	  tmp = inv;
	  inv = nil;
	  [tmp release];
	  NS_VOIDRETURN;
	}

      /* It is possible that our connection died while the method was
       * being called - in this case we mustn't try to send the result
       * back to the remote application!
       */
      if ([self isValid] == NO)
	{
	  NSDebugMLLog(@"RMC", @"RMC %d invalidated ... no return", seq);
	  tmp = inv;
	  inv = nil;
	  [tmp release];
	  NS_VOIDRETURN;
	}

      /* Encode the return value and pass-by-reference values, if there
	 are any.  This logic must match exactly that in
	 cifframe_build_return(). */
      /* OUT_PARAMETERS should be true here in exactly the same
	 situations as it was true in cifframe_dissect_call(). */

      /* We create a new coder object and encode a flag to
       * say that this is not an exception.
       */
      encoder = [self _newOutRmc: seq generate: 0 reply: NO];
      [encoder encodeValueOfObjCType: @encode(BOOL) at: &is_exception];

      /* Only encode return values if there is a non-void return value,
	 a non-oneway void return value, or if there are values that were
	 passed by reference. */

      if (YES == is_void)
	{
	  if ((flags & _F_ONEWAY) == 0)
	    {
	      int	dummy = 0;

	      [encoder encodeValueOfObjCType: @encode(int) at: (void*)&dummy];
	    }
	  /* No return value to encode; do nothing. */
	}
      else
	{
	  void	*datum;

	  if (*tmptype == _C_PTR)
	    {
	      /* The argument is a pointer to something; increment TYPE
		 so we can see what it is a pointer to. */
	      tmptype++;
	      datum = alloca (objc_sizeof_type (tmptype));
	    }
	  else
	    {
	      datum = alloca (objc_sizeof_type (tmptype));
	    }
	  [inv getReturnValue: datum];
	  [encoder encodeValueOfObjCType: tmptype at: datum];
	}


      /* Encode the values returned by reference.  Note: this logic
	 must match exactly the code in cifframe_build_return(); that
	 function should decode exactly what we encode here. */

      if (out_parameters)
	{
	  /* Step through all the arguments, finding the ones that were
	     passed by reference. */
	  for (tmptype = skip_argspec (tmptype),
		 argnum = 0,
		 etmptype = skip_argspec (etmptype);
	       *tmptype != '\0';
	       tmptype = skip_argspec (tmptype),
		 argnum++,
		 etmptype = skip_argspec (etmptype))
	    {
	      /* Get the type qualifiers, like IN, OUT, INOUT, ONEWAY. */
	      flags = objc_get_type_qualifiers(etmptype);
	      /* Skip over the type qualifiers, so now TYPE is pointing directly
		 at the char corresponding to the argument's type. */
	      tmptype = objc_skip_type_qualifiers (tmptype);

	      /* Decide how, (or whether or not), to encode the argument
		 depending on its FLAGS and TMPTYPE. */
	      if (((flags & _F_OUT) || !(flags & _F_IN))
		&& (*tmptype == _C_PTR || *tmptype == _C_CHARPTR))
		{
		  void	*datum;

		  if (*tmptype == _C_PTR)
		    {
		      /* The argument is a pointer (to a non-char), and the
			 pointer's value is qualified as an OUT parameter, or
			 it not explicitly qualified as an IN parameter, then
			 it is a pass-by-reference argument.*/
		      ++tmptype;
		      [inv getArgument: &datum atIndex: argnum];
		      [encoder encodeValueOfObjCType: tmptype at: datum];
		    }
		  else if (*tmptype == _C_CHARPTR)
		    {
		      datum = alloca (sizeof (char*));
		      [inv getArgument: datum atIndex: argnum];
		      [encoder encodeValueOfObjCType: tmptype at: datum];
		    }
		}
	    }
	}
      tmp = inv;
      inv = nil;
      [tmp release];
      tmp = encoder;
      encoder = nil;
      NSDebugMLLog(@"RMC", @"RMC %d replying with %s and %u out parameters",
	seq, (YES == is_void ? "void result" : "result"), out_parameters);

      [self _sendOutRmc: tmp type: METHOD_REPLY sequence: seq];
    }
  NS_HANDLER
    {
      if (debug_connection > 3)
	NSLog(@"forwarding exception for (%@) - %@", self, localException);

      /* Send the exception back to the client. */
      if (IisValid == YES)
	{
	  BOOL is_exception = YES;

	  NS_DURING
	    {
	      NSPortCoder	*op;

	      if (inv != nil)
		{
		  [inv release];
		}
	      if (decoder != nil)
		{
		  [self _failInRmc: decoder];
		}
	      if (encoder != nil)
		{
		  [self _failOutRmc: encoder];
		}
	      op = [self _newOutRmc: seq generate: 0 reply: NO];
	      [op encodeValueOfObjCType: @encode(BOOL)
				     at: &is_exception];
	      [op encodeBycopyObject: localException];
	      [self _sendOutRmc: op type: METHOD_REPLY sequence: seq];
	    }
	  NS_HANDLER
	    {
	      NSLog(@"Exception when sending exception back to client - %@",
		localException);
	    }
	  NS_ENDHANDLER;
	}
    }
  NS_ENDHANDLER;
}

- (void) _service_rootObject: (NSPortCoder*)rmc
{
  id		rootObject = rootObjectForInPort(IreceivePort);
  int		sequence;
  NSPortCoder	*op;

  NSParameterAssert(IreceivePort);
  NSParameterAssert(IisValid);
  NSParameterAssert([rmc connection] == self);

  [rmc decodeValueOfObjCType: @encode(int) at: &sequence];
  [self _doneInRmc: rmc];
  op = [self _newOutRmc: sequence generate: 0 reply: NO];
  [op encodeObject: rootObject];
  [self _sendOutRmc: op type: ROOTPROXY_REPLY sequence: sequence];
}

- (void) _service_release: (NSPortCoder*)rmc
{
  unsigned int	count;
  unsigned int	pos;
  int		sequence;

  NSParameterAssert (IisValid);

  [rmc decodeValueOfObjCType: @encode(int) at: &sequence];
  [rmc decodeValueOfObjCType: @encode(__typeof__(count)) at: &count];

  for (pos = 0; pos < count; pos++)
    {
      unsigned		target;
      NSDistantObject	*prox;

      [rmc decodeValueOfObjCType: @encode(__typeof__(target)) at: &target];

      prox = [self includesLocalTarget: target];
      if (prox != 0)
	{
	  if (debug_connection > 3)
	    NSLog(@"releasing object with target (0x%x) on (%@) counter %d",
		target, self, prox->_counter);
	  GS_M_LOCK(IrefGate);
	  NS_DURING
	    {
	      if (--(prox->_counter) == 0)
		{
		  id	rootObject = rootObjectForInPort(IreceivePort);

		  if (rootObject == prox->_object)
		    {
		      /* Don't deallocate root object ...
		       */
		      prox->_counter = 0;
		    }
		  else
		    {
		      [self removeLocalObject: (id)prox];
		    }
		}
	    }
	  NS_HANDLER
	    {
	      GSM_UNLOCK(IrefGate);
	      [localException raise];
	    }
	  NS_ENDHANDLER
	  GSM_UNLOCK(IrefGate);
	}
      else if (debug_connection > 3)
	NSLog(@"releasing object with target (0x%x) on (%@) - nothing to do",
		target, self);
    }
  [self _doneInRmc: rmc];
}

- (void) _service_retain: (NSPortCoder*)rmc
{
  unsigned		target;
  NSPortCoder		*op;
  int			sequence;
  NSDistantObject	*local;
  NSString		*response = nil;

  NSParameterAssert (IisValid);

  [rmc decodeValueOfObjCType: @encode(int) at: &sequence];
  op = [self _newOutRmc: sequence generate: 0 reply: NO];

  [rmc decodeValueOfObjCType: @encode(__typeof__(target)) at: &target];
  [self _doneInRmc: rmc];

  if (debug_connection > 3)
    NSLog(@"looking to retain local object with target (0x%x) on (%@)",
      target, self);

  GS_M_LOCK(IrefGate);
  local = [self locateLocalTarget: target];
  if (local == nil)
    {
      response = @"target not found anywhere";
    }
  else
    {
      local->_counter++;	// Vended on connection.
    }
  GSM_UNLOCK(IrefGate);

  [op encodeObject: response];
  [self _sendOutRmc: op type: RETAIN_REPLY sequence: sequence];
}

- (void) _shutdown
{
  NSParameterAssert(IreceivePort);
  NSParameterAssert (IisValid);
  NS_DURING
    {
      NSPortCoder	*op;
      int		sno;

      op = [self _newOutRmc: 0 generate: &sno reply: NO];
      [self _sendOutRmc: op type: CONNECTION_SHUTDOWN sequence: sno];
    }
  NS_HANDLER
  NS_ENDHANDLER
}

- (void) _service_shutdown: (NSPortCoder*)rmc
{
  NSParameterAssert (IisValid);
  IshuttingDown = YES;		// Prevent shutdown being sent back to other end
  [self _doneInRmc: rmc];
  [self invalidate];
}

- (void) _service_typeForSelector: (NSPortCoder*)rmc
{
  NSPortCoder	*op;
  unsigned	target;
  NSDistantObject *p;
  int		sequence;
  id		o;
  SEL		sel;
  const char	*type;
  struct objc_method* m;

  NSParameterAssert(IreceivePort);
  NSParameterAssert (IisValid);

  [rmc decodeValueOfObjCType: @encode(int) at: &sequence];
  op = [self _newOutRmc: sequence generate: 0 reply: NO];

  [rmc decodeValueOfObjCType: ":" at: &sel];
  [rmc decodeValueOfObjCType: @encode(unsigned) at: &target];
  [self _doneInRmc: rmc];
  p = [self includesLocalTarget: target];
  o = (p != nil) ? p->_object : nil;

  /* xxx We should make sure that TARGET is a valid object. */
  /* Not actually a Proxy, but we avoid the warnings "id" would have made. */
  m = GSGetMethod(object_getClass(o), sel, YES, YES);
  /* Perhaps I need to be more careful in the line above to get the
     version of the method types that has the type qualifiers in it.
     Search the protocols list. */
  if (m)
    type = method_getTypeEncoding(m);
  else
    type = "";
  [op encodeValueOfObjCType: @encode(char*) at: &type];
  [self _sendOutRmc: op type: METHODTYPE_REPLY sequence: sequence];
}



/*
 * Check the queue, then try to get it from the network by waiting
 * while we run the NSRunLoop.  Raise exception if we don't get anything
 * before timing out.
 */
- (NSPortCoder*) _getReplyRmc: (int)sn for: (const char*)request
{
  NSPortCoder		*rmc = nil;
  GSIMapNode		node = 0;
  NSDate		*timeout_date = nil;
  NSTimeInterval	delay_interval = 0.0;
  NSTimeInterval	last_interval;
  NSTimeInterval	maximum_interval;
  NSDate		*delay_date = nil;
  NSDate		*start_date = nil;
  NSRunLoop		*runLoop;
  BOOL			isLocked = NO;

  if (IisValid == NO)
    {
      [NSException raise: NSObjectInaccessibleException
        format: @"Connection has been invalidated for reply %d (%s)",
        sn, request];
    }

  /*
   * If we have sent out a request on a run loop that we don't already
   * know about, it must be on a new thread - so if we have multipleThreads
   * enabled, we must add the run loop of the new thread so that we can
   * get the reply in this thread.
   */
  runLoop = GSRunLoopForThread(nil);
  if ([IrunLoops indexOfObjectIdenticalTo: runLoop] == NSNotFound)
    {
      if (ImultipleThreads == YES)
	{
	  [self addRunLoop: runLoop];
	}
      else
	{
	  [NSException raise: NSObjectInaccessibleException
            format: @"Waiting for reply %d (%s) in wrong thread",
            sn, request];
	}
    }

  if (ImultipleThreads == YES)
    {
      /* Since multiple threads are using this connection, another
       * thread may read the reply we are waiting for - so we must
       * break out of the runloop frequently to check.  We do this
       * by setting a small delay and increasing it each time round
       * so that this semi-busy wait doesn't consume too much
       * processor time (I hope).
       * We set an upper limit on the delay to avoid responsiveness
       * problems.
       */
      last_interval = 0.0001;
      maximum_interval = 1.0;
    }
  else
    {
      /* As the connection is single threaded, we can wait indefinitely
       * for a response ... but we recheck every five minutes anyway.
       */
      last_interval = maximum_interval = 300.0;
    }

  NS_DURING
    {
      BOOL	warned = NO;

      if (debug_connection > 5)
	NSLog(@"Waiting for reply %d (%s) on %@", sn, request, self);
      GS_M_LOCK(IrefGate); isLocked = YES;
      while (IisValid == YES
	&& (node = GSIMapNodeForKey(IreplyMap, (GSIMapKey)(NSUInteger)sn)) != 0
	&& node->value.obj == dummyObject)
	{
	  NSDate	*limit_date;

	  GSM_UNLOCK(IrefGate); isLocked = NO;
	  if (start_date == nil)
	    {
	      start_date = [dateClass allocWithZone: NSDefaultMallocZone()];
	      start_date = [start_date init];
	      timeout_date = [dateClass allocWithZone: NSDefaultMallocZone()];
	      timeout_date
		= [timeout_date initWithTimeIntervalSinceNow: IreplyTimeout];
	    }
	  RELEASE(delay_date);
	  delay_date = [dateClass allocWithZone: NSDefaultMallocZone()];
	  if (delay_interval < maximum_interval)
	    {
	      NSTimeInterval	next_interval = last_interval + delay_interval;

	      last_interval = delay_interval;
	      delay_interval = next_interval;
	    }
	  delay_date
	    = [delay_date initWithTimeIntervalSinceNow: delay_interval];

	  /*
	   * We must not set a delay date that is further in the future
	   * than the timeout date for the response to be returned.
	   */
	  if ([timeout_date earlierDate: delay_date] == timeout_date)
	    {
	      limit_date = timeout_date;
	    }
	  else
	    {
	      limit_date = delay_date;
	    }

	  /*
	   * If the runloop returns without having done anything, AND we
	   * were waiting for the final timeout, then we must break out
	   * of the loop.
	   */
	  if (([runLoop runMode: NSConnectionReplyMode
		     beforeDate: limit_date] == NO
	    && (limit_date == timeout_date))
	    || [timeout_date timeIntervalSinceNow] <= 0.0)
	    {
	      GS_M_LOCK(IrefGate); isLocked = YES;
	      node = GSIMapNodeForKey(IreplyMap, (GSIMapKey)(NSUInteger)sn);
	      break;
	    }
	  else if (warned == NO && [start_date timeIntervalSinceNow] <= -300.0)
	    {
	      warned = YES;
	      NSLog(@"WARNING ... waiting for reply %u (%s) since %@ on %@",
		sn, request, start_date, self);
	    }
	  GS_M_LOCK(IrefGate); isLocked = YES;
	}
      if (node == 0)
	{
	  rmc = nil;
	}
      else
	{
	  rmc = node->value.obj;
	  GSIMapRemoveKey(IreplyMap, (GSIMapKey)(NSUInteger)sn);
	}
      GSM_UNLOCK(IrefGate); isLocked = NO;
      TEST_RELEASE(start_date);
      TEST_RELEASE(delay_date);
      TEST_RELEASE(timeout_date);
      if (rmc == nil)
	{
	  [NSException raise: NSInternalInconsistencyException
            format: @"no reply available %d (%s)",
            sn, request];
	}
      if (rmc == dummyObject)
	{
	  if (IisValid == YES)
	    {
	      [NSException raise: NSPortTimeoutException
                format: @"timed out waiting for reply %d (%s)",
                sn, request];
	    }
	  else
	    {
	      [NSException raise: NSInvalidReceivePortException
                format: @"invalidated while awaiting reply %d (%s)",
                sn, request];
	    }
	}
    }
  NS_HANDLER
    {
      if (isLocked == YES)
	{
	  GSM_UNLOCK(IrefGate);
	}
      [localException raise];
    }
  NS_ENDHANDLER

  NSDebugMLLog(@"NSConnection", @"Consuming reply %d (%s) on %"PRIxPTR,
    sn, request, (NSUInteger)self);
  return rmc;
}

- (void) _doneInReply: (NSPortCoder*)c
{
  [self _doneInRmc: c];
  IrepInCount++;
}

- (void) _doneInRmc: (NSPortCoder*) NS_CONSUMED c
{
  GS_M_LOCK(IrefGate);
  if (debug_connection > 5)
    {
      NSLog(@"done rmc %p", c);
    }
  if (cacheCoders == YES && IcachedDecoders != nil)
    {
      [IcachedDecoders addObject: c];
    }
  [c dispatch];	/* Tell NSPortCoder to release the connection.	*/
  RELEASE(c);
  GSM_UNLOCK(IrefGate);
}

/*
 * This method called if an exception occurred, and we don't know
 * whether we have already tidied the NSPortCoder object up or not.
 */
- (void) _failInRmc: (NSPortCoder*)c
{
  GS_M_LOCK(IrefGate);
  if (cacheCoders == YES && IcachedDecoders != nil
    && [IcachedDecoders indexOfObjectIdenticalTo: c] == NSNotFound)
    {
      [IcachedDecoders addObject: c];
    }
  if (debug_connection > 5)
    {
      NSLog(@"fail rmc %p", c);
    }
  [c dispatch];	/* Tell NSPortCoder to release the connection.	*/
  RELEASE(c);
  GSM_UNLOCK(IrefGate);
}

/*
 * This method called if an exception occurred, and we don't know
 * whether we have already tidied the NSPortCoder object up or not.
 */
- (void) _failOutRmc: (NSPortCoder*)c
{
  GS_M_LOCK(IrefGate);
  if (cacheCoders == YES && IcachedEncoders != nil
    && [IcachedEncoders indexOfObjectIdenticalTo: c] == NSNotFound)
    {
      [IcachedEncoders addObject: c];
    }
  [c dispatch];	/* Tell NSPortCoder to release the connection.	*/
  RELEASE(c);
  GSM_UNLOCK(IrefGate);
}

- (NSPortCoder*) _newInRmc: (NSMutableArray*)components
{
  NSPortCoder	*coder;
  NSUInteger	count;

  NSParameterAssert(IisValid);

  GS_M_LOCK(IrefGate);
  if (cacheCoders == YES && IcachedDecoders != nil
    && (count = [IcachedDecoders count]) > 0)
    {
      coder = RETAIN([IcachedDecoders objectAtIndex: --count]);
      [IcachedDecoders removeObjectAtIndex: count];
    }
  else
    {
      coder = [recvCoderClass allocWithZone: NSDefaultMallocZone()];
    }
  GSM_UNLOCK(IrefGate);

  coder = [coder initWithReceivePort: IreceivePort
			    sendPort: IsendPort
			  components: components];
  return coder;
}

/*
 * Create an NSPortCoder object for encoding an outgoing message or reply.
 *
 * sno		Is the seqence number to encode into the coder.
 * ret		If non-null, generate a new sequence number and return it
 *		here.  Ignore the sequence number passed in sno.
 * rep		If this flag is YES, add a placeholder to the IreplyMap
 *		so we handle an incoming reply for this sequence number.
 */
- (NSPortCoder*) _newOutRmc: (int)sno generate: (int*)ret reply: (BOOL)rep
{
  NSPortCoder	*coder;
  NSUInteger	count;

  NSParameterAssert(IisValid);

  GS_M_LOCK(IrefGate);
  /*
   * Generate a new sequence number if required.
   */
  if (ret != 0)
    {
      sno = ImessageCount++;
      *ret = sno;
    }
  /*
   * Add a placeholder to the reply map if we expect a reply.
   */
  if (rep == YES)
    {
      GSIMapAddPair(IreplyMap,
	(GSIMapKey)(NSUInteger)sno, (GSIMapVal)dummyObject);
    }
  /*
   * Locate or create an rmc
   */
  if (cacheCoders == YES && IcachedEncoders != nil
    && (count = [IcachedEncoders count]) > 0)
    {
      coder = RETAIN([IcachedEncoders objectAtIndex: --count]);
      [IcachedEncoders removeObjectAtIndex: count];
    }
  else
    {
      coder = [sendCoderClass allocWithZone: NSDefaultMallocZone()];
    }
  GSM_UNLOCK(IrefGate);

  coder = [coder initWithReceivePort: IreceivePort
			    sendPort:IsendPort
			  components: nil];
  [coder encodeValueOfObjCType: @encode(int) at: &sno];
  NSDebugMLLog(@"NSConnection",
    @"Make out RMC %u on %@", sno, self);
  return coder;
}

- (void) _sendOutRmc: (NSPortCoder*)c type: (int)msgid sequence: (int)sno
{
  NSDate		*limit;
  BOOL			sent = NO;
  BOOL			raiseException = NO;
  NSMutableArray	*components = [c _components];

  if (IauthenticateOut == YES
    && (msgid == METHOD_REQUEST || msgid == METHOD_REPLY))
    {
      NSData	*d;

      d = [[self delegate] authenticationDataForComponents: components];
      if (d == nil)
	{
	  RELEASE(c);
	  [NSException raise: NSGenericException
		      format: @"Bad authentication data provided by delegate"];
	}
      [components addObject: d];
    }

  switch (msgid)
    {
      case PROXY_RETAIN:
      case CONNECTION_SHUTDOWN:
      case METHOD_REPLY:
      case ROOTPROXY_REPLY:
      case METHODTYPE_REPLY:
      case PROXY_RELEASE:
      case RETAIN_REPLY:
	raiseException = NO;
	break;

      case METHOD_REQUEST:
      case ROOTPROXY_REQUEST:
      case METHODTYPE_REQUEST:
      default:
	raiseException = YES;
	break;
    }

  NSDebugMLLog(@"NSConnection",
    @"Sending %d (%@) on %@", sno, stringFromMsgType(msgid), self);

  limit = [dateClass dateWithTimeIntervalSinceNow: IrequestTimeout];
  sent = [IsendPort sendBeforeDate: limit
			     msgid: msgid
			components: components
			      from: IreceivePort
			  reserved: [IsendPort reservedSpaceLength]];

  GS_M_LOCK(IrefGate);

  /*
   * We replace the coder we have just used in the cache, and tell it not to
   * retain this connection any more.
   */
  if (cacheCoders == YES && IcachedEncoders != nil)
    {
      [IcachedEncoders addObject: c];
    }
  [c dispatch];	/* Tell NSPortCoder to release the connection.	*/
  RELEASE(c);
  GSM_UNLOCK(IrefGate);

  if (sent == NO)
    {
      NSString	*text = stringFromMsgType(msgid);

      if ([IsendPort isValid] == NO)
	{
	  text = [text stringByAppendingFormat: @" - port was invalidated"];
	}
      if (raiseException == YES)
	{
	  [NSException raise: NSPortTimeoutException format: @"%@", text];
	}
      else
	{
	  NSLog(@"Port operation timed out - %@", text);
	}
    }
  else
    {
      switch (msgid)
	{
	  case METHOD_REQUEST:
	    IreqOutCount++;		/* Sent a request.	*/
	    break;
	  case METHOD_REPLY:
	    IrepOutCount++;		/* Sent back a reply. */
	    break;
	  default:
	    break;
	}
    }
}



/* Managing objects and proxies. */
- (void) addLocalObject: (NSDistantObject*)anObj
{
  static unsigned	local_object_counter = 0;
  id			object;
  unsigned		target;
  GSIMapNode    	node;

  GS_M_LOCK(IrefGate);
  NSParameterAssert (IisValid);

  object = anObj->_object;
  target = anObj->_handle;

  /*
   * If there is no target allocated to the proxy, we add one.
   */
  if (target == 0)
    {
      anObj->_handle = target = ++local_object_counter;
    }

  /*
   * Record the value in the IlocalObjects map, retaining it.
   */
  node = GSIMapNodeForKey(IlocalObjects, (GSIMapKey)object);
  NSAssert(node == 0, NSInternalInconsistencyException);
  node = GSIMapNodeForKey(IlocalTargets, (GSIMapKey)(NSUInteger)target);
  NSAssert(node == 0, NSInternalInconsistencyException);

  IF_NO_GC([anObj retain];)
  GSIMapAddPair(IlocalObjects, (GSIMapKey)object, (GSIMapVal)((id)anObj));
  GSIMapAddPair(IlocalTargets,
    (GSIMapKey)(NSUInteger)target, (GSIMapVal)((id)anObj));

  if (debug_connection > 2)
    NSLog(@"add local object (%p) target (0x%x) "
	  @"to connection (%@)", object, target, self);

  GSM_UNLOCK(IrefGate);
}

- (NSDistantObject*) retainOrAddLocal: (NSDistantObject*)proxy
			    forObject: (id)object
{
  GSIMapNode		node;
  NSDistantObject	*p;

  /* Don't assert (IisValid); */
  GS_M_LOCK(IrefGate);
  node = GSIMapNodeForKey(IlocalObjects, (GSIMapKey)object);
  if (node == 0)
    {
      p = nil;
    }
  else
    {
      p = RETAIN(node->value.obj);
      DESTROY(proxy);
    }
  if (p == nil && proxy != nil)
    {
      p = proxy;
      [self addLocalObject: p];
    }
  GSM_UNLOCK(IrefGate);
  return p;
}

- (void) removeLocalObject: (NSDistantObject*)prox
{
  id		anObj;
  unsigned	target;
  unsigned	val = 0;
  GSIMapNode	node;

  GS_M_LOCK(IrefGate);
  anObj = prox->_object;
  node = GSIMapNodeForKey(IlocalObjects, (GSIMapKey)anObj);

  /*
   * The NSDistantObject concerned may not belong to this connection,
   * so we need to check that any matching proxy is identical to the
   * argument we were given.
   */
  if (node != 0 && node->value.obj == prox)
    {
      target = prox->_handle;

      /*
       * If this proxy has been vended onwards to another process
       * which has not myet released it, we need to keep a reference
       * to the local object around for a while in case that other
       * process needs it.
       */
      if ((prox->_counter) != 0)
	{
	  CachedLocalObject	*item;

	  (prox->_counter) = 0;
	  GS_M_LOCK(cached_proxies_gate);
	  if (timer == nil)
	    {
	      timer = [NSTimer scheduledTimerWithTimeInterval: 1.0
		target: connectionClass
		selector: @selector(_timeout:)
		userInfo: nil
		repeats: YES];
	    }
	  item = [CachedLocalObject newWithObject: prox time: 5];
	  NSMapInsert(targetToCached, (void*)(uintptr_t)target, item);
	  GSM_UNLOCK(cached_proxies_gate);
	  RELEASE(item);
	  if (debug_connection > 3)
	    NSLog(@"placed local object (%p) target (0x%x) in cache",
			anObj, target);
	}

      /*
       * Remove the proxy from IlocalObjects and release it.
       */
      GSIMapRemoveKey(IlocalObjects, (GSIMapKey)anObj);
      RELEASE(prox);

      /*
       * Remove the target info too - no release required.
       */
      GSIMapRemoveKey(IlocalTargets, (GSIMapKey)(NSUInteger)target);

      if (debug_connection > 2)
	NSLog(@"removed local object (%p) target (0x%x) "
	  @"from connection (%@) (ref %d)", anObj, target, self, val);
    }
  GSM_UNLOCK(IrefGate);
}

- (void) _release_target: (unsigned)target count: (unsigned)number
{
  NS_DURING
    {
      /*
       *	Tell the remote app that it can release its local objects
       *	for the targets in the specified list since we don't have
       *	proxies for them any more.
       */
      if (IreceivePort != nil && IisValid == YES && number > 0)
	{
	  id		op;
	  unsigned 	i;
	  int		sequence;

	  op = [self _newOutRmc: 0 generate: &sequence reply: NO];

	  [op encodeValueOfObjCType: @encode(unsigned) at: &number];

	  for (i = 0; i < number; i++)
	    {
	      [op encodeValueOfObjCType: @encode(unsigned) at: &target];
	      if (debug_connection > 3)
		NSLog(@"sending release for target (0x%x) on (%@)",
		  target, self);
	    }

	  [self _sendOutRmc: op type: PROXY_RELEASE sequence: sequence];
	}
    }
  NS_HANDLER
    {
      if (debug_connection)
        NSLog(@"failed to release targets - %@", localException);
    }
  NS_ENDHANDLER
}

- (NSDistantObject*) locateLocalTarget: (unsigned)target
{
  NSDistantObject	*proxy = nil;
  GSIMapNode		node;

  GS_M_LOCK(IrefGate);

  /*
   * Try a quick lookup to see if the target references a local object
   * belonging to the receiver ... usually it should.
   */
  node = GSIMapNodeForKey(IlocalTargets, (GSIMapKey)(NSUInteger)target);
  if (node != 0)
    {
      proxy = node->value.obj;
    }

  /*
   * If the target doesn't exist in the receiver, but still
   * persists in the cache (ie it was recently released) then
   * we move it back from the cache to the receiver.
   */
  if (proxy == nil)
    {
      CachedLocalObject	*cached;

      GS_M_LOCK(cached_proxies_gate);
      cached = NSMapGet (targetToCached, (void*)(uintptr_t)target);
      if (cached != nil)
	{
	  proxy = [cached obj];
	  /*
	   * Found in cache ... add to this connection as the object
	   * is no longer in use by any connection.
	   */
	  ASSIGN(proxy->_connection, self);
	  [self addLocalObject: proxy];
	  NSMapRemove(targetToCached, (void*)(uintptr_t)target);
	  if (debug_connection > 3)
	    NSLog(@"target (0x%x) moved from cache", target);
	}
      GSM_UNLOCK(cached_proxies_gate);
    }

  /*
   * If not found in the current connection or the cache of local references
   * of recently invalidated connections, try all other existing connections.
   */
  if (proxy == nil)
    {
      NSHashEnumerator	enumerator;
      NSConnection	*c;

      GS_M_LOCK(connection_table_gate);
      enumerator = NSEnumerateHashTable(connection_table);
      while (proxy == nil
	&& (c = (NSConnection*)NSNextHashEnumeratorItem(&enumerator)) != nil)
	{
	  if (c != self && [c isValid] == YES)
	    {
	      GS_M_LOCK(GSIVar(c, _refGate));
	      node = GSIMapNodeForKey(GSIVar(c, _localTargets),
		(GSIMapKey)(NSUInteger)target);
	      if (node != 0)
		{
		  id		local;
		  unsigned	nTarget;

		  /*
		   * We found the local object in use in another connection
		   * so we create a new reference to the same object and
		   * add it to our connection, adjusting the target of the
		   * new reference to be the value we need.
		   *
		   * We don't want to just share the NSDistantObject with
		   * the other connection, since we might want to keep
		   * track of information on a per-connection basis in
		   * order to handle connection shutdown cleanly.
		   */
		  proxy = node->value.obj;
		  local = RETAIN(proxy->_object);
		  proxy = [NSDistantObject proxyWithLocal: local
					       connection: self];
		  nTarget = proxy->_handle;
		  GSIMapRemoveKey(IlocalTargets,
		    (GSIMapKey)(NSUInteger)nTarget);
		  proxy->_handle = target;
		  GSIMapAddPair(IlocalTargets,
		    (GSIMapKey)(NSUInteger)target, (GSIMapVal)((id)proxy));
		}
	      GSM_UNLOCK(GSIVar(c, _refGate));
	    }
	}
      NSEndHashTableEnumeration(&enumerator);
      GSM_UNLOCK(connection_table_gate);
    }

  GSM_UNLOCK(IrefGate);

  if (proxy == nil)
    {
      if (debug_connection > 3)
	NSLog(@"target (0x%x) not found anywhere", target);
    }
  return proxy;
}

- (void) vendLocal: (NSDistantObject*)aProxy
{
  GS_M_LOCK(IrefGate);
  aProxy->_counter++;
  GSM_UNLOCK(IrefGate);
}

- (void) acquireProxyForTarget: (unsigned)target
{
  NSDistantObject	*found;
  GSIMapNode		node;

  /* Don't assert (IisValid); */
  GS_M_LOCK(IrefGate);
  node = GSIMapNodeForKey(IremoteProxies, (GSIMapKey)(NSUInteger)target);
  if (node == 0)
    {
      found = nil;
    }
  else
    {
      found = node->value.obj;
    }
  GSM_UNLOCK(IrefGate);
  if (found == nil)
    {
      NS_DURING
	{
	  /*
	   * Tell the remote app that it must retain the local object
	   * for the target on this connection.
	   */
	  if (IreceivePort && IisValid)
	    {
	      NSPortCoder	*op;
	      id	ip;
	      id	result;
	      int	seq_num;

	      op = [self _newOutRmc: 0 generate: &seq_num reply: YES];
	      [op encodeValueOfObjCType: @encode(__typeof__(target)) at: &target];
	      [self _sendOutRmc: op type: PROXY_RETAIN sequence: seq_num];

	      ip = [self _getReplyRmc: seq_num for: "retain"];
	      [ip decodeValueOfObjCType: @encode(id) at: &result];
	      [self _doneInRmc: ip];
	      if (result != nil)
		NSLog(@"failed to retain target - %@", result);
	      else if (debug_connection > 3)
		NSLog(@"sending retain for target - %u", target);
	    }
	}
      NS_HANDLER
	{
	  NSLog(@"failed to retain target - %@", localException);
	}
      NS_ENDHANDLER
    }
}

- (id) retain
{
  return [super retain];
}

- (void) removeProxy: (NSDistantObject*)aProxy
{
  GS_M_LOCK(IrefGate);
  if (IisValid == YES)
    {
      unsigned		target;
      unsigned		count = 1;
      GSIMapNode	node;

      target = aProxy->_handle;
      node = GSIMapNodeForKey(IremoteProxies, (GSIMapKey)(NSUInteger)target);

      /*
       * Only remove if the proxy for the target is the same as the
       * supplied argument.
       */
      if (node != 0 && node->value.obj == aProxy)
	{
	  count = aProxy->_counter;
	  GSIMapRemoveKey(IremoteProxies, (GSIMapKey)(NSUInteger)target);
	  /*
	   * Tell the remote application that we have removed our proxy and
	   * it can release it's local object.
	   */
	  [self _release_target: target count: count];
	}
    }
  GSM_UNLOCK(IrefGate);
}


/**
 * Private method used only when a remote process/thread has sent us a
 * target which we are decoding into a proxy in this process/thread.
 * <p>The argument aProxy may be nil, in which case an existing proxy
 * matching aTarget is retrieved retained, and returned (this is done
 * when a proxy target is sent to us by a remote process).
 * </p>
 * <p>If aProxy is not nil, but a proxy with the same target already
 * exists, then aProxy is released and the existing proxy is returned
 * as in the case where aProxy was nil.
 * </p>
 * <p>If aProxy is not nil and there was no prior proxy with the same
 * target, aProxy is added to the receiver and returned.
 * </p>
 */
- (NSDistantObject*) retainOrAddProxy: (NSDistantObject*)aProxy
			    forTarget: (unsigned)aTarget
{
  NSDistantObject	*p;
  GSIMapNode		node;

  /* Don't assert (IisValid); */
  NSParameterAssert(aTarget > 0);
  NSParameterAssert(aProxy == nil
    || object_getClass(aProxy) == distantObjectClass);
  NSParameterAssert(aProxy == nil || [aProxy connectionForProxy] == self);
  NSParameterAssert(aProxy == nil || aTarget == aProxy->_handle);

  GS_M_LOCK(IrefGate);
  node = GSIMapNodeForKey(IremoteProxies, (GSIMapKey)(NSUInteger)aTarget);
  if (node == 0)
    {
      p = nil;
    }
  else
    {
      p = RETAIN(node->value.obj);
      DESTROY(aProxy);
    }
  if (p == nil && aProxy != nil)
    {
      p = aProxy;
      GSIMapAddPair(IremoteProxies, (GSIMapKey)(NSUInteger)aTarget, (GSIMapVal)((id)p));
    }
  /*
   * Whether this is a new proxy or an existing proxy, this method is
   * only called for an object being vended by a remote process/thread.
   * We therefore need to increment the count of the number of times
   * the proxy has been vended.
   */
  if (p != nil)
    {
      p->_counter++;
    }
  GSM_UNLOCK(IrefGate);
  return p;
}

- (id) includesLocalObject: (id)anObj
{
  NSDistantObject	*ret;
  GSIMapNode		node;

  /* Don't assert (IisValid); */
  GS_M_LOCK(IrefGate);
  node = GSIMapNodeForKey(IlocalObjects, (GSIMapKey)(NSUInteger)anObj);
  if (node == 0)
    {
      ret = nil;
    }
  else
    {
      ret = node->value.obj;
    }
  GSM_UNLOCK(IrefGate);
  return ret;
}

- (NSDistantObject*) includesLocalTarget: (unsigned)target
{
  NSDistantObject	*ret;
  GSIMapNode		node;

  /* Don't assert (IisValid); */
  GS_M_LOCK(IrefGate);
  node = GSIMapNodeForKey(IlocalTargets, (GSIMapKey)(NSUInteger)target);
  if (node == 0)
    {
      ret = nil;
    }
  else
    {
      ret = node->value.obj;
    }
  GSM_UNLOCK(IrefGate);
  return ret;
}

/* Prevent trying to encode the connection itself */

- (void) encodeWithCoder: (NSCoder*)anEncoder
{
  [self shouldNotImplement: _cmd];
}
- (id) initWithCoder: (NSCoder*)aDecoder;
{
  [self shouldNotImplement: _cmd];
  return self;
}

/*
 *	We register this method for a notification when a port dies.
 *	NB. It is possible that the death of a port could be notified
 *	to us after we are invalidated - in which case we must ignore it.
 */
- (void) _portIsInvalid: (NSNotification*)notification
{
  if (IisValid)
    {
      id port = [notification object];

      if (debug_connection)
	{
	  NSLog(@"Received port invalidation notification for "
	      @"connection %@\n\t%@", self, port);
	}

      /* We shouldn't be getting any port invalidation notifications,
	  except from our own ports; this is how we registered ourselves
	  with the NSNotificationCenter in
	  +newForInPort: outPort: ancestorConnection. */
      NSParameterAssert (port == IreceivePort || port == IsendPort);

      [self invalidate];
    }
}

/**
 * On thread exit, we need all connections to be removed from the runloop
 * of the thread or they will retain that and cause a memory leak.
 */
+ (void) _threadWillExit: (NSNotification*)notification
{
  NSRunLoop *runLoop = GSRunLoopForThread ([notification object]);

  if (runLoop != nil)
    {
      NSEnumerator	*enumerator;
      NSConnection	*c;

      GS_M_LOCK (connection_table_gate);
      enumerator = [NSAllHashTableObjects(connection_table) objectEnumerator];
      GSM_UNLOCK (connection_table_gate);

      /*
       * We enumerate an array copy of the contents of the hash table
       * as we know we can do that safely outside the locked region.
       * The temporary array and the enumerator are autoreleased and
       * will be deallocated with the threads autorelease pool.
       */
      while ((c = [enumerator nextObject]) != nil)
	{
	  [c removeRunLoop: runLoop];
	}
    }
}
@end
