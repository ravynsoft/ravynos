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

/*	CFSocketStream.c
	Copyright (c) 2000-2014, Apple Inc. All rights reserved.
	Responsibility: Jeremy Wyld
*/
//	Original Author: Becky Willrich
#include <CoreFoundation/CFStream.h>
#include <CoreFoundation/CFNumber.h>
#include "CFInternal.h"
#include "CFStreamInternal.h"
#include "CFStreamPriv.h"

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
// On Mach these live in CF for historical reasons, even though they are declared in CFNetwork

const int kCFStreamErrorDomainSSL = 3;
const int kCFStreamErrorDomainSOCKS = 5;

CONST_STRING_DECL(kCFStreamPropertyShouldCloseNativeSocket, "kCFStreamPropertyShouldCloseNativeSocket")
CONST_STRING_DECL(kCFStreamPropertyAutoErrorOnSystemChange, "kCFStreamPropertyAutoErrorOnSystemChange");

CONST_STRING_DECL(kCFStreamPropertySOCKSProxy, "kCFStreamPropertySOCKSProxy")
CONST_STRING_DECL(kCFStreamPropertySOCKSProxyHost, "SOCKSProxy")
CONST_STRING_DECL(kCFStreamPropertySOCKSProxyPort, "SOCKSPort")
CONST_STRING_DECL(kCFStreamPropertySOCKSVersion, "kCFStreamPropertySOCKSVersion")
CONST_STRING_DECL(kCFStreamSocketSOCKSVersion4, "kCFStreamSocketSOCKSVersion4")
CONST_STRING_DECL(kCFStreamSocketSOCKSVersion5, "kCFStreamSocketSOCKSVersion5")
CONST_STRING_DECL(kCFStreamPropertySOCKSUser, "kCFStreamPropertySOCKSUser")
CONST_STRING_DECL(kCFStreamPropertySOCKSPassword, "kCFStreamPropertySOCKSPassword")

CONST_STRING_DECL(kCFStreamPropertySocketSecurityLevel, "kCFStreamPropertySocketSecurityLevel");
CONST_STRING_DECL(kCFStreamSocketSecurityLevelNone, "kCFStreamSocketSecurityLevelNone");
CONST_STRING_DECL(kCFStreamSocketSecurityLevelSSLv2, "kCFStreamSocketSecurityLevelSSLv2");
CONST_STRING_DECL(kCFStreamSocketSecurityLevelSSLv3, "kCFStreamSocketSecurityLevelSSLv3");
CONST_STRING_DECL(kCFStreamSocketSecurityLevelTLSv1, "kCFStreamSocketSecurityLevelTLSv1");
CONST_STRING_DECL(kCFStreamSocketSecurityLevelNegotiatedSSL, "kCFStreamSocketSecurityLevelNegotiatedSSL");

#endif


#if DEPLOYMENT_TARGET_WINDOWS
typedef void (*CF_SOCKET_STREAM_PAIR)(CFAllocatorRef, CFStringRef, UInt32, CFSocketNativeHandle, const CFSocketSignature*, CFReadStreamRef*, CFWriteStreamRef*);
#endif

// These are duplicated in CFNetwork, who actually externs them in its headers
CONST_STRING_DECL(kCFStreamPropertySocketSSLContext, "kCFStreamPropertySocketSSLContext")
CONST_STRING_DECL(_kCFStreamPropertySocketSecurityAuthenticatesServerCertificate, "_kCFStreamPropertySocketSecurityAuthenticatesServerCertificate");


CF_EXPORT
void _CFSocketStreamSetAuthenticatesServerCertificateDefault(Boolean shouldAuthenticate) {
    CFLog(__kCFLogAssertion, CFSTR("_CFSocketStreamSetAuthenticatesServerCertificateDefault(): This call has been deprecated.  Use SetProperty(_kCFStreamPropertySocketSecurityAuthenticatesServerCertificate, kCFBooleanTrue/False)\n"));
}


/* CF_EXPORT */ Boolean
_CFSocketStreamGetAuthenticatesServerCertificateDefault(void) {
    CFLog(__kCFLogAssertion, CFSTR("_CFSocketStreamGetAuthenticatesServerCertificateDefault(): This call has been removed as a security risk.  Use security properties on individual streams instead.\n"));
    return FALSE;
}


