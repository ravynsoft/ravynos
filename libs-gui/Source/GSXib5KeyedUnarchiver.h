/** <title>GSXib5KeyedUnarchiver.h</title>

 <abstract>The XIB 5 keyed unarchiver</abstract>

 Copyright (C) 2016,2017 Free Software Foundation, Inc.

 Author:  Marcian Lytwyn <gnustep@advcsi.com>
 Date: 12/28/16

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

#import <Foundation/Foundation.h>
#import "GNUstepGUI/GSXibKeyedUnarchiver.h"

@class GSXibElement;

@interface GSXib5KeyedUnarchiver : GSXibKeyedUnarchiver
{
  GSXibElement        *_IBObjectContainer;
  GSXibElement        *_connectionRecords;
  GSXibElement        *_objectRecords;
  GSXibElement        *_orderedObjects;
  GSXibElement        *_flattenedProperties;
  GSXibElement        *_runtimeAttributes;
  NSMutableDictionary *_orderedObjectsDict;
  NSArray             *_resources;
}

- (NSRange) decodeRangeForKey: (NSString*)key;
@end
