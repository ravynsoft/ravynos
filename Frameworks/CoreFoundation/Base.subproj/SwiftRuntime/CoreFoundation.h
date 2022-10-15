// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//

/*
    This version of CoreFoundation.h is for the "Swift Runtime" mode of CF only.
 
    Note: The contents of this file are only meant for compiling the Swift Foundation module. The library is not ABI or API stable and is not meant to be used as a general-purpose C library on Linux.
 
*/

#if !defined(__COREFOUNDATION_COREFOUNDATION__)
#define __COREFOUNDATION_COREFOUNDATION__ 1
#define __COREFOUNDATION__ 1

#define DEPLOYMENT_RUNTIME_SWIFT 1

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

#if __has_include(<netdb.h>)
#include <netdb.h> // for Host.swift
#endif

#if __has_include(<ifaddrs.h>) && !defined(__wasi__)
#include <ifaddrs.h> // for Host.swift
#endif

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

#if !TARGET_OS_WASI
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFPlugIn.h>
#include <CoreFoundation/CFMessagePort.h>
#include <CoreFoundation/CFPreferences.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreFoundation/CFStream.h>
#include <CoreFoundation/CFSocket.h>
#include <CoreFoundation/CFMachPort.h>
#endif

#include <CoreFoundation/CFAttributedString.h>
#include <CoreFoundation/CFNotificationCenter.h>

#include <CoreFoundation/ForSwiftFoundationOnly.h>

#endif /* ! __COREFOUNDATION_COREFOUNDATION__ */

