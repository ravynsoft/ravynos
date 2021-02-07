/** <title>NSUserDefaultsController</title>

   <abstract>Controller class for user defaults</abstract>

   Copyright <copy>(C) 2006 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: September 2006

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

#import <Foundation/NSCoder.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/NSUserDefaultsController.h"

static id shared = nil;

@interface GSUserDefaultsHelper : NSObject
{
@public
  NSUserDefaultsController *controller;
  NSMutableDictionary *values;
}

- (id) initWithController: (NSUserDefaultsController*)udc;
- (id) valueForKey: (NSString*)key;
- (void) setValue: (id)value forKey: (NSString*)key;

- (void) revert;
- (void) revertToInitialValues: (NSDictionary *)initial;
- (void) defaultsDidChange: (NSUserDefaults *)defaults;
@end

@implementation GSUserDefaultsHelper

- (id) initWithController: (NSUserDefaultsController*)udc
{
  if ((self = [super init]) != nil)
    {
      // We are retained by the controller
      controller = udc;
      values = [[NSMutableDictionary alloc] init];
    }

  return self;
}

- (void) dealloc
{
  RELEASE(values);
  [super dealloc];
}

- (id) valueForKey: (NSString*)key
{
  id value = [values objectForKey: key];

  if (!value)
    {
      // If the value isn't cached, get it from the controller and cache it.
      value = [[controller defaults] objectForKey: key];
      if (!value)
        {
          value = [[controller initialValues] objectForKey: key];
        }
      // We need to cache the values we return to be able to 
      // report changes to them when the defaults change.
      if (value)
        {
          [values setObject: value forKey: key];
        }
      else
        {
          // Use a marker for missing values
          [values setObject: [NSNull null] forKey: key];
        }
    }
  else if ([[NSNull null] isEqual: value])
    {
      // filter out our marker value
      return nil;
    }

  return value;
}

- (void) setValue: (id)value forKey: (NSString*)key
{
  [self willChangeValueForKey: key];
  [values setObject: value forKey: key];
  if ([controller appliesImmediately])
    [[controller defaults] setObject: value forKey: key];
  [self didChangeValueForKey: key];
}

- (void) revert
{
  NSEnumerator *e;
  NSString *key;

  e = [values keyEnumerator];
  while ((key = (NSString *)[e nextObject]))
    {
      [self willChangeValueForKey: key];
      [values removeObjectForKey: key];
      [self didChangeValueForKey: key];
    }
}

- (void) revertToInitialValues: (NSDictionary *)initial
{
  NSEnumerator *e;
  NSString *key;

  e = [values keyEnumerator];
  while ((key = (NSString *)[e nextObject]))
    {
      id val = [values objectForKey: key];
      id oldVal = [initial objectForKey: key];
        
      if (oldVal && ![val isEqual: oldVal])
        {
          [self willChangeValueForKey: key];
          [values setObject: oldVal forKey: key];
          // When appliesImmediately is YES, should we save these values?
          [self didChangeValueForKey: key];
        }
   }
}

- (void) defaultsDidChange: (NSUserDefaults *)defaults
{
  NSEnumerator *e;
  NSString *key;

  e = [values keyEnumerator];
  while ((key = (NSString *)[e nextObject]))
    {
      id val = [values objectForKey: key];
      id newVal = [defaults objectForKey: key];

      if (newVal && ![val isEqual: newVal])
        {
          [self willChangeValueForKey: key];
          [values setObject: newVal forKey: key];
          [self didChangeValueForKey: key];
        }      
    }
}

@end

@implementation NSUserDefaultsController

+ (id) sharedUserDefaultsController
{
  if (shared == nil)
    {
      shared = [[NSUserDefaultsController alloc] 
                   initWithDefaults: nil
                   initialValues: nil];    
    }
  return shared;
}

- (id) initWithDefaults: (NSUserDefaults*)defaults
          initialValues: (NSDictionary*)initialValues
{
  if ((self = [super init]) != nil)
    {
      if (defaults == nil)
        {
          defaults = [NSUserDefaults standardUserDefaults];
        }

      ASSIGN(_defaults, defaults);
      [self setAppliesImmediately: YES];
      [self setInitialValues: initialValues];
      _values = [[GSUserDefaultsHelper alloc] 
                    initWithController: self];
      // Watch for user default change notifications
      [[NSNotificationCenter defaultCenter] 
          addObserver: self
          selector: @selector(defaultsDidChange:)
          name: NSUserDefaultsDidChangeNotification
          object: _defaults];
    }

  return self;
}

- (void) dealloc
{
  if (self == shared)
    {
      // Should never get here
      shared = nil;
    }

  [[NSNotificationCenter defaultCenter] removeObserver: self];
  RELEASE(_values);
  RELEASE(_defaults);
  RELEASE(_initial_values);
  [super dealloc];
}

- (NSUserDefaults*) defaults
{
  return _defaults;
}

- (id) values
{
  return _values;  
}

- (NSDictionary*) initialValues
{
  return _initial_values;
}

- (void) setInitialValues: (NSDictionary*)values
{
  ASSIGNCOPY(_initial_values, values);
}

- (BOOL) appliesImmediately
{
  return _applies_immediately;
}

- (void) setAppliesImmediately: (BOOL)flag
{
  _applies_immediately = flag; 
}

- (void) revert: (id)sender
{
  [self discardEditing];
  if (![self appliesImmediately])
    {
      [_values revert];
    } 
}

- (void) revertToInitialValues: (id)sender
{
  [self discardEditing];
  [_values revertToInitialValues: _initial_values];
}

- (void) save: (id)sender
{
  if (![self appliesImmediately])
    {
      NSDictionary *values = ((GSUserDefaultsHelper*)_values)->values;
      NSEnumerator *e;
      NSString *key;
      
      e = [values keyEnumerator];
      while ((key = (NSString *)[e nextObject]))
        {
          [_defaults setObject: [values objectForKey: key] forKey: key];
        }
    }
}

- (BOOL) hasUnappliedChanges
{
  NSDictionary *values = ((GSUserDefaultsHelper*)_values)->values;
  NSEnumerator *e;
  NSString *key;

  e = [values keyEnumerator];
  while ((key = (NSString *)[e nextObject]))
    {
      id val = [values objectForKey: key];
      id newVal = [_defaults objectForKey: key];

      if (![val isEqual: newVal])
        {
          return YES;
        }      
    }
  return NO;
}

- (void) defaultsDidChange: (NSNotification*)notification
{
  [_values defaultsDidChange: _defaults];
}

- (void) encodeWithCoder: (NSCoder *)aCoder
{ 
  if ([aCoder allowsKeyedCoding])
    if (self == shared)
      {
        [aCoder encodeBool: YES forKey: @"NSSharedInstance"];
        return;
      }

  [super encodeWithCoder: aCoder];
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    if ([aDecoder decodeBoolForKey: @"NSSharedInstance"])
      {
        RELEASE(self);
        return RETAIN([NSUserDefaultsController sharedUserDefaultsController]);
      }

  return [super initWithCoder: aDecoder];
}

@end
