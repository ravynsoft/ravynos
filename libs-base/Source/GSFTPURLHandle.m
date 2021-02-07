/** GSFTPURLHandle.m - Class GSFTPURLHandle
   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by:	Richard Frith-Macdonald <rfm@gnu.org>
   Date:	June 2002 		

   This file is part of the GNUstep Library.

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
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSURLHandle.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSRunLoop.h"
#import "Foundation/NSByteOrder.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSFileHandle.h"
#import "GNUstepBase/GSMime.h"
#import "GSPrivate.h"

GS_EXPORT NSString * const GSTelnetNotification;
GS_EXPORT NSString * const GSTelnetErrorKey;
GS_EXPORT NSString * const GSTelnetTextKey;

@interface	GSTelnetHandle : NSObject
{
  NSStringEncoding	enc;
  NSFileHandle		*remote;
  NSMutableData		*ibuf;
  unsigned		pos;
  BOOL			lineMode;
  BOOL			connected;
}
- (id) initWithHandle: (NSFileHandle*)handle isConnected: (BOOL)flag;
- (void) putTelnetLine: (NSString*)s;
- (void) putTelnetText: (NSString*)s;
- (void) setEncoding: (NSStringEncoding)e;
- (void) setLineMode: (BOOL)flag;
@end



@interface	GSTelnetHandle (Private)
- (void) _didConnect: (NSNotification*)notification;
- (void) _didRead: (NSNotification*)notification;
- (void) _didWrite: (NSNotification*)notification;
@end

NSString * const GSTelnetNotification = @"GSTelnetNotification";
NSString * const GSTelnetErrorKey = @"GSTelnetErrorKey";
NSString * const GSTelnetTextKey = @"GSTelnetTextKey";

@implementation	GSTelnetHandle

#define	WILL	251
#define	WONT	252
#define	DO	253
#define	DONT	254
#define	IAC	255

- (void) dealloc
{
  NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];

  [nc removeObserver: self];
  RELEASE(remote);
  RELEASE(ibuf);
  [super dealloc];
}

- (id) init
{
  return [self initWithHandle: nil isConnected: NO];
}

- (id) initWithHandle: (NSFileHandle*)handle isConnected: (BOOL)flag
{
  if (handle == nil)
    {
      DESTROY(self);
    }
  else
    {
      NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];

      connected = flag;
      enc = NSUTF8StringEncoding;
      ibuf = [NSMutableData new];
      remote = RETAIN(handle);
      if (connected == YES)
	{
	  [nc addObserver: self
		 selector: @selector(_didRead:)
		     name: NSFileHandleReadCompletionNotification
		   object: remote];
	  [nc addObserver: self
		 selector: @selector(_didWrite:)
		     name: GSFileHandleWriteCompletionNotification
		   object: remote];
	  [remote readInBackgroundAndNotify];
	}
      else
	{
	  [nc addObserver: self
		 selector: @selector(_didConnect:)
		     name: GSFileHandleConnectCompletionNotification
		   object: remote];
	}
    }
  return self;
}

- (void) putTelnetLine: (NSString*)s
{
  if ([s hasSuffix: @"\n"] == NO)
    {
      s = [s stringByAppendingString: @"\r\n"];
    }
  [self putTelnetText: s];
}

- (void) putTelnetText: (NSString*)s
{
  NSMutableData	*md;
  unsigned char	*to;
  NSData	*d = [s dataUsingEncoding: enc];
  unsigned char	*from = (unsigned char *)[d bytes];
  unsigned int	len = [d length];
  unsigned int	i = 0;
  unsigned int	count = 0;

  for (i = 0; i < len; i++)
    {
      if (from[i] == IAC)
	{
	  count++;
	}
    }

  md = [[NSMutableData alloc] initWithLength: len + count];
  to = [md mutableBytes];
  for (i = 0; i < len; i++)
    {
      if (*from == IAC)
	{
	  *to++ = IAC;
	}
      *to++ = *from++;
    }
//NSLog(@"Write - '%*.*s'", [md length], [md length], [md bytes]);
  [remote writeInBackgroundAndNotify: md];
  DESTROY(md);
}

/**
 * Set the string encoding used to convert strings to be sent to the
 * remote system into raw data, and to convert incoming data from that
 * system inot input text.
 */
