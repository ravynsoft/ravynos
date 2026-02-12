/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
 
#include <CoreFoundation/CoreFoundation.h>

#include "format.h"

static void
_format_dict(const void *key, const void *value, void *context)
{
	CFStringRef valstr, str;
	valstr = format_small(value);
	str = CFStringCreateWithFormat(NULL, NULL, CFSTR("'%@': %@"), key, valstr);
	CFArrayAppendValue(context, str);
	CFRelease(str);
	CFRelease(valstr);
}

static void
_format_array(const void *value, void *context)
{
	CFStringRef valstr;

	valstr = format_small(value);
	CFArrayAppendValue(context, valstr);
	CFRelease(valstr);
}

CFStringRef
format_small(CFTypeRef obj)
{
	CFTypeID type;
	CFStringRef result;

	type = CFGetTypeID(obj);

	if (type == CFDictionaryGetTypeID()) {
		CFMutableArrayRef items;
		CFStringRef combined;

		items = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		CFDictionaryApplyFunction(obj, _format_dict, items);
		combined = CFStringCreateByCombiningStrings(NULL, items, CFSTR(", "));
		CFRelease(items);
		result = CFStringCreateWithFormat(NULL, NULL, CFSTR("{%@}"), combined);
		CFRelease(combined);
	} else if (type == CFArrayGetTypeID()) {
		if (CFArrayGetCount(obj) == 1) {
			result = CFStringCreateWithFormat(NULL, NULL, CFSTR("'%@'"), CFArrayGetValueAtIndex(obj, 0));
		} else {
			CFMutableArrayRef items;
			CFStringRef combined;

			items = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			CFArrayApplyFunction(obj, CFRangeMake(0, CFArrayGetCount(obj)), _format_array, items);
			combined = CFStringCreateByCombiningStrings(NULL, items, CFSTR(", "));
			CFRelease(items);
			result = CFStringCreateWithFormat(NULL, NULL, CFSTR("[%@]"), combined);
			CFRelease(combined);
		}
	} else {
		result = CFStringCreateWithFormat(NULL, NULL, CFSTR("'%@'"), obj);
	}

	return result;
}
