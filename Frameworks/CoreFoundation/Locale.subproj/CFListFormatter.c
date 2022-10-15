/*  CFListFormatter.h
    Copyright (c) 2018-2019, Apple Inc. and the Swift project authors

    Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
    Licensed under Apache License v2.0 with Runtime Library Exception
    See http://swift.org/LICENSE.txt for license information
    See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#include "CFListFormatter.h"

#include "CFICULogging.h"
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include <assert.h>

#define BUFFER_SIZE 256
#define RESULT_BUFFER_SIZE 768

struct __CFListFormatter {
    CFRuntimeBase _base;
    CFLocaleRef _locale;
};

static void __CFListFormatterDeallocate(CFTypeRef cf) {
    assert(cf != NULL);
    CFListFormatterRef formatter = (CFListFormatterRef)cf;
    if (formatter->_locale) { CFRelease(formatter->_locale); }
}

static CFStringRef __CFListFormatterCopyDescription(CFTypeRef cf) {
    assert(cf != NULL);
    CFListFormatterRef formatter = (CFListFormatterRef)cf;
    return CFStringCreateWithFormat(CFGetAllocator(formatter), NULL, CFSTR("<CFListFormatter %p>[%p]"), formatter, CFGetAllocator(formatter));
}

CFTypeID _CFListFormatterGetTypeID(void) {
    return _kCFRuntimeIDCFListFormatter;
}

const CFRuntimeClass __CFListFormatterClass = {
    0,
    "CFListFormatter",
    NULL,   // init
    NULL,   // copy
    __CFListFormatterDeallocate,
    NULL,   // equal
    NULL,   // hash
    NULL,   // copy formatting desc
    __CFListFormatterCopyDescription
};

CFListFormatterRef _CFListFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale) {
    assert(allocator != NULL);
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());

    assert(locale != NULL);
    __CFGenericValidateType(locale, CFLocaleGetTypeID());

    size_t size = sizeof(struct __CFListFormatter) - sizeof(CFRuntimeBase);
    struct __CFListFormatter *memory = (struct __CFListFormatter *)_CFRuntimeCreateInstance(allocator, _CFListFormatterGetTypeID(), size, NULL);
    if (memory == NULL) {
        return NULL;
    }

    memory->_locale = CFRetain(locale);

    return memory;
}

CFStringRef _CFListFormatterCreateStringByJoiningStrings(CFAllocatorRef allocator, const CFListFormatterRef formatter, const CFArrayRef strings) {
    CFAssert1(strings != NULL, __kCFLogAssertion, "%s(): strings should not be NULL", __PRETTY_FUNCTION__);
    if (strings == NULL) {
        return NULL;
    }

    CFIndex const count = CFArrayGetCount(strings);
    if (!count) {
        return CFSTR("");
    }

    CFLocaleRef locale = formatter->_locale;
    CFAssert1(locale != NULL, __kCFLogAssertion, "%s(): locale should not be NULL", __PRETTY_FUNCTION__);

    UChar** ucharStrings = malloc(sizeof(UChar *) * count);
    int32_t* uStringLengths = malloc(sizeof(int32_t) * count);
    bool *needsFree = calloc(count, sizeof(bool));

    CFStringRef string;
    for (int i = 0; i < count; ++i) {
        string = CFArrayGetValueAtIndex(strings, i);
        CFIndex const len = CFStringGetLength(string);
        UChar const *ucharString = CFStringGetCharactersPtr(string);
        if (ucharString == NULL) {
            UChar *ubuffer = malloc(sizeof(UChar) * len);
            CFStringGetCharacters(string, CFRangeMake(0, len), ubuffer);
            ucharString = ubuffer;
            needsFree[i] = true;
        }

        uStringLengths[i] = len;
        ucharStrings[i] = (UChar *)ucharString;
    }

    UErrorCode status = U_ZERO_ERROR;
    CFStringRef localeName = CFLocaleGetIdentifier(locale);
    char const *cstr = CFStringGetCStringPtr(localeName, kCFStringEncodingASCII);
    char buffer[BUFFER_SIZE];
    if (NULL == cstr && CFStringGetCString(localeName, buffer, BUFFER_SIZE, kCFStringEncodingASCII)) {
        cstr = buffer;
    }

    UListFormatter *fmt = __cficu_ulistfmt_open(cstr, &status);
    void (^cleanUp)(void) = ^{
        if (fmt) {
            __cficu_ulistfmt_close(fmt);
        }
        for (int i = 0; i < count; ++i) {
            UChar *ucharString = ucharStrings[i];
            if (ucharString && needsFree[i]) {
                free(ucharString);
            }
        }
        free(needsFree);
        free(ucharStrings);
        free(uStringLengths);
    };

    if (U_FAILURE(status)) {
        cleanUp();
        return NULL;
    }

    CFAssert1(fmt != NULL, __kCFLogAssertion, "%s(): UListFormatter should not be NULL", __PRETTY_FUNCTION__);

    CFStringRef result = NULL;
    UChar resultBuffer[RESULT_BUFFER_SIZE + 1];
    status = U_ZERO_ERROR;
    int32_t length = __cficu_ulistfmt_format(fmt, (const UChar **)ucharStrings, uStringLengths, count, resultBuffer, RESULT_BUFFER_SIZE, &status);
    if (U_SUCCESS(status)) {
        result = CFStringCreateWithCharacters(allocator, resultBuffer, length);
    } else if (status == U_BUFFER_OVERFLOW_ERROR || length > count) {
        status = U_ZERO_ERROR;
        UChar *largeBuffer = malloc(sizeof(UChar) * (length + 1));
        length = __cficu_ulistfmt_format(fmt, (const UChar **)ucharStrings, uStringLengths, count, largeBuffer, length + 1, &status);
        if (U_SUCCESS(status)) {
            result = CFStringCreateWithCharacters(allocator, largeBuffer, length);
        }
        free(largeBuffer);
    }

    cleanUp();

    return result;
}