- (void) setEncoding: (NSStringEncoding)e
{
  enc = e;
}

/**
 * Sets a flag to say whether observers are to be notified of incoming
 * data after every chunk read, or only when one or more entire lines
 * are read. <br />
 * When switching out of line mode, this will cause a notification to be
 * generated if there is any buffered data available for use.
 */
- (void) setLineMode: (BOOL)flag
{
  if (lineMode != flag)
    {
      lineMode = flag;
      if (lineMode == NO)
	{
	  [self _didRead: nil];
	}
    }
}

@end

@implementation	GSTelnetHandle (Private)
- (void) _didConnect: (NSNotification*)notification
{
  NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];
  NSDictionary		*info = [notification userInfo];
  NSString		*e;

  e = [info objectForKey: GSFileHandleNotificationError];
  if (e == nil)
    {
      [nc removeObserver: self
		    name: GSFileHandleConnectCompletionNotification
		  object: [notification object]];
      [nc addObserver: self
	     selector: @selector(_didRead:)
		 name: NSFileHandleReadCompletionNotification
	       object: remote];
      [nc addObserver: self
	     selector: @selector(_didWrite:)
		 name: GSFileHandleWriteCompletionNotification
	       object: remote];
      [remote readInBackgroundAndNotify];
    }
  else
    {
      info = [NSDictionary dictionaryWithObject: e
					 forKey: GSTelnetErrorKey];
      [nc postNotificationName: GSTelnetNotification
			object: self
		      userInfo: info];
    }
}

