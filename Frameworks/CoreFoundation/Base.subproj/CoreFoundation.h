/*	CoreFoundation.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_COREFOUNDATION__)
#define __COREFOUNDATION_COREFOUNDATION__ 1
#define __COREFOUNDATION__ 1

#if !defined(CF_EXCLUDE_CSTD_HEADERS)

#include <sys/types.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#if !defined(__wasi__)
#include <setjmp.h>
#endif
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__STDC_VERSION__) && (199901L <= __STDC_VERSION__)

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#endif

#endif

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFBag.h>
#include <CoreFoundation/CFBinaryHeap.h>
#include <CoreFoundation/CFBitVector.h>
#include <CoreFoundation/CFByteOrder.h>
#include <CoreFoundation/CFCalendar.h>
#include <CoreFoundation/CFCharacterSet.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFDateFormatter.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFError.h>
#include <CoreFoundation/CFLocale.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFNumberFormatter.h>
#if !TARGET_OS_WASI
#include <CoreFoundation/CFPreferences.h>
#endif
#include <CoreFoundation/CFPropertyList.h>
#include <CoreFoundation/CFSet.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingExt.h>
#include <CoreFoundation/CFTimeZone.h>
#include <CoreFoundation/CFTree.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFURLAccess.h>
#include <CoreFoundation/CFUUID.h>
#include <CoreFoundation/CFUtilities.h>
#include <CoreFoundation/CFBundle.h>

#if TARGET_OS_OSX || TARGET_OS_IPHONE || TARGET_OS_WIN32
#include <CoreFoundation/CFMessagePort.h>
#include <CoreFoundation/CFPlugIn.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFStream.h>
#include <CoreFoundation/CFSocket.h>
#include <CoreFoundation/CFMachPort.h>

#include <CoreFoundation/CFAttributedString.h>
#include <CoreFoundation/CFNotificationCenter.h>

#endif

#if TARGET_OS_OSX || TARGET_OS_IPHONE
#endif

#include <CoreFoundation/CFUserNotification.h>

#if !DEPLOYMENT_RUNTIME_SWIFT
#include <CoreFoundation/CFXMLNode.h>
#include <CoreFoundation/CFXMLParser.h>
#endif

#endif /* ! __COREFOUNDATION_COREFOUNDATION__ */

