/** <title>NSPasteboardItem</title>

   <abstract>class for wrapping pasteboard content</abstract>

   Copyright <copy>(C) 2017 Free Software Foundation, Inc.</copy>

   Author: Daniel Ferreira <dtf@stanford.edu>
   Date: July 2017

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*
 * FIXME: this class is useless until NSPasteboard implements
 * NSPasteboardReading + NSPasteboardWriting.
 */

#import <Foundation/Foundation.h>
#import <AppKit/NSPasteboardItem.h>

@implementation NSPasteboardItem
- (id)init
{
  if ((self = [super init]))
    {
      _providerMap = [[NSMutableDictionary alloc] init];
      _dataMap = [[NSMutableDictionary alloc] init];
      _types = [[NSMutableArray alloc] init];
    }

  return self;
}

- (NSArray *)types
{
  return [NSArray arrayWithArray: _types];
}

- (BOOL)setDataProvider:(id<NSPasteboardItemDataProvider>)dataProvider
               forTypes:(NSArray *)types
{
  NSUInteger i;

  if (![dataProvider conformsToProtocol: @protocol(NSPasteboardItemDataProvider)])
    {
      NSLog(@"Pasteboard item data provider %@ must conform to"
             "NSPasteboardItemDataProviderProtocol", dataProvider);
      return NO;
    }

  for (i = 0; i < [types count]; i++)
    {
      NSString *type = [types objectAtIndex: i];
      [_providerMap setObject: dataProvider forKey: type];
    }

  [_types addObjectsFromArray: types];
  return YES;
}

- (NSString *)availableTypeFromArray:(NSArray *)types
{
  // FIXME: this would require a dependency on UTIs
  return nil;
}

- (BOOL)setData:(NSData *)data forType:(NSString *)type
{
  if (![data isKindOfClass: [NSData class]])
    {
      return NO;
    }
  
  [_dataMap setObject: data forKey: type];
  return YES;
}

- (BOOL)setString:(NSString *)string forType:(NSString *)type
{
  if (![string isKindOfClass: [NSString class]])
    return NO;
  
  [_dataMap setObject: string forKey: type];
  return YES;
}

- (BOOL)setPropertyList:(id)propertyList forType:(NSString *)type
{
  if (![NSPropertyListSerialization propertyList: propertyList
                                isValidForFormat: NSPropertyListXMLFormat_v1_0])
    return NO;

  [_dataMap setObject: propertyList forKey: type];
  return YES;
}

- (NSData *)dataForType:(NSString *)type
{
  id object = [_dataMap objectForKey: type];
  if ([object isKindOfClass: [NSData class]])
    return (NSData *)object;
  
  return [NSPropertyListSerialization dataWithPropertyList: object
                                          format: NSPropertyListXMLFormat_v1_0
                                         options: 0
                                           error: NULL];
}

- (NSString *)stringForType:(NSString *)type
{
  id object = [_dataMap objectForKey: type];
  if ([object isKindOfClass: [NSString class]])
    return (NSString *)object;
  
  NSData *data = [self dataForType: type];
  return [[[NSString alloc] initWithData: data
                                encoding: NSUTF8StringEncoding] autorelease];
}

- (id)propertyListForType:(NSString *)type
{
  id object = [_dataMap objectForKey: type];
  if (![object isKindOfClass: [NSData class]])
    return object;

  return [NSPropertyListSerialization propertyListWithData: object
                               options: 0
                                format: NULL 
                                 error: NULL];
}

// Delegate Methods
- (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type
{
  if ((self = [self init]))
    {
      [self setPropertyList: propertyList forType: type];
    }

  return self;
}

+ (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard
{
  return [NSArray arrayWithObject: @"public.data"];
}

- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
  return _types;
}

- (id)pasteboardPropertyListForType:(NSString *)type
{
  return [self propertyListForType: type];
}

- (void)dealloc
{
  RELEASE(_providerMap);
  RELEASE(_dataMap);
  RELEASE(_types);
  
  [super dealloc];
}
@end
