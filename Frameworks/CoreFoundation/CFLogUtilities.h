/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFLogUtilities.h
	Copyright (c) 2004-2014, Apple Inc. All rights reserved.
*/

/*
        APPLE SPI:  NOT TO BE USED OUTSIDE APPLE!
*/

#if !defined(__COREFOUNDATION_CFLOGUTILITIES__)
#define __COREFOUNDATION_CFLOGUTILITIES__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

CF_EXTERN_C_BEGIN


enum {	// Legal level values for CFLog()
    kCFLogLevelEmergency = 0,
    kCFLogLevelAlert = 1,
    kCFLogLevelCritical = 2,
    kCFLogLevelError = 3,
    kCFLogLevelWarning = 4,
    kCFLogLevelNotice = 5,
    kCFLogLevelInfo = 6,
    kCFLogLevelDebug = 7,
};

CF_EXPORT void CFLog(int32_t level, CFStringRef format, ...);
/*	Passing in a level value which is outside the range of 0-7 will cause the the call to do nothing.
	CFLog() logs the message using the asl.h API, and uses the level parameter as the log level.
	Note that the asl subsystem ignores some log levels by default.
	CFLog() is not fast, and is not going to be guaranteed to be fast.
	Even "no-op" CFLogs are not necessarily fast.
	If you care about performance, you shouldn't be logging.
*/

CF_EXTERN_C_END

#endif /* ! __COREFOUNDATION_CFLOGUTILITIES__ */

