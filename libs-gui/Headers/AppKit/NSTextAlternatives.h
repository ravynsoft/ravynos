/*
   NSTextAlternatives.m

   Select an alternative for a given text.

   Copyright (C) 2017 Free Software Foundation, Inc.

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: 2017

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

#ifndef _GNUstep_H_NSTextAlternatives 
#define _GNUstep_H_NSTextAlternatives 

#import <AppKit/AppKitDefines.h>
#import <Foundation/NSString.h>
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8, GS_API_LATEST)

@interface NSTextAlternatives : NSObject {
  NSString *_primaryString;
  NSArray *_alternativeStrings;
}
- (id)initWithPrimaryString:(NSString *)primaryString
         alternativeStrings:(NSArray *)alternativeStrings;

#if GS_HAS_DECLARED_PROPERTIES
@property (readonly, copy) NSString *primaryString;
@property (readonly, copy) NSArray *alternativeStrings;
#else
- (NSString *)primaryString;
- (NSArray *)alternativeStrings;
#endif

- (void)noteSelectedAlternativeString:(NSString *)alternativeString;
@end

APPKIT_EXPORT NSString *NSTextAlternativesSelectedAlternativeStringNotification;

#endif
#endif
