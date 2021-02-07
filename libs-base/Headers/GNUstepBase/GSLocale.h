/* GSLocale - various functions for localization
    
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Created: Oct 2000

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.
*/

#ifndef __GSLocale_H_
#define __GSLocale_H_

#ifndef NeXT_Foundation_LIBRARY
#import <Foundation/NSString.h>
#else
#import <Foundation/Foundation.h>
#endif
#import "GSObjCRuntime.h"

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSDictionary;

GS_EXPORT const char *GSSetLocaleC(int category, const char *loc);
GS_EXPORT NSString   *GSSetLocale(int category, NSString *locale);

GS_EXPORT NSDictionary *GSDomainFromDefaultLocale(void);

/**
 * Returns the locale string for LC_MESSAGES
 */
GS_EXPORT NSString *GSDefaultLanguageLocale(void);

/**
 * Returns a language name string for a given locale.
 * e.g. GSLanguageFromLocale(@"en_CA") returns @"CanadaEnglish"
 */
GS_EXPORT NSString *GSLanguageFromLocale(NSString *locale);

/**
 * Return an array of variants of a locale, formed by stripping
 * off parts of the identifier, ordered from most similar to 
 * least similar.
 *
 * e.g. GSLocaleVariants(@"en_CA") returns  (@"en_CA", @"en").
 */
GS_EXPORT NSArray *GSLocaleVariants(NSString *locale);

/**
 * Convenience function which calls GSLocaleVariants to expand
 * the given locale to a list of variants, and then calls 
 * GSLanguageFromLocale on each.
 * 
 * e.g. GSLanguagesFromLocale(@"en_CA") returns
 * (@"CanadaEnglish", @"English")
 */
GS_EXPORT NSArray *GSLanguagesFromLocale(NSString *locale);

#if	defined(__cplusplus)
}
#endif

#endif

