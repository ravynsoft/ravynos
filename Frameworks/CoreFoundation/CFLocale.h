/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef struct __NSLocale *CFLocaleRef;

#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFArray.h>
#import <CoreFoundation/CFDictionary.h>

COREFOUNDATION_EXPORT const CFStringRef kCFLocaleIdentifier;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleLanguageCode;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleCountryCode;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleScriptCode;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleVariantCode;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleExemplarCharacterSet;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleCalendarIdentifier;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleCalendar;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleCollationIdentifier;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleUsesMetricSystem;

COREFOUNDATION_EXPORT const CFStringRef kCFLocaleMeasurementSystem;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleDecimalSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleGroupingSeparator;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleCurrencySymbol;
COREFOUNDATION_EXPORT const CFStringRef kCFLocaleCurrencyCode;

COREFOUNDATION_EXPORT const CFStringRef kCFGregorianCalendar;
COREFOUNDATION_EXPORT const CFStringRef kCFBuddhistCalendar;
COREFOUNDATION_EXPORT const CFStringRef kCFChineseCalendar;
COREFOUNDATION_EXPORT const CFStringRef kCFHebrewCalendar;
COREFOUNDATION_EXPORT const CFStringRef kCFIslamicCalendar;
COREFOUNDATION_EXPORT const CFStringRef kCFIslamicCivilCalendar;
COREFOUNDATION_EXPORT const CFStringRef kCFJapaneseCalendar;

COREFOUNDATION_EXPORT const CFStringRef kCFLocaleCurrentLocaleDidChangeNotification;

COREFOUNDATION_EXPORT CFTypeID CFLocaleGetTypeID(void);
COREFOUNDATION_EXPORT CFArrayRef CFLocaleCopyAvailableLocaleIdentifiers(void);
COREFOUNDATION_EXPORT CFArrayRef CFLocaleCopyCommonISOCurrencyCodes(void);
COREFOUNDATION_EXPORT CFLocaleRef CFLocaleCopyCurrent(void);
COREFOUNDATION_EXPORT CFArrayRef CFLocaleCopyISOCountryCodes(void);
COREFOUNDATION_EXPORT CFArrayRef CFLocaleCopyISOCurrencyCodes(void);
COREFOUNDATION_EXPORT CFArrayRef CFLocaleCopyISOLanguageCodes(void);
COREFOUNDATION_EXPORT CFArrayRef CFLocaleCopyPreferredLanguages(void);
COREFOUNDATION_EXPORT CFLocaleRef CFLocaleGetSystem(void);

COREFOUNDATION_EXPORT CFLocaleRef CFLocaleCreate(CFAllocatorRef allocator, CFStringRef identifier);
COREFOUNDATION_EXPORT CFStringRef CFLocaleCreateCanonicalLanguageIdentifierFromString(CFAllocatorRef allocator, CFStringRef identifier);
COREFOUNDATION_EXPORT CFStringRef CFLocaleCreateCanonicalLocaleIdentifierFromString(CFAllocatorRef allocator, CFStringRef identifier);
COREFOUNDATION_EXPORT CFDictionaryRef CFLocaleCreateComponentsFromLocaleIdentifier(CFAllocatorRef allocator, CFStringRef identifier);

COREFOUNDATION_EXPORT CFLocaleRef CFLocaleCreateCopy(CFAllocatorRef allocator, CFLocaleRef self);

COREFOUNDATION_EXPORT CFStringRef CFLocaleCopyDisplayNameForPropertyValue(CFLocaleRef self, CFStringRef key, CFStringRef value);
COREFOUNDATION_EXPORT CFStringRef CFLocaleCreateLocaleIdentifierFromComponents(CFAllocatorRef allocator, CFDictionaryRef dictionary);
COREFOUNDATION_EXPORT CFStringRef CFLocaleGetIdentifier(CFLocaleRef self);
COREFOUNDATION_EXPORT CFTypeRef CFLocaleGetValue(CFLocaleRef self, CFStringRef key);
