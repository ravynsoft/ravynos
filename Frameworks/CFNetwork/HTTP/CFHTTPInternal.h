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
 *  CFHTTPInternal.h
 *  CFNetwork
 *  Copyright (c) 2003 Apple Computer. All rights reserved.
 */

#if !defined(__CFNETWORK_CFHTTPINTERNAL__)
#define __CFNETWORK_CFHTTPINTERNAL__ 1

#include "CFNetConnection.h"

#include "CFNetworkInternal.h"
#include "CFNetConnection.h"
#include <CFNetwork/CFNetworkPriv.h>

#define kCFStreamEventMarkEncountered (32)

#if defined(__cplusplus)
extern "C" {
#endif

extern CFHTTPAuthenticationRef _CFHTTPMessageGetAuthentication(CFHTTPMessageRef message, Boolean proxy);
extern void _CFHTTPMessageSetAuthentication(CFHTTPMessageRef message, CFHTTPAuthenticationRef auth, Boolean proxy);
extern void _CFHTTPMessageSetResponseURL(CFHTTPMessageRef response, CFURLRef url);

extern void _CFHTTPMessageSetHeader(CFHTTPMessageRef msg, CFStringRef theHeader, CFStringRef value, CFIndex position);

    
extern void _CFHTTPMessageSetLaxParsing(CFHTTPMessageRef msg, Boolean allowLaxParsing);
extern CFDataRef _CFHTTPMessageCopySerializedMessage(CFHTTPMessageRef msg, Boolean forProxy);
extern void _CFHTTPMessageSetHeader(CFHTTPMessageRef msg, CFStringRef theHeader, CFStringRef value, CFIndex position);
extern Boolean _CFHTTPMessageConvertToDataOnlyResponse(CFHTTPMessageRef message);
extern Boolean _CFHTTPMessageIsEmpty(CFHTTPMessageRef message);
extern Boolean _CFHTTPMessageCanStandAlone(CFHTTPMessageRef message);
extern CFDataRef _CFHTTPMessageGetBody(CFHTTPMessageRef msg);
extern Boolean _CFHTTPMessageIsGetMethod(CFHTTPMessageRef msg);

extern const CFStringRef _kCFStreamPropertyHTTPZeroLengthResponseExpected;
extern const CFStringRef _kCFStreamPropertyHTTPLaxParsing;
extern const CFStringRef _kCFStreamPropertyHTTPSProxyHoldYourFire;

// Internal support for persistant connection stuff
extern const CFStringRef _kCFStreamPropertyHTTPPersistent;
extern const CFStringRef _kCFStreamPropertyHTTPNewHeader;
extern const SInt32 _kCFStreamErrorHTTPStreamAtMark;

// Private HTTP error codes
extern const SInt32 _kCFStreamErrorHTTPSProxyFailure;

/* Mark management */
extern Boolean _CFHTTPReadStreamIsAtMark(CFReadStreamRef filteredStream);
extern void _CFHTTPReadStreamReadMark(CFReadStreamRef filteredStream);
extern void _CFHTTPWriteStreamWriteMark(CFWriteStreamRef filteredStream);

/* Utilities in CFHTTPStream.c */
extern void cleanUpRequest(CFHTTPMessageRef req, int length, Boolean forPersistentConnection, Boolean forProxy);
extern Boolean canKeepAlive(CFHTTPMessageRef responseHeaders, CFHTTPMessageRef request);
extern void emptyPerform(void *info);
extern CFStringRef _CFNetworkUserAgentString(void);

extern CFStringRef _CFEncodeBase64(CFAllocatorRef allocator, CFDataRef inputData);
extern CFDataRef _CFDecodeBase64(CFAllocatorRef allocator, CFStringRef inputStr);

extern const CFStringRef kCFHTTPAuthenticationSchemeBasic;
extern const CFStringRef kCFHTTPAuthenticationSchemeDigest;
extern const CFStringRef kCFHTTPAuthenticationSchemeNegotiate;
extern const CFStringRef kCFHTTPAuthenticationSchemeNTLM;

extern CFStringRef _CFCapitalizeHeader(CFStringRef headerString);
extern UInt8 *_CFURLPortionForRequest(CFAllocatorRef alloc, CFURLRef url, Boolean useCompleteURL, UInt8 **buf, CFIndex bufLength, Boolean *deallocateBuffer);

#if defined(__WIN32__)
extern void _CFHTTPMessageCleanup(void);
extern void _CFHTTPStreamCleanup(void);

// support from CFHTTPAUth-Win32.c

typedef struct _CFSSPIState *_CFSSPIStateRef;

extern Boolean _CFMD5(const UInt8* d, UInt32 n, UInt8* md, UInt32 md_length);
extern Boolean _CFSSPIPackageIsEnabled(const char *packageName);

extern _CFSSPIStateRef _CFCreateSSPIState(CFAllocatorRef alloc, CFStringRef method);
extern Boolean _CFTrySSPIHandshakeForHTTP(_CFSSPIStateRef sspi, CFStringRef username, CFStringRef password, CFStringRef principal, CFStringRef inputString, CFStringRef *outputString, CFStreamError* error);
extern void _CFFreeSSPIState(_CFSSPIStateRef sspi);
extern Boolean _CFCanTryKerberos(void);

#endif

#if defined(__cplusplus)
}
#endif

#endif /* ! __CFNETWORK_CFHTTPINTERNAL__ */

