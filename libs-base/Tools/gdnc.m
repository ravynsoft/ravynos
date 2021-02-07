/* Copyright (C) 1998 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1998

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYING.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   */

#import	"common.h"

#include <stdio.h>
#ifdef _WIN32
#include <io.h>
#endif

#import	"Foundation/NSArray.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSBundle.h"
#import	"Foundation/NSConnection.h"
#import	"Foundation/NSData.h"
#import	"Foundation/NSDistantObject.h"
#import	"Foundation/NSDistributedNotificationCenter.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSHashTable.h"
#import	"Foundation/NSHost.h"
#import	"Foundation/NSNotification.h"
#import	"Foundation/NSPort.h"
#import	"Foundation/NSPortNameServer.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSRunLoop.h"
#import	"Foundation/NSTask.h"
#import	"Foundation/NSTimer.h"
#import	"Foundation/NSUserDefaults.h"


#if	defined(__MINGW__)
#include	"process.h"
#endif


#if	defined(HAVE_SYS_FCNTL_H)
#  include	<sys/fcntl.h>
#elif	defined(HAVE_FCNTL_H)
#  include	<fcntl.h>
#endif

#ifdef	HAVE_SYSLOG_H
#include <syslog.h>
#endif

#if	defined(HAVE_SYS_SIGNAL_H)
#  include	<sys/signal.h>
#elif	defined(HAVE_SIGNAL_H)
#  include	<signal.h>
#endif

#ifndef NSIG
#define NSIG    32
#endif

static BOOL	debugging = NO;
static BOOL	is_daemon = NO;		/* Currently running as daemon.	 */
static BOOL	auto_stop = NO;		/* Should we shut down when unused? */

static NSTimer  *timer = nil;           /* When to shut down. */

#if defined(HAVE_SYSLOG) || defined(HAVE_SLOGF)
#  if defined(HAVE_SLOGF)
#    include <sys/slogcodes.h>
#    include <sys/slog.h>
#    define LOG_CRIT _SLOG_CRITICAL
#    define LOG_DEBUG _SLOG_DEBUG1
#    define LOG_ERR _SLOG_ERROR
#    define LOG_INFO _SLOG_INFO
#    define LOG_WARNING _SLOG_WARNING
#    define syslog(prio, msg,...) slogf(_SLOG_SETCODE(_SLOG_SYSLOG, 0), prio, msg, __VA_ARGS__)
#  endif
static int	log_priority = LOG_DEBUG;

static void
gdnc_log (int prio, const char *ebuf)
{
  if (is_daemon)
    {
#   if defined(HAVE_SLOGF)
	  // Let's not have 0 as the value for prio. It means "shutdown" on QNX
      syslog (prio ? prio : log_priority, "%s", ebuf);
#   else
      syslog (log_priority | prio, "%s", ebuf);
#   endif
    }
  else if (prio == LOG_INFO)
    {
      write (1, ebuf, strlen (ebuf));
      write (1, "\n", 1);
    }
  else
    {
      write (2, ebuf, strlen (ebuf));
      write (2, "\n", 1);
    }

  if (prio == LOG_CRIT)
    {
      if (is_daemon)
	{
	  syslog (LOG_CRIT, "%s", "exiting.");
	}
      else
     	{
	  fprintf (stderr, "exiting.\n");
	  fflush (stderr);
	}
      exit(EXIT_FAILURE);
    }
}

#else

#define	LOG_CRIT	2
#define LOG_DEBUG	0
#define LOG_ERR		1
#define LOG_INFO	0
#define LOG_WARNING	0
static void
gdnc_log (int prio, const char *ebuf)
{
  write (2, ebuf, strlen (ebuf));
  write (2, "\n", 1);
  if (prio == LOG_CRIT)
    {
      fprintf (stderr, "exiting.\n");
      fflush (stderr);
      exit(EXIT_FAILURE);
    }
}
#endif

