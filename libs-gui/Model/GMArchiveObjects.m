/*
   GMArchiveObjects.m

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: October 1997

   Copyright (C) 1997 Free Software Foundation, Inc.
   All rights reserved.

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

/* This file declares categories to various OpenStep classes so they get
   archived correctly by GMArchiver. The main things deal with encoding in
   place of some objects like imutable strings, arrays, dictionaries and
   data objects (basically the property list classes).

  This file is included by GMArchiver rather than compiled as a separate file
  because of the linking problems with categories (they are not linked into
  the executable even if you refer a method from category; you should refer a
  symbol from the category's file in order to force it link.
 */

#ifndef GNUSTEP
#include <Foundation/Foundation.h>
#else
#include <Foundation/NSData.h>
#endif


@implementation NSObject (ModelArchivingMethods)
- (id)replacementObjectForModelArchiver:(GMArchiver*)archiver
{
  return [self replacementObjectForCoder:nil];
}

- (Class)classForModelArchiver
{
  return [self classForCoder];
}

+ (id)createObjectForModelUnarchiver:(GMUnarchiver*)unarchiver
{
  return [[[self allocWithZone:[unarchiver objectZone]] init] autorelease];
}

/* Works around a bug in MacOSX 10.1.x?. [NSPatternColor -isEqual:] doesn't
   check if the object is a color before calling colorSpaceName on it */
- (NSString *) colorSpaceName
{
  return nil;
}
@end


@implementation NSString (ModelArchivingMethods)
- (void)encodeWithModelArchiver:(id)archiver
{
  [archiver encodeString:self withName:@"string"];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  return [unarchiver decodeStringWithName:@"string"];
}

- (Class)classForModelArchiver
{
  return [NSString class];
}
@end


@implementation NSMutableString (ModelArchivingMethods)
- (Class)classForModelArchiver
{
  return [NSMutableString class];
}

- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  return [[[unarchiver decodeStringWithName:@"string"] mutableCopy]
		autorelease];
}
@end


@implementation NSArray (ModelArchivingMethods)
- (void)encodeWithModelArchiver:(id)archiver
{
  [archiver encodeArray:self withName:@"elements"];
}

- (Class)classForModelArchiver
{
  return [NSMutableArray class];
}
@end


@implementation NSMutableArray (ModelArchivingMethods)
- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  id array = [unarchiver decodeArrayWithName:@"elements"];
  int i, count;

  for (i = 0, count = [array count]; i < count; i++)
    [self addObject:[array objectAtIndex:i]];

  return self;
}
@end


@implementation NSDictionary (ModelArchivingMethods)
- (void)encodeWithModelArchiver:(id)archiver
{
  [archiver encodeDictionary:self withName:@"elements"];
}

- (Class)classForModelArchiver
{
  return [NSMutableDictionary class];
}
@end


@implementation NSMutableDictionary (ModelArchivingMethods)
- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  id dictionary = [unarchiver decodeDictionaryWithName:@"elements"];
  id enumerator = [dictionary keyEnumerator];
  id key, value;

  while ((key = [enumerator nextObject])) {
    value = [dictionary objectForKey:key];
    [self setObject:value forKey:key];
  }

  return self;
}
@end


@implementation NSData (ModelArchivingMethods)
- (void)encodeWithModelArchiver:(id)archiver
{
  [archiver encodeData:self withName:@"data"];
}

- (Class)classForModelArchiver
{
  return [NSMutableData class];
}
@end


@implementation NSMutableData (ModelArchivingMethods)
- (id)initWithModelUnarchiver:(GMUnarchiver*)unarchiver
{
  id data = [unarchiver decodeDataWithName:@"data"];

  [self appendData:data];
  return self;
}
@end
