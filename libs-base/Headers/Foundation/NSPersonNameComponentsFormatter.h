/* Definition of class NSPersonNameComponents
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   Implemented by: Gregory Casamento <greg.casamento@gmail.com>
   Date: Sep 2019

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

#ifndef __NSPersonNameComponentsFormatter_h_GNUSTEP_BASE_INCLUDE
#define __NSPersonNameComponentsFormatter_h_GNUSTEP_BASE_INCLUDE


#import <Foundation/NSFormatter.h>
#import <Foundation/NSPersonNameComponents.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_11,GS_API_LATEST)

// Style...
enum {
    NSPersonNameComponentsFormatterStyleDefault = 0,
    NSPersonNameComponentsFormatterStyleShort,
    NSPersonNameComponentsFormatterStyleMedium,
    NSPersonNameComponentsFormatterStyleLong,
    NSPersonNameComponentsFormatterStyleAbbreviated
};
typedef NSUInteger NSPersonNameComponentsFormatterStyle;

// Options...
enum { NSPersonNameComponentsFormatterPhonetic = (1UL << 1) }; 
typedef NSUInteger NSPersonNameComponentsFormatterOptions;

@class NSString;

GS_EXPORT_CLASS
@interface NSPersonNameComponentsFormatter : NSFormatter
{
  @private
    BOOL _phonetic;
    NSPersonNameComponentsFormatterStyle _style;
    NSPersonNameComponentsFormatterOptions _nameOptions;
}

// Designated init...
+ (NSString *) localizedStringFromPersonNameComponents: (NSPersonNameComponents *)components
                                                 style: (NSPersonNameComponentsFormatterStyle)nameFormatStyle
                                               options: (NSPersonNameComponentsFormatterOptions)nameOptions;

// Setters
- (NSPersonNameComponentsFormatterStyle) style;
- (void) setStyle: (NSPersonNameComponentsFormatterStyle)style;
- (BOOL) isPhonetic;
- (void) setPhonetic: (BOOL)flag;

// Convenience methods...
- (NSString *) stringFromPersonNameComponents: (NSPersonNameComponents *)components;
- (NSAttributedString *) annotatedStringFromPersonNameComponents: (NSPersonNameComponents *)components;
- (NSPersonNameComponents *) personNameComponentsFromString: (NSString *)string;
- (BOOL)getObjectValue: (id *)obj
             forString: (NSString *)string
      errorDescription: (NSString **)error;

@end

// components for attributed strings;
GS_EXPORT NSString * const NSPersonNameComponentKey;
GS_EXPORT NSString * const NSPersonNameComponentGivenName;
GS_EXPORT NSString * const NSPersonNameComponentFamilyName;
GS_EXPORT NSString * const NSPersonNameComponentMiddleName;
GS_EXPORT NSString * const NSPersonNameComponentPrefix;
GS_EXPORT NSString * const NSPersonNameComponentSuffix;
GS_EXPORT NSString * const NSPersonNameComponentNickname;
GS_EXPORT NSString * const NSPersonNameComponentDelimiter; 

#endif
#endif
