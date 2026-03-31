/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
/*
 *  CFHTTPAuthentication.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on Tue Feb 10 2004.
 *  Copyright 2004, Apple, Inc. All rights reserved.
 *
 */

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFPriv.h>
#include <CFNetwork/CFHTTPMessage.h>
#include <CFNetwork/CFHTTPMessagePriv.h>
#include <CFNetwork/CFHTTPStreamPriv.h>
#include "CFNetworkInternal.h"
#include "CFHTTPInternal.h"
#include <CFNetwork/CFHTTPStream.h>
#include <security_cdsa_utils/cuEnc64.h>

#if defined(__MACH__)
#include <pthread.h>
#include <CommonCrypto/CommonDigest.h>
#include <mach-o/dyld.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "spnegoBlob.h"
//#include "spnegoDER.h"
#include "spnegoKrb.h"

#include "NTLM/NtlmGenerator.h"
#endif // __MACH__

#if defined(__WIN32__)
// allows code using inet_addr() to port easily to Win32
typedef unsigned long in_addr_t;
#endif

// The Win32 versions of support for SPNEGO, NTLM, MD5 are in CFHTTPAuth-Win32.c

/*
	CFHTTPAuthentication ends up being a somewhat complex beast.  For simple authentication
	types, it is fairly straight forward.  Each CFHTTPAuthentication object represents the
	required bits needed for performing HTTP authentication against a given domain.  Domain
	here refers to the authentication domain and not necessarily a hostname. Once a 401 or
	a 407 is encountered, the authentication headers are parsed for all authentication types
	and their respective data.  Once parsed, a preferred scheme is chosen.  The order in
	which schemes are chosen is based upon a mix of security and IE compatability.  Currently,
	the order of preference is Negotiate, NTLM, Digest, and then Basic.  NTLM is chosen before
	Digest for two reasons.  The first is that this is what IE does.  The second is that
	even though IIS servers will return Digest most are not actually set up correctly to
	perform it.  Once Digest is turned on, the administrator will have to re-assign passwords
	or only accounts created after Digest was turned on will work.  Once the preferred choice
	has been made, the authentication object is ready for use.
 
	Application of credentials for Basic and Digest authentication will immediately place
	the authorization in the headers of the request.  For NTLM and Negotiate, only the
	association of the authentication object and the request are made.  The actual header
	is not/should not be placed on the request object until the request is to be sent.  In
	these cases, a connection reference is required in order to perform multi-leg authentication
	should the protocol require it.  A single authentication object holds onto the multi-leg
	information by mapping a supplied connection identifier to the state of the transaction.
	This allows a single authentication object to be used across multiple, simultaneous
	connections.  This also means that should any one connection fail the full transaction,
	the entire object will be invalidated.
	
	This brings up the point that any authentication object is basically good until the
	server says that it is bad.  Any connection drops or connection failures will be handled
	in an automatic nature under the covers.  Users of _CFHTTPAuthenticationApplyHeaderToRequest
	should make sure to call _CFHTTPAuthenticationDisassociateConnection in order to keep a
	trimmed list of mappings too.
 
	All automatic carry-over of data from response to the authentication object occurs as a
	result of calling _CFHTTPAuthenticationUpdateFromResponse.  This function looks at the server's
	response and makes the needed adjustments to the authentication object.  It will parse all
	the authentication headers, but only the preferred scheme's data will be carried, therefore
	once authentication has started with an object, it's preferred scheme should not be changed.
 */

#if 0
#pragma mark -
#pragma mark Constant Strings
#endif

// Windows' "username" for forcing use of "single sign-on" path
CONST_STRING_DECL(_kCFStreamSingleSignOnUserName, "Single Sign-On")

// Keys for dictionary for applying credentials.
CONST_STRING_DECL(kCFHTTPAuthenticationAccountDomain, "kCFHTTPAuthenticationAccountDomain")
CONST_STRING_DECL(kCFHTTPAuthenticationPassword, "kCFHTTPAuthenticationPassword")
CONST_STRING_DECL(kCFHTTPAuthenticationUsername, "kCFHTTPAuthenticationUsername")

// HTTP headers that contain authentication information
#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHTTPMessageHeaderWWWAuthenticate			CFSTR("WWW-Authenticate")
#define	_kCFHTTPMessageHeaderProxyAuthenticate			CFSTR("Proxy-Authenticate")
#define _kCFHTTPMessageHeaderAuthenticationInfo			CFSTR("Authentication-Info")
#define _kCFHTTPMessageHeaderProxyAuthenticationInfo	CFSTR("Proxy-Authentication-Info")
#define _kCFHTTPMessageHeaderProxyAuthorization			CFSTR("Proxy-Authorization")
#define _kCFHTTPMessageHeaderAuthorization				CFSTR("Authorization")
#else
static CONST_STRING_DECL(_kCFHTTPMessageHeaderWWWAuthenticate, "WWW-Authenticate")
static CONST_STRING_DECL(_kCFHTTPMessageHeaderProxyAuthenticate, "Proxy-Authenticate")
static CONST_STRING_DECL(_kCFHTTPMessageHeaderAuthenticationInfo, "Authentication-Info")
static CONST_STRING_DECL(_kCFHTTPMessageHeaderProxyAuthenticationInfo, "Proxy-Authentication-Info")
static CONST_STRING_DECL(_kCFHTTPMessageHeaderProxyAuthorization, "Proxy-Authorization")
static CONST_STRING_DECL(_kCFHTTPMessageHeaderAuthorization, "Authorization")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Keys used for authentication schemes
#ifdef __CONSTANT_CFSTRINGS__
#define _kCFHTTPAuthenticationPropertyPreferredScheme	CFSTR("_kCFHTTPAuthenticationPropertyPreferredScheme")
#define _kCFHTTPAuthenticationPropertyAuthenticateType	CFSTR("_kCFHTTPAuthenticationPropertyAuthenticateType")
#define kCFHTTPAuthenticationPropertyMethod				CFSTR("kCFHTTPAuthenticationPropertyMethod")
#else
static CONST_STRING_DECL(_kCFHTTPAuthenticationPropertyPreferredScheme, "_kCFHTTPAuthenticationPropertyPreferredScheme")
static CONST_STRING_DECL(_kCFHTTPAuthenticationPropertyAuthenticateType, "_kCFHTTPAuthenticationPropertyAuthenticateType")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyMethod, "kCFHTTPAuthenticationPropertyMethod")
#endif	/* __CONSTANT_CFSTRINGS__ */

// Values for the Method prop.  Note we will use these as atoms (tested for with ==) when they are
// coming out of a scheme dict.
CONST_STRING_DECL(kCFHTTPAuthenticationSchemeBasic, "Basic")
CONST_STRING_DECL(kCFHTTPAuthenticationSchemeDigest, "Digest")
CONST_STRING_DECL(kCFHTTPAuthenticationSchemeNegotiate, "Negotiate")
CONST_STRING_DECL(kCFHTTPAuthenticationSchemeNTLM, "NTLM")

// Parts of authentication information sent by the server.
#ifdef __CONSTANT_CFSTRINGS__
#define kCFHTTPAuthenticationPropertyRealm				CFSTR("Realm")
#define kCFHTTPAuthenticationPropertyDomain				CFSTR("Domain")
#define _kCFHTTPAuthenticationPropertyDigestStale		CFSTR("Stale")
#define _kCFHTTPAuthenticationDigestStaleTrue			CFSTR("True")
#define kCFHTTPAuthenticationPropertyDigestNonce		CFSTR("Nonce")
#define kCFHTTPAuthenticationPropertyDigestNextNonce	CFSTR("Nextnonce")
#define kCFHTTPAuthenticationPropertyDigestNonceCount	CFSTR("Nc")
#define kCFHTTPAuthenticationPropertyDigestQop			CFSTR("Qop")
#define kCFHTTPAuthenticationDigestQopAuth				CFSTR("auth")
#define kCFHTTPAuthenticationPropertyDigestCNonce		CFSTR("Cnonce")
#define kCFHTTPAuthenticationPropertyDigestOpaque		CFSTR("Opaque")
#define kCFHTTPAuthenticationPropertyDigestAlgorithm	CFSTR("Algorithm")
#define kCFHTTPAuthenticationDigestAlgorithmMD5			CFSTR("MD5")
#define kCFHTTPAuthenticationDigestAlgorithmMD5Session	CFSTR("MD5-sess")
#define kCFHTTPAuthenticationPropertyNegotiateAuthData	CFSTR("Auth-Data")
#define kCFHTTPAuthenticationComma						CFSTR(",")
#else
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyRealm, "Realm")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDomain, "Domain")
static CONST_STRING_DECL(_kCFHTTPAuthenticationPropertyDigestStale, "Stale")
static CONST_STRING_DECL(_kCFHTTPAuthenticationDigestStaleTrue, "True")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDigestNonce, "Nonce")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDigestNextNonce, "Nextnonce")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDigestNonceCount, "Nc"
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDigestQop, "Qop")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestQopAuth, "auth")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDigestCNonce, "Cnonce")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDigestOpaque, "Opaque")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyDigestAlgorithm, "Algorithm")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestAlgorithmMD5, "MD5")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestAlgorithmMD5Session, "MD5-sess")
static CONST_STRING_DECL(kCFHTTPAuthenticationPropertyNegotiateAuthData, "Auth-Data")
static CONST_STRING_DECL(kCFHTTPAuthenticationComma, ",")
#endif	/* __CONSTANT_CFSTRINGS__ */

#ifdef __CONSTANT_CFSTRINGS__
#define kHTTPAuthenticationUndecidedMethodDescription				CFSTR("<undecided>")
#define kHTTPAuthenticationDescriptionFormat	CFSTR("<CFHTTPAuthentication 0x%x>{state = %s; scheme = %@, forProxy = %s}")
#else
static CONST_STRING_DECL(kHTTPAuthenticationUndecidedMethodDescription, "<undecided>")
static CONST_STRING_DECL(kHTTPAuthenticationDescriptionFormat, "<CFHTTPAuthentication 0x%x>{state = %s; scheme = %@, forProxy = %s}")
#endif	/* __CONSTANT_CFSTRINGS__ */
	
// Basic authentication strings
#ifdef __CONSTANT_CFSTRINGS__
#define kCFHTTPAuthenticationBasicFormat		CFSTR("Basic %@")
#else
static CONST_STRING_DECL(kCFHTTPAuthenticationBasicFormat, "Basic %@")
#endif	/* __CONSTANT_CFSTRINGS__ */
						 
// Digest authentication strings
#ifdef __CONSTANT_CFSTRINGS__
#define kCFHTTPAuthenticationHTTPSScheme					CFSTR("https")
#define kCFHTTPAuthenticationCONNECTMethod					CFSTR("CONNECT")
#define kCFHTTPAuthenticationHostPortFormat					CFSTR("%@:%d")
#define kCFHTTPAuthenticationMD5HashFormat					CFSTR("%lx")
#define	kCFHTTPAuthenticationDigestHashA1Format				CFSTR("%@:%@:%@")
#define kCFHTTPAuthenticationDigestHashA2NoQopFormat		CFSTR("%@:%@")
#define kCFHTTPAuthenticationDigestHashFormat				CFSTR("%@:%@:%@")
#define kCFHTTPAuthenticationDigestHashQopFormat			CFSTR("%@:%@:%08lx:%@:%@:%@")
#define kCFHTTPAuthenticationDigestHeaderFormat				CFSTR("%@ username=\"%@\", realm=\"%@\", nonce=\"%@\", uri=\"%@\", response=\"%@\"")
#define kCFHTTPAuthenticationDigestHeaderOpaqueFormat		CFSTR(", opaque=\"%@\"")
#define kCFHTTPAuthenticationDigestHeaderAlgorithmFormat	CFSTR(", algorithm=\"%@\"")
#define kCFHTTPAuthenticationDigestHeaderNoncesFormat		CFSTR(", cnonce=\"%@\", nc=%08lx, qop=\"%@\"")
#else
static CONST_STRING_DECL(kCFHTTPAuthenticationHTTPSScheme, "https")
static CONST_STRING_DECL(kCFHTTPAuthenticationCONNECTMethod, "CONNECT")
static CONST_STRING_DECL(kCFHTTPAuthenticationHostPortFormat, "%@:%d")
static CONST_STRING_DECL(kCFHTTPAuthenticationMD5HashFormat, "%lx")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHashA1Format, "%@:%@:%@")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHashA2NoQopFormat, "%@:%@")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHashFormat, "%@:%@:%@")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHashQopFormat, "%@:%@:%08lx:%@:%@:%@")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHeaderFormat, "%@ username=\"%@\", realm=\"%@\", nonce=\"%@\", uri=\"%@\", response=\"%@\"")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHeaderOpaqueFormat, ", opaque=\"%@\"")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHeaderAlgorithmFormat, ", algorithm=\"%@\"")
static CONST_STRING_DECL(kCFHTTPAuthenticationDigestHeaderNoncesFormat, ", cnonce=\"%@\", nc=%08lx, qop=\"%@\"")
#endif	/* __CONSTANT_CFSTRINGS__ */
						 