static void
ihandler(int sig)
{
  static BOOL	beenHere = NO;
  BOOL		action;
  NSString	*e;

  /*
   * Deal with recursive call of handler.
   */
  if (beenHere == YES)
    {
      abort();
    }
  beenHere = YES;

  /*
   * If asked to terminate, do so cleanly.
   */
  if (sig == SIGTERM)
    {
      exit(EXIT_FAILURE);
    }

#ifdef	DEBUG
  action = YES;		// abort() by default.
#else
  action = NO;		// exit() by default.
#endif
  e = [[[NSProcessInfo processInfo] environment] objectForKey:
    @"CRASH_ON_ABORT"];
  if (e != nil)
    {
      action = [e boolValue];
    }

  if (action == YES)
    {
      abort();
    }
  else
    {
      fprintf(stderr, "gdnc killed by signal %d\n", sig);
      exit(sig);
    }
}


#include	"gdnc.h"

/*
 * The following dummy class is here solely as a workaround for pre 3.3
 * versions of gcc where protocols didn't work properly unless implemented
 * in the source where the '@protocol()' directive is used.
 */
@interface NSDistributedNotificationCenterGDNCDummy : NSObject <GDNCClient>
- (oneway void) postNotificationName: (NSString*)name
                              object: (NSString*)object
                            userInfo: (NSData*)info
                            selector: (NSString*)aSelector
                                  to: (uint64_t)observer;
@end
@implementation	NSDistributedNotificationCenterGDNCDummy
- (oneway void) postNotificationName: (NSString*)name
                              object: (NSString*)object
                            userInfo: (NSData*)info
                            selector: (NSString*)aSelector
                                  to: (uint64_t)observer
{
  return;
}
@end

@interface	GDNCNotification : NSObject
{
@public
  NSString	*name;
  NSString	*object;
  NSData	*info;
}
+ (GDNCNotification*) notificationWithName: (NSString*)notificationName
				    object: (NSString*)notificationObject
				      data: (NSData*)notificationData;
@end

@implementation	GDNCNotification
- (void) dealloc
{
  RELEASE(name);
  RELEASE(object);
  RELEASE(info);
  [super dealloc];
}
- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ Name:'%@' Object:'%@' Info:'%@'",
    [super description], name, object, info];
}
+ (GDNCNotification*) notificationWithName: (NSString*)notificationName
				    object: (NSString*)notificationObject
				      data: (NSData*)notificationData
{
  GDNCNotification	*tmp = [GDNCNotification alloc];

  tmp->name = RETAIN(notificationName);
  tmp->object = RETAIN(notificationObject);
  tmp->info = RETAIN(notificationData);
  return AUTORELEASE(tmp);
}
@end


/*
 *	Information about a notification observer.
 */
@interface	GDNCClient : NSObject
{
@public
  BOOL			suspended;
  id <GDNCClient>	client;
  NSMutableArray	*observers;
}
@end

@implementation	GDNCClient
- (void) dealloc
{
  RELEASE(observers);
  [super dealloc];
}

- (id) init
{
  observers = [NSMutableArray new];
  return self;
}
@end



/*
 *	Information about a notification observer.
 */
@interface	GDNCObserver : NSObject
{
@public
  uint64_t		observer;
  NSString		*notificationName;
  NSString		*notificationObject;
  NSString		*selector;
  GDNCClient		*client;
  NSMutableArray	*queue;
  NSNotificationSuspensionBehavior	behavior;
}
@end

@implementation	GDNCObserver

- (void) dealloc
{
  RELEASE(queue);
  RELEASE(selector);
  RELEASE(notificationName);
  RELEASE(notificationObject);
  [super dealloc];
}

- (id) init
{
  queue = [[NSMutableArray alloc] initWithCapacity: 1];
  return self;
}
@end


@interface	GDNCServer : NSObject <GDNCProtocol>
{
  NSConnection		*conn;
  NSMapTable		*connections;
  NSHashTable		*allObservers;
  NSMutableDictionary	*observersForNames;
  NSMutableDictionary	*observersForObjects;
}

- (void) addObserver: (uint64_t)anObserver
	    selector: (NSString*)aSelector
	        name: (NSString*)notificationName
	      object: (NSString*)anObject
  suspensionBehavior: (NSNotificationSuspensionBehavior)suspensionBehavior
		 for: (id<GDNCClient>)client;

- (BOOL) connection: (NSConnection*)ancestor
  shouldMakeNewConnection: (NSConnection*)newConn;

