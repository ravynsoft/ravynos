/*
   gpbs.m

   GNUstep pasteboard server

   Copyright (C) 1997,1999 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: August 1997

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.
    
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public  
   License along with this library; see the file COPYING.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include <Foundation/Foundation.h>
#include <AppKit/NSPasteboard.h>
#include <GNUstepGUI/GSPasteboardServer.h>
#include "config.h"

#include <fcntl.h>
#ifdef	HAVE_SYSLOG_H
#include <syslog.h>
#endif

#include <signal.h>
#include <unistd.h>
#include <ctype.h>

#ifdef __MINGW__
#include	"process.h"
#endif

#ifndef	NSIG
#define	NSIG	32
#endif

static BOOL	is_daemon = NO;		/* Currently running as daemon.	 */
static BOOL	auto_stop = NO;		/* Stop when all connections closed. */

static NSMutableArray	*connections = nil;

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
gpbs_log (int prio, const char *ebuf)
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
void
gpbs_log (int prio, const char *ebuf)
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

@class PasteboardServer;
@class PasteboardObject;

@protocol XPb
+ (id) ownerByOsPb: (NSString*)p;
+ (BOOL) initializePasteboard;
@end
static Class	xPbClass;

int verbose = 0;

#define MAXHIST 100

PasteboardServer	*server = nil;
NSConnection		*conn = nil;
NSLock			*dictionary_lock = nil;
NSMutableDictionary	*pasteboards = nil;

@interface	NSPasteboard (GNULocal)
+ (void) _localServer: (id<GSPasteboardSvr>)s;
@end

@interface PasteboardData: NSObject
{
  NSData	*data;
  NSString	*type;
  id		owner;
  id		pboard;
  id		entry;
}
+ (PasteboardData*) newWithType: (NSString*)aType
			  owner: (id)anObject
			 pboard: (id)anotherObject
			  entry: (id)anEntry;
- (BOOL) checkConnection: (NSConnection*)c;
- (NSData*) data;
- (NSData*) dataWithVersion: (int)version;
- (id) owner;
- (id) pboard;
- (void) setData: (NSData*)d;
- (NSString*) type;
@end

@implementation PasteboardData

+ (PasteboardData*) newWithType: (NSString*)aType
			  owner: (id)anObject
			 pboard: (id)anotherObject
			  entry: (id)anEntry
{
  PasteboardData*   d = [PasteboardData alloc];

  if (d)
    {
      d->type = RETAIN(aType);
      d->owner = RETAIN(anObject);
      d->pboard = RETAIN(anotherObject);
      d->entry = RETAIN(anEntry);
    }
  return d;
}

- (BOOL) checkConnection: (NSConnection*)c
{
  BOOL	ourConnection = NO;

  if (owner && [owner isProxy] && [owner connectionForProxy] == c)
    {
      DESTROY(owner);
      DESTROY(pboard);
      DESTROY(entry);
      ourConnection = YES;
    }
  if (pboard && [pboard isProxy] && [pboard connectionForProxy] == c)
    {
      DESTROY(owner);
      DESTROY(pboard);
      DESTROY(entry);
      ourConnection = YES;
    }
  return ourConnection;
}

- (void) dealloc
{
  RELEASE(type);
  RELEASE(data);
  RELEASE(owner);
  RELEASE(pboard);
  RELEASE(entry);
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat:
    @"%@ %p for type '%@' in %@",
    [super description], data, type, entry];
}

- (NSData*) data
{
  if (verbose)
    {
      NSLog(@"-data for %@", self);
    }
  return data;
}

- (NSData*) dataWithVersion: (int)version
{
  if (verbose)
    {
      NSLog(@"-dataWithversion:%d for %@", version, self);
    }
  /*
   * If the owner of this item is an X window - we can't use the data from
   * the last time the selection was accessed because the X window may have
   * changed it's selection without telling us - isn't X wonderful :-(
   */
  if (data != nil && owner != nil
    && [owner isProxy] == NO && [owner isKindOfClass: xPbClass] == YES)
    {
      DESTROY(data);
    }

  if (data == nil && (owner && pboard))
    {
      if ([owner respondsToSelector:
	@selector(pasteboard:provideDataForType:andVersion:)])
	{
	  [owner pasteboard: pboard
	 provideDataForType: type
		 andVersion: version];
	}
      else if ([owner respondsToSelector:
	@selector(pasteboard:provideDataForType:)])
	{
	  [owner pasteboard: pboard
	 provideDataForType: type];
	}
    }
  return [self data];
}

