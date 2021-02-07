/** Additional functions and macros for GNUStep
   Copyright (C) 2005 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Created: 2005
   
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
 
   AutogsdocSource:	Additions/GSFunctions.m
   */ 

#ifndef __GSFunctions_h_GNUSTEP_BASE_INCLUDE
#define __GSFunctions_h_GNUSTEP_BASE_INCLUDE
#import "GNUstepBase/GNUstep.h"

#import "GNUstepBase/GSObjCRuntime.h"

#if	defined(__cplusplus)
extern "C" {
#endif

#define GSLocalizedStringFromTableInFramework(key, tbl, fpth, comment) \
  [[NSBundle mainBundle] localizedStringForKey:(key) value:@"" \
  table: [bundle pathForGNUstepResource:(tbl) ofType: nil inDirectory: (fpth)]

  /* Now Support for Quick Localization */

  /* The quickest possible way to localize a string:
    
     NSLog (_(@"New Game"));
    
     Please make use of the longer functions taking a comment when you
     get the time to localize seriously your code.
  */

#if	GS_API_VERSION(GS_API_NONE,011500)
@class	NSArray;
@class	NSString;
/** 
 * Try to locate file/directory (aName).(anExtension) in paths.
 * Will return the first found or nil if nothing is found.<br />
 * Deprecated ... may be removed in later release.
 */
GS_EXPORT NSString *GSFindNamedFile(NSArray *paths, NSString *aName,
  NSString *anExtension);
#endif

#if	defined(__cplusplus)
}
#endif

#endif /* __NSPathUtilities_h_GNUSTEP_BASE_INCLUDE */
