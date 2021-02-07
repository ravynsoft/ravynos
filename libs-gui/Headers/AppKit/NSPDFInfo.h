/* Definition of class NSPDFInfo
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sat Nov 16 21:20:46 EST 2019

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

#ifndef _NSPDFInfo_h_GNUSTEP_GUI_INCLUDE
#define _NSPDFInfo_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/NSPrintInfo.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSURL, NSArray, NSMutableDictionary;
  
@interface NSPDFInfo : NSObject <NSCoding, NSCopying>
{
  NSURL *_url;
  BOOL _fileExtensionHidden;
  NSArray *_tagNames;
  NSSize _paperSize;
  NSMutableDictionary *_attributes;
  NSPaperOrientation _orientation;
}
  
- (NSURL *) URL;

- (BOOL) isFileExtensionHidden;
- (void) setFileExtensionHidden: (BOOL)flag;
  
- (NSArray *) tagNames;

- (NSPaperOrientation) orientation;
- (void) setOrientation: (NSPaperOrientation)orientation;

- (NSSize) paperSize;
- (void) setPaperSize: (NSSize)size;

- (NSMutableDictionary *) attributes;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSPDFInfo_h_GNUSTEP_GUI_INCLUDE */