- (id) owner
{
  return owner;
}

- (id) pboard
{
  return pboard;
}

- (void) setData: (NSData*)d
{
  ASSIGN(data, d);
  if (verbose)
    {
      NSLog(@"-setData: for %@", self);
    }
}

- (NSString*) type
{
  return type;
}

@end



@interface PasteboardEntry: NSObject
{
  int			refNum;
  id			owner;
  id			pboard;
  NSMutableArray	*items;
}
+ (PasteboardEntry*) newWithTypes: (NSArray*)someTypes
			    owner: (id)anOwner
			   pboard: (id)aPboard
			      ref: (int)count;
- (void) addTypes: (NSArray*)types owner: (id)owner pasteboard: (id)pb;
- (BOOL) checkConnection: (NSConnection*)c;
- (PasteboardData*) itemForType: (NSString*)type;
- (void) lostOwnership;
- (id) owner;
- (int) refNum;
- (NSArray*) types;
@end

@implementation PasteboardEntry

+ (PasteboardEntry*) newWithTypes: (NSArray*)someTypes
			    owner: (id)anOwner
			   pboard: (id)aPboard
			      ref: (int)count
{
  PasteboardEntry*  e = [PasteboardEntry alloc];

  if (e)
    {
      int     i;

      e->owner = RETAIN(anOwner);
      e->pboard = RETAIN(aPboard);

      e->items = [[NSMutableArray alloc] initWithCapacity: [someTypes count]];
      for (i = 0; i < [someTypes count]; i++)
	{
	  NSString		*type = [someTypes objectAtIndex: i];
	  PasteboardData	*d;

	  d = [PasteboardData newWithType: type
				    owner: anOwner
				   pboard: aPboard
				    entry: e];
	  [e->items addObject: d];
	  RELEASE(d);
	}
      e->refNum = count;
      if (verbose > 1)
	{
	  NSLog(@"New PasteboardEntry %@ with items - %@", e, e->items);
	}
    }
  return e;
}

- (void) addTypes: (NSArray*)newTypes 
	    owner: (id)newOwner 
       pasteboard: (id)pb
{
  int	i;

  for (i = 0; i < [newTypes count]; i++)
    {
      NSString	*type = (NSString*)[newTypes objectAtIndex: i];

      if ([self itemForType: type] == nil)
	{
	  PasteboardData*   d;

	  d = [PasteboardData newWithType: type
				    owner: newOwner
				   pboard: pb
				    entry: self];
	  [items addObject: d];
	  RELEASE(d);
	}
    }
  if (verbose > 1)
    {
      NSLog(@"Modified %@ with items - %@", self, items);
    }
}

- (BOOL) checkConnection: (NSConnection*)c
{
  BOOL		ourConnection = NO;
  unsigned	i;
  id		o;

  if (owner && [owner isProxy] && [owner connectionForProxy] == c)
    {
      o = owner;
      owner = nil;
      RELEASE(o);
      o = pboard;
      pboard = nil;
      RELEASE(o);
      ourConnection = YES;
    }

  if (pboard && [pboard isProxy] && [pboard connectionForProxy] == c)
    {
      o = owner;
      owner = nil;
      RELEASE(o);
      o = pboard;
      pboard = nil;
      RELEASE(o);
      ourConnection = YES;
    }

  for (i = [items count]; i > 0; i--)
    {
      PasteboardData	*d = [items objectAtIndex: i-1];

      if ([d checkConnection: c] == YES && [d data] == nil && [d owner] == nil)
	{
	  if (verbose > 1)
	    {
	      NSLog(@"Removing item from PasteboardEntry %d", refNum);
	    }
	  [items removeObjectAtIndex: i-1];
	}
    }
  return ourConnection;
}

- (void) dealloc
{
  RELEASE(owner);
  RELEASE(pboard);
  RELEASE(items);
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat:
    @"%@ ref %d on %@ owned by %@",
    [super description], refNum, pboard, owner];
}

- (PasteboardData*) itemForType: (NSString*)type
{
  unsigned i, count;

  count = [items count];
  for (i = 0; i < count; i++)
    {
      PasteboardData	*d = [items objectAtIndex: i];

      if ([[d type] isEqual: type])
	{
	  return d;
	}
    }
  return nil;
}