#ifdef __CONSTANT_CFSTRINGS__
#define kCFHTTPAuthenticationNegotiateNegotiateFormat	CFSTR("Negotiate %@")
#define kCFHTTPAuthenticationNegotiateNTLMFormat		CFSTR("NTLM %@")
#define kCFHTTPAuthenticationNTLMDomainUserSeparator	CFSTR("\\")
#else
static CONST_STRING_DECL(kCFHTTPAuthenticationNegotiateNegotiateFormat, "Negotiate %@")
static CONST_STRING_DECL(kCFHTTPAuthenticationNegotiateNTLMFormat, "NTLM %@")
static CONST_STRING_DECL(kCFHTTPAuthenticationNTLMDomainUserSeparator, "\\")
#endif	/* __CONSTANT_CFSTRINGS__ */

						 
#if 0
#pragma mark -
#pragma mark CFHTTPAuthentication Base Type
#endif

struct _CFHTTPAuthentication {

    CFRuntimeBase			_base;

    _CFMutex				_lock;

    CFStreamError			_error;
	
	CFStringRef				_user;			// Currently only used for ntlm
	CFStringRef				_domain;		// Currently only used for ntlm
	CFDataRef				_hash[2];		// Currently only used for ntlm (1st is ntlm hash, 2nd is lm hash)

    CFMutableDictionaryRef	_preferred;		// non-retained pointer to one of the schemes
    CFMutableDictionaryRef	_schemes;		// scheme props keyed by scheme name
	CFMutableDictionaryRef	_connections;	// Mapping of connection id to connection specific state information

	Boolean					_proxy;			// Authentication is for a proxy

#ifdef __WIN32__
    _CFSSPIStateRef			_sspi;			// state needed to use Win32 SSPI routines
#endif
};


#if 0
#pragma mark -
#pragma mark Other Types
#endif

/*
	The use of the following structure is fairly straight forward for Negotiate authentication.
	Only the _negotiation field is used.  Currently, SPNEGO is not doing multi-leg negotiation,
	so _authdata is not used.
 
	For NTLM, usage is actually fairly complex (more so than you would think).  When authentication
	on a connection starts, all fields are NULL (state 1).  Upon application of the first round of
	credentials (state 2), the _ntlm field will hold the object used for encoding and decoding the
	NTLM blobs.  At this same time, _negotiation will get filled with the first response to be sent
	to the server.  The server should respond with authentication data which is saved in _authdata
	(state 3).  At this point, all three fields are filled and the authentication is halfway done.
 
	When the call to apply credentials again, the username and password will actually be used to
	create the final NTLM hash to be sent to the server.  The new blob will be saved in _negotation,
	the NTLM generator will be thrown out, and _ntlm will be set to NULL (state 4).  When the actual
	authorization header is placed on the next outgoing connection, the _authdata is set to NULL
	and _negotiate is left as is (state 5).  This is done in order to signal the final header has
	gone out.  _negotiate will not be placed on any more outgoing requests.  On a successful reply,
	the _negotiate header will be moved to _authdata and _negotiate will be set to NULL to indicate
	a successful use of NTLM.
 
	NTLM here is a little confusing this chart should help a little (0 indicates field is NULL and
	1 indicates non-NULL; values are in structure order):
 
	0 0 0	<- start
	1 0	1	<- successful call of ApplyCredentials
	1 1 1	<- received Auth-Data response from server and saved
	0 1 1	<- successful second call of ApplyCredentials
	0 0 1	<- last leg attempt is being made
	0 1 0	<- successful result from server (non-401 or 407)
 
	At this point, the authorization header will not be applied to the outgoing request although
	the user will continue to apply the authentication to the requests.
*/
typedef struct {
	NtlmGeneratorRef	_ntlm;				// NTLM object used for creating hashes for auth
	CFStringRef			_authdata;			// Auth-Data received from the server
	CFStringRef			_negotiation;		// Final negotiation hash to be sent to the server
} _AuthConnectionSpecific;


#if 0
#pragma mark -
#pragma mark Extern Function Declarations
#endif

extern Boolean _CFHTTPAuthenticationConnectionAuthenticated(CFHTTPAuthenticationRef auth, const void* connection);

	
#if 0
#pragma mark -
#pragma mark Static Function Declarations
#endif

static void _HTTPAuthenticationDestroy(CFHTTPAuthenticationRef auth);
static CFStringRef _HTTPAuthenticationDescribe(CFHTTPAuthenticationRef auth);
static void _HTTPAuthenticationRegisterClass(void);

static CFStringRef _canonicalSchemeName(CFStringRef scheme);
static UInt8* _CFHTTPAuthenticationParseToken(CFAllocatorRef alloc, UInt8* buffer, CFStringRef* token);
static UInt8* _CFHTTPAuthenticationSkipLWS(UInt8* buffer);
static UInt8* _CFHTTPAuthenticationSkipToLWS(UInt8* buffer);
static UInt8* _CFHTTPAuthenticationParseQuoted(CFAllocatorRef alloc, UInt8* buffer, CFStringRef* token);
static UInt8* _CFHTTPAuthenticationParseBase64(CFAllocatorRef alloc, UInt8* buffer, CFStringRef* token);

static void _CFHTTPAuthenticationParseDomains(CFHTTPAuthenticationRef auth, CFURLRef base);

static void _CFHTTPAuthenticationSetError(CFHTTPAuthenticationRef auth, CFStreamErrorDomain domain, SInt32 error);

static Boolean _CFHTTPAuthenticationParseHeader(CFStringRef headerValue, Boolean isInfoHeader, CFMutableDictionaryRef schemes);

static CFTypeRef _CFHTTPAuthenticationGetProperty(CFHTTPAuthenticationRef auth, CFStringRef propertyKey);
static CFTypeRef _CFHTTPAuthenticationLockAndCopyProperty(CFHTTPAuthenticationRef auth, CFStringRef propertyKey);

static CFStringRef _CFStringQuote(CFStringRef unquoted);
static CFStringRef _CFStringUnquote(CFStringRef quoted);

static Boolean _CFApplyCredentials_Unsafe(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth, CFDictionaryRef dict, CFStreamError* error);

static Boolean _CFHTTPMessageSetBasicAuthenticationOnRequest(CFHTTPMessageRef request, CFStringRef user, CFStringRef password, Boolean forProxy, CFStreamError* error);

static CFStringRef _CFStringCreateMD5HashWithString(CFAllocatorRef alloc, CFStringRef string);
static CFStringRef _CFStringCreateDigestHashA1(CFAllocatorRef alloc, CFHTTPAuthenticationRef auth, CFStringRef username, CFStringRef password);
static CFStringRef _CFStringCreateDigestHashA2(CFAllocatorRef alloc, CFHTTPAuthenticationRef auth, CFHTTPMessageRef request);
static CFStringRef _CFStringCreateDigestHash(CFAllocatorRef alloc, CFHTTPAuthenticationRef auth, CFStringRef a1, CFStringRef a2);
static CFStringRef _CFStringCreateDigestAuthenticationHeaderValueForRequest(CFAllocatorRef alloc, CFHTTPAuthenticationRef auth, CFHTTPMessageRef request, CFStringRef username, CFStringRef hash);

static Boolean _CFHTTPMessageSetDigestAuthenticationOnRequest(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth, CFStringRef username, CFStringRef password);

static Boolean _CFHTTPMessageSetNegotiateAuthenticationOnRequest(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth, CFStringRef username, CFStringRef password);
static CFStringRef _CFHTTPAuthenticationCreateNegotiateHeaderForRequest(CFHTTPAuthenticationRef auth, CFHTTPMessageRef request, const void* connection);

static Boolean _CFHTTPMessageSetNTLMAuthenticationOnRequest(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth, CFStringRef username, CFStringRef password, CFStringRef domain);
static CFStringRef _CFHTTPAuthenticationCreateNTLMHeaderForRequest(CFHTTPAuthenticationRef auth, CFHTTPMessageRef request, const void* connection);

static _AuthConnectionSpecific* _AuthConnectionSpecificRetain(CFAllocatorRef allocator, _AuthConnectionSpecific* specific);
static void _AuthConnectionSpecificRelease(CFAllocatorRef allocator, _AuthConnectionSpecific* specific);

#if defined(__MACH__)
static Boolean _CFMD5(const UInt8* d, UInt32 n, UInt8* md, UInt32 md_length);
//static Boolean _CFCanTryKerberos(void);
#endif // __MACH__


#if 0
#pragma mark -
#pragma mark Globals
#endif

static CFTypeID kHTTPAuthenticationTypeID = _kCFRuntimeNotATypeID;
static _CFOnceLock gHTTPAuthenticationClassRegistration = _CFOnceInitializer;


#if 0
#pragma mark -
#pragma mark CFRuntimeClass Methods
#endif

/* static */ void
_HTTPAuthenticationRegisterClass(void) {

    static const CFRuntimeClass HTTPAuthenticationClass = {
        0,																// version
        "CFHTTPAuthentication",											// name
        NULL,															// init
        NULL,															// copy
        (void(*)(CFTypeRef))&(_HTTPAuthenticationDestroy),				// finalize
        NULL,															// equal
        NULL,															// hash
        NULL,															// copy formatting description
        (CFStringRef(*)(CFTypeRef cf))&(_HTTPAuthenticationDescribe)	// copy debug description
    };

    kHTTPAuthenticationTypeID = _CFRuntimeRegisterClass(&HTTPAuthenticationClass);
}


/* static */ void
_HTTPAuthenticationDestroy(CFHTTPAuthenticationRef auth) {
	
	int i;
	
    _CFMutexDestroy(&auth->_lock);
    if (auth->_schemes)
        CFRelease(auth->_schemes);

	if (auth->_connections)
		CFRelease(auth->_connections);
	
	if (auth->_user)
		CFRelease(auth->_user);
	
	if (auth->_domain)
		CFRelease(auth->_domain);
	
	for (i = 0; i < (sizeof(auth->_hash) / sizeof(auth->_hash[0])); i++) {

		if (auth->_hash[i])
			CFRelease(auth->_hash[i]);
	}
	
#ifdef __WIN32__
    if (auth->_sspi)
        _CFFreeSSPIState(auth->_sspi);
#endif  /* __WIN32__ */
}


/* static */ CFStringRef
_HTTPAuthenticationDescribe(CFHTTPAuthenticationRef auth) {
    CFStringRef method = auth->_preferred ? (CFStringRef)CFDictionaryGetValue(auth->_preferred, kCFHTTPAuthenticationPropertyMethod) : kHTTPAuthenticationUndecidedMethodDescription;
    const char *forProxy = auth->_proxy ? "true" : "false";
    const char *state;

    if (auth->_error.error)
        state = "Failed";
    else
        state = "InProgress";

    return CFStringCreateWithFormat(NULL, NULL, kHTTPAuthenticationDescriptionFormat, auth, state, method, forProxy);
}


#if 0
#pragma mark -
#pragma mark Utilities
#endif

// This is a #define's instead of an inline so we get a line number reported from where they are used
#if defined(__WIN32__)
// since we punted pthreads on windows, all locks are recursive, so this assert no longer works
#define _CFAssertLocked(lock) assert(TRUE)
#else
#define _CFAssertLocked(lock) \
    assert(_CFMutexTryLock(lock) == FALSE)
#endif

#if defined(__MACH__)

//static Boolean _CFCanTryKerberos(void) {
//    return TRUE;
//}

#endif // __MACH__


/* static */
void _CFHTTPAuthenticationSetError(CFHTTPAuthenticationRef auth, CFStreamErrorDomain domain, SInt32 error) {

    _CFAssertLocked(&auth->_lock);

    auth->_error.error = error;
    auth->_error.domain = domain;

	CFDictionaryRemoveAllValues(auth->_connections);
}


/* static */ _AuthConnectionSpecific*
_AuthConnectionSpecificRetain(CFAllocatorRef allocator, _AuthConnectionSpecific* specific) {

	_AuthConnectionSpecific* result = (_AuthConnectionSpecific*)CFAllocatorAllocate(allocator, sizeof(specific), 0);
	
	memmove(result, specific, sizeof(result[0]));
	
	if (result->_negotiation)
		CFRetain(result->_negotiation);
	
	if (result->_authdata)
		CFRetain(result->_authdata);
	
	return result;
}


/* static */ void
_AuthConnectionSpecificRelease(CFAllocatorRef allocator, _AuthConnectionSpecific* specific) {
	
	if (specific->_negotiation)
		CFRelease(specific->_negotiation);
	
	if (specific->_authdata)
		CFRelease(specific->_authdata);
	
	if (specific->_ntlm)
		NtlmGeneratorRelease(specific->_ntlm);
	
	CFAllocatorDeallocate(allocator, specific);
}


#if 0
#pragma mark -
#pragma mark Header Parsing
#endif