- (id) connectionBecameInvalid: (NSNotification*)notification;

- (oneway void) postNotificationName: (NSString*)notificationName
			      object: (NSString*)notificationObject
			    userInfo: (NSData*)d
		  deliverImmediately: (BOOL)deliverImmediately
				 for: (id<GDNCClient>)client;

- (void) removeObserver: (GDNCObserver*)observer;

- (void) removeObserversForClients: (NSMapTable*)clients;

- (void) removeObserver: (uint64_t)anObserver
		   name: (NSString*)notificationName
		 object: (NSString*)notificationObject
		    for: (id<GDNCClient>)client;

- (void) setSuspended: (BOOL)flag
		  for: (id<GDNCClient>)client;
@end

@implementation	GDNCServer

- (void) autoStop: (NSTimer*)t
{
  if (t == timer)
    {
      timer = nil;
    }
  if (auto_stop == YES && NSCountMapTable(connections) == 0)
    {
      /* There is nothing else using this process, stop.
       */
      exit(EXIT_SUCCESS);
    }
}

- (void) dealloc
{
  NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];
  NSMapEnumerator	enumerator;
  NSConnection		*connection;
  NSMapTable		*clients;

  if (conn)
    {
      [nc removeObserver: self
		    name: NSConnectionDidDieNotification
		  object: conn];
      DESTROY(conn);
    }

  /*
   *	Free all the client map tables in the connections map table and
   *	ignore notifications from those connections.
   */
  enumerator = NSEnumerateMapTable(connections);
  while (NSNextMapEnumeratorPair(&enumerator,
		(void**)&connection, (void**)&clients) == YES)
    {
      [nc removeObserver: self
		    name: NSConnectionDidDieNotification
		  object: connection];
      [self removeObserversForClients: clients];
      NSFreeMapTable(clients);
    }

  /*
   *	Now free the connections map itself and the table of observers.
   */
  NSFreeMapTable(connections);
  NSFreeHashTable(allObservers);

  /*
   *	And release the maps of notification names and objects.
   */
  RELEASE(observersForNames);
  RELEASE(observersForObjects);
  [super dealloc];
}

- (id) init
{
  NSString		*hostname;
  NSString		*service;
  BOOL			isNetwork = NO;
  BOOL			isPublic = NO;
  NSPort		*port;
  NSPortNameServer	*ns;
  NSUserDefaults	*defs;

  connections = NSCreateMapTable(NSObjectMapKeyCallBacks,
		NSNonOwnedPointerMapValueCallBacks, 0);
  allObservers = NSCreateHashTable(NSNonOwnedPointerHashCallBacks, 0);
  observersForNames = [NSMutableDictionary new];
  observersForObjects = [NSMutableDictionary new];

  defs = [NSUserDefaults standardUserDefaults];
  hostname = [defs stringForKey: @"NSHost"];
  if ([hostname length] > 0 || [defs boolForKey: @"GSPublic"] == YES)
    {
      if (hostname == nil || [hostname isEqualToString: @"localhost"] == YES
	|| [hostname isEqualToString: @"127.0.0.1"] == YES)
	{
	  hostname = @"";
	}
      isPublic = YES;
    }
  else if ([defs boolForKey: @"GSNetwork"] == YES)
    {
      isNetwork = YES;
    }

  if (isNetwork)
    {
      service = GDNC_NETWORK;
      ns = [NSSocketPortNameServer sharedInstance];
      port = (NSPort*)[NSSocketPort port];
    }
  else if (isPublic)
    {
      service = GDNC_SERVICE;
      ns = [NSSocketPortNameServer sharedInstance];
      port = (NSPort*)[NSSocketPort port];
    }
  else
    {
      NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];

      if ([defs objectForKey: @"NSPortIsMessagePort"] != nil
	&& [defs boolForKey: @"NSPortIsMessagePort"] == NO)
	{
	  ns = [NSSocketPortNameServer sharedInstance];
	  port = (NSPort*)[NSSocketPort port];
	}
      else
	{
	  ns = [NSMessagePortNameServer sharedInstance];
	  port = (NSPort*)[NSMessagePort port];
	}
      hostname = @"";
      service = GDNC_SERVICE;
    }

  conn = [[NSConnection alloc] initWithReceivePort: port sendPort: nil];
  [conn setRootObject: self];

  if ([hostname length] == 0
    || [[NSHost hostWithName: hostname] isEqual: [NSHost currentHost]] == YES)
    {
      if ([conn registerName: service withNameServer: ns] == NO)
	{
	  NSLog(@"gdnc - unable to register with name server as %@ - quiting.",
	    service);
	  DESTROY(self);
	  return self;
	}
    }
  else
    {
      NSHost		*host = [NSHost hostWithName: hostname];
      NSPort		*port = [conn receivePort];
      NSArray		*a;
      unsigned		c;

      if (host == nil)
	{
	  NSLog(@"gdnc - unknown NSHost argument  ... %@ - quiting.", hostname);
	  DESTROY(self);
	  return self;
	}
      a = [host names];
      c = [a count];
      while (c-- > 0)
	{
	  NSString	*name = [a objectAtIndex: c];

	  name = [service stringByAppendingFormat: @"-%@", name];
	  if ([ns registerPort: port forName: name] == NO)
	    {
              NSLog(@"gdnc - failed to register as %@", name);
	    }
	}
      a = [host addresses];
      c = [a count];
      while (c-- > 0)
	{
	  NSString	*name = [a objectAtIndex: c];

	  name = [service stringByAppendingFormat: @"-%@", name];
	  if ([ns registerPort: port forName: name] == NO)
	    {
              NSLog(@"gdnc - failed to register as %@", name);
	    }
	}
    }

  /*
   *	Get notifications for new connections and connection losses.
   */
  [conn setDelegate: self];
  [[NSNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector(connectionBecameInvalid:)
	   name: NSConnectionDidDieNotification
	 object: conn];
  return self;
}

