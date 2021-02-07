/** <title>NSEvent</title>

   <abstract>The event class</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: 1996
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: Sept 1998
   Updated: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: June 1999

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

#include "config.h"
#import <Foundation/NSDictionary.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSException.h>
#import <Foundation/NSDebug.h>

#import "AppKit/NSEvent.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/PSOperators.h"

#import "GNUstepGUI/GSDisplayServer.h"

/*
 *        gstep-base has a faster mechanism to get the current thread.
 */
#ifndef GNUSTEP_BASE_LIBRARY
#define GSCurrentThread()           [NSThread currentThread]
#define GSCurrentThreadDictionary() [[NSThread currentThread] threadDictionary]
#endif

@implementation NSEvent

/*
 * Class variables
 */
static NSString *timerKey = @"NSEventTimersKey";
static Class dateClass;
static Class eventClass;

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSEvent class])
    {
      [self setVersion: 3];
      dateClass = [NSDate class];
      eventClass = [NSEvent class];
    }
}

/*
 * Creating NSEvent objects
 */
+ (NSEvent*) enterExitEventWithType: (NSEventType)type
                           location: (NSPoint)location
                      modifierFlags: (NSUInteger)flags
                          timestamp: (NSTimeInterval)time
                       windowNumber: (NSInteger)windowNum
                            context: (NSGraphicsContext*)context
                        eventNumber: (NSInteger)eventNum
                     trackingNumber: (NSInteger)trackingNum
                           userData: (void *)userData
{
  NSEvent *e;

  if (type == NSCursorUpdate)
    RETAIN((id)userData);
  else if ((type != NSMouseEntered) && (type != NSMouseExited))
    [NSException raise: NSInvalidArgumentException
                format: @"enterExitEvent with wrong type"];

  e = (NSEvent*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  if (self != eventClass)
    e = [e init];
  AUTORELEASE(e);

  e->event_type = type;
  e->location_point = location;
  e->modifier_flags = flags;
  e->event_time = time;
  e->window_num = windowNum;
  e->event_context = context;
  e->event_data.tracking.event_num = eventNum;
  e->event_data.tracking.tracking_num = trackingNum;
  e->event_data.tracking.user_data = userData;

  return e;
}

+ (NSEvent*) keyEventWithType: (NSEventType)type
                     location: (NSPoint)location
                modifierFlags: (NSUInteger)flags
                    timestamp: (NSTimeInterval)time
                 windowNumber: (NSInteger)windowNum
                      context: (NSGraphicsContext *)context
                   characters: (NSString *)keys
  charactersIgnoringModifiers: (NSString *)ukeys
                    isARepeat: (BOOL)repeatKey
                      keyCode: (unsigned short)code
{
  NSEvent *e;

  if (!(NSEventMaskFromType(type) & GSKeyEventMask))
    [NSException raise: NSInvalidArgumentException
                format: @"keyEvent with wrong type"];

  e = (NSEvent*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  if (self != eventClass)
    e = [e init];
  AUTORELEASE(e);

  e->event_type = type;
  e->location_point = location;
  e->modifier_flags = flags;
  e->event_time = time;
  e->window_num = windowNum;
  e->event_context = context;
  RETAIN(keys);
  e->event_data.key.char_keys = keys;
  RETAIN(ukeys);
  e->event_data.key.unmodified_keys = ukeys;
  e->event_data.key.repeat = repeatKey;
  e->event_data.key.key_code = code;

  return e;
}

+ (NSEvent*) mouseEventWithType: (NSEventType)type
                       location: (NSPoint)location
                  modifierFlags: (NSUInteger)flags
                      timestamp: (NSTimeInterval)time
                   windowNumber: (NSInteger)windowNum
                        context: (NSGraphicsContext*)context
                    eventNumber: (NSInteger)eventNum
                     clickCount: (NSInteger)clickNum
                       pressure: (float)pressureValue
{
  NSEvent *e;

  if (!(NSEventMaskFromType(type) & GSMouseEventMask))
    [NSException raise: NSInvalidArgumentException
                format: @"mouseEvent with wrong type"];

  e = (NSEvent*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  if (self != eventClass)
    e = [e init];
  AUTORELEASE(e);

  e->event_type = type;
  e->location_point = location;
  e->modifier_flags = flags;
  e->event_time = time;
  e->window_num = windowNum;
  e->event_context = context;
  e->event_data.mouse.event_num = eventNum;
  e->event_data.mouse.click = clickNum;
  e->event_data.mouse.pressure = pressureValue;

  return e;
}

+ (NSEvent*) mouseEventWithType: (NSEventType)type        
                       location: (NSPoint)location
                  modifierFlags: (NSUInteger)flags
                      timestamp: (NSTimeInterval)time
                   windowNumber: (NSInteger)windowNum
                        context: (NSGraphicsContext*)context        
                    eventNumber: (NSInteger)eventNum
                     clickCount: (NSInteger)clickNum
                       pressure: (float)pressureValue
                   buttonNumber: (NSInteger)buttonNum
                         deltaX: (CGFloat)deltaX
                         deltaY: (CGFloat)deltaY
                         deltaZ: (CGFloat)deltaZ
{
  NSEvent *e;

  if (!(NSEventMaskFromType(type) & GSMouseEventMask))
    [NSException raise: NSInvalidArgumentException
                format: @"mouseEvent with wrong type"];

  e = (NSEvent*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  if (self != eventClass)
    e = [e init];
  AUTORELEASE(e);

  e->event_type = type;
  e->location_point = location;
  e->modifier_flags = flags;
  e->event_time = time;
  e->window_num = windowNum;
  e->event_context = context;
  e->event_data.mouse.event_num = eventNum;
  e->event_data.mouse.click = clickNum;
  e->event_data.mouse.button = buttonNum;
  e->event_data.mouse.pressure = pressureValue;
  e->event_data.mouse.deltaX = deltaX;
  e->event_data.mouse.deltaY = deltaY;
  e->event_data.mouse.deltaZ = deltaZ;

  return e;
}

/**
 * Returns the current mouse location.
 */
+ (NSPoint) mouseLocation
{
  return [GSCurrentServer() mouselocation];
}

+ (NSEvent*) otherEventWithType: (NSEventType)type
                       location: (NSPoint)location
                  modifierFlags: (NSUInteger)flags
                      timestamp: (NSTimeInterval)time
                   windowNumber: (NSInteger)windowNum
                        context: (NSGraphicsContext*)context
                        subtype: (short)subType
                          data1: (NSInteger)data1
                          data2: (NSInteger)data2
{
  NSEvent *e;

  if (!(NSEventMaskFromType(type) & GSOtherEventMask))
    [NSException raise: NSInvalidArgumentException
                format: @"otherEvent with wrong type"];

  e = (NSEvent*)NSAllocateObject(self, 0, NSDefaultMallocZone());
  if (self != eventClass)
    e = [e init];
  AUTORELEASE(e);

  e->event_type = type;
  e->location_point = location;
  e->modifier_flags = flags;
  e->event_time = time;
  e->window_num = windowNum;
  e->event_context = context;
  e->event_data.misc.sub_type = subType;
  e->event_data.misc.data1 = data1;
  e->event_data.misc.data2 = data2;

  return e;
}

/*
 * Requesting Periodic Events
 */
+ (void) startPeriodicEventsAfterDelay: (NSTimeInterval)delaySeconds
                            withPeriod: (NSTimeInterval)periodSeconds
{
  NSTimer             *timer;
  NSMutableDictionary *dict = GSCurrentThreadDictionary();

  NSDebugLLog (@"NSEvent", @"startPeriodicEventsAfterDelay: withPeriod: ");

  if ([dict objectForKey: timerKey])
    [NSException raise: NSInternalInconsistencyException
                format: @"Periodic events are already being generated for "
                        @"this thread %p", GSCurrentThread()];

  /*
   *  Register a timer that will fire in delaySeconds.
   *  This timer will fire the first event and register
   *  a repeat timer that will send the following events
   */
  timer = [NSTimer timerWithTimeInterval: delaySeconds
                   target: self
                   selector: @selector(_registerRealTimer:)
                   userInfo: [NSNumber numberWithDouble: periodSeconds]
                   repeats: NO];

  [[NSRunLoop currentRunLoop] addTimer: timer
                               forMode: NSEventTrackingRunLoopMode];
  [dict setObject: timer forKey: timerKey];
}

+ (void) _timerFired: (NSTimer*)timer
{
  NSTimeInterval timeInterval;
  NSEvent        *periodicEvent;

  timeInterval = [dateClass timeIntervalSinceReferenceDate];
  periodicEvent = [self otherEventWithType: NSPeriodic
                                  location: NSZeroPoint
                             modifierFlags: 0
                                 timestamp: timeInterval
                              windowNumber: 0
                                   context: [NSApp context]
                                   subtype: 0
                                     data1: 0
                                     data2: 0];

  NSDebugLLog (@"NSEvent", @"_timerFired: ");
  [NSApp postEvent: periodicEvent atStart: NO];
}

/*
 * This method provides a means of delaying the start of periodic events
 */
+ (void) _registerRealTimer: (NSTimer*)timer
{
  NSTimer             *realTimer;
  NSMutableDictionary *dict = GSCurrentThreadDictionary();

  NSDebugLLog (@"NSEvent", @"_registerRealTimer: ");
  {
    NSTimeInterval timeInterval;
    NSEvent        *periodicEvent;
    
    timeInterval = [dateClass timeIntervalSinceReferenceDate];
    periodicEvent = [self otherEventWithType: NSPeriodic
                          location: NSZeroPoint
                          modifierFlags: 0
                          timestamp: timeInterval
                              windowNumber: 0
                          context: [NSApp context]
                          subtype: 0
                          data1: 0
                          data2: 0];
    
    [NSApp postEvent: periodicEvent atStart: NO];
  }

  realTimer = [NSTimer timerWithTimeInterval: [[timer userInfo] doubleValue]
                                      target: self
                                    selector: @selector(_timerFired:)
                                    userInfo: nil
                                     repeats: YES];
  [dict setObject: realTimer forKey: timerKey];
  [[NSRunLoop currentRunLoop] addTimer: realTimer
                               forMode: NSEventTrackingRunLoopMode];
}

+ (void) stopPeriodicEvents
{
  NSTimer             *timer;
  NSMutableDictionary *dict = GSCurrentThreadDictionary();

  NSDebugLLog (@"NSEvent", @"stopPeriodicEvents");
  timer = [dict objectForKey: timerKey];
  [timer invalidate];
  [dict removeObjectForKey: timerKey];
}

/**
 * Returns the button number for the mouse button pressed in a mouse
 * event.  Intended primarily for the case where a mouse has three or
 * more buttons, and you want to know which button an 'other' mouse
 * event refers to.
 */
- (NSInteger) buttonNumber
{
  if (!(NSEventMaskFromType(event_type) & GSMouseEventMask))
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"buttonNumber requested for non-mouse event"];
    }
  return event_data.mouse.button;
}

/**
 * Returns the string of characters for a keyboard event.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event.
 */
- (NSString *) characters
{
  if ((event_type != NSKeyUp) && (event_type != NSKeyDown))
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"characters requested for non-keyboard event"];
    }
  return event_data.key.char_keys;
}

