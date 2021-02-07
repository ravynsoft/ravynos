/** Interface for NSErrorRecoveryAttempting for GNUStep
   Copyright (C) 2007 Free Software Foundation, Inc.

   Written by:  Fred Kiefer <fredkiefer@gmx.de>
   Date: July 2007
   
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

   */ 

#ifndef __NSErrorRecoveryAttempting_h_GNUSTEP_BASE_INCLUDE
#define __NSErrorRecoveryAttempting_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if	OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

/**
 * These methods define the informal error recovery protocol
 */
@interface NSObject (NSErrorRecoveryAttempting)

- (BOOL) attemptRecoveryFromError: (NSError*)error
                      optionIndex: (unsigned int)recoveryOptionIndex;
- (void) attemptRecoveryFromError: (NSError*)error
                      optionIndex: (unsigned int)recoveryOptionIndex
                         delegate: (id)delegate
               didRecoverSelector: (SEL)didRecoverSelector
                      contextInfo: (void*)contextInfo;

@end

#if	defined(__cplusplus)
}
#endif

#endif

#endif	/* __NSErrorRecoveryAttempting_h_GNUSTEP_BASE_INCLUDE*/
