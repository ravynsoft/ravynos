/*
   AppKitExceptions.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: February 1997
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef __AppKit_AppKitExceptions_h__
#define __AppKit_AppKitExceptions_h__
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/AppKitDefines.h>

@class NSString;

APPKIT_EXPORT NSString *NSAbortModalException;
APPKIT_EXPORT NSString *NSAbortPrintingException;
APPKIT_EXPORT NSString *NSAppKitIgnoredException;
APPKIT_EXPORT NSString *NSAppKitVirtualMemoryException;
APPKIT_EXPORT NSString *NSBadBitmapParametersException;
APPKIT_EXPORT NSString *NSBadComparisonException;
APPKIT_EXPORT NSString *NSBadRTFColorTableException;
APPKIT_EXPORT NSString *NSBadRTFDirectiveException;
APPKIT_EXPORT NSString *NSBadRTFFontTableException;
APPKIT_EXPORT NSString *NSBadRTFStyleSheetException;
APPKIT_EXPORT NSString *NSBrowserIllegalDelegateException;
APPKIT_EXPORT NSString *NSColorListIOException;
APPKIT_EXPORT NSString *NSColorListNotEditableException;
APPKIT_EXPORT NSString *NSDraggingException;
APPKIT_EXPORT NSString *NSFontUnavailableException;
APPKIT_EXPORT NSString *NSIllegalSelectorException;
APPKIT_EXPORT NSString *NSImageCacheException;
APPKIT_EXPORT NSString *NSNibLoadingException;
APPKIT_EXPORT NSString *NSPPDIncludeNotFoundException;
APPKIT_EXPORT NSString *NSPPDIncludeStackOverflowException;
APPKIT_EXPORT NSString *NSPPDIncludeStackUnderflowException;
APPKIT_EXPORT NSString *NSPPDParseException;
APPKIT_EXPORT NSString *NSPasteboardCommunicationException;
APPKIT_EXPORT NSString *NSPrintOperationExistsException;
APPKIT_EXPORT NSString *NSPrintPackageException;
APPKIT_EXPORT NSString *NSPrintingCommunicationException;
APPKIT_EXPORT NSString *NSRTFPropertyStackOverflowException;
APPKIT_EXPORT NSString *NSTIFFException;
APPKIT_EXPORT NSString *NSTextLineTooLongException;
APPKIT_EXPORT NSString *NSTextNoSelectionException;
APPKIT_EXPORT NSString *NSTextReadException;
APPKIT_EXPORT NSString *NSTextWriteException;
APPKIT_EXPORT NSString *NSTypedStreamVersionException;
APPKIT_EXPORT NSString *NSWindowServerCommunicationException;
APPKIT_EXPORT NSString *NSWordTablesReadException;
APPKIT_EXPORT NSString *NSWordTablesWriteException;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
APPKIT_EXPORT NSString *GSWindowServerInternalException;
#endif

#endif /* __AppKit_AppKitExceptions_h__ */
