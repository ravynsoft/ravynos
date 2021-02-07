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

#import <Foundation/NSCoder.h>
#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import <AppKit/NSColorSpace.h>
#import "GSGuiPrivate.h"

@implementation NSColorSpace

- (id) _initWithColorSpaceModel: (NSColorSpaceModel)model
{
  if ((self = [super init]))
    {
      // FIXME: Load corresponding data

      _colorSpaceModel = model;
    }
  return self;
}

#define COLORSPACE(model) \
  static NSColorSpace *csp = nil;                     \
  if (!csp)                                                           \
    csp = [[self alloc] _initWithColorSpaceModel: model];             \
  return csp;

+ (NSColorSpace *) deviceCMYKColorSpace
{
  COLORSPACE(NSCMYKColorSpaceModel);
}

+ (NSColorSpace *) deviceGrayColorSpace
{
  COLORSPACE(NSGrayColorSpaceModel); 
}

+ (NSColorSpace *) deviceRGBColorSpace
{
  COLORSPACE(NSRGBColorSpaceModel);
}

+ (NSColorSpace *) genericCMYKColorSpace
{
  COLORSPACE(NSCMYKColorSpaceModel);
}

+ (NSColorSpace *) genericGrayColorSpace
{
  COLORSPACE(NSGrayColorSpaceModel);
}

+ (NSColorSpace *) genericRGBColorSpace
{
  COLORSPACE(NSRGBColorSpaceModel);
}

- (id) initWithColorSyncProfile: (void *)prof
{
  if ((self = [super init]))
    {
      _colorSyncProfile = prof;
      _colorSpaceModel = NSUnknownColorSpaceModel;
    }
  return self;
}

- (id) initWithICCProfileData: (NSData *)iccData
{
  if ((self = [super init]))
    {
      ASSIGN(_iccData, iccData);
      _colorSpaceModel = NSUnknownColorSpaceModel;
    }
  return self;
}

-(void) dealloc
{
  // FIXME: Free _colorSyncProfile
  TEST_RELEASE(_iccData);
  [super dealloc];
}

- (NSColorSpaceModel) colorSpaceModel
{
  return _colorSpaceModel;
}

- (void *) colorSyncProfile
{
  return _colorSyncProfile;
}

- (NSData *) ICCProfileData
{
  if (!_iccData)
    {
      // FIXME: Try to compute this from _colorSyncProfile
    }
  return _iccData;
}

- (NSString *) localizedName
{
  switch (_colorSpaceModel)
    {
      default:
      case NSUnknownColorSpaceModel: 
        return NSLocalizedString(@"unknown", @"color space");
      case NSGrayColorSpaceModel:
        return NSLocalizedString(@"Grayscale", @"color space");
      case NSRGBColorSpaceModel:
        return NSLocalizedString(@"RGB", @"color space");
      case NSCMYKColorSpaceModel:
        return NSLocalizedString(@"CMYK", @"color space");
      case NSLABColorSpaceModel:
        return NSLocalizedString(@"LAB", @"color space");
      case NSDeviceNColorSpaceModel:
        return NSLocalizedString(@"DeviceN", @"color space");
    }
}

- (int) numberOfColorComponents
{
  switch (_colorSpaceModel)
    {
      default:
      case NSUnknownColorSpaceModel: return 0;
      case NSGrayColorSpaceModel: return 1;
      case NSRGBColorSpaceModel: return 3;
      case NSCMYKColorSpaceModel: return 4;
      case NSLABColorSpaceModel: return 3;	// FIXME
      case NSDeviceNColorSpaceModel: return 3;	// FIXME
    }
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // FIXME
  if ([coder allowsKeyedCoding])
    {
    }
  else
    {
    }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  // FIXME
  if ([aDecoder allowsKeyedCoding])
    {
    }
  else
    {
    }
  return self;
}

@end
