/* 
   NSStringDrawing.h

   Categories which add measure capabilities to NSAttributedString 
   and NSString.

   Copyright (C) 1997 Free Software Foundation, Inc.

   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: Aug 1998
   Rewrite: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Mar 1999
   
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

#ifndef _GNUstep_H_NSStringDrawing
#define _GNUstep_H_NSStringDrawing
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)

#import <Foundation/NSAttributedString.h>
#import <Foundation/NSGeometry.h>
#import <Foundation/NSString.h>

@class NSDictionary;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
typedef enum 
{
    NSStringDrawingUsesLineFragmentOrigin=0x01,
    NSStringDrawingUsesFontLeading=0x02,
    NSStringDrawingDisableScreenFontSubstitution=0x04,
    NSStringDrawingUsesDeviceMetrics=0x08,
    NSStringDrawingOneShot=0x10
} NSStringDrawingOptions;
#endif

@interface NSString (NSStringDrawing)

- (void) drawAtPoint: (NSPoint)point withAttributes: (NSDictionary*)attrs;
- (void) drawInRect: (NSRect)rect withAttributes: (NSDictionary*)attrs;
- (NSSize) sizeWithAttributes: (NSDictionary*)attrs;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSRect) boundingRectWithSize: (NSSize)size
                        options: (NSStringDrawingOptions)options
                     attributes: (NSDictionary *)attributes;
- (void) drawWithRect: (NSRect)rect
              options: (NSStringDrawingOptions)options
           attributes: (NSDictionary *)attributes;
#endif

@end

@interface NSAttributedString (NSStringDrawing)

- (NSSize) size;
- (void) drawAtPoint: (NSPoint)point;
- (void) drawInRect: (NSRect)rect;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSRect) boundingRectWithSize: (NSSize)size
                        options: (NSStringDrawingOptions)options;
- (void) drawWithRect: (NSRect)rect
              options: (NSStringDrawingOptions)options;
#endif

@end

#else
@class NSAttributedString;
#endif

#endif /* _GNUstep_H_NSStringDrawing */
