/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFDate.h>

typedef struct __NSDateFormatter *CFDateFormatterRef;

enum {
    kCFDateFormatterNoStyle = 0,
    kCFDateFormatterShortStyle = 1,
    kCFDateFormatterMediumStyle = 2,
    kCFDateFormatterLongStyle = 3,
    kCFDateFormatterFullStyle = 4
};

typedef CFIndex CFDateFormatterStyle;

COREFOUNDATION_EXPORT CFTypeID CFDateFormatterGetTypeID(void);

COREFOUNDATION_EXPORT CFDateFormatterRef CFDateFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale, CFDateFormatterStyle dateStyle, CFDateFormatterStyle timeStyle);
COREFOUNDATION_EXPORT CFDateRef CFDateFormatterCreateDateFromString(CFAllocatorRef allocator, CFDateFormatterRef self, CFStringRef string, CFRange *rangep);

COREFOUNDATION_EXPORT CFLocaleRef CFDateFormatterGetLocale(CFDateFormatterRef self);
COREFOUNDATION_EXPORT CFDateFormatterStyle CFDateFormatterGetDateStyle(CFDateFormatterRef self);
COREFOUNDATION_EXPORT CFDateFormatterStyle CFDateFormatterGetTimeStyle(CFDateFormatterRef self);

COREFOUNDATION_EXPORT CFTypeRef CFDateFormatterCopyProperty(CFDateFormatterRef self, CFStringRef key);
COREFOUNDATION_EXPORT CFStringRef CFDateFormatterCreateStringWithAbsoluteTime(CFAllocatorRef allocator, CFDateFormatterRef self, CFAbsoluteTime absoluteTime);
COREFOUNDATION_EXPORT CFStringRef CFDateFormatterCreateStringWithDate(CFAllocatorRef allocator, CFDateFormatterRef self, CFDateRef date);
COREFOUNDATION_EXPORT Boolean CFDateFormatterGetAbsoluteTimeFromString(CFDateFormatterRef self, CFStringRef string, CFRange *rangep, CFAbsoluteTime *absoluteTimep);
COREFOUNDATION_EXPORT CFStringRef CFDateFormatterGetFormat(CFDateFormatterRef self);
COREFOUNDATION_EXPORT void CFDateFormatterSetFormat(CFDateFormatterRef self, CFStringRef format);
COREFOUNDATION_EXPORT void CFDateFormatterSetProperty(CFDateFormatterRef self, CFStringRef key, CFTypeRef value);
