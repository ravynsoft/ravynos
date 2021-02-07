/** <title>NSObjectController</title>

   <abstract>Controller class</abstract>

   Copyright <copy>(C) 2006 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: June 2006

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
#import <Foundation/NSDictionary.h>
#import <Foundation/NSPredicate.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArchiver.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSKeyValueCoding.h>
#import "AppKit/NSObjectController.h"
#import "AppKit/NSKeyValueBinding.h"
#import "GSBindingHelpers.h"

@interface _NSManagedProxy : NSObject
{
  NSString *_entity_name_key;
}

- (void) setEntityName: (NSString *)name;
- (NSString *) entityName;
@end

@implementation _NSManagedProxy
- (id) initWithCoder: (NSCoder *)coder
{
  if ((self = [super init]) != nil)
    {
      if ([coder allowsKeyedCoding])
	{
	  ASSIGN(_entity_name_key,[coder decodeObjectForKey: @"NSEntityName"]);
	}
      else
	{
	  ASSIGN(_entity_name_key,[coder decodeObject]);
	}
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeObject: _entity_name_key forKey: @"NSEntityName"];
    }
  else
    {
      [coder encodeObject: _entity_name_key];
    }
}

- (void) dealloc
{
  RELEASE(_entity_name_key);
  [super dealloc];
}

- (void) setEntityName: (NSString *)name
{
  ASSIGN(_entity_name_key, name);
}

- (NSString *) entityName
{
  return _entity_name_key;
}
@end

@implementation NSObjectController

+ (void) initialize
{
  if (self == [NSObjectController class])
    {
      [self exposeBinding: NSContentObjectBinding];
      [self setKeys: [NSArray arrayWithObject: @"editable"] 
            triggerChangeNotificationsForDependentKey: @"canAdd"];
      [self setKeys: [NSArray arrayWithObject: @"editable"] 
            triggerChangeNotificationsForDependentKey: @"canRemove"];
      [self setKeys: [NSArray arrayWithObjects: @"content", NSContentObjectBinding, nil] 
            triggerChangeNotificationsForDependentKey: @"selectedObjects"];
      [self setKeys: [NSArray arrayWithObjects: @"content", NSContentObjectBinding, nil] 
            triggerChangeNotificationsForDependentKey: @"selection"];
    }
}

- (id) initWithContent: (id)content
{
  if ((self = [super init]) != nil)
    {
      [self setContent: content];
      [self setObjectClass: [NSMutableDictionary class]];
      [self setEditable: YES];
      _managed_proxy = nil;
    }

  return self;
}

- (id) init
{
  return [self initWithContent: nil];
}

- (void) dealloc
{
  [GSKeyValueBinding unbindAllForObject: self];
  RELEASE(_content);
  RELEASE(_entity_name_key);
  RELEASE(_fetch_predicate);
  RELEASE(_selection);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder *)coder
{ 
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeBool: _is_editable forKey: @"NSEditable"];
      [coder encodeBool: _automatically_prepares_content forKey: @"NSAutomaticallyPreparesContent"];
      [coder encodeObject: _managed_proxy forKey: @"_NSManagedProxy"];
      [coder encodeObject: NSStringFromClass([self objectClass])
                   forKey: @"NSObjectClassName"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_is_editable];
      [coder encodeValueOfObjCType: @encode(BOOL) at: &_automatically_prepares_content];
      [coder encodeConditionalObject: _managed_proxy];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{ 
  if ((self = [super initWithCoder: coder]) != nil)
    {
      if ([self automaticallyPreparesContent])
	{
	  if ([self managedObjectContext] != nil)
	    {
	      [self fetch: coder];  
	    }
	  else 
	    {
	      [self prepareContent];
	    }
	}
      
      if ([coder allowsKeyedCoding])
	{
	  _is_editable = [coder decodeBoolForKey: @"NSEditable"];
	  _automatically_prepares_content = [coder decodeBoolForKey: @"NSAutomaticallyPreparesContent"];
	  ASSIGN(_managed_proxy, [coder decodeObjectForKey: @"_NSManagedProxy"]);
          if ([coder containsValueForKey: @"NSObjectClassName"])
            {
              NSString *className = [coder decodeObjectForKey: @"NSObjectClassName"];
              [self setObjectClass: NSClassFromString(className)];
            }
	}
      else
	{
	  [coder decodeValueOfObjCType: @encode(BOOL) at: &_is_editable];
	  [coder decodeValueOfObjCType: @encode(BOOL) at: &_automatically_prepares_content];
	  ASSIGN(_managed_proxy, [coder decodeObject]);
	}
    }

  return self; 
}

- (id) content
{
  return _content;
}

- (void) setContent: (id)content
{
  if (content != _content)
    {
      NSMutableArray *selection;

      ASSIGN(_content, content);
      if (content)
        {
          selection = [[NSMutableArray alloc] initWithObjects: &content count: 1];
        }
      else
        {
          selection = [[NSMutableArray alloc] init];
        }
      ASSIGN(_selection, selection);
      RELEASE(selection);
    }
}

- (void)bind: (NSString *)binding 
    toObject: (id)anObject
 withKeyPath: (NSString *)keyPath
     options: (NSDictionary *)options
{
  if ([binding isEqual: NSContentObjectBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueBinding alloc] initWithBinding: @"content" 
                                              withName: binding
                                              toObject: anObject
                                           withKeyPath: keyPath
                                               options: options
                                            fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else
    {
      [super bind: binding 
         toObject: anObject 
      withKeyPath: keyPath 
          options: options];
    }
}

- (Class) objectClass
{
  return _object_class;
}

- (void) setObjectClass: (Class)aClass
{
  _object_class = aClass;
}

- (id) newObject
{
  return [[[self objectClass] alloc] init];
}

- (void) prepareContent
{
  id new = [self newObject];

  [self setContent: new];
  RELEASE(new);
}

- (BOOL) automaticallyPreparesContent
{
  return _automatically_prepares_content;
}

- (void) setAutomaticallyPreparesContent: (BOOL)flag
{ 
  _automatically_prepares_content = flag;
}

- (void) add: (id)sender
{
  if ([self canAdd])
    {
      id new = [self newObject];

      [self addObject: new];
      RELEASE(new);
    }
}

- (void) addObject: (id)obj
{
  GSKeyValueBinding *theBinding;

  [self setContent: obj];
  theBinding = [GSKeyValueBinding getBinding: NSContentObjectBinding 
                                   forObject: self];
  if (theBinding != nil)
    [theBinding reverseSetValueFor: @"content"];
}

- (void) remove: (id)sender
{
  if ([self canRemove])
    {
      [self removeObject: [self content]];
    }
}

- (void) removeObject: (id)obj
{
  if (obj == [self content])
    {
      GSKeyValueBinding *theBinding;

      [self setContent: nil];
      theBinding = [GSKeyValueBinding getBinding: NSContentObjectBinding 
                                       forObject: self];
      if (theBinding != nil)
        [theBinding reverseSetValueFor: @"content"];
    }
}

- (BOOL) canAdd
{
  return [self isEditable];
}

- (BOOL) canRemove
{
  return [self isEditable] && [[self selectedObjects] count] > 0;
}

- (BOOL) isEditable
{
  return _is_editable;
}

- (void) setEditable: (BOOL)flag
{
  _is_editable = flag;
}

- (NSArray*) selectedObjects
{
  // TODO
  return _selection;
}

- (id) selection
{
  // TODO
  return _content;
}


- (BOOL) validateMenuItem: (id <NSMenuItem>)item
{
  SEL action = [item action];

  if (sel_isEqual(action, @selector(add:)))
    {
      return [self canAdd];
    }
  else if (sel_isEqual(action, @selector(remove:)))
    {
      return [self canRemove];
    }

  return YES;
}

- (NSString*) entityNameKey
{
  return _entity_name_key;
}

- (void) setEntityName: (NSString*)entityName
{
  ASSIGN(_entity_name_key, entityName);
}

- (NSPredicate*) fetchPredicate
{
  return _fetch_predicate;
}

- (void) setFetchPredicate: (NSPredicate*)predicate
{
  ASSIGN(_fetch_predicate, predicate);
}

- (void) fetch: (id)sender
{
  NSError *error;
  
  [self fetchWithRequest: nil merge: NO error: &error];
}

- (BOOL) fetchWithRequest: (NSFetchRequest*)fetchRequest
                    merge: (BOOL)merge
                    error: (NSError**)error
{
  // TODO
  //[_managed_object_context executeFetchRequest: fetchRequest error: error];
  return NO;
}

- (NSManagedObjectContext*) managedObjectContext
{
  return _managed_object_context;
}

- (void) setManagedObjectContext: (NSManagedObjectContext*)managedObjectContext
{
  _managed_object_context = managedObjectContext;
}

@end
