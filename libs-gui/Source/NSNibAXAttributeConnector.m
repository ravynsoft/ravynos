/** <title>NSNibAXAttributeConnector</title>

   <abstract>
   </abstract>

   Copyright (C) 2007 Free Software Foundation, Inc.

   Author: Gregory John Casamento
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

#import <GNUstepGUI/GSNibLoading.h>
#import <Foundation/NSString.h>

@implementation NSNibAXAttributeConnector
// Attribute name/type.
- (NSString *) attributeType
{
  return _attributeType;
}

- (void) setAttributeType: (NSString *)type
{
  ASSIGN(_attributeType, type);
}

- (NSString *) attributeValue
{
  return _attributeValue;
}

- (void) setAttributeValue: (NSString *)value
{
  ASSIGN(_attributeValue, value);
}

// Source destination, connectors.
- (id) destination
{
  return _destination;
}

- (void) setDestination: (id)destination
{
  ASSIGN(_destination, destination);
}

- (NSString *) label
{
  return _label;
}

- (void) setLabel: (NSString *)label
{
  ASSIGN(_label, label);
}

- (id) source
{
  return _source;
}

- (void) setSource: (id)source
{
  ASSIGN(_source, source);
}

// establish connection...
- (void) establishConnection
{
}

// archiving....
- (id) initWithCoder: (NSCoder *)coder
{
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
}

- (void) dealloc
{
  [super dealloc];
}
@end