- (void) addObserver: (uint64_t)anObserver
	    selector: (NSString*)aSelector
	        name: (NSString*)notificationName
	      object: (NSString*)anObject
  suspensionBehavior: (NSNotificationSuspensionBehavior)suspensionBehavior
		 for: (id<GDNCClient>)client
{
  GDNCClient	*info;
  NSMapTable	*clients;
  GDNCObserver	*obs;
  NSConnection	*connection;

  if (debugging)
    NSLog(@"Adding observer %llu for %@ %@",
      (unsigned long long)anObserver, notificationName, anObject);

  connection = [(NSDistantObject*)client connectionForProxy];
  clients = (NSMapTable*)NSMapGet(connections, connection);
  if (clients == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Unknown connection for new observer"];
    }
  info = (GDNCClient*)NSMapGet(clients, client);
  if (info == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Unknown client for new observer"];
    }

  /*
   *	Create new observer info and add to array of observers for this
   *	client and the table of all observers.
   */
  obs = [GDNCObserver new];
  obs->observer = anObserver;
  obs->client = info;
  obs->behavior = suspensionBehavior;
  obs->selector = [aSelector copy];
  [info->observers addObject: obs];
  RELEASE(obs);
  NSHashInsert(allObservers, obs);

  /*
   *	Now add the observer to the lists of observers interested in it's
   *	particular notification names and objects.
   */
  if (anObject)
    {
      NSMutableArray	*objList;

      objList = [observersForObjects objectForKey: anObject];
      if (objList == nil)
	{
	  objList = [NSMutableArray new];
	  [observersForObjects setObject: objList forKey: anObject];
	  RELEASE(objList);
	}
      /*
       *	If possible use an existing string as the key.
       */
      if ([objList count] > 0)
	{
	  GDNCObserver	*tmp = [objList objectAtIndex: 0];

	  anObject = tmp->notificationObject;
	}
      obs->notificationObject = RETAIN(anObject);
      [objList addObject: obs];
    }

  if (notificationName)
    {
      NSMutableArray	*namList;

      namList = [observersForNames objectForKey: notificationName];
      if (namList == nil)
	{
	  namList = [NSMutableArray new];
	  [observersForNames setObject: namList forKey: notificationName];
	  RELEASE(namList);
	}
      /*
       *	If possible use an existing string as the key.
       */
      if ([namList count] > 0)
	{
	  GDNCObserver	*tmp = [namList objectAtIndex: 0];

	  notificationName = tmp->notificationName;
	}
      obs->notificationName = RETAIN(notificationName);
      [namList addObject: obs];
    }

}