/**
 * Returns the string of characters for a keyboard event, as if no modifier
 * keys had been pressed when the keyboard event occirred.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event.
 */
- (NSString *) charactersIgnoringModifiers
{
  if ((event_type != NSKeyUp) && (event_type != NSKeyDown))
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"charactersIgnoringModifiers requested for "
                          @"non-keyboard event"];
    }
  return event_data.key.unmodified_keys;
}

/**
 * Return the number of clicks associated with the mouse down or up
 * event.  This method is not applicable for any event type other
 * than a mouse down or mouse up.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event.
 */
- (NSInteger) clickCount
{
  /* Make sure it is one of the right event types */
  if (!(NSEventMaskFromType(event_type) & GSMouseEventMask))
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"clickCount requested for non-mouse event"];
    }
  return event_data.mouse.click;
}

/**
 * Returns the graphics context for which this event was generated.
 */
- (NSGraphicsContext*) context
{
  return event_context;
}

- (id) copyWithZone: (NSZone*)zone
{
  NSEvent *e = (NSEvent*)NSCopyObject (self, 0, zone);

  if ((NSEventMaskFromType(event_type) & GSKeyEventMask))
    {
      event_data.key.char_keys = [event_data.key.char_keys copyWithZone: zone];
      event_data.key.unmodified_keys
        = [event_data.key.unmodified_keys copyWithZone: zone];
    }
  else if (event_type == NSCursorUpdate)
    {
      event_data.tracking.user_data
        = (void *)[(id)event_data.tracking.user_data copyWithZone: zone];
    }
  return e;
}

