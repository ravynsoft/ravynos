/* Definition of class NSAppearance
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Wed Jan 15 07:03:39 EST 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSAppearance_h_GNUSTEP_GUI_INCLUDE
#define _NSAppearance_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSBundle.h>
#import <AppKit/AppKitDefines.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

typedef NSString* NSAppearanceName;

@interface NSAppearance : NSObject <NSCoding>
{
  NSString *_name;
  BOOL _allowsVibrancy;
  NSAppearanceName _currentAppearance;
}

// Creating an appearance...
+ (instancetype) appearanceNamed: (NSString *)name;
- (instancetype) initWithAppearanceNamed: (NSString *)name bundle: (NSBundle *)bundle;

// Getting the appearance name
- (NSString *) name;

// Determining the most appropriate appearance
- (NSAppearanceName) bestMatchFromAppearancesWithNames: (NSArray *)appearances;

// Getting and setting the appearance
+ (void) setCurrentAppearance: (NSAppearance *)appearance;
+ (NSAppearance *) currentAppearance;

// Managing vibrancy
- (BOOL) allowsVibrancy;
  
@end

APPKIT_EXPORT NSAppearanceName const NSAppearanceNameAqua;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameDarkAqua;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameVibrantLight;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameVibrantDark;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameAccessibilityHighContrastAqua;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameAccessibilityHighContrastDarkAqua;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameAccessibilityHighContrastVibrantLight;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameAccessibilityHighContrastVibrantDark;
APPKIT_EXPORT NSAppearanceName const NSAppearanceNameLightContent;


#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSAppearance_h_GNUSTEP_GUI_INCLUDE */

