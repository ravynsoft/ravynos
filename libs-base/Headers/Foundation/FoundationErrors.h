/* Interface for FoundationErrors for GNUstep
   Copyright (C) 2008 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2008
   
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

#ifndef __FoundationErrors_h_GNUSTEP_BASE_INCLUDE
#define __FoundationErrors_h_GNUSTEP_BASE_INCLUDE

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

/* These are those of the NSError code values for the NSCocoaErrorDomain
 * which are defined in the foundation/base library.
 */

enum
{

  NSFileErrorMaximum = 1023,
  NSFileErrorMinimum = 0,
  NSFileLockingError = 255,
  NSFileNoSuchFileError = 4,
  NSFileReadCorruptFileError = 259,
  NSFileReadInapplicableStringEncodingError = 261,
  NSFileReadInvalidFileNameError = 258,
  NSFileReadNoPermissionError = 257,
  NSFileReadNoSuchFileError = 260,
  NSFileReadUnknownError = 256,
  NSFileReadUnsupportedSchemeError = 262,
  NSFileWriteInapplicableStringEncodingError = 517,
  NSFileWriteInvalidFileNameError = 514,
  NSFileWriteFileExistsError = 516,
  NSFileWriteNoPermissionError = 513,
  NSFileWriteOutOfSpaceError = 640,
  NSFileWriteUnknownError = 512,
  NSFileWriteUnsupportedSchemeError = 518,
  NSFormattingError = 2048,
  NSFormattingErrorMaximum = 2559,
  NSFormattingErrorMinimum = 2048,
  NSKeyValueValidationError = 1024,
  NSUserCancelledError = 3072,
  NSValidationErrorMaximum = 2047,
  NSValidationErrorMinimum = 1024,

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
  NSExecutableArchitectureMismatchError = 3585,
  NSExecutableErrorMaximum = 3839,
  NSExecutableErrorMinimum = 3584,
  NSExecutableLinkError = 3588,
  NSExecutableLoadError = 3587,
  NSExecutableNotLoadableError = 3584,
  NSExecutableRuntimeMismatchError = 3586,
  NSFileReadTooLargeError = 263,
  NSFileReadUnknownStringEncodingError = 264,
#endif

  GSFoundationPlaceHolderError = 9999
};

#endif
#endif

