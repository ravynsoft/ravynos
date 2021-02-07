/** <title>NSNibBindingConnector</title>

   <abstract>
   </abstract>

   Copyright (C) 2007 Free Software Foundation, Inc.

   Author: Gregory John Casamento
   Date: 2007
   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: Nov 2007

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

#import <GNUstepGUI/GSNibLoading.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <AppKit/NSKeyValueBinding.h>

@implementation NSNibBindingConnector

- (id) init
{
  if((self = [super init]) != nil)
    {
      _binding = @"";
      _keyPath = @"";
      _options = RETAIN([NSMutableDictionary dictionary]);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_binding);
  RELEASE(_keyPath);
  RELEASE(_options);
  [super dealloc];
}

- (NSString *) binding
{
  return _binding;
}

- (NSString *) keyPath
{
  return _keyPath;
}

- (NSDictionary *) options
{
  return _options;
}

- (void) setBinding: (NSString *)binding
{
  ASSIGN(_binding, binding);
}

- (void) setKeyPath: (NSString *)keyPath
{
  ASSIGN(_keyPath, keyPath);
}

- (void) setOptions: (NSDictionary *)options
{
  ASSIGN(_options, options);
}

- (void) replaceObject: (id)anObject withObject: (id)anotherObject
{
  if (_binding == anObject)
    {
      ASSIGN(_binding, anotherObject);
    }
  if (_keyPath == anObject)
    {
      ASSIGN(_keyPath, anotherObject);
    }
  if (_options == anObject)
    {
      ASSIGN(_options, anotherObject);
    }

  [super replaceObject: anObject withObject: anotherObject];
}

- (void) establishConnection
{
  [_src bind: _binding 
        toObject: _dst
        withKeyPath: _keyPath
        options: _options];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      if (_binding != nil)
        {
          [aCoder encodeObject: _binding forKey: @"NSBinding"];
        }
      if (_keyPath != nil)
        {
          [aCoder encodeObject: _keyPath forKey: @"NSKeyPath"];
        }
      if (_options != nil)
        {
          [aCoder encodeObject: _options forKey: @"NSOptions"];
        }
    }
  else
    {
      [aCoder encodeObject: _binding];
      [aCoder encodeObject: _keyPath];
      [aCoder encodeObject: _options];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if (!(self = [super initWithCoder: aDecoder]))
    {
      return nil;
    }

  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSNibBindingConnectorVersion"])
        {
          int version = [aDecoder decodeIntForKey: @"NSNibBindingConnectorVersion"];
          if (version != 2)
            {
              NSLog(@"Unexpected NSNibBindingConnectorVersion %d", version);
              RELEASE(self);
              return nil;
            }
        }
      if ([aDecoder containsValueForKey: @"NSBinding"])
        {
          ASSIGN(_binding, [aDecoder decodeObjectForKey: @"NSBinding"]);
        }
      if ([aDecoder containsValueForKey: @"NSKeyPath"])
        {
          ASSIGN(_keyPath, [aDecoder decodeObjectForKey: @"NSKeyPath"]);
        }
      if ([aDecoder containsValueForKey: @"NSOptions"])
        {
          ASSIGN(_options, [aDecoder decodeObjectForKey: @"NSOptions"]);
        }
    }
  else
    {
      ASSIGN(_binding,[aDecoder decodeObject]);
      ASSIGN(_keyPath,[aDecoder decodeObject]);
      ASSIGN(_options,[aDecoder decodeObject]);
    }

  return self;
}

- (NSString *) description
{
  return [NSString stringWithFormat: @"<%@ binding=%@ keypath=%@ options=%@>",
                   [super description],
                   [self binding],
                   [self keyPath],
                   [self options]];
}

@end