/* CF_EXPORT */ void
_CFSocketStreamPairSetAuthenticatesServerCertificate(CFReadStreamRef rStream, CFWriteStreamRef wStream, Boolean authenticates) {
    
    CFBooleanRef value = (!authenticates ? kCFBooleanFalse : kCFBooleanTrue);
    
    if (rStream)
        CFReadStreamSetProperty(rStream, _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate, value);
    else
        CFWriteStreamSetProperty(wStream, _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate, value);
}


// Flags for dyld loading of libraries.
enum {
    kTriedToLoad = 0,
    kInitialized
};

static struct {
    CFLock_t lock;
    UInt32	flags;
#if DEPLOYMENT_TARGET_WINDOWS
    HMODULE	image;
#endif
    void (*_CFSocketStreamCreatePair)(CFAllocatorRef, CFStringRef, UInt32, CFSocketNativeHandle, const CFSocketSignature*, CFReadStreamRef*, CFWriteStreamRef*);
    CFErrorRef (*_CFErrorCreateWithStreamError)(CFAllocatorRef, CFStreamError*);
    CFStreamError (*_CFStreamErrorFromCFError)(CFErrorRef);
} CFNetworkSupport = {
    CFLockInit,
    0x0,
#if DEPLOYMENT_TARGET_WINDOWS
    NULL,
#endif
    NULL,
    NULL,
    NULL
};

#define CFNETWORK_CALL(sym, args)		((CFNetworkSupport.sym)args)

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#define CFNETWORK_LOAD_SYM(sym)   __CFLookupCFNetworkFunction(#sym)
#elif DEPLOYMENT_TARGET_WINDOWS
#define CFNETWORK_LOAD_SYM(sym)   (void *)GetProcAddress(CFNetworkSupport.image, #sym)
#endif

static void initializeCFNetworkSupport(void) {
    __CFBitSet(CFNetworkSupport.flags, kTriedToLoad);

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    CFNetworkSupport._CFSocketStreamCreatePair = CFNETWORK_LOAD_SYM(_CFSocketStreamCreatePair);
    CFNetworkSupport._CFErrorCreateWithStreamError = CFNETWORK_LOAD_SYM(_CFErrorCreateWithStreamError);
    CFNetworkSupport._CFStreamErrorFromCFError = CFNETWORK_LOAD_SYM(_CFStreamErrorFromCFError);
#elif DEPLOYMENT_TARGET_WINDOWS
    if (!CFNetworkSupport.image) {
#if _DEBUG
        CFNetworkSupport.image = GetModuleHandleW(L"CFNetwork_debug.dll");
#else
        CFNetworkSupport.image = GetModuleHandleW(L"CFNetwork.dll");
#endif
    }

    if (!CFNetworkSupport.image) {
        // not loaded yet, try to load from the filesystem
        char path[MAX_PATH+1];
        if (!CFNetworkSupport.image) {
            strlcpy(path, (const char *)_CFDLLPath(), sizeof(path));
#if _DEBUG
            strlcat(path, "\\CFNetwork_debug.dll", sizeof(path));
#else
            strlcat(path, "\\CFNetwork.dll", sizeof(path));
#endif
            CFNetworkSupport.image = LoadLibraryA(path);
        }
    }
            
    if (!CFNetworkSupport.image) {
        CFLog(__kCFLogAssertion, CFSTR("CoreFoundation: failed to dynamically load CFNetwork"));
    } else  {
        CFNetworkSupport._CFSocketStreamCreatePair = (CF_SOCKET_STREAM_PAIR)CFNETWORK_LOAD_SYM(_CFSocketStreamCreatePair);
        CFNetworkSupport._CFErrorCreateWithStreamError = (CFErrorRef(*)(CFAllocatorRef, CFStreamError *))CFNETWORK_LOAD_SYM(_CFErrorCreateWithStreamError);
        CFNetworkSupport._CFStreamErrorFromCFError = (CFStreamError(*)(CFErrorRef))CFNETWORK_LOAD_SYM(_CFStreamErrorFromCFError);
    }
#endif

    if (!CFNetworkSupport._CFSocketStreamCreatePair) CFLog(__kCFLogAssertion, CFSTR("CoreFoundation: failed to dynamically link symbol _CFSocketStreamCreatePair"));
    if (!CFNetworkSupport._CFErrorCreateWithStreamError) CFLog(__kCFLogAssertion, CFSTR("CoreFoundation: failed to dynamically link symbol _CFErrorCreateWithStreamError"));
    if (!CFNetworkSupport._CFStreamErrorFromCFError) CFLog(__kCFLogAssertion, CFSTR("CoreFoundation: failed to dynamically link symbol _CFStreamErrorFromCFError"));
            
    __CFBitSet(CFNetworkSupport.flags, kInitialized);
}

