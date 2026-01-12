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

/*	CFPropertyList.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/

#if !defined(__COREFOUNDATION_CFPROPERTYLIST__)
#define __COREFOUNDATION_CFPROPERTYLIST__ 1

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFData.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFError.h>
#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_EMBEDDED
#include <CoreFoundation/CFStream.h>
#endif

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

typedef CF_OPTIONS(CFOptionFlags, CFPropertyListMutabilityOptions) {
    kCFPropertyListImmutable = 0,
    kCFPropertyListMutableContainers,
    kCFPropertyListMutableContainersAndLeaves
};

CF_IMPLICIT_BRIDGING_DISABLED

/*
	Creates a property list object from its XML description; xmlData should
	be the raw bytes of that description, possibly the contents of an XML
	file. Returns NULL if the data cannot be parsed; if the parse fails
	and errorString is non-NULL, a human-readable description of the failure
	is returned in errorString. It is the caller's responsibility to release
	either the returned object or the error string, whichever is applicable.
 
        This function is deprecated. See CFPropertyListCreateWithData() for a replacement.
*/
CF_EXPORT
CFPropertyListRef CFPropertyListCreateFromXMLData(CFAllocatorRef allocator, CFDataRef xmlData, CFOptionFlags mutabilityOption, CFStringRef *errorString) CF_DEPRECATED(10_0, 10_10, 2_0, 8_0, "Use CFPropertyListCreateWithData instead.");

/*
	Returns the XML description of the given object; propertyList must
	be one of the supported property list types, and (for composite types
	like CFArray and CFDictionary) must not contain any elements that
	are not themselves of a property list type. If a non-property list
	type is encountered, NULL is returned. The returned data is
	appropriate for writing out to an XML file. Note that a data, not a
	string, is returned because the bytes contain in them a description
	of the string encoding used.
 
        This function is deprecated. See CFPropertyListCreateData() for a replacement.
*/
CF_EXPORT
CFDataRef CFPropertyListCreateXMLData(CFAllocatorRef allocator, CFPropertyListRef propertyList) CF_DEPRECATED(10_0, 10_10, 2_0, 8_0, "Use CFPropertyListCreateData instead.");

CF_IMPLICIT_BRIDGING_ENABLED

/*
	Recursively creates a copy of the given property list (so nested arrays
	and dictionaries are copied as well as the top-most container). The
	resulting property list has the mutability characteristics determined
	by mutabilityOption.
*/
CF_EXPORT
CFPropertyListRef CFPropertyListCreateDeepCopy(CFAllocatorRef allocator, CFPropertyListRef propertyList, CFOptionFlags mutabilityOption);

typedef CF_ENUM(CFIndex, CFPropertyListFormat) {
    kCFPropertyListOpenStepFormat = 1,
    kCFPropertyListXMLFormat_v1_0 = 100,
    kCFPropertyListBinaryFormat_v1_0 = 200
};

/* Returns true if the object graph rooted at plist is a valid property list
 * graph -- that is, no cycles, containing only plist objects, and dictionary
 * keys are strings. The debugging library version spits out some messages
 * to be helpful. The plist structure which is to be allowed is given by
 * the format parameter. */
CF_EXPORT
Boolean CFPropertyListIsValid(CFPropertyListRef plist, CFPropertyListFormat format);

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_EMBEDDED
CF_IMPLICIT_BRIDGING_DISABLED

/* Writes the bytes of a plist serialization out to the stream.  The
 * stream must be opened and configured -- the function simply writes
 * a bunch of bytes to the stream. The output plist format can be chosen.
 * Leaves the stream open, but note that reading a plist expects the
 * reading stream to end wherever the writing ended, so that the
 * end of the plist data can be identified. Returns the number of bytes
 * written, or 0 on error. Error messages are not currently localized, but
 * may be in the future, so they are not suitable for comparison. 
 *
 * This function is deprecated. See CFPropertyListWrite() for a replacement. */
CF_EXPORT
CFIndex CFPropertyListWriteToStream(CFPropertyListRef propertyList, CFWriteStreamRef stream, CFPropertyListFormat format, CFStringRef *errorString) CF_DEPRECATED(10_2, 10_10, 2_0, 8_0, "Use CFPropertyListWrite instead.");