- (void) lostOwnership
{
  NSMutableArray	*a = [NSMutableArray arrayWithCapacity: 4];
  unsigned		i;

  NS_DURING
    {
      if (owner && [owner respondsToSelector:
	@selector(pasteboardChangedOwner:)])
	{
	  [a addObject: owner];
	}

      for (i = 0; i < [items count]; i++)
	{
	  PasteboardData	*d = [items objectAtIndex: i];
	  id			o = [d owner];

	  if (o && [o respondsToSelector: @selector(pasteboardChangedOwner:)]
	    && [a indexOfObjectIdenticalTo: o] == NSNotFound)
	    {
	      [a addObject: o];
	    }
	}

      if (owner && [owner respondsToSelector:
	@selector(pasteboardChangedOwner:)])
	{
	  [owner pasteboardChangedOwner: pboard];
	  if (owner != nil)
	    {
	      [a removeObjectIdenticalTo: owner];
	    }
	}

      for (i = 0; i < [items count] && [a count] > 0; i++)
	{
	  PasteboardData	*d = [items objectAtIndex: i];
	  id			o = [d owner];

	  if (o != nil && [a containsObject: o])
	    {
	      [o pasteboardChangedOwner: [d pboard]];
	      [a removeObjectIdenticalTo: o];
	    }
	}
    }
  NS_HANDLER
    {
      NSLog(@"Error informing objects of ownership change - %@",
	[localException reason]);
    }
  NS_ENDHANDLER
}

- (id) owner
{
  return owner;
}

- (int) refNum
{
  return refNum;
}

- (NSArray*) types
{
  NSMutableArray*   t = [NSMutableArray arrayWithCapacity: [items count]];
  unsigned int	    i;

  for (i = 0; i < [items count]; i++)
    {
      PasteboardData* d = [items objectAtIndex: i];
      [t addObject: [d type]];
    }
  return t;
}

@end



@interface PasteboardObject: NSObject <GSPasteboardObj>
{
  NSString		*name;
  int			nextCount;
  unsigned		histLength;
  NSMutableArray	*history;
  PasteboardEntry	*current;
}

+ (PasteboardObject*) pasteboardWithName: (NSString*)name;

- (int) addTypes: (NSArray*)types
	   owner: (id)owner
      pasteboard: (NSPasteboard*)pboard
	oldCount: (int)count;
- (NSString*) availableTypeFromArray: (NSArray*)types
			 changeCount: (int*)count;
- (int) changeCount;
- (BOOL) checkConnection: (NSConnection*)c;
- (NSData*) dataForType: (NSString*)type
	       oldCount: (int)count
	  mustBeCurrent: (BOOL)flag;
- (int) declareTypes: (NSArray*)types
	       owner: (id)owner
	  pasteboard: (NSPasteboard*)pboard;
- (PasteboardEntry*) entryByCount: (int)count;
- (NSString*) name;
- (void) releaseGlobally;
- (BOOL) setData: (NSData*)data
	 forType: (NSString*)type
	  isFile: (BOOL)flag
	oldCount: (int)count;
- (void) setHistory: (unsigned)length;
- (NSArray*) typesAndChangeCount: (int*)count;

@end

@implementation PasteboardObject

+ (void) initialize
{
  pasteboards = [[NSMutableDictionary alloc] initWithCapacity: 8];
  dictionary_lock = [[NSLock alloc] init];
}

+ (PasteboardObject*) pasteboardWithName: (NSString*)aName
{
  static int	    number = 0;
  PasteboardObject* pb;

  [dictionary_lock lock];
  while (aName == nil)
    {
      aName = [NSString stringWithFormat: @"%dlocalName", number++];
      if ([pasteboards objectForKey: aName] == nil)
	{
	  break;	// This name is unique.
	}
      else
	{
	  aName = nil;	// Name already in use - try another.
	}
    }

  pb = [pasteboards objectForKey: aName];
  if (pb == nil)
    {
      pb = [PasteboardObject alloc];
      pb->name = RETAIN(aName);
      pb->nextCount = 1;
      pb->histLength = 1;
      pb->history = [[NSMutableArray alloc] initWithCapacity: 2];
      pb->current = nil;
      [pasteboards setObject: pb forKey: aName];
      AUTORELEASE(pb);
    }
  [dictionary_lock unlock];
  return pb;
}