/* static */
CFStringRef _canonicalSchemeName(CFStringRef scheme) {

    // If we ever get a ton of these you could lowercase scheme and look up the canonical
    // version in a dict.
    if (!CFStringCompare(scheme, kCFHTTPAuthenticationSchemeNegotiate, kCFCompareCaseInsensitive))
        return kCFHTTPAuthenticationSchemeNegotiate;
    else if (!CFStringCompare(scheme, kCFHTTPAuthenticationSchemeNTLM, kCFCompareCaseInsensitive))
        return kCFHTTPAuthenticationSchemeNTLM;
    else if (!CFStringCompare(scheme, kCFHTTPAuthenticationSchemeBasic, kCFCompareCaseInsensitive))
        return kCFHTTPAuthenticationSchemeBasic;
    else if (!CFStringCompare(scheme, kCFHTTPAuthenticationSchemeDigest, kCFCompareCaseInsensitive))
        return kCFHTTPAuthenticationSchemeDigest;
    else
        return CFRetain(scheme);
}


/* static */
UInt8* _CFHTTPAuthenticationParseToken(CFAllocatorRef alloc, UInt8* buffer, CFStringRef* token) {
	
	UInt8 *start = buffer;
	UInt8 c = *buffer;
	
	do {
	
		if (c & 0x80) break;								// Limit to CHAR (octets 0 - 127)
	
		if (c < 0x20) break;								// Don't allow CTL (octets 0 - 31)
		
        if (strchr("()<>@,;:\\\"[]?={} \t", c)) break;		// Don't allow separators

        buffer++;

        c = *buffer;
		
	} while (1);
    
	if (token)
		*token = ((buffer == start) ? NULL : CFStringCreateWithBytes(alloc, start, buffer - start, kCFStringEncodingISOLatin1, FALSE));
	
	return buffer;
}


/* static */
UInt8* _CFHTTPAuthenticationSkipLWS(UInt8* buffer) {
	
    while (*buffer && strchr(" \t\r\n", *buffer))
        buffer++;
	
	return buffer;
}


/* static */
UInt8* _CFHTTPAuthenticationSkipToLWS(UInt8* buffer) {
    
    while (*buffer && !strchr(" \t\r\n", *buffer))
        buffer++;
        
    return buffer;
}


/* static */
UInt8* _CFHTTPAuthenticationParseQuoted(CFAllocatorRef alloc, UInt8* buffer, CFStringRef* token) {

    UInt8* start = ++buffer;		// Skip '"'

    do {
        UInt8 c = *(buffer = _CFHTTPAuthenticationSkipLWS(buffer));

        if (c < 0x20) break; 	// Don't allow CTL (octets 0 - 31)

        // **FIXME** Replace quoted pairs

        if ((c == '"') && (*(buffer - 1) != '\\')) break;

        buffer++;

    } while (1);

    if (token)
        *token = CFStringCreateWithBytes(alloc, start, (buffer - start), kCFStringEncodingISOLatin1, FALSE);

    if (*buffer == '"')
        buffer++;					// Skip '"'

    return buffer;
}


/* static */
UInt8* _CFHTTPAuthenticationParseBase64(CFAllocatorRef alloc, UInt8* buffer, CFStringRef* token) {

    UInt8 *start = buffer;
    UInt8 c = *buffer;

    do {

        if (c & 0x80) break;								// Limit to CHAR (octets 0 - 127)

		if (!isalnum(c) && (c != '+') && (c != '=') && (c != '/'))
			break;

        buffer++;

        c = *buffer;

    } while (1);

    if (token)
        *token = ((buffer == start) ? NULL : CFStringCreateWithBytes(alloc, start, buffer - start, kCFStringEncodingISOLatin1, FALSE));

    return buffer;
}


/* static */
void _CFHTTPAuthenticationParseDomains(CFHTTPAuthenticationRef auth, CFURLRef base) {

    _CFAssertLocked(&auth->_lock);

    CFTypeRef domain_list = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDomain);
    
    if (!domain_list) {
    
        CFMutableArrayRef new_list = CFArrayCreateMutable(CFGetAllocator(auth), 0, &kCFTypeArrayCallBacks);
        CFArrayAppendValue(new_list, base);
        
        CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDomain, new_list);
        CFRelease(new_list);
        
        return;
    }
    
    if (CFGetTypeID(domain_list) == CFStringGetTypeID()) {
    
        UInt8 *buffer, *start, *end;
        CFIndex buffer_size;
        
        CFAllocatorRef alloc = CFGetAllocator(auth);
        CFMutableArrayRef new_list = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
		
		// Get the bytes of the conversion
		start = buffer = _CFStringGetOrCreateCString(alloc, domain_list, NULL, &buffer_size, kCFStringEncodingISOLatin1);
        
        end = _CFHTTPAuthenticationSkipToLWS(start);
        do {
            CFURLRef url = CFURLCreateWithBytes(alloc, start, end - start, kCFStringEncodingISOLatin1, base);
            
            if (url) {
                CFArrayAppendValue(new_list, url);
                CFRelease(url);
            }
            
            start = _CFHTTPAuthenticationSkipLWS(end);
            end = _CFHTTPAuthenticationSkipToLWS(start);
        } while (start != end);
        
        CFAllocatorDeallocate(alloc, buffer);
        
        CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDomain, new_list);
        CFRelease(new_list);
    }
}


// Parses the value of a WWW-Authenticate header.  Note that if we got multiple headers, the values were
// previously combined into one value, separated by commas.
/* static */
Boolean _CFHTTPAuthenticationParseHeader(CFStringRef headerValue, Boolean isInfoHeader, CFMutableDictionaryRef schemes) {

    CFAllocatorRef alloc = CFGetAllocator(schemes);
    UInt8* buffer, *head;
    CFIndex buffer_size;

	// **FIXME** Should the original header value be saved?

	// Get the bytes of the conversion
	head = buffer = _CFStringGetOrCreateCString(alloc, headerValue, NULL, &buffer_size, kCFStringEncodingISOLatin1);

    // Every time we go through this loop to handle a token in the string, we're in one of five states:
    // 1) Expecting a new scheme (at the start, after a comma after a base64 token)
    // 2) Expecting a new key (after a scheme, after a key=value)
    // 3) Expecting either a new key or a new scheme (after a comma after a key=value)
    // 4) Expecting a value (after key=)
    // 5) Expecting a base64 value (after non-key-value schemes, like Negotiate or NTLM)
    Boolean expectingBase64 = FALSE;
    Boolean lookAheadForSchemeOrKey = FALSE;
    CFMutableDictionaryRef current_scheme = NULL;
    CFStringRef key = NULL;
    Boolean parseError = FALSE;

    // Hack to parse Authentication-Info headers, which just have key=value pairs, but no scheme name.
    // We basically pre-load the parsing state as if we had already seen a scheme name, and dump the
    // keys and values into the top level dict the client gave us.
    if (isInfoHeader) {
        current_scheme = schemes;
    }
    
	while (*buffer) {
		
		CFStringRef value = NULL;

        if (!expectingBase64) {
            if (*buffer == '"')
                buffer = _CFHTTPAuthenticationParseQuoted(alloc, buffer, &value);
            else
                buffer = _CFHTTPAuthenticationParseToken(alloc, buffer, &value);
        } else
            buffer = _CFHTTPAuthenticationParseBase64(alloc, buffer, &value);

        if (!value) {
            // found delimiters, control chars, garbage - bail
            parseError = TRUE;
            break;
        }        

        if (lookAheadForSchemeOrKey) {
            // After a comma we might get new "key=value" pairs or a new scheme.  We look ahead for
            // the "=" to make our guess.
            if (*buffer != '=') {
                if (!isInfoHeader) {
                    //if (*buffer) buffer++;
                    current_scheme = NULL;	// treat it like a new scheme
                } else {
                    // Auth-Info headers can't have schemes, just key=value's
					CFRelease(value);
                    parseError = TRUE;
                    break;
                }
            }
            lookAheadForSchemeOrKey = FALSE;
        }

        if (!current_scheme) {
            assert(!key);
            // Found a new scheme.
            // Canonicalize the name so we can look it up reliably in schemes dict
            CFStringRef canonName = _canonicalSchemeName(value);
            current_scheme = CFDictionaryCreateMutable(alloc, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFDictionarySetValue(schemes, canonName, current_scheme);
            CFRelease(canonName);
            CFRelease(current_scheme);
            CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyMethod, canonName);
            expectingBase64 = (canonName == kCFHTTPAuthenticationSchemeNegotiate)
                              || (canonName == kCFHTTPAuthenticationSchemeNTLM);
        }

        else if (!key && !expectingBase64) {
            assert(current_scheme);
            buffer = _CFHTTPAuthenticationSkipLWS(buffer);
            if (*buffer == '=') {
                // Found "key=", will look for value next time around
                key = CFRetain(value);
                buffer++;
            } else {
                // Appears to be a new scheme; or at least that's what it is now
                CFStringRef canonName = _canonicalSchemeName(value);
                current_scheme = CFDictionaryCreateMutable(alloc, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                CFDictionarySetValue(schemes, canonName, current_scheme);
                CFRelease(canonName);
                CFRelease(current_scheme);
                CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyMethod, canonName);
                expectingBase64 = (canonName == kCFHTTPAuthenticationSchemeNegotiate)
                        || (canonName == kCFHTTPAuthenticationSchemeNTLM);
            }
        }
        
        else if (key) {
            assert(current_scheme && !expectingBase64);
            // Sucessfully found key=value
            CFStringRef properKey = _CFCapitalizeHeader(key);
            CFDictionarySetValue(current_scheme, properKey, value);
            CFRelease(properKey);
            CFRelease(key);
            key = NULL;
        }

        else {
            assert(expectingBase64 && current_scheme && !key);
            CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyNegotiateAuthData, value);
        }
    
        CFRelease(value);

        buffer = _CFHTTPAuthenticationSkipLWS(buffer);
        if (*buffer == ',') {
            if (!key) {
                if (expectingBase64) {
                    // Base64-style schemes must be followed by another scheme
                    current_scheme = NULL;
                    expectingBase64 = FALSE;  // Since the next is a scheme, no need to expect this.
                } else {
                    // might get another key=value, might get another scheme
                    lookAheadForSchemeOrKey = TRUE;
                }
                buffer++;
                buffer = _CFHTTPAuthenticationSkipLWS(buffer);
            } else {
                // We found "key=", but instead of a value got the end of the scheme
                parseError = TRUE;
                break;
            }
        }
    }

    CFAllocatorDeallocate(alloc, head);

    if (key) {
        // We found "key=", but instead of a value got the end of the scheme
        parseError = TRUE;
        CFRelease(key);
    }

    if (parseError)
        CFDictionaryRemoveAllValues(schemes);

    return !parseError;
}


