/** Implementation for GNU Objective-C version of NSDistantObject
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Based on code by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Created: August 1997

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

   <title>NSDistantObject class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#define	EXPOSE_NSDistantObject_IVARS	1
#import "GNUstepBase/DistributedObjects.h"
#import "GNUstepBase/GSObjCRuntime.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSPort.h"
#import "Foundation/NSMethodSignature.h"
#import "Foundation/NSException.h"
#import "Foundation/NSHashTable.h"
#import "Foundation/NSInvocation.h"
#include <objc/Protocol.h>
#import "GSInvocation.h"


@interface NSDistantObject(GNUstepExtensions)
- (Class) classForPortCoder;
- (void) finalize;
@end

#define DO_FORWARD_INVOCATION(_SELX, _ARG1) ({			\
  sig = [self methodSignatureForSelector: @selector(_SELX)];	\
  if (sig != nil)						\
  {								\
    inv = [NSInvocation invocationWithMethodSignature: sig];	\
    [inv setSelector: @selector(_SELX)];			\
    [inv setTarget: self];					\
    [inv setArgument: (void*)&_ARG1 atIndex: 2];		\
    [self forwardInvocation: inv];				\
    [inv getReturnValue: &m];					\
  }								\
  else								\
  {								\
    NSWarnLog(@"DO_FORWARD_INVOCATION failed, got nil signature for '%@'.", \
      NSStringFromSelector(@selector(_SELX)));			\
  }})


static int	debug_proxy = 0;
static Class	placeHolder = 0;
static Class	distantObjectClass = 0;

#ifndef __GNUSTEP_RUNTIME__
@interface Object (NSConformsToProtocolNamed)
- (BOOL) _conformsToProtocolNamed: (const char*)aName;
@end
#endif
@interface NSObject (NSConformsToProtocolNamed)
- (BOOL) _conformsToProtocolNamed: (const char*)aName;
@end
/*
 * Evil hack ... if a remote system wants to know if we conform
 * to a protocol we use a local protocol with the same name.
 */
#ifndef __GNUSTEP_RUNTIME__
@interface Object (conformsTo)
- (BOOL) conformsTo: (Protocol*)p;
@end
@implementation Object (NSConformsToProtocolNamed)
- (BOOL) _conformsToProtocolNamed: (const char*)aName
{
  Protocol	*p;

  p = objc_getProtocol(aName);
  return [(id)self conformsTo: p];
}
@end
#endif
@implementation NSObject (NSConformsToProtocolNamed)
- (BOOL) _conformsToProtocolNamed: (const char*)aName
{
  Protocol	*p;

  p = objc_getProtocol(aName);
  return [self conformsToProtocol: p];
}
@end

@interface NSConnection (DistantObjectHacks)
- (void) acquireProxyForTarget: (unsigned)target;
- (NSDistantObject*) retainOrAddLocal: (NSDistantObject*)aProxy
			    forObject: (id)anObject;
- (NSDistantObject*) retainOrAddProxy: (NSDistantObject*)aProxy
			    forTarget: (unsigned)aTarget;
- (void) removeProxy: (id)obj;
- (void) vendLocal: (NSDistantObject*)aProxy;
@end

/* This is the proxy tag; it indicates where the local object is,
   and determines whether the reply port to the Connection-where-the-
   proxy-is-local needs to encoded/decoded or not. */
enum proxyLocation
{
  PROXY_LOCAL_FOR_RECEIVER = 0,
  PROXY_LOCAL_FOR_SENDER,
  PROXY_REMOTE_FOR_BOTH
};



/*
 *	The GSDistantObjectPlaceHolder class is simply used as a placeholder
 *	for an NSDistantObject so we can manage efficient allocation and
 *	initialisation - in most cases when we ask for an NSDistantObject
 *	instance, we will get a pre-existing one, so we don't want to go
 *	allocating the memory for a new instance unless absolutely necessary.
 */
