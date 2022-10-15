/*	CFStringDefaultEncoding.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFSTRINGDEFAULTENCODING__)
#define __COREFOUNDATION_CFSTRINGDEFAULTENCODING__ 1

#include <CoreFoundation/CFBase.h>

#if TARGET_OS_OSX || TARGET_OS_IPHONE
#include <stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/param.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <xlocale.h>

CF_EXTERN_C_BEGIN

#define __kCFUserEncodingEnvVariableName ("__CF_USER_TEXT_ENCODING")
#define __kCFMaxDefaultEncodingFileLength (64)
#define __kCFUserEncodingFileName ("/.CFUserTextEncoding")

CF_EXPORT void _CFStringGetUserDefaultEncoding(UInt32 *oScriptValue, UInt32 *oRegionValue);
CF_EXPORT void _CFStringGetInstallationEncodingAndRegion(uint32_t *encoding, uint32_t *region);
CF_EXPORT Boolean _CFStringSaveUserDefaultEncoding(UInt32 iScriptValue, UInt32 iRegionValue);

CF_INLINE void __CFStringGetUserDefaultEncoding(UInt32 *oScriptValue, UInt32 *oRegionValue) { _CFStringGetUserDefaultEncoding(oScriptValue, oRegionValue); }
CF_INLINE void __CFStringGetInstallationEncodingAndRegion(uint32_t *encoding, uint32_t *region) { _CFStringGetInstallationEncodingAndRegion(encoding, region); }
CF_INLINE void __CFStringSaveUserDefaultEncoding(UInt32 iScriptValue, UInt32 iRegionValue) { _CFStringSaveUserDefaultEncoding(iScriptValue, iRegionValue); }

CF_EXTERN_C_END

#endif

#endif /* ! __COREFOUNDATION_CFSTRINGDEFAULTENCODING__ */