/**
 * Returns the 'data1' item associated with the event.
 * <br />Raises NSInternalInconsistencyException if the event is not
 * of type NSAppKitDefined, NSSystemDefined, NSApplicationDefined,
 * or NSPeriodic
 */
- (NSInteger) data1
{
  if (event_type < NSAppKitDefined || event_type > NSPeriodic)
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"data1 requested for invalid event type"];
    }
  return event_data.misc.data1;
}

/**
 * Returns the 'data2' item associated with the event.
 * <br />Raises NSInternalInconsistencyException if the event is not
 * of type NSAppKitDefined, NSSystemDefined, NSApplicationDefined,
 * or NSPeriodic
 */
- (NSInteger) data2
{
  if (event_type < NSAppKitDefined || event_type > NSPeriodic)
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"data2 requested for invalid event type"];
    }
  return event_data.misc.data2;
}

- (void) dealloc
{
  if ((NSEventMaskFromType(event_type) & GSKeyEventMask))
    {
      RELEASE(event_data.key.char_keys);
      RELEASE(event_data.key.unmodified_keys);
    }
  else if (event_type == NSCursorUpdate)
    {
      RELEASE((id)event_data.tracking.user_data);
    }
  [super dealloc];
}

/**
   <p>
   Returns the movement of the mouse on the X axis.
   </p>
   <p>
   This method is only valid for NSMouseMoved, NS*MouseDragged and 
   NSScrollWheel events, otherwise it will return 0.
   </p>
 */
