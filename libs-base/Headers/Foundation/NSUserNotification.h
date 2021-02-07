/* Interface for NSUserNotification for GNUstep
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

#ifndef __NSUserNotification_h_INCLUDE
#define __NSUserNotification_h_INCLUDE

#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_8,GS_API_LATEST)
#if __has_feature(objc_default_synthesize_properties)

#import <Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif


@class NSString, NSDictionary, NSArray, NSDateComponents, NSDate, NSTimeZone, NSImage, NSAttributedString;
@class NSMutableArray;

@protocol NSUserNotificationCenterDelegate;

enum
{
  NSUserNotificationActivationTypeNone				= 0,
  NSUserNotificationActivationTypeContentsClicked		= 1,
  NSUserNotificationActivationTypeActionButtonClicked           = 2
#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)
  ,NSUserNotificationActivationTypeReplied			= 3
#endif
};
typedef NSInteger NSUserNotificationActivationType;


GS_EXPORT_CLASS
@interface NSUserNotification : NSObject <NSCopying>
{
#if	GS_EXPOSE(NSUserNotification)
  @public
  id _uniqueId;
#endif
}

@property (copy) NSString *title;
@property (copy) NSString *subtitle;
@property (copy) NSString *informativeText;
@property (copy) NSString *actionButtonTitle;
@property (copy) NSDictionary *userInfo;
@property (copy) NSDate *deliveryDate;
@property (copy) NSTimeZone *deliveryTimeZone;
@property (copy) NSDateComponents *deliveryRepeatInterval;
@property (readonly) NSDate *actualDeliveryDate;
@property (readonly, getter=isPresented) BOOL presented;

@property (readonly, getter=isRemote) BOOL remote;
@property (copy) NSString *soundName;
@property BOOL hasActionButton;
@property (readonly) NSUserNotificationActivationType activationType;
@property (copy) NSString *otherButtonTitle;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST)

@property (copy) NSString *identifier;
@property (copy) NSImage *contentImage;
@property BOOL hasReplyButton;
@property (copy) NSString *responsePlaceholder;
@property (readonly) NSAttributedString *response;

#endif /* OS_API_VERSION(MAC_OS_X_VERSION_10_9,GS_API_LATEST) */

@end

GS_EXPORT NSString * const NSUserNotificationDefaultSoundName;

GS_EXPORT_CLASS
@interface NSUserNotificationCenter : NSObject
{
#if	GS_EXPOSE(NSUserNotificationCenter)
  NSMutableArray *_scheduledNotifications;
  NSMutableArray *_deliveredNotifications;
#endif
}

+ (NSUserNotificationCenter *)defaultUserNotificationCenter;

@property (assign) id <NSUserNotificationCenterDelegate> delegate;
@property (copy) NSArray *scheduledNotifications;

- (void) scheduleNotification: (NSUserNotification *)notification;
- (void) removeScheduledNotification: (NSUserNotification *)notification;

@property (readonly) NSArray *deliveredNotifications;

- (void) deliverNotification: (NSUserNotification *)notification;
- (void) removeDeliveredNotification: (NSUserNotification *)notification;
- (void) removeAllDeliveredNotifications;

@end


@protocol NSUserNotificationCenterDelegate <NSObject>
#if GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSUserNotificationCenterDelegateMethods)
#endif

- (void) userNotificationCenter: (NSUserNotificationCenter *)center
         didDeliverNotification: (NSUserNotification *)notification;
- (void) userNotificationCenter: (NSUserNotificationCenter *)center
        didActivateNotification: (NSUserNotification *)notification;
- (BOOL) userNotificationCenter: (NSUserNotificationCenter *)center
      shouldPresentNotification: (NSUserNotification *)notification;

@end

#if	defined(__cplusplus)
}
#endif

#endif /* OS_API_VERSION(MAC_OS_X_VERSION_10_8,GS_API_LATEST) */
#endif /* __has_feature(objc_default_synthesize_properties) */
#endif	/* __NSUserNotification_h_INCLUDE */