- (int) addTypes: (NSArray*)types
	   owner: (id)owner
      pasteboard: (NSPasteboard*)pb
	oldCount: (int)count
{
  PasteboardEntry *e = [self entryByCount: count];

  if ([owner isProxy] == YES)
    {
      Protocol	*p = @protocol(GSPasteboardCallback);

      [owner setProtocolForProxy: p];
    }

  if (e)
    {
      id	x = [xPbClass ownerByOsPb: name];

      [e addTypes: types owner: owner pasteboard: pb];

      /*
       * If there is an X pasteboard corresponding to this pasteboard, and the
       * X system doesn't currently own the pasteboard, we must inform it of
       * the change in the types of data supplied by this pasteboard.
       * We do this by simulating a change of pasteboard ownership.
       */
      if (x != owner && x != nil)
	[x pasteboardChangedOwner: pb];
      return count;
    }
  return 0;
}

- (NSString*) availableTypeFromArray: (NSArray*)types
			 changeCount: (int*)count
{
  PasteboardEntry *e = nil;

  if (*count <= 0)
    {
      e = current;
    }
  else
    {
      e = [self entryByCount: *count];
    }
  if (e)
    {
      unsigned	  i;

      *count = [e refNum];
      for (i = 0; i < [types count]; i++)
	{
	  NSString* key = [types objectAtIndex: i];

	  if ([e itemForType: key] != nil)
	    {
	      return key;
	    }
	}
    }
  return nil;
}

- (int) changeCount
{
  if (current)
    {
      return [current refNum];
    }
  return 0;
}

- (BOOL) checkConnection: (NSConnection*)c
{
  unsigned	i;
  BOOL		found = NO;

  for (i = 0; i < [history count]; i++)
    {
      if ([[history objectAtIndex: i] checkConnection: c] == YES)
	{
	  found = YES;
	}
    }
  return found;
}

- (NSData*) dataForType: (NSString*)type
	       oldCount: (int)count
	  mustBeCurrent: (BOOL)flag
{
  PasteboardEntry *e = nil;

  if (flag)
    {
      e = current;
    }
  else
    {
      e = [self entryByCount: count];
    }
  if (verbose)
    {
      NSLog(@"%@ get data for type '%@' version %d",
	self, type, e ? [e refNum] : -1);
    }
  if (e)
    {
      PasteboardData	*d = [e itemForType: type];

      if (d)
	{
	  return [d dataWithVersion: [e refNum]];
	}
    }
  return nil;
}

- (void) dealloc
{
  RELEASE(name);
  RELEASE(history);
  [super dealloc];
}

- (int) declareTypes: (bycopy NSArray*)types
	       owner: (id)owner
	  pasteboard: (NSPasteboard*)pb
{
  PasteboardEntry	*old = RETAIN(current);
  id			x = [xPbClass ownerByOsPb: name];

  if ([owner isProxy] == YES)
    {
      Protocol		*p = @protocol(GSPasteboardCallback);
      NSConnection	*c = [owner connectionForProxy];

      [owner setProtocolForProxy: p];

      /* If this is on a connection we don't know about, add it to our
       * list of pasteboard connections so that we can track its removal
       * in order to auto_stop if necessary.
       */
      if ([connections indexOfObjectIdenticalTo: c] == NSNotFound)
        {
	  [connections addObject: c];
	}
    }
  /*
   * If neither the new nor the old owner of the pasteboard is the X
   * pasteboard owner corresponding to this pasteboard, we will need
   * to inform the X owner of the change of ownership.
   */
  if (x == owner)
    x = nil;
  else if (x == [old owner])
    x = nil;

  current = [PasteboardEntry newWithTypes: types
				    owner: owner
				   pboard: pb
				      ref: nextCount++];
  [history addObject: current];
  RELEASE(current);
  if ([history count] > histLength)
    {
      [history removeObjectAtIndex: 0];
    }
  [old lostOwnership];
  RELEASE(old);
  /*
   * If there is an interested X pasteboard - inform it of the ownership
   * change.
   */
  if (x != nil)
    [x pasteboardChangedOwner: pb];
  if (verbose)
    {
      NSLog(@"%@ declare types '%@' version %d on %@ for %@",
	self, types, [current refNum], pb, owner);
    }
  return [current refNum];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ %@",
    [super description], name];
}

