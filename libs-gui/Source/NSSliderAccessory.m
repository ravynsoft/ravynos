/* Implementation of class NSSliderAccessory
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 31-07-2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import "AppKit/NSSliderAccessory.h"

  
@implementation NSSliderAccessory

+ (NSSliderAccessory *) accessoryWithImage: (NSImage *)image
{
  return nil;
}

- (NSSliderAccessoryBehavior *) behavior
{
  return _behavior;
}

- (void) setBehavior: (NSSliderAccessoryBehavior *)behavior
{
  ASSIGN(_behavior, behavior);
}

- (BOOL) isEnabled
{
  return _enabled;
}

- (void) setEnabled: (BOOL)flag
{
  _enabled = flag;
}

- (id) copyWithZone: (NSZone *)z
{
  return self;
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
}

@end

@implementation NSSliderAccessoryBehavior 

// Initializers
+ (NSSliderAccessoryBehavior *) behaviorWithHandler: (GSSliderAccessoryBehaviorHandler)handler
{
  return nil;
}

+ (NSSliderAccessoryBehavior *) behaviorWithTarget: (id)target action: (SEL)action
{
  return nil;
}

// Behaviors...
+ (NSSliderAccessoryBehavior *) automaticBehavior
{
  return nil;
}

+ (NSSliderAccessoryBehavior *) valueResetBehavior
{
  return nil;
}

+ (NSSliderAccessoryBehavior *) valueStepBehavior
{
  return nil;
}

// Handle events...
- (void) handleAction: (NSSliderAccessory *)sender
{
  // do nothing
}

- (id) copyWithZone: (NSZone *)z
{
  return self;
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
}
  
@end