- (CGFloat) deltaX
{
  if (!(NSEventMaskFromType(event_type) & GSMouseMovedEventMask))
    {
      return 0.0;
    }
  return event_data.mouse.deltaX;
}

/**
   <p>
   Returns the movement of the mouse on the Y axis.
   </p>
   <p>
   This method is only valid for NSMouseMoved, NS*MouseDragged and 
   NSScrollWheel events, otherwise it will return 0.
   </p>
 */
- (CGFloat) deltaY
{
  if (!(NSEventMaskFromType(event_type) & GSMouseMovedEventMask))
    {
      return 0.0;
    }
  return event_data.mouse.deltaY;
}

/**
   <p>
   Returns the movement of the mouse on the Z axis.
   </p>
   <p>
   This method is only valid for NSMouseMoved, NS*MouseDragged and 
   NSScrollWheel events, otherwise it will return 0.
   </p>
   <p>
   The value returned is 0.0 in most cases.
   </p>
 */
- (CGFloat) deltaZ
{
  if (!(NSEventMaskFromType(event_type) & GSMouseMovedEventMask))
    {
      return 0.0;
    }
  return event_data.mouse.deltaZ;
}

- (NSString*) description
{
static const char *eventTypes[] = {
    "nullEvent",
    "leftMouseDown",
    "leftMouseUp",
    "rightMouseDown",
    "rightMouseUp",
    "mouseMoved",
    "leftMouseDragged",
    "rightMouseDragged",
    "mouseEntered",
    "mouseExited",
    "keyDown",
    "keyUp",
    "flagsChanged",
    "appKitDefined",
    "systemDefined",
    "applicationDefined",
    "periodic",
    "cursorUpdate",
    "", "", "", "",
    "scrollWheel",
    "tabletPoint",
    "tabletProximity",
    "otherMouseDown",
    "otherMouseUp",
    "otherMouseDragged"
  };

  switch (event_type)
    {
      case NSLeftMouseDown:
      case NSLeftMouseUp:
      case NSOtherMouseDown:
      case NSOtherMouseUp:
      case NSRightMouseDown:
      case NSRightMouseUp:
        return [NSString stringWithFormat:
          @"NSEvent: eventType = %s, point = { %f, %f }, modifiers = %lu,"
          @" time = %f, window = %ld, dpsContext = %p,"
          @" event number = %ld, click = %ld, pressure = %f",
          eventTypes[event_type], location_point.x, location_point.y,
          (unsigned long) modifier_flags, event_time, (long) window_num,
	  event_context, (long int) event_data.mouse.event_num,
	  (long int) event_data.mouse.click,
          event_data.mouse.pressure];
        break;

      case NSMouseEntered:
      case NSMouseExited:
        return [NSString stringWithFormat:
          @"NSEvent: eventType = %s, point = { %f, %f }, modifiers = %lu,"
          @" time = %f, window = %ld, dpsContext = %p, "
          @" event number = %ld, tracking number = %ld, user data = %p",
          eventTypes[event_type], location_point.x, location_point.y,
          (unsigned long) modifier_flags, event_time, (long) window_num, event_context,
          (long) event_data.tracking.event_num,
          (long) event_data.tracking.tracking_num,
          event_data.tracking.user_data];
        break;

      case NSKeyDown:
      case NSKeyUp:
      case NSFlagsChanged:
        return [NSString stringWithFormat:
          @"NSEvent: eventType = %s, point = { %f, %f }, modifiers = %lu,"
          @" time = %f, window = %ld, dpsContext = %p, "
          @" repeat = %s, keys = %@, ukeys = %@, keyCode = 0x%x",
          eventTypes[event_type], location_point.x, location_point.y,
          (unsigned long) modifier_flags, event_time, (long) window_num, event_context,
          (event_data.key.repeat ? "YES" : "NO"),
          event_data.key.char_keys, event_data.key.unmodified_keys,
          event_data.key.key_code];
        break;

      case NSPeriodic:
      case NSCursorUpdate:
      case NSAppKitDefined:
      case NSSystemDefined:
      case NSApplicationDefined:
        return [NSString stringWithFormat:
          @"NSEvent: eventType = %s, point = { %f, %f }, modifiers = %lu,"
          @" time = %f, window = %ld, dpsContext = %p, "
          @" subtype = %d, data1 = %lx, data2 = %lx",
          eventTypes[event_type], location_point.x, location_point.y,
          (unsigned long) modifier_flags, event_time, (long) window_num, event_context,
          event_data.misc.sub_type, (long) event_data.misc.data1,
          (long) event_data.misc.data2];
        break;

      case NSScrollWheel:
      case NSMouseMoved:
      case NSLeftMouseDragged:
      case NSOtherMouseDragged:
      case NSRightMouseDragged:
        return [NSString stringWithFormat:
          @"NSEvent: eventType = %s, point = { %f, %f }, modifiers = %lu,"
          @" time = %f, window = %ld, dpsContext = %p,"
          @" event number = %ld, click = %ld, pressure = %f"
          @" button = %ld, deltaX = %f, deltaY = %f, deltaZ = %f",
          eventTypes[event_type], location_point.x, location_point.y,
          (unsigned long) modifier_flags, event_time, (long) window_num, event_context,
          (long ) event_data.mouse.event_num, (long) event_data.mouse.click,
          event_data.mouse.pressure, (long) event_data.mouse.button,
          event_data.mouse.deltaX,
          event_data.mouse.deltaY,
          event_data.mouse.deltaZ];
        break;

      // FIXME: Tablet events
      case NSTabletPoint:
      case NSTabletProximity:
        break;

      default:
        return [NSString stringWithFormat:
          @"NSEvent: eventType = UNKNOWN!, point = { %f, %f }, modifiers = %lu,"
          @" time = %f, window = %ld",
          location_point.x, location_point.y,
          (unsigned long) modifier_flags, event_time, (long) window_num];
        break;
    }

  return [super description];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &event_type];
      [aCoder encodePoint: location_point];
      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &modifier_flags];
      [aCoder encodeValueOfObjCType: @encode(NSTimeInterval) at: &event_time];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &window_num];
      
      switch (event_type)
        {
        case NSLeftMouseDown:
        case NSLeftMouseUp:
        case NSOtherMouseDown:
        case NSOtherMouseUp:
        case NSRightMouseDown:
        case NSRightMouseUp:
        case NSScrollWheel:
        case NSMouseMoved:
        case NSLeftMouseDragged:
        case NSOtherMouseDragged:
        case NSRightMouseDragged:
          [aCoder encodeValuesOfObjCTypes: "iififff", &event_data.mouse.event_num,
                  &event_data.mouse.click, &event_data.mouse.pressure,
                  &event_data.mouse.button, &event_data.mouse.deltaX,
                  &event_data.mouse.deltaY, &event_data.mouse.deltaZ];
          break;

        case NSMouseEntered:
        case NSMouseExited:
        case NSCursorUpdate:
          // Can't do anything with the user_data!?
          [aCoder encodeValuesOfObjCTypes: "ii", &event_data.tracking.event_num,
                  &event_data.tracking.tracking_num];
          break;
          
        case NSKeyDown:
        case NSKeyUp:
        case NSFlagsChanged:
          [aCoder encodeValueOfObjCType: @encode(BOOL)
                                     at: &event_data.key.repeat];
          [aCoder encodeObject: event_data.key.char_keys];
          [aCoder encodeObject: event_data.key.unmodified_keys];
          [aCoder encodeValueOfObjCType: "S" at: &event_data.key.key_code];
          break;
          
        case NSPeriodic:
        case NSAppKitDefined:
        case NSSystemDefined:
        case NSApplicationDefined:
          [aCoder encodeValuesOfObjCTypes: "sii", &event_data.misc.sub_type,
                  &event_data.misc.data1, &event_data.misc.data2];
          break;
          
        case NSTabletPoint:
        case NSTabletProximity:
          // FIXME: Tablet events
          break;
        }
    }
}

