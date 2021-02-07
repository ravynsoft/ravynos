/* Interface of class NSAccessibilityCustomRotor
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Mon 15 Jun 2020 03:18:59 AM EDT

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSAccessibilityCustomRotor_h_GNUSTEP_GUI_INCLUDE
#define _NSAccessibilityCustomRotor_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSGeometry.h>

#import <AppKit/NSAccessibilityProtocols.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_13, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@protocol NSAccessibilityCustomRotorItemSearchDelegate;
@protocol NSAccessibilityCustomRotorItemLoadDelegate;

@class NSAccessibilityCustomRotor;
@class NSAccessibilityCustomRotorItemResult;
@class NSAccessibilityCustomRotorSearchParameters;
@class NSString;
@class NSAccessibilityElement;
  
enum
  {
   NSAccessibilityCustomRotorSearchDirectionPrevious,
   NSAccessibilityCustomRotorSearchDirectionNext,
};
typedef NSInteger NSAccessibilityCustomRotorSearchDirection;
  
enum
  {
   NSAccessibilityCustomRotorTypeCustom = 0,
   NSAccessibilityCustomRotorTypeAny = 1,
   NSAccessibilityCustomRotorTypeAnnotation,
   NSAccessibilityCustomRotorTypeBoldText,
   NSAccessibilityCustomRotorTypeHeading,
   NSAccessibilityCustomRotorTypeHeadingLevel1,
   NSAccessibilityCustomRotorTypeHeadingLevel2,
   NSAccessibilityCustomRotorTypeHeadingLevel3,
   NSAccessibilityCustomRotorTypeHeadingLevel4,
   NSAccessibilityCustomRotorTypeHeadingLevel5,
   NSAccessibilityCustomRotorTypeHeadingLevel6,
   NSAccessibilityCustomRotorTypeImage,
   NSAccessibilityCustomRotorTypeItalicText,
   NSAccessibilityCustomRotorTypeLandmark,
   NSAccessibilityCustomRotorTypeLink,
   NSAccessibilityCustomRotorTypeList,
   NSAccessibilityCustomRotorTypeMisspelledWord,
   NSAccessibilityCustomRotorTypeTable,
   NSAccessibilityCustomRotorTypeTextField,
   NSAccessibilityCustomRotorTypeUnderlinedText,
   NSAccessibilityCustomRotorTypeVisitedLink,
}; 
typedef NSInteger NSAccessibilityCustomRotorType;

// Rotor...
@interface NSAccessibilityCustomRotor : NSObject
  
- (instancetype) initWithLabel: (NSString *)label
                   itemSearchDelegate: (id<NSAccessibilityCustomRotorItemSearchDelegate>)delegate;

- (instancetype) initWithRotorType: (NSAccessibilityCustomRotorType)rotorType
                   itemSearchDelegate: (id<NSAccessibilityCustomRotorItemSearchDelegate>)delegate;

- (NSAccessibilityCustomRotorType) type;
- (void) setType: (NSAccessibilityCustomRotorType)type;

- (NSString *) label;
- (void) setLabel: (NSString *)label;

- (id<NSAccessibilityCustomRotorItemSearchDelegate>) itemSearchDelegate;
- (void) setItemSearchDelegate: (id<NSAccessibilityCustomRotorItemSearchDelegate>) delegate;

- (id<NSAccessibilityElementLoading>) itemLoadingDelegate;
- (void) setItemLoadingDelegate: (id<NSAccessibilityElementLoading>) delegate;
  
@end

// Results...
@interface NSAccessibilityCustomRotorItemResult : NSObject

- (instancetype)initWithTargetElement:(id<NSAccessibilityElement>)targetElement;

- (instancetype)initWithItemLoadingToken: (id<NSAccessibilityLoadingToken>)token
                             customLabel: (NSString *)customLabel;

- (id<NSAccessibilityElement>) targetElement;
  
- (id<NSAccessibilityLoadingToken>) itemLoadingToken;

- (NSRange) targetRange;

- (NSString *) customLabel;

@end


// Protocol...
@protocol NSAccessibilityCustomRotorItemSearchDelegate <NSObject>

#if GS_PROTOCOLS_HAVE_OPTIONAL
@required
#endif
- (NSAccessibilityCustomRotorItemResult *) rotor: (NSAccessibilityCustomRotor *)rotor
                       resultForSearchParameters: (NSAccessibilityCustomRotorSearchParameters *)parameters;

@end


#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSAccessibilityCustomRotor_h_GNUSTEP_GUI_INCLUDE */

