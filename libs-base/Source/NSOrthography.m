/* Implementation of class NSOrthography
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
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

#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSOrthography.h"
#import "Foundation/NSString.h"

@implementation NSOrthography

- (instancetype) initWithDominantScript: (NSString *)script
                            languageMap: (NSDictionary *)map
{
  self = [super init];
  if (self != nil)
    {
      ASSIGNCOPY(_dominantScript, script);
      ASSIGNCOPY(_languageMap, map);
    }
  return self;
}

- (oneway void) dealloc
{
  RELEASE(_dominantScript);
  RELEASE(_languageMap);
  [super dealloc];
}

- (NSString *) dominantScript
{
  return _dominantScript;
}

- (NSDictionary *) languationMap
{
  return _languageMap;
}

- (NSArray *) languagesForScript: (NSString *)script
{
  return nil;
}

- (NSString *) dominantLanguageForScript: (NSString *)script
{
  return nil;
}

- (NSString *) dominantLanguage
{
  return nil;
}

- (NSArray *) allScripts
{
  return nil;
}

- (NSArray *) allLanguages
{
  return nil;
}  

+ (instancetype) defaultOrthographyForLanguage: (NSString *)language
{
  return nil;
}

+ (instancetype) orthographyWithDominantScript: (NSString *)script languageMap: (NSDictionary *)map
{
  NSOrthography *result = [[NSOrthography alloc] initWithDominantScript: script
                                                            languageMap: map];
  AUTORELEASE(result);
  return result;
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  return nil;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
}

- (instancetype) copyWithZone: (NSZone *)zone
{
  return [[[self class] allocWithZone: zone]
           initWithDominantScript: _dominantScript
                      languageMap: _languageMap];  
}
@end

