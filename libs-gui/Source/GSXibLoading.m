/* <title>GSXibLoading</title>

   <abstract>Xib (Cocoa XML) loading helper classes</abstract>

   Copyright (C) 2010, 2011 Free Software Foundation, Inc.

   Written by: Fred Kiefer <FredKiefer@gmx.de>
   Created: March 2010

   Author: Gregory John Casamento
   Date: 2010, 2012

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
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSKeyedArchiver.h>
#import "AppKit/NSControl.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSNibLoading.h"
#import "GNUstepGUI/GSXibLoading.h"

@interface IBAccessibilityAttribute : NSObject <NSCoding>
@end

@interface IBNSLayoutConstraint : NSObject <NSCoding>
@end

@interface IBLayoutConstant : NSObject <NSCoding>
@end

@implementation IBUserDefinedRuntimeAttribute
 
 - (void) encodeWithCoder: (NSCoder *)coder
 {
   if([coder allowsKeyedCoding])
     {
      [coder encodeObject: typeIdentifier forKey: @"typeIdentifier"];
      [coder encodeObject: keyPath forKey: @"keyPath"];
      [coder encodeObject: value forKey: @"value"];
     }
 }

- (id) initWithCoder: (NSCoder *)coder
{
  if([coder allowsKeyedCoding])
    {
      [self setTypeIdentifier: [coder decodeObjectForKey: @"typeIdentifier"]];
      [self setKeyPath: [coder decodeObjectForKey: @"keyPath"]];
      [self setValue: [coder decodeObjectForKey: @"value"]];
    }
  return self;
}

- (void) setTypeIdentifier: (NSString *)type
{
  ASSIGN(typeIdentifier, type);
}

- (NSString *) typeIdentifier
{
  return typeIdentifier;
}

- (void) setKeyPath: (NSString *)kpath
{
  ASSIGN(keyPath, kpath);
}

- (NSString *) keyPath
{
  return keyPath;
}

- (void) setValue: (id)val
{
  ASSIGN(value, val);
}

- (id) value
{
  return value;
}

- (NSString*) description
{
  NSMutableString *description = [[super description] mutableCopy];

  [description appendString: @" <"];
  [description appendFormat: @" type: %@", typeIdentifier];
  [description appendFormat: @" keyPath: %@", keyPath];
  [description appendFormat: @" value: %@", value];
  [description appendString: @">"];
  return AUTORELEASE(description);
}

@end

@implementation IBUserDefinedRuntimeAttributesPlaceholder

- (void) encodeWithCoder: (NSCoder *)coder
{
  if([coder allowsKeyedCoding])
  {
    [coder encodeObject: name forKey: @"IBUserDefinedRuntimeAttributesPlaceholderName"];
    [coder encodeObject: runtimeAttributes forKey: @"userDefinedRuntimeAttributes"];
  }
}

- (id) initWithCoder: (NSCoder *)coder
{
  if([coder allowsKeyedCoding])
  {
    [self setName: [coder decodeObjectForKey: @"IBUserDefinedRuntimeAttributesPlaceholderName"]];
    [self setRuntimeAttributes: [coder decodeObjectForKey: @"userDefinedRuntimeAttributes"]];
  }
  return self;
}

- (void) setName: (NSString *)value
{
  ASSIGN(name, value);
}

- (NSString *) name
{
  return name;
}

- (void) setRuntimeAttributes: (NSArray *)attrbutes
{
  ASSIGN(runtimeAttributes, attrbutes);
}

- (NSArray *) runtimeAttributes
{
  return runtimeAttributes;
}

@end

@implementation IBAccessibilityAttribute

- (void) encodeWithCoder: (NSCoder *)coder
{
}

- (id) initWithCoder: (NSCoder *)coder
{
  return self;
}

@end

@implementation IBNSLayoutConstraint
- (void) encodeWithCoder: (NSCoder *)coder
{
  // Do nothing...
}

- (id) initWithCoder: (NSCoder *)coder
{
  return self;
}
@end

@implementation IBLayoutConstant
- (void) encodeWithCoder: (NSCoder *)coder
{
  // Do nothing...
}

- (id) initWithCoder: (NSCoder *)coder
{
  return self;
}
@end

@implementation FirstResponder

+ (id) allocWithZone: (NSZone*)zone
{
  return nil;
}

@end

@implementation IBClassDescriptionSource

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"majorKey"])
        {
          ASSIGN(majorKey, [coder decodeObjectForKey: @"majorKey"]);
        }
      if ([coder containsValueForKey: @"minorKey"])
        {
          ASSIGN(minorKey, [coder decodeObjectForKey: @"minorKey"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(majorKey);
  DESTROY(minorKey);
  [super dealloc];
}

@end

@implementation IBPartialClassDescription

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"className"])
        {
          ASSIGN(className, [coder decodeObjectForKey: @"className"]);
        }
      if ([coder containsValueForKey: @"superclassName"])
        {
          ASSIGN(superclassName, [coder decodeObjectForKey: @"superclassName"]);
        }
      if ([coder containsValueForKey: @"actions"])
        {
          ASSIGN(actions, [coder decodeObjectForKey: @"actions"]);
        }
      if ([coder containsValueForKey: @"outlets"])
        {
          ASSIGN(outlets, [coder decodeObjectForKey: @"outlets"]);
        }
      if ([coder containsValueForKey: @"sourceIdentifier"])
        {
          ASSIGN(sourceIdentifier, [coder decodeObjectForKey: @"sourceIdentifier"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(className);
  DESTROY(superclassName);
  DESTROY(actions);
  DESTROY(outlets);
  DESTROY(sourceIdentifier);
  [super dealloc];
}

@end

@implementation IBClassDescriber

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"referencedPartialClassDescriptions"])
        {
          ASSIGN(referencedPartialClassDescriptions, [coder decodeObjectForKey: @"referencedPartialClassDescriptions"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(referencedPartialClassDescriptions);
  [super dealloc];
}

@end

@implementation IBConnection

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"label"])
        {
          ASSIGN(label, [coder decodeObjectForKey: @"label"]);
        }
      if ([coder containsValueForKey: @"source"])
        {
          ASSIGN(source, [coder decodeObjectForKey: @"source"]);
        }
      if ([coder containsValueForKey: @"destination"])
        {
          ASSIGN(destination, [coder decodeObjectForKey: @"destination"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  // FIXME
}

- (void) dealloc
{
  DESTROY(label);
  DESTROY(source);
  DESTROY(destination);
  [super dealloc];
}

- (NSString*) label
{
  return label;
}

- (id) source
{
  return source;
}

- (id) destination
{
  return destination;
}

- (NSNibConnector*) nibConnector
{
  NSString *tag = [self label];
  NSRange colonRange = [tag rangeOfString: @":"];
  NSUInteger location = colonRange.location;
  NSNibConnector *result = nil;

  if (location == NSNotFound)
    {
      result = [[NSNibOutletConnector alloc] init];
    }
  else
    {
      result = [[NSNibControlConnector alloc] init];
    }

  [result setDestination: [self destination]];
  [result setSource: [self source]];
  [result setLabel: [self label]];

  return result;
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ - label: %@, source: %@, destination: %@",
                   [super description], label, NSStringFromClass([source class]),
                   NSStringFromClass([destination class])];
}

- (id) nibInstantiate
{
  if ([source respondsToSelector: @selector(nibInstantiate)])
    {
      ASSIGN(source, [source nibInstantiate]);
    }
  if ([destination respondsToSelector: @selector(nibInstantiate)])
    {
      ASSIGN(destination, [destination nibInstantiate]);
    }

  return self;
}

- (void) establishConnection
{
}

@end

@implementation IBActionConnection

- (void) dealloc
{
  DESTROY(trigger);
  [super dealloc];
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if (self)
  {
    if ([coder allowsKeyedCoding])
      {
        // label and source string tags have changed for XIB5...
        if ([coder containsValueForKey: @"selector"])
          {
            ASSIGN(label, [coder decodeObjectForKey: @"selector"]);
          }
        if ([coder containsValueForKey: @"target"])
          {
            ASSIGN(source, [coder decodeObjectForKey: @"target"]);
          }

        // Looks like the 'trigger' attribute should be used to override the
        // action setup method...
        if ([coder containsValueForKey: @"trigger"])
          {
            ASSIGN(trigger, [coder decodeObjectForKey: @"trigger"]);
          }
      }
    else
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"Can't decode %@ with %@.", NSStringFromClass([self class]),
       NSStringFromClass([coder class])];
    }
  }
  return self;
}

- (NSString*) trigger
{
  return trigger;
}

- (void) establishConnection
{
  SEL sel = NSSelectorFromString(label);

  [destination setTarget: source];

  if (trigger && [trigger length])
    {
      NSString *selName = [NSString stringWithFormat: @"set%@%@:",
                           [[trigger substringToIndex: 1] uppercaseString],
                           [trigger substringFromIndex: 1]];
      SEL       trigsel = NSSelectorFromString(selName);

      if (sel && trigsel && [destination respondsToSelector: trigsel])
        {
          NSWarnMLog(@"setting trigger %@ to selector %@", selName, label);
          [destination performSelector: trigsel withObject: (id)sel];
        }
      else if (!sel)
        {
          NSWarnMLog(@"label %@ does not correspond to any selector", label);
        }
      else if (!trigsel)
        {
          NSWarnMLog(@"trigger %@ does not correspond to any selector", trigger);
        }
      else
        {
          NSWarnMLog(@"destination class (%@) does not respond to trigger selector %@",
                     NSStringFromClass([destination class]), selName);
        }
    }
  else
    {
      // Otherwise invoke the normal method...
      [destination setAction: sel];
    }
}

@end

@implementation IBOutletConnection

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if (self)
    {
      if ([coder allowsKeyedCoding])
        {
          // label string tag has changed for XIB5...
          if ([coder containsValueForKey: @"property"])
            {
              ASSIGN(label, [coder decodeObjectForKey: @"property"]);
            }
        }
      else
        {
          [NSException raise: NSInvalidArgumentException
                      format: @"Can't decode %@ with %@.", NSStringFromClass([self class]),
           NSStringFromClass([coder class])];
        }
    }
  return self;
}

- (void) establishConnection
{
  NS_DURING
    {
      if (source != nil)
	{
          NSString *selName;
          SEL sel;

          selName = [NSString stringWithFormat: @"set%@%@:",
                      [[label substringToIndex: 1] uppercaseString],
                      [label substringFromIndex: 1]];
          sel = NSSelectorFromString(selName);

          if (sel && [source respondsToSelector: sel])
            {
              [source performSelector: sel withObject: destination];
            }
          else
            {
              /*
               * We cannot use the KVC mechanism here, as this would always retain _dst
               * and it could also affect _setXXX methods and _XXX ivars that aren't
               * affected by the Cocoa code.
               */
              const char *name = [label cString];
              Class class = object_getClass(source);
              Ivar ivar = class_getInstanceVariable(class, name);

              if (ivar != 0)
                {
                  object_setIvar(source, ivar, destination);
                }
              else
                {
                  NSWarnMLog(@"class '%@' has no instance var named: %@", NSStringFromClass(class), label);
                }
           }
	}
    }
  NS_HANDLER
    {
      NSLog(@"Error while establishing connection %@: %@",self,[localException reason]);
    }
  NS_ENDHANDLER;
}

