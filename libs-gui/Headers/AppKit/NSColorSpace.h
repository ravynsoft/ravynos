/* 
   NSColorSpace.h

   The color space class

   Copyright (C) 2007 Free Software Foundation, Inc.

   Author:  H. Nikolaus Schaller <hns@computer.org>
   Date: 2007
   
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

#ifndef _GNUstep_H_NSColorSpace
#define _GNUstep_H_NSColorSpace
#import <GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
#import <Foundation/NSObject.h>

@class NSData;
@class NSString;

typedef enum _NSColorSpaceModel
{
  NSUnknownColorSpaceModel = -1,
  NSGrayColorSpaceModel,
  NSRGBColorSpaceModel,
  NSCMYKColorSpaceModel,
  NSLABColorSpaceModel,
  NSDeviceNColorSpaceModel
} NSColorSpaceModel;

@interface NSColorSpace : NSObject <NSCoding>
{
  NSColorSpaceModel _colorSpaceModel;
  NSData *_iccData;
  void *_colorSyncProfile;
}

+ (NSColorSpace *)deviceCMYKColorSpace;
+ (NSColorSpace *)deviceGrayColorSpace;
+ (NSColorSpace *)deviceRGBColorSpace;
+ (NSColorSpace *)genericCMYKColorSpace;
+ (NSColorSpace *)genericGrayColorSpace;
+ (NSColorSpace *)genericRGBColorSpace;

- (NSColorSpaceModel)colorSpaceModel;
- (void *)colorSyncProfile;
- (NSData *)ICCProfileData;
- (id)initWithColorSyncProfile:(void *)prof;
- (id)initWithICCProfileData:(NSData *)iccData;
- (NSString *)localizedName;
- (int)numberOfColorComponents;

@end

#endif // MAC_OS_X_VERSION_10_4
#endif // _GNUstep_H_NSColorSpace
