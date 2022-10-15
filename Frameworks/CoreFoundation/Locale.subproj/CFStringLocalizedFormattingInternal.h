/*	CFStringLocalizedFormattingInternal.h
	Copyright (c) 2015, Apple Inc. All rights reserved.
 */

#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFCharacterSet.h>

CF_PRIVATE CFDictionaryRef _CFStringGetLocalizedFormattingInfo(void);
CF_PRIVATE CFDictionaryRef _CFStringGetInputIdentifierFormatterMappingFromDescriptor(CFDictionaryRef inputInfo);
CF_PRIVATE bool _CFStringContentsInCharacterSet(CFStringRef str, CFCharacterSetRef set);
CF_PRIVATE CFDictionaryRef _CFStringGetRelevantLocaleInfoFromLocaleSummary(CFDictionaryRef localeSummaryInfo, CFLocaleRef locale);