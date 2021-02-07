/* Interface of class NSSliderAccessory
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

#ifndef _NSSliderAccessory_h_GNUSTEP_GUI_INCLUDE
#define _NSSliderAccessory_h_GNUSTEP_GUI_INCLUDE

#import <Foundation/NSObject.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSSliderAccessoryBehavior;
@class NSImage;
  
@interface NSSliderAccessory : NSObject <NSCopying, NSCoding>
{
  NSImage *_image;
  NSSliderAccessoryBehavior *_behavior;
  BOOL _enabled;
}

+ (NSSliderAccessory *) accessoryWithImage: (NSImage *)image;

- (NSSliderAccessoryBehavior *) behavior;
- (void) setBehavior: (NSSliderAccessoryBehavior *)behavior;

- (BOOL) isEnabled;
- (void) setEnabled: (BOOL)flag;
  
@end

// Behavior...
DEFINE_BLOCK_TYPE(GSSliderAccessoryBehaviorHandler, void, NSSliderAccessory*);
  
@interface NSSliderAccessoryBehavior : NSObject <NSCopying, NSCoding>

// Initializers
+ (NSSliderAccessoryBehavior *) behaviorWithHandler: (GSSliderAccessoryBehaviorHandler)handler;
+ (NSSliderAccessoryBehavior *) behaviorWithTarget: (id)target action: (SEL)action;

// Behaviors...
+ (NSSliderAccessoryBehavior *) automaticBehavior;
+ (NSSliderAccessoryBehavior *) valueResetBehavior;
+ (NSSliderAccessoryBehavior *) valueStepBehavior;

// Handle events...
- (void) handleAction: (NSSliderAccessory *)sender;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSSliderAccessory_h_GNUSTEP_GUI_INCLUDE */

