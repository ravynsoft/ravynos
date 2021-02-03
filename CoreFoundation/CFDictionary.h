/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef const struct __CFDictionary *CFDictionaryRef;
typedef struct __CFDictionary *CFMutableDictionaryRef;

#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFString.h>

typedef const void *(*CFDictionaryRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void (*CFDictionaryReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFStringRef (*CFDictionaryCopyDescriptionCallBack)(const void *value);
typedef Boolean (*CFDictionaryEqualCallBack)(const void *value, const void *other);
typedef CFHashCode (*CFDictionaryHashCallBack)(const void *value);

typedef struct {
    CFIndex version;
    CFDictionaryRetainCallBack retain;
    CFDictionaryReleaseCallBack release;
    CFDictionaryCopyDescriptionCallBack copyDescription;
    CFDictionaryEqualCallBack equal;
    CFDictionaryHashCallBack hash;
} CFDictionaryKeyCallBacks;

typedef struct {
    CFIndex version;
    CFDictionaryRetainCallBack retain;
    CFDictionaryReleaseCallBack release;
    CFDictionaryCopyDescriptionCallBack copyDescription;
    CFDictionaryEqualCallBack equal;
} CFDictionaryValueCallBacks;

typedef void (*CFDictionaryApplierFunction)(const void *key, const void *value, void *context);

COREFOUNDATION_EXPORT const CFDictionaryKeyCallBacks kCFCopyStringDictionaryKeyCallBacks;
COREFOUNDATION_EXPORT const CFDictionaryKeyCallBacks kCFTypeDictionaryKeyCallBacks;
COREFOUNDATION_EXPORT const CFDictionaryValueCallBacks kCFTypeDictionaryValueCallBacks;

COREFOUNDATION_EXPORT CFTypeID CFDictionaryGetTypeID(void);
COREFOUNDATION_EXPORT CFDictionaryRef CFDictionaryCreate(CFAllocatorRef allocator, const void **keys, const void **values, CFIndex count, const CFDictionaryKeyCallBacks *keyCallbacks, const CFDictionaryValueCallBacks *valueCallbacks);

COREFOUNDATION_EXPORT CFDictionaryRef CFDictionaryCreateCopy(CFAllocatorRef allocator, CFDictionaryRef self);

COREFOUNDATION_EXPORT void CFDictionaryApplyFunction(CFDictionaryRef self, CFDictionaryApplierFunction function, void *context);
COREFOUNDATION_EXPORT Boolean CFDictionaryContainsKey(CFDictionaryRef self, const void *key);
COREFOUNDATION_EXPORT Boolean CFDictionaryContainsValue(CFDictionaryRef self, const void *value);
COREFOUNDATION_EXPORT CFIndex CFDictionaryGetCount(CFDictionaryRef self);
COREFOUNDATION_EXPORT CFIndex CFDictionaryGetCountOfKey(CFDictionaryRef self, const void *key);
COREFOUNDATION_EXPORT CFIndex CFDictionaryGetCountOfValue(CFDictionaryRef self, const void *value);
COREFOUNDATION_EXPORT void CFDictionaryGetKeysAndValues(CFDictionaryRef self, const void **keys, const void **values);
COREFOUNDATION_EXPORT const void *CFDictionaryGetValue(CFDictionaryRef self, const void *key);
COREFOUNDATION_EXPORT Boolean CFDictionaryGetValueIfPresent(CFDictionaryRef self, const void *key, const void **value);

COREFOUNDATION_EXPORT CFMutableDictionaryRef CFDictionaryCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallbacks, const CFDictionaryValueCallBacks *valueCallbacks);

COREFOUNDATION_EXPORT CFMutableDictionaryRef CFDictionaryCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFDictionaryRef self);

COREFOUNDATION_EXPORT void CFDictionaryAddValue(CFMutableDictionaryRef self, const void *key, const void *value);
COREFOUNDATION_EXPORT void CFDictionaryRemoveAllValues(CFMutableDictionaryRef self);
COREFOUNDATION_EXPORT void CFDictionaryRemoveValue(CFMutableDictionaryRef self, const void *key);
COREFOUNDATION_EXPORT void CFDictionaryReplaceValue(CFMutableDictionaryRef self, const void *key, const void *value);
COREFOUNDATION_EXPORT void CFDictionarySetValue(CFMutableDictionaryRef self, const void *key, const void *value);