- (BOOL) connection: (NSConnection*)ancestor
  shouldMakeNewConnection: (NSConnection*)newConn
{
  NSMapTable	*table;

  [[NSNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector(connectionBecameInvalid:)
	   name: NSConnectionDidDieNotification
	 object: newConn];
  [newConn setDelegate: self];
  /*
   *	Create a new map table entry for this connection with a value that
   *	is a table (normally with a single entry) containing registered
   *	clients (proxies for NSDistributedNotificationCenter objects).
   */
  table = NSCreateMapTable(NSObjectMapKeyCallBacks,
		NSObjectMapValueCallBacks, 0);
  NSMapInsert(connections, newConn, table);
  if (nil != timer)
    {
      [timer invalidate];
      timer = nil;
    }
  return YES;
}

- (id) connectionBecameInvalid: (NSNotification*)notification
{
  id connection = [notification object];

  [[NSNotificationCenter defaultCenter]
    removeObserver: self
	      name: NSConnectionDidDieNotification
	    object: connection];

  if (connection == conn)
    {
      NSLog(@"argh - gdnc server root connection has been destroyed.");
      exit(EXIT_FAILURE);
    }
  else
    {
      NSMapTable	*table;

      /*
       *	Remove all clients registered via this connection
       *	(should normally only be 1) - then the connection.
       */
      table = NSMapGet(connections, connection);
      NSMapRemove(connections, connection);
      if (table != 0)
	{
	  [self removeObserversForClients: table];
	  NSFreeMapTable(table);
	}

      if (auto_stop == YES && NSCountMapTable(connections) == 0)
        {
          /* There is nothing left using this notification center,
           * so schedule the auto_stop to occur in a short while
           * if nothing has connected to us.
           */
          if (nil != timer)
            {
              [timer invalidate];
            }
          timer = [NSTimer scheduledTimerWithTimeInterval: 15.0
                                                   target: self
                                                 selector: @selector(autoStop:)
                                                 userInfo: nil
                                                  repeats: NO];
	}
    }
  return nil;
}

- (void) registerClient: (id<GDNCClient>)client
{
  NSMapTable	*table;
  GDNCClient	*info;

  table = NSMapGet(connections, [(NSDistantObject*)client connectionForProxy]);
  if (table == 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"registration with unknown connection"];
    }
  if (NSMapGet(table, client) != 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"registration with registered client"];
    }
  info = [GDNCClient new];
  if ([(id)client isProxy] == YES)
    {
      Protocol	*p = @protocol(GDNCClient);

      [(id)client setProtocolForProxy: p];
    }
  info->client = client;
  NSMapInsert(table, client, info);
  RELEASE(info);
}