@end

@implementation IBBindingConnection

- (void) dealloc
{
  DESTROY(connector);
  [super dealloc];
}

- (id) initWithCoder: (NSCoder*)coder
{
  self = [super initWithCoder: coder];
  if (self == nil)
    return nil;

  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"connector"])
        {
          ASSIGN(connector, [coder decodeObjectForKey: @"connector"]);
        }
    }

  return self;
}

- (void) establishConnection
{
  [connector establishConnection];
}

@end

@implementation IBConnectionRecord

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"connection"])
        {
          ASSIGN(connection, [coder decodeObjectForKey: @"connection"]);
        }
      else
        {
          NSString *format = [NSString stringWithFormat:@"%s:Can't decode %@ without a connection ID",
                              __PRETTY_FUNCTION__,
                              NSStringFromClass([self class])];
          [NSException raise: NSInvalidArgumentException
                      format: @"%@", format];
        }

      // Load the connection ID....
      if ([coder containsValueForKey: @"connectionID"])
        {
          // PRE-4.6 XIBs....
          connectionID = [coder decodeIntForKey: @"connectionID"];
        }
      else if ([coder containsValueForKey: @"id"])
        {
          // 4.6+ XIBs....
          NSString *string = [coder decodeObjectForKey: @"id"];

          if (string && [string isKindOfClass:[NSString class]] && [string length])
            {
              connectionID = [string intValue];
            }
          else
            {
              NSString *format = [NSString stringWithFormat:@"%s:class: %@ - connection ID is missing or zero!",
                                  __PRETTY_FUNCTION__, NSStringFromClass([self class])];
              [NSException raise: NSInvalidArgumentException
                          format: @"%@", format];
            }
        }
      else
        {
          NSString *format = [NSString stringWithFormat:@"%s:Can't decode %@ without a connection ID",
                              __PRETTY_FUNCTION__,
                              NSStringFromClass([self class])];
          [NSException raise: NSInvalidArgumentException
                      format: @"%@", format];
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(connection);
  [super dealloc];
}

