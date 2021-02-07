/* Definition of class NSDataAsset
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Fri Jan 17 10:25:34 EST 2020

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

#ifndef _NSDataAsset_h_GNUSTEP_GUI_INCLUDE
#define _NSDataAsset_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_11, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSData, NSBundle, NSString; 
  
typedef NSString* NSDataAssetName;
  
@interface NSDataAsset : NSObject <NSCopying>
{
  NSDataAssetName _name;
  NSBundle *_bundle;
  NSData *_data;
  NSString *_typeIdentifier;
}
  
// Initializing the Data Asset
- (instancetype) initWithName: (NSDataAssetName)name;
- (instancetype) initWithName: (NSDataAssetName)name bundle: (NSBundle *)bundle;

// Accessing data...
- (NSData *) data;

// Getting data asset information
- (NSDataAssetName) name;
- (NSString *) typeIdentifier;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSDataAsset_h_GNUSTEP_GUI_INCLUDE */

