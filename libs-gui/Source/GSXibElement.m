/* <title>GSXibElement</title>

   <abstract>Xib element</abstract>

   Copyright (C) 2010, 2011 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Created: March 2010
   Written by: Gregory Casamento <greg.casamento@gmail.com>
   Created: March 2014

   This file is part of the GNUstep Base Library.

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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02110 USA.
*/

#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

#import "GNUstepGUI/GSXibElement.h"

@implementation GSXibElement

- (GSXibElement*) initWithType: (NSString*)typeName 
               andAttributes: (NSDictionary*)attribs
{
  ASSIGN(type, typeName);
  attributes = [attribs mutableCopy];
  elements = [[NSMutableDictionary alloc] init];
  values = [[NSMutableArray alloc] init];

  return self;
}

- (void) dealloc
{
  DESTROY(type);
  DESTROY(attributes);
  DESTROY(elements);
  DESTROY(values);
  DESTROY(value);
  [super dealloc];
}

- (NSString*) type
{
  return type;
}

- (NSString*) value
{
  return value;
}

- (NSDictionary*) elements
{
  return elements;
}

- (NSArray*) values
{
  return values;
}

- (void) addElement: (GSXibElement*)element
{
  [values addObject: element];
}

- (void) setElement: (GSXibElement*)element forKey: (NSString*)key
{
  [elements setObject: element forKey: key];
}

- (void) setValue: (NSString*)text
{
  ASSIGN(value, text);
}

- (NSString*) attributeForKey: (NSString*)key
{
  return [attributes objectForKey: key];
}

- (void) setAttribute: (NSString*)attribute forKey: (NSString*)key
{
  [attributes setObject: attribute forKey: key];
}

- (GSXibElement*) elementForKey: (NSString*)key
{
  return [elements objectForKey: key];
}

- (NSDictionary *)attributes
{
  return attributes;
}

- (NSString*) description
{
  return [NSString stringWithFormat: 
                     @"GSXibElement <%@> attrs (%@) elements [%@] values [%@] %@", 
                   type, attributes, elements, values, value, nil];
}

@end