- (IBConnection*) connection
{
  return connection;
}

- (id) nibInstantiate
{
  ASSIGN(connection, [connection nibInstantiate]);
  return self;
}

- (void) establishConnection
{
  [connection establishConnection];
}

@end

@implementation IBToolTipAttribute

- (NSString*) toolTip
{
  return toolTip;
}

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"name"])
        {
          ASSIGN(name, [coder decodeObjectForKey: @"name"]);
        }
      if ([coder containsValueForKey: @"object"])
        {
          ASSIGN(object, [coder decodeObjectForKey: @"object"]);
        }
      if ([coder containsValueForKey: @"toolTip"])
        {
          ASSIGN(toolTip, [coder decodeObjectForKey: @"toolTip"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(name);
  DESTROY(object);
  DESTROY(toolTip);
  [super dealloc];
}

@end

@implementation IBInitialTabViewItemAttribute

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"name"])
        {
          ASSIGN(name, [coder decodeObjectForKey: @"name"]);
        }
      if ([coder containsValueForKey: @"object"])
        {
          ASSIGN(object, [coder decodeObjectForKey: @"object"]);
        }
      if ([coder containsValueForKey: @"initialTabViewItem"])
        {
          ASSIGN(initialTabViewItem, [coder decodeObjectForKey: @"initialTabViewItem"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(name);
  DESTROY(object);
  DESTROY(initialTabViewItem);
  [super dealloc];
}

@end

@implementation IBObjectRecord

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"objectID"])
        {
          // PRE-4.6 XIBs....
          objectID = [coder decodeObjectForKey: @"objectID"];
        }
      else if ([coder containsValueForKey: @"id"])
        {
          // 4.6+ XIBs....
          objectID = [coder decodeObjectForKey: @"id"];
        }
      else
        {
          // Cannot process without object ID...
          NSString *format = [NSString stringWithFormat:@"%s:Can't decode %@ without an object ID",
                              __PRETTY_FUNCTION__,
                              NSStringFromClass([self class])];
          [NSException raise: NSInvalidArgumentException
                      format: @"%@", format];
        }

      if ([coder containsValueForKey: @"object"])
        {
          ASSIGN(object, [coder decodeObjectForKey: @"object"]);
        }
      if ([coder containsValueForKey: @"children"])
        {
          ASSIGN(children, [coder decodeObjectForKey: @"children"]);
        }
      if ([coder containsValueForKey: @"parent"])
        {
          ASSIGN(parent, [coder decodeObjectForKey: @"parent"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(object);
  DESTROY(children);
  DESTROY(parent);
  [super dealloc];
}

- (id) object
{
  return object;
}

- (id) parent
{
  return parent;
}

- (id) objectID
{
  return objectID;
}

- (NSString *) description
{
  return [NSString stringWithFormat: @"<%@, %@, %@, %p>",
		   [self className],
		   object,
		   parent,
		   objectID];
}

@end

@implementation IBMutableOrderedSet

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"orderedObjects"])
        {
          ASSIGN(orderedObjects, [coder decodeObjectForKey: @"orderedObjects"]);
        }
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) dealloc
{
  DESTROY(orderedObjects);
  [super dealloc];
}

