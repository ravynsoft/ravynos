/* 
   NSButtonImageSource.h

   Copyright (C) 2006 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2006
   
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

#ifndef _GNUstep_H_NSImageSource
#define _GNUstep_H_NSImageSource

#import <Foundation/NSObject.h>
#import <AppKit/NSButtonCell.h>

@class NSString, NSMutableDictionary;

/**
 * Handle images for button cell theming.
 */
@interface NSButtonImageSource : NSObject <NSCoding, NSCopying>
{
  NSString		*imageName;
  NSMutableDictionary	*images;
}
+ (BOOL) archiveButtonImageSourceWithName: (NSString*)name
			      toDirectory: (NSString*)path;
+ (id) buttonImageSourceWithName: (NSString*)name;
- (id) imageForState: (struct NSButtonState)state;
@end

@interface NSButtonCell (NSButtonImageSource)
- (id) _buttonImageSource;
- (void) _setButtonImageSource: (id)source;
@end

#endif /* _GNUstep_H_NSImageSource */