- (PasteboardEntry*) entryByCount: (int)count
{
  if (current == nil)
    {
      return nil;
    }
  else if ([current refNum] == count)
    {
      return current;
    }
  else
    {
      int i;

      for (i = 0; i < [history count]; i++)
	{
	  if ([[history objectAtIndex: i] refNum] == count)
	    {
	      return (PasteboardEntry*)[history objectAtIndex: i];
	    }
	}
      return nil;
    }
}

- (NSString*) name
{
  return name;
}

- (void) releaseGlobally
{
  if ([name isEqual: NSDragPboard]) return;
  if ([name isEqual: NSFindPboard]) return;
  if ([name isEqual: NSFontPboard]) return;
  if ([name isEqual: NSGeneralPboard]) return;
  if ([name isEqual: NSRulerPboard]) return;
  [pasteboards removeObjectForKey: name];
}

- (BOOL) setData: (NSData*)data
	 forType: (NSString*)type
	  isFile: (BOOL)flag
	oldCount: (int)count
{
  PasteboardEntry	*e;

  e = [self entryByCount: count];
  if (verbose)
    {
      NSLog(@"%@ set data %p for type '%@' version %d in %@",
	self, data, type, count, e);
    }
  if (e)
    {
      PasteboardData	*d;

      if (flag)
	{
	  d = [e itemForType: NSFileContentsPboardType];
	  if (d)
	    {
	      [d setData: data];
	    }
	  else
	    {
	      return NO;
	    }
	  if (type && [type isEqual: NSFileContentsPboardType] == NO)
	    {
	      d = [e itemForType: type];
	      if (d)
		{
		  [d setData: data];
		}
	      else
		{
		  return NO;
		}
	    }
	  return YES;
	}
      else if (type)
	{
	  d = [e itemForType: type];
	  if (d)
	    {
	      [d setData: data];
	      return YES;
	    }
	  else
	    {
	      return NO;
	    }
	}
      else
	{
	  return NO;
	}
    }
  else
    {
      return NO;
    }
}

- (void) setHistory: (unsigned)length
{
  if (length < 1) length = 1;
  if (length > MAXHIST) length = MAXHIST;

  histLength = length;
  if (length < histLength)
    {
      while ([history count] > histLength)
	{
	  [history removeObjectAtIndex: 0];
	}
    }
}

- (NSArray*) typesAndChangeCount: (int*)count
{
  PasteboardEntry *e = nil;

  if (*count <= 0)
    {
      e = current;
    }
  else
    {
      e = [self entryByCount: *count];
    }
  if (e)
    {
      *count = [e refNum];
      return [e types];
    }
  return nil;
}

@end





@interface PasteboardServer : NSObject <GSPasteboardSvr>
{
  NSMutableArray	*permenant;
}
- (BOOL) connection: (NSConnection*)ancestor
  shouldMakeNewConnection: (NSConnection*)newConn;
- (id) connectionBecameInvalid: (NSNotification*)notification;
- (id<GSPasteboardObj>) pasteboardWithName: (NSString*)name;
@end



@implementation PasteboardServer

- (BOOL) connection: (NSConnection*)ancestor
  shouldMakeNewConnection: (NSConnection*)newConn;
{
  [[NSNotificationCenter defaultCenter]
    addObserver: self
       selector: @selector(connectionBecameInvalid:)
	   name: NSConnectionDidDieNotification
	 object: newConn];
  [newConn setDelegate: self];
  return YES;
}

