/** Interface for NSLog for GNUStep
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: November 1996

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

   <title>NSLog reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSCalendarDate.h"
#import "Foundation/NSTimeZone.h"
#import "Foundation/NSException.h"
#import "Foundation/NSProcessInfo.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSData.h"
#import "Foundation/NSThread.h"
#import "GNUstepBase/NSString+GNUstepBase.h"

// Some older BSD systems used a non-standard range of thread priorities.
#ifdef	HAVE_SYSLOG_H
#include <syslog.h>
#elif HAVE_SYS_SLOG_H
  /* we are on a QNX-ish system, which has a syslog symbol somewhere,
   * but it isn't our syslog function (we use slogf().)
   */
# ifdef HAVE_SYSLOG
#   undef HAVE_SYSLOG
# endif
# include <sys/slog.h>
# ifdef HAVE_SYS_SLOGCODES_H
#   include <sys/slogcodes.h>
# endif
#endif

#define	UNISTR(X) \
((const unichar*)[(X) cStringUsingEncoding: NSUnicodeStringEncoding])

#if	defined(HAVE_SYSLOG)
# if	defined(LOG_ERR)
#   if	defined(LOG_USER)
#     define	SYSLOGMASK	(LOG_ERR|LOG_USER)
#   else
#     define	SYSLOGMASK	(LOG_ERR)
#   endif	// LOG_USER
# elif	defined(LOG_ERROR)
#   if	defined(LOG_USER)
#     define	SYSLOGMASK	(LOG_ERROR|LOG_USER)
#   else
#     define	SYSLOGMASK	(LOG_ERROR)
#   endif	// LOG_USER
# else
#   error "Help, I can't find a logging level for syslog"
# endif
#endif	// HAVE_SYSLOG

#import "GSPrivate.h"

extern NSThread	*GSCurrentThread();

/**
 * A variable holding the file descriptor to which NSLogv() messages are
 * written by default.  GNUstep initialises this to stderr.<br />
 * You may change this, but for thread safety should
 * use the lock provided by GSLogLock() to protect the change.
 */
int _NSLogDescriptor = 2;

static NSRecursiveLock	*myLock = nil;
static IMP              lockImp = 0;
static IMP              unlockImp = 0;

/**
 * Returns the lock used to protect the GNUstep NSLogv() implementation.
 * Use this to protect changes to
 * <ref type="variable" id="_NSLogDescriptor">_NSLogDescriptor</ref> and
 * <ref type="variable" id="_NSLog_printf_handler">_NSLog_printf_handler</ref>
 */
NSRecursiveLock *
GSLogLock()
{
  if (myLock == nil)
    {
      [gnustep_global_lock lock];
      if (myLock == nil)
	{
	  myLock = [NSRecursiveLock new];
          lockImp = [myLock methodForSelector: @selector(lock)];
          unlockImp = [myLock methodForSelector: @selector(unlock)];
	}
      [gnustep_global_lock unlock];
    }
  return myLock;
}