/* extern */
void _CFHTTPAuthenticationUpdateFromResponse(CFHTTPAuthenticationRef auth, CFHTTPMessageRef response, const void* conn) {
	
	_CFMutexLock(&auth->_lock);
	
	if (!auth->_error.error) {
		
		CFStringRef scheme, value;
		Boolean isInfoHeader = TRUE;
		Boolean isProxy = auth->_proxy ? TRUE : FALSE;
		UInt32 code = CFHTTPMessageGetResponseStatusCode(response);
		
		assert(auth->_preferred);
				
		/* Save the authentication on the response which allows carrying information from request to request. */
		_CFHTTPMessageSetAuthentication(response, auth, auth->_proxy);
		
		/* Get the current scheme being used. */
		scheme = (CFStringRef)CFDictionaryGetValue(auth->_preferred, kCFHTTPAuthenticationPropertyMethod);
		
		/* Try grabbing the respective Authentication-Info header first */
		value = CFHTTPMessageCopyHeaderFieldValue(response, isProxy ?
															_kCFHTTPMessageHeaderProxyAuthenticationInfo :
															_kCFHTTPMessageHeaderAuthenticationInfo);
		
		/* If no Authentication-Info header, go for the respective Authenticate header. */
		if (!value) {
			value = CFHTTPMessageCopyHeaderFieldValue(response, isProxy ?
																_kCFHTTPMessageHeaderProxyAuthenticate :
																_kCFHTTPMessageHeaderWWWAuthenticate);
			
			isInfoHeader = FALSE;
		}
		
		if (!value) {
			
			/* If got an authentication error for this type of authentication, then it failed. */
			if ((isProxy && (code == 407)) || (!isProxy && (code == 401)))
				_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);
			else if (scheme == kCFHTTPAuthenticationSchemeNTLM) {
				
				_AuthConnectionSpecific* spec = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, conn);
				
				if (spec && !spec->_ntlm && !spec->_authdata && spec->_negotiation) {
					spec->_authdata = spec->_negotiation;
					spec->_negotiation = NULL;
				}
			}
		}
		else {
			
			CFAllocatorRef alloc = CFGetAllocator(auth);
			CFMutableDictionaryRef newInfo = CFDictionaryCreateMutable(alloc,
																	   0,
																	   &kCFCopyStringDictionaryKeyCallBacks,
																	   &kCFTypeDictionaryValueCallBacks);
			
			if (!_CFHTTPAuthenticationParseHeader(value, isInfoHeader, newInfo))
				_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPParseFailure);
			
			else if (scheme == kCFHTTPAuthenticationSchemeBasic) {
				
				/* If got an authentication error for this type of authentication, then it failed. */
				if ((isProxy && (code == 407)) || (!isProxy && (code == 401)))
					_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);
				
				/*
				** Don't carry anything over from the response.  Could it be considered
				** an error if there is some data? **FIXME**
				*/
			}
			
			else if (scheme == kCFHTTPAuthenticationSchemeDigest) {
				
				CFTypeRef v = NULL;
				Boolean stale = FALSE;
				CFDictionaryRef parsed = (CFDictionaryRef)CFDictionaryGetValue(newInfo, scheme);
				
				/*
				** **FIXME** Should qop values be checked for validation? Do other
				** values need to migrate from newInfo to auth?
				*/
				
				if (parsed) {
					
					v = CFDictionaryGetValue(parsed, kCFHTTPAuthenticationPropertyDigestNextNonce);
					if (v) {
						SInt32 zero = 0;
						CFNumberRef nonce_count;
						
						CFStringRef uqNonce = _CFStringUnquote(v);
						CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDigestNextNonce, uqNonce);
						CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDigestNonce, uqNonce);
						CFRelease(uqNonce);
						
						nonce_count = CFNumberCreate(alloc, kCFNumberSInt32Type, &zero);
						CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDigestNonceCount, nonce_count);
						CFRelease(nonce_count);
					}
					
					v = CFDictionaryGetValue(parsed, kCFHTTPAuthenticationPropertyDigestNonce);
					if (v) {
						SInt32 zero = 0;
						CFNumberRef nonce_count;
						
						CFStringRef uqNonce = _CFStringUnquote(v);
						CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDigestNonce, uqNonce);
						CFRelease(uqNonce);
						
						nonce_count = CFNumberCreate(alloc, kCFNumberSInt32Type, &zero);
						CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDigestNonceCount, nonce_count);
						CFRelease(nonce_count);
					}
					
					v = CFDictionaryGetValue(parsed, _kCFHTTPAuthenticationPropertyDigestStale);
					stale = v && (kCFCompareEqualTo == CFStringCompare(v, _kCFHTTPAuthenticationDigestStaleTrue, kCFCompareCaseInsensitive));
				}
				
				if (!stale) {
					
					/* If got an authentication failure and not stale, need to error the auth object. */
					if ((isProxy && (code == 407)) || (!isProxy && (code == 401)))
						_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);
				}
			}
			
			else if (scheme == kCFHTTPAuthenticationSchemeNTLM) {
				
				_AuthConnectionSpecific* spec = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, conn);
				
				if (spec) {
					
					CFDictionaryRef parsed = (CFDictionaryRef)CFDictionaryGetValue(newInfo, scheme);
					CFStringRef data = parsed ? CFDictionaryGetValue(parsed, kCFHTTPAuthenticationPropertyNegotiateAuthData) : NULL;

					if (data) {
						
						CFDataRef server = _CFDecodeBase64(alloc, data);
						
						if (spec->_authdata)
							CFRelease(spec->_authdata);
						
						if (spec->_ntlm) {
							OSStatus result;
							CFDataRef blob = NULL;
							result = _NtlmCreateClientResponse(spec->_ntlm, server, auth->_domain, auth->_user, auth->_hash[0], auth->_hash[1], &blob);
							
							NtlmGeneratorRelease(spec->_ntlm);
							spec->_ntlm = NULL;
							
							if (result) {
								_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainMacOSStatus, result);
							}
							
							if (blob) {
								if (spec->_negotiation) CFRelease(spec->_negotiation);
								spec->_negotiation = _CFEncodeBase64(alloc, blob);
								CFRelease(blob);
							}
						}
						CFRelease(server);
						
						spec->_authdata = CFRetain(data);
					}
					
					/* Failed to do the negotiated authentication. */
					else if ((!spec->_ntlm && spec->_negotiation) && ((isProxy && (code == 407)) || (!isProxy && (code == 401))))
						_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);
				}
			}
			
			else if (scheme == kCFHTTPAuthenticationSchemeNegotiate) {
				
				_AuthConnectionSpecific* spec = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, conn);
				CFDictionaryRef parsed = (CFDictionaryRef)CFDictionaryGetValue(newInfo, scheme);
				CFStringRef data = parsed ? CFDictionaryGetValue(parsed, kCFHTTPAuthenticationPropertyNegotiateAuthData) :
											NULL;
				
				if (spec && data)
					CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyNegotiateAuthData, data);
				
				/* Failed to do the negotiated authentication. */
				else if (spec && ((isProxy && (code == 407)) || (!isProxy && (code == 401)))) {
					
					/*
					** **FIXME** This used to attempt to do a fallback to NTLM if negotiate failed.
					** This was done primarily for Win32, but on MacOS, tickets and such are checked
					** up front.  This will need to be fixed once Win32 is brought up to snuff again.
					*/
					
					_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);
				}
			}
			
			CFRelease(newInfo);
			CFRelease(value);
		}
	}
	
	_CFMutexUnlock(&auth->_lock);
}


#if 0
#pragma mark -
#pragma mark Header Creation - Encoding, Decoding, Crypto
#endif

/* extern */
CFStringRef _CFEncodeBase64(CFAllocatorRef allocator, CFDataRef inputData) {
	
	unsigned outDataLen;	
	CFStringRef result = NULL;
	unsigned char *outData = cuEnc64(CFDataGetBytePtr(inputData), CFDataGetLength(inputData), &outDataLen);
	
	if(outData) {
		
		/* current cuEnc64 appends \n and NULL, trim them */
		unsigned char *c = outData + outDataLen - 1;
		while((*c == '\n') || (*c == '\0')) {
			c--;
			outDataLen--;
		}

		result = CFStringCreateWithBytes(allocator, outData, outDataLen, kCFStringEncodingASCII, FALSE);
		free(outData);
	}
	
	return result;
}


/* extern */
CFDataRef _CFDecodeBase64(CFAllocatorRef allocator, CFStringRef str) {
	
	CFDataRef result = NULL;
	UInt8 stack_buffer[1024];
	UInt8* buffer = stack_buffer;
	CFIndex length = sizeof(stack_buffer);

	buffer = _CFStringGetOrCreateCString(allocator, str, buffer, &length, kCFStringEncodingASCII);

	if (buffer) {
		unsigned decoded;
		unsigned char* decode = cuDec64(buffer, length, &decoded);
		
		if (buffer != stack_buffer)
			CFAllocatorDeallocate(allocator, buffer);
		
		/*
		** Don't use CFDataCreateWithBytesNoCopy since the bytes might
		** be backed by the stack buffer (stack_buffer).
		*/
		if (decode) {
			result = CFDataCreate(allocator, decode, decoded);
			free(decode);
		}
	}

	return result;
}


static CFDataRef dataForUserPassword(CFAllocatorRef alloc, CFStringRef user, CFStringRef password, CFStreamError* error) {
    CFIndex position;
    CFIndex userLen = CFStringGetLength(user);
    CFIndex passLen = CFStringGetLength(password);
    CFIndex numBytes;
    UInt8 *buf;
    CFStreamError extra;
    if (!error)
        error = &extra;

    memset(error, 0, sizeof(error[0]));

    numBytes = CFStringGetMaximumSizeForEncoding(userLen, kCFStringEncodingISOLatin1) + CFStringGetMaximumSizeForEncoding(passLen, kCFStringEncodingISOLatin1) + 1;  // +1 for the colon
    buf = CFAllocatorAllocate(alloc, numBytes, 0);
    
    if (CFStringGetBytes(user, CFRangeMake(0, userLen), kCFStringEncodingISOLatin1, 0, FALSE, buf, numBytes, &position) != userLen || userLen >= numBytes) {
        CFAllocatorDeallocate(alloc, buf);
        error->error = kCFStreamErrorHTTPAuthenticationBadUserName;
        error->domain = kCFStreamErrorDomainHTTP;
        return NULL;
    }
    buf[position] = ':';
    position ++;
    userLen = position;
    if (CFStringGetBytes(password, CFRangeMake(0, passLen), kCFStringEncodingISOLatin1, 0, FALSE, buf+position, numBytes - position, &position) != passLen) {
        CFAllocatorDeallocate(alloc, buf);
        error->error = kCFStreamErrorHTTPAuthenticationBadPassword;
        error->domain = kCFStreamErrorDomainHTTP;
        return NULL;
    }

    return CFDataCreateWithBytesNoCopy(alloc, buf, userLen + position, alloc);
}


// Currently this is the one auth scheme we let people apply without having an auth object (as they
// can call the API without any previous response).
/* static */
Boolean _CFHTTPMessageSetBasicAuthenticationOnRequest(CFHTTPMessageRef request, CFStringRef user, CFStringRef password, Boolean forProxy, CFStreamError* error) {
    CFAllocatorRef alloc = CFGetAllocator(request);
    CFStringRef base64String;
    CFStringRef authString;

    CFDataRef userPasswdData = dataForUserPassword(alloc, user, password, error);
    if (!userPasswdData) return FALSE;

    base64String = _CFEncodeBase64(alloc, userPasswdData);
    CFRelease(userPasswdData);
    if (!base64String) return FALSE;

    authString = CFStringCreateWithFormat(alloc, 0, kCFHTTPAuthenticationBasicFormat, base64String);
    CFRelease(base64String);
    if (forProxy) {
        CFHTTPMessageSetHeaderFieldValue(request, _kCFHTTPMessageHeaderProxyAuthorization, authString);
    } else {
        CFHTTPMessageSetHeaderFieldValue(request, _kCFHTTPMessageHeaderAuthorization, authString);
    }
    CFRelease(authString);
    return TRUE;
}


/* static */
CFStringRef _CFStringQuote(CFStringRef unquoted) {

    CFAllocatorRef alloc = CFGetAllocator(unquoted);
    CFStringRef result = unquoted;
    CFIndex i = 0, length = CFStringGetLength(unquoted);
    CFStringInlineBuffer buffer;

    CFRetain(unquoted);

    while (i < length) {

        CFIndex j = 0;
        CFRange r = CFRangeMake(j, ((__kCFStringInlineBufferLength > (length - i)) ? (length - i) :  __kCFStringInlineBufferLength));
        CFStringRef append;

        CFStringInitInlineBuffer(unquoted, &buffer, r);

        while (j < r.length) {

            UniChar c = CFStringGetCharacterFromInlineBuffer(&buffer, j);

            if (c > 255) {
                if (result != unquoted)
                    CFRelease(result);
                return NULL;
            }

            if (c == '"') {

                char replacement[16];
                strcpy(replacement, "\\\"");

                if (result != unquoted)
                    append = CFStringCreateWithSubstring(alloc, unquoted, CFRangeMake(i + r.location, j - r.location));
                else {
                    result = CFStringCreateMutable(alloc, 0);
                
                    append = CFStringCreateWithSubstring(alloc, unquoted, CFRangeMake(0, i + j));
                    CFRelease(unquoted);
                }
                
                CFStringAppend((CFMutableStringRef)result, append);
                CFRelease(append);
                r.location = j + 1;
                
                CFStringAppendCString((CFMutableStringRef)result, replacement, kCFStringEncodingASCII);
            }
                
            j++;
        }

        if ((result != unquoted) && (r.location != j)) {

            append = CFStringCreateWithSubstring(alloc, unquoted, CFRangeMake(i + r.location, j - r.location));
            CFStringAppend((CFMutableStringRef)result, append);
            CFRelease(append);
        }

        i += r.length;
    }

    return result;
}


/* static */
CFStringRef _CFStringUnquote(CFStringRef quoted) {

    CFAllocatorRef alloc = CFGetAllocator(quoted);
    CFStringRef result = quoted;
    CFIndex i = 0, length = CFStringGetLength(quoted);
    CFStringInlineBuffer buffer;

    CFRetain(quoted);

    while (i < length) {

        CFIndex j = 0;
        CFRange r = CFRangeMake(j, ((__kCFStringInlineBufferLength > (length - i)) ? (length - i) :  __kCFStringInlineBufferLength));
        CFStringRef append;

        CFStringInitInlineBuffer(quoted, &buffer, r);

        while (j < r.length) {

            UniChar c = CFStringGetCharacterFromInlineBuffer(&buffer, j);

            if (c == '\\') {

                if (result != quoted)
                    append = CFStringCreateWithSubstring(alloc, quoted, CFRangeMake(i + r.location, j - r.location));
                else {
                    result = CFStringCreateMutable(alloc, 0);

                    append = CFStringCreateWithSubstring(alloc, quoted, CFRangeMake(0, i + j));
                    CFRelease(quoted);
                }

                CFStringAppend((CFMutableStringRef)result, append);
                CFRelease(append);
                r.location = j + 1;
            }

            j++;
        }

        if ((result != quoted) && (r.location != j)) {

            append = CFStringCreateWithSubstring(alloc, quoted, CFRangeMake(i + r.location, j - r.location));
            CFStringAppend((CFMutableStringRef)result, append);
            CFRelease(append);
        }

        i += r.length;
    }

    return result;
}


#if defined(__MACH__)
/* static */
Boolean _CFMD5(const UInt8* d, UInt32 n, UInt8* md, UInt32 md_length) {

	CC_MD5_CTX ctx;
	CC_MD5_Init(&ctx);
	CC_MD5_Update(&ctx, d, n);
	CC_MD5_Final(md, &ctx);

	return TRUE;
}
#endif