- (NSArray*) orderedObjects
{
  return orderedObjects;
}

- (id) objectWithObjectID: (id)objID
{
  NSEnumerator *en;
  IBObjectRecord *obj;

  en = [orderedObjects objectEnumerator];
  while ((obj = [en nextObject]) != nil)
    {
      if ([[obj objectID] isEqual:objID])
        {
          return [obj object];
        }
    }
  return nil;
}

@end

@implementation IBObjectContainer

- (id) initWithCoder: (NSCoder*)coder
{
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"sourceID"])
        {
          ASSIGN(sourceID, [coder decodeObjectForKey: @"sourceID"]);
        }
      if ([coder containsValueForKey: @"maxID"])
        {
          maxID = [coder decodeIntForKey: @"maxID"];
        }
      if ([coder containsValueForKey: @"flattenedProperties"])
        {
          ASSIGN(flattenedProperties, [coder decodeObjectForKey: @"flattenedProperties"]);
        }
      if ([coder containsValueForKey: @"objectRecords"])
        {
          ASSIGN(objectRecords, [coder decodeObjectForKey: @"objectRecords"]);
        }
      if ([coder containsValueForKey: @"connectionRecords"])
        {
          ASSIGN(connectionRecords, [coder decodeObjectForKey: @"connectionRecords"]);
        }
      // We could load more data here, but we currently don't need it.
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
                   format: @"Can't decode %@ with %@.",NSStringFromClass([self class]),
                   NSStringFromClass([coder class])];
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  // FIXME
}

- (void) dealloc
{
  DESTROY(connectionRecords);
  DESTROY(objectRecords);
  DESTROY(flattenedProperties);
  DESTROY(unlocalizedProperties);
  DESTROY(activeLocalization);
  DESTROY(localizations);
  DESTROY(sourceID);
  [super dealloc];
}

