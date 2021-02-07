/** Implementation of NSPortNameServer class for Distributed Objects
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

   <title>NSPortNameServer class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSException.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSPort.h"
#import "Foundation/NSPortNameServer.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSUserDefaults.h"
#import "GSPortPrivate.h"


/**
 * The abstract port name server class.  This defines an API for
 * working with port name servers ... objects used to manage access
 * to ports in the distributed objects system (see [NSConnection]).
 */
@implementation NSPortNameServer

+ (id) allocWithZone: (NSZone*)aZone
{
  [NSException raise: NSGenericException
	      format: @"attempt to create extra port name server"];
  return nil;
}

+ (void) initialize
{
  if (self == [NSPortNameServer class])
    {
    }
}

/**
 * <p>Returns the default port name server for the process.<br />
 * This is a nameserver for host-local connections private to the current
 * user.  If you with to create public connections  or connections to other
 * hosts, you must use [NSSocketPortNameServer+sharedInstance] instead.
 * </p>
 * This default behavior may be altered by setting the
 * <code>NSPortIsMessagePort</code> user default to NO, in which case
 * an [NSSocketPortNameServer] will be used as the default system name server
 * and you will have to use [NSMessagePortNameServer+sharedInstance]
 * for host-local, private connections.
 */
+ (id) systemDefaultPortNameServer
{
  static id	nameServer = nil;

  if (nameServer == nil)
    {
      [gnustep_global_lock lock];
      if (nameServer == nil)
	{
	  NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
	  id			o;

	  if ([defs objectForKey: @"NSPortIsMessagePort"] != nil
	    && [defs boolForKey: @"NSPortIsMessagePort"] == NO)
	    {
	      o = [NSSocketPortNameServer sharedInstance];
	    }
	  else
	    {
	      o = [NSMessagePortNameServer sharedInstance];
	    }
	  nameServer = RETAIN(o);
	}
      [gnustep_global_lock unlock];
    }
  return nameServer;
}

- (void) dealloc
{
  [NSException raise: NSGenericException
	      format: @"attempt to deallocate default port name server"];
  GSNOSUPERDEALLOC;
}

/**
 * Looks up the port with the specified name on the local host and
 * returns it or nil if no port is found with that name.<br />
 * Different nameservers  have different namespaces appropriate to the
 * type of port they deal with, so failing to find a named port with one
 * nameserver does not guarantee that a port does with that name does
 * not exist.<br />
 * This is a convenience method calling -portForName:onHost: with a nil
 * host argument.
 */
- (NSPort*) portForName: (NSString*)name
{
  return [self portForName: name onHost: nil];
}

/** <override-subclass />
 * Looks up the port with the specified name on host and returns it
 * or nil if no port is found with that name.<br />
 * Different nameservers  have different namespaces appropriate to the
 * type of port they deal with, so failing to find a named port with one
 * nameserver does not guarantee that a port does with that name does
 * not exist.
 */
- (NSPort*) portForName: (NSString*)name
		 onHost: (NSString*)host
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/** <override-subclass />
 * Registers port with the supplied name, so that other processes can
 * look it up to contact it.  A port may be registered with more than
 * one name by making multiple calls to this method.<br />
 * Returns YES on success, NO otherwise.<br />
 * The common cause for failure is that another port is already registered
 * with the name.
 * Raises NSInvalidArgumentException if given bad arguments.
 */
- (BOOL) registerPort: (NSPort*)port
	      forName: (NSString*)name
{
  [self subclassResponsibility: _cmd];
  return NO;
}

/** <override-subclass />
 * Removes any port registration for the supplied name (whether
 * registered in the current process or another).<br />
 * The common cause for failure is that no port is registered
 * with the name.<br />
 * Raises NSInvalidArgumentException if given bad arguments.
 */
- (BOOL) removePortForName: (NSString*)name
{
  [self subclassResponsibility: _cmd];
  return NO;
}
@end

/**
 * Some extensions to make cleaning up port names easier.
 */
@implementation	NSPortNameServer (GNUstep)
/** <override-subclass />
 * Return all names that have been registered with the receiver for port.
 */
- (NSArray*) namesForPort: (NSPort*)port
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
 * Remove all names registered with the receiver for port.
 * Probably inefficient ... subclasses might want to override this.
 */
- (BOOL) removePort: (NSPort*)port
{
  NSEnumerator	*e = [[self namesForPort: port] objectEnumerator];
  NSString	*n;
  BOOL		removed = NO;

  while ((n = [e nextObject]) != nil)
    {
      if ([self removePort: port forName: n] == YES)
	{
	  removed = YES;
	}
    }
  return removed;
}

/** <override-subclass />
 * Remove the name if and only if it is registered with the receiver
 * for the given port.
 */
- (BOOL) removePort: (NSPort*)port forName: (NSString*)name
{
  [self subclassResponsibility: _cmd];
  return NO;
}
@end