/* static */
CFStringRef _CFStringCreateMD5HashWithString(CFAllocatorRef alloc, CFStringRef string) {

    UInt32 i;
    UInt8 hash[16];			// Maximum length for hash
    CFIndex buffer_size;    
    CFStringRef result = NULL;
	
	// Get the bytes of the conversion
	UInt8* buffer = _CFStringGetOrCreateCString(alloc, string, NULL, &buffer_size, kCFStringEncodingISOLatin1);
	Boolean did = _CFMD5(buffer, buffer_size, hash, sizeof(hash));
        
	CFAllocatorDeallocate(alloc, buffer);

    if (did) {
        
		char str[33] = {'\0'};
		
        for (i = 0; i < sizeof(hash); i++) {
            char small_buffer[3];
            sprintf(small_buffer, "%02x", hash[i]);
            strcat(str, small_buffer);
        }
        
        result = CFStringCreateWithCString(alloc, str, kCFStringEncodingASCII);
    }
        
	return result;
}


/* static */
CFStringRef _CFStringCreateDigestHashA1(CFAllocatorRef alloc, CFHTTPAuthenticationRef auth, CFStringRef username, CFStringRef password) {

	CFStringRef a1, value, result = NULL;
    CFStringRef user = _CFStringUnquote(username);

    _CFAssertLocked(&auth->_lock);

    a1 = CFStringCreateWithFormat(alloc,
                                  NULL,
                                  kCFHTTPAuthenticationDigestHashA1Format,
                                  user,
                                  _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyRealm),
                                  password);
    result = _CFStringCreateMD5HashWithString(alloc, a1);
    CFRelease(a1);
    CFRelease(user);
	
    value = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestAlgorithm);
    
    if (value && !CFStringCompare(value, kCFHTTPAuthenticationDigestAlgorithmMD5Session, kCFCompareCaseInsensitive)) {
        
        a1 = CFStringCreateWithFormat(alloc,
                                      NULL,
                                      kCFHTTPAuthenticationDigestHashA1Format,
                                      result,
                                      _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestNonce),
                                      _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestCNonce));
                                         
        CFRelease(result);
        result = _CFStringCreateMD5HashWithString(alloc, a1);
        CFRelease(a1);
    }
	
	return result;
}


/* static */
CFStringRef _CFStringCreateDigestHashA2(CFAllocatorRef alloc, CFHTTPAuthenticationRef auth, CFHTTPMessageRef request) {

	// **FIXME** Currently, Digest authentication does not support auth-int qop.

	CFStringRef a2, result = NULL;
    //CFStringRef body_hash = NULL;
	CFStringRef method = CFHTTPMessageCopyRequestMethod(request);
	CFURLRef url = CFHTTPMessageCopyRequestURL(request);
	CFStringRef path = CFURLCopyPath(url);

    _CFAssertLocked(&auth->_lock);

    if (auth->_proxy) {
        CFStringRef scheme = CFURLCopyScheme(url);
        if (scheme) {
            if (CFStringCompare(scheme, kCFHTTPAuthenticationHTTPSScheme, 0) == kCFCompareEqualTo) {
                
                SInt32 port = CFURLGetPortNumber(url);
                CFStringRef host = CFURLCopyHostName(url);
            
                CFRelease(method);
                method = CFRetain(kCFHTTPAuthenticationCONNECTMethod);
                
                CFRelease(path);
                path = CFStringCreateWithFormat(alloc, NULL, kCFHTTPAuthenticationHostPortFormat, host, ((port == -1) ? 443 : port));
            }
            CFRelease(scheme);
        }
    }
    else {
        UInt8 buf[512], *bytes = buf, *pathBytes;
        Boolean deallocBytes;
        pathBytes = _CFURLPortionForRequest(alloc, url, FALSE, &bytes, sizeof(buf)/sizeof(UInt8), &deallocBytes);
        CFRelease(path);
        path = CFStringCreateWithBytes(alloc, pathBytes, strlen((const char*)pathBytes), kCFStringEncodingISOLatin1, FALSE);
        if (deallocBytes) CFAllocatorDeallocate(alloc, bytes);
    }
    
	CFRelease(url);
	
    //if (_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestQop)) {
	//	
	//	/* Do nothing */ ;
    //    // **FIXME** If "auth-int" compute BodyHash
    //}
	
    //if (body_hash)
	//	a2 = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@:%@:%@"), method, path, body_hash);
	//else
		a2 = CFStringCreateWithFormat(alloc, NULL, kCFHTTPAuthenticationDigestHashA2NoQopFormat, method, path);
		
	CFRelease(method);
	CFRelease(path);
    
    result = _CFStringCreateMD5HashWithString(alloc, a2);
    CFRelease(a2);
	
	return result;
}


/* static */
CFStringRef _CFStringCreateDigestHash(CFAllocatorRef alloc, CFHTTPAuthenticationRef auth, CFStringRef a1, CFStringRef a2) {

	CFStringRef hash, result = NULL;

    _CFAssertLocked(&auth->_lock);

    if (_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestQop)) {
	
		UInt32 value;
		CFNumberRef count = (CFNumberRef)_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestNonceCount);
		
		CFNumberGetValue(count, kCFNumberSInt32Type, &value);
		value++;
		
		count = CFNumberCreate(CFGetAllocator(auth), kCFNumberSInt32Type, &value);
		CFDictionarySetValue(auth->_preferred, kCFHTTPAuthenticationPropertyDigestNonceCount, count);
		CFRelease(count);
	
        hash = CFStringCreateWithFormat(alloc,
										NULL,
										kCFHTTPAuthenticationDigestHashQopFormat,
										a1,
										_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestNonce),
										value,
										_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestCNonce),
										_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestQop),
										a2);
    }
    
    else {
        hash = CFStringCreateWithFormat(alloc,
										NULL,
										kCFHTTPAuthenticationDigestHashFormat,
										a1,
										_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestNonce),
										a2);        
    }
	
    result = _CFStringCreateMD5HashWithString(alloc, hash);
    CFRelease(hash);
	
	return result;
}


/* static */
CFStringRef _CFStringCreateDigestAuthenticationHeaderValueForRequest(CFAllocatorRef alloc,
                                                                     CFHTTPAuthenticationRef auth,
																	 CFHTTPMessageRef request,
                                                                     CFStringRef username,
																	 CFStringRef hash)
{	
	CFMutableStringRef header = CFStringCreateMutable(alloc, 0);
	
	CFURLRef url = CFHTTPMessageCopyRequestURL(request);
	CFStringRef path = CFURLCopyPath(url);
	CFStringRef value, qRealm, qNonce;

    _CFAssertLocked(&auth->_lock);

    if (auth->_proxy) {
        CFStringRef scheme = CFURLCopyScheme(url);
        if (scheme) {
            if (CFStringCompare(scheme, kCFHTTPAuthenticationHTTPSScheme, 0) == kCFCompareEqualTo) {
                
                SInt32 port = CFURLGetPortNumber(url);
                CFStringRef host = CFURLCopyHostName(url);
                
                CFRelease(path);
                path = CFStringCreateWithFormat(alloc, NULL, kCFHTTPAuthenticationHostPortFormat, host, ((port == -1) ? 443 : port));
            }
            CFRelease(scheme);
        }
    }
    else {
        UInt8 buf[512], *bytes = buf, *pathBytes;
        Boolean deallocBytes;
        pathBytes = _CFURLPortionForRequest(alloc, url, FALSE, &bytes, sizeof(buf)/sizeof(UInt8), &deallocBytes);
        CFRelease(path);
        path = CFStringCreateWithBytes(alloc, pathBytes, strlen((const char*)pathBytes), kCFStringEncodingISOLatin1, FALSE);
        if (deallocBytes) CFAllocatorDeallocate(alloc, bytes);
    }
	CFRelease(url);
    
    qRealm = _CFStringQuote(_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyRealm));
    qNonce = _CFStringQuote(_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestNonce));
    
	CFStringAppendFormat(header,
						 NULL,
						 kCFHTTPAuthenticationDigestHeaderFormat,
						 _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod),
						 username,
						 qRealm,
						 qNonce,
						 path,
						 hash);
	
	CFRelease(path);
    CFRelease(qNonce);
    CFRelease(qRealm);
	
	value = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestOpaque);
	if (value)
		CFStringAppendFormat(header, NULL, kCFHTTPAuthenticationDigestHeaderOpaqueFormat, value);
		
	value = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestAlgorithm);
	if (value)
		CFStringAppendFormat(header, NULL, kCFHTTPAuthenticationDigestHeaderAlgorithmFormat, value);
		
	value = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestQop);
	if (value) {
		UInt32 nc;
        CFStringRef qCNonce = _CFStringQuote(_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestCNonce));
		CFNumberRef count = (CFNumberRef)_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestNonceCount);
		CFNumberGetValue(count, kCFNumberSInt32Type, &nc);
		
		CFStringAppendFormat(header,
							 NULL,
							 kCFHTTPAuthenticationDigestHeaderNoncesFormat,
							 qCNonce,
							 nc,
							 value);
                             
        CFRelease(qCNonce);
	}
	
	return header;
}


/* static */
Boolean _CFHTTPMessageSetDigestAuthenticationOnRequest(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth,
                                                       CFStringRef username, CFStringRef password)
{
    CFAllocatorRef alloc = CFGetAllocator(request);
    
    CFStringRef a1 = _CFStringCreateDigestHashA1(alloc, auth, username, password);
	CFStringRef a2 = _CFStringCreateDigestHashA2(alloc, auth, request);
	CFStringRef hash = _CFStringCreateDigestHash(alloc, auth, a1, a2);
	CFStringRef header;
	
	CFRelease(a1);
	CFRelease(a2);

    // Create header value and place on request
	header = _CFStringCreateDigestAuthenticationHeaderValueForRequest(alloc, auth, request, username, hash);
	
	CFRelease(hash);
    
	if (!auth->_proxy)
		CFHTTPMessageSetHeaderFieldValue(request, _kCFHTTPMessageHeaderAuthorization, header);
	else
		CFHTTPMessageSetHeaderFieldValue(request, _kCFHTTPMessageHeaderProxyAuthorization, header);

	CFRelease(header);
	
    return TRUE;
}


/* static */ Boolean
_CFHTTPMessageSetNegotiateAuthenticationOnRequest(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth,
												  CFStringRef username, CFStringRef password)
{
	CFURLRef url = CFHTTPMessageCopyRequestURL(request);
	CFURLRef absolute = url ? CFURLCopyAbsoluteURL(url) : NULL;
	CFStringRef host = absolute ? CFURLCopyHostName(absolute) : NULL;
	CFStringRef scheme = absolute ? CFURLCopyScheme(absolute) : NULL;
	Boolean result = (host && url) ? TRUE : FALSE;

	if (scheme) CFRelease(scheme);
	if (host) CFRelease(host);
	if (absolute) CFRelease(absolute);
	if (url) CFRelease(url);
	
	if (!result)
		_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPBadURL);
	
	return result;
}


/* static */ CFStringRef
_CFHTTPAuthenticationCreateNegotiateHeaderForRequest(CFHTTPAuthenticationRef auth, CFHTTPMessageRef request, const void* connection) {
	
	CFStringRef header = NULL;
	_AuthConnectionSpecific* specific = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, connection);

	if (!specific) {
		
		_AuthConnectionSpecific s = {NULL, NULL, NULL};
		CFDictionaryAddValue(auth->_connections, connection, &s);
		
		/* Re-fetch because the add will make a new copy in the dictionary. */
		specific = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, connection);
		
		if (!specific)
			_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainPOSIX, ENOMEM);
	}
	
	if (specific) {
		
		CFAllocatorRef alloc = CFGetAllocator(auth);
		CFURLRef url = CFHTTPMessageCopyRequestURL(request);
		CFURLRef absolute = url ? CFURLCopyAbsoluteURL(url) : NULL;
		CFStringRef host = absolute ? CFURLCopyHostName(absolute) : NULL;
		CFStringRef scheme = absolute ? CFURLCopyScheme(absolute) : NULL;
		
		CFMutableStringRef tmp = host ? CFStringCreateMutableCopy(alloc, 0, host) : NULL;
		
		if (tmp) {
			CFStringLowercase(tmp, NULL);
			CFRelease(host);
			host = tmp;
		}
		
		tmp = scheme ? CFStringCreateMutableCopy(alloc, 0, scheme) : NULL;
		
		if (tmp) {
			CFStringLowercase(tmp, NULL);
			CFRelease(scheme);
			scheme = tmp;
		}
		
		if (!host || !scheme) {
			_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainPOSIX, ENOMEM);		
		}
		else {
			
			char* blob = NULL;
			SInt32 spnegoError;
			CFDataRef data = NULL;
			
			UInt8 buf1[1024], buf2[1024];
			CFIndex len = sizeof(buf1);
			
			UInt8* hostname = _CFStringGetOrCreateCString(alloc, host, buf1, &len, kCFStringEncodingASCII);
			UInt8* servicetype;
			
			len = sizeof(buf2);
			servicetype = _CFStringGetOrCreateCString(alloc, scheme, buf2, &len, kCFStringEncodingASCII);
			
			// if this is http or https, we're going to try 2 forms of ticket retrieval
			if (!strncmp("http", (const char*)servicetype, 4)) {

				// first force try the uppercase
				spnegoError = spnegoTokenInitFromPrincipal((const char*)hostname, "HTTP", &blob, (unsigned*)&len);
				if( spnegoError ) {
					spnegoError = spnegoTokenInitFromPrincipal((const char*)hostname, "http", &blob, (unsigned*)&len);
				}
			} else {
				spnegoError = spnegoTokenInitFromPrincipal((const char*)hostname, (const char *)servicetype, &blob, (unsigned*)&len);
			}
			
			if (hostname != buf1)
				CFAllocatorDeallocate(alloc, hostname);
			
			if (servicetype != buf2)
				CFAllocatorDeallocate(alloc, servicetype);
			
			if (spnegoError)
				_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);
			
			else {
				
				data = CFDataCreateWithBytesNoCopy(alloc, (const UInt8*)blob, len, kCFAllocatorNull);
				
				if (specific->_negotiation) CFRelease(specific->_negotiation);
				specific->_negotiation = _CFEncodeBase64(alloc, data);
				
				if (specific->_negotiation)
					header = CFStringCreateWithFormat(alloc, NULL, kCFHTTPAuthenticationNegotiateNegotiateFormat, specific->_negotiation);

				if (!header)
					_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainPOSIX, ENOMEM);		
				
				CFRelease(data);
				free(blob);
			}
		}
		
		if (scheme) CFRelease(scheme);
		if (host) CFRelease(host);
		if (absolute) CFRelease(absolute);
		if (url) CFRelease(url);
	}
	
	return header;
}


