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
 *  ProxySupport.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on 11/4/04.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#include "ProxySupport.h"

#include <CFNetwork/CFNetworkPriv.h>
#include "CFNetworkInternal.h"
#include <CoreFoundation/CFStreamPriv.h>

#ifndef __WIN32__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif


#if defined(__MACH__)
#include <JavaScriptGlue/JavaScriptGlue.h>
#include <SystemConfiguration/SystemConfiguration.h>


#ifdef __CONSTANT_CFSTRINGS__
#define _kProxySupportCFNetworkBundleID		CFSTR("com.apple.CFNetwork")
#define _kProxySupportLocalhost				CFSTR("localhost")
#define _kProxySupportIPv4Loopback			CFSTR("127.0.0.1")
#define _kProxySupportIPv6Loopback			CFSTR("::1")
#define _kProxySupportDot					CFSTR(".")
#define _kProxySupportSlash					CFSTR("/")
#define _kProxySupportDotZero				CFSTR(".0")
#define _kProxySupportDotFormat				CFSTR(".%@")
#define _kProxySupportStar					CFSTR("*")
#define _kProxySupportColon					CFSTR(":")
#define _kProxySupportSemiColon				CFSTR(";")
#define _kProxySupportFILEScheme			CFSTR("file")
#define _kProxySupportHTTPScheme			CFSTR("http")
#define _kProxySupportHTTPSScheme			CFSTR("https")
#define _kProxySupportHTTPPort				CFSTR("80")
#define _kProxySupportFTPScheme				CFSTR("ftp")
#define _kProxySupportFTPSScheme			CFSTR("ftps")
#define _kProxySupportFTPPort				CFSTR("21")
#define _kProxySupportSOCKS4Scheme			CFSTR("socks4")
#define _kProxySupportSOCKS5Scheme			CFSTR("socks5")
#define _kProxySupportSOCKSPort				CFSTR("1080")
#define _kProxySupportDIRECT				CFSTR("DIRECT")
#define _kProxySupportPROXY					CFSTR("PROXY")
#define _kProxySupportSOCKS					CFSTR("SOCKS")
#define _kProxySupportGETMethod				CFSTR("GET")
#define _kProxySupportURLLongFormat			CFSTR("%@://%@:%@@%@:%d")
#define _kProxySupportURLShortFormat		CFSTR("%@://%@:%d")
#define _kProxySupportExceptionsList		CFSTR("ExceptionsList")
#define _kProxySupportLoadingPacPrivateMode	CFSTR("_kProxySupportLoadingPacPrivateMode")
#define _kProxySupportPacSupportFileName	CFSTR("PACSupport")
#define _kProxySupportJSExtension			CFSTR("js")
#define _kProxySupportExpiresHeader			CFSTR("Expires")
#define _kProxySupportNowHeader                 CFSTR("Date")
#else
static CONST_STRING_DECL(_kProxySupportCFNetworkBundleID, "com.apple.CFNetwork")
static CONST_STRING_DECL(_kProxySupportLocalhost, "localhost")
static CONST_STRING_DECL(_kProxySupportIPv4Loopback, "127.0.0.1")
static CONST_STRING_DECL(_kProxySupportIPv6Loopback, "::1")
static CONST_STRING_DECL(_kProxySupportDot, ".")
static CONST_STRING_DECL(_kProxySupportSlash, "/")
static CONST_STRING_DECL(_kProxySupportDotZero, ".0")
static CONST_STRING_DECL(_kProxySupportDotFormat, ".%@")
static CONST_STRING_DECL(_kProxySupportStar, "*")
static CONST_STRING_DECL(_kProxySupportColon, ":")
static CONST_STRING_DECL(_kProxySupportSemiColon, ";")
static CONST_STRING_DECL(_kProxySupportFILEScheme, "file")
static CONST_STRING_DECL(_kProxySupportHTTPScheme, "http")
static CONST_STRING_DECL(_kProxySupportHTTPSScheme, "https")
static CONST_STRING_DECL(_kProxySupportHTTPPort, "80")
static CONST_STRING_DECL(_kProxySupportFTPScheme, "ftp")
static CONST_STRING_DECL(_kProxySupportFTPSScheme, "ftps")
static CONST_STRING_DECL(_kProxySupportFTPPort, "21")
static CONST_STRING_DECL(_kProxySupportSOCKS4Scheme, "socks4")
static CONST_STRING_DECL(_kProxySupportSOCKS5Scheme, "socks5")
static CONST_STRING_DECL(_kProxySupportSOCKSPort, "1080")
static CONST_STRING_DECL(_kProxySupportDIRECT, "DIRECT")
static CONST_STRING_DECL(_kProxySupportPROXY, "PROXY")
static CONST_STRING_DECL(_kProxySupportSOCKS, "SOCKS")
static CONST_STRING_DECL(_kProxySupportGETMethod, "GET")
static CONST_STRING_DECL(_kProxySupportURLLongFormat, "%@://%@:%@@%@:%d")
static CONST_STRING_DECL(_kProxySupportURLShortFormat, "%@://%@:%d")
static CONST_STRING_DECL(_kProxySupportExceptionsList, "ExceptionsList")
static CONST_STRING_DECL(_kProxySupportLoadingPacPrivateMode, "_kProxySupportLoadingPacPrivateMode")
static CONST_STRING_DECL(_kProxySupportPacSupportFileName, "PACSupport")
static CONST_STRING_DECL(_kProxySupportJSExtension, "js")
static CONST_STRING_DECL(_kProxySupportExpiresHeader, "Expires")
static CONST_STRING_DECL(_kProxySupportNowHeader, "Date")
#endif	/* __CONSTANT_CFSTRINGS__ */


static JSObjectRef _JSDnsResolveFunction(void* context, JSObjectRef ctxt, CFArrayRef args);
static JSObjectRef _JSPrimaryIpv4AddressesFunction(void* context, JSObjectRef ctxt, CFArrayRef args);

#elif defined(__WIN32__)

#include <winsock2.h>
#include <ws2tcpip.h>	// for ipv6
#include <wininet.h>	// for InternetTimeToSystemTime
                        // WinHTTP has the same function, but it has more OS/SP constraints
#include <objbase.h>

// RunGuts are defined below, with other COM code
typedef struct _JSRunGuts JSRun, *JSRunRef;

CF_EXPORT CFAbsoluteTime _CFAbsoluteTimeFromFileTime(FILETIME* ftime);

// Returns the path to the CF DLL, which we can then use to find resources
static const char *_CFDLLPath(void) {
    static TCHAR cachedPath[MAX_PATH+1] = "";
    
    if ('\0' == cachedPath[0]) {
#if defined(DEBUG)
        char *DLLFileName = "CFNetwork_debug";
#elif defined(PROFILE)
        char *DLLFileName = "CFNetwork_profile";
#else
        char *DLLFileName = "CFNetwork";
#endif
        HMODULE ourModule = GetModuleHandle(DLLFileName);
        assert(ourModule);      // GetModuleHandle failed to find our own DLL
        
        DWORD wResult = GetModuleFileName(ourModule, cachedPath, MAX_PATH+1);
        assert(wResult > 0);            // GetModuleFileName failure
        assert(wResult < MAX_PATH+1);   // GetModuleFileName result truncated
		
        // strip off last component, the DLL name
        CFIndex idx;
        for (idx = wResult - 1; idx; idx--) {
            if ('\\' == cachedPath[idx]) {
                cachedPath[idx] = '\0';
                break;
            }
        }
    }
    return cachedPath;
}

#endif	/* __WIN32__ */

static CFStringRef _JSFindProxyForURL(CFURLRef pac, CFURLRef url, CFStringRef host);
static CFStringRef _JSFindProxyForURLAsync(CFURLRef pac, CFURLRef url, CFStringRef host, Boolean *mustBlock);


#define PAC_STREAM_LOAD_TIMEOUT		30.0

static CFReadStreamRef BuildStreamForPACURL(CFAllocatorRef alloc, CFURLRef pacURL, CFURLRef targetURL, CFStringRef targetScheme, CFStringRef targetHost, _CFProxyStreamCallBack callback, void *clientInfo);
static CFStringRef _loadJSSupportFile(void);
static CFStringRef _loadPACFile(CFAllocatorRef alloc, CFURLRef pac, CFAbsoluteTime *expires, CFStreamError *err);
static JSRunRef _createJSRuntime(CFAllocatorRef alloc, CFStringRef js_support, CFStringRef js_pac);
static void _freeJSRuntime(JSRunRef runtime);
static CFStringRef _callPACFunction(CFAllocatorRef alloc, JSRunRef runtime, CFURLRef url, CFStringRef host);
static CFArrayRef _resolveDNSName(CFStringRef name);
static CFReadStreamRef _streamForPACFile(CFAllocatorRef alloc, CFURLRef pac, Boolean *isFile);
CFStringRef _stringFromLoadedPACStream(CFAllocatorRef alloc, CFMutableDataRef contents, CFReadStreamRef stream, CFAbsoluteTime *expires);
static void _JSSetEnvironmentForPAC(CFAllocatorRef alloc, CFURLRef url, CFAbsoluteTime expires, CFStringRef pacString);

/*
 ** Determine whether a given "enabled" entry ("HTTPEnable", "HTTPSEnable", ...) means 
 ** that the described proxy should be enabled.
 **
 ** Although this seems wrong, a NULL entry means the proxy SHOULD be enabled.  The
 ** idea is that a developer could create their own proxy dictionary which would be
 ** missing the "enable" flag.  In this case, the lack of the "enable" flag should 
 ** be interpreted as enabled if the other relevant proxy information is available 
 ** (e.g. the proxy host name).
 **
 ** Also, because SysConfig does little to no checking of its values, we must be prepared
 ** for an arbitrary CFTypeRef, just as if the value were coming out of CFPrefs.
 */
static inline Boolean _proxyEnabled(CFTypeRef enabledEntry) {
    if (!enabledEntry) return TRUE;
    if (CFGetTypeID(enabledEntry) == CFNumberGetTypeID()) {
        SInt32 val;
        CFNumberGetValue(enabledEntry, kCFNumberSInt32Type, &val);
        return (val != 0);
    }
    return (enabledEntry == kCFBooleanTrue);
}


// Currently not used, so hand dead-stripping for now.
#if 0
/* extern */ Boolean
_CFNetworkCFHostDoesNeedProxy(CFHostRef host, CFArrayRef bypasses, CFBooleanRef localBypass) {
    
    CFArrayRef names = CFHostGetNames(host, NULL);
	
	localBypass = _proxyEnabled(localBypass) ? kCFBooleanTrue : kCFBooleanFalse;
    
    if (names && CFArrayGetCount(names)) {
        return _CFNetworkDoesNeedProxy((CFStringRef)CFArrayGetValueAtIndex(names, 0), bypasses, localBypass);
    }
    
    names = CFHostGetAddressing(host, NULL);
    if (names && CFArrayGetCount(names)) {
        
        CFDataRef saData = (CFDataRef)CFArrayGetValueAtIndex(names, 0);
        CFStringRef name = stringFromAddr((const struct sockaddr*)CFDataGetBytePtr(saData), CFDataGetLength(saData));
        if (name) {
            Boolean result = _CFNetworkDoesNeedProxy(name, bypasses, localBypass);
            CFRelease(name);
            return result;
        }
        return FALSE;
    }
    
    return TRUE;
}
#endif


