/* Definition of class NSOrthography
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: heron
   Date: Tue Nov  5 03:43:39 EST 2019

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

#ifndef _NSOrthography_h_GNUSTEP_BASE_INCLUDE
#define _NSOrthography_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSDictionary, NSString, NSArray;

GS_EXPORT_CLASS
@interface NSOrthography : NSObject <NSCopying, NSCoding>
{
  NSDictionary *_languageMap;
  NSString *_dominantScript;
}
  
- (instancetype) initWithDominantScript: (NSString *)script
                            languageMap: (NSDictionary *)map;

- (NSString *) dominantScript;
- (NSDictionary *) languationMap;

- (NSArray *) languagesForScript: (NSString *)script; 
- (NSString *) dominantLanguageForScript: (NSString *)script;

- (NSString *) dominantLanguage;
- (NSArray *) allScripts;
- (NSArray *) allLanguages;
  
+ (instancetype) defaultOrthographyForLanguage: (NSString *)language;
+ (instancetype) orthographyWithDominantScript: (NSString *)script languageMap: (NSDictionary *)map;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSOrthography_h_GNUSTEP_BASE_INCLUDE */