- (void) _didRead: (NSNotification*)notification
{
  NSDictionary		*userInfo = [notification userInfo];
  NSMutableArray	*text = nil;
  NSData		*d;

  d = [userInfo objectForKey: NSFileHandleNotificationDataItem];
  /*
   * If the notification is nil, this method has been called to flush
   * any buffered data out.
   */
  if (notification != nil && (d == nil || [d length] == 0))
    {
      NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];
      NSDictionary		*info;

      info = [NSDictionary dictionaryWithObject: @"EOF"
					 forKey: GSTelnetErrorKey];
      [nc postNotificationName: GSTelnetNotification
			object: self
		      userInfo: info];
    }
  else
    {
      NSMutableData	*toWrite = nil;
      unsigned char	*ptr;
      unsigned char	c;
      unsigned int	s = 0;
      int		old;
      int		len;
      int		i;	// May be negative.

      if (d != nil)
	{
//  NSLog(@"Read - '%@'", d);
	  [ibuf appendData: d];
	}
      old = len = (int)[ibuf length];
      ptr = [ibuf mutableBytes];

      for (i = pos; i < len; i++)
	{
	  NSData	*line = nil;

	  c = ptr[i];
	  if (c == IAC)
	    {
	      if (i < len - 1)
		{
		  c = ptr[i+1];
		  if (c == WILL || c == WONT || c == DO || c == DONT)
		    {
		      /*
		       * refuse any negotiation attempts.
		       */
		      if (c == WILL || c == DO)
			{
			  unsigned char	opt[3];

			  if (toWrite == nil)
			    {
			      toWrite = [NSMutableData alloc];
			      toWrite = [toWrite initWithCapacity: 12];
			    }
			  opt[0] = IAC;
			  if (c == DO)
			    {
			      opt[1] = WONT;
			    }
			  else
			    {
			      opt[1] = DONT;
			    }
			  opt[2] = ptr[i+2];
			  [toWrite appendBytes: opt length: 3];
			}
		      if (i < len - 2)
			{
// NSLog(@"Command: %d %d", ptr[1], ptr[2]);
			  len -= 3;
			  if (len - i > 0)
			    {
			      memmove(ptr, &ptr[3], len - i);
			    }
			  i--;			// Try again.
			}
		      else
			{
			  i--;
			  break;		// Need more data
			}
		    }
		  else if (c == IAC)		// Escaped IAC
		    {
		      len--;
		      if (len - i > 0)
			{
			  memmove(ptr, &ptr[1], len - i);
			}
		    }
		  else
		    {
// NSLog(@"Command: %d", ptr[1]);
		      /*
		       * Strip unimplemented escape sequence.
		       */
		      len -= 2;
		      if (len - i > 0)
			{
			  memmove(ptr, &ptr[2], len - i);
			}
		      i--;	// Try again from here.
		    }
		}
	      else
		{
		  i--;
		  break;	// Need more data
		}
	    }
	  else if (c == '\r' && (int)i < len - 1 && ptr[i+1] == '\n')
	    {
	      line = [[NSData alloc] initWithBytes: &ptr[s] length: i-s+2];
	      i++;
	      s = i + 1;
	    }
	  else if (c == '\n')
	    {
	      line = [[NSData alloc] initWithBytes: &ptr[s] length: i-s+1];
	      s = i + 1;
	    }
	  if (line != nil)
	    {
	      NSString	*lineString;

	      lineString = [[NSString alloc] initWithData: line encoding: enc];
	      DESTROY(line);
	      if (text == nil)
		{
		  text = [[NSMutableArray alloc] initWithCapacity: 4];
		}
	      [text addObject: lineString];
	      DESTROY(lineString);
	    }
	}
      pos = i;

      /*
       * If not in line mode, we can add the remainder of the data to
       * the array of strings for notification.
       */
      if (lineMode == NO && s != pos)
	{
	  NSString	*lineString;
	  NSData	*line;

	  line = [[NSData alloc] initWithBytes: &ptr[s] length: pos - s];
	  s = pos;
	  lineString = [[NSString alloc] initWithData: line encoding: enc];
	  DESTROY(line);
	  if (text == nil)
	    {
	      text = [[NSMutableArray alloc] initWithCapacity: 4];
	    }
	  [text addObject: lineString];
	  DESTROY(lineString);
	}

      /*
       * Adjust size of data remaining in buffer if necessary.
       */
      if (old != len || s > 0)
	{
	  if (s > 0)
	    {
	      len -= s;
	      pos -= s;
	      if (len > 0)
		{
		  memmove(ptr, &ptr[s], len);
		}
	    }
	  [ibuf setLength: len];
	}

      /*
       * Send telnet protocol negotion info if necessary.
       */
      if (toWrite != nil)
	{
// NSLog(@"Write - '%@'", toWrite);
	  [remote writeInBackgroundAndNotify: toWrite];
	  DESTROY(toWrite);
	}

      /*
       * Send notification for text read as necessary.
       */
      if (text != nil)
	{
	  NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];
	  NSNotification	*n;
	  NSDictionary		*info;

	  info = [NSDictionary dictionaryWithObject: text
					     forKey: GSTelnetTextKey];
	  DESTROY(text);
	  n = [NSNotification notificationWithName: GSTelnetNotification
					    object: self
					  userInfo: info];
	  [nc postNotification: n];
	}
      [remote readInBackgroundAndNotify];
    }
}

- (void) _didWrite: (NSNotification*)notification
{
  NSDictionary	*userInfo = [notification userInfo];
  NSString	*e;

  e = [userInfo objectForKey: GSFileHandleNotificationError];
  if (e)
    {
      NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];
      NSDictionary		*info;

      info = [NSDictionary dictionaryWithObject: e
					 forKey: GSTelnetErrorKey];
      [nc postNotificationName: GSTelnetNotification
			object: self
		      userInfo: info];
    }
}
@end