/**
 * Returns the event number associated with any mouse event or tracking
 * event.  Event numbers are allocated sequentially when the system
 * creates these events.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event.
 */
- (NSInteger) eventNumber
{
  /* Make sure it is one of the right event types */
  if (!(NSEventMaskFromType(event_type) & GSMouseEventMask) && 
      !(NSEventMaskFromType(event_type) & GSEnterExitEventMask))
    [NSException raise: NSInternalInconsistencyException
                format: @"eventNumber requested for non-tracking event"];

  if ((event_type == NSMouseEntered) || (event_type == NSMouseExited))
    return event_data.tracking.event_num;
  else
    return event_data.mouse.event_num;
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      int version = [aDecoder versionForClassName: @"NSEvent"];
      
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &event_type];
      location_point = [aDecoder decodePoint];
      [aDecoder decodeValueOfObjCType: @encode(NSUInteger) at: &modifier_flags];
      [aDecoder decodeValueOfObjCType: @encode(NSTimeInterval) at: &event_time];
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &window_num];
      
      if (version == 1)
        {
          // For the unlikely case that old events have been stored, convert them.
          switch ((int)event_type)
            {
            case 0: event_type = NSLeftMouseDown; break;
            case 1: event_type = NSLeftMouseUp; break;
            case 2: event_type = NSOtherMouseDown; break;
            case 3: event_type = NSOtherMouseUp; break;
            case 4: event_type = NSRightMouseDown; break;
            case 5: event_type = NSRightMouseUp; break;
            case 6: event_type = NSMouseMoved; break;
            case 7: event_type = NSLeftMouseDragged; break;
            case 8: event_type = NSOtherMouseDragged; break;
            case 9: event_type = NSRightMouseDragged; break;
            case 10: event_type = NSMouseEntered; break;
            case 11: event_type = NSMouseExited; break;
            case 12: event_type = NSKeyDown; break;
            case 13: event_type = NSKeyUp; break;
            case 14: event_type = NSFlagsChanged; break;
            case 15: event_type = NSAppKitDefined; break;
            case 16: event_type = NSSystemDefined; break;
            case 17: event_type = NSApplicationDefined; break;
            case 18: event_type = NSPeriodic; break;
            case 19: event_type = NSCursorUpdate; break;
            case 20: event_type = NSScrollWheel; break;
            default: break;
            }  
    }

      // Previously flag change events where encoded wrongly
      if ((version == 2) && (event_type == NSFlagsChanged))
        {
          [aDecoder decodeValuesOfObjCTypes: "sii", &event_data.misc.sub_type,
                    &event_data.misc.data1, &event_data.misc.data2];
          return self;
        }
      
      // Decode the event date based upon the event type
      switch (event_type)
        {
        case NSLeftMouseDown:
        case NSLeftMouseUp:
        case NSOtherMouseDown:
        case NSOtherMouseUp:
        case NSRightMouseDown:
        case NSRightMouseUp:
        case NSScrollWheel:
        case NSMouseMoved:
        case NSLeftMouseDragged:
        case NSOtherMouseDragged:
        case NSRightMouseDragged:
          [aDecoder decodeValuesOfObjCTypes: "iififff",
                    &event_data.mouse.event_num, &event_data.mouse.click,
                    &event_data.mouse.pressure, &event_data.mouse.button,
                    &event_data.mouse.deltaX, &event_data.mouse.deltaY,
                    &event_data.mouse.deltaZ];
          break;
          
        case NSMouseEntered:
        case NSMouseExited:
        case NSCursorUpdate:
          // Can't do anything with the user_data!?
          [aDecoder decodeValuesOfObjCTypes: "ii", &event_data.tracking.event_num,
                    &event_data.tracking.tracking_num];
          break;
          
        case NSKeyDown:
        case NSKeyUp:
        case NSFlagsChanged:
          [aDecoder decodeValueOfObjCType: @encode(BOOL)
                                       at: &event_data.key.repeat];
          event_data.key.char_keys = [aDecoder decodeObject];
          event_data.key.unmodified_keys = [aDecoder decodeObject];
          [aDecoder decodeValueOfObjCType: "S" at: &event_data.key.key_code];
          break;
          
        case NSPeriodic:
        case NSAppKitDefined:
        case NSSystemDefined:
        case NSApplicationDefined:
          [aDecoder decodeValuesOfObjCTypes: "sii", &event_data.misc.sub_type,
                    &event_data.misc.data1, &event_data.misc.data2];
          break;

        case NSTabletPoint:
        case NSTabletProximity:
          // FIXME: Tablet events
          break;
        }
    }

  return self;
}

