/* Definition of the AppKitErrors header
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Tues Apr 7 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef _AppKitErrors_h_GNUSTEP_GUI_INCLUDE
#define _AppKitErrors_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

// All of the AppKit errors in the NSCocoaErrorDomain.

enum
  {
    NSFontAssetDownloadError                 = 66304,
    NSFontErrorMinimum                       = 66304,
    NSFontErrorMaximum                       = 66335,
    NSServiceApplicationNotFoundError        = 66560,   
    NSServiceApplicationLaunchFailedError    = 66561,
    NSServiceRequestTimedOutError            = 66562,
    NSServiceInvalidPasteboardDataError      = 66563,
    NSServiceMalformedServiceDictionaryError = 66564,
    NSServiceMiscellaneousError              = 66800,
    NSServiceErrorMinimum                    = 66560,
    NSServiceErrorMaximum                    = 66817,
    NSSharingServiceNotConfiguredError       = 67072,
    NSSharingServiceErrorMinimum             = 67072,
    NSSharingServiceErrorMaximum             = 67327,
    NSTextReadInapplicableDocumentTypeError  = 65806,
    NSTextWriteInapplicableDocumentTypeError = 66062,
    NSTextReadWriteErrorMinimum              = 65792,
    NSTextReadWriteErrorMaximum              = 66303,
    NSWorkspaceAuthorizationInvalidError     = 67328,
    NSWorkspaceErrorMinimum                  = 67328,
    NSWorkspaceErrorMaximum                  = 67455,
  };

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _AppKitErrors_h_GNUSTEP_GUI_INCLUDE */