- (oneway void) postNotificationName: (NSString*)notificationName
			      object: (NSString*)notificationObject
			    userInfo: (NSData*)d
		  deliverImmediately: (BOOL)deliverImmediately
				 for: (id<GDNCClient>)client
{
  NSMutableArray	*observers = [NSMutableArray array];
  NSMutableArray	*byName;
  NSMutableArray	*byObject;
  unsigned		pos;
  GDNCNotification	*notification;

  byName = [observersForNames objectForKey: notificationName];
  byObject = [observersForObjects objectForKey: notificationObject];
  /*
   *	Build up a list of all those observers that should get sent this.
   */
  for (pos = [byName count]; pos > 0; pos--)
    {
      GDNCObserver	*obs = [byName objectAtIndex: pos - 1];

      if (obs->notificationObject == nil
	|| [obs->notificationObject isEqual: notificationObject])
	{
	  [observers addObject: obs];
	}
    }
  for (pos = [byObject count]; pos > 0; pos--)
    {
      GDNCObserver	*obs = [byObject objectAtIndex: pos - 1];

      if (obs->notificationName == nil
	|| [obs->notificationName isEqual: notificationName])
	{
	  if ([observers indexOfObjectIdenticalTo: obs] == NSNotFound)
	    {
	      [observers addObject: obs];
	    }
	}
    }

  if ([observers count] == 0)
    {
      return;
    }

  /*
   *	Build notification object to queue for observer.
   */
  notification = [GDNCNotification notificationWithName: notificationName
                                                 object: notificationObject
                                                   data: d];

  /*
   *	Add the object to the queue for this observer depending on suspension
   *	state of the client NSDistributedNotificationCenter etc.
   */
  for (pos = [observers count]; pos > 0; pos--)
    {
      GDNCObserver	*obs = [observers objectAtIndex: pos - 1];

      if (obs->client->suspended == NO || deliverImmediately == YES)
	{
	  [obs->queue addObject: notification];
	}
      else
	{
	  switch (obs->behavior)
	    {
	      case NSNotificationSuspensionBehaviorDrop:
		break;
	      case NSNotificationSuspensionBehaviorCoalesce:
		[obs->queue removeAllObjects];
		[obs->queue addObject: notification];
		break;
	      case NSNotificationSuspensionBehaviorHold:
		[obs->queue addObject: notification];
		break;
	      case NSNotificationSuspensionBehaviorDeliverImmediately:
		[obs->queue addObject: notification];
		break;
	    }
	}
    }

  /*
   *	Now perform the actual posting of the notification to the observers in
   *	our array.
   */
  for (pos = [observers count]; pos > 0; pos--)
    {
      GDNCObserver	*obs = [observers objectAtIndex: pos - 1];

      if (obs->client->suspended == NO || deliverImmediately == YES)
	{
	  /*
	   *	Post notifications to the observer until:
	   *		an exception		(obs is set to nil)
	   *		the queue is empty	([obs->queue count] == 0)
	   *		the observer is removed	(obs is not in allObservers)
	   */
	  while (obs != nil && [obs->queue count] > 0
	    && NSHashGet(allObservers, obs) != 0)
	    {
	      GDNCNotification *n;
	      n = RETAIN([obs->queue objectAtIndex: 0]);
	      NS_DURING
		{
		  [obs->queue removeObjectAtIndex: 0];
  if (debugging)
    NSLog(@"Posting to observer %llu with %@", (unsigned long long)obs->observer, n);
		  [obs->client->client postNotificationName: n->name
						     object: n->object
						   userInfo: n->info
						   selector: obs->selector
							 to: obs->observer];
		}
	      NS_HANDLER
		{
		  obs = nil;
		  NSLog(@"Problem posting notification to client: %@",
		    localException);
		}
	      NS_ENDHANDLER
	      RELEASE(n);
	    }
	}
    }
}

- (void) removeObserver: (GDNCObserver*)observer
{
  if (debugging)
    NSLog(@"Removing observer %llu for %@ %@",
      (unsigned long long)observer->observer, observer->notificationName,
      observer->notificationObject);

  if (observer->notificationObject)
    {
      NSMutableArray	*objList;

      objList= [observersForObjects objectForKey: observer->notificationObject];
      if (objList != nil)
	{
	  [objList removeObjectIdenticalTo: observer];
	}
    }
  if (observer->notificationName)
    {
      NSMutableArray	*namList;

      namList = [observersForNames objectForKey: observer->notificationName];
      if (namList != nil)
	{
	  [namList removeObjectIdenticalTo: observer];
	}
    }
  NSHashRemove(allObservers, observer);
  [observer->client->observers removeObjectIdenticalTo: observer];
}

- (void) removeObserversForClients: (NSMapTable*)clients
{
  NSMapEnumerator	enumerator;
  NSObject		*client;
  GDNCClient		*info;

  enumerator = NSEnumerateMapTable(clients);
  while (NSNextMapEnumeratorPair(&enumerator,
		(void**)&client, (void**)&info) == YES)
    {
      while ([info->observers count] > 0)
	{
	  [self removeObserver: [info->observers objectAtIndex: 0]];
	}
    }
}

