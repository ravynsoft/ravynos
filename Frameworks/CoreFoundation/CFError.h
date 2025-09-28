/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct __NSError *CFErrorRef;

COREFOUNDATION_EXPORT const CFStringRef kCFErrorDomainPOSIX;
COREFOUNDATION_EXPORT const CFStringRef kCFErrorDomainCocoa;
COREFOUNDATION_EXPORT const CFStringRef kCFErrorDomainOSStatus;
COREFOUNDATION_EXPORT const CFStringRef kCFErrorDomainMach;

COREFOUNDATION_EXPORT const CFStringRef kCFErrorDescriptionKey;
COREFOUNDATION_EXPORT const CFStringRef kCFErrorLocalizedDescriptionKey;
COREFOUNDATION_EXPORT const CFStringRef kCFErrorUnderlyingErrorKey;
COREFOUNDATION_EXPORT const CFStringRef kCFErrorLocalizedFailureReasonKey;
COREFOUNDATION_EXPORT const CFStringRef kCFErrorLocalizedRecoverySuggestionKey;

COREFOUNDATION_EXPORT CFTypeID CFErrorGetTypeID(void);

COREFOUNDATION_EXPORT CFErrorRef CFErrorCreate(CFAllocatorRef allocator, CFStringRef domain, CFIndex code, CFDictionaryRef userInfo);
COREFOUNDATION_EXPORT CFErrorRef CFErrorCreateWithUserInfoKeysAndValues(CFAllocatorRef allocator, CFStringRef domain, CFIndex code, const void *const *userInfoKeys, const void *const *userInfoValues, CFIndex userInfoCount);

COREFOUNDATION_EXPORT CFStringRef CFErrorGetDomain(CFErrorRef self);
COREFOUNDATION_EXPORT CFIndex CFErrorGetCode(CFErrorRef self);
COREFOUNDATION_EXPORT CFDictionaryRef CFErrorCopyUserInfo(CFErrorRef self);
COREFOUNDATION_EXPORT CFStringRef CFErrorCopyFailureReason(CFErrorRef self);
COREFOUNDATION_EXPORT CFStringRef CFErrorCopyRecoverySuggestion(CFErrorRef self);
COREFOUNDATION_EXPORT CFStringRef CFErrorCopyDescription(CFErrorRef self);
