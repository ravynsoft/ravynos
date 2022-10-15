/*	CFAttributedStringPriv.h
	Copyright (c) 2004-2019, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFATTRIBUTEDSTRINGPRIV__)
#define __COREFOUNDATION_CFATTRIBUTEDSTRINGPRIV__ 1

#include <CoreFoundation/CFAttributedString.h>

CF_EXTERN_C_BEGIN

/* CFAttributedStringCreateWithRuns() creates an attributed string from the specified string and a list of sparse attribute dictionaries. The ranges for the dictionaries do not have to cover the string, but they should be ordered from low to high indexes, and they shouldn't overlap. CFAttributedStringGetRuns() will return conformant arrays.
*/
CF_EXPORT CFAttributedStringRef _CFAttributedStringCreateWithRuns(CFAllocatorRef alloc, CFStringRef str, const CFDictionaryRef *attrDictionaries, const CFRange *runRanges, CFIndex numRuns);

/* The next two functions are SPI which allow return bulk information about attributes. The includeEmpty argument enables choosing to ignore or include runs where the attribute dictionaries are empty. The size of the CFRange and CFDictionaryRef arguments passed to CFAttributedStringGetRuns() should be obtained from CFAttributedStringGetNumberOfRuns(), called with the same value for includeEmpty.
*/
CF_EXPORT void _CFAttributedStringGetRuns(CFAttributedStringRef attrStr, Boolean includeEmpty, CFDictionaryRef *attrDictionaries, CFRange *runRanges);
CF_EXPORT CFIndex _CFAttributedStringGetNumberOfRuns(CFAttributedStringRef attrStr, Boolean includeEmpty);


CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFATTRIBUTEDSTRINGPRIV__ */