/* static */ Boolean
_CFHTTPMessageSetNTLMAuthenticationOnRequest(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth,
											 CFStringRef username, CFStringRef password, CFStringRef domain)
{
	CFIndex i, count = CFDictionaryGetCount(auth->_connections);
	
	_AuthConnectionSpecific* stack_specifics[16];	
	_AuthConnectionSpecific** specifics = &stack_specifics[0];
	
	CFAllocatorRef alloc = CFGetAllocator(request);
	
	if (!auth->_hash[0]) {
		
		if (username && password) {
			
			OSStatus result;
			
			auth->_user = (CFStringRef)CFRetain(username);
			auth->_domain = domain ? (CFStringRef)CFRetain(domain) : NULL;
			
			if (noErr != (result = NtlmGeneratePasswordHashes(alloc, password, &(auth->_hash[0]), &(auth->_hash[1])))) {
				
				_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainMacOSStatus, result);
				
				return FALSE;
			}
		}
		
		else {			
			_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, username ? kCFStreamErrorHTTPAuthenticationBadPassword : kCFStreamErrorHTTPAuthenticationBadUserName);
			
			return FALSE;
		}
	}
	
    if (count > (sizeof(stack_specifics) / sizeof(stack_specifics[0]))) {
	
		specifics = (_AuthConnectionSpecific**)CFAllocatorAllocate(alloc, count * sizeof(stack_specifics[0]), 0);
		
		if (!specifics) {
			
			_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainPOSIX, ENOMEM);
				
			return FALSE;
		}
	}
	
	CFDictionaryGetKeysAndValues(auth->_connections, NULL, (const void **)specifics);
	
	for (i = 0; i < count; i++) {
		
		CFDataRef blob = NULL;
		CFStreamError error = {kCFStreamErrorDomainMacOSStatus, 0};

		if (!specifics[i]->_ntlm) {
		
			if (!specifics[i]->_negotiation && !specifics[i]->_authdata) {
				error.error = NtlmGeneratorCreate(NW_Any, &(specifics[i]->_ntlm));
				if (!error.error)
					error.error = NtlmCreateClientRequest(specifics[i]->_ntlm, &blob);
			}
		}
		
		else if (specifics[i]->_authdata) {

			CFDataRef server = _CFDecodeBase64(alloc, specifics[i]->_authdata);
			
			error.error = _NtlmCreateClientResponse(specifics[i]->_ntlm, server, auth->_domain, auth->_user, auth->_hash[0], auth->_hash[1], &blob);
			CFRelease(server);
			
			NtlmGeneratorRelease(specifics[i]->_ntlm);
			specifics[i]->_ntlm = NULL;
		}

		if (error.error) {
			_CFHTTPAuthenticationSetError(auth, error.domain, error.error);
			break;
		}
		
		if (blob) {
			if (specifics[i]->_negotiation) CFRelease(specifics[i]->_negotiation);
			specifics[i]->_negotiation = _CFEncodeBase64(alloc, blob);
			CFRelease(blob);
		}
	}
	
	if (specifics != &stack_specifics[0])
		CFAllocatorDeallocate(alloc, specifics);
	
	return auth->_error.error ? FALSE : TRUE;
}


/* static */ CFStringRef
_CFHTTPAuthenticationCreateNTLMHeaderForRequest(CFHTTPAuthenticationRef auth, CFHTTPMessageRef request, const void* connection) {
	
	CFStringRef header = NULL;
	CFAllocatorRef alloc = CFGetAllocator(auth);
	_AuthConnectionSpecific* specific = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, connection);
	
	if (!specific) {
		
		CFDataRef blob = NULL;
		
		_AuthConnectionSpecific s = {NULL, NULL, NULL};
		
		OSErr err = NtlmGeneratorCreate(NW_Any, &(s._ntlm));
		if (err || (err = NtlmCreateClientRequest(s._ntlm, &blob)))
			_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainMacOSStatus, err);
		else {
			s._negotiation = _CFEncodeBase64(alloc, blob);
			CFRelease(blob);
			
			CFDictionaryAddValue(auth->_connections, connection, &s);
			CFRelease(s._negotiation);
			
			/* Re-fetch because the add will make a new copy in the dictionary. */
			specific = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, connection);
		}
	}
	
	if (specific && specific->_negotiation && (specific->_ntlm || specific->_authdata)) {
		
		header = CFStringCreateWithFormat(alloc, NULL, kCFHTTPAuthenticationNegotiateNTLMFormat, specific->_negotiation);
		
		if (!specific->_ntlm && specific->_authdata) {
			CFRelease(specific->_authdata);
			specific->_authdata = NULL;
		}
		
		if (!header)
			_CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainPOSIX, ENOMEM);
	}
	
	return header;
}


#if 0
#pragma mark -
#pragma mark Extern Function Definitions
#endif


/* CF_EXPORT */ CFTypeID
CFHTTPAuthenticationGetTypeID(void) {

    _CFDoOnce(&gHTTPAuthenticationClassRegistration, _HTTPAuthenticationRegisterClass);

    return kHTTPAuthenticationTypeID;
}