GS_ROOT_CLASS @interface	GSDistantObjectPlaceHolder
+ (id) initWithCoder: (NSCoder*)aCoder;
+ (id) initWithLocal: (id)anObject connection: (NSConnection*)aConnection;
+ (id) initWithTarget: (unsigned)target connection: (NSConnection*)aConnection;
+ (id) autorelease;
+ (void) release;
+ (id) retain;
+ (BOOL) respondsToSelector: (SEL)sel;
@end

@implementation	GSDistantObjectPlaceHolder

+ (id) autorelease
{
  return self;
}

+ (void) release
{
}

+ (id) retain
{
  return self;
}

+ (void) initialize
{
  if (self == objc_lookUpClass("GSDistantObjectPlaceHolder"))
    {
      distantObjectClass = [NSDistantObject class];
    }
}

+ (BOOL) respondsToSelector: (SEL)sel
{
  return class_getClassMethod(self, sel) != NULL;
}

+ (id) initWithCoder: (NSCoder*)aCoder
{
  uint8_t		proxy_tag;
  unsigned		target;
  id			decoder_connection;
  NSDistantObject	*o;

/*
  if ([aCoder isKindOfClass: [NSPortCoder class]] == NO)
    {
      [NSException raise: NSGenericException
		  format: @"NSDistantObject objects only decode with "
			  @"NSPortCoder class"];
    }
*/

  decoder_connection = [(NSPortCoder*)aCoder connection];
  NSAssert(decoder_connection, NSInternalInconsistencyException);

  /* First get the tag, so we know what values need to be decoded. */
  [aCoder decodeValueOfObjCType: @encode(__typeof__(proxy_tag))
			     at: &proxy_tag];

  switch (proxy_tag)
    {
      case PROXY_LOCAL_FOR_RECEIVER:
	/*
	 *	This was a proxy on the other side of the connection, but
	 *	here it's local.
	 *	Lookup the target handle to ensure that it exists here.
	 *	Return a retained copy of the local target object.
	 */
	[aCoder decodeValueOfObjCType: @encode(__typeof__(target))
				   at: &target];

        if (debug_proxy)
	  NSLog(@"Receiving a proxy for local object 0x%x "
		@"connection %p\n", target, decoder_connection);

	o = [decoder_connection locateLocalTarget: target];
        if (o == nil)
	  {
	    [NSException raise: @"ProxyDecodedBadTarget"
			format: @"No local object with given target (0x%x)",
				target];
	  }
	else
	  {
	    if (debug_proxy)
	      {
		NSLog(@"Local object is %p (%p)\n",
		  (void*)(uintptr_t)o, o->_object);
	      }
	    return RETAIN(o->_object);
	  }

      case PROXY_LOCAL_FOR_SENDER:
        /*
	 *	This was a local object on the other side of the connection,
	 *	but here it's a proxy object.  Get the target address, and
	 *	send [NSDistantObject +proxyWithTarget:connection:]; this will
	 *	return the proxy object we already created for this target, or
	 *	create a new proxy object if necessary.
	 */
	[aCoder decodeValueOfObjCType: @encode(__typeof__(target))
				   at: &target];
	if (debug_proxy)
	  NSLog(@"Receiving a proxy, was local 0x%x connection %p\n",
		  target, decoder_connection);
	o = [self initWithTarget: target
		      connection: decoder_connection];
	return o;

      case PROXY_REMOTE_FOR_BOTH:
        /*
	 *	This was a proxy on the other side of the connection, and it
	 *	will be a proxy on this side too; that is, the local version
	 *	of this object is not on this host, not on the host the
	 *	NSPortCoder is connected to, but on a *third* host.
	 *	This is why I call this a "triangle connection".  In addition
	 *	to decoding the target, we decode the NSPort object that we
	 *	will use to talk directly to this third host.  We send
	 *	[NSConnection +connectionWithReceivePort:sendPort:]; this
	 *	will either return the connection already created for this
	 *	inPort/outPort pair, or create a new connection if necessary.
	 */
	{
	  NSConnection		*proxy_connection;
	  NSPort		*proxy_connection_out_port = nil;
	  unsigned		intermediary;

	  /*
	   *	There is an object on the intermediary host that is keeping
	   *	that hosts proxy for the original object retained, thus
	   *	ensuring that the original is not released.  We create a
	   *	proxy for that intermediate proxy.  When we release this
	   *	proxy, the intermediary will be free to release it's proxy
	   *	and the original can then be released.  Of course, by that
	   *	time we will have obtained our own proxy for the original
	   *	object ...
	   */
	  [aCoder decodeValueOfObjCType: @encode(__typeof__(intermediary))
				     at: &intermediary];
	  AUTORELEASE([self initWithTarget: intermediary
				connection: decoder_connection]);

	  /*
	   *	Now we get the target number and port for the orignal object
	   *	and (if necessary) get the originating process to retain the
	   *	object for us.
	   */
	  [aCoder decodeValueOfObjCType: @encode(__typeof__(target))
				     at: &target];

	  [aCoder decodeValueOfObjCType: @encode(id)
				     at: &proxy_connection_out_port];

	  NSAssert(proxy_connection_out_port, NSInternalInconsistencyException);
	  /*
	   #	If there already exists a connection for talking to the
	   *	out port, we use that one rather than creating a new one from
	   *	our listening port.
	   *
	   *	First we try for a connection from our receive port,
	   *	Then we try any connection to the send port
	   *	Finally we resort to creating a new connection - we don't
	   *	release the newly created connection - it will get released
	   *	automatically when no proxies are left on it.
	   */
	  proxy_connection = [[decoder_connection class]
	    connectionWithReceivePort: [decoder_connection receivePort]
			     sendPort: proxy_connection_out_port];

	  if (debug_proxy)
	    NSLog(@"Receiving a triangle-connection proxy 0x%x "
		  @"connection %p\n", target, proxy_connection);

	  NSAssert(proxy_connection != decoder_connection,
	    NSInternalInconsistencyException);
	  NSAssert([proxy_connection isValid],
	    NSInternalInconsistencyException);

	  /*
	   * We may not already have a proxy for the object on the
	   * remote system, we must tell the connection to make sure
	   * the other end knows we are creating one.
	   */
	  [proxy_connection acquireProxyForTarget: target];

	  /*
	   *	Finally - we get a proxy via a direct connection to the
	   *	originating server.  We have also created a proxy to an
	   *	intermediate object - but this proxy has not been retained
	   *	and will therefore go away when the current autorelease
	   *	pool is destroyed.
	   */
	  o = [self initWithTarget: target
			connection: proxy_connection];
	  return o;
        }

    default:
      /* xxx This should be something different than NSGenericException. */
      [NSException raise: NSGenericException
		  format: @"Bad proxy tag"];
    }
  /* Not reached. */
  return nil;
}

