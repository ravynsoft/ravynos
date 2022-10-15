/*	CFUserNotification.h
	Copyright (c) 2000-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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

typedef struct CF_BRIDGED_MUTABLE_TYPE(id) __CFUserNotification * CFUserNotificationRef;

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
CFTypeID CFUserNotificationGetTypeID(void) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
CFUserNotificationRef CFUserNotificationCreate(CFAllocatorRef allocator, CFTimeInterval timeout, CFOptionFlags flags, SInt32 *error, CFDictionaryRef dictionary) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
SInt32 CFUserNotificationReceiveResponse(CFUserNotificationRef userNotification, CFTimeInterval timeout, CFOptionFlags *responseFlags) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
CFStringRef CFUserNotificationGetResponseValue(CFUserNotificationRef userNotification, CFStringRef key, CFIndex idx) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
CFDictionaryRef CFUserNotificationGetResponseDictionary(CFUserNotificationRef userNotification) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
SInt32 CFUserNotificationUpdate(CFUserNotificationRef userNotification, CFTimeInterval timeout, CFOptionFlags flags, CFDictionaryRef dictionary) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
SInt32 CFUserNotificationCancel(CFUserNotificationRef userNotification) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
CFRunLoopSourceRef CFUserNotificationCreateRunLoopSource(CFAllocatorRef allocator, CFUserNotificationRef userNotification, CFUserNotificationCallBack callout, CFIndex order) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

/* Convenience functions for handling the simplest and most common cases:  
a one-way notification, and a notification with up to three buttons. */
    
CF_EXPORT
SInt32 CFUserNotificationDisplayNotice(CFTimeInterval timeout, CFOptionFlags flags, CFURLRef iconURL, CFURLRef soundURL, CFURLRef localizationURL, CFStringRef alertHeader, CFStringRef alertMessage, CFStringRef defaultButtonTitle) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
SInt32 CFUserNotificationDisplayAlert(CFTimeInterval timeout, CFOptionFlags flags, CFURLRef iconURL, CFURLRef soundURL, CFURLRef localizationURL, CFStringRef alertHeader, CFStringRef alertMessage, CFStringRef defaultButtonTitle, CFStringRef alternateButtonTitle, CFStringRef otherButtonTitle, CFOptionFlags *responseFlags) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);


/* Flags */

CF_ENUM(CFOptionFlags) {
    kCFUserNotificationStopAlertLevel API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos)    = 0,
    kCFUserNotificationNoteAlertLevel API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos)    = 1,
    kCFUserNotificationCautionAlertLevel API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos) = 2,
    kCFUserNotificationPlainAlertLevel API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos)   = 3
};

CF_ENUM(CFOptionFlags) {
    kCFUserNotificationDefaultResponse API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos)   = 0,
    kCFUserNotificationAlternateResponse API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos) = 1,
    kCFUserNotificationOtherResponse API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos)     = 2,
    kCFUserNotificationCancelResponse API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos)    = 3
};

CF_ENUM(CFOptionFlags) {
    kCFUserNotificationNoDefaultButtonFlag API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos) = (1UL << 5),
    kCFUserNotificationUseRadioButtonsFlag API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos) = (1UL << 6)
};

CF_INLINE CFOptionFlags CFUserNotificationCheckBoxChecked(CFIndex i) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos) {return ((CFOptionFlags)(1UL << (8 + i)));}
CF_INLINE CFOptionFlags CFUserNotificationSecureTextField(CFIndex i) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos) {return ((CFOptionFlags)(1UL << (16 + i)));}
CF_INLINE CFOptionFlags CFUserNotificationPopUpSelection(CFIndex n) API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos) {return ((CFOptionFlags)(n << 24));}


/* Keys */

CF_EXPORT
const CFStringRef kCFUserNotificationIconURLKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationSoundURLKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationLocalizationURLKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationAlertHeaderKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationAlertMessageKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationDefaultButtonTitleKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationAlternateButtonTitleKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationOtherButtonTitleKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationProgressIndicatorValueKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationPopUpTitlesKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationTextFieldTitlesKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationCheckBoxTitlesKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationTextFieldValuesKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationPopUpSelectionKey	API_AVAILABLE(macos(10.3)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXPORT
const CFStringRef kCFUserNotificationAlertTopMostKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);
        
CF_EXPORT
const CFStringRef kCFUserNotificationKeyboardTypesKey API_AVAILABLE(macos(10.0)) API_UNAVAILABLE(ios, watchos, tvos);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFUSERNOTIFICATION__ */