/* CF_EXPORT */
CFHTTPAuthenticationRef CFHTTPAuthenticationCreateFromResponse(CFAllocatorRef alloc, CFHTTPMessageRef response) {
    
	UInt32 code = CFHTTPMessageGetResponseStatusCode(response);
	CFDictionaryKeyCallBacks key_cbs = {0, NULL, NULL, NULL, NULL};
	CFDictionaryValueCallBacks value_cbs = {
		0,
		(CFDictionaryRetainCallBack)_AuthConnectionSpecificRetain,
		(CFDictionaryReleaseCallBack)_AuthConnectionSpecificRelease,
		NULL,
		NULL
	};	
	
	if ((code != 401) && (code != 407))
		return NULL;
	
    CFHTTPAuthenticationRef lastAuth = _CFHTTPMessageGetAuthentication(response, (code == 407));

	if (lastAuth && CFHTTPAuthenticationIsValid(lastAuth, NULL))
		return (CFHTTPAuthenticationRef)CFRetain(lastAuth);
	
    CFMutableDictionaryRef current_scheme = NULL;
    CFURLRef url;
    CFHTTPAuthenticationRef result =
        (CFHTTPAuthenticationRef)_CFRuntimeCreateInstance(alloc,
                                                         CFHTTPAuthenticationGetTypeID(),
                                                         sizeof(result[0]) - sizeof(result->_base),
                                                         NULL);
    _CFMutexInit(&result->_lock, FALSE);
    result->_error.domain = 0;
    result->_error.error = 0;
    result->_preferred = NULL;
    result->_schemes = NULL;
    result->_connections = CFDictionaryCreateMutable(alloc, 0, &key_cbs, &value_cbs);
	result->_user = NULL;
	result->_domain = NULL;
	memset(result->_hash, 0, sizeof(result->_hash));

#ifdef __WIN32__
    result->_sspi = NULL;
#endif  /* __WIN32__ */

    // Even though no one else could be using this auth, GetProperty and SetError need us to be locked,
    // and there it's cheap since there can be no contention
    _CFMutexLock(&result->_lock);

    CFStringRef headerValue = CFHTTPMessageCopyHeaderFieldValue(response, _kCFHTTPMessageHeaderWWWAuthenticate);

    if (headerValue) {
    
        result->_proxy = FALSE;
    }
    else {

        result->_proxy = TRUE;

        headerValue = CFHTTPMessageCopyHeaderFieldValue(response, _kCFHTTPMessageHeaderProxyAuthenticate);
		if (!headerValue) {
		
            _CFHTTPAuthenticationSetError(result, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPParseFailure);	// No authenticate header
            _CFMutexUnlock(&result->_lock);
			return result;
		}
	}

    result->_schemes = CFDictionaryCreateMutable(alloc, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    if (!_CFHTTPAuthenticationParseHeader(headerValue, FALSE, result->_schemes)) {
        _CFHTTPAuthenticationSetError(result, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPParseFailure);
    }
    
    CFRelease(headerValue);
    
    url = CFHTTPMessageCopyRequestURL(response);
	if (url) {
		
		CFIndex length;
		CFRange range;
		
		UInt8 buffer[1024];
		UInt8* ptr = buffer;
		
		/* Make sure to have an absolute url. */
		if (CFURLGetBaseURL(url)) {
			
			CFURLRef tmp = CFURLCopyAbsoluteURL(url);
			
			if (tmp) {
				CFRelease(url);
				url = tmp;
			}
			
			/* NOTE that if absolute couldn't create, this will just try to do the best possible. */
		}
		
		/* Need to strip the user info from the url */
		
		length = CFURLGetBytes(url, ptr, sizeof(buffer) / sizeof(buffer[0]));
		
		/* Need to allocate a buffer if stack one wasn't big enough. */
		if (length == -1) {
			
			length = CFURLGetBytes(url, NULL, 0);
			
			ptr = CFAllocatorAllocate(alloc, length, 0);
			
			CFURLGetBytes(url, ptr, length);
		}
		
		/* Find the user info to strip. */
		range = CFURLGetByteRangeForComponent(url, kCFURLComponentUserInfo, NULL);
		
		if (range.location != kCFNotFound) {
			
			CFIndex end = range.location + range.length + 1;
			
			/* Collapse around the user information. */
			memmove(ptr + range.location, ptr + end, length - end - 1);
			
			CFRelease(url);
			
			/* Create the new url. */
			url = CFURLCreateWithBytes(alloc, ptr, length - (end - range.location), kCFStringEncodingISOLatin1, NULL);
		}
		
		if (ptr != buffer)
			CFAllocatorDeallocate(alloc, ptr);
	}
	
    // Find scheme to use - try them in priority order

    current_scheme = (CFMutableDictionaryRef)CFDictionaryGetValue(result->_schemes, kCFHTTPAuthenticationSchemeNegotiate);
	
	// SPNEGO is not to be used for authenticating with proxies.
    if (current_scheme && url && !result->_proxy) {

#if defined(__MACH__)
		
        CFStringRef host = CFURLCopyHostName(url);
		CFStringRef scheme = CFURLCopyScheme(url);

		CFMutableStringRef tmp = host ? CFStringCreateMutableCopy(alloc, 0, host) : NULL;
		
		if (tmp) {
			CFStringLowercase(tmp, NULL);
			CFRelease(host);
			host = tmp;
		}
        
		tmp = scheme ? CFStringCreateMutableCopy(alloc, 0, scheme) : NULL;
				
		if (tmp) {
			CFStringLowercase(tmp, NULL);
			CFRelease(scheme);
			scheme = tmp;
		}

		if (host && scheme) {
			
			unsigned tktLen;
			UInt8* ticket = NULL;
			
			UInt8 buf1[1024], buf2[1024];
			CFIndex len = sizeof(buf1);
			
			UInt8* hostname = _CFStringGetOrCreateCString(alloc, host, buf1, &len, kCFStringEncodingASCII);
			UInt8* servicetype;
			
			len = sizeof(buf2);
			servicetype = _CFStringGetOrCreateCString(alloc, scheme, buf2, &len, kCFStringEncodingASCII);
				
            // if this is http or https, we're going to try 2 forms of ticket retrieval
			if (!strncmp("http", (const char*)servicetype, 4)) {
                
				// first force try the uppercase then try lowercase
                if (!GetSvcTicketForHost((const char*)hostname, "HTTP", &tktLen, &ticket)) {
                    result->_preferred = current_scheme;
                } else if (!GetSvcTicketForHost((const char*)hostname, "http", &tktLen, &ticket)) {
                    result->_preferred = current_scheme;
                }
			} else if (!GetSvcTicketForHost((const char*)hostname, (const char*)servicetype, &tktLen, &ticket)) {
                result->_preferred = current_scheme;
			}
			
			if (hostname != buf1)
				CFAllocatorDeallocate(alloc, hostname);
			
			if (servicetype != buf2)
				CFAllocatorDeallocate(alloc, servicetype);
			
			if (ticket)
				free(ticket);
		}
		
		if (host) CFRelease(host);
		if (scheme) CFRelease(scheme);

#elif defined(__WIN32__)
        // We don't need to try to make the tokens as on OSX, since Spnego on Windows will
        // downshift from Kerberos to NTLM for us.
        if (_CFSSPIPackageIsEnabled("Negotiate"))
            result->_preferred = current_scheme;
#else
#error SPNEGO not supported, should be disabled on this platform
#endif
    }

    current_scheme = (CFMutableDictionaryRef)CFDictionaryGetValue(result->_schemes, kCFHTTPAuthenticationSchemeNTLM);
#ifndef __WIN32__
	if (!result->_preferred && current_scheme) {
        result->_preferred = current_scheme;
	}
#else
    if (!result->_preferred && current_scheme && _CFSSPIPackageIsEnabled("NTLM")) {
        result->_preferred = current_scheme;
    }
#endif  /* __WIN32__ */

    current_scheme = (CFMutableDictionaryRef)CFDictionaryGetValue(result->_schemes, kCFHTTPAuthenticationSchemeDigest);
    if (!result->_preferred && current_scheme && url
        && CFDictionaryContainsKey(current_scheme, kCFHTTPAuthenticationPropertyRealm)
        && CFDictionaryContainsKey(current_scheme, kCFHTTPAuthenticationPropertyDigestNonce))
    {
        do {
            
            CFStringRef hash;
            CFNumberRef nonce_count;
            CFStringRef value;

            value = CFDictionaryGetValue(current_scheme, kCFHTTPAuthenticationPropertyDigestAlgorithm);
            if (value
                && CFStringCompare(value, kCFHTTPAuthenticationDigestAlgorithmMD5, kCFCompareCaseInsensitive)
                && CFStringCompare(value, kCFHTTPAuthenticationDigestAlgorithmMD5Session, kCFCompareCaseInsensitive))
            {
                break;
            }

            // Pull out and settle on method of QOP.
            value = CFDictionaryGetValue(current_scheme, kCFHTTPAuthenticationPropertyDigestQop);
            if (value) {
                Boolean supported_qop = FALSE;
                CFArrayRef qops = CFStringCreateArrayBySeparatingStrings(alloc, value, kCFHTTPAuthenticationComma);
                CFIndex x, qop_count = CFArrayGetCount(qops);

                for (x = 0; (x < qop_count) && !supported_qop; x++) {
                    CFMutableStringRef qop = CFStringCreateMutableCopy(alloc, 0, (CFStringRef)CFArrayGetValueAtIndex(qops, x));

                    CFStringTrimWhitespace(qop);

                    // Currently, only support kCFHTTPAuthenticationDigestQopAuth ("auth").
                    if (!CFStringCompare(qop, kCFHTTPAuthenticationDigestQopAuth, kCFCompareCaseInsensitive)) {
                        CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyDigestQop, qop);
                        supported_qop = TRUE;
                    }

                    CFRelease(qop);
                }

                CFRelease(qops);

                if (!supported_qop)
                    break;
            }

            value = CFStringCreateWithFormat(alloc, NULL, kCFHTTPAuthenticationMD5HashFormat, (UInt32)result);
            hash = _CFStringCreateMD5HashWithString(alloc, value);

            CFRelease(value);
            CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyDigestCNonce, hash);
            CFRelease(hash);

            value = _CFStringUnquote(CFDictionaryGetValue(current_scheme, kCFHTTPAuthenticationPropertyRealm));
            CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyRealm, value);
            CFRelease(value);

            value = _CFStringUnquote(CFDictionaryGetValue(current_scheme, kCFHTTPAuthenticationPropertyDigestNonce));
            CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyDigestNonce, value);
            CFRelease(value);

            SInt32 zero = 0;
            nonce_count = CFNumberCreate(alloc, kCFNumberSInt32Type, &zero);
            CFDictionarySetValue(current_scheme, kCFHTTPAuthenticationPropertyDigestNonceCount, nonce_count);
            CFRelease(nonce_count);

            result->_preferred = current_scheme;
        } while (0);
    }

    current_scheme = (CFMutableDictionaryRef)CFDictionaryGetValue(result->_schemes, kCFHTTPAuthenticationSchemeBasic);
    if (!result->_preferred && current_scheme
        && CFDictionaryContainsKey(current_scheme, kCFHTTPAuthenticationPropertyRealm))
    {
        result->_preferred = current_scheme;
    }

    // If we found one
    if (result->_preferred) {

        CFStringRef method = (CFStringRef)CFDictionaryGetValue(result->_preferred, kCFHTTPAuthenticationPropertyMethod);

        if (method == kCFHTTPAuthenticationSchemeDigest) {
            // We want scheme://host:port from the original URL
            UInt8 buf[1024], *urlBytes = buf;
            CFIndex length = CFURLGetBytes(url, urlBytes, sizeof(buf)/sizeof(UInt8));
            CFRange pathRg;
            if (length == -1) {
                length = CFURLGetBytes(url, NULL, 0);
                urlBytes = CFAllocatorAllocate(alloc, length, 0);
                CFURLGetBytes(url, urlBytes, length);
            }
			
            CFURLGetByteRangeForComponent(url, kCFURLComponentPath, &pathRg);

			CFRelease(url);
    
            url = CFURLCreateWithBytes(alloc, urlBytes, pathRg.location, kCFStringEncodingISOLatin1, NULL);

            if (urlBytes != buf) {
                CFAllocatorDeallocate(alloc, urlBytes);
            }
        }

        // Basic authentication uses last symbolic element.
        else if (!CFURLHasDirectoryPath(url)) {
            CFURLRef new_url = CFURLCreateCopyDeletingLastPathComponent(alloc, url);
            CFRelease(url);
            url = new_url;
        }

        // Setup domain list
        _CFHTTPAuthenticationParseDomains(result, url);
    }

    else if (result->_error.error == 0)
        _CFHTTPAuthenticationSetError(result, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationTypeUnsupported);		// No scheme supported
            
    if (url) CFRelease(url);

    _CFMutexUnlock(&result->_lock);

    return result;
}


/* CF_EXPORT */
Boolean CFHTTPAuthenticationIsValid(CFHTTPAuthenticationRef auth, CFStreamError* error) {

	CFStreamError extra;
	if (!error)
		error = &extra;
	
    _CFMutexLock(&auth->_lock);

	*error = auth->_error;

    _CFMutexUnlock(&auth->_lock);

    return (error->error == 0);
}


/* CF_EXPORT */
Boolean CFHTTPAuthenticationAppliesToRequest(CFHTTPAuthenticationRef auth, CFHTTPMessageRef request) {

    Boolean result = FALSE;
    CFURLRef url = CFHTTPMessageCopyRequestURL(request);

	if (url) {
		CFURLRef tmp = CFURLCopyAbsoluteURL(url);
		CFRelease(url);
		url = tmp;
	}
	
    _CFMutexLock(&auth->_lock);

	if (auth->_proxy) {
		result = TRUE;
	}
	
	else {
		CFArrayRef domains = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDomain);

		if (domains && url) {
			
			CFIndex i, count = CFArrayGetCount(domains);
			CFStringRef url_str = CFURLGetString(url);
			
			for (i = 0; i < count; i++) {
				
				CFURLRef abs_url = CFURLCopyAbsoluteURL((CFURLRef)CFArrayGetValueAtIndex(domains, i));
				CFStringRef domain_url = CFURLGetString(abs_url);
				
				
				if (CFStringHasPrefix(url_str, domain_url)) {
					result = TRUE;
			CFRelease(abs_url);
					break;
				}
				CFRelease(abs_url);
			}
		}
	}

    _CFMutexUnlock(&auth->_lock);

    if (url)
        CFRelease(url);
    
    return result;
}


/* CF_EXPORT */
Boolean CFHTTPAuthenticationRequiresOrderedRequests(CFHTTPAuthenticationRef auth) {

    _CFMutexLock(&auth->_lock);

    Boolean result = FALSE;
    CFStringRef method = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod);

    if (method) {

        // Negotiate and NTLM auth needs ordered requests.
        if (method == kCFHTTPAuthenticationSchemeNegotiate || method == kCFHTTPAuthenticationSchemeNTLM)
            result = TRUE;

        // Digest auth may need ordered requests.
        else if (method == kCFHTTPAuthenticationSchemeDigest) {

            // If it has a "nextnonce" key, order is required.
            if (_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestNextNonce))
				result = TRUE;

            // If it has a "qop" key, order is required
            else if (_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDigestQop))
                result = TRUE;
        }
    }

    _CFMutexUnlock(&auth->_lock);

    return result;
}


/* CF_EXPORT */
Boolean CFHTTPMessageApplyCredentials(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth, CFStringRef username, CFStringRef password, CFStreamError* error) {

	Boolean result = FALSE;
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(CFGetAllocator(request),
															0,
															&kCFTypeDictionaryKeyCallBacks,
															&kCFTypeDictionaryValueCallBacks);
	
	if (dict) {
		
		if (username) {
			
			if (!CFHTTPAuthenticationRequiresAccountDomain(auth))
				CFDictionaryAddValue(dict, kCFHTTPAuthenticationUsername, username);
			
			else {
				CFArrayRef list = CFStringCreateArrayBySeparatingStrings(CFGetAllocator(username), username, kCFHTTPAuthenticationNTLMDomainUserSeparator);
				
				if (!list || CFArrayGetCount(list) != 2)
					CFDictionaryAddValue(dict, kCFHTTPAuthenticationUsername, username);
				
				else {
					CFDictionaryAddValue(dict, kCFHTTPAuthenticationAccountDomain, CFArrayGetValueAtIndex(list, 0));
					CFDictionaryAddValue(dict, kCFHTTPAuthenticationUsername, CFArrayGetValueAtIndex(list, 1));
				}
				
				if (list) CFRelease(list);
			}
		}
		
		if (password)
			CFDictionaryAddValue(dict, kCFHTTPAuthenticationPassword, password);
			
		result = CFHTTPMessageApplyCredentialDictionary(request, auth, dict, error);
		CFRelease(dict);
	}
	
    return result;
}


/* CF_EXPORT */
Boolean CFHTTPMessageApplyCredentialDictionary(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth, CFDictionaryRef dict, CFStreamError* error) {

    _CFMutexLock(&auth->_lock);
    
    Boolean result = _CFApplyCredentials_Unsafe(request, auth, dict, error);

    _CFMutexUnlock(&auth->_lock);

    return result;
}


/* static */
Boolean _CFApplyCredentials_Unsafe(CFHTTPMessageRef request, CFHTTPAuthenticationRef auth, CFDictionaryRef dict, CFStreamError* error) {

    CFStreamError extra;
    Boolean result = FALSE;
    CFStringRef method = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod);
	CFStringRef username = dict ? CFDictionaryGetValue(dict, kCFHTTPAuthenticationUsername) : NULL;
	CFStringRef password = dict ? CFDictionaryGetValue(dict, kCFHTTPAuthenticationPassword) : NULL;
	CFStringRef domain = dict ? CFDictionaryGetValue(dict, kCFHTTPAuthenticationAccountDomain) : NULL;

    if (!error)
        error = &extra;
	
    if (auth->_error.error) {
        // This auth already failed, can't apply it
        *error = auth->_error;
        return FALSE;
    }
    
    error->domain = error->error = 0;

    if (!method
        || (method != kCFHTTPAuthenticationSchemeNegotiate && method != kCFHTTPAuthenticationSchemeNTLM))
    {
        if (!username || username == _kCFStreamSingleSignOnUserName)
            _CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);

        if (!password)
            _CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadPassword);
    }
    
    if (auth->_error.error == 0) {

        if (method == kCFHTTPAuthenticationSchemeBasic) {

            result = _CFHTTPMessageSetBasicAuthenticationOnRequest(request, username, password, auth->_proxy, &auth->_error);
        }
        
        else if (method == kCFHTTPAuthenticationSchemeDigest) {
        
            // Check username.
            CFStringRef user = _CFStringQuote(username);
            
            if (user) {
            	result = _CFHTTPMessageSetDigestAuthenticationOnRequest(request, auth, user, password);
                CFRelease(user);
            }
            else {
                _CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationBadUserName);
            }
        }

        else if (method == kCFHTTPAuthenticationSchemeNegotiate) {
			_CFHTTPMessageSetNegotiateAuthenticationOnRequest(request, auth, username, password);
			result = (auth->_error.error == 0);
		}
		else if (method == kCFHTTPAuthenticationSchemeNTLM) {
			_CFHTTPMessageSetNTLMAuthenticationOnRequest(request, auth, username, password, domain);
			result = (auth->_error.error == 0);
        }
        
        else {
            _CFHTTPAuthenticationSetError(auth, kCFStreamErrorDomainHTTP, kCFStreamErrorHTTPAuthenticationTypeUnsupported);
        }
    
        if (result)
            _CFHTTPMessageSetAuthentication(request, auth, auth->_proxy);
    }

    *error = auth->_error;

    return result;
}