@interface GSFTPURLHandle : NSURLHandle
{
  GSTelnetHandle	*cHandle;
  NSFileHandle          *dHandle;
  NSURL                 *url;
  NSData		*wData;
  NSString		*term;
  enum {
    idle,
    cConnect,		// Establishing control connection
    sentUser,		// Sent username
    sentPass,		// Sent password
    sentType,		// Sent data type
    sentPasv,		// Requesting host/port information for data link
    data,		// Establishing or using data connection
    list,		// listing directory
  } state;
}
- (void) _control: (NSNotification*)n;
- (void) _data: (NSNotification*)n;
@end

/**
 * <p>
 *   This is a <em>PRIVATE</em> subclass of NSURLHandle.
 *   It is documented here in order to give you information about the
 *   default behavior of an NSURLHandle created to deal with a URL
 *   that has the <code>ftp</code> scheme.
 *   The name and/or other implementation details of this class
 *   may be changed at any time.
 * </p>
 * <p>
 *   A GSFTPURLHandle instance is used to manage connections to
 *   <code>ftp</code> URLs.
 * </p>
 */
@implementation GSFTPURLHandle

static NSMutableDictionary	*urlCache = nil;
static NSLock			*urlLock = nil;

+ (NSURLHandle*) cachedHandleForURL: (NSURL*)newUrl
{
  NSURLHandle	*obj = nil;

  if ([[newUrl scheme] caseInsensitiveCompare: @"ftp"] == NSOrderedSame)
    {
      NSString	*page = [newUrl absoluteString];
// NSLog(@"Lookup for handle for '%@'", page);
      [urlLock lock];
      obj = [urlCache objectForKey: page];
      IF_NO_GC([[obj retain] autorelease];)
      [urlLock unlock];
// NSLog(@"Found handle %@", obj);
    }
  return obj;
}

+ (void) initialize
{
  if (self == [GSFTPURLHandle class])
    {
      urlCache = [NSMutableDictionary new];
      [[NSObject leakAt: &urlCache] release];
      urlLock = [NSLock new];
      [[NSObject leakAt: &urlLock] release];
    }
}

- (void) dealloc
{
  if (state != idle)
    {
      [self endLoadInBackground];
    }
  RELEASE(url);
  RELEASE(wData);
  RELEASE(term);
  [super dealloc];
}

- (id) initWithURL: (NSURL*)newUrl
	    cached: (BOOL)cached
{
  if ((self = [super initWithURL: newUrl cached: cached]) != nil)
    {
      ASSIGN(url, newUrl);
      state = idle;
      if (cached == YES)
        {
	  NSString	*page = [newUrl absoluteString];

	  [urlLock lock];
	  [urlCache setObject: self forKey: page];
	  [urlLock unlock];
// NSLog(@"Cache handle %@ for '%@'", self, page);
	}
    }
  return self;
}

+ (BOOL) canInitWithURL: (NSURL*)newUrl
{
  if ([[newUrl scheme] isEqualToString: @"ftp"] == YES)
    {
      return YES;
    }
  return NO;
}