+ (id) initWithLocal: (id)anObject connection: (NSConnection*)aConnection
{
  NSDistantObject	*proxy;

  NSAssert([aConnection isValid], NSInternalInconsistencyException);

  /*
   *	If there already is a local proxy for this target/connection
   *	combination, don't create a new one, just return the old one.
   */
  proxy = [aConnection retainOrAddLocal: nil forObject: anObject];
  if (proxy == nil)
    {
      proxy = (NSDistantObject*)NSAllocateObject(distantObjectClass,
	0, NSDefaultMallocZone());
      proxy = [proxy initWithLocal: anObject connection: aConnection];
    }
  return proxy;
}

+ (id) initWithTarget: (unsigned)target connection: (NSConnection*)aConnection
{
  NSDistantObject	*proxy;

  NSAssert([aConnection isValid], NSInternalInconsistencyException);

  /*
   * If there already is a local proxy for this target/connection
   * combination, don't create a new one, just return the old one.
   */
  proxy = [aConnection retainOrAddProxy: nil forTarget: target];
  if (proxy == nil)
    {
      proxy = (NSDistantObject*)NSAllocateObject(distantObjectClass,
	0, NSDefaultMallocZone());
      proxy = [proxy initWithTarget: target connection: aConnection];
    }
  return proxy;
}
@end