- (void) removeObserver: (uint64_t)anObserver
		   name: (NSString*)notificationName
		 object: (NSString*)notificationObject
		    for: (id<GDNCClient>)client
{
  if (anObserver == 0)
    {
      if (notificationName == nil)
	{
	  NSMutableArray	*observers;

	  /*
	   *	No notification name - so remove all with matching object.
	   */
	  observers = [observersForObjects objectForKey: notificationObject];
	  while ([observers count] > 0)
	    {
	      GDNCObserver	*obs;

	      obs = [observers objectAtIndex: 0];
	      [self removeObserver: obs];
	    }
	}
      else if (notificationObject == nil)
	{
	  NSMutableArray	*observers;

	  /*
	   *	No notification object - so remove all with matching name.
	   */
	  observers = [observersForObjects objectForKey: notificationName];
	  while ([observers count] > 0)
	    {
	      GDNCObserver	*obs;

	      obs = [observers objectAtIndex: 0];
	      [self removeObserver: obs];
	    }
	}
      else
	{
	  NSMutableArray	*byName;
	  NSMutableArray	*byObject;
	  unsigned		pos;

	  /*
	   *	Remove observers that match both name and object.
	   */
	  byName = [observersForObjects objectForKey: notificationName];
	  byObject = [observersForObjects objectForKey: notificationName];
	  for (pos = [byName count]; pos > 0; pos--)
	    {
	      GDNCObserver	*obs;

	      obs = [byName objectAtIndex: pos - 1];
	      if ([byObject indexOfObjectIdenticalTo: obs] != NSNotFound)
		{
		  [self removeObserver: obs];
		}
	    }
	  for (pos = [byObject count]; pos > 0; pos--)
	    {
	      GDNCObserver	*obs;

	      obs = [byObject objectAtIndex: pos - 1];
	      if ([byName indexOfObjectIdenticalTo: obs] != NSNotFound)
		{
		  [self removeObserver: obs];
		}
	    }
	}
    }
  else
    {
      NSMapTable	*table;
      GDNCClient	*info;

      /*
       *	If an observer object (as an unsigned) was specified then
       *	the observer MUST be from this client - so we can look
       *	through the per-client list of objects.
       */
      table = NSMapGet(connections,
		[(NSDistantObject*)client connectionForProxy]);
      if (table == 0)
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"removeObserver with unknown connection"];
	}
      info = (GDNCClient*)NSMapGet(table, client);
      if (info != nil)
	{
	  unsigned	pos = [info->observers count];

	  while (pos > 0)
	    {
	      GDNCObserver	*obs = [info->observers objectAtIndex: --pos];

	      if (obs->observer == anObserver)
		{
		  if (notificationName == nil ||
			[notificationName isEqual: obs->notificationName])
		    {
		      if (notificationObject == nil ||
			[notificationObject isEqual: obs->notificationObject])
			{
			  [self removeObserver: obs];
			}
		    }
		}
	    }
	}
    }
}

- (void) setSuspended: (BOOL)flag
		  for: (id<GDNCClient>)client
{
  NSMapTable	*table;
  GDNCClient	*info;

  table = NSMapGet(connections, [(NSDistantObject*)client connectionForProxy]);
  if (table == 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"setSuspended: with unknown connection"];
    }
  info = (GDNCClient*)NSMapGet(table, client);
  if (info == nil)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"setSuspended: with unregistered client"];
    }
  info->suspended = flag;
}

- (void) unregisterClient: (id<GDNCClient>)client
{
  NSMapTable	*table;
  GDNCClient	*info;

  table = NSMapGet(connections, [(NSDistantObject*)client connectionForProxy]);
  if (table == 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"unregistration with unknown connection"];
    }
  info = (GDNCClient*)NSMapGet(table, client);
  if (info == nil)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"unregistration with unregistered client"];
    }
  while ([info->observers count] > 0)
    {
      [self removeObserver: [info->observers objectAtIndex: 0]];
    }
  NSMapRemove(table, client);
}

@end


/**
  <p>The  gdnc  daemon is used by GNUstep programs to send notifications and
     messages to one another, in conjunction with the Base library
     Notification-related classes.</p>

  <p>Every user needs to have his own instance of gdnc running.  While  gdnc
     will be started automatically as soon as it is needed, it is recommended
     to start gdnc in a personal login script like  ~/.bashrc  or  ~/.cshrc.
     Alternatively (if you have no command-line tools which use distributed
     notifications) you  can  launch gdnc when your windowing system or the
     window manager is started. For example, on systems  with  X11  you  can
     launch  gdnc  from  your  .xinitrc script or alternatively - if you are
     running Window Maker - put it in Window Maker's autostart script.   See
     the GNUstep Build Guide for a sample startup script.</p>

  <p>Please see the man page for more information.</p>
 */