- (NSString*) description
{
  return [NSString stringWithFormat: @"%@ - sourceID: %@: maxID: %d:"
    @" objectRecords: %@: flattenedProperties: %@: connectionRecords: %@: ",
   [super description], sourceID, maxID,
   objectRecords, flattenedProperties, connectionRecords];
}

- (NSEnumerator*) connectionRecordEnumerator
{
  return [connectionRecords objectEnumerator];
}

- (NSEnumerator*) objectRecordEnumerator
{
  return [[objectRecords orderedObjects] objectEnumerator];
}

- (NSDictionary*) propertiesForObjectID: (id)objectID
{
  NSEnumerator *en;
  NSString *idString;
  NSString *key;
  NSMutableDictionary *properties;
  int idLength;

  idString = [NSString stringWithFormat: @"%@.", objectID];
  idLength = [idString length];
  properties = [[NSMutableDictionary alloc] init];
  en = [flattenedProperties keyEnumerator];
  while ((key = [en nextObject]) != nil)
    {
      if ([key hasPrefix: idString])
        {
          id value = [flattenedProperties objectForKey: key];
          [properties setObject: value forKey: [key substringFromIndex: idLength]];
        }
    }

  return AUTORELEASE(properties);
}

/*
  Returns a dictionary of the custom class names keyed on the objectIDs.
 */
- (NSDictionary*) customClassNames
{
  NSMutableDictionary *properties;
  int i;

  properties = [[NSMutableDictionary alloc] init];
  // We have special objects at -3, -2, -1 and 0
  for (i = -3; i < maxID; i++)
    {
      NSString *idString;
      id value;

      idString = [NSString stringWithFormat: @"%d.CustomClassName", i];
      value = [flattenedProperties objectForKey: idString];
      if (value)
        {
          NSString *key;

          key = [NSString stringWithFormat: @"%d", i];
          [properties setObject: value forKey: key];
        }
    }

  return properties;
}

- (id) nibInstantiate
{
  NSEnumerator *en;
  id obj;

  // iterate over connections, instantiate, and then establish them.
  en = [connectionRecords objectEnumerator];
  while ((obj = [en nextObject]) != nil)
    {
      [obj nibInstantiate];
      [obj establishConnection];
    }

  // awaken all objects.
  en = [[objectRecords orderedObjects] objectEnumerator];
  while ((obj = [en nextObject]) != nil)
    {
      id realObj;
      NSDictionary *properties;
      id value;

      realObj = [obj object];
      if ([realObj respondsToSelector: @selector(nibInstantiate)])
        {
          realObj = [realObj nibInstantiate];
        }

      properties = [self propertiesForObjectID: [obj objectID]];
      NSDebugLLog(@"XIB", @"object %@ props %@", [obj objectID], properties);

      //value = [properties objectForKey: @"windowTemplate.maxSize"];
      //value = [properties objectForKey: @"CustomClassName"];

      // Activate windows
      value = [properties objectForKey: @"NSWindowTemplate.visibleAtLaunch"];
      if (value != nil)
        {
          if ([value boolValue] == YES)
            {
              if ([realObj isKindOfClass: [NSWindow class]])
                {
                  // bring visible windows to front...
                  [(NSWindow *)realObj orderFront: self];
                }
            }
        }

      // Tool tips
      value = [properties objectForKey: @"IBAttributePlaceholdersKey"];
      if (value != nil)
        {
          NSDictionary *infodict = (NSDictionary*)value;

          // Process tooltips...
          IBToolTipAttribute *tooltip = [infodict objectForKey: @"ToolTip"];

          if (tooltip && [realObj respondsToSelector: @selector(setToolTip:)])
            {
              [realObj setToolTip: [tooltip toolTip]];
            }

          // Process XIB runtime attributes...
          if ([infodict objectForKey:@"IBUserDefinedRuntimeAttributesPlaceholderName"])
            {
              IBUserDefinedRuntimeAttributesPlaceholder *placeholder =
                [infodict objectForKey:@"IBUserDefinedRuntimeAttributesPlaceholderName"];
              NSArray *attributes = [placeholder runtimeAttributes];
              NSEnumerator *objectIter = [attributes objectEnumerator];
              IBUserDefinedRuntimeAttribute *attribute;

              while ((attribute = [objectIter nextObject]) != nil)
                {
                  [realObj setValue: [attribute value] forKeyPath: [attribute keyPath]];
                }
            }
        }

      if ([realObj respondsToSelector: @selector(awakeFromNib)])
        {
          [realObj awakeFromNib];
        }
    }

  return self;
}

@end
