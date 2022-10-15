/*	CFDateIntervalFormatter.c
	Copyright (c) 1998-2018, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#include <CoreFoundation/CFDateIntervalFormatter.h>
#include <CoreFoundation/CFRuntime.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"

#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFDateFormatter.h>
#include <CoreFoundation/CFDateInterval.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFTimeZone.h>

#include <unicode/udateintervalformat.h>

#if TARGET_OS_WASI
#define LOCK() do {} while (0)
#define UNLOCK() do {} while (0)
#else
#include <dispatch/dispatch.h>

#define LOCK() do { dispatch_semaphore_wait(formatter->_lock, DISPATCH_TIME_FOREVER); } while(0)
#define UNLOCK() do { dispatch_semaphore_signal(formatter->_lock); } while(0)
#endif

CF_INLINE void __CFReleaseIfNotNull(CFTypeRef object) {
    if (object) {
        CFRelease(object);
    }
}

CF_INLINE CFTypeRef __CFRetainIfNotNull(CFTypeRef object) {
    if (object) {
        CFRetain(object);
    }
    
    return object;
}

struct __CFDateIntervalFormatter {
    CFRuntimeBase _base;
    CFLocaleRef _locale;
    CFCalendarRef _calendar;
    CFTimeZoneRef _timeZone;
    CFStringRef _dateTemplateFromStyles;
    CFStringRef _dateTemplate;
    void *_formatter;
    CFDateIntervalFormatterStyle _dateStyle;
    CFDateIntervalFormatterStyle _timeStyle;
    _CFDateIntervalFormatterBoundaryStyle _boundaryStyle;
#if !TARGET_OS_WASI
    dispatch_semaphore_t _lock;
#endif
    bool _modified:1;
    bool _useTemplate:1;
};

static void __CFDateIntervalFormatterDeallocate(CFTypeRef object);
const CFRuntimeClass __CFDateIntervalFormatterClass = {
    0,
    "CFDateIntervalFormatter",
    NULL,    // init
    NULL,    // copy
    &__CFDateIntervalFormatterDeallocate,    // dealloc
    NULL,    // equals
    NULL,    // hash
    NULL,    // copyFormattingDescription
    NULL,    // copyDescription
};

static CFStringRef createLocaleIDFromLocaleAndCalendar(CFLocaleRef locale, CFCalendarRef calendar) {
    CFMutableDictionaryRef dict;
    
    {
        CFDictionaryRef immutableDict = CFLocaleCreateComponentsFromLocaleIdentifier(kCFAllocatorSystemDefault, CFLocaleGetIdentifier(locale));
        dict = CFDictionaryCreateMutableCopy(kCFAllocatorSystemDefault, 0, immutableDict);
        CFRelease(immutableDict);
    }
    
    if (calendar) {
        CFDictionarySetValue(dict, kCFLocaleCalendar, calendar);
    }
    CFStringRef localeID = CFLocaleCreateLocaleIdentifierFromComponents(kCFAllocatorSystemDefault, dict);
    CFRelease(dict);
    
    return localeID;
}

static void updateDateTemplate(CFDateIntervalFormatterRef dif, CFDateIntervalFormatterStyle dateStyle, CFDateIntervalFormatterStyle timeStyle);
static void updateDateTemplateFromCurrentSettings(CFDateIntervalFormatterRef dif) {
    updateDateTemplate(dif, dif->_dateStyle, dif->_timeStyle);
}

static void updateDateTemplate(CFDateIntervalFormatterRef dif, CFDateIntervalFormatterStyle dateStyle, CFDateIntervalFormatterStyle timeStyle) {
    CFDateFormatterRef formatter;
    {
        CFLocaleRef locale = dif->_locale ? CFRetain(dif->_locale) : CFLocaleCopyCurrent();
        CFCalendarRef unretainedCalendar = dif->_calendar ?: (CFCalendarRef)CFLocaleGetValue(locale, kCFLocaleCalendar);
        formatter = CFDateFormatterCreate(kCFAllocatorSystemDefault, locale, (CFDateFormatterStyle)dateStyle, (CFDateFormatterStyle)timeStyle);
        CFDateFormatterSetProperty(formatter, kCFDateFormatterCalendar, unretainedCalendar);
        CFRelease(locale);
    }
    
    CFStringRef template = CFDateFormatterGetFormat(formatter);
    __CFReleaseIfNotNull(dif->_dateTemplateFromStyles);
    dif->_dateTemplateFromStyles = CFStringCreateCopy(kCFAllocatorSystemDefault, template);
    CFRelease(formatter);
}

static void updateFormatter(CFDateIntervalFormatterRef dif) {
    if (dif->_modified && dif->_formatter != NULL) {
        udtitvfmt_close(dif->_formatter);
        dif->_formatter = NULL;
        dif->_modified = false;
    }
    
    if (dif->_formatter == NULL) {
        CFLocaleRef locale = dif->_locale;
        if (locale) {
            CFRetain(locale);
        } else {
            locale = CFLocaleCopyCurrent();
        }
        
        CFCalendarRef unretainedCalendar = dif->_calendar;
        if (unretainedCalendar == NULL) {
            unretainedCalendar = (CFCalendarRef)CFLocaleGetValue(locale, kCFDateFormatterCalendar);
        }
        
        CFStringRef localeID = createLocaleIDFromLocaleAndCalendar(locale, unretainedCalendar);
        
        char localeBuffer[100] = {0};
        CFStringGetCString(localeID, localeBuffer, 100, kCFStringEncodingUTF8);

        UniChar timeZoneID[100] = {0};
        CFTimeZoneRef timeZone = dif->_timeZone;
        if (timeZone) {
            CFRetain(timeZone);
        } else {
            timeZone = CFTimeZoneCopyDefault();
        }
        CFStringRef unretainedTimeZoneName = CFTimeZoneGetName(timeZone);
        CFStringGetCharacters(unretainedTimeZoneName, CFRangeMake(0, __CFMin(CFStringGetLength(unretainedTimeZoneName), 100)), timeZoneID);
        
        CFStringRef unretainedTemplate = dif->_dateTemplateFromStyles;
        if (dif->_useTemplate) {
            unretainedTemplate = dif->_dateTemplate;
        }
        UniChar templateStr[100] = {0};
        CFStringGetCharacters(unretainedTemplate, CFRangeMake(0, __CFMin(CFStringGetLength(unretainedTemplate), 100)), templateStr);
        
        UErrorCode status = U_ZERO_ERROR;
        dif->_formatter = udtitvfmt_open(localeBuffer, templateStr, CFStringGetLength(unretainedTemplate), timeZoneID, CFStringGetLength(unretainedTimeZoneName), &status);
        if (U_FAILURE(status)) {
            CFLog(kCFLogLevelError, CFSTR("udtitvfmt_open failed!  Tried to set template: %@ for locale %s and timezone %@ and got status code: %s"), unretainedTemplate, localeBuffer, unretainedTimeZoneName, u_errorName(status));
        }
        if (!(dif->_formatter)) {
            CFLog(kCFLogLevelError, CFSTR("udtitvfmt_open failed!  Formatter is NULL! -- locale: %s, template: %@, timezone: %@, status: %s"), localeBuffer, unretainedTemplate, unretainedTimeZoneName, u_errorName(status));
        }
        
#if TARGET_OS_MAC
        UDateIntervalFormatAttributeValue uDateIntervalMinimizationStyle = UDTITVFMT_MINIMIZE_NONE;
        const _CFDateIntervalFormatterBoundaryStyle type = dif->_boundaryStyle;
        switch (type) {
            case kCFDateIntervalFormatterBoundaryStyleMinimizeAdjacentMonths:
                uDateIntervalMinimizationStyle = UDTITVFMT_MINIMIZE_ADJACENT_MONTHS;
                
            default:
                break;
        }
        
        if (dif->_formatter) {
            udtitvfmt_setAttribute(dif->_formatter, UDTITVFMT_MINIMIZE_TYPE, uDateIntervalMinimizationStyle, &status);
            if (U_FAILURE(status)) {
                CFLog(kCFLogLevelError, CFSTR("udtitvfmt_setAttribute failed!  Tried to set minimize type: %d and got status code: %s"), (int)dif->_boundaryStyle, u_errorName(status));
            }
        }
#endif
        
        CFRelease(locale);
        CFRelease(localeID);
        CFRelease(timeZone);
    }
}

CFDateIntervalFormatterRef CFDateIntervalFormatterCreate(CFAllocatorRef allocator, CFLocaleRef locale, CFDateIntervalFormatterStyle dateStyle, CFDateIntervalFormatterStyle timeStyle) {
    struct __CFDateIntervalFormatter *memory;
    uint32_t size = sizeof(struct __CFDateIntervalFormatter) - sizeof(CFRuntimeBase);
    if (!allocator) {
        allocator = __CFGetDefaultAllocator();
    }
    __CFGenericValidateType(allocator, CFAllocatorGetTypeID());
    memory = (struct __CFDateIntervalFormatter *)_CFRuntimeCreateInstance(allocator, _kCFRuntimeIDCFDateIntervalFormatter, size, NULL);
    if (!memory) {
        return (CFDateIntervalFormatterRef _Nonnull)NULL;
    }
    
    switch (dateStyle) {
        case kCFDateIntervalFormatterNoStyle:
        case kCFDateIntervalFormatterShortStyle:
        case kCFDateIntervalFormatterMediumStyle:
        case kCFDateIntervalFormatterLongStyle:
        case kCFDateIntervalFormatterFullStyle: break;
        default:
            CFAssert2(0, __kCFLogAssertion, "%s(): unknown date style %ld", __PRETTY_FUNCTION__, dateStyle);
            memory->_dateStyle = kCFDateIntervalFormatterMediumStyle;
            break;
    }
    switch (timeStyle) {
        case kCFDateIntervalFormatterNoStyle:
        case kCFDateIntervalFormatterShortStyle:
        case kCFDateIntervalFormatterMediumStyle:
        case kCFDateIntervalFormatterLongStyle:
        case kCFDateIntervalFormatterFullStyle: break;
        default:
            CFAssert2(0, __kCFLogAssertion, "%s(): unknown time style %ld", __PRETTY_FUNCTION__, dateStyle);
            memory->_timeStyle = kCFDateIntervalFormatterMediumStyle;
            break;
    }
    
    memory->_dateStyle = dateStyle;
    memory->_timeStyle = timeStyle;
    
    memory->_locale = locale ? CFRetain(locale) : NULL;
    
    memory->_calendar = NULL;
    memory->_timeZone = NULL;
    memory->_dateTemplateFromStyles = NULL;
    memory->_dateTemplate = CFRetain(CFSTR(""));
    memory->_formatter = NULL;
    memory->_boundaryStyle = kCFDateIntervalFormatterBoundaryStyleDefault;
#if !TARGET_OS_WASI
    memory->_lock = dispatch_semaphore_create(1);
#endif
    memory->_modified = false;
    memory->_useTemplate = false;
    
    return (CFDateIntervalFormatterRef)memory;
}

void _CFDateIntervalFormatterInitializeFromCoderValues(CFDateIntervalFormatterRef formatter,
                                                       int64_t dateStyle,
                                                       int64_t timeStyle,
                                                       CFStringRef _Nullable dateTemplate,
                                                       CFStringRef _Nullable dateTemplateFromStyles,
                                                       Boolean modified,
                                                       Boolean useTemplate,
                                                       CFLocaleRef _Nullable locale,
                                                       CFCalendarRef _Nullable calendar,
                                                       CFTimeZoneRef _Nullable timeZone) {
    LOCK();
    formatter->_dateStyle = dateStyle;
    formatter->_timeStyle = timeStyle;
    
#define __CFSetObjectField(field, value) \
{ \
    __auto_type _value = value; \
    if (field != _value) { \
        __CFReleaseIfNotNull(field); \
        field = (__typeof(_value))__CFRetainIfNotNull(_value); \
    } \
}
    
    __CFSetObjectField(formatter->_dateTemplate, dateTemplate);
    __CFSetObjectField(formatter->_dateTemplateFromStyles, dateTemplateFromStyles);
    
    formatter->_modified = modified;
    formatter->_useTemplate = useTemplate;
    
    __CFSetObjectField(formatter->_locale, locale);
    __CFSetObjectField(formatter->_calendar, calendar);
    __CFSetObjectField(formatter->_timeZone, timeZone);
    
    UNLOCK();
}

void _CFDateIntervalFormatterCopyCoderValues(CFDateIntervalFormatterRef formatter,
                                             int64_t *dateStyle,
                                             int64_t *timeStyle,
                                             CFStringRef _Nullable *dateTemplate,
                                             CFStringRef _Nullable *dateTemplateFromStyles,
                                             Boolean *modified,
                                             Boolean *useTemplate,
                                             CFLocaleRef _Nullable *locale,
                                             CFCalendarRef _Nullable *calendar,
                                             CFTimeZoneRef _Nullable *timeZone) {
    LOCK();
    
    *dateStyle = formatter->_dateStyle;
    *timeStyle = formatter->_timeStyle;
    *dateTemplate = __CFRetainIfNotNull(formatter->_dateTemplate);
    *dateTemplateFromStyles = __CFRetainIfNotNull(formatter->_dateTemplateFromStyles);
    *modified = formatter->_modified;
    *useTemplate = formatter->_useTemplate;
    *locale = __CFRetainIfNotNull(formatter->_locale);
    *calendar = (CFCalendarRef)__CFRetainIfNotNull(formatter->_calendar);
    *timeZone = __CFRetainIfNotNull(formatter->_timeZone);
    
    UNLOCK();
}

CFDateIntervalFormatterRef CFDateIntervalFormatterCreateCopy(CFAllocatorRef _Nullable allocator, CFDateIntervalFormatterRef formatter) {
    LOCK();
    CFDateIntervalFormatterRef newFormatter = CFDateIntervalFormatterCreate(allocator, formatter->_locale, formatter->_dateStyle, formatter->_timeStyle);
    
    if (formatter->_calendar) {
        newFormatter->_calendar = _CFCalendarCreateCopy(allocator, formatter->_calendar);
    }
    if (formatter->_timeZone) {
        newFormatter->_timeZone = CFRetain(formatter->_timeZone);
    }
    if (formatter->_dateTemplateFromStyles) {
        newFormatter->_dateTemplateFromStyles = CFStringCreateCopy(allocator, formatter->_dateTemplateFromStyles);
    }
    if (formatter->_dateTemplate) {
        newFormatter->_dateTemplate = CFStringCreateCopy(allocator, formatter->_dateTemplate);
    }
    
    newFormatter->_dateStyle = formatter->_dateStyle;
    newFormatter->_timeStyle = formatter->_timeStyle;
    newFormatter->_modified = formatter->_modified;
    newFormatter->_useTemplate = formatter->_useTemplate;
    UNLOCK();
    
    return newFormatter;
}

static void __CFDateIntervalFormatterDeallocate(CFTypeRef object) {
    CFDateIntervalFormatterRef formatter = (CFDateIntervalFormatterRef)object;
    
    __CFReleaseIfNotNull(formatter->_locale);
    __CFReleaseIfNotNull(formatter->_calendar);
    __CFReleaseIfNotNull(formatter->_timeZone);
    __CFReleaseIfNotNull(formatter->_dateTemplateFromStyles);
    __CFReleaseIfNotNull(formatter->_dateTemplate);
    
    if (formatter->_formatter) {
        udtitvfmt_close(formatter->_formatter);
    }
    
#if !TARGET_OS_WASI
    dispatch_release(formatter->_lock);
#endif
}

CFLocaleRef CFDateIntervalFormatterCopyLocale(CFDateIntervalFormatterRef formatter) {
    LOCK();
    CFLocaleRef locale = formatter->_locale;
    if (locale) {
        CFRetain(locale);
    }
    UNLOCK();
    
    if (!locale) {
        locale = CFLocaleCopyCurrent();
    }
    return locale;
}

void CFDateIntervalFormatterSetLocale(CFDateIntervalFormatterRef formatter, CFLocaleRef locale) {
    LOCK();
    if (locale != formatter->_locale) {
        __CFReleaseIfNotNull(formatter->_locale);
        formatter->_locale = locale ? CFLocaleCreateCopy(kCFAllocatorSystemDefault, locale) : NULL;
        formatter->_modified = true;
        updateDateTemplateFromCurrentSettings(formatter);
    }
    UNLOCK();
}

CFCalendarRef CFDateIntervalFormatterCopyCalendar(CFDateIntervalFormatterRef formatter) {
    LOCK();
    CFCalendarRef calendar = (CFCalendarRef)__CFRetainIfNotNull(formatter->_calendar);
    if (!calendar) {
        if (formatter->_locale) {
            calendar = (CFCalendarRef)CFLocaleGetValue(formatter->_locale, kCFLocaleCalendar);
            CFRetain(calendar);
        } else {
            calendar = CFCalendarCopyCurrent();
        }
    }
    UNLOCK();
    return calendar;
}

void CFDateIntervalFormatterSetCalendar(CFDateIntervalFormatterRef formatter, CFCalendarRef calendar) {
    LOCK();
    if (calendar != formatter->_calendar) {
        __CFReleaseIfNotNull(formatter->_calendar);
        formatter->_calendar = calendar ? _CFCalendarCreateCopy(kCFAllocatorSystemDefault, calendar) : NULL;
        formatter->_modified = true;
        updateDateTemplateFromCurrentSettings(formatter);
    }
    UNLOCK();
}

CFTimeZoneRef CFDateIntervalFormatterCopyTimeZone(CFDateIntervalFormatterRef formatter) {
    LOCK();
    CFTimeZoneRef timeZone = formatter->_timeZone;
    if (timeZone) {
        CFRetain(timeZone);
    }
    UNLOCK();
    
    if (!timeZone) {
        timeZone = CFTimeZoneCopyDefault();
    }
    return timeZone;
}

void CFDateIntervalFormatterSetTimeZone(CFDateIntervalFormatterRef formatter, CFTimeZoneRef timeZone) {
    LOCK();
    if (timeZone != formatter->_timeZone) {
        __CFReleaseIfNotNull(formatter->_timeZone);
        formatter->_timeZone = timeZone ? CFRetain(timeZone) : NULL;
        formatter->_modified = true;
        updateDateTemplateFromCurrentSettings(formatter);
    }
    UNLOCK();
}

CFStringRef CFDateIntervalFormatterCopyDateTemplate(CFDateIntervalFormatterRef formatter) {
    LOCK();
    CFStringRef dateTemplate = formatter->_dateTemplate;
    if (dateTemplate) {
        CFRetain(dateTemplate);
    } else {
        dateTemplate = formatter->_dateTemplateFromStyles;
        if (dateTemplate) {
            CFRetain(dateTemplate);
        }
    }
    UNLOCK();
    
    return dateTemplate;
}

void CFDateIntervalFormatterSetDateTemplate(CFDateIntervalFormatterRef formatter, CFStringRef dateTemplate) {
    if (!dateTemplate) {
        dateTemplate = CFSTR("");
    }
    
    LOCK();
    if (!CFEqual(dateTemplate, formatter->_dateTemplate)) {
        __CFReleaseIfNotNull(formatter->_dateTemplate);
        formatter->_dateTemplate = CFStringCreateCopy(kCFAllocatorSystemDefault, dateTemplate);
        formatter->_modified = true;
        formatter->_useTemplate = true;
    }
    UNLOCK();
}

CFDateIntervalFormatterStyle CFDateIntervalFormatterGetDateStyle(CFDateIntervalFormatterRef formatter) {
    LOCK();
    CFDateIntervalFormatterStyle result = formatter->_dateStyle;
    UNLOCK();
    return result;
}

void CFDateIntervalFormatterSetDateStyle(CFDateIntervalFormatterRef formatter, CFDateIntervalFormatterStyle dateStyle) {
    LOCK();
    formatter->_dateStyle = dateStyle;
    formatter->_modified = true;
    formatter->_useTemplate = false;
    updateDateTemplateFromCurrentSettings(formatter);
    UNLOCK();
}

CFDateIntervalFormatterStyle CFDateIntervalFormatterGetTimeStyle(CFDateIntervalFormatterRef formatter) {
    LOCK();
    CFDateIntervalFormatterStyle result = formatter->_timeStyle;
    UNLOCK();
    return result;
}

void CFDateIntervalFormatterSetTimeStyle(CFDateIntervalFormatterRef formatter, CFDateIntervalFormatterStyle timeStyle) {
    LOCK();
    formatter->_timeStyle = timeStyle;
    formatter->_modified = true;
    formatter->_useTemplate = false;
    updateDateTemplateFromCurrentSettings(formatter);
    UNLOCK();
}

_CFDateIntervalFormatterBoundaryStyle _CFDateIntervalFormatterGetBoundaryStyle(CFDateIntervalFormatterRef formatter) {
    LOCK();
    _CFDateIntervalFormatterBoundaryStyle result = formatter->_boundaryStyle;
    UNLOCK();
    return result;
}

void _CFDateIntervalFormatterSetBoundaryStyle(CFDateIntervalFormatterRef formatter, _CFDateIntervalFormatterBoundaryStyle boundaryStyle) {
    LOCK();
    formatter->_boundaryStyle = boundaryStyle;
    formatter->_modified = true;
    UNLOCK();
}

CFStringRef CFDateIntervalFormatterCreateStringFromDateToDate(CFDateIntervalFormatterRef formatter, CFDateRef fromDate, CFDateRef toDate) {
    LOCK();
    
    CFStringRef resultStr = CFSTR("");
    updateFormatter(formatter);
    
    if (formatter->_formatter) {
        UDate fromUDate = (CFDateGetAbsoluteTime(fromDate) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        UDate toUDate = (CFDateGetAbsoluteTime(toDate) + kCFAbsoluteTimeIntervalSince1970) * 1000.0;
        
#define BUFFER_LENGTH 1000
        
        UChar result[BUFFER_LENGTH];
        memset(result, '\0', BUFFER_LENGTH * sizeof(UChar));
        UErrorCode status = U_ZERO_ERROR;
        int32_t len = udtitvfmt_format(formatter->_formatter, fromUDate, toUDate, result, BUFFER_LENGTH, 0, &status);
        
        if (len > BUFFER_LENGTH) {
            UChar *resultp = (UChar *)calloc(len, sizeof(UChar));
            status = U_ZERO_ERROR;
            len = udtitvfmt_format(formatter->_formatter, fromUDate, toUDate, resultp, len, 0, &status);
            resultStr = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, resultp, len);
            free(resultp);
        } else {
            resultStr = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, result, len);
        }
    } else {
        resultStr = CFSTR("");
    }
    UNLOCK();
    
    return resultStr;
}

CFStringRef CFDateIntervalFormatterCreateStringFromDateInterval(CFDateIntervalFormatterRef formatter, CFDateIntervalRef interval) {
    CFDateRef start = CFDateIntervalCopyStartDate(interval);
    CFDateRef end = CFDateIntervalCopyEndDate(interval);
    CFStringRef result = CFDateIntervalFormatterCreateStringFromDateToDate(formatter, start, end);
    CFRelease(start);
    CFRelease(end);
    return result;
}