@interface NSDistantObject (Debug)
+ (int) setDebug: (int)val;
@end

@implementation NSDistantObject (Debug)
+ (int) setDebug: (int)val
{
  int   old = debug_proxy;

  debug_proxy = val;
  return old;
}
@end

/**
 * Instances of this class act as proxies to remote objects using
 * the Distributed Objects system.  They also hold references to
 * local objects which are vended to remote processes.
 */
@implementation NSDistantObject

+ (void) initialize
{
  if (self == [NSDistantObject class])
    {
      placeHolder = objc_lookUpClass("GSDistantObjectPlaceHolder");
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  return (NSDistantObject*)placeHolder;
}

/**
 * Creates and returns a proxy associated with aConnection
 * which will hold a reference to the local object anObject.
 */
+ (NSDistantObject*) proxyWithLocal: (id)anObject
			 connection: (NSConnection*)aConnection
{
  return AUTORELEASE([placeHolder initWithLocal: anObject
				     connection: aConnection]);
}

/**
 * Creates and returns a proxy associated with aConnection
 * which will provide a link to a remote object whose reference
 * locally is anObject.
 */
+ (NSDistantObject*) proxyWithTarget: (unsigned)anObject
			  connection: (NSConnection*)aConnection
{
  return AUTORELEASE([placeHolder initWithTarget: anObject
				      connection: aConnection]);
}

/**
 * Returns the [NSConnection] instance with which the receiver is
 * associated.
 */
- (NSConnection*) connectionForProxy
{
  return _connection;
}
/**
 * NSProxy subclasses must override -init or an exception will be thrown.  This
 * calls the forwarding mechanism to invoke -init on the remote object.
 */
- (id) init
{
  NSMethodSignature	*sig;
  NSInvocation		*inv;
  id			ret;

  sig = [self methodSignatureForSelector: _cmd];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setTarget: self];
  [inv setSelector: _cmd];
  [self forwardInvocation: inv];
  [inv getReturnValue: &ret];
  return ret;
}

