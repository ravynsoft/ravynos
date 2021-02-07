/** Declaration of extension methods for base additions

   Copyright (C) 2003-2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   and:         Adam Fedor <fedor@gnu.org>

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

#ifndef	INCLUDED_NSProcessInfo_GNUstepBase_h
#define	INCLUDED_NSProcessInfo_GNUstepBase_h

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSProcessInfo.h>

#if	defined(__cplusplus)
extern "C" {
#endif

#if	OS_API_VERSION(GS_API_NONE,GS_API_LATEST)

@interface NSProcessInfo(GNUstepBase)

/**
 * Returns a indication of whether debug logging is enabled.
 * This returns YES unless a call to -setDebugLoggingEnabled: has
 * been used to turn logging off.
 */
- (BOOL) debugLoggingEnabled;

/**
 * This method returns a set of debug levels set using the
 * --GNU-Debug=... command line option and/or the GNU-Debug
 * user default.<br />
 * You can modify this set to change the debug logging under
 * your programs control ... but such modifications are not
 * thread-safe.
 */
- (NSMutableSet*) debugSet;

/**
 * This method permits you to turn all debug logging on or off
 * without modifying the set of debug levels in use.
 */
- (void) setDebugLoggingEnabled: (BOOL)flag;

@end

#endif	/* OS_API_VERSION */

#if	defined(__cplusplus)
}
#endif

#endif	/* INCLUDED_NSProcessInfo_GNUstepBase_h */

