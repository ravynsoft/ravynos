/* Definition of class NSErrors
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Thu Mar 26 09:13:56 EDT 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSErrors_h_GNUSTEP_GUI_INCLUDE
#define _NSErrors_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

APPKIT_EXPORT NSExceptionName NSAbortModalException;
APPKIT_EXPORT NSExceptionName NSAbortPrintingException;
APPKIT_EXPORT NSExceptionName NSAccessibilityException;
APPKIT_EXPORT NSExceptionName NSAppKitIgnoredException;
APPKIT_EXPORT NSExceptionName NSAppKitVirtualMemoryException;
APPKIT_EXPORT NSExceptionName NSBadBitmapParametersException;
APPKIT_EXPORT NSExceptionName NSBadComparisonException;
APPKIT_EXPORT NSExceptionName NSBadRTFColorTableException;
APPKIT_EXPORT NSExceptionName NSBadRTFDirectiveException;
APPKIT_EXPORT NSExceptionName NSBadRTFFontTableException;
APPKIT_EXPORT NSExceptionName NSBadRTFStyleSheetException;
APPKIT_EXPORT NSExceptionName NSBrowserIllegalDelegateException;
APPKIT_EXPORT NSExceptionName NSColorListIOException;
APPKIT_EXPORT NSExceptionName NSColorListNotEditableException;
APPKIT_EXPORT NSExceptionName NSDraggingException;
APPKIT_EXPORT NSExceptionName NSFontUnavailableException;
APPKIT_EXPORT NSExceptionName NSIllegalSelectorException;
APPKIT_EXPORT NSExceptionName NSImageCacheException;
APPKIT_EXPORT NSExceptionName NSNibLoadingException;
APPKIT_EXPORT NSExceptionName NSPPDIncludeNotFoundException;
APPKIT_EXPORT NSExceptionName NSPPDIncludeStackOverflowException;
APPKIT_EXPORT NSExceptionName NSPPDIncludeStackUnderflowException;
APPKIT_EXPORT NSExceptionName NSPPDParseException;
APPKIT_EXPORT NSExceptionName NSPasteboardCommunicationException;
APPKIT_EXPORT NSExceptionName NSPrintPackageException;
APPKIT_EXPORT NSExceptionName NSPrintingCommunicationException;
APPKIT_EXPORT NSExceptionName NSRTFPropertyStackOverflowException;
APPKIT_EXPORT NSExceptionName NSTIFFException;
APPKIT_EXPORT NSExceptionName NSTextLineTooLongException;
APPKIT_EXPORT NSExceptionName NSTextNoSelectionException;
APPKIT_EXPORT NSExceptionName NSTextReadException;
APPKIT_EXPORT NSExceptionName NSTextWriteException;
APPKIT_EXPORT NSExceptionName NSTypedStreamVersionException;
APPKIT_EXPORT NSExceptionName NSWindowServerCommunicationException;
APPKIT_EXPORT NSExceptionName NSWordTablesReadException;

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSErrors_h_GNUSTEP_GUI_INCLUDE */

