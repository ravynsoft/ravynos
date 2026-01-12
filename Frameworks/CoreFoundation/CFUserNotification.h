/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*	CFUserNotification.h
	Copyright (c) 2000-2014, Apple Inc.  All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFUSERNOTIFICATION__)
#define __COREFOUNDATION_CFUSERNOTIFICATION__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFRunLoop.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef struct __CFUserNotification * CFUserNotificationRef;

/* A CFUserNotification is a notification intended to be presented to a 
user at the console (if one is present).  This is for the use of processes
that do not otherwise have user interfaces, but may need occasional
interaction with a user.  There is a parallel API for this functionality
at the System framework level, described in UNCUserNotification.h.

The contents of the notification can include a header, a message, textfields,
a popup button, radio buttons or checkboxes, a progress indicator, and up to
three ordinary buttons.  All of these items are optional, but a default
button will be supplied even if not specified unless the
kCFUserNotificationNoDefaultButtonFlag is set.

The contents of the notification are specified in the dictionary used to
create the notification, whose keys should be taken from the list of constants
below, and whose values should be either strings or arrays of strings
(except for kCFUserNotificationProgressIndicatorValueKey, in which case the
value should be a number between 0 and 1, for a "definite" progress indicator,
or a boolean, for an "indefinite" progress indicator).  Additionally, URLs can
optionally be supplied for an icon, a sound, and a bundle whose Localizable.strings
files will be used to localize strings.
    
Certain request flags are specified when a notification is created.
These specify an alert level for the notification, determine whether
radio buttons or check boxes are to be used, specify which if any of these
are checked by default, specify whether any of the textfields are to
be secure textfields, and determine which popup item should be selected
by default.  A timeout is also specified, which determines how long the
notification should be supplied to the user (if zero, it will not timeout).
    
A CFUserNotification is dispatched for presentation when it is created.
If any reply is required, it may be awaited in one of two ways:  either
synchronously, using CFUserNotificationReceiveResponse, or asynchronously,
using a run loop source.  CFUserNotificationReceiveResponse has a timeout
parameter that determines how long it will block (zero meaning indefinitely)
and it may be called as many times as necessary until a response arrives.
If a notification has not yet received a response, it may be updated with
new information, or it may be cancelled.  Notifications may not be reused.
    
When a response arrives, it carries with it response flags that describe
which button was used to dismiss the notification, which checkboxes or
radio buttons were checked, and what the selection of the popup was.
It also carries a response dictionary, which describes the contents
of the textfields.  */
    
typedef void (*CFUserNotificationCallBack)(CFUserNotificationRef userNotification, CFOptionFlags responseFlags);

CF_EXPORT
CFTypeID CFUserNotificationGetTypeID(void);

CF_EXPORT
CFUserNotificationRef CFUserNotificationCreate(CFAllocatorRef allocator, CFTimeInterval timeout, CFOptionFlags flags, SInt32 *error, CFDictionaryRef dictionary);

CF_EXPORT
SInt32 CFUserNotificationReceiveResponse(CFUserNotificationRef userNotification, CFTimeInterval timeout, CFOptionFlags *responseFlags);

CF_EXPORT
CFStringRef CFUserNotificationGetResponseValue(CFUserNotificationRef userNotification, CFStringRef key, CFIndex idx);

CF_EXPORT
CFDictionaryRef CFUserNotificationGetResponseDictionary(CFUserNotificationRef userNotification);

CF_EXPORT
SInt32 CFUserNotificationUpdate(CFUserNotificationRef userNotification, CFTimeInterval timeout, CFOptionFlags flags, CFDictionaryRef dictionary);

CF_EXPORT
SInt32 CFUserNotificationCancel(CFUserNotificationRef userNotification);

CF_EXPORT
CFRunLoopSourceRef CFUserNotificationCreateRunLoopSource(CFAllocatorRef allocator, CFUserNotificationRef userNotification, CFUserNotificationCallBack callout, CFIndex order);

/* Convenience functions for handling the simplest and most common cases:  
a one-way notification, and a notification with up to three buttons. */
    
CF_EXPORT
SInt32 CFUserNotificationDisplayNotice(CFTimeInterval timeout, CFOptionFlags flags, CFURLRef iconURL, CFURLRef soundURL, CFURLRef localizationURL, CFStringRef alertHeader, CFStringRef alertMessage, CFStringRef defaultButtonTitle);

CF_EXPORT
SInt32 CFUserNotificationDisplayAlert(CFTimeInterval timeout, CFOptionFlags flags, CFURLRef iconURL, CFURLRef soundURL, CFURLRef localizationURL, CFStringRef alertHeader, CFStringRef alertMessage, CFStringRef defaultButtonTitle, CFStringRef alternateButtonTitle, CFStringRef otherButtonTitle, CFOptionFlags *responseFlags);


/* Flags */

enum {
    kCFUserNotificationStopAlertLevel 		= 0,
    kCFUserNotificationNoteAlertLevel 		= 1,
    kCFUserNotificationCautionAlertLevel 	= 2,
    kCFUserNotificationPlainAlertLevel		= 3
};

enum {
    kCFUserNotificationDefaultResponse		= 0,
    kCFUserNotificationAlternateResponse	= 1,
    kCFUserNotificationOtherResponse		= 2,
    kCFUserNotificationCancelResponse		= 3
};

enum {
    kCFUserNotificationNoDefaultButtonFlag 	= (1UL << 5),
    kCFUserNotificationUseRadioButtonsFlag 	= (1UL << 6)
};

CF_INLINE CFOptionFlags CFUserNotificationCheckBoxChecked(CFIndex i) {return ((CFOptionFlags)(1UL << (8 + i)));}
CF_INLINE CFOptionFlags CFUserNotificationSecureTextField(CFIndex i) {return ((CFOptionFlags)(1UL << (16 + i)));}
CF_INLINE CFOptionFlags CFUserNotificationPopUpSelection(CFIndex n) {return ((CFOptionFlags)(n << 24));}


/* Keys */

CF_EXPORT
const CFStringRef kCFUserNotificationIconURLKey;

CF_EXPORT
const CFStringRef kCFUserNotificationSoundURLKey;

CF_EXPORT
const CFStringRef kCFUserNotificationLocalizationURLKey;

CF_EXPORT
const CFStringRef kCFUserNotificationAlertHeaderKey;

CF_EXPORT
const CFStringRef kCFUserNotificationAlertMessageKey;

CF_EXPORT
const CFStringRef kCFUserNotificationDefaultButtonTitleKey;

CF_EXPORT
const CFStringRef kCFUserNotificationAlternateButtonTitleKey;

CF_EXPORT
const CFStringRef kCFUserNotificationOtherButtonTitleKey;

CF_EXPORT
const CFStringRef kCFUserNotificationProgressIndicatorValueKey;

CF_EXPORT
const CFStringRef kCFUserNotificationPopUpTitlesKey;

CF_EXPORT
const CFStringRef kCFUserNotificationTextFieldTitlesKey;

CF_EXPORT
const CFStringRef kCFUserNotificationCheckBoxTitlesKey;

CF_EXPORT
const CFStringRef kCFUserNotificationTextFieldValuesKey;

CF_EXPORT
const CFStringRef kCFUserNotificationPopUpSelectionKey	CF_AVAILABLE(10_3, NA);

#if (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
CF_EXPORT
const CFStringRef kCFUserNotificationAlertTopMostKey;
        
CF_EXPORT
const CFStringRef kCFUserNotificationKeyboardTypesKey;
#endif

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFUSERNOTIFICATION__ */