- (id) connectionBecameInvalid: (NSNotification*)notification
{
  id connection = [notification object];

  if (connection == conn)
    {
      NSLog(@"Help - pasteboard server connection has died!");
      exit(EXIT_FAILURE);
    }
  if ([connection isKindOfClass: [NSConnection class]])
    {
      NSEnumerator	*e = [pasteboards objectEnumerator];
      PasteboardObject	*o;

      while ((o = [e nextObject]) != nil)
	{
	  [o checkConnection: connection];
	}
    }
  [connections removeObjectIdenticalTo: connection];
  if (auto_stop == YES && [connections count] == 0)
    {
      exit(EXIT_SUCCESS);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(permenant);
  [super dealloc];
}

- (id) init
{
  self = [super init];
  if (self)
    {
      /*
       * Tell the NSPasteboard class to use us as the server so that the X
       * pasteboard owners can talk to us directly rather than over D.O.
       */
      [NSPasteboard _localServer: (id<GSPasteboardSvr>)self];

      /*
       *  Create all the pasteboards which must persist forever and add them
       *  to a local array.
       */
      permenant = [[NSMutableArray alloc] initWithCapacity: 5];
      [permenant addObject: [self pasteboardWithName: NSGeneralPboard]];
      [permenant addObject: [self pasteboardWithName: NSDragPboard]];
      [permenant addObject: [self pasteboardWithName: NSFontPboard]];
      [permenant addObject: [self pasteboardWithName: NSRulerPboard]];
      [permenant addObject: [self pasteboardWithName: NSFindPboard]];

      /*
       * Ensure that the OS pasteboard system is initialised.
       */
#if defined(__WIN32__) || defined(__CYGWIN__)
      xPbClass = NSClassFromString(@"Win32PbOwner");
#else      
      xPbClass = NSClassFromString(@"XPbOwner");
#endif

      /*
      If the OS pasteboard system fails to initialize, pretend that it isn't
      there. In practice, this happens if gpbs has been compiled with X
      support but is run on a display-less system. Note that
      +initializePasteboard will already have printed a warning in this case.
      */
      if (xPbClass && ![xPbClass initializePasteboard])
	xPbClass = nil;
    }
  return self;
}

- (id<GSPasteboardObj>) pasteboardWithName: (NSString*)name
{
  return [PasteboardObject pasteboardWithName: name];
}

@end



static void
ihandler(int sig)
{
  static BOOL	beenHere = NO;
  BOOL		action;
  const char	*e;

  /*
 * Prevent recursion.
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
  e = getenv("CRASH_ON_ABORT");
  if (e != 0)
    {
      if (strcasecmp(e, "yes") == 0 || strcasecmp(e, "true") == 0)
	action = YES;
      else if (strcasecmp(e, "no") == 0 || strcasecmp(e, "false") == 0)
	action = NO;
      else if (isdigit(*e) && *e != '0')
	action = YES;
      else
	action = NO;
    }

  if (action == YES)
    {
      abort();
    }
  else
    {
      fprintf(stderr, "gpbs killed by signal %d\n", sig);
      exit(sig);
    }
}

static void
init(int argc, char** argv, char **env)
{
  NSUserDefaults	*defs;
  NSProcessInfo		*pInfo;
  NSMutableArray	*args;
  unsigned		count;
  BOOL			shouldFork = YES;

  pInfo = [NSProcessInfo processInfo];
  args = AUTORELEASE([[pInfo arguments] mutableCopy]);
  for (count = 1; count < [args count]; count++)
    {
      NSString	*a = [args objectAtIndex: count];

      if ([a isEqualToString: @"--help"] == YES)
	{
	  printf("gpbs\n\n");
	  printf("GNU Pasteboard server\n");
	  printf("--help\tfor help\n");
	  printf("--no-fork\tavoid fork() to make debugging easy\n");
	  printf("--verbose\tMore verbose debug output\n");
	  exit(EXIT_SUCCESS);
	}
      else if ([a isEqualToString: @"--auto"] == YES)
	{
	  auto_stop = YES;
	}
      else if ([a isEqualToString: @"--daemon"] == YES)
	{
	  is_daemon = YES;
	  shouldFork = NO;
	}
      else if ([a isEqualToString: @"--no-fork"] == YES)
	{
	  shouldFork = NO;
	}
      else if ([a isEqualToString: @"--verbose"] == YES)
	{
	  verbose++;
	}
      else if ([a hasPrefix: @"-"] == YES)
	{
	  count++;	// Skip user default specification
	}
      else if ([a length] > 0)
	{
	  printf("gpbs - GNU Pasteboard server\n");
	  printf("I don't understand '%s'\n", [a cString]);
	  printf("--help	for help\n");
	  exit(EXIT_SUCCESS);
	}
    }

  for (count = 0; count < NSIG; count++)
    {
      if (count == SIGABRT) continue;
#ifdef	SIGPROF
      if (count == SIGPROF) continue;
#endif
      signal((int)count, ihandler);
    }
#ifdef SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif 
#ifdef SIGTTOU
  signal(SIGTTOU, SIG_IGN);
#endif 
#ifdef SIGTTIN
  signal(SIGTTIN, SIG_IGN);
#endif 
#ifdef SIGHUP
  signal(SIGHUP, SIG_IGN);
#endif 
  signal(SIGTERM, ihandler);

  if (shouldFork == YES)
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
	  gpbs_log(LOG_CRIT, [[localException description] UTF8String]);
	  DESTROY(t);
	}
      NS_ENDHANDLER
      exit(EXIT_FAILURE);
    }

  /*
   * Make gpbs logging go to syslog unless overridden by user.
   */
  defs = [NSUserDefaults standardUserDefaults];
  [defs registerDefaults: [NSDictionary dictionaryWithObjectsAndKeys:
    @"YES", @"GSLogSyslog", nil]];
}


int
main(int argc, char** argv, char **env)
{
  CREATE_AUTORELEASE_POOL(pool);
  NSString      *hostname;

#ifdef GS_PASS_ARGUMENTS
  [NSProcessInfo initializeWithArguments:argv count:argc environment:env];
#endif

  init(argc, argv, env);

  // [NSObject enableDoubleReleaseCheck: YES];

  server = [[PasteboardServer alloc] init];

  if (server == nil)
    {
      NSLog(@"Unable to create server object.");
      exit(EXIT_FAILURE);
    }

  /* Register a connection that provides the server object to the network */
  conn = [NSConnection defaultConnection];
  [conn setRootObject: server];
  [conn setDelegate: server];
  [[NSNotificationCenter defaultCenter]
    addObserver: server
       selector: @selector(connectionBecameInvalid:)
	   name: NSConnectionDidDieNotification
	 object: (id)conn];

  connections = [NSMutableArray new];

  hostname = [[NSUserDefaults standardUserDefaults] stringForKey: @"NSHost"];
  if ([hostname length] == 0
    ||  [[NSHost hostWithName: hostname] isEqual: [NSHost currentHost]] == YES)
    {
      if ([conn registerName: PBSNAME] == NO)
        {
          NSLog(@"Unable to register with name server.");
          exit(EXIT_FAILURE);
        }
    }
  else
    {
      NSHost            *host = [NSHost hostWithName: hostname];
      NSPort            *port = [conn receivePort];
      NSPortNameServer  *ns = [NSPortNameServer systemDefaultPortNameServer];
      NSArray           *a;
      unsigned          c;

      if (host == nil)
        {
          NSLog(@"gpbs - unknown NSHost argument  ... %@ - quitting.", hostname);
          exit(EXIT_FAILURE);
        }
      a = [host names];
      c = [a count];
      while (c-- > 0)
        {
          NSString      *name = [a objectAtIndex: c];

          name = [PBSNAME stringByAppendingFormat: @"-%@", name];
          if ([ns registerPort: port forName: name] == NO)
            {
            }
        }
      a = [host addresses];
      c = [a count];
      while (c-- > 0)
        {
          NSString      *name = [a objectAtIndex: c];

          name = [PBSNAME stringByAppendingFormat: @"-%@", name];
          if ([ns registerPort: port forName: name] == NO)
            {
            }
        }
    }

  if (verbose)
    {
      NSLog(@"GNU pasteboard server startup.");
    }

  if ([[NSUserDefaults standardUserDefaults]
	  stringForKey: @"GSStartupNotification"])
    {
      [[NSDistributedNotificationCenter defaultCenter]
	postNotificationName: [[NSUserDefaults standardUserDefaults]
				  stringForKey: @"GSStartupNotification"]
		      object: nil];
    }

  [[NSRunLoop currentRunLoop] run];
  RELEASE(server);
  RELEASE(pool);
  exit(EXIT_SUCCESS);
}

/*
 * The following dummy classe is here solely as a workaround for pre 3.3
 * versions of gcc where protocols didn't work properly unless implemented
 * in the source where the '@protocol()' directive is used.
 */
@interface NSPasteboardOwnerDummy : NSObject <GSPasteboardCallback>
- (void) pasteboard: (NSPasteboard*)pb
 provideDataForType: (NSString*)type;
- (void) pasteboard: (NSPasteboard*)pb
 provideDataForType: (NSString*)type
         andVersion:(int)v;
- (void) pasteboardChangedOwner: (NSPasteboard*)pb;
@end

@implementation NSPasteboardOwnerDummy
- (void) pasteboard: (NSPasteboard*)pb
 provideDataForType: (NSString*)type
{
}
- (void) pasteboard: (NSPasteboard*)pb
 provideDataForType: (NSString*)type
         andVersion:(int)v
{
}
- (void) pasteboardChangedOwner: (NSPasteboard*)pb
{
}
@end

