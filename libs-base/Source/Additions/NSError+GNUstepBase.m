/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/

#import "common.h"
#include <stdio.h>

#import "Foundation/NSDictionary.h"
#import "Foundation/NSError.h"
#import "Foundation/NSLock.h"
#import "GSPrivate.h"

/**
 * GNUstep specific (non-standard) additions to the NSError class.
 * Possibly to be made public
 */
@implementation NSError(GNUstepBase)


#if !defined(_WIN32)
#if !defined(HAVE_STRERROR_R)
#if defined(HAVE_STRERROR)
static int
strerror_r(int eno, char *buf, int len)
{
  const char *ptr;
  int   result;

  [gnustep_global_lock lock];
  ptr = strerror(eno);
  if (ptr == 0)
    {
      strncpy(buf, "unknown error number", len - 1);
      result = -1;
    }
  else
    {
      strncpy(buf, strerror(eno), len - 1);
      result = 0;
    }
  buf[len - 1] = '\0';
  [gnustep_global_lock unlock];
  return result;
}
#else
static int
strerror_r(int eno, char *buf, int len)
{
  extern char  *sys_errlist[];
  extern int    sys_nerr;

  if (eno < 0 || eno >= sys_nerr)
    {
      strncpy(buf, "unknown error number", len - 1);
      return -1;
    }
  strncpy(buf, sys_errlist[eno], len - 1);
  buf[len - 1] = '\0';
  return 0;
}
#endif
#endif
#endif

/* Returns an NSError instance encapsulating the last system error,
 * or nil if the last system call did not result in an error.
 * The user info dictionary of this object will be mutable, so that
 * additional information can be placed in it by higher level code.
 */
+ (NSError*) _last
{
  int	eno;
#if defined(_WIN32)
  eno = GetLastError();
  if (eno == 0) eno = errno;
#else
  eno = errno;
#endif
  if (0 == eno)
    {
      return nil;
    }
  return [self _systemError: eno];
}

+ (NSError*) _systemError: (long)code
{
  NSError	*error;
  NSString	*domain;
  NSDictionary	*info;
#if defined(_WIN32)
  LPVOID	lpMsgBuf;
  NSString	*message=nil;

  domain = NSOSStatusErrorDomain;
  FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPWSTR) &lpMsgBuf, 0, NULL );
  if (lpMsgBuf != NULL)
    {
      message
	= [NSString stringWithCharacters: lpMsgBuf length: wcslen(lpMsgBuf)];
      LocalFree(lpMsgBuf);
    }
  info = [NSMutableDictionary dictionaryWithObjectsAndKeys:
    message, NSLocalizedDescriptionKey,
    nil];
#else
  NSString	*message;
  char          buf[BUFSIZ];
# if STRERROR_R_CHAR_P
  char          *result;
# else
  int           result;
#endif

  /* FIXME ... not all are POSIX, should we use NSMachErrorDomain for some? */
  domain = NSPOSIXErrorDomain;
  result = strerror_r(code, buf, BUFSIZ);
# if STRERROR_R_CHAR_P
  if (result == 0)
    {
      snprintf(buf, sizeof(buf), "%ld", code);
      result = buf;
    }
  message = [NSString stringWithCString: result
			       encoding: [NSString defaultCStringEncoding]];
# else
  if (result < 0)
    {
      snprintf(buf, sizeof(buf), "%ld", code);
    }
  message = [NSString stringWithCString: buf
			       encoding: [NSString defaultCStringEncoding]];
# endif
  /* FIXME ... can we do better localisation? */
  info = [NSMutableDictionary dictionaryWithObjectsAndKeys:
    message, NSLocalizedDescriptionKey,
    nil];
#endif

  /* NB we use a mutable dictionary so that calling code can add extra
   * information to the dictionary before passing it up to higher level
   * code.
   */
  error = [self errorWithDomain: domain code: code userInfo: info];
  return error;
}
@end