- (void) _control: (NSNotification*)n
{
  NSDictionary		*info = [n userInfo];
  NSString		*e = [info objectForKey: GSTelnetErrorKey];
  NSArray		*text;
  NSString		*line;

  if (e == nil)
    {
      NSEnumerator	*enumerator;

      text = [info objectForKey: GSTelnetTextKey];
// NSLog(@"Ctl: %@", text);
      /* Find first reply line which is not a continuation of another.
       */
      enumerator = [text objectEnumerator];
      while ((line = [enumerator nextObject]) != nil)
	{
	  if (term == nil)
	    {
	      if ([line length] > 4)
		{
		  char	buf[4];	

		  buf[0] = (char)[line characterAtIndex: 0];
		  buf[1] = (char)[line characterAtIndex: 1];
		  buf[2] = (char)[line characterAtIndex: 2];
		  buf[3] = (char)[line characterAtIndex: 3];
		  if (isdigit(buf[0]) && isdigit(buf[1]) && isdigit(buf[2]))
		    {
		      if (buf[3] == '-')
			{
			  /* Got start of a multiline block ...
			   * set the terminator we need to look for.
			   */
			  buf[3] = ' ';
			  term = [[NSString alloc]
			    initWithCString: buf length: 4];
			}
		      else if (buf[3] == ' ')
			{
			  /* Found single line response.
			   */
			  break;
			}
		    }
		}
	    }
	  else if ([line hasPrefix: term] == YES)
	    {
	      /* Found end of a multiline response.
	       */
	      DESTROY(term);
	      break;
	    }
	}
      if (line == nil)
	{
	  return;
	}

      if (state == cConnect)
	{
	  if ([line hasPrefix: @"2"] == YES)
	    {
	      NSString	*user = [url user];

	      if (user == nil)
		{
		  user = @"anonymous";
		}
	      [cHandle putTelnetLine: [@"USER " stringByAppendingString: user]];
	      state = sentUser;
	    }
	  else
	    {
	      e = line;
	    }
	}
      else if (state == sentUser)
	{
	  if ([line hasPrefix: @"230"] == YES)	// No password required
	    {
	      [cHandle putTelnetLine: @"TYPE I"];
	      state = sentType;
	    }
	  else if ([line hasPrefix: @"331"] == YES)
	    {
	      NSString	*pass = [url password];

	      if (pass == nil)
		{
		  pass = [url user];
		  if (pass == nil)
		    {
		      pass = @"GNUstep@here";
		    }
		  else
		    {
		      pass = @"";
		    }
		}
	      [cHandle putTelnetLine: [@"PASS " stringByAppendingString: pass]];
	      state = sentPass;
	    }
	  else
	    {
	      e = line;
	    }
	}
      else if (state == sentPass)
	{
	  if ([line hasPrefix: @"2"] == YES)
	    {
	      [cHandle putTelnetLine: @"TYPE I"];
	      state = sentType;
	    }
	  else
	    {
	      e = line;
	    }
	}
      else if (state == sentType)
	{
	  if ([line hasPrefix: @"2"] == YES)
	    {
	      [cHandle putTelnetLine: @"PASV"];
	      state = sentPasv;
	    }
	  else
	    {
	      e = line;
	    }
	}
      else if (state == sentPasv)
	{
	  if ([line hasPrefix: @"227"] == YES)
	    {
	      NSRange	r = [line rangeOfString: @"("];
	      NSString	*h = nil;
	      NSString	*p = nil;

	      if (r.length > 0)
		{
		  unsigned	pos = NSMaxRange(r);

		  r = [line rangeOfString: @")"];
		  if (r.length > 0 && r.location > pos)
		    {
		      NSArray	*a;

		      r = NSMakeRange(pos, r.location - pos);
		      line = [line substringWithRange: r];
		      a = [line componentsSeparatedByString: @","];
		      if ([a count] == 6)
			{
			  h = [NSString stringWithFormat: @"%@.%@.%@.%@",
			    [a objectAtIndex: 0], [a objectAtIndex: 1],
			    [a objectAtIndex: 2], [a objectAtIndex: 3]];
			  p = [NSString stringWithFormat: @"%d",
			    [[a objectAtIndex: 4] intValue] * 256
			    + [[a objectAtIndex: 5] intValue]];
			}
		    }
		}
	      if (h == nil)
		{
		  e = @"Invalid host/port information for pasv command";
		}
	      else
		{
		  NSNotificationCenter	*nc;

		  dHandle = [NSFileHandle
		    fileHandleAsClientInBackgroundAtAddress: h service: p
		    protocol: @"tcp"];
      		  IF_NO_GC([dHandle retain];)
		  nc = [NSNotificationCenter defaultCenter];
		  [nc addObserver: self
			 selector: @selector(_data:)
			     name: GSFileHandleConnectCompletionNotification
			   object: dHandle];
		  state = data;
		}
	    }
	  else
	    {
	      e = line;
	    }
	}
      else if (state == data)
	{
	  if ([line hasPrefix: @"550"] == YES && wData == nil)
	    {
	      state = list;
	      [cHandle putTelnetLine:
		[NSString stringWithFormat: @"NLST %@", [url path]]];
	    }
	  else if ([line hasPrefix: @"1"] == NO && [line hasPrefix: @"2"] == NO)
	    {
	      e = line;
	    }
	}
      else if (state == list)
	{
	  if ([line hasPrefix: @"550"] == YES)
	    {
	      NSRange	r = [line rangeOfString: @"not a plain file"];

	      /*
	       * Some servers may return an error on listing even though
	       * the path was a valid directory.  We try to catch some of
	       * those cases and produce an empty listing instead.
	       */
	      if (r.location > 0)
		{
		  NSNotificationCenter	*nc;

		  nc = [NSNotificationCenter defaultCenter];
		  if (dHandle != nil)
		    {
		      [nc removeObserver: self name: nil object: dHandle];
		      [dHandle closeFile];
		      DESTROY(dHandle);
		    }
		  [nc removeObserver: self
				name: GSTelnetNotification
			      object: cHandle];
		  DESTROY(cHandle);
		  state = idle;
		  [self didLoadBytes: [NSData data] loadComplete: YES];
		}
	      else
		{
		  e = line;
		}
	    }
	  else if ([line hasPrefix: @"1"] == NO && [line hasPrefix: @"2"] == NO)
	    {
	      e = line;
	    }
	}
      else
	{
	  e = @"Message in unknown state";
	}
    }
  if (e != nil)
    {
      /*
       * Tell superclass that the load failed - let it do housekeeping.
       */
      [self endLoadInBackground];
      [self backgroundLoadDidFailWithReason: e];
    }
}

