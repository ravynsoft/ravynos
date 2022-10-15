/*	CFURLComponents_Internal.h
	Copyright (c) 2015-2019, Apple Inc. All rights reserved.
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#ifndef CFURLComponents_Internal_h
#define CFURLComponents_Internal_h

#include <CoreFoundation/CFString.h>

// _URIParseInfo keeps track of what parts of a parsed uriReference are
// present and the offset of where they start in the uriReference.
struct _URIParseInfo {
    // schemeOffset is not needed because if there is a scheme, it is always at offset 0.
    unsigned long	userinfoNameOffset;
    unsigned long	userinfoPasswordOffset;
    unsigned long	hostOffset;
    unsigned long	portOffset;
    unsigned long	pathOffset;
    unsigned long	queryOffset;
    unsigned long	fragmentOffset;
    unsigned long	endOffset;
    
    // set if the URI component/subcomponent exists
    unsigned long	schemeExists            : 1;
    unsigned long	authorityExists         : 1;    // this tells us if the hier-part of relative-part was in the form: "//" authority path-abempty
    unsigned long	userinfoNameExists      : 1;
    unsigned long	userinfoPasswordExists	: 1;
    unsigned long	hostExists              : 1;
    unsigned long	portExists              : 1;
    // pathExists is not needed because there's always a path... it just might be zero length.
    unsigned long	semicolonInPathExists   : 1;    // param is obsolete, but we still percent-encode the ';' for backwards compatiblity with NSURL/CFURL.
    unsigned long	queryExists             : 1;
    unsigned long	fragmentExists          : 1;
};

typedef CF_ENUM(unsigned short, _CFURIParserURLAllowedCharacter) {
    kURLSchemeAllowed	= 0x0001,
    kURLUserAllowed	= 0x0002,
    kURLPasswordAllowed	= 0x0004,
    kURLHostAllowed	= 0x0008,
    kURLPortAllowed	= 0x0010,
    kURLPathAllowed	= 0x0020,
    kURLQueryAllowed	= 0x0040,
    kURLFragmentAllowed	= 0x0080,
    kURLHexDigAllowed	= 0x0100,
    kURLAlphaAllowed	= 0x0200,
    kURLQueryItemNameAllowed = 0x0400, // the same as kURLQueryAllowed only '=' and '&' are not allowed
};

CF_PRIVATE Boolean _CFURIParserParseURIReference(CFStringRef urlString, struct _URIParseInfo *parseInfo);
CF_PRIVATE CFRange _CFURIParserGetSchemeRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE CFRange _CFURIParserGetUserinfoNameRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE CFRange _CFURIParserGetUserinfoPasswordRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE CFRange _CFURIParserGetHostRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE CFRange _CFURIParserGetPortRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE CFRange _CFURIParserGetPathRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE CFRange _CFURIParserGetQueryRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE CFRange _CFURIParserGetFragmentRange(const struct _URIParseInfo *parseInfo, Boolean includeSeparators);
CF_PRIVATE Boolean _CFURIParserAlphaAllowed(UniChar ch);
CF_PRIVATE Boolean _CFURIParserValidateComponent(CFStringRef urlString, CFRange componentRange, _CFURIParserURLAllowedCharacter allowedMask, Boolean pctEncodedAllowed);
CF_PRIVATE Boolean _CFURIParserURLStringIsValid(CFStringRef urlString, struct _URIParseInfo *parseInfo);

#endif /* CFURLComponents_Internal_h */