/**
 * Returns a flag to say if this is a keyboard repeat event.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event than an NSKeyUp or NSKeyDown.
 */
- (BOOL) isARepeat
{
  if ((event_type != NSKeyUp) && (event_type != NSKeyDown))
    [NSException raise: NSInternalInconsistencyException
                format: @"isARepeat requested for non-keyboard event"];

  return event_data.key.repeat;
}

/**
 * Returns the numeric key code of a keyboard event.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event than an NSKeyUp or NSKeyDown.
 */
- (unsigned short) keyCode
{
  if (!(NSEventMaskFromType(event_type) & GSKeyEventMask))
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"keyCode requested for non-keyboard event"];
    }
  return event_data.key.key_code;
}

/**
 * Returns the window location for which this event was generated (in the
 * base coordinate system of the window).
 */
- (NSPoint) locationInWindow
{
  return location_point;
}

/**
 * Returns the modifier flag bits associated with the event.
 */
- (NSEventModifierFlags) modifierFlags
{
  return modifier_flags;
}

/**
 * Returns the pressure associated with a mouse event.  This is a value
 * in the range 0.0 to 1.0 and for mormal mouse events should be set to
 * one of those extremes.  This is used by pressure sensitive input devices.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event than a mouse event.
 */
- (float) pressure
{
  /* Make sure it is one of the right event types */
  if (!(NSEventMaskFromType(event_type) & GSMouseEventMask))
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"pressure requested for non-mouse event"];
    }
  return event_data.mouse.pressure;
}