int
main(int argc, char** argv, char** env)
{
  GDNCServer		*server;
  BOOL			subtask = YES;
  NSProcessInfo		*pInfo;
  NSMutableArray	*args;
  CREATE_AUTORELEASE_POOL(pool);

#ifdef GS_PASS_ARGUMENTS
  GSInitializeProcess(argc, argv, env);
#endif
//[NSObject enableDoubleReleaseCheck: YES];
  pInfo = [NSProcessInfo processInfo];
  args = AUTORELEASE([[pInfo arguments] mutableCopy]);

  if ([[pInfo arguments] containsObject: @"--help"] == YES)
    {
      printf("gdnc\n\n");
      printf("GNU Distributed Notification Center\n");
      printf("--help\tfor help\n");
      printf("--no-fork\tavoid fork() to make debugging easy\n");
      printf("--verbose\tMore verbose debug output\n");
      exit(EXIT_SUCCESS);
    }
  if ([[pInfo arguments] containsObject: @"--auto"] == YES)
    {
      auto_stop = YES;
    }
  if ([[pInfo arguments] containsObject: @"--daemon"] == YES)
    {
      subtask = NO;
      is_daemon = YES;
    }
  if ([[pInfo arguments] containsObject: @"-f"] == YES
    || [[pInfo arguments] containsObject: @"--no-fork"] == YES)
    {
      subtask = NO;
    }
  if ([[pInfo arguments] containsObject: @"--verbose"] == YES)
    {
      debugging = YES;
    }
  if ([[NSUserDefaults standardUserDefaults] boolForKey: @"debug"] == YES)
    {
      subtask = NO;
      debugging = YES;
    }

  if (subtask)
    {
      NSFileHandle	*null;
      NSTask		*t;

      t = [NSTask new];
      NS_DURING
	{
	  [args removeObjectAtIndex: 0];
	  [args addObject: @"--daemon"];
	  [t setLaunchPath: [[NSBundle mainBundle] executablePath]];
	  [t setArguments: args];
	  [t setEnvironment: [pInfo environment]];
	  null = [NSFileHandle fileHandleWithNullDevice];
	  [t setStandardInput: null];
	  [t setStandardOutput: null];
	  [t setStandardError: null];
	  [t launch];
	  DESTROY(t);
	}
      NS_HANDLER
	{
	  gdnc_log(LOG_CRIT, [[localException description] UTF8String]);
	  DESTROY(t);
	}
      NS_ENDHANDLER
      exit(EXIT_FAILURE);
    }
  RELEASE(pool);

  {
    CREATE_AUTORELEASE_POOL(pool);
    NSUserDefaults	*defs;
    int			sym;

    for (sym = 0; sym < NSIG; sym++)
      {
	if (sym == SIGABRT) continue;
#ifdef	SIGPROF
	if (sym == SIGPROF) continue;
#endif
	signal(sym, ihandler);
      }
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
#endif
    signal(SIGTERM, ihandler);

    /*
     * Make gdnc logging go to syslog unless overridden by user.
     */
    defs = [NSUserDefaults standardUserDefaults];
    [defs registerDefaults: [NSDictionary dictionaryWithObjectsAndKeys:
      @"YES", @"GSLogSyslog", nil]];

    server = [GDNCServer new];

    /*
     * Close standard input, output, and error to run as daemon.
     */
    [[NSFileHandle fileHandleWithStandardInput] closeFile];
    [[NSFileHandle fileHandleWithStandardOutput] closeFile];
#ifndef __MINGW__
    if (debugging == NO)
      {
	[[NSFileHandle fileHandleWithStandardError] closeFile];
      }
#endif

    RELEASE(pool);
  }

  if (server != nil)
    {
      CREATE_AUTORELEASE_POOL(pool);
      [[NSRunLoop currentRunLoop] run];
      RELEASE(pool);
    }
  exit(EXIT_SUCCESS);
}