static void
createPair(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFSocketNativeHandle sock, const CFSocketSignature* sig, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream)
{
    if (readStream)
        *readStream = NULL;
        
    if (writeStream)
        *writeStream = NULL;

    __CFLock(&(CFNetworkSupport.lock));
    if (!__CFBitIsSet(CFNetworkSupport.flags, kTriedToLoad)) initializeCFNetworkSupport();
    __CFUnlock(&(CFNetworkSupport.lock));

    CFNETWORK_CALL(_CFSocketStreamCreatePair, (alloc, host, port, sock, sig, readStream, writeStream));
}


CF_EXPORT void CFStreamCreatePairWithSocket(CFAllocatorRef alloc, CFSocketNativeHandle sock, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream) {
    createPair(alloc, NULL, 0, sock, NULL, readStream, writeStream);
}

CF_EXPORT void CFStreamCreatePairWithSocketToHost(CFAllocatorRef alloc, CFStringRef host, UInt32 port, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream) {
    createPair(alloc, host, port, 0, NULL, readStream, writeStream);
}

CF_EXPORT void CFStreamCreatePairWithPeerSocketSignature(CFAllocatorRef alloc, const CFSocketSignature* sig, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream) {
    createPair(alloc, NULL, 0, 0, sig, readStream, writeStream);
}

CF_PRIVATE CFStreamError _CFStreamErrorFromError(CFErrorRef error) {
    CFStreamError result;
    Boolean canUpCall;
    
    __CFLock(&(CFNetworkSupport.lock));
    if (!__CFBitIsSet(CFNetworkSupport.flags, kTriedToLoad)) initializeCFNetworkSupport();
    canUpCall = (CFNetworkSupport._CFStreamErrorFromCFError != NULL);
    __CFUnlock(&(CFNetworkSupport.lock));

    if (canUpCall) {
        result = CFNETWORK_CALL(_CFStreamErrorFromCFError, (error));
    } else {
        CFStringRef domain = CFErrorGetDomain(error); 
        if (CFEqual(domain, kCFErrorDomainPOSIX)) {
            result.domain = kCFStreamErrorDomainPOSIX;
        } else if (CFEqual(domain, kCFErrorDomainOSStatus)) {
            result.domain = kCFStreamErrorDomainMacOSStatus;
        } else if (CFEqual(domain, kCFErrorDomainMach)) {
            result.domain = 11; // kCFStreamErrorDomainMach, but that symbol is in CFNetwork
        } else {
            result.domain = kCFStreamErrorDomainCustom;
        }
        result.error = CFErrorGetCode(error);
    }
    return result;
}

CF_PRIVATE CFErrorRef _CFErrorFromStreamError(CFAllocatorRef alloc, CFStreamError *streamError) {
    CFErrorRef result;
    Boolean canUpCall;
    
    __CFLock(&(CFNetworkSupport.lock));
    if (!__CFBitIsSet(CFNetworkSupport.flags, kTriedToLoad)) initializeCFNetworkSupport();
    canUpCall = (CFNetworkSupport._CFErrorCreateWithStreamError != NULL);
    __CFUnlock(&(CFNetworkSupport.lock));

    if (canUpCall) {
        result = CFNETWORK_CALL(_CFErrorCreateWithStreamError, (alloc, streamError));
    } else {
        if (streamError->domain == kCFStreamErrorDomainPOSIX) {
            return CFErrorCreate(alloc, kCFErrorDomainPOSIX, streamError->error, NULL);
        } else if (streamError->domain == kCFStreamErrorDomainMacOSStatus) {
            return CFErrorCreate(alloc, kCFErrorDomainOSStatus, streamError->error, NULL);
        } else {
            CFStringRef key = CFSTR("CFStreamErrorDomainKey");
            CFNumberRef value = CFNumberCreate(alloc, kCFNumberCFIndexType, &streamError->domain);
            CFDictionaryRef dict = CFDictionaryCreate(alloc, (const void **)(&key), (const void **)(&value), 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            result = CFErrorCreate(alloc, CFSTR("BogusCFStreamErrorCompatibilityDomain"), streamError->error, dict);
            CFRelease(value);
            CFRelease(dict);
        }
    }
    return result;
}
