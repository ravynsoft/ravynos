/** <title>NSMovie</title>

   <abstract>Encapsulate a Quicktime movie</abstract>

   Copyright <copy>(C) 2003 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2003

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

#import <Foundation/NSArray.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSData.h>
#import <Foundation/NSURL.h>
#import "AppKit/NSMovie.h"
#import "AppKit/NSPasteboard.h"

@implementation NSMovie

+ (NSArray*) movieUnfilteredFileTypes
{
  return [NSArray arrayWithObject: @"mov"];
}

+ (NSArray*) movieUnfilteredPasteboardTypes
{
  // FIXME
  return [NSArray arrayWithObject: @"QuickTimeMovie"];
}

+ (BOOL) canInitWithPasteboard: (NSPasteboard*)pasteboard
{
  NSArray *pbTypes = [pasteboard types];
  NSArray *myTypes = [self movieUnfilteredPasteboardTypes];

  return ([pbTypes firstObjectCommonWithArray: myTypes] != nil);
}

- (id) initWithData: (NSData *)movie
{
  if (movie == nil)
    {
      RELEASE(self);
      return nil;
    }
  
  [super init];
  ASSIGN(_movie, movie);

  return self;
}

- (id) initWithMovie: (void*)movie
{
  //FIXME

  return self;
}

- (id) initWithURL: (NSURL*)url byReference: (BOOL)byRef
{
  NSData* data = [url resourceDataUsingCache: YES];

  self = [self initWithData: data];

  if (byRef)
    {
      ASSIGN(_url, url);
    }

  return self;
}

- (id) initWithPasteboard: (NSPasteboard*)pasteboard
{
  NSString *type;
  NSData* data;

  type = [pasteboard availableTypeFromArray: 
			 [object_getClass(self) movieUnfilteredPasteboardTypes]];
  if (type == nil)
    {
      //NSArray *array = [pasteboard propertyListForType: NSFilenamesPboardType];
      // FIXME
      data = nil;
    }
  else 
    {
      data = [pasteboard dataForType: type];
    }

  if (data == nil)
    {
      RELEASE(self);
      return nil;
    }

  self = [self initWithData: data];

  return self;
}

- (void) dealloc
{
  TEST_RELEASE(_url);
  TEST_RELEASE(_movie);
    
  [super dealloc];
}

- (void*) QTMovie
{
  return (void*)[_movie bytes];
}

- (NSURL*) URL
{
  return _url;
}

// NSCopying protocoll
- (id) copyWithZone: (NSZone *)zone
{
  NSMovie *new = (NSMovie*)NSCopyObject (self, 0, zone);

  new->_movie = [_movie copyWithZone: zone];
  new->_url = [_url copyWithZone: zone];
  return new;
}

// NSCoding protocoll
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      [aCoder encodeObject: _movie];
      [aCoder encodeObject: _url];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      ASSIGN (_movie, [aDecoder decodeObject]);
      ASSIGN (_url, [aDecoder decodeObject]);
    }
  return self;
}

@end
