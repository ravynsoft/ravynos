/*
   GSTrackingRect.h

   Tracking rectangle class

   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#ifndef _GNUstep_H_GSTrackingRect
#define _GNUstep_H_GSTrackingRect

#import <Foundation/NSObject.h>
#import <AppKit/NSView.h>

@interface GSTrackingRect : NSObject <NSCoding>
{
@public
  NSRect		rectangle;
  NSTrackingRectTag	tag;
  id			owner;
  void			*user_data;
  struct TrackFlagsType {
    unsigned	inside:1;
    unsigned	isValid:1;
    unsigned	checked:1;
    unsigned	ownerRespondsToMouseEntered:1;
    unsigned	ownerRespondsToMouseExited:1;
  } flags;
}

- (id) initWithRect: (NSRect)aRect
		tag: (NSTrackingRectTag)aTag
	      owner: (id)anObject
	   userData: (void *)theData
	     inside: (BOOL)flag;

- (NSRect) rectangle;
- (void) reset: (NSRect)aRect inside: (BOOL)flag;
- (NSTrackingRectTag) tag;
- (id) owner;
- (void*) userData;
- (BOOL) inside;

- (BOOL) isValid;
- (void) invalidate;

@end

#endif /* _GNUstep_H_GSTrackingRect */

