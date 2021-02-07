/* <title>GSXibLoading</title>

   <abstract>Xib (Cocoa XML) model loader</abstract>

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Created: March 2010
   Refactored slightly by: Gregory Casamento <greg.casamento@gmail.com>
   Created: May 2010

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

#ifndef _GNUstep_H_GSXibElement
#define _GNUstep_H_GSXibElement

#import <Foundation/NSObject.h>

@class NSString, NSDictionary, NSMutableDictionary, NSMutableArray;

@interface GSXibElement: NSObject
{
  NSString *type;
  NSMutableDictionary *attributes;
  NSString *value;
  NSMutableDictionary *elements;
  NSMutableArray *values;
}

- (GSXibElement*) initWithType: (NSString*)typeName 
                 andAttributes: (NSDictionary*)attribs;
- (NSString*) type;
- (NSString*) value;
- (NSDictionary*) elements;
- (NSArray*) values;
- (void) addElement: (GSXibElement*)element;
- (void) setElement: (GSXibElement*)element forKey: (NSString*)key;
- (void) setValue: (NSString*)text;
- (NSString*) attributeForKey: (NSString*)key;
- (void) setAttribute: (NSString*)attribute forKey: (NSString*)key;
- (GSXibElement*) elementForKey: (NSString*)key;
- (NSDictionary *) attributes;
@end

#endif

