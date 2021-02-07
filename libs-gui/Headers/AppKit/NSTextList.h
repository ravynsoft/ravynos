/* -*-objc-*-
   NSTextList.h

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: January 2008
   
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

#ifndef _GNUstep_H_NSTextList
#define _GNUstep_H_NSTextList
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
#import <Foundation/NSObject.h>

@class NSString;

enum {
	NSTextListPrependEnclosingMarker = 1
};

@interface NSTextList : NSObject <NSCopying, NSCoding>
{
	NSString *_markerFormat;
	unsigned int _listOptions;
	NSInteger _startingItemNumber;
}

- (id) initWithMarkerFormat: (NSString *)format 
                    options: (unsigned int)mask;
- (unsigned int) listOptions;
- (NSString *) markerForItemNumber: (int)item;
- (NSString *) markerFormat;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
#if GS_HAS_DECLARED_PROPERTIES
@property NSInteger startingItemNumber;
#else
- (NSInteger) startingItemNumber;
- (void) setStartingItemNumber: (NSInteger)start;
#endif
#endif

@end

#endif 

#endif // _GNUstep_H_NSTextList
