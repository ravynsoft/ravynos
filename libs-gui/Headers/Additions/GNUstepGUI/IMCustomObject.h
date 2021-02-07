/*
   IMCustomObject.h

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: November 1997
   
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

/* This class was inspired by CustomObject class from objcX, "an Objective-C
   class library for a window system". The code was originally written by
   Paul Kunz and Imran Qureshi. */

#ifndef _GNUstep_H_IMCustomObject
#define _GNUstep_H_IMCustomObject

#ifndef GNUSTEP
#import <AppKit/AppKit.h>
#else
#import <Foundation/NSObject.h>
#import <AppKit/NSView.h>

@class NSString;
#endif

/* Add an archiving category to object so every object can respond to
   -nibInstantiate
*/
@interface NSObject (ModelUnarchiving)
- (id)nibInstantiate;
@end


@interface IMCustomObject : NSObject
{
  NSString* className;
  id realObject;
  id extension;
}

- (id)nibInstantiate;

@end


@interface IMCustomView : NSView
{
  NSString* className;
  id realObject;
  id extension;
}

- (id)nibInstantiate;

@end

#endif /* _GNUstep_H_IMCustomObject */
