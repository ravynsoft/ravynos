/* Definition of class NSPICTImageRep
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Fri Nov 15 04:24:51 EST 2019

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

#ifndef _NSPICTImageRep_h_GNUSTEP_GUI_INCLUDE
#define _NSPICTImageRep_h_GNUSTEP_GUI_INCLUDE

#import <AppKit/NSImageRep.h>
#import <Foundation/NSGeometry.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_0, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData;
  
@interface NSPICTImageRep : NSImageRep
{
  NSBitmapImageRep *_pageRep;
  NSData *_pictRepresentation;
  NSRect  _boundingBox;
}

+ (instancetype) imageRepWithData: (NSData *)imageData;

- (instancetype) initWithData: (NSData *)imageData;

- (NSRect) boundingBox;

- (NSData *) PICTRepresentation;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSPICTImageRep_h_GNUSTEP_GUI_INCLUDE */

