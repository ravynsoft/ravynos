/*
   IMLoading.h

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

#ifndef _GNUstep_H_IMLoading
#define _GNUstep_H_IMLoading

#ifndef GNUSTEP
#import <Foundation/Foundation.h>
#else
#import <Foundation/NSObject.h>
@class NSArray;
@class NSBundle;
@class NSString;
#endif

@interface NSObject (NibAwaking)
- (void)awakeFromModel;
@end

@interface GMModel : NSObject
{
  NSArray* objects;
  NSArray* connections;
}
+ (BOOL)loadIMFile:(NSString*)path owner:(id)owner;
+ (BOOL)loadIMFile:(NSString*)path owner:(id)owner bundle:(NSBundle*)bundle;
- (void)_makeConnections;
- (void)_setObjects:objects connections:connections;
- (NSArray *) objects;
- (NSArray *) connections;
@end

#endif /* _GNUstep_H_IMLoading */
