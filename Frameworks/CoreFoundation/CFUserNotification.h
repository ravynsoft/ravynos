/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFDate.h>
#import <CoreFoundation/CFRunLoop.h>
#import <CoreFoundation/CFURL.h>

typedef struct CFUserNotification *CFUserNotificationRef;

typedef void (*CFUserNotificationCallBack)(CFUserNotificationRef userNotification, CFOptionFlags responseFlags);
enum {
    kCFUserNotificationStopAlertLevel = 0,
    kCFUserNotificationNoteAlertLevel = 1,
    kCFUserNotificationCautionAlertLevel = 2,
    kCFUserNotificationPlainAlertLevel = 3,
};

enum {
    kCFUserNotificationDefaultResponse = 0,
    kCFUserNotificationAlternateResponse = 1,
    kCFUserNotificationOtherResponse = 2,
    kCFUserNotificationCancelResponse = 3,
};

enum {
    kCFUserNotificationNoDefaultButtonFlag = (1 << 5),
    kCFUserNotificationUseRadioButtonsFlag = (1 << 6),
};

COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationIconURLKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationSoundURLKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationLocalizationURLKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationAlertHeaderKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationAlertMessageKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationDefaultButtonTitleKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationAlternateButtonTitleKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationOtherButtonTitleKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationProgressIndicatorValueKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationPopUpTitlesKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationTextFieldTitlesKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationCheckBoxTitlesKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationTextFieldValuesKey;
COREFOUNDATION_EXPORT const CFStringRef kCFUserNotificationPopUpSelectionKey;

COREFOUNDATION_EXPORT CFTypeID CFUserNotificationGetTypeID(void);

COREFOUNDATION_EXPORT CFOptionFlags CFUserNotificationCheckBoxChecked(CFIndex i);
COREFOUNDATION_EXPORT CFOptionFlags CFUserNotificationPopUpSelection(CFIndex n);
COREFOUNDATION_EXPORT CFOptionFlags CFUserNotificationSecureTextField(CFIndex i);

COREFOUNDATION_EXPORT CFInteger CFUserNotificationDisplayAlert(CFTimeInterval timeout, CFOptionFlags flags, CFURLRef iconURL, CFURLRef soundURL, CFURLRef localizationURL, CFStringRef alertHeader, CFStringRef alertMessage, CFStringRef defaultButtonTitle, CFStringRef alternateButtonTitle, CFStringRef otherButtonTitle, CFOptionFlags *responseFlags);
COREFOUNDATION_EXPORT CFInteger CFUserNotificationDisplayNotice(CFTimeInterval timeout, CFOptionFlags flags, CFURLRef iconURL, CFURLRef soundURL, CFURLRef localizationURL, CFStringRef alertHeader, CFStringRef alertMessage, CFStringRef defaultButtonTitle);

COREFOUNDATION_EXPORT CFUserNotificationRef CFUserNotificationCreate(CFAllocatorRef allocator, CFTimeInterval timeout, CFOptionFlags flags, CFInteger *error, CFDictionaryRef dictionary);

COREFOUNDATION_EXPORT CFInteger CFUserNotificationCancel(CFUserNotificationRef self);
COREFOUNDATION_EXPORT CFRunLoopSourceRef CFUserNotificationCreateRunLoopSource(CFAllocatorRef allocator, CFUserNotificationRef self, CFUserNotificationCallBack callback, CFIndex order);
COREFOUNDATION_EXPORT CFDictionaryRef CFUserNotificationGetResponseDictionary(CFUserNotificationRef self);
COREFOUNDATION_EXPORT CFStringRef CFUserNotificationGetResponseValue(CFUserNotificationRef self, CFStringRef key, CFIndex index);
COREFOUNDATION_EXPORT CFInteger CFUserNotificationReceiveResponse(CFUserNotificationRef self, CFTimeInterval timeout, CFOptionFlags *responseFlags);
COREFOUNDATION_EXPORT CFInteger CFUserNotificationUpdate(CFUserNotificationRef self, CFTimeInterval timeout, CFOptionFlags flags, CFDictionaryRef dictionary);