/**
 * Returns the 'subtype' item associated with the event.
 * <br />Raises NSInternalInconsistencyException if the event is not
 * of type NSAppKitDefined, NSSystemDefined, NSApplicationDefined,
 * NSPeriodic or a mouve event.
 */
- (short) subtype
{
  if (!(NSEventMaskFromType(event_type) & GSOtherEventMask) && 
      !(NSEventMaskFromType(event_type) & GSMouseEventMask))
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"subtype requested for invalid event type"];
    }

  if (NSEventMaskFromType(event_type) & GSOtherEventMask)
    return event_data.misc.sub_type;
  else
    {
      if (event_type == NSTabletPoint)
        return NSTabletPointEventSubtype;
      else if (event_type == NSTabletProximity)
        return NSTabletProximityEventSubtype;
      else
        return NSMouseEventSubtype;
    }
}

/**
 * Returns the time interval since system startup at which this
 * event was generated.
 */
- (NSTimeInterval) timestamp
{
  return event_time;
}

/**
 * Returns a number identifying the tracking rectangle entered or exited.
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event than a mouse entered or exited event.
 */
- (NSInteger) trackingNumber
{
  if (event_type != NSMouseEntered && event_type != NSMouseExited
    &&  event_type != NSCursorUpdate)
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"trackingNumber requested for non-tracking event"];
    }
  return event_data.tracking.tracking_num;
}

