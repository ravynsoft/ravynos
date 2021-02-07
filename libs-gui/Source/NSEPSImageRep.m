/** <title>NSEPSImageRep</title>

   <abstract>EPS image representation.</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.
   
   Author:  Adam Fedor <fedor@colorado.edu>
   Date: Feb 1996
   
   This file is part of the GNUstep Application Kit Library.

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

#import "config.h"

#import <Foundation/NSArray.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSData.h>
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSEPSImageRep.h"
#import "GNUstepGUI/GSImageMagickImageRep.h"

@implementation NSEPSImageRep 

+ (BOOL) canInitWithData: (NSData *)data
{
  NSData *header = [data subdataWithRange: NSMakeRange(0,4)];
  NSString *str = [[NSString alloc] initWithData: header encoding: NSUTF8StringEncoding];
  BOOL result = [str isEqualToString: @"%!PS"];
  AUTORELEASE(str);
  return result;
}

+ (NSArray *) imageUnfilteredFileTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      types = [[NSArray alloc] initWithObjects: @"eps", nil];
    }

  return types;
}

+ (NSArray *) imageUnfilteredPasteboardTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      types = [[NSArray alloc] initWithObjects: NSPostScriptPboardType, nil];
    }
  
  return types;
}

// Initializing a New Instance 
+ (id) imageRepWithData: (NSData *)epsData
{
  return AUTORELEASE([[self alloc] initWithData: epsData]);
}

- (id) initWithData: (NSData *)epsData
{
  self = [super init];
  if (self != nil)
    {
#if HAVE_IMAGEMAGICK
      ASSIGN(_pageRep, [GSImageMagickImageRep imageRepWithData: epsData]);
      _size = [_pageRep size];
#else
      _pageRep = nil;
      _size = NSMakeSize(0,0);
#endif
      ASSIGNCOPY(_epsData, epsData);
    }
  
  return self;
}

- (void) dealloc
{
  RELEASE(_epsData);
  RELEASE(_pageRep);
  [super dealloc];
}

// Getting Image Data 
- (NSRect) boundingBox
{
  NSSize size = [self size];
  NSRect rect = NSMakeRect(0, 0, size.width, size.height);
  return rect;
}

- (NSData *) EPSRepresentation
{
  return _epsData;
}

- (void) prepareGState
{
  // This is for subclasses only
}

// Override to draw the specified page...
- (BOOL) draw
{
  [self prepareGState];
  
  return [_pageRep draw];
}

// NSCopying protocol
- (id) copyWithZone: (NSZone *)zone
{
  NSEPSImageRep *copy = [super copyWithZone: zone];

  copy->_epsData = [_epsData copyWithZone: zone];

  return copy;
}

// NSCoding protocol
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  NSData *data = [self EPSRepresentation];

  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      [data encodeWithCoder: aCoder];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  NSData *data = nil;

  self = [super initWithCoder: aDecoder];
  if ([aDecoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      data = [aDecoder decodeObject];
    }
  return [self initWithData: data];
}

@end