- (void) dealloc
{
  [self finalize];
  if (_sigs != 0) [(NSMutableDictionary*)_sigs release];
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder*)aRmc
{
  unsigned	proxy_target;
  uint8_t	proxy_tag;
  NSConnection	*encoder_connection;

/*
  if ([aRmc isKindOfClass: [NSPortCoder class]] == NO)
    {
      [NSException raise: NSGenericException
		  format: @"NSDistantObject objects only "
			  @"encode with NSPortCoder class"];
    }
*/

  encoder_connection = [(NSPortCoder*)aRmc connection];
  NSAssert(encoder_connection, NSInternalInconsistencyException);
  if (![encoder_connection isValid])
    [NSException
	    raise: NSGenericException
	   format: @"Trying to encode to an invalid Connection.\n"
      @"You should request NSConnectionDidDieNotification's and\n"
      @"release all references to the proxy's of invalid Connections."];

  proxy_target = _handle;

  if (encoder_connection == _connection)
    {
      if (_object)
	{
	  /*
	   *	This proxy is a local to us, remote to other side.
	   */
	  proxy_tag = PROXY_LOCAL_FOR_SENDER;

	  if (debug_proxy)
	    NSLog(@"Sending a proxy, will be remote 0x%x connection %p\n",
			proxy_target, _connection);

	  [aRmc encodeValueOfObjCType: @encode(__typeof__(proxy_tag))
				   at: &proxy_tag];

	  [aRmc encodeValueOfObjCType: @encode(__typeof__(proxy_target))
				   at: &proxy_target];
	  /*
	   * Tell connection this object is being vended.
	   */
	  [_connection vendLocal: self];
	}
      else
	{
	  /*
	   *	This proxy is a local object on the other side.
	   */
	  proxy_tag = PROXY_LOCAL_FOR_RECEIVER;

	  if (debug_proxy)
	    NSLog(@"Sending a proxy, will be local 0x%x connection %p\n",
			proxy_target, _connection);

	  [aRmc encodeValueOfObjCType: @encode(__typeof__(proxy_tag))
				   at: &proxy_tag];

	  [aRmc encodeValueOfObjCType: @encode(__typeof__(proxy_target))
				   at: &proxy_target];
	}
    }
  else
    {
      /*
       *	This proxy will still be remote on the other side
       */
      NSPort		*proxy_connection_out_port = [_connection sendPort];
      NSDistantObject	*localProxy;

      NSAssert(proxy_connection_out_port,
	NSInternalInconsistencyException);
      NSAssert([proxy_connection_out_port isValid],
	NSInternalInconsistencyException);
      NSAssert(proxy_connection_out_port != [encoder_connection sendPort],
	NSInternalInconsistencyException);

      proxy_tag = PROXY_REMOTE_FOR_BOTH;

      /*
       * Get a proxy to refer to self - we send this to the other
       * side so we will be retained until the other side has
       * obtained a proxy to the original object via a connection
       * to the original vendor.
       */
      localProxy = [NSDistantObject proxyWithLocal: self
					connection: encoder_connection];

      if (debug_proxy)
	NSLog(@"Sending triangle-connection proxy 0x%x "
	      @"proxy-conn %p to-proxy 0x%x to-conn %p\n",
		localProxy->_handle, localProxy->_connection,
		proxy_target, _connection);

      /*
       * It's remote here, so we need to tell other side where to form
       * triangle connection to
       */
      [aRmc encodeValueOfObjCType: @encode(__typeof__(proxy_tag))
			       at: &proxy_tag];

      [aRmc encodeValueOfObjCType: @encode(__typeof__(localProxy->_handle))
			       at: &localProxy->_handle];

      [aRmc encodeValueOfObjCType: @encode(__typeof__(proxy_target))
			       at: &proxy_target];

      [aRmc encodeBycopyObject: proxy_connection_out_port];

      /*
       * Tell connection that localProxy is being vended.
       */
      [encoder_connection vendLocal: localProxy];
    }
}

/*
 *	This method needs to be implemented to actually do anything.
 */
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  if (debug_proxy)
    NSLog(@"NSDistantObject forwardInvocation: %@\n", anInvocation);

  if (![_connection isValid])
    [NSException
	   raise: NSGenericException
	  format: @"Trying to send message to an invalid Proxy.\n"
      @"You should request NSConnectionDidDieNotification's and\n"
      @"release all references to the proxy's of invalid Connections."];

  /* We could be released while the connection is forwarding, so we need
   * to retain self.  But, the remote end couild raise an exception, so
   * we can't rely on being able to release again; use autorelease.
   */
  AUTORELEASE(RETAIN(self));
  [_connection forwardInvocation: anInvocation forProxy: self];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  DESTROY(self);
  [NSException raise: NSGenericException
	      format: @"NSDistantObject decodes from placeholder"];
  return nil;
}

/**
 * Initialises and returns a proxy associated with aConnection
 * which will hold a reference to the local object anObject.
 */
- (id) initWithLocal: (id)anObject connection: (NSConnection*)aConnection
{
  NSAssert([aConnection isValid], NSInternalInconsistencyException);

  _object = RETAIN(anObject);
  _handle = 0;
  _connection = RETAIN(aConnection);
  self = [_connection retainOrAddLocal: self forObject: anObject];

  if (debug_proxy)
    NSLog(@"Created new local=%p object %p target 0x%x connection %p\n",
     self, _object, _handle, _connection);

  return self;
}

/**
 * Initialises and returns a proxy associated with aConnection
 * which will provide a link to a remote object whose reference
 * locally is target.
 */
