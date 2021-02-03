/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct __NSNotificationCenter *CFNotificationCenterRef;

typedef enum {
    CFNotificationSuspensionBehaviorDrop = 1,
    CFNotificationSuspensionBehaviorCoalesce = 2,
    CFNotificationSuspensionBehaviorHold = 3,
    CFNotificationSuspensionBehaviorDeliverImmediately = 4,
} CFNotificationSuspensionBehavior;

typedef void (*CFNotificationCallback)(CFNotificationCenterRef self, void *observer, CFStringRef name, const void *object, CFDictionaryRef userInfo);

COREFOUNDATION_EXPORT CFTypeID CFNotificationCenterGetTypeID(void);

COREFOUNDATION_EXPORT CFNotificationCenterRef CFNotificationCenterGetLocalCenter(void);
COREFOUNDATION_EXPORT CFNotificationCenterRef CFNotificationCenterGetDistributedCenter(void);

COREFOUNDATION_EXPORT void CFNotificationCenterAddObserver(CFNotificationCenterRef self, const void *observer, CFNotificationCallback callback, CFStringRef name, const void *object, CFNotificationSuspensionBehavior suspensionBehavior);
COREFOUNDATION_EXPORT void CFNotificationCenterPostNotification(CFNotificationCenterRef self, CFStringRef name, const void *object, CFDictionaryRef userInfo, Boolean immediate);
COREFOUNDATION_EXPORT void CFNotificationCenterPostNotificationWithOptions(CFNotificationCenterRef self, CFStringRef name, const void *object, CFDictionaryRef userInfo, CFOptionFlags options);
COREFOUNDATION_EXPORT void CFNotificationCenterRemoveEveryObserver(CFNotificationCenterRef self, const void *observer);
COREFOUNDATION_EXPORT void CFNotificationCenterRemoveObserver(CFNotificationCenterRef self, const void *observer, CFStringRef name, const void *object);
