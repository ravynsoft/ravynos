/*
	CFStringEncodingDatabase.h
	CoreFoundation
 
	Copyright (c) 2007-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#include <CoreFoundation/CFString.h>

CF_PRIVATE uint16_t __CFStringEncodingGetWindowsCodePage(CFStringEncoding encoding);
CF_PRIVATE CFStringEncoding __CFStringEncodingGetFromWindowsCodePage(uint16_t codepage);

CF_PRIVATE bool __CFStringEncodingGetCanonicalName(CFStringEncoding encoding, char *buffer, CFIndex bufferSize);
CF_PRIVATE CFStringEncoding __CFStringEncodingGetFromCanonicalName(const char *canonicalName);

CF_PRIVATE CFStringEncoding __CFStringEncodingGetMostCompatibleMacScript(CFStringEncoding encoding);

CF_PRIVATE const char *__CFStringEncodingGetName(CFStringEncoding encoding); // Returns simple non-localizd name