- (id) initWithTarget: (unsigned)target connection: (NSConnection*)aConnection
{
  NSAssert([aConnection isValid], NSInternalInconsistencyException);

  _object = nil;
  _handle = target;
  _connection = RETAIN(aConnection);

  /*
   * We register this object with the connection using it.
   * Conceivably this could result in self being changed.
   */
  self = [_connection retainOrAddProxy: self forTarget: target];

  if (debug_proxy)
      NSLog(@"Created new proxy=%p target 0x%x connection %p\n",
	 self, _handle, _connection);

  return self;
}

/**
 * <p>Returns the method signature describing the arguments and return
 * types of the method in the object referred to by the receiver
 * which implements the aSelector message.
 * </p>
 * <p>This method may need to refer to another process (causing relatively
 * slow network communication) and approximately double the time taken for
 * sending a distributed objects message, so you are advised to use the
 * -setProtocolForProxy: method to avoid this occurring.
 * </p>
 */
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  if (0 == aSelector)
    {
      return nil;
    }
  if (_object != nil)
    {
      return [_object methodSignatureForSelector: aSelector];
    }
  else
    {
      /*
       * Evil hack to prevent recursion - if we are asking a remote
       * object for a method signature, we can't ask it for the
       * signature of methodSignatureForSelector:, so we hack in
       * the signature required manually :-(
       */
      if (sel_isEqual(aSelector, _cmd))
	{
	  static	NSMethodSignature	*sig = nil;

	  if (sig == nil)
	    {
	      sig = RETAIN([NSMethodSignature signatureWithObjCTypes: "@@::"]);
	    }
	  return sig;
	}
      if (sel_isEqual(aSelector, @selector(methodType)))
	{
	  static	NSMethodSignature	*sig = nil;

	  if (sig == nil)
	    {
	      sig = RETAIN([NSMethodSignature signatureWithObjCTypes: "r*@:"]);
	    }
	  return sig;
	}

      if (_protocol != nil)
	{
	  struct objc_method_description mth;
	  mth = GSProtocolGetMethodDescriptionRecursive(_protocol, aSelector, YES, YES);

	  if (mth.name == NULL && mth.types == NULL)
	    {
	      // Search for class method
	      mth = GSProtocolGetMethodDescriptionRecursive(_protocol, aSelector, YES, NO);
	    }

	  if (mth.types)
	    return [NSMethodSignature signatureWithObjCTypes: mth.types];
	}

      if (_sigs != 0)
	{
	  NSMutableDictionary	*d = (NSMutableDictionary*)_sigs;
	  NSString		*s = NSStringFromSelector(aSelector);
	  NSMethodSignature	*m = [d objectForKey: s];

	  if (m != nil) return m;
	}

	{
	  id		m = nil;
	  id		inv;
	  id		sig;

	  DO_FORWARD_INVOCATION(methodSignatureForSelector:, aSelector);

	  if ([m isProxy] == YES)
	    {
	      const char	*types;

	      types = [m methodType];
	      /* Create a local method signature.
	       */
	      m = [NSMethodSignature signatureWithObjCTypes: types];
	    }
	  if (m != nil)
	    {
	      NSMutableDictionary	*d = (NSMutableDictionary*)_sigs;
	      NSString			*s = NSStringFromSelector(aSelector);

	      if (d == nil)
		{
		  d = [NSMutableDictionary new];
		  _sigs = (void*)d;
		}
	      [d setObject: m forKey: s];
	    }
	  return m;
	}
    }
}