/* Same as current function CFPropertyListCreateFromXMLData()
 * but takes a stream instead of data, and works on any plist file format.
 * CFPropertyListCreateFromXMLData() also works on any plist file format.
 * The stream must be open and configured -- the function simply reads a bunch
 * of bytes from it starting at the current location in the stream, to the END
 * of the stream, which is expected to be the end of the plist, or up to the
 * number of bytes given by the length parameter if it is not 0. Error messages
 * are not currently localized, but may be in the future, so they are not
 * suitable for comparison. 
 *
 * This function is deprecated. See CFPropertyListCreateWithStream() for a replacement. */
CF_EXPORT
CFPropertyListRef CFPropertyListCreateFromStream(CFAllocatorRef allocator, CFReadStreamRef stream, CFIndex streamLength, CFOptionFlags mutabilityOption, CFPropertyListFormat *format, CFStringRef *errorString) CF_DEPRECATED(10_2, 10_10, 2_0, 8_0, "Use CFPropertyListCreateWithStream instead.");

CF_IMPLICIT_BRIDGING_ENABLED
#endif

CF_IMPLICIT_BRIDGING_DISABLED

enum {
    kCFPropertyListReadCorruptError = 3840,              // Error parsing a property list
    kCFPropertyListReadUnknownVersionError = 3841,       // The version number in the property list is unknown
    kCFPropertyListReadStreamError = 3842,               // Stream error reading a property list
    kCFPropertyListWriteStreamError = 3851,              // Stream error writing a property list
} CF_ENUM_AVAILABLE(10_6, 4_0);

/* Create a property list with a CFData input. If the format parameter is non-NULL, it will be set to the format of the data after parsing is complete. The options parameter is used to specify CFPropertyListMutabilityOptions. If an error occurs while parsing the data, the return value will be NULL. Additionally, if an error occurs and the error parameter is non-NULL, the error parameter will be set to a CFError describing the problem, which the caller must release. If the parse succeeds, the returned value is a reference to the new property list. It is the responsibility of the caller to release this value.
 */
CF_EXPORT
CFPropertyListRef CFPropertyListCreateWithData(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags options, CFPropertyListFormat *format, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);

#if TARGET_OS_MAC || TARGET_OS_WIN32 || TARGET_OS_EMBEDDED

/* Create and return a property list with a CFReadStream input. If the format parameter is non-NULL, it will be set to the format of the data after parsing is complete. The options parameter is used to specify CFPropertyListMutabilityOptions. The streamLength parameter specifies the number of bytes to read from the stream. Set streamLength to 0 to read until the end of the stream is detected. If an error occurs while parsing the data, the return value will be NULL. Additionally, if an error occurs and the error parameter is non-NULL, the error parameter will be set to a CFError describing the problem, which the caller must release. If the parse succeeds, the returned value is a reference to the new property list. It is the responsibility of the caller to release this value.
 */
CF_EXPORT
CFPropertyListRef CFPropertyListCreateWithStream(CFAllocatorRef allocator, CFReadStreamRef stream, CFIndex streamLength, CFOptionFlags options, CFPropertyListFormat *format, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);

/* Write the bytes of a serialized property list out to a stream. The stream must be opened and configured. The format of the property list can be chosen with the format parameter. The options parameter is currently unused and should be set to 0. The return value is the number of bytes written or 0 in the case of an error. If an error occurs and the error parameter is non-NULL, the error parameter will be set to a CFError describing the problem, which the caller must release.
 */
CF_EXPORT
CFIndex CFPropertyListWrite(CFPropertyListRef propertyList, CFWriteStreamRef stream, CFPropertyListFormat format, CFOptionFlags options, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);

#endif

/* Create a CFData with the bytes of a serialized property list. The format of the property list can be chosen with the format parameter. The options parameter is currently unused and should be set to 0. If an error occurs while parsing the data, the return value will be NULL. Additionally, if an error occurs and the error parameter is non-NULL, the error parameter will be set to a CFError describing the problem, which the caller must release. If the conversion succeeds, the returned value is a reference to the created data. It is the responsibility of the caller to release this value.
 */
CF_EXPORT
CFDataRef CFPropertyListCreateData(CFAllocatorRef allocator, CFPropertyListRef propertyList, CFPropertyListFormat format, CFOptionFlags options, CFErrorRef *error) CF_AVAILABLE(10_6, 4_0);

CF_IMPLICIT_BRIDGING_ENABLED

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFPROPERTYLIST__ */

