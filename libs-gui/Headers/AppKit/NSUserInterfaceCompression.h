/* Definition of class NSUserInterfaceCompression
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Tue Apr  7 08:13:30 EDT 2020

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

#ifndef _NSUserInterfaceCompression_h_GNUSTEP_GUI_INCLUDE
#define _NSUserInterfaceCompression_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_13, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSSet, NSString, NSArray;

@interface NSUserInterfaceCompressionOptions : NSObject <NSCopying, NSCoding>

- (instancetype) initWithIdentifier: (NSString *)identifier;

- (instancetype) initWithCompressionOptions: (NSSet *)opts; 

- (BOOL) containsOptions: (NSUserInterfaceCompressionOptions *)opts;

- (BOOL) intersectsOptions: (NSUserInterfaceCompressionOptions *)opts;

- (BOOL) isEmpty; 

- (NSUserInterfaceCompressionOptions *) optionsByAddingOptions: (NSUserInterfaceCompressionOptions *)opts;

- (NSUserInterfaceCompressionOptions *) optionsByRemovingOptions: (NSUserInterfaceCompressionOptions *)opts;

+ (NSUserInterfaceCompressionOptions *) hideImagesOption;

+ (NSUserInterfaceCompressionOptions *) hideTextOption;

+ (NSUserInterfaceCompressionOptions *) reduceMetricsOption;

+ (NSUserInterfaceCompressionOptions *) breakEqualWidthsOption;

+ (NSUserInterfaceCompressionOptions *) standardOptions;

@end

@protocol NSUserInterfaceCompression

- (void) compressWithPrioritizedCompressionOptions: (NSArray *)prioritizedOptions;

- (NSSize) minimumSizeWithPrioritizedCompressionOptions: (NSArray *)prioritizedOptions;

- (NSUserInterfaceCompressionOptions *) activeCompressionOptions;
  
@end


#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSUserInterfaceCompression_h_GNUSTEP_GUI_INCLUDE */