static void
_NSLog_standard_printf_handler(NSString* message)
{
  NSData	*d;
  const char	*buf;
  unsigned	len;
#if	defined(_WIN32)
  LPCWSTR	null_terminated_buf;
#else
#if	defined(HAVE_SYSLOG) || defined(HAVE_SLOGF)
  char	*null_terminated_buf = NULL;
#endif
#endif
  static NSStringEncoding enc = 0;

  if (enc == 0)
    {
      enc = [NSString defaultCStringEncoding];
    }
  d = [message dataUsingEncoding: enc allowLossyConversion: NO];
  if (d == nil)
    {
      d = [message dataUsingEncoding: NSUTF8StringEncoding
		allowLossyConversion: NO];
    }

  if (d == nil)		// Should never happen.
    {
      buf = [message lossyCString];
      len = strlen(buf);
    }
  else
    {
      buf = (const char*)[d bytes];
      len = [d length];
    }

#if	defined(_WIN32)
  null_terminated_buf = UNISTR(message);

  OutputDebugStringW(null_terminated_buf);

  if ((GSPrivateDefaultsFlag(GSLogSyslog) == YES
    || write(_NSLogDescriptor, buf, len) != (int)len) && !IsDebuggerPresent())
    {
      static HANDLE eventloghandle = 0;

      if (!eventloghandle)
	{
	  eventloghandle = RegisterEventSourceW(NULL,
	    UNISTR([[NSProcessInfo processInfo] processName]));
	}
      if (eventloghandle)
	{
	  ReportEventW(eventloghandle,	// event log handle
	    EVENTLOG_WARNING_TYPE,	// event type
	    0,				// category zero
	    0,				// event identifier
	    NULL,			// no user security identifier
	    1,				// one substitution string
	    0,				// no data
	    &null_terminated_buf,	// pointer to string array
	    NULL);			// pointer to data
	}
    }
#else

#if	defined(HAVE_SYSLOG)
  if (GSPrivateDefaultsFlag(GSLogSyslog) == YES
    || write(_NSLogDescriptor, buf, len) != (int)len)
    {
      null_terminated_buf = malloc(sizeof (char) * (len + 1));
      strncpy (null_terminated_buf, buf, len);
      null_terminated_buf[len] = '\0';

      syslog(SYSLOGMASK, "%s",  null_terminated_buf);

      free(null_terminated_buf);
    }
#elif defined(HAVE_SLOGF)
  if (GSPrivateDefaultsFlag(GSLogSyslog) == YES
    || write(_NSLogDescriptor, buf, len) != (int)len)
    {
      /* QNX's slog has a size limit per entry. We might need to iterate over
       * _SLOG_MAXSIZEd chunks of the buffer
       */
      const char *newBuf = buf;
      unsigned newLen = len;

      // Allocate at most _SLOG_MAXSIZE bytes
      null_terminated_buf = malloc(sizeof(char) * MIN(newLen, _SLOG_MAXSIZE));
      // If it's shorter than that, we never even enter the loop
      while (newLen >= _SLOG_MAXSIZE)
        {
          strncpy(null_terminated_buf, newBuf, (_SLOG_MAXSIZE - 1));
          null_terminated_buf[_SLOG_MAXSIZE] = '\0';
          slogf(_SLOG_SETCODE(_SLOG_SYSLOG, 0), _SLOG_ERROR, "%s",
            null_terminated_buf);
          newBuf += (_SLOG_MAXSIZE - 1);
          newLen -= (_SLOG_MAXSIZE - 1);
        }
      /* Write out the rest (which will be at most (_SLOG_MAXSIZE - 1) chars,
       * so the terminator still fits.
       */
      if (0 != newLen)
        {
          strncpy(null_terminated_buf, newBuf, newLen);
          null_terminated_buf[newLen] = '\0';
          slogf(_SLOG_SETCODE(_SLOG_SYSLOG, 0), _SLOG_ERROR, "%s",
            null_terminated_buf);
        }
      free(null_terminated_buf);
    }
#else
  write(_NSLogDescriptor, buf, len);
#endif
#endif // _WIN32
}

/**
 * A pointer to a function used to actually write the log data.
 * <p>
 *   GNUstep initialises this to a function implementing the standard
 *   behavior for logging, but you may change this in your program
 *   in order to implement any custom behavior you wish.  You should
 *   use the lock returned by GSLogLock() to protect any change you make.
 * </p>
 * <p>
 *   Calls from NSLogv() to the function pointed to by this variable
 *   are protected by a lock, and should therefore be thread safe.
 * </p>
 * <p>
 *   This function should accept a single NSString argument and return void.
 * </p>
 * The default implementation in GNUstep performs as follows -
 * <list>
 *   <item>
 *     Converts the string to be logged to data in the default CString
 *     encoding or, if that is not possible, to UTF8 data.
 *   </item>
 *   <item>
 *     If the system supports writing to syslog and the user default to
 *     say that logging should be done to syslog (GSLogSyslog) is set,
 *     writes the data to the syslog.<br />
 *     On an mswindows system, where syslog is not available, the
 *     GSLogSyslog user default controls whether or not data is written
 *     to the system event log,
 *   </item>
 *   <item>
 *     Otherwise, writes the data to the file descriptor stored in the
 *     variable
 *     <ref type="variable" id="_NSLogDescriptor">_NSLogDescriptor</ref>,
 *     which is set by default to stderr.<br />
 *     Your program may change this descriptor ... but you should protect
 *     changes using the lock provided by GSLogLock().<br />
 *     NB. If the write to the descriptor fails, and the system supports
 *     writing to syslog, then the log is written to syslog as if the
 *     appropriate user default had been set.
 *   </item>
 * </list>
 */
NSLog_printf_handler *_NSLog_printf_handler = _NSLog_standard_printf_handler;

/**
 * <p>Provides the standard OpenStep logging facility.  For details see
 * the lower level NSLogv() function (which this function uses).
 * </p>
 * <p>GNUstep provides powerful alternatives for logging ... see
 * NSDebugLog(), NSWarnLog() and GSPrintf() for example.  We recommend
 * the use of NSDebugLog() and its relatives for debug purposes, and
 * GSPrintf() for general log messages, with NSLog() being reserved
 * for reporting possible/likely errors.  GSPrintf() is declared in
 * GSObjCRuntime.h.
 * </p>
 */