/**
 * <p>A key method for Distributed Objects performance.  This sets the
 * a protocol that the distant object referred to by the proxy should
 * conform to.  When messages in that protocol are sent to the proxy,
 * the proxy knows that it does not need to ask the remote object for
 * the method signature in order to send the message to it, but can
 * send the message straight away based on the local method signature
 * information obtained from the protocol.
 * </p>
 * <example>
 *   if ([anObj isProxy] == YES)
 *     {
 *       [anObj setProtocolForProxy: @protocol(MyProtocol)];
 *     }
 * </example>
 * <p>It is <em>highly recommended</em> that you make use of this facility,
 * but you must beware that versions of the compiler prior to 3.3 suffer a
 * serious bug with respect to the @protocol directive.  If the protocol
 * referred to is not declared and implemented in the file where @protocol
 * is used to refer to the protocol by name, a runtime error will occur
 * when you try to use it.
 * </p>
 * <p>Beware, if you don't use this method to set the protocol, the system
 * might well ask the remote process for method signature information, and
 * the remote process might get it <em>wrong</em>.  This is because the
 * class of the remote object needs to have been declared to conform to the
 * protocol in order for it to know about any protocol qualifiers (the
 * keywords <code>bycopy, byref, in, out, inout,</code> and
 * <code>oneway</code>).  If the author of the server process forgot to do
 * this, the type information returned from that process may not be what
 * you are expecting.
 * </p>
 * The class of the server object should be declared like this ...
 * <example>
 * @interface MyServerClass : NSObject &lt;MyProtocol&gt;
 * ...
 * @end
 * </example>
 */
- (void) setProtocolForProxy: (Protocol*)aProtocol
{
  _protocol = aProtocol;
}

@end

/**
 *  Adds some backwards-compatibility and other methods.
 */
@implementation NSDistantObject(GNUstepExtensions)

- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  return 0;
}

/**
 * Used by the garbage collection system to tidy up when a proxy is destroyed.
 */
- (void) finalize
{
  if (_connection)
    {
      if (debug_proxy > 3)
	NSLog(@"retain count for connection (%p) is now %lx\n",
		_connection, [_connection retainCount]);
      /*
       * A proxy for local object retains its target - so we release it.
       * For a local object the connection also retains this proxy, so we
       * can't be deallocated unless we are already removed from the
       * connection, and there is no need to remove self from connection.
       *
       * A proxy has a nil local object, and retains it's connection so
       * that the connection will continue to exist as long as there is
       * a something to use it.
       * So we release our reference to the connection here just as soon
       * as we have removed ourself from the connection.
       */
      if (_object == nil)
	[_connection removeProxy: self];
      else
	DESTROY(_object);
      RELEASE(_connection);
    }
}




- (Class) classForCoder
{
  return object_getClass(self);
}

/**
 * Returns the class of the receiver.
 */
- (Class) classForPortCoder
{
  return object_getClass(self);
}

/**
 * If a protocol has been set for the receiver, this method tests to
 * see that the set protocol conforms to aProtocol. Otherwise, the
 * remote object is checked to see whether it conforms to aProtocol.
 */
- (BOOL) conformsToProtocol: (Protocol*)aProtocol
{
  if (_protocol != nil)
    {
      return protocol_conformsToProtocol(_protocol, aProtocol);
    }
  else
    {
      return [(id)self _conformsToProtocolNamed: protocol_getName(aProtocol)];
    }
}

- (BOOL) respondsToSelector: (SEL)aSelector
{
  BOOL		m = NO;
  id		inv;
  id		sig;

  DO_FORWARD_INVOCATION(respondsToSelector:, aSelector);

  return m;
}

- (id) replacementObjectForCoder: (NSCoder*)aCoder
{
  return self;
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  return self;
}

- (NSUInteger) sizeInBytesExcluding: (NSHashTable*)exclude
{
  if (0 == NSHashGet(exclude, self))
    {
      Class             c = object_getClass(self);
      NSUInteger        size = class_getInstanceSize(c);

      return size;
    }
  return 0;
}

@end


@implementation Protocol (DistributedObjectsCoding)

- (Class) classForPortCoder
{
  return object_getClass(self);
}

- (id) replacementObjectForPortCoder: (NSPortCoder*)aRmc;
{
  if ([aRmc isBycopy])
    return self;
  else
    return [NSDistantObject proxyWithLocal: self
				connection: [aRmc connection]];
}

@end

