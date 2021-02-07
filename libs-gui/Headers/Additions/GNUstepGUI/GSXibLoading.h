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

#ifndef _GNUstep_H_GSXibLoading
#define _GNUstep_H_GSXibLoading

#import <Foundation/NSObject.h>
#import <Foundation/NSKeyedArchiver.h>
#import "GNUstepGUI/GSXibKeyedUnarchiver.h"

@class NSString, NSDictionary, NSArray, NSMutableDictionary, NSMutableArray;
@class NSNibBindingConnector;
@class GSXibElement;
@class NSNibConnector;

// Hack: This allows the class name FirstResponder in NSCustomObject and
// correctly returns nil as the corresponding object.
@interface FirstResponder: NSObject
{
}
@end

@interface IBClassDescriptionSource: NSObject
{
  NSString *majorKey;
  NSString *minorKey;
}
@end

@interface IBPartialClassDescription: NSObject
{
  NSString *className;
  NSString *superclassName;
  NSMutableDictionary *actions;
  NSMutableDictionary *outlets;
  IBClassDescriptionSource *sourceIdentifier;
}
@end

@interface IBClassDescriber: NSObject
{
  NSMutableArray *referencedPartialClassDescriptions;
}
@end

@interface IBConnection: NSObject <NSCoding>
{
  NSString *label;
  id source;
  id destination;
}
- (NSString *) label;
- (id) source;
- (id) destination;
- (NSNibConnector *) nibConnector;
- (id) nibInstantiate;
- (void) establishConnection;
@end

@interface IBActionConnection: IBConnection
{
  NSString *trigger;
}
@end

@interface IBOutletConnection: IBConnection
{
}
@end

@interface IBBindingConnection: IBConnection
{
  NSNibBindingConnector *connector;
}
@end

@interface IBConnectionRecord: NSObject
{
  IBConnection *connection;
  int connectionID;
}
- (IBConnection *) connection;
@end

@interface IBToolTipAttribute: NSObject
{
  NSString *name;
  id object;
  NSString *toolTip;
}
@end

@interface IBInitialTabViewItemAttribute: NSObject
{
  NSString *name;
  id object;
  id initialTabViewItem;
}
@end

@interface IBObjectRecord: NSObject
{
  id objectID;
  id object;
  id children;
  id parent;
}
- (id) object;
- (id) parent;
- (id) objectID;
@end

@interface IBMutableOrderedSet: NSObject
{
  NSArray *orderedObjects;
}
- (NSArray *)orderedObjects;
- (id) objectWithObjectID: (id)objID;
@end

@interface IBObjectContainer: NSObject <NSCoding>
{
  NSMutableArray *connectionRecords;
  IBMutableOrderedSet *objectRecords;
  NSMutableDictionary *flattenedProperties;
  NSMutableDictionary *unlocalizedProperties;
  id activeLocalization;
  NSMutableDictionary *localizations;
  id sourceID;
  int maxID;
}
- (id) nibInstantiate;
- (NSEnumerator *) connectionRecordEnumerator;
- (NSEnumerator *) objectRecordEnumerator;
@end

@interface IBUserDefinedRuntimeAttributesPlaceholder : NSObject <NSCoding>
{
  NSArray  *runtimeAttributes;
  NSString *name;
}

- (void) setName: (NSString *)name;
- (NSString *) name;

- (void) setRuntimeAttributes: (NSArray *)attributes;
- (NSArray *) runtimeAttributes;

@end

@interface IBUserDefinedRuntimeAttribute : NSObject <NSCoding>
{
  NSString *typeIdentifier;
  NSString *keyPath;
  id value;
}

- (void) setTypeIdentifier: (NSString *)type;
- (NSString *) typeIdentifier;

- (void) setKeyPath: (NSString *)keyPath;
- (NSString *) keyPath;

- (void) setValue: (id)value;
- (id) value;

@end

#endif