void
NSLog(NSString* format, ...)
{
  va_list ap;

  va_start(ap, format);
  NSLogv(format, ap);
  va_end(ap);
}

/**
 * The core logging function ...
 * <p>
 *   The function generates a standard log entry by prepending
 *   process ID and date/time information to your message, and
 *   ensuring that a newline is present at the end of the message.
 * </p>
 * <p>
 *   In GNUstep, the GSLogThread user default may be set to YES in
 *   order to instruct this function to include the name (if any)
 *   of the current thread after the process ID.  This can help you
 *   to track the behavior of a multi-threaded program.<br />
 *   Also the GSLogOffset user default may be set to YES in order
 *   to instruct this function to include the time zone offset in
 *   the timestamp it logs (good when examining debug logs from
 *   systems running in different countries).
 * </p>
 * <p>
 *   The resulting message is then passed to a handler function to
 *   perform actual output.  Locking is performed around the call to
 *   the function actually writing the message out, to ensure that
 *   logging is thread-safe.  However, the actual creation of the
 *   message written is only as safe as the [NSObject-description] methods
 *   of the arguments you supply.
 * </p>
 * <p>
 *   The function to write the data is pointed to by
 *   <ref type="variable" id="_NSLog_printf_handler">_NSLog_printf_handler</ref>
 * </p>
 */
void
NSLogv(NSString* format, va_list args)
{
  NSMutableString	*prefix;
  NSString              *message;
  NSString              *threadName = nil;
  NSThread              *t = nil;
  /* NB. On systems like Android where there is no operating system thread
   * ID available, the value returned by GSPrivateThreadID() should actually
   * be the pointer to the NSThread object.  We will check for that later.
   */
  NSUInteger            tid = GSPrivateThreadID();
  static int		pid = 0;

  if (_NSLog_printf_handler == NULL)
    {
      _NSLog_printf_handler = *_NSLog_standard_printf_handler;
    }

  if (pid == 0)
    {
#if defined(_WIN32)
      pid = (int)GetCurrentProcessId();
#else
      pid = (int)getpid();
#endif
    }

  if (GSPrivateDefaultsFlag(GSLogThread) == YES)
    {
      /* If no name has been set for the current thread,
       * we log the address of the NSThread object instead.
       */
      t = GSCurrentThread();
      threadName = [t name];
    }

  prefix = [[NSMutableString alloc] initWithCapacity: 1000];

#ifdef	HAVE_SYSLOG
  if (GSPrivateDefaultsFlag(GSLogSyslog) == YES)
    {
      if (nil == t || ((NSThread*)tid == t && nil == threadName))
        {
          [prefix appendFormat: @"[thread:%"PRIuPTR"] ",
            tid];
        }
      else if (nil == threadName)
        {
          [prefix appendFormat: @"[thread:%"PRIuPTR",%p] ",
            tid, t];
        }
      else
        {
          [prefix appendFormat: @"[thread:%"PRIuPTR",%@] ",
            tid, threadName];
        }
    }
  else
#endif
    {
      NSString  *fmt;
      NSString  *cal;

      if (GSPrivateDefaultsFlag(GSLogOffset) == YES)
        {
          fmt = @"%Y-%m-%d %H:%M:%S.%F %z";
        }
      else
        {
          fmt = @"%Y-%m-%d %H:%M:%S.%F";
        }
      cal = [[NSCalendarDate calendarDate] descriptionWithCalendarFormat: fmt];

      [prefix appendString: cal];
      [prefix appendString: @" "];
      [prefix appendString: [[NSProcessInfo processInfo] processName]];
      if (nil == t || ((NSThread*)tid == t && nil == threadName))
        {
          [prefix appendFormat: @"[%d:%"PRIuPTR"] ",
            pid, tid];
        }
      else if (nil == threadName)
        {
          [prefix appendFormat: @"[%d:%"PRIuPTR",%p] ",
            pid, tid, t];
        }
      else
        {
          [prefix appendFormat: @"[%d:%"PRIuPTR",%@] ",
            pid, tid, threadName];
        }
    }

  message = [[NSString alloc] initWithFormat: format arguments: args];
  [prefix appendString: message];
  [message release];
  if ([prefix hasSuffix: @"\n"] ==  NO)
    {
      [prefix appendString: @"\n"];
    }

  if (nil == myLock)
    {
      GSLogLock();
    }

  (*lockImp)(myLock, @selector(lock));

  _NSLog_printf_handler(prefix);

  (*unlockImp)(myLock, @selector(unlock));

  [prefix release];
}