- (void) _data: (NSNotification*)n
{
  NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];
  NSString		*name = [n name];
  NSDictionary          *info = [n userInfo];
  NSString		*e = [info objectForKey: GSFileHandleNotificationError];

// NSLog(@"_data: %@", n);
  [nc removeObserver: self name: name object: dHandle];

  /*
   * See if the connection attempt caused an error.
   */
  if (e != nil)
    {
      if ([name isEqualToString: GSFileHandleConnectCompletionNotification])
	{
	  NSLog(@"Unable to connect to %@:%@ via socket ... %@",
	    [dHandle socketAddress], [dHandle socketService], e);
	}
// NSLog(@"Fail - %@", e);
      /*
       * Tell superclass that the load failed - let it do housekeeping.
       */
      [self endLoadInBackground];
      [self backgroundLoadDidFailWithReason: e];
      return;
    }
  if ([name isEqualToString: GSFileHandleConnectCompletionNotification])
    {
      if (wData == nil)
	{
	  [cHandle putTelnetLine:
	    [NSString stringWithFormat: @"RETR %@", [url path]]];
	  [nc addObserver: self
		 selector: @selector(_data:)
		     name: NSFileHandleReadCompletionNotification
		   object: dHandle];
	  [dHandle readInBackgroundAndNotify];
	}
      else
	{
	  [cHandle putTelnetLine:
	    [NSString stringWithFormat: @"STOR %@", [url path]]];
	  [nc addObserver: self
		 selector: @selector(_data:)
		     name: GSFileHandleWriteCompletionNotification
		   object: dHandle];
	  [dHandle writeInBackgroundAndNotify: wData];
	}
    }
  else
    {
      if (wData == nil)
	{
	  NSData	*d;

	  d = [info objectForKey: NSFileHandleNotificationDataItem];
	  if ([d length] > 0)
	    {
	      [self didLoadBytes: d loadComplete: NO];
	      [nc addObserver: self
		     selector: @selector(_data:)
			 name: NSFileHandleReadCompletionNotification
		       object: dHandle];
	      [dHandle readInBackgroundAndNotify];
	    }
	  else
	    {
	      NSNotificationCenter	*nc;

	      nc = [NSNotificationCenter defaultCenter];
	      if (dHandle != nil)
		{
		  [nc removeObserver: self name: nil object: dHandle];
		  [dHandle closeFile];
		  DESTROY(dHandle);
		}
	      [nc removeObserver: self
			    name: GSTelnetNotification
			  object: cHandle];
	      DESTROY(cHandle);
	      state = idle;

	      [self didLoadBytes: d loadComplete: YES];
	    }
	}
      else
	{
	  NSNotificationCenter	*nc;
	  NSData		*tmp;

	  nc = [NSNotificationCenter defaultCenter];
	  if (dHandle != nil)
	    {
	      [nc removeObserver: self name: nil object: dHandle];
	      [dHandle closeFile];
	      DESTROY(dHandle);
	    }
	  [nc removeObserver: self
			name: GSTelnetNotification
		      object: cHandle];
	  DESTROY(cHandle);
	  state = idle;

	  /*
	   * Tell superclass that we have successfully loaded the data.
	   */
	  tmp = wData;
	  wData = nil;
	  [self didLoadBytes: tmp loadComplete: YES];
	  DESTROY(tmp);
	}
    }
}

