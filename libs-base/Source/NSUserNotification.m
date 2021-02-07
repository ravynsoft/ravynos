/* Implementation for NSUserNotification for GNUstep
   Copyright (C) 2014 Free Software Foundation, Inc.

   Written by:  Marcus Mueller <znek@mulle-kybernetik.com>
   Date: 2014

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
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import	<GNUstepBase/GSVersionMacros.h>

#if __has_feature(objc_default_synthesize_properties)

#define	EXPOSE_NSUserNotification_IVARS	1
#define	EXPOSE_NSUserNotificationCenter_IVARS	1

#import "GNUstepBase/NSObject+GNUstepBase.h"
#import "Foundation/NSUserNotification.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSBundle.h"
#import "Foundation/NSCalendar.h"
#import "Foundation/NSDate.h"
#import "Foundation/NSString.h"
#import "Foundation/NSTimeZone.h"

NSString * const NSUserNotificationDefaultSoundName
  = @"NSUserNotificationDefaultSoundName";

@interface NSUserNotification ()
@property (readwrite) NSDate *actualDeliveryDate;
@property (readwrite, getter=isPresented) BOOL presented;
@property (readwrite, getter=isRemote) BOOL remote;
@property (readwrite) NSUserNotificationActivationType activationType;
@property (readwrite) NSAttributedString *response;
@end

@implementation NSUserNotification

- (id) init
{
  if (nil != (self = [super init]))
    {
      [self setHasActionButton: YES];
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_uniqueId);
  [super dealloc];
}

- (id) copyWithZone: (NSZone *)zone
{
  return NSCopyObject(self, 0, zone);
}

- (NSString *) description
{
  NSMutableString *d = [NSMutableString stringWithCapacity:80];

  [d appendFormat: @"<%s:%p< {", object_getClassName(self), self];
  [d appendFormat: @" title: \"%@\"", [self title]];
  [d appendFormat: @" informativeText: \"%@\"", [self informativeText]];
  [d appendFormat: @" actionButtonTitle: \"%@\"", [self actionButtonTitle]];
  if ([self actualDeliveryDate])
    {
      [d appendFormat: @" actualDeliveryDate: %@", [self actualDeliveryDate]];
      [d appendFormat: @" presented: %s", [self isPresented] ? "YES" : "NO"];
    }
  [d appendFormat: @" next delivery date: %@", [self deliveryDate]];
  [d appendString: @" }"];
  return d;
}

@end

@interface NSUserNotificationCenter ()
@property (readwrite) NSArray *deliveredNotifications;
@end

@interface NSUserNotificationCenter (Private)
+ (Class) defaultUserNotificationCenterClass;
+ (void) setDefaultUserNotificationCenter: (NSUserNotificationCenter *)unc;
- (NSUserNotification *) deliveredNotificationWithUniqueId: (id)uniqueId;
- (NSDate *) nextDeliveryDateForNotification: (NSUserNotification *)un;
@end

@implementation NSUserNotificationCenter

static NSUserNotificationCenter *defaultUserNotificationCenter = nil;

+ (Class) defaultUserNotificationCenterClass
{
  NSBundle *bundle = [NSBundle bundleForClass: [self class]];
  NSString *bundlePath = [bundle pathForResource: @"NSUserNotification"
                                          ofType: @"bundle"
                                     inDirectory: nil];
  if (bundlePath)
    {
      bundle = [NSBundle bundleWithPath: bundlePath];
      if (bundle)
        {
          return [bundle principalClass];
        }
    }
  return self;
}

+ (void) atExit
{
  DESTROY(defaultUserNotificationCenter);
}

+ (void) initialize
{
  if ([NSUserNotificationCenter class] == self)
    {
      Class uncClass = [self defaultUserNotificationCenterClass];
      defaultUserNotificationCenter = [[uncClass alloc] init];
      [self registerAtExit];
    }
}

+ (void) setDefaultUserNotificationCenter: (NSUserNotificationCenter *)unc
{
  ASSIGN(defaultUserNotificationCenter, unc);
}

+ (NSUserNotificationCenter *) defaultUserNotificationCenter
{
  return defaultUserNotificationCenter;
}


- (id) init
{
  self = [super init];
  if (self)
    {
      _scheduledNotifications = [[NSMutableArray alloc] init];
      _deliveredNotifications = [[NSMutableArray alloc] init];
    }
  return self;
}

- (void) dealloc
{
  [NSObject cancelPreviousPerformRequestsWithTarget: self];
  RELEASE(_scheduledNotifications);
  [self removeAllDeliveredNotifications];
  RELEASE(_deliveredNotifications);
  [super dealloc];
}

- (void) scheduleNotification: (NSUserNotification *)un
{
  if (![un deliveryDate])
    {
      [self deliverNotification: un];
      return;
    }
  [_scheduledNotifications addObject: un];
  NSTimeInterval delay = [[un deliveryDate] timeIntervalSinceNow];
  [self performSelector: @selector(deliverNotification:)
             withObject: un
             afterDelay: delay];
}

- (void) removeScheduledNotification: (NSUserNotification *)un
{
  [NSObject cancelPreviousPerformRequestsWithTarget: self
    selector: @selector(deliverNotification:)
    object: un];
  [_scheduledNotifications removeObject: un];
}

- (void) _deliverNotification: (NSUserNotification *)un
{
  [un setPresented: YES];
  NSLog(@"NOTE: %@", un);
}

- (NSDate *) nextDeliveryDateForNotification: (NSUserNotification *)un
{
  NSDateComponents *repeatInterval = [un deliveryRepeatInterval];
  if (!repeatInterval)
    return nil;

  NSCalendar *cal = [[repeatInterval calendar] copy];
  if (!cal)
    cal = [[NSCalendar currentCalendar] copy];
  if ([repeatInterval timeZone])
    [cal setTimeZone:[repeatInterval timeZone]];
  if (![cal timeZone])
    [cal setTimeZone:[NSTimeZone localTimeZone]];

  NSDate *nextDeliveryDate
    = [cal dateByAddingComponents: repeatInterval
                           toDate: [un actualDeliveryDate]
                          options: 0];
  RELEASE(cal);
  return nextDeliveryDate;
}

- (void) deliverNotification: (NSUserNotification *)un
{
  [self removeScheduledNotification: un];
  [self _deliverNotification: un];

  NSDate *actualDeliveryDate = [un deliveryDate];
  if (!actualDeliveryDate)
    actualDeliveryDate = [NSDate date];
  [un setActualDeliveryDate: actualDeliveryDate];
  [_deliveredNotifications addObject: un];
  [un setDeliveryDate: [self nextDeliveryDateForNotification: un]];
  if ([un deliveryDate])
    [self scheduleNotification: un];

  if ([self delegate] && [[self delegate] respondsToSelector:
    @selector(userNotificationCenter:didDeliverNotification:)])
    {
      [[self delegate] userNotificationCenter: self didDeliverNotification: un];
    }
}


- (void) _removeDeliveredNotification: (NSUserNotification *)un
{
}

- (void) removeDeliveredNotification: (NSUserNotification *)un
{
  [self _removeDeliveredNotification: un];
  [_deliveredNotifications removeObject: un];
}

- (void) removeAllDeliveredNotifications
{
  NSUInteger i, count = [_deliveredNotifications count];

  for (i = 0; i < count; i++)
    {
      NSUserNotification *un = [_deliveredNotifications objectAtIndex: i];
      [self _removeDeliveredNotification:un];
    }
  [_deliveredNotifications removeAllObjects];
}

- (NSUserNotification *) deliveredNotificationWithUniqueId: (id)uniqueId
{
  NSUInteger i, count = [_deliveredNotifications count];

  for (i = 0; i < count; i++)
    {
      NSUserNotification *un = [_deliveredNotifications objectAtIndex: i];
      if ([un->_uniqueId isEqual: uniqueId])
        {
          return un;
        }
    }
  return nil;
}

@end

#endif /* __has_feature(objc_default_synthesize_properties) */