/* static */
CFTypeRef _CFHTTPAuthenticationGetProperty(CFHTTPAuthenticationRef auth, CFStringRef propertyKey) {

    _CFAssertLocked(&auth->_lock);

    if (CFEqual(propertyKey, _kCFHTTPAuthenticationPropertyPreferredScheme))
        return auth->_preferred;

    else if (CFEqual(propertyKey, _kCFHTTPAuthenticationPropertyAuthenticateType))
        return auth->_proxy ? _kCFHTTPMessageHeaderProxyAuthenticate : _kCFHTTPMessageHeaderWWWAuthenticate;

    else if (auth->_preferred)
        return CFDictionaryGetValue(auth->_preferred, propertyKey);

    else
        return NULL;
}


/* static */
CFTypeRef _CFHTTPAuthenticationLockAndCopyProperty(CFHTTPAuthenticationRef auth, CFStringRef propertyKey) {

    _CFMutexLock(&auth->_lock);

    CFTypeRef result = _CFHTTPAuthenticationGetProperty(auth, propertyKey);
    
    if (result) {
        CFTypeID id = CFGetTypeID(result);
        if (id == CFStringGetTypeID())
            result = CFStringCreateCopy(CFGetAllocator(result), result);
        else if (id == CFArrayGetTypeID())
            result = CFArrayCreateCopy(CFGetAllocator(result), result);
        else
            result = CFRetain(result);
    }

    _CFMutexUnlock(&auth->_lock);

    return result;
}


/* extern */ CFStreamError
_CFHTTPAuthenticationApplyHeaderToRequest(CFHTTPAuthenticationRef auth, CFHTTPMessageRef request, const void* connection) {
	
	CFStreamError result = {0, 0};
	
	_CFMutexLock(&auth->_lock);
	
	if (!auth->_error.error) {
		
		CFStringRef header = NULL;
		CFStringRef method = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod);
		
		if (method == kCFHTTPAuthenticationSchemeNTLM)
			header = _CFHTTPAuthenticationCreateNTLMHeaderForRequest(auth, request, connection);
		
		else if (method == kCFHTTPAuthenticationSchemeNegotiate)
			header = _CFHTTPAuthenticationCreateNegotiateHeaderForRequest(auth, request, connection);
		
		if (header) {
			
			if (!auth->_proxy)
				CFHTTPMessageSetHeaderFieldValue(request, _kCFHTTPMessageHeaderAuthorization, header);
			else
				CFHTTPMessageSetHeaderFieldValue(request, _kCFHTTPMessageHeaderProxyAuthorization, header);
			
			CFRelease(header);
		}
	} 
	
	memmove(&result, &(auth->_error), sizeof(result));
	
    _CFMutexUnlock(&auth->_lock);
	
	return result;
}


/* extern */ void
_CFHTTPAuthenticationDisassociateConnection(CFHTTPAuthenticationRef auth, const void* connection) {

	_CFMutexLock(&auth->_lock);
	
	CFDictionaryRemoveValue(auth->_connections, connection);
	
    _CFMutexUnlock(&auth->_lock);
}


/* extern */ CFArrayRef
_CFHTTPAuthenticationCopyServerSupportedSchemes(CFHTTPAuthenticationRef auth) {
	
	CFArrayRef result = NULL;
	
	if (CFHTTPAuthenticationIsValid(auth, NULL)) {

		_CFMutexLock(&auth->_lock);
		
		if (auth->_schemes) {
			
			CFStringRef static_buffer[16];
			CFStringRef* keys = &static_buffer[0];
			CFAllocatorRef alloc = CFGetAllocator(auth);
			CFIndex count = CFDictionaryGetCount(auth->_schemes);
			
			if (count > (sizeof(static_buffer) / sizeof(static_buffer[0])))
				keys = (CFStringRef*)CFAllocatorAllocate(alloc, count * sizeof(keys[0]), 0);
			
			if (keys) {
				
				CFDictionaryGetKeysAndValues(auth->_schemes, (const void**)keys, NULL);
				
				result = CFArrayCreate(alloc, (const void**)keys, count, &kCFTypeArrayCallBacks);
				
				if (keys != &static_buffer[0])
					CFAllocatorDeallocate(alloc, keys);
			}
		}
		
		_CFMutexUnlock(&auth->_lock);
	}
	
	return result;
}


/* extern */ Boolean
_CFHTTPAuthenticationSetPreferredScheme(CFHTTPAuthenticationRef auth, CFStringRef scheme) {
	
	Boolean result = FALSE;
	
	/* **FIXME** Currently does not check to make sure it hasn't been used. */
	
	_CFMutexLock(&auth->_lock);
	
	if (auth->_schemes) {
		
		CFMutableDictionaryRef replacement = (CFMutableDictionaryRef)CFDictionaryGetValue(auth->_schemes, scheme);
		
		if (replacement) {
				
			auth->_preferred = replacement;
			result = TRUE;
		}
	}
	
	_CFMutexUnlock(&auth->_lock);

	return result;
}


/* CF_EXPORT */
CFStringRef CFHTTPAuthenticationCopyRealm(CFHTTPAuthenticationRef auth) {
	
	CFStringRef result;
	
    _CFMutexLock(&auth->_lock);
	
    result = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyRealm);
    
	if (result)
		result = CFStringCreateCopy(CFGetAllocator(result), result);

	else {
	
		CFArrayRef domains = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyDomain);
		
		if (domains && CFArrayGetCount(domains))
			result = CFURLCopyHostName((CFURLRef)CFArrayGetValueAtIndex(domains, 0));
	}
	
    _CFMutexUnlock(&auth->_lock);
	
    return result;
}


/* CF_EXPORT */
CFArrayRef CFHTTPAuthenticationCopyDomains(CFHTTPAuthenticationRef auth) {
    return (CFArrayRef)_CFHTTPAuthenticationLockAndCopyProperty(auth, kCFHTTPAuthenticationPropertyDomain);
}


/* CF_EXPORT */
CFStringRef CFHTTPAuthenticationCopyMethod(CFHTTPAuthenticationRef auth) {
    return (CFStringRef)_CFHTTPAuthenticationLockAndCopyProperty(auth, kCFHTTPAuthenticationPropertyMethod);
}


/* CF_EXPORT */
Boolean CFHTTPAuthenticationRequiresUserNameAndPassword(CFHTTPAuthenticationRef auth) {

    Boolean result = TRUE;

    _CFMutexLock(&auth->_lock);
	
	/*
	** **FIXME** Under MacOS, this is totally fine since tickets and such are checked
	** up front and the scheme is known.  On Win32, negotiate may try to fall back to
	** NTLM.  This is not implemented correctly for this case.
	*/
	if (_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod) == kCFHTTPAuthenticationSchemeNegotiate)
		result = FALSE;

	_CFMutexUnlock(&auth->_lock);

    return result;
}


/* CF_EXPORT */
Boolean CFHTTPAuthenticationRequiresAccountDomain(CFHTTPAuthenticationRef auth) {

    Boolean result = FALSE;
	
    _CFMutexLock(&auth->_lock);

	if (_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod) == kCFHTTPAuthenticationSchemeNTLM)
        result = TRUE;
	
    _CFMutexUnlock(&auth->_lock);

    return result;
}


/* CF_EXPORT */
Boolean CFHTTPAuthenticationAllowsSingleSignOn(CFHTTPAuthenticationRef auth) {

#if defined(__WIN32__)
    _CFMutexLock(&auth->_lock);

    CFStringRef method = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod);

    _CFMutexUnlock(&auth->_lock);

    if (method && method == kCFHTTPAuthenticationSchemeNegotiate) {
        // See trickyness comment in CFHTTPAuthenticationRequiresUserNameAndPassword
        return CFHTTPAuthenticationRequiresUserNameAndPassword(auth);
    }
    else if (method && method == kCFHTTPAuthenticationSchemeNTLM)
        return TRUE;
    else
        return FALSE;
#else
    return FALSE;		// OSX has no single sign-on, yet
#endif
}


/* CF_EXPORT */
Boolean _CFHTTPAuthenticationPasswordInClear(CFHTTPAuthenticationRef auth) {

    _CFMutexLock(&auth->_lock);

    CFStringRef method = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod);

    _CFMutexUnlock(&auth->_lock);

    if (method && (method == kCFHTTPAuthenticationSchemeDigest
                   || method == kCFHTTPAuthenticationSchemeNegotiate
                   || method == kCFHTTPAuthenticationSchemeNTLM))
        return FALSE;
    else
        return TRUE;
}


/* extern */ Boolean
_CFHTTPAuthenticationConnectionAuthenticated(CFHTTPAuthenticationRef auth, const void* connection) {
	
	Boolean result = TRUE;
	
	_CFMutexLock(&auth->_lock);
	
	_AuthConnectionSpecific* specific = (_AuthConnectionSpecific*)CFDictionaryGetValue(auth->_connections, connection);
	
	if (specific) {
		
		CFStringRef method = _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod);
		
		if (method == kCFHTTPAuthenticationSchemeNTLM) {
			result = !specific->_negotiation && specific->_authdata && !specific->_ntlm;
		}
		
		else if (method == kCFHTTPAuthenticationSchemeNegotiate) {
			result = specific->_negotiation && specific->_authdata && !specific->_ntlm;
		}
	}
	
    _CFMutexUnlock(&auth->_lock);
	
	return result;
}


/* CF_EXPORT */
Boolean _CFHTTPMessageCanRetry(CFHTTPMessageRef response) {
	
	CFHTTPAuthenticationRef auth = _CFHTTPMessageGetAuthentication(response, (CFHTTPMessageGetResponseStatusCode(response) == 407));

	return auth ? CFHTTPAuthenticationIsValid(auth, NULL) : FALSE;
}


/* CF_EXPORT */
Boolean CFHTTPMessageAddAuthentication(CFHTTPMessageRef request, CFHTTPMessageRef authenticationFailureResponse, CFStringRef username, CFStringRef password, CFStringRef scheme, Boolean forProxy) {

    Boolean result = FALSE;
    CFHTTPAuthenticationRef auth = NULL;

    if (authenticationFailureResponse)
        auth = CFHTTPAuthenticationCreateFromResponse(CFGetAllocator(authenticationFailureResponse), authenticationFailureResponse);

    // Even though no one else could be using this auth, GetProperty needs us to be locked
    if (auth)
        _CFMutexLock(&auth->_lock);

    if (!scheme && auth) {
        scheme = (CFStringRef)_CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod);
		
		/* Clients of this API are not prepared for NTLM until it can be done entirely under the covers. */
		if (scheme == kCFHTTPAuthenticationSchemeNTLM) {
			
			CFMutableDictionaryRef replacement = (CFMutableDictionaryRef)CFDictionaryGetValue(auth->_schemes, kCFHTTPAuthenticationSchemeDigest);
			if (!replacement)
				replacement = (CFMutableDictionaryRef)CFDictionaryGetValue(auth->_schemes, kCFHTTPAuthenticationSchemeBasic);
			
			if (replacement)
				auth->_preferred = replacement;
			else
				scheme = NULL;
		}
	}

    // Careful, scheme may come from the client, do can't treat as an atom and do == compares
    if (scheme) {
        if (!CFStringCompare(scheme, kCFHTTPAuthenticationSchemeBasic, kCFCompareCaseInsensitive))
            result = _CFHTTPMessageSetBasicAuthenticationOnRequest(request, username, password, forProxy, NULL);
        else if (auth && !auth->_error.error
                 && !CFStringCompare(scheme, kCFHTTPAuthenticationSchemeDigest, kCFCompareCaseInsensitive)
                 && !CFStringCompare(scheme, _CFHTTPAuthenticationGetProperty(auth, kCFHTTPAuthenticationPropertyMethod), kCFCompareCaseInsensitive))
		{
			CFStringRef keys[] = {kCFHTTPAuthenticationUsername, kCFHTTPAuthenticationPassword};
			CFStringRef values[] = {username, password};
			CFDictionaryRef dict = CFDictionaryCreate(CFGetAllocator(request),
													  (const void**)(&keys[0]),
													  (const void**)(&values[0]),
													  sizeof(keys) / sizeof(keys[0]),
													  &kCFTypeDictionaryKeyCallBacks,
													  &kCFTypeDictionaryValueCallBacks);
            result = _CFApplyCredentials_Unsafe(request, auth, dict, NULL);
			CFRelease(dict);
		}
    }

    // This API is incapable of doing any multi-leg schemes, like NTLM.  The auth object would need to
    // persist across the multiple legs.
    if (auth) {
        _CFMutexUnlock(&auth->_lock);
        CFRelease(auth);
    }

    return result;
}