- (void) endLoadInBackground
{
  if (state != idle)
    {
      NSNotificationCenter	*nc = [NSNotificationCenter defaultCenter];

      if (dHandle != nil)
	{
	  [nc removeObserver: self name: nil object: dHandle];
	  [dHandle closeFile];
	  DESTROY(dHandle);
	}
      [nc removeObserver: self name: GSTelnetNotification object: cHandle];
      DESTROY(cHandle);
      state = idle;
    }
  [super endLoadInBackground];
}

- (void) loadInBackground
{
  NSNotificationCenter	*nc;
  NSString		*host = nil;
  NSString		*port = nil;
  NSNumber	*p;
  NSFileHandle		*sock;

  /*
   * Don't start a load if one is in progress.
   */
  if (state != idle)
    {
      NSLog(@"Attempt to load an ftp handle which is not idle ... ignored");
      return;
    }

  [self beginLoadInBackground];
  host = [url host];
  p = [url port];
  if (p != nil)
    {
      port = [NSString stringWithFormat: @"%u", [p unsignedIntValue]];
    }
  else
    {
      port = [url scheme];
    }
  sock = [NSFileHandle fileHandleAsClientInBackgroundAtAddress: host
						       service: port
						      protocol: @"tcp"];
  if (sock == nil)
    {
      /*
       * Tell superclass that the load failed - let it do housekeeping.
       */
      [self backgroundLoadDidFailWithReason: [NSString stringWithFormat:
	@"Unable to connect to %@:%@ ... %@",
	host, port, [NSError _last]]];
      return;
    }
  cHandle = [[GSTelnetHandle alloc] initWithHandle: sock isConnected: NO];
  nc = [NSNotificationCenter defaultCenter];
  [nc addObserver: self
         selector: @selector(_control:)
             name: GSTelnetNotification
           object: cHandle];
  state = cConnect;
}

/**
 * We cannot get/set any properties for FTP
 */
- (id) propertyForKey: (NSString*)propertyKey
{
  return nil;
}

/**
 * We cannot get/set any properties for FTP
 */
- (id) propertyForKeyIfAvailable: (NSString*)propertyKey
{
  return nil;
}

/**
 * Sets the specified data to be written to the URL on the next load.
 */
- (BOOL) writeData: (NSData*)data
{
  ASSIGN(wData, data);
  return YES;
}

/**
 * We cannot get/set any properties for FTP
 */
- (BOOL) writeProperty: (id)propertyValue
		forKey: (NSString*)propertyKey
{
  return NO;
}

@end