/* extern */ Boolean
_CFNetworkDoesNeedProxy(CFStringRef hostname, CFArrayRef bypasses, CFBooleanRef localBypass) {
	
    Boolean result = TRUE;
	
    CFIndex i, hostnc, count, length;
    CFArrayRef hostnodes;
	
	struct in_addr ip;
	Boolean is_ip = FALSE;
    
    if (!hostname) return TRUE;
	
	localBypass = _proxyEnabled(localBypass) ? kCFBooleanTrue : kCFBooleanFalse;
    
    if (CFStringCompare(hostname, _kProxySupportLocalhost, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        return FALSE;
    if (CFStringCompare(hostname, _kProxySupportIPv4Loopback, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        return FALSE;
    if (CFStringCompare(hostname, _kProxySupportIPv6Loopback, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
        return FALSE;
	
	length = CFStringGetLength(hostname);

	/* Uncomment the following code to bypass .local. addresses by default */
	/*
	if ((length > 7) && CFStringCompareWithOptions(hostname, CFSTR(".local."), CFRangeMake(length - 7, 7), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		return FALSE;
	*/
	
    if (localBypass && CFEqual(localBypass, kCFBooleanTrue) && CFStringFind(hostname, _kProxySupportDot, 0).location == kCFNotFound)
        return FALSE;
        
    count = bypasses ? CFArrayGetCount(bypasses) : 0;
    if (!count) return result;
    
    hostnodes = CFStringCreateArrayBySeparatingStrings(NULL, hostname, _kProxySupportDot);
    hostnc = hostnodes ? CFArrayGetCount(hostnodes) : 0;
    if (!hostnc) {
        CFRelease(hostnodes);
        hostnodes = NULL;
    }
    
	if (((hostnc == 4) || ((hostnc == 5) && (CFStringGetLength((CFStringRef)CFArrayGetValueAtIndex(hostnodes, 4)) == 0))) && 
		(length <= 16))
	{
		UInt8 stack_buffer[32];
		UInt8* buffer = stack_buffer;
		CFIndex bufferLength = sizeof(stack_buffer);
		CFAllocatorRef allocator = CFGetAllocator(hostname);
		
		buffer = _CFStringGetOrCreateCString(allocator, hostname, buffer, &bufferLength, kCFStringEncodingASCII);
		
		if (inet_pton(AF_INET, (const char*)buffer, &ip) == 1)
			is_ip = TRUE;
			
		if (buffer != stack_buffer)
			CFAllocatorDeallocate(allocator, buffer);
	}
	
    for (i = 0; result && (i < count); i++) {
		
        CFStringRef bypass = (CFStringRef)CFArrayGetValueAtIndex(bypasses, i);
        
        // Explicitely listed hosts gets bypassed
        if (CFStringCompare(hostname, bypass, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
            result = FALSE;
        }
        
		else if (is_ip && CFStringFindWithOptions(bypass, _kProxySupportSlash, CFRangeMake(0, CFStringGetLength(bypass)), 0, FALSE)) {
		
            CFArrayRef pieces = CFStringCreateArrayBySeparatingStrings(NULL, bypass, _kProxySupportSlash);
			
			if (pieces && (CFArrayGetCount(pieces) == 2)) {
				
				SInt32 cidr = CFStringGetIntValue((CFStringRef)CFArrayGetValueAtIndex(pieces, 1));
				if ((cidr > 0) && (cidr < 33)) {
				
					CFArrayRef bypassnodes = CFStringCreateArrayBySeparatingStrings(NULL,
																					(CFStringRef)CFArrayGetValueAtIndex(pieces, 0),
																					_kProxySupportDot);
																					
					if (bypassnodes) {
					
						CFIndex bypassnc = CFArrayGetCount(bypassnodes);
						if (bypassnc <= 4) {
							
							CFIndex n;
							CFMutableStringRef cp = CFStringCreateMutableCopy(CFGetAllocator(hostname),
																			  0,
																			  (CFStringRef)CFArrayGetValueAtIndex(bypassnodes, 0));
																			  
							for (n = 1; n < 4; n++) {
								
								if (n >= bypassnc)
									CFStringAppend(cp, _kProxySupportDotZero);
									
								else {
									
									CFStringRef piece = (CFStringRef)CFArrayGetValueAtIndex(bypassnodes, n);
									
									if (CFStringGetLength(piece))
										CFStringAppendFormat(cp, NULL, _kProxySupportDotFormat, piece);
									else
										CFStringAppend(cp, _kProxySupportDotZero);
								}
							}
							
							n = CFStringGetLength(cp);
							
							if (n <= 16) {
								UInt8 stack_buffer[32];
								UInt8* buffer = stack_buffer;
								CFIndex bufferLength = sizeof(stack_buffer);
								CFAllocatorRef allocator = CFGetAllocator(hostname);
								struct in_addr bypassip;
								
								buffer = _CFStringGetOrCreateCString(allocator, cp, buffer, &bufferLength, kCFStringEncodingASCII);
								
								if ((inet_pton(AF_INET, (const char*)buffer, &bypassip) == 1) &&
									(((((1 << cidr) - 1) << (32 - cidr)) & ntohl(*((uint32_t*)(&ip)))) == ntohl(*((uint32_t*)(&bypassip)))))
								{
									result = FALSE;
								}
									
								if (buffer != stack_buffer)
									CFAllocatorDeallocate(allocator, buffer);
							}
							
							CFRelease(cp);
						}
					
						CFRelease(bypassnodes);
					}
				}
			}

			if (pieces)
				CFRelease(pieces);
		}
		
        else if (hostnodes) {
        
            CFIndex bypassnc;
            CFArrayRef bypassnodes = CFStringCreateArrayBySeparatingStrings(NULL, bypass, _kProxySupportDot);
            
            if (!bypassnodes) continue;
            
            bypassnc = CFArrayGetCount(bypassnodes);
            if (bypassnc > 1) {
            
                CFIndex j = hostnc - 1;
                CFIndex k = bypassnc - 1;
                
                while ((j >= 0) && (k >= 0)) {
                
                    CFStringRef hostnode = (CFStringRef)CFArrayGetValueAtIndex(hostnodes, j);
                    CFStringRef bypassnode = (CFStringRef)CFArrayGetValueAtIndex(bypassnodes, k);
                    
					if ((k == 0) && (CFStringGetLength(bypassnode) == 0))
						bypassnode = _kProxySupportStar;
					
                    if (CFStringCompare(hostnode, bypassnode, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                        if (!j && !k) {
                            result = FALSE;
                            break;
                        }
                        else {
                            j--, k--;
                        }
                    }
                    
                    else if (CFStringCompare(bypassnode, _kProxySupportStar, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                        
                        while (k >= 0) {
                            bypassnode = (CFStringRef)CFArrayGetValueAtIndex(bypassnodes, k);

							if ((k == 0) && (CFStringGetLength(bypassnode) == 0))
								bypassnode = _kProxySupportStar;

                            if (CFStringCompare(bypassnode, _kProxySupportStar, kCFCompareCaseInsensitive) != kCFCompareEqualTo)
                                break;
                            k--;
                        }
                        
                        if (k < 0) {
                            result = FALSE;
                            break;
                        }
                        
                        else {
                            
                            while (j >= 0) {
                                
                                hostnode = (CFStringRef)CFArrayGetValueAtIndex(hostnodes, j);
                                if (CFStringCompare(bypassnode, hostnode, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
                                    break;
                                j--;
                            }
                            
                            if (j < 0)
                                break;
                        }
                    }
                    
                    else
                        break;
                }
            }
            
            CFRelease(bypassnodes);
        }
    }	

    if (hostnodes)
    CFRelease(hostnodes);
	
    return result;
}

static CFURLRef _URLForProxyEntry(CFAllocatorRef alloc, CFStringRef entry, CFIndex startIndex, CFStringRef scheme) {

    CFCharacterSetRef whitespaceSet = CFCharacterSetGetPredefined(kCFCharacterSetWhitespace);
    CFIndex len = CFStringGetLength(entry);
    CFRange colonRange = CFStringFind(entry, _kProxySupportColon, 0);

    CFStringRef host;
    CFStringRef portString;
    Boolean hasPort = TRUE;
    
    CFMutableStringRef urlString;
    CFURLRef url;
    
    if (colonRange.location == kCFNotFound) {
        colonRange.location = len;
        hasPort = FALSE;
    }
    while (startIndex < len && CFCharacterSetIsCharacterMember(whitespaceSet, CFStringGetCharacterAtIndex(entry, startIndex))) {
        startIndex ++;
    }
    if (startIndex >= colonRange.location) {
        return NULL;
    }
    host = CFStringCreateWithSubstring(CFGetAllocator(entry), entry, CFRangeMake(startIndex, colonRange.location - startIndex));

    if (hasPort) {
        portString = CFStringCreateWithSubstring(CFGetAllocator(entry), entry, CFRangeMake(colonRange.location + 1, len - colonRange.location - 1));
    } else if (CFStringCompare(scheme, _kProxySupportHTTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo || 
        CFStringCompare(scheme, _kProxySupportHTTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        portString = _kProxySupportHTTPPort;
        CFRetain(portString);
    } else if (CFStringCompare(scheme, _kProxySupportFTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        portString = _kProxySupportFTPPort;
        CFRetain(portString);
    } else if (CFStringCompare(scheme, _kProxySupportSOCKS4Scheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo || CFStringCompare(scheme, _kProxySupportSOCKS5Scheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
        portString = _kProxySupportSOCKSPort;
        CFRetain(portString);
    } else {
        CFRelease(host);
        return NULL;
    }
    
    urlString = CFStringCreateMutable(alloc, CFStringGetLength(scheme) + CFStringGetLength(host) + CFStringGetLength(portString) + 4);
    CFStringAppend(urlString, scheme);
    CFStringAppendCString(urlString, "://", kCFStringEncodingASCII);
    CFStringAppend(urlString, host);
    CFRelease(host);
    CFStringAppendCString(urlString, ":", kCFStringEncodingASCII);
    CFStringAppend(urlString, portString);
    CFRelease(portString);
    url = CFURLCreateWithString(NULL, urlString, NULL);
    CFRelease(urlString);
    return url;
}


static void _appendProxiesFromPACResponse(CFAllocatorRef alloc, CFMutableArrayRef proxyList, CFStringRef pacResponse, CFStringRef scheme) {
    CFArrayRef entries;
    CFIndex i, c;

    if (!pacResponse) return;

    entries = CFStringCreateArrayBySeparatingStrings(alloc, pacResponse, _kProxySupportSemiColon);                            
    c = CFArrayGetCount(entries);
    Boolean isFTP = (CFStringCompare(scheme, _kProxySupportFTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo);
        
    for (i = 0; i < c; i++) {
            
        CFURLRef to_add = NULL;
        CFStringRef untrimmedEntry = (CFStringRef)CFArrayGetValueAtIndex(entries, i);
        CFMutableStringRef entry = untrimmedEntry ? CFStringCreateMutableCopy(alloc, CFStringGetLength(untrimmedEntry), untrimmedEntry) : NULL;
        CFIndex entryLength;
        CFRange range;
        
        if (!entry)  continue;
        CFStringTrimWhitespace(entry);
        entryLength = CFStringGetLength(entry);
        if (entryLength >= 6 && CFStringFindWithOptions(entry, _kProxySupportDIRECT, CFRangeMake(0, 6), kCFCompareCaseInsensitive, NULL)) {
            CFArrayAppendValue(proxyList, kCFNull);
            // NOTE that "to_add" is not changed and the array is altered directly.
        }

        else if (entryLength >= 5 && CFStringFindWithOptions(entry, _kProxySupportPROXY, CFRangeMake(0, 5), kCFCompareCaseInsensitive, &range)) {
            CFIndex urlStart = 5;
            to_add = _URLForProxyEntry(CFGetAllocator(proxyList), entry, urlStart, scheme);
            
            // In the case of FTP, dump an extra entry to try FTP over HTTP.
            if (to_add && isFTP) {
                CFURLRef extra = _URLForProxyEntry(CFGetAllocator(proxyList), entry, urlStart, _kProxySupportHTTPScheme);
                if (extra) {
                    CFArrayAppendValue(proxyList, extra);
                    CFRelease(extra);
                }
            }
        }
        
        else if (entryLength >= 5 && CFStringFindWithOptions(entry, _kProxySupportSOCKS, CFRangeMake(0, 5), kCFCompareCaseInsensitive, &range)) {
            to_add = _URLForProxyEntry(CFGetAllocator(proxyList), entry, 5, _kProxySupportSOCKS5Scheme);
        }
        
        if (to_add) {
            CFArrayAppendValue(proxyList, to_add);
            CFRelease(to_add);
        }
        CFRelease(entry);
    }
    CFRelease(entries);
}

static CFURLRef proxyURLForComponents(CFAllocatorRef alloc, CFStringRef scheme, CFStringRef host, SInt32 port, CFStringRef user, CFStringRef password) {
    CFStringRef urlString;
    CFURLRef url;
    
    if (user) {
        urlString = CFStringCreateWithFormat(alloc, NULL, _kProxySupportURLLongFormat, scheme, user, password, host, port);
    } else {
        urlString = CFStringCreateWithFormat(alloc, NULL, _kProxySupportURLShortFormat, scheme, host, port);
    }
    url = CFURLCreateWithString(alloc, urlString, NULL);
    CFRelease(urlString);
    return url;
}


/* Synchronous if callback == NULL. */
/* extern */ CFMutableArrayRef
_CFNetworkFindProxyForURLAsync(CFStringRef scheme, CFURLRef url, CFStringRef host, CFDictionaryRef proxies, _CFProxyStreamCallBack callback, void *clientInfo, CFReadStreamRef *proxyStream) {

    // Priority of proxies in typical usage scenarios
    //		1.  pac file
    //		2.  protocol specific proxy
    //		3.  SOCKS
    //		4.  direct

    CFAllocatorRef alloc = url ? CFGetAllocator(url) : host ? CFGetAllocator(host) : proxies ? CFGetAllocator(proxies) : kCFAllocatorDefault;
    CFMutableArrayRef result = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
	
    if (!proxies) {
        CFArrayAppendValue(result, kCFNull);
        return result;
    }
    
    if (host) {
        CFRetain(host);
    } else {
        if (!url) {
            CFArrayAppendValue(result, kCFNull);
            return result;
        }
        host = CFURLCopyHostName(url);
		if (!host) {
			CFArrayAppendValue(result, kCFNull);
			return result;
		}
    }

    if (!scheme)
        scheme = url ? CFURLCopyScheme(url) : NULL;
    else
        CFRetain(scheme);

    do {
        /*
        ** Note that once a proxy or list of proxies is deteremined for the given
        ** url/host, the code will dump out of the do...while loop.  This allows fall
        ** through in proxy setup attempts according to the previously mentioned priorities.
        */
        CFStringRef proxy;
        CFNumberRef port;
        // On Windows we don't look for *Enabled* keys, so this just stays NULL
        CFTypeRef enabled = NULL;
        CFArrayRef bypass = (CFArrayRef)CFDictionaryGetValue(proxies, _kProxySupportExceptionsList);
        CFBooleanRef localBypass = _proxyEnabled(CFDictionaryGetValue(proxies, kCFStreamPropertyProxyLocalBypass)) ? kCFBooleanTrue : kCFBooleanFalse;
        
        if (bypass && CFGetTypeID(bypass) != CFArrayGetTypeID()) {
            bypass = NULL;
        }

        if (scheme) {

            SInt32 default_port = 0;
            CFStringRef proxy_key = NULL, port_key = NULL;
#if defined(__MACH__)
            CFStringRef enable_key = NULL;
#endif
            
#if defined(__MACH__)
            enabled = CFDictionaryGetValue(proxies, _kCFStreamPropertyHTTPProxyProxyAutoConfigEnable);
#endif
            if (_proxyEnabled(enabled)) {

                proxy = (CFStringRef)CFDictionaryGetValue(proxies, _kCFStreamPropertyHTTPProxyProxyAutoConfigURLString);

                // If PAC file exists, attempt it.
                if (proxy && CFGetTypeID(proxy) == CFStringGetTypeID()) {

                    CFStringRef url_str = CFURLCreateStringByAddingPercentEscapes(alloc, proxy, NULL, NULL, kCFStringEncodingUTF8);
                    CFURLRef pac = CFURLCreateWithString(alloc, url_str, NULL);
                    CFRelease(url_str);

                    if (pac) {
                        Boolean mustLoad = FALSE;
                        CFStringRef list;
                        if (callback) {
                            // Asynchronous
                            list = _JSFindProxyForURLAsync(pac, url, host, &mustLoad);
                        } else {
                             list = _JSFindProxyForURL(pac, url, host);
                             mustLoad = FALSE;
                        }

                        if (mustLoad) {
                            *proxyStream = BuildStreamForPACURL(alloc, pac, url, scheme, host, callback, clientInfo);
                            CFRelease(result);
                            result = NULL; // NULL means async load in progress
                        } else {
                            _appendProxiesFromPACResponse(alloc, result, list, scheme);
                            if (list) CFRelease(list);
                        }
                        CFRelease(pac);
                    }

                    break;	// Proxy list should be set.  If there were failures, don't continue to
                }		// progress because the PAC file was to be consulted and used.
            }

            // No PAC file so attempt the scheme specific proxy.
            if (CFStringCompare(scheme, _kProxySupportHTTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                proxy_key = kCFStreamPropertyHTTPProxyHost;
                port_key = kCFStreamPropertyHTTPProxyPort;
#if defined(__MACH__)
                enable_key = kSCPropNetProxiesHTTPEnable;
#endif
                default_port = 80;
            }
            else if (CFStringCompare(scheme, _kProxySupportHTTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                proxy_key = kCFStreamPropertyHTTPSProxyHost;
                port_key = kCFStreamPropertyHTTPSProxyPort;
#if defined(__MACH__)
                enable_key = kSCPropNetProxiesHTTPSEnable;
#endif
                default_port = 80;
            }
            else if (CFStringCompare(scheme, _kProxySupportFTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                proxy_key = kCFStreamPropertyFTPProxyHost;
                port_key = kCFStreamPropertyFTPProxyPort;
#if defined(__MACH__)
                enable_key = kSCPropNetProxiesFTPEnable;
#endif
                default_port = 21;
            }
            else if (CFStringCompare(scheme, _kProxySupportFTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) {
                proxy_key = kCFStreamPropertyFTPProxyHost;
                port_key = kCFStreamPropertyFTPProxyPort;
#if defined(__MACH__)
                enable_key = kSCPropNetProxiesFTPEnable;
#endif
                default_port = 990;
            }
            
            if (proxy_key) {
#if defined(__MACH__)
                enabled = CFDictionaryGetValue(proxies, enable_key);
#endif
                if (_proxyEnabled(enabled)) {

                    proxy = (CFStringRef)CFDictionaryGetValue(proxies, proxy_key);
                    if (proxy && CFGetTypeID(proxy) == CFStringGetTypeID() && _CFNetworkDoesNeedProxy(host, bypass, localBypass)) {
                        CFURLRef to_add;
                        SInt32 portNum;

                        port = (CFNumberRef)CFDictionaryGetValue(proxies, port_key);
                        if (!port || CFGetTypeID(port) != CFNumberGetTypeID() || !CFNumberGetValue(port, kCFNumberSInt32Type, &portNum)) {
                            portNum = default_port;
                        }

                        to_add = proxyURLForComponents(alloc, scheme, proxy, portNum, NULL, NULL);
                        if (proxy_key == kCFStreamPropertyFTPProxyHost) {
                            CFURLRef extra = proxyURLForComponents(alloc, _kProxySupportHTTPScheme, proxy, portNum == 21 ? 80 : portNum, NULL, NULL);
                            if (extra) {
                                CFArrayAppendValue(result, extra);
                                CFRelease(extra);
                            }
                        }
                        if (to_add) {
                            CFArrayAppendValue(result, to_add);
                            CFRelease(to_add);
                        }

                        break;	// Proxy list is set.  If the URL creation failed, don't continue on because
                    }		// nothing else should not be attempted.  It's better to return an empty list.
                }
            }
        }

#if defined(__MACH__)
        enabled = CFDictionaryGetValue(proxies, kSCPropNetProxiesSOCKSEnable);
#endif
        if (_proxyEnabled(enabled)) {

            CFStringRef user = (CFStringRef)CFDictionaryGetValue(proxies, kCFStreamPropertySOCKSUser);
            CFStringRef pass = (CFStringRef)CFDictionaryGetValue(proxies, kCFStreamPropertySOCKSPassword);
            CFStringRef version = (CFStringRef)CFDictionaryGetValue(proxies, kCFStreamPropertySOCKSVersion);
            
            proxy = (CFStringRef)CFDictionaryGetValue(proxies, kCFStreamPropertySOCKSProxyHost);

            if (proxy && CFGetTypeID(proxy) == CFStringGetTypeID() && (!bypass || !host || _CFNetworkDoesNeedProxy(host, bypass, localBypass))) {
                CFStringRef socksScheme;
                SInt32 portNum;
                CFURLRef to_add;
				
                port = (CFNumberRef)CFDictionaryGetValue(proxies, kCFStreamPropertySOCKSProxyPort);
                if (!port || CFGetTypeID(port) != CFNumberGetTypeID() || !CFNumberGetValue(port, kCFNumberSInt32Type, &portNum)) {
                    portNum = 1080;
                }
                if (!user || !pass || CFGetTypeID(user) != CFStringGetTypeID() || CFGetTypeID(pass) != CFStringGetTypeID()) {
                    user = NULL;
                    pass = NULL;
                }
                socksScheme = (version && CFEqual(version, kCFStreamSocketSOCKSVersion4)) ? _kProxySupportSOCKS4Scheme : _kProxySupportSOCKS5Scheme;
                to_add = proxyURLForComponents(alloc, socksScheme, proxy, portNum, user, pass);
                if (to_add) {
                    CFArrayAppendValue(result, to_add);
                    CFRelease(to_add);
                }

                break;	// Proxy list is set.  If the URL creation failed, don't put DIRECT in the list
            }		// because direct should not be attempted.  It's better to return an empty list.
        }

        // Gotten this far, so only direct is left.
        CFArrayAppendValue(result, kCFNull);

    } while (0);

    if (scheme) CFRelease(scheme);
    if (host) CFRelease(host);
    return result;
}

static void
_ReadStreamClientCallBack(CFReadStreamRef stream, CFStreamEventType type, CFMutableDataRef contents) {

    if (type == kCFStreamEventHasBytesAvailable) {
        UInt8 buffer[8192];
        CFIndex bytesRead = CFReadStreamRead(stream, buffer, sizeof(buffer));
        if (bytesRead > 0)
            CFDataAppendBytes(contents, buffer, bytesRead);
    }
}


static void
_RunLoopTimerCallBack(CFRunLoopTimerRef timer, Boolean* timedout) {

    *timedout = TRUE;
}


static CFStreamError
_LoadStreamIntoData(CFReadStreamRef stream, CFMutableDataRef contents, CFTimeInterval timeout, Boolean isFile) {

    CFStreamError result = {0, 0};
    CFRunLoopRef rl = CFRunLoopGetCurrent();
    CFStreamClientContext ctxt = {0, contents, NULL, NULL, NULL};

    CFReadStreamSetClient(stream,
                          kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred,
                          (CFReadStreamClientCallBack)_ReadStreamClientCallBack,
                          &ctxt);
    CFReadStreamScheduleWithRunLoop(stream, rl, _kProxySupportLoadingPacPrivateMode);

    if (!CFReadStreamOpen(stream))
        result = CFReadStreamGetError(stream);
    else {
        Boolean timedout = FALSE;
        CFRunLoopTimerContext timerCtxt = {0, &timedout, NULL, NULL, NULL};
        CFRunLoopTimerRef t = timeout ? CFRunLoopTimerCreate(CFGetAllocator(stream),
                                                             CFAbsoluteTimeGetCurrent() + timeout,
                                                             0,
                                                             0,
                                                             0,
                                                             (CFRunLoopTimerCallBack)_RunLoopTimerCallBack,
                                                             &timerCtxt) : NULL;

        if (t)
            CFRunLoopAddTimer(rl, t, _kProxySupportLoadingPacPrivateMode);

        do {
            CFStreamStatus status = CFReadStreamGetStatus(stream);
            if (status == kCFStreamStatusAtEnd || status == kCFStreamStatusError)
                break;
            
            CFRunLoopRunInMode(_kProxySupportLoadingPacPrivateMode, 1e+20, TRUE);
            
        } while (!timedout);
        
        if (t) {
            CFRunLoopRemoveTimer(rl, t, _kProxySupportLoadingPacPrivateMode);
            CFRelease(t);
        }

        result = CFReadStreamGetError(stream);
        if (result.error || timedout) {
            CFIndex len = CFDataGetLength(contents);
            if (len)
                CFDataDeleteBytes(contents, CFRangeMake(0, len));
        }

        if (timedout) {
            result.domain = kCFStreamErrorDomainCustom;
            result.error = -1;
        }
        
        CFReadStreamClose(stream);
    }

    CFReadStreamUnscheduleFromRunLoop(stream, rl, _kProxySupportLoadingPacPrivateMode);
    CFReadStreamSetClient(stream, 0, NULL, NULL);

    return result;
}


/* static */ CFStringRef
_loadJSSupportFile(void) {
    
    static CFURLRef _JSRuntimeFunctionsLocation = NULL;
    static CFStringRef _JSRuntimeFunctions = NULL;

    if (_JSRuntimeFunctionsLocation == NULL) {
#if !defined(__WIN32__)
        CFBundleRef cfNetworkBundle = CFBundleGetBundleWithIdentifier(_kProxySupportCFNetworkBundleID);
        _JSRuntimeFunctionsLocation = CFBundleCopyResourceURL(cfNetworkBundle, _kProxySupportPacSupportFileName, _kProxySupportJSExtension, NULL);
#else
        static UInt8 path[MAX_PATH+1];
        const char *resourcePath = _CFDLLPath();
        strcpy(path, resourcePath);
        strcat(path, "\\PACSupport.js");
        _JSRuntimeFunctionsLocation = CFURLCreateFromFileSystemRepresentation(NULL, path, strlen(path), FALSE);
#endif

        CFReadStreamRef stream = CFReadStreamCreateWithFile(NULL, _JSRuntimeFunctionsLocation);
        if (stream) {
        
            CFMutableDataRef contents = CFDataCreateMutable(NULL, 0);
        
            // NOTE that the result value is not taken here since this is the load
            // of the local support file.  If that fails, DIRECT should not be
            // the fallback as will happen with the actual PAC file.
            _LoadStreamIntoData(stream, contents, PAC_STREAM_LOAD_TIMEOUT, TRUE);
            CFRelease(stream);
            
            CFIndex bytesRead = CFDataGetLength(contents);
            // Check to see if read until the end of the file.
            if (bytesRead) {
                _JSRuntimeFunctions = CFStringCreateWithBytes(NULL, CFDataGetBytePtr(contents), bytesRead, kCFStringEncodingUTF8, TRUE);
            }
            CFRelease(contents);
        }
    }
    return _JSRuntimeFunctions;
}


static CFReadStreamRef _streamForPACFile(CFAllocatorRef alloc, CFURLRef pac, Boolean *isFile) {
    CFStringRef scheme = CFURLCopyScheme(pac);
    CFReadStreamRef stream = NULL;
    *isFile = FALSE;

    if (!scheme || (CFStringCompare(scheme, _kProxySupportFILEScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
        stream = CFReadStreamCreateWithFile(alloc, pac);
        *isFile = TRUE;
    }
    
    else if ((CFStringCompare(scheme, _kProxySupportHTTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) ||
             (CFStringCompare(scheme, _kProxySupportHTTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo))
    {
        CFHTTPMessageRef msg = CFHTTPMessageCreateRequest(alloc, _kProxySupportGETMethod, pac, kCFHTTPVersion1_0);
        if (msg) {
            stream = CFReadStreamCreateForHTTPRequest(alloc, msg);
			if (stream)
				CFReadStreamSetProperty(stream, kCFStreamPropertyHTTPShouldAutoredirect, kCFBooleanTrue);
            CFRelease(msg);
        }
    }
    
    else if ((CFStringCompare(scheme, _kProxySupportFTPScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo) ||
             (CFStringCompare(scheme, _kProxySupportFTPSScheme, kCFCompareCaseInsensitive) == kCFCompareEqualTo))
    {
        stream = CFReadStreamCreateWithFTPURL(alloc, pac);
    }
    return stream;
}

CFStringRef _stringFromLoadedPACStream(CFAllocatorRef alloc, CFMutableDataRef contents, CFReadStreamRef stream, CFAbsoluteTime *expires) {
    CFHTTPMessageRef msg = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
    CFStringRef result = NULL;
    CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    *expires = now > 1 ? now - 1 : now; // Just some number that's less than now.

    if (msg) {
		
        UInt32 code = CFHTTPMessageGetResponseStatusCode(msg);
        
        if (code > 299) {
            CFDataSetLength(contents, 0);
        }
        
        else {
            
            CFStringRef expiryString = CFHTTPMessageCopyHeaderFieldValue(msg, _kProxySupportExpiresHeader);
            
            if (expiryString) {
                CFGregorianDate expiryDate;
                CFTimeZoneRef expiryTZ = NULL;
                
                if (_CFGregorianDateCreateWithString(alloc, expiryString, &expiryDate, &expiryTZ)) {
                    CFStringRef nowString = CFHTTPMessageCopyHeaderFieldValue(msg, _kProxySupportNowHeader);
		    if (nowString) {
                        CFTimeZoneRef nowTZ;
                        CFGregorianDate nowDate;
                        if (_CFGregorianDateCreateWithString(alloc, nowString, &nowDate, &nowTZ)) {
                            *expires = now + (CFGregorianDateGetAbsoluteTime(expiryDate, expiryTZ) - CFGregorianDateGetAbsoluteTime(nowDate, nowTZ));
                        } else {
                            *expires = CFGregorianDateGetAbsoluteTime(expiryDate, expiryTZ);
                        }
                        CFRelease(nowString);
                    } else {
                        *expires = CFGregorianDateGetAbsoluteTime(expiryDate, expiryTZ);
                    }
                }

                CFRelease(expiryString);
            }
        }
        
        CFRelease(msg);
    }
    
    if (*expires < now) {
        *expires = now + (24 * 60 * 60);
    }

    CFIndex bytesRead = CFDataGetLength(contents);
    if (bytesRead) {
        result = CFStringCreateWithBytes(alloc, CFDataGetBytePtr(contents), bytesRead, kCFStringEncodingUTF8, TRUE);
    }
    return result;
}

/* static */ CFStringRef
_loadPACFile(CFAllocatorRef alloc, CFURLRef pac, CFAbsoluteTime *expires, CFStreamError *err) {
    Boolean isFile;
    CFReadStreamRef stream = _streamForPACFile(alloc, pac, &isFile);
    CFStringRef result = NULL;

    // Load pac file at url location.
    if (stream) {
        CFMutableDataRef contents = CFDataCreateMutable(alloc, 0);
        *err = _LoadStreamIntoData(stream, contents, PAC_STREAM_LOAD_TIMEOUT, isFile);
        
        if (err->domain == 0) {
            result = _stringFromLoadedPACStream(alloc, contents, stream, expires);
        }
        CFRelease(contents);
        CFRelease(stream);
    }
    return result;
}


#if !defined(__WIN32__)

/* static */ JSRunRef
_createJSRuntime(CFAllocatorRef alloc, CFStringRef js_support, CFStringRef js_pac) {

    CFStringRef allCode = CFStringCreateWithFormat(alloc, NULL, CFSTR("%@\n%@\n"), js_support, js_pac);
    JSRunRef runtime = NULL;
    if (allCode) {
        JSLockInterpreter();
        runtime = JSRunCreate(allCode, 0);
        JSUnlockInterpreter();
    }
    
    if (!JSRunCheckSyntax(runtime)) {
        JSRelease(runtime);
        runtime = NULL;
    }
    
    if (runtime) {
        
        JSObjectRef g = JSRunCopyGlobalObject(runtime);
        JSObjectCallBacks c = {NULL, NULL, NULL, NULL,  NULL, NULL, NULL};
        
        if (g) {
            
            JSObjectRef func;
            
            c.callFunction = (JSObjectCallFunctionProcPtr)_JSDnsResolveFunction;
            JSLockInterpreter();
            func = JSObjectCreate(NULL, &c);
            
            if (func) {
                JSObjectSetProperty(g, CFSTR("__dnsResolve"), func);
                JSRelease(func);
            }
            
            c.callFunction = (JSObjectCallFunctionProcPtr)_JSPrimaryIpv4AddressesFunction;
            func = JSObjectCreate(NULL, &c);
            
            if (func) {
                JSObjectSetProperty(g, CFSTR("__primaryIPv4Addresses"), func);
                JSRelease(func);
            }
            
            JSObjectRef strObj = JSObjectCreateWithCFType(CFSTR("MACH"));
            JSObjectSetProperty(g, CFSTR("__platformName"), strObj);
            JSRelease(strObj);
            
            JSUnlockInterpreter();
            
            JSRelease(g);
        }
        
        g = JSRunEvaluate(runtime);
        if (g) JSRelease(g);
    }
    return runtime;
}


/* static */ void
_freeJSRuntime(JSRunRef runtime) {
    JSRelease(runtime);
}


/* static */ CFStringRef
_callPACFunction(CFAllocatorRef alloc, JSRunRef runtime, CFURLRef url, CFStringRef host) {

    JSObjectRef jsResult = NULL;
    CFStringRef result = NULL;
    JSObjectRef g = runtime ? JSRunCopyGlobalObject(runtime) : NULL;
    JSObjectRef func = g ? JSObjectCopyProperty(g, CFSTR("__Apple_FindProxyForURL")) : NULL;
    
    if (func) {
        CFArrayRef cfArgs = NULL;
        CFMutableArrayRef jsArgs;
        CFTypeRef args[2];
        CFURLRef absURL = CFURLCopyAbsoluteURL(url);
        
        args[0] = CFURLGetString(absURL);
        args[1] = host;
        
        cfArgs = CFArrayCreate(alloc, args, 2, &kCFTypeArrayCallBacks);
        CFRelease(absURL);
        
        JSLockInterpreter();
        jsArgs = JSCreateJSArrayFromCFArray(cfArgs);
        CFRelease(cfArgs);
        
        jsResult = JSObjectCallFunction(func, g, jsArgs);
        JSUnlockInterpreter();
        JSRelease(func);
        CFRelease(jsArgs);
        
        if (jsResult) {
            result = (CFStringRef)JSObjectCopyCFValue(jsResult);
            JSRelease(jsResult);
            if (result && CFGetTypeID(result) != CFStringGetTypeID()) {
				CFRelease(result);
				result = NULL;
            }
        }
    }
    
    if (g) JSRelease(g);

    //CFLog(0, CFSTR("FindProxyForURL returned: %@"), result);
    return result;
}


#else

/*
 Doc on scripting interfaces:
     http://msdn.microsoft.com/library/en-us/script56/html/scripting.asp
 Doc on IDispatch interface:
     http://msdn.microsoft.com/library/en-us/automat/htm/chap5_78v9.asp
 Doc on passing arrays though IDispatch:
     http://msdn.microsoft.com/library/en-us/automat/htm/chap7_5dyr.asp
 
 COM code is cobbled together from the following samples:
     http://www.microsoft.com/msj/1099/visualprog/visualprog1099.aspx
     http://www.codeproject.com/com/mfcscripthost.asp
     http://www.microsoft.com/mind/0297/activescripting.asp
     MSDN KB 183698: http://support.microsoft.com:80/support/kb/articles/q183/6/98.asp
 
 Info on doing COM in C vs C++
     http://msdn.microsoft.com/library/en-us/dncomg/html/msdn_com_co.asp
 
 COM memory management rules, for items pass by reference and interfaces
     http://msdn.microsoft.com/library/en-us/com/htm/com_3rub.asp
     http://msdn.microsoft.com/library/en-us/com/htm/com_1vxv.asp
 
 Big scripting FAQ, including how to reuse a script engine with cloning - we don't do this yet
 but maybe it would be an interesting optimization.
     http://www.mindspring.com/~mark_baker/
 
 I found mixed info as to how compatible GCC's vtables are with COM.  I didn't try a C++ implementation
 yet, but there is some chance that it work work with GCC.
*/

/*
 Originally on Windows we tried doing PAC using the WinHTTP library, but that has problems:
 - Does not support ftp target URLs
 - PAC files returning "SOCKS foo" generate a result of DIRECT
 - No way to intersperse a DIRECT result with explicit server results, so no control over failover.

 The impl code is in CVS in revision 1.13.4.3

 WinHTTP PAC main link:
 http://msdn.microsoft.com/library/en-us/winhttp/http/winhttp_autoproxy_support.asp

 WinInet Pac main link:
 http://msdn.microsoft.com/library/en-us/wininet/wininet/autoproxy_support_in_wininet.asp

 Note the WinInet stuff says their support may go away, and recommends using WinHTTP.  WinHTTP only
 has PAC in version 5.1, shipped with Win2K+SP3 and WinXP+SP1.  It is NOT redistributable for running
 on the earlier SP's.
 */

// WinHttpGetIEProxyConfigForCurrentUser could also be used to get the user settings from IE.
// WinHttpGetDefaultProxyConfiguration could be used to get settings from the registry. (non-IE)


#define COBJMACROS
// Note: this one isn't in the Cygwin set, had to copy this file from MSVC.
#include <ACTIVSCP.H>

//#define DO_COM_LOGGING
#ifdef DO_COM_LOGGING
#define COM_LOG printf
#else
#define COM_LOG while(0) printf
#endif

// All the state for a particular instance of the JavaScript engine
struct _JSRunGuts {
    // interfaces in the engine we call
    IActiveScript* pActiveScript;
    IActiveScriptParse* pActiveScriptParse;
};

static HRESULT _parseCodeChunk(CFStringRef code, JSRunRef runtime);
static void _prepareStringParam(CFStringRef str, VARIANTARG *variant);
static void _prepareStringArrayReturnValue(CFArrayRef cfArray, VARIANT *pVarResult);
static CFArrayRef _JSPrimaryIpv4AddressesFunction(void);

#define CHECK_HRESULT(hr, func) if (!SUCCEEDED(hr)) { \
    CFLog(0, CFSTR("%s failed: 0x%X"), func, hResult); \
        break; \
} else COM_LOG("==== Call to %s succeeded ====\n", func);

// Name we use to add methods to the global namespace.  Basically a cookie used passed to
// IActiveScript::AddNamedItem and later received by IActiveScriptSite::GetItemInfo.
#define ITEM_NAME_ADDED_TO_JS OLESTR("_CFNetwork_Global_Routines_")


//
// Implemetation of IDispatch interface, vTable and other COM glue.
// The purpose of this class is to allow a couple C routines to be called from JS.
//

// Instance vars for our dispatcher
typedef struct {
    IDispatch iDispatch;    // mostly just the vtable
    UInt32 refCount;
} Dispatcher;

// Convert an IDispatch interface to one of our instance pointers
static inline Dispatcher *
thisFromIDispatch(IDispatch *disp) {
    return (Dispatcher*)((char *)disp - offsetof(Dispatcher, iDispatch));
}

// numerical DISPID's, which are cookies that map to our functions that we can dispatch
enum {
    DNSResolveDISPID = 22,              // the DNSResolve function
    PrimaryIPv4AddressesDISPID = 33,    // the PrimaryIPv4Addresses function
    PlatformNameDISPID = 44             // a global string property holding the platform name
};

static HRESULT STDMETHODCALLTYPE DispatchQueryInterface(IDispatch *disp, REFIID riid, void **ppv);
static ULONG STDMETHODCALLTYPE DispatchAddRef(IDispatch *disp);
static ULONG STDMETHODCALLTYPE DispatchRelease(IDispatch *disp);
static HRESULT STDMETHODCALLTYPE DispatchGetTypeInfoCount(IDispatch *disp, unsigned int *pctinfo);
static HRESULT STDMETHODCALLTYPE DispatchGetTypeInfo(IDispatch *disp, unsigned int iTInfo, LCID lcid, ITypeInfo **ppTInfo);
static HRESULT STDMETHODCALLTYPE DispatchGetIDsOfNames(IDispatch *disp, REFIID riid, OLECHAR ** rgszNames, unsigned intcNames, LCID lcid, DISPID *rgDispId);
static HRESULT STDMETHODCALLTYPE DispatchInvoke(IDispatch *disp, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, unsigned int *puArgErr);

static IDispatchVtbl DispatchVTable = {
    DispatchQueryInterface,
    DispatchAddRef,
    DispatchRelease,
    DispatchGetTypeInfoCount,
    DispatchGetTypeInfo,
    DispatchGetIDsOfNames,
    DispatchInvoke
};

static HRESULT STDMETHODCALLTYPE
DispatchQueryInterface(IDispatch *disp, REFIID riid, void **ppv) {

#ifdef DO_COM_LOGGING
    LPOLESTR interfaceString;
    StringFromIID(riid, &interfaceString);
    COM_LOG("DispatchQueryInterface: %ls\n", interfaceString);
    CoTaskMemFree(interfaceString);
#endif

    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID (riid, &IID_IDispatch)) {
        *ppv = disp;
        DispatchAddRef(disp);
        return NOERROR;
    }
    else {
        *ppv = 0;
        return E_NOINTERFACE;
    }
}

static ULONG STDMETHODCALLTYPE
DispatchAddRef(IDispatch *disp) {

    COM_LOG("DispatchAddRef\n");
    Dispatcher *this = thisFromIDispatch(disp);
    return ++this->refCount;
}

static ULONG STDMETHODCALLTYPE
DispatchRelease(IDispatch *disp) {

    COM_LOG("DispatchRelease\n");
    Dispatcher *this = thisFromIDispatch(disp);
    if (--this->refCount == 0) {
        CFAllocatorDeallocate(NULL, this);
        return 0;
    }
    return this->refCount;
}

static HRESULT STDMETHODCALLTYPE
DispatchGetTypeInfoCount(IDispatch *disp, unsigned int *pctinfo) {

    COM_LOG("DispatchGetTypeInfoCount\n");
    if (pctinfo == NULL) {
        return E_INVALIDARG;
    }
    else {
        *pctinfo = 0;
        return NOERROR;
    }
}

static HRESULT STDMETHODCALLTYPE
DispatchGetTypeInfo(IDispatch *disp, unsigned int iTInfo, LCID lcid, ITypeInfo **ppTInfo) {

    COM_LOG("DispatchGetTypeInfo\n");
    assert(FALSE);      // should never be called, since we return 0 from GetTypeInfoCount
    return E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE
DispatchGetIDsOfNames(IDispatch *disp, REFIID riid, OLECHAR **rgszNames, unsigned intcNames, LCID lcid, DISPID *rgDispId) {

    HRESULT retVal = S_OK;
    int i;
    for (i = 0; i < intcNames; i++) {
        if (wcscmp(OLESTR("__dnsResolve"), rgszNames[i]) == 0) {
            COM_LOG("DispatchGetIDsOfNames - resolved DNSResolveDISPID\n");
            rgDispId[i] = DNSResolveDISPID;
        }
        else if (wcscmp(OLESTR("__primaryIPv4Addresses"), rgszNames[i]) == 0) {
            COM_LOG("DispatchGetIDsOfNames - resolved PrimaryIPv4AddressesDISPID\n");
            rgDispId[i] = PrimaryIPv4AddressesDISPID;
        }
        else if (wcscmp(OLESTR("__platformName"), rgszNames[i]) == 0) {
            COM_LOG("DispatchGetIDsOfNames - resolved PlatformNameDISPID\n");
            rgDispId[i] = PlatformNameDISPID;
        }
        else {
            COM_LOG("DispatchGetIDsOfNames - unknown member %ls\n", rgszNames[i]);
            rgDispId[i] = DISPID_UNKNOWN;
            retVal = DISP_E_UNKNOWNNAME;
        }
    }
    return retVal;
}

static HRESULT STDMETHODCALLTYPE
DispatchInvoke(IDispatch *disp, DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, unsigned int *puArgErr)
{

    if (dispIdMember == DNSResolveDISPID) {
        COM_LOG("DispatchInvoke - DNSResolveDISPID, wFlags=%d\n", wFlags);
        assert(wFlags & DISPATCH_METHOD);
        if (pDispParams->cArgs != 1)
            return DISP_E_BADPARAMCOUNT;
        if (pDispParams->cNamedArgs != 0)
            return DISP_E_NONAMEDARGS;
        if (pDispParams->rgvarg[0].vt != VT_BSTR) {
            puArgErr = 0;
            return DISP_E_TYPEMISMATCH;
        }
        
        // Convert COM arg from BSTR to CFString, call our C code
        BSTR bstrNameParam = pDispParams->rgvarg[0].bstrVal;
        COM_LOG("DispatchInvoke - DNSResolveDISPID - input=%ls\n", pDispParams->rgvarg[0].bstrVal);
        CFStringRef cfNameParam = CFStringCreateWithCharactersNoCopy(NULL, bstrNameParam, SysStringLen(bstrNameParam), kCFAllocatorNull);
        CFArrayRef cfAddrList = _resolveDNSName(cfNameParam);
        CFRelease(cfNameParam);
        _prepareStringArrayReturnValue(cfAddrList, pVarResult);
        if (cfAddrList) CFRelease(cfAddrList);
        return S_OK;
    }

    else if (dispIdMember == PrimaryIPv4AddressesDISPID) {
        COM_LOG("DispatchInvoke - PrimaryIPv4AddressesDISPID, wFlags=%d\n", wFlags);
        assert(wFlags & DISPATCH_METHOD);
        if (pDispParams->cArgs != 0)
            return DISP_E_BADPARAMCOUNT;
        if (pDispParams->cNamedArgs != 0)
            return DISP_E_NONAMEDARGS;

        CFArrayRef cfAddrList = _JSPrimaryIpv4AddressesFunction();
        _prepareStringArrayReturnValue(cfAddrList, pVarResult);
        if (cfAddrList) CFRelease(cfAddrList);
        return S_OK;
    }
    
    else if (dispIdMember == PlatformNameDISPID) {
        COM_LOG("DispatchInvoke - PlatformNameDISPID, wFlags=%d\n", wFlags);
        assert(wFlags == DISPATCH_PROPERTYGET);
        if (pDispParams->cArgs != 0)
            return DISP_E_BADPARAMCOUNT;
        if (pDispParams->cNamedArgs != 0)
            return DISP_E_NONAMEDARGS;
        
        VariantInit(pVarResult);
        pVarResult->vt = VT_BSTR;
        pVarResult->bstrVal = SysAllocString(OLESTR("Win32"));
        return S_OK;
    }
    
    else {
        COM_LOG("DispatchInvoke - UNKNOWN MEMBER REQUESTED\n");
        return DISP_E_MEMBERNOTFOUND;
    }
}


//
// Implemetation of IActiveScriptSite interface, vTable and other COM glue
//

// Instance vars for our script site
typedef struct {
    IActiveScriptSite iSite;    // mostly just the vtable
    UInt32 refCount;
    CFAllocatorRef alloc;
} ScriptSite;

// Convert an ActiveScriptSite interface to one of our instance pointers
static inline ScriptSite *
thisFromISite(IActiveScriptSite *site) {
    return (ScriptSite*)((char *)site - offsetof(ScriptSite, iSite));
}

static HRESULT STDMETHODCALLTYPE SiteQueryInterface(IActiveScriptSite *site, REFIID riid, void **ppv);
static ULONG STDMETHODCALLTYPE SiteAddRef(IActiveScriptSite *site);
static ULONG STDMETHODCALLTYPE SiteRelease(IActiveScriptSite *site);
static HRESULT STDMETHODCALLTYPE SiteGetLCID(IActiveScriptSite *site, LCID *plcid);
static HRESULT STDMETHODCALLTYPE SiteGetItemInfo(IActiveScriptSite *site, LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppunkItem, ITypeInfo **ppTypeInfo);
static HRESULT STDMETHODCALLTYPE SiteGetDocVersionString(IActiveScriptSite *site, BSTR *pbstrVersionString);
static HRESULT STDMETHODCALLTYPE SiteOnScriptTerminate(IActiveScriptSite *site, const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
static HRESULT STDMETHODCALLTYPE SiteOnStateChange(IActiveScriptSite *site, SCRIPTSTATE ssScriptState);
static HRESULT STDMETHODCALLTYPE SiteOnScriptError(IActiveScriptSite *site, IActiveScriptError *pase);
static HRESULT STDMETHODCALLTYPE SiteOnEnterScript(IActiveScriptSite *site);
static HRESULT STDMETHODCALLTYPE SiteOnLeaveScript(IActiveScriptSite *site);

static const IActiveScriptSiteVtbl ScriptSiteVTable = {
    SiteQueryInterface,
    SiteAddRef,
    SiteRelease,
    SiteGetLCID,
    SiteGetItemInfo,
    SiteGetDocVersionString,
    SiteOnScriptTerminate,
    SiteOnStateChange,
    SiteOnScriptError,
    SiteOnEnterScript,
    SiteOnLeaveScript
};

static HRESULT STDMETHODCALLTYPE
SiteQueryInterface(IActiveScriptSite *site, REFIID riid, void **ppv) {

#ifdef DO_COM_LOGGING
    LPOLESTR interfaceString;
    StringFromIID(riid, &interfaceString);
    COM_LOG("SiteQueryInterface: %ls\n", interfaceString);
    CoTaskMemFree(interfaceString);
#endif

    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID (riid, &IID_IActiveScriptSite)) {
        *ppv = site;
        SiteAddRef(site);
        return NOERROR;
    }
    else {
        *ppv = 0;
        return E_NOINTERFACE;
    }
}

static ULONG STDMETHODCALLTYPE
SiteAddRef(IActiveScriptSite *site) {

    COM_LOG("SiteAddRef\n");
    ScriptSite *this = thisFromISite(site);
    return ++this->refCount;
}

static ULONG STDMETHODCALLTYPE
SiteRelease(IActiveScriptSite *site) {

    COM_LOG("SiteRelease\n");
    ScriptSite *this = thisFromISite(site);
    if (--this->refCount == 0) {
        CFAllocatorDeallocate(NULL, this);
        return 0;
    }
    return this->refCount;
}

static HRESULT STDMETHODCALLTYPE
SiteGetLCID(IActiveScriptSite *site, LCID *plcid) {

    COM_LOG("SiteGetLCID\n");
    return(plcid == NULL) ? E_POINTER : E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE
SiteGetItemInfo(IActiveScriptSite *site, 
            LPCOLESTR pstrName,             // address of item name
            DWORD dwReturnMask,             // bit mask for information retrieval
            IUnknown **ppunkItem,           // address of pointer to item's IUnknown
            ITypeInfo **ppTypeInfo)         // address of pointer to item's ITypeInfo
{
    COM_LOG("SiteGetItemInfo: %ls %lu\n", pstrName, dwReturnMask);

    if (dwReturnMask & SCRIPTINFO_IUNKNOWN) {
        if (!ppunkItem)
            return E_INVALIDARG;

        if (wcscmp(ITEM_NAME_ADDED_TO_JS, pstrName) == 0) {
            COM_LOG("SiteGetItemInfo: returning C gateway object\n");
            // create our dispatcher object, which we hand back to the engine
            ScriptSite *this = thisFromISite(site);
            Dispatcher *newDispatcher = CFAllocatorAllocate(this->alloc, sizeof(Dispatcher), 0);
            newDispatcher->iDispatch.lpVtbl = &DispatchVTable;
            newDispatcher->refCount = 0;
            DispatchAddRef(&(newDispatcher->iDispatch));
            *ppunkItem = (IUnknown *)&(newDispatcher->iDispatch);
            return S_OK;
        }
        else
            *ppunkItem = NULL;
    }
    if (dwReturnMask & SCRIPTINFO_ITYPEINFO) {
        if (!ppTypeInfo)
            return E_INVALIDARG;
        *ppTypeInfo = NULL;
    }
    return TYPE_E_ELEMENTNOTFOUND;
}

static HRESULT STDMETHODCALLTYPE
SiteGetDocVersionString(IActiveScriptSite *site, BSTR *pbstrVersionString) {

    COM_LOG("SiteGetDocVersionString\n");
    return (pbstrVersionString == NULL) ? E_POINTER : E_NOTIMPL;
}

static HRESULT STDMETHODCALLTYPE
SiteOnScriptTerminate(IActiveScriptSite *site, 
                      const VARIANT *pvarResult,      // address of script results
                      const EXCEPINFO *pexcepinfo)    // address of structure with exception information
{
    COM_LOG("SiteOnScriptTerminate\n");
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SiteOnStateChange(IActiveScriptSite *site, SCRIPTSTATE ssScriptState) {
    
    COM_LOG("SiteOnStateChange: %d\n", ssScriptState);
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SiteOnScriptError(IActiveScriptSite *site, IActiveScriptError *pase) {

    COM_LOG("SiteOnScriptError\n");
//#ifdef DO_COM_LOGGING
    do {
        char buffer[1024];
        EXCEPINFO exception;
        HRESULT hResult = IActiveScriptError_GetExceptionInfo(pase, &exception);
        CHECK_HRESULT(hResult, "IActiveScriptError_GetExceptionInfo");
        wcstombs(buffer, exception.bstrDescription, 1024);
        CFLog(0, CFSTR("JavaScript error when processing PAC file: %s"), buffer);
        BSTR sourceLine;
        hResult = IActiveScriptError_GetSourceLineText(pase, &sourceLine);
        if (hResult == S_OK) {
            wcstombs(buffer, sourceLine, 1024);
            CFLog(0, CFSTR("    offending code: %s"), buffer);
        }
    } while (0);
//#endif
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SiteOnEnterScript(IActiveScriptSite *site) {

    COM_LOG("SiteOnEnterScript\n");
    return S_OK;
}

static HRESULT STDMETHODCALLTYPE
SiteOnLeaveScript(IActiveScriptSite *site) {

    COM_LOG("SiteOnLeaveScript\n");
    return S_OK;
}

//
//  End of COM interface implementations
//


// Feed a piece of code to the script engine
/* static */ HRESULT
_parseCodeChunk(CFStringRef code, JSRunRef runtime) {
    
    // convert from CFString to unicode buffer
    CFIndex len = CFStringGetLength(code);
    UniChar stackBuffer[12*1024];
    UniChar *uniBuf;
    if (len >= 12*1024) {
        uniBuf = malloc((len+1) * sizeof(UniChar));
    } else {
        uniBuf = stackBuffer;
    }
    CFStringGetCharacters(code, CFRangeMake(0, len), uniBuf);
    uniBuf[len] = 0;

    // parse the code
    EXCEPINFO exception = { 0 };
    HRESULT hResult = IActiveScriptParse_ParseScriptText(runtime->pActiveScriptParse, uniBuf, NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISVISIBLE, NULL, &exception);
    if (uniBuf != stackBuffer)
        free(uniBuf);
    return hResult;
}

/* static */ JSRunRef
_createJSRuntime(CFAllocatorRef alloc, CFStringRef js_support, CFStringRef js_pac) {

    JSRunRef runtime = (JSRunRef)CFAllocatorAllocate(alloc, sizeof(JSRun), 0);
    runtime->pActiveScript = NULL;
    runtime->pActiveScriptParse = NULL;

    // create our site object, which we hand to the script object
    ScriptSite *newSite = CFAllocatorAllocate(alloc, sizeof(ScriptSite), 0);
    newSite->iSite.lpVtbl = &ScriptSiteVTable;
    newSite->refCount = 0;
    newSite->alloc = alloc;
    // take a reference until site gets hooked up to other objects
    SiteAddRef(&(newSite->iSite));

    //??? How do we choose between COINIT_APARTMENTTHREADED and COINIT_MULTITHREADED
    //??? We need to arrange to make this call once per thread somehow
    //J: i would init/uninit for each usage and then deal with poor performance if it's a problem
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    HRESULT hResult = S_OK;
    do {
        CLSID clsid;
        hResult = CLSIDFromProgID( L"JavaScript", &clsid);
        CHECK_HRESULT(hResult, "CLSIDFromProgID");

        hResult = CoCreateInstance(&clsid, NULL, CLSCTX_INPROC_SERVER, &IID_IActiveScript, (void**)&(runtime->pActiveScript));
        CHECK_HRESULT(hResult, "CoCreateInstance");

        hResult = IActiveScript_QueryInterface(runtime->pActiveScript, &IID_IActiveScriptParse, (void **)&(runtime->pActiveScriptParse));
        CHECK_HRESULT(hResult, "QueryInterface:IID_IActiveScriptParse");
        
        hResult = IActiveScript_SetScriptSite(runtime->pActiveScript, &(newSite->iSite));
        CHECK_HRESULT(hResult, "IActiveScript_SetScriptSite");
        
        hResult = IActiveScriptParse_InitNew(runtime->pActiveScriptParse);
        CHECK_HRESULT(hResult, "IActiveScriptParse_InitNew");
        
        hResult = IActiveScript_AddNamedItem(runtime->pActiveScript, ITEM_NAME_ADDED_TO_JS, SCRIPTITEM_GLOBALMEMBERS|SCRIPTITEM_ISVISIBLE);
        CHECK_HRESULT(hResult, "IActiveScript_AddNamedItem");
        
        hResult = IActiveScript_SetScriptState(runtime->pActiveScript, SCRIPTSTATE_STARTED);
        CHECK_HRESULT(hResult, "IActiveScript_SetScriptState");
        
        hResult = _parseCodeChunk(js_support, runtime);
        CHECK_HRESULT(hResult, "IActiveScriptParse_ParseScriptText - js_support");
        
        hResult = _parseCodeChunk(js_pac, runtime);
        CHECK_HRESULT(hResult, "IActiveScriptParse_ParseScriptText - js_pac");
        
    } while (0);

    SiteRelease(&(newSite->iSite));
    if (hResult == S_OK) {
        return runtime;
    }
    else {
        _freeJSRuntime(runtime);
        return NULL;
    }
}


/* static */ void
_freeJSRuntime(JSRunRef runtime) {
    
    do {
        if (runtime->pActiveScript) {
            HRESULT hResult = IActiveScript_Close(runtime->pActiveScript);
            CHECK_HRESULT(hResult, "IActiveScript_Close");
        }

        long int refs;
#ifdef DO_COM_LOGGING
        // Print out current refCounts.  Both should be 2 at this point.  From experimenting,
        // it appears that when you Release either pActiveScript or pActiveScriptParse both end
        // losing one ref, so by releasing each one once, they both will go away.
        
        // In addition, another good test is to grep the log output for "DispatchAddRef" and
        // "DispatchRelease".  After this routine runs there should be an equal number (23 as of
        // this writing).
        if (runtime->pActiveScriptParse) {
            IActiveScriptParse_AddRef(runtime->pActiveScriptParse);
            refs = IActiveScriptParse_Release(runtime->pActiveScriptParse);
            COM_LOG("IActiveScriptParse started with %ld refs in _freeJSRuntime, should be 2\n", refs);
        }

        if (runtime->pActiveScript) {
            IActiveScript_AddRef(runtime->pActiveScript);
            refs = IActiveScript_Release(runtime->pActiveScript);
            COM_LOG("IActiveScript started with %ld refs in _freeJSRuntime, should be 2\n", refs);
        }
#endif

        if (runtime->pActiveScriptParse) {
            refs = IActiveScriptParse_Release(runtime->pActiveScriptParse);
            COM_LOG("IActiveScriptParse has %ld refs in _freeJSRuntime, should be 1\n", refs);
        }
        if (runtime->pActiveScriptParse) {
            refs = IActiveScript_Release(runtime->pActiveScript);
            COM_LOG("IActiveScript has %ld refs in _freeJSRuntime, should be 0\n", refs);
        }
    } while (0);

    CFAllocatorDeallocate(NULL, runtime);
}


// Prepare a CFString for passing as a param via IDispatch to a COM method
/* static */ void
_prepareStringParam(CFStringRef str, VARIANTARG *variant) {
    
    VariantInit(variant);
    variant->vt = VT_BSTR;
    CFIndex len = CFStringGetLength(str);
    UniChar stackBuffer[1024];
    UniChar *uniBuf;
    if (len > 1024) {
        uniBuf = malloc(len * sizeof(UniChar));
    } else {
        uniBuf = stackBuffer;
    }
    CFStringGetCharacters(str, CFRangeMake(0, len), uniBuf);
    variant->bstrVal = SysAllocStringLen(uniBuf, len);
    if (uniBuf != stackBuffer)
        free(uniBuf);
}


// Prepare a CFArray of CFString to be returned via IDispatch to a COM method
/* static */ void
_prepareStringArrayReturnValue(CFArrayRef cfArray, VARIANT *pVarResult) {

    CFIndex count = cfArray ? CFArrayGetCount(cfArray) : 0;
    VariantInit(pVarResult);
    pVarResult->vt = VT_ARRAY | VT_VARIANT;
    pVarResult->parray = SafeArrayCreateVector(VT_VARIANT, 0, count);
    long i;
    for (i = 0; i < count; i++) {
        CFStringRef cfValue = CFArrayGetValueAtIndex(cfArray, i);
        UniChar stackBuffer[1024];
        UniChar *uniBuf;
        CFIndex len = CFStringGetLength(cfValue);
        if (len > 1024)
            uniBuf = malloc(len * sizeof(UniChar));
        else
            uniBuf = stackBuffer;
        CFStringGetCharacters(cfValue, CFRangeMake(0, len), uniBuf);
        BSTR bstrValue = SysAllocStringLen(uniBuf, len);
        if (uniBuf != stackBuffer)
            free(uniBuf);

        COM_LOG("DispatchInvoke - converting result string=%ls\n", bstrValue);
        VARIANT variant;
        VariantInit(&variant);
        variant.vt = VT_BSTR;
        variant.bstrVal = bstrValue;
        HRESULT hResult = SafeArrayPutElement(pVarResult->parray, &i, &variant);
        CHECK_HRESULT(hResult, "SafeArrayPutElement");
        SysFreeString(bstrValue);
    }    
}


/* static */ CFStringRef
_callPACFunction(CFAllocatorRef alloc, JSRunRef runtime, CFURLRef url, CFStringRef host) {

    CFStringRef result = NULL;
    HRESULT hResult = S_OK;
    IDispatch *pDisp = NULL;
    VARIANTARG paramValues[2];
    memset(paramValues, 0, sizeof(paramValues));
    do {
        // get the iDispatch interface we can use to call JS
        hResult = IActiveScript_GetScriptDispatch(runtime->pActiveScript, NULL, &pDisp);
        CHECK_HRESULT(hResult, "IActiveScript_GetScriptDispatch");
        
        // find the dispid (a cookie) for the function we want to call
        LPOLESTR funcName = OLESTR("__Apple_FindProxyForURL");
        DISPID dispid;
        hResult = (pDisp->lpVtbl->GetIDsOfNames)(pDisp, &IID_NULL, &funcName, 1, LOCALE_NEUTRAL, &dispid);
        CHECK_HRESULT(hResult, "IDispatch_GetIDsOfNames");

        // set up the two args we will pass to the JS routine.  args are in last-to-first order
        _prepareStringParam(host, &paramValues[0]);
        CFURLRef absURL = CFURLCopyAbsoluteURL(url);
        _prepareStringParam(CFURLGetString(absURL), &paramValues[1]);
        CFRelease(absURL);
        DISPPARAMS params = { paramValues, NULL, 2, 0 };

        // call it, afterwhich we can release the dispatch interface
        VARIANT pvarRetVal;
        hResult = (pDisp->lpVtbl->Invoke)(pDisp, dispid, &IID_NULL, LOCALE_NEUTRAL, DISPATCH_METHOD, &params, &pvarRetVal, NULL, NULL);
        CHECK_HRESULT(hResult, "IDispatch_Invoke");

        // convert result to CFString
        if (pvarRetVal.vt == VT_BSTR) {
            result = CFStringCreateWithCharacters(alloc, pvarRetVal.bstrVal, SysStringLen(pvarRetVal.bstrVal));
            COM_LOG("FindProxyForURL returned %ls\n", pvarRetVal.bstrVal);
        }
        else {
            COM_LOG("FindProxyForURL returned unexpected type %d\n", pvarRetVal.vt);
        }
        SysFreeString(pvarRetVal.bstrVal);
    } while (0);

    if (pDisp)
        (pDisp->lpVtbl->Release)(pDisp);
    if (paramValues[0].bstrVal)
        SysFreeString(paramValues[0].bstrVal);
    if (paramValues[1].bstrVal)
        SysFreeString(paramValues[1].bstrVal);

    return result;
}

/* static */ CFArrayRef
_JSPrimaryIpv4AddressesFunction(void) {

    CFMutableArrayRef list = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock != INVALID_SOCKET) {

        // Note from MS KB 181520: this struct is a different size on NT 4.0, if we ever care about that OS
        INTERFACE_INFO interfaces[20];     // we only consider a maximum of 20, should be plenty
        DWORD len = sizeof(interfaces);
        DWORD resultLen;
        if (WSAIoctl(sock, SIO_GET_INTERFACE_LIST, NULL, 0, (LPVOID)interfaces, len, &resultLen, NULL, NULL) != SOCKET_ERROR) {
            int i;
            for (i = 0; i < (resultLen/sizeof(INTERFACE_INFO)); i++) {
                if ((interfaces[i].iiFlags & IFF_UP)
                    && !(interfaces[i].iiFlags & IFF_LOOPBACK)
                    && interfaces[i].iiAddress.Address.sa_family == AF_INET)
                {
                    CFStringRef str = stringFromAddr(&(interfaces[i].iiAddress.Address), sizeof(struct sockaddr_in));
                    if (str) {
                        CFArrayAppendValue(list, str);
                        CFRelease(str);
                    }
                }
            }
        }
        closesocket(sock);
    }
    //CFLog(0, CFSTR("_JSPrimaryIpv4AddressesFunction results: %@"), list);
    return list;
}

#endif  /* __WIN32__ */

static CFURLRef _JSPacFileLocation = NULL;
static CFAbsoluteTime _JSPacFileExpiration = 0;
static JSRunRef _JSRuntime = NULL;

/* Must be called while holding the _JSLock.  It is the caller's responsibility to verify that expires is a valid 
   value; bad values can cause problems.  In general, the caller should make sure to successfully go through
   _stringFromLoadedPACStream to produce the expiry value. */ 
static void _JSSetEnvironmentForPAC(CFAllocatorRef alloc, CFURLRef url, CFAbsoluteTime expires, CFStringRef pacString) {

    if (_JSRuntime) {
        _freeJSRuntime(_JSRuntime);
        _JSRuntime = NULL;
    }

    _JSPacFileExpiration = 0;

    if (_JSPacFileLocation) {
        CFRelease(_JSPacFileLocation);
        _JSPacFileLocation = NULL;
    }
    
    CFStringRef js_support = _loadJSSupportFile();
    if (js_support) {
        _JSRuntime = _createJSRuntime(alloc, js_support, pacString);
        if (_JSRuntime) {
            _JSPacFileExpiration = expires;
            _JSPacFileLocation = CFRetain(url);
        }
    }
}

static CFSpinLock_t _JSLock = 0;

/* static */ CFStringRef
_JSFindProxyForURL(CFURLRef pac, CFURLRef url, CFStringRef host) {
    CFAllocatorRef alloc = CFGetAllocator(pac);
    CFStringRef result = NULL;
    CFStreamError err = {0, 0};

    if (!host) {
        return CFRetain(_kProxySupportDIRECT);
    }
    
    __CFSpinLock(&_JSLock);

    if (!_JSRuntime ||
        !_JSPacFileExpiration || (CFAbsoluteTimeGetCurrent() > _JSPacFileExpiration) ||
        !_JSPacFileLocation || !CFEqual(pac, _JSPacFileLocation)) {

        CFAbsoluteTime expires;
        CFStringRef js_pac = _loadPACFile(alloc, pac, &expires, &err);
        if (js_pac) {
            _JSSetEnvironmentForPAC(alloc, pac, expires, js_pac);
            CFRelease(js_pac);
        }
    }

    if (_JSRuntime) {
        result = _callPACFunction(alloc, _JSRuntime, url, host);
#if 0
        // debug code to enable leak checking - see more info in _freeJSRuntime
        _freeJSRuntime(_JSRuntime);
        _JSRuntime = NULL;
#endif
    }
    else if ((err.domain == kCFStreamErrorDomainNetDB) ||					// Host name lookup failure
             (err.domain == kCFStreamErrorDomainSystemConfiguration) ||		// Connection lost or not reachable
             (err.domain == kCFStreamErrorDomainSSL) ||						// SSL errors (bad cert)
             (err.domain == _kCFStreamErrorDomainNativeSockets) ||				// Socket errors
             (err.domain == kCFStreamErrorDomainCustom))					// Timedout error for loader
    {
        result = CFRetain(_kProxySupportDIRECT);
    }

    __CFSpinUnlock(&_JSLock);
    return result;
}

static CFStringRef
_JSFindProxyForURLAsync(CFURLRef pac, CFURLRef url, CFStringRef host, Boolean *mustBlock) {
    CFStringRef result = NULL;
    CFAllocatorRef alloc = CFGetAllocator(pac);
    
    if (!host) {
        return CFRetain(_kProxySupportDIRECT);
    }
    
     __CFSpinLock(&_JSLock);

    if (!_JSRuntime || !_JSPacFileExpiration || 
        (CFAbsoluteTimeGetCurrent() > _JSPacFileExpiration) ||
        !_JSPacFileLocation || !CFEqual(pac, _JSPacFileLocation)) {
        *mustBlock = TRUE;
    } else {
        result = _callPACFunction(alloc, _JSRuntime, url, host);
        *mustBlock = FALSE;
    }
    __CFSpinUnlock(&_JSLock);

    return result;
}

// Platform independent piece of the DnsResolve callbacks
static CFArrayRef
_resolveDNSName(CFStringRef name) {
    
    CFStreamError error;
    CFMutableArrayRef list = NULL;
    CFHostRef lookup = CFHostCreateWithName(kCFAllocatorDefault, name);
    
    if (lookup && CFHostStartInfoResolution(lookup, kCFHostAddresses, &error) && !error.error) {
        
        CFArrayRef addrs = CFHostGetAddressing(lookup, NULL);
        CFIndex count = addrs ? CFArrayGetCount(addrs) : 0;
        
        if (count) {
            
            list = CFArrayCreateMutable(kCFAllocatorDefault, count, &kCFTypeArrayCallBacks);
            if (list) {
                CFIndex i;
                for (i = 0; i < count; i++) {
                    
                    CFDataRef saData = (CFDataRef)CFArrayGetValueAtIndex(addrs, i);
					CFStringRef str = _CFNetworkCFStringCreateWithCFDataAddress(kCFAllocatorDefault, saData);
                    if (str) {
                        CFArrayAppendValue(list, str);
                        CFRelease(str);
                    }
                }
                
                if (!CFArrayGetCount(list)) {
                    CFRelease(list);
                    list = NULL;
                }
            }
            
        }
        
    }
    
    if (lookup) CFRelease(lookup);
    
    return list;
}

#if defined(__MACH__)
static JSObjectRef
_JSDnsResolveFunction(void* context, JSObjectRef ctxt, CFArrayRef args) {

    JSObjectRef result = NULL;
    
    if (args && CFArrayGetCount(args) == 1) {
        
        CFTypeRef name = JSObjectCopyCFValue((JSObjectRef)CFArrayGetValueAtIndex(args, 0));
        if (name && CFGetTypeID(name) == CFStringGetTypeID()) {
            
            CFArrayRef list = _resolveDNSName((CFStringRef)name);
            if (list) {
                result = JSObjectCreateWithCFType(list);
                CFRelease(list);
            }
        }
        
        if (name) CFRelease(name);
    }
    
    return result;
}


/* static */ JSObjectRef
_JSPrimaryIpv4AddressesFunction(void* context, JSObjectRef ctxt, CFArrayRef args) {

    JSObjectRef result = NULL;
    
    do {
        CFStringRef key = NULL;
        CFDictionaryRef value = NULL;
        CFStringRef interface = NULL;
        SCDynamicStoreRef store = SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("JSEvaluator"), NULL, NULL);
        
        if (!store)
            break;
        
        key = SCDynamicStoreKeyCreateNetworkGlobalEntity(kCFAllocatorDefault, kSCDynamicStoreDomainState, kSCEntNetIPv4);
        if (!key) {
            CFRelease(store);
            break;
        }
        
        value = SCDynamicStoreCopyValue(store, key);
        CFRelease(key);
        
        if (!value) {
            CFRelease(store);
            break;
        }
        
        interface = CFDictionaryGetValue(value, kSCDynamicStorePropNetPrimaryInterface);
        if (!interface) {
            CFRelease(value);
            CFRelease(store);
            break;
        }
        
        key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(kCFAllocatorDefault, kSCDynamicStoreDomainState, interface, kSCEntNetIPv4);
        CFRelease(value);

        if (!key) {
            CFRelease(store);
            break;
        }
        
        value = SCDynamicStoreCopyValue(store, key);
        
        if (!value) {
            CFRelease(store);
            break;
        }

        result = JSObjectCreateWithCFType(value);
        CFRelease(value);
        CFRelease(key);
        
    } while (0);
    
    return result;
}
#endif


#define BUF_SIZE 4096
static void releasePACStreamContext(void *info);

typedef struct {
    CFURLRef pacURL;
    CFURLRef targetURL;
    CFStringRef targetScheme;
    CFStringRef targetHost;
    
    CFMutableDataRef data;
    void *clientInfo;
    _CFProxyStreamCallBack cb;
} _PACStreamContext;

static void releasePACStreamContext(void *info) {
    _PACStreamContext *ctxt = (_PACStreamContext *)info;
    CFAllocatorRef alloc = CFGetAllocator(ctxt->data);
    CFRelease(ctxt->pacURL);
    CFRelease(ctxt->targetURL);
    CFRelease(ctxt->targetScheme);
    CFRelease(ctxt->targetHost);
    CFRelease(ctxt->data);
    CFAllocatorDeallocate(alloc, ctxt);
}

static void readBytesFromProxyStream(CFReadStreamRef proxyStream, _PACStreamContext *ctxt) {
    UInt8 buf[BUF_SIZE];
    CFIndex bytesRead = CFReadStreamRead(proxyStream, buf, BUF_SIZE);
    if (bytesRead > 0) {
        CFDataAppendBytes(ctxt->data, buf, bytesRead);
    }
}

static void proxyStreamCallback(CFReadStreamRef proxyStream, CFStreamEventType type, void *clientCallBackInfo) {
    _PACStreamContext *ctxt = (_PACStreamContext *)clientCallBackInfo;
    switch (type) {
    case kCFStreamEventHasBytesAvailable: 
        readBytesFromProxyStream(proxyStream, ctxt);
        break;
    case kCFStreamEventEndEncountered:
    case kCFStreamEventErrorOccurred:
        ctxt->cb(proxyStream, ctxt->clientInfo);
        break;
    default:
        ;
    }
}


static CFReadStreamRef BuildStreamForPACURL(CFAllocatorRef alloc, CFURLRef pacURL, CFURLRef targetURL, CFStringRef targetScheme, CFStringRef targetHost, _CFProxyStreamCallBack callback, void *clientInfo) {
    Boolean isFile;
    CFReadStreamRef stream = _streamForPACFile(alloc, pacURL, &isFile);
    if (stream) {
        _PACStreamContext *pacContext = CFAllocatorAllocate(alloc, sizeof(_PACStreamContext), 0);
        CFRetain(pacURL);
        pacContext->pacURL = pacURL;
        CFRetain(targetURL);
        pacContext->targetURL = targetURL;
        CFRetain(targetScheme);
        pacContext->targetScheme = targetScheme;
        CFRetain(targetHost);
        pacContext->targetHost = targetHost;
        
        pacContext->data = CFDataCreateMutable(alloc, 0);
        pacContext->clientInfo = clientInfo;
        pacContext->cb = callback;

        CFStreamClientContext streamContext;
        streamContext.version = 0;
        streamContext.info = pacContext;
        streamContext.retain = NULL;
        streamContext.release = releasePACStreamContext;
        streamContext.copyDescription = NULL;
        
        CFReadStreamSetClient(stream, kCFStreamEventHasBytesAvailable | kCFStreamEventErrorOccurred | kCFStreamEventEndEncountered, proxyStreamCallback, &streamContext);
        CFReadStreamOpen(stream);
    }
    return stream;
}

CFMutableArrayRef _CFNetworkCopyProxyFromProxyStream(CFReadStreamRef proxyStream, Boolean *isComplete) {
    CFStreamStatus status = CFReadStreamGetStatus(proxyStream);
    if (status == kCFStreamStatusOpen && CFReadStreamHasBytesAvailable(proxyStream)) {
        _PACStreamContext *pacContext = (_PACStreamContext *)_CFReadStreamGetClient(proxyStream);
        readBytesFromProxyStream(proxyStream, pacContext);
        status = CFReadStreamGetStatus(proxyStream);
    }
    if (status == kCFStreamStatusAtEnd) {
        _PACStreamContext *pacContext = (_PACStreamContext *)_CFReadStreamGetClient(proxyStream);
        CFAllocatorRef alloc = CFGetAllocator(pacContext->data);
        CFStringRef pacString;
        CFStringRef pacResult;
        CFAbsoluteTime expiry;
        *isComplete = TRUE;
	 __CFSpinLock(&_JSLock);
        pacString = _stringFromLoadedPACStream(alloc, pacContext->data, proxyStream, &expiry);
        _JSSetEnvironmentForPAC(alloc, pacContext->pacURL, expiry, pacString);
        if (pacString)
			CFRelease(pacString);
        pacResult = _callPACFunction(alloc, _JSRuntime, pacContext->targetURL, pacContext->targetHost);
	__CFSpinUnlock(&_JSLock);

        CFMutableArrayRef result = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);
        _appendProxiesFromPACResponse(alloc, result, pacResult, pacContext->targetScheme);
        if(pacResult) CFRelease(pacResult);
        CFReadStreamClose(proxyStream);
        return result;
    } else if (status == kCFStreamStatusError) {
        CFMutableArrayRef result = CFArrayCreateMutable(CFGetAllocator(proxyStream), 0, &kCFTypeArrayCallBacks);
        CFArrayAppendValue(result, kCFNull);
        *isComplete = TRUE;
        return result;
    } else {
        *isComplete = FALSE;
        return NULL;
    }
}