/**
 * returns the type of this event.
 */
- (NSEventType) type
{
  return event_type;
}

/**
 * Returns usder data associated with a tracking event... the data assigned to
 * the tracking rectangle concerned when it was created..
 * <br />Raises an NSInternalInconsistencyException if applied to any
 * other type of event than a mouse entered or exited event.
 */
- (void *) userData
{
  if (event_type != NSMouseEntered && event_type != NSMouseExited
    &&  event_type != NSCursorUpdate)
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"userData requested for non-tracking event"];
    }
  return event_data.tracking.user_data;
}

/**
 * Returns the window for which this event was generated.<br />
 * Periodic events have no associated window, and you should not call
 * this method for those events.
 */
- (NSWindow *) window
{
  return GSWindowWithNumber(window_num);
}

/**
 * Returns the window number of the window for which this event was generated.
 * <br />Periodic events have no associated window, and you should not call
 * this method for those events.
 */
- (NSInteger) windowNumber
{
  return window_num;
}

- (NSEventMask) associatedEventsMask
{
  // FIXME
  return 0;
}

/*
 * Methods for tablet events
 */
- (NSInteger) absoluteX
{
  // FIXME
  return 0;
}

- (NSInteger) absoluteY
{
  // FIXME
  return 0;
}

- (NSInteger) absoluteZ
{
  // FIXME
  return 0;
}

- (NSEventButtonMask) buttonMask
{
  // FIXME
  return 0;
}

- (NSUInteger) capabilityMask
{
  // FIXME
  return 0;
}

- (NSUInteger) deviceID
{
  // FIXME
  return 0;
}

- (BOOL) isEnteringProximity
{
  // FIXME
  return NO;
}

- (NSUInteger) pointingDeviceID
{
  // FIXME
  return 0;
}

- (NSUInteger) pointingDeviceSerialNumber
{
  // FIXME
  return 0;
}

- (NSPointingDeviceType) pointingDeviceType
{
  // FIXME
  return NSUnknownPointingDevice;
}

- (float) rotation
{
  // FIXME
  return 0.0;
}

- (NSUInteger) systemTabletID
{
  // FIXME
  return 0;
}

- (NSUInteger) tabletID
{
  // FIXME
  return 0;
}

- (float) tangentialPressure
{
  // FIXME
  return 0.0;
}

- (NSPoint) tilt
{
  // FIXME
  return NSMakePoint(0.0, 0.0);
}

- (unsigned long long) uniqueID
{
  // FIXME
  return 0;
}

- (id) vendorDefined
{
  // FIXME
  return nil;
}

- (NSUInteger) vendorID
{
  // FIXME
  return 0;
}

- (NSUInteger) vendorPointingDeviceType
{
  // FIXME
  return 0;
}

+ (NSEvent*) eventWithEventRef: (const void *)eventRef
{
  // FIXME
  return nil;
}

- (const void *) eventRef
{
  // FIXME
  return NULL;
}

+ (void) setMouseCoalescingEnabled: (BOOL)flag
{
  // FIXME
}

+ (BOOL) isMouseCoalescingEnabled
{
  // FIXME
  return YES;
}

+ (NSEventModifierFlags) modifierFlags
{
  // FIXME
  return 0;
}

+ (NSTimeInterval) keyRepeatDelay
{
  // FIXME
  return 0;
}

+ (NSTimeInterval) keyRepeatInterval
{
  // FIXME
  return 0;
}

+ (NSUInteger) pressedMouseButtons
{
  // FIXME
  return 0;
}

+ (NSTimeInterval) doubleClickInterval
{
  // FIXME
  return 0;
}

- (NSEventPhase) phase
{
  // FIXME
  return NSEventPhaseNone;
}

- (NSEventPhase) momentumPhase
{
  // FIXME
  return NSEventPhaseNone;
}

@end
