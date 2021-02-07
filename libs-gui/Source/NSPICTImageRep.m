/* Implementation of class NSPICTImageRep
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory Casamento <greg.casamento@gmail.com>
   Date: Fri Nov 15 04:24:51 EST 2019

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
#import "config.h"

#import <AppKit/NSPICTImageRep.h>
#import <AppKit/NSPasteboard.h>
#import <Foundation/NSData.h>
#import <Foundation/NSArray.h>
#import <GNUstepGUI/GSImageMagickImageRep.h>

@implementation NSPICTImageRep

+ (NSArray *) imageUnfilteredFileTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      types = [[NSArray alloc] initWithObjects: @"pct", @"pict", nil];
    }

  return types;
}

+ (NSArray *) imageUnfilteredPasteboardTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      types = [[NSArray alloc] initWithObjects: NSPICTPboardType, nil];
    }
  
  return types;
}

+ (instancetype) imageRepWithData: (NSData *)imageData
{
  return [[[self class] alloc] initWithData: imageData];
}

- (instancetype) initWithData: (NSData *)imageData
{
  self = [super init];
  if (self != nil)
    {
#if HAVE_IMAGEMAGICK
      ASSIGN(_pageRep, [GSImageMagickImageRep imageRepWithData: imageData]);
      _size = [_pageRep size];
#else
      _pageRep = nil;
      _size = NSMakeSize(0,0);
#endif
      ASSIGNCOPY(_pictRepresentation, imageData);  
    }
  return self;
}

- (NSRect) boundingBox
{
  NSSize size = [self size];
  NSRect rect = NSMakeRect(0, 0, size.width, size.height);
  return rect;
}

- (NSData *) PICTRepresentation
{
  return [_pictRepresentation copy];
}

// Override to draw the specified page...
- (BOOL) draw
{
  return [_pageRep draw];
}
@end

