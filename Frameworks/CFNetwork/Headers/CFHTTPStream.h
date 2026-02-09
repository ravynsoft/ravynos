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
     File:       CFNetwork/CFHTTPStream.h
 
     Contains:   CoreFoundation Network HTTP streams header
 
     Copyright:  © 2001-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file may contain unreleased API's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHTTPStream.i
                     Revision:        1.6
                     Dated:           2004/08/03 22:57:05
                     Last change by:  jiarocci
                     Last comment:    Update Interface files to include headerdoc and remove warnings.
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHTTPSTREAM__
#define __CFHTTPSTREAM__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __CFSTREAM__
#include <CoreFoundation/CFStream.h>
#endif

#ifndef __CFHTTPMESSAGE__
#include <CFNetwork/CFHTTPMessage.h>
#endif





#include <AvailabilityMacros.h>

#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint on
#endif

/*
 *  kCFStreamErrorDomainHTTP
 *  
 *  Discussion:
 *    Result code returned by HTTP server
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const SInt32 kCFStreamErrorDomainHTTP                         AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;

/*
 *  CFStreamErrorHTTP
 *  
 *  Discussion:
 *    Errors from the kCFStreamErrorDomainHTTP domain.
 */
enum CFStreamErrorHTTP {

  /*
   * Could not parse the request/response.
   */
  kCFStreamErrorHTTPParseFailure = -1,

  /*
   * A loop was detected during redirection.
   */
  kCFStreamErrorHTTPRedirectionLoop = -2,

  /*
   * Could not retreive url for request/response.
   */
  kCFStreamErrorHTTPBadURL      = -3
};
typedef enum CFStreamErrorHTTP CFStreamErrorHTTP;

/*
 *  kCFStreamPropertyHTTPResponseHeader
 *  
 *  Discussion:
 *    Stream property key, for copy operations. Value is a
 *    CFHTTPMessage with 0 bytes data.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPResponseHeader         AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  kCFStreamPropertyHTTPFinalURL
 *  
 *  Discussion:
 *    Stream property key, for copy operations. The response header
 *    value is the CFURL from the final request; will only differ from
 *    the URL in the original request if an autoredirection has
 *    occurred.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPFinalURL               AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  kCFStreamPropertyHTTPProxy
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations. The value
 *    is a CFDictionary. HTTP proxy information is set the same way as
 *    SOCKS proxies (see CFSocketStream.h). Call
 *    CFReadStreamSetProperty() passing an HTTP stream and the property
 *    kCFStreamPropertyHTTPProxy. The value should include at least one
 *    Host/Port pair from the keys below. Use the dictionary returned
 *    by SystemConfiguration.framework to set the default values for
 *    the system. HTTP proxies are not applied automatically.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPProxy                  AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  kCFStreamPropertyHTTPProxyHost
 *  
 *  Discussion:
 *    Proxy dictionary key. The hostname of an HTTP proxy. The value is
 *    a CFString. The key name matches kSCPropNetProxiesHTTPProxy.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPProxyHost              AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  kCFStreamPropertyHTTPProxyPort
 *  
 *  Discussion:
 *    Proxy dictionary key. Value is a CFNumber.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPProxyPort              AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
/* matches kSCPropNetProxiesHTTPPort */


/*
 *  kCFStreamPropertyHTTPSProxyHost
 *  
 *  Discussion:
 *    Proxy dictionary key. Value is a CFString.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPSProxyHost             AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
/* matches kSCPropNetProxiesHTTPSProxy */


/*
 *  kCFStreamPropertyHTTPSProxyPort
 *  
 *  Discussion:
 *    Proxy dictionary key. Value is a CFNumber.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPSProxyPort             AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;
/* matches kSCPropNetProxiesHTTPSPort */


/*
 *  kCFStreamPropertyHTTPShouldAutoredirect
 *  
 *  Discussion:
 *    Proxy dictionary key. Value is a CFBoolean. Redirection is not
 *    performed by default.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPShouldAutoredirect     AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  kCFStreamPropertyHTTPAttemptPersistentConnection
 *  
 *  Discussion:
 *    Proxy dictionary key. Value is a CFBoolean.  If this property is
 *    set to kCFBooleanTrue, an HTTP stream will look for an
 *    appropriate extant persistent connection to use, and if it finds
 *    none, will try to create one. Persistent connections are not used
 *    by default.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPAttemptPersistentConnection AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  kCFStreamPropertyHTTPRequestBytesWrittenCount
 *  
 *  Discussion:
 *    Proxy dictionary key. Value is a CFNumber. This property can only
 *    be retrieved, not set. The number returned is the number of bytes
 *    from the body of the request that have been written to the
 *    underlying socket
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyHTTPRequestBytesWrittenCount AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*********************/
/* Creation routines */
/*********************/
/*
 *  CFReadStreamCreateForHTTPRequest()
 *  
 *  Discussion:
 *    Create an HTTP read stream for the response to the given request.
 *    When the stream is opened, it will begin transmitting the
 *    request. The bytes returned are the pure body bytes; the response
 *    header has been parsed off. To retrieve the response header, ask
 *    for kCFStreamPropertyHTTPResponseHeader, above, any time after
 *    the first bytes arrive on the stream (or when stream end is
 *    reported, if there are no data bytes). When an HTTP/1.1 server
 *    returns a chunked a response, the chunks will be formed into one
 *    continuous stream.
 *  
 *  Parameters:
 *    
 *    alloc:
 *      A pointer to the CFAllocator which should be used to allocate
 *      memory for the CF read stream and its storage for values. If
 *      this reference is not a valid CFAllocator, the behavior is
 *      undefined.
 *    
 *    request:
 *      A pointer to a CFHTTPMessage created by the
 *      CFHTTPMessageCreateRequest function.
 *  
 *  Result:
 *    A pointer to the CF read stream created, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    read stream.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFReadStreamRef 
CFReadStreamCreateForHTTPRequest(
  CFAllocatorRef     alloc,
  CFHTTPMessageRef   request)                                 AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFReadStreamCreateForStreamedHTTPRequest()
 *  
 *  Discussion:
 *    Creates a read stream for the response to the given
 *    requestHeaders plus requestBody. Use in preference to
 *    CFReadStreamCreateForHTTPRequest() when the body of the request
 *    is larger than you wish to be resident in memory.  Note that
 *    because streams cannot be reset, read streams created this way
 *    cannot have autoredirection enabled.  If the Content-Length
 *    header is set in requestHeaders, it is assumed that the caller
 *    got the length right and that requestBody will report
 *    end-of-stream after precisely Content-Length bytes have been read
 *    from it. If the Content-Length header is not set, the chunked
 *    transfer-encoding will be added to requestHeaders, and bytes read
 *    from requestBody will be transmitted chunked. The body of
 *    requestHeaders is ignored.
 *  
 *  Parameters:
 *    
 *    alloc:
 *      A pointer to the CFAllocator which should be used to allocate
 *      memory for the CF read stream and its storage for values. If
 *      this reference is not a valid CFAllocator, the behavior is
 *      undefined.
 *    
 *    requestHeaders:
 *      A pointer to a CFHTTPMessage created by the
 *      CFHTTPMessageCreateRequest function. The body of requestHeaders
 *      is ignored.
 *    
 *    requestBody:
 *      A pointer to a CFReadStream.
 *  
 *  Result:
 *    A pointer to the CF read stream created, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    read stream.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFReadStreamRef 
CFReadStreamCreateForStreamedHTTPRequest(
  CFAllocatorRef     alloc,
  CFHTTPMessageRef   requestHeaders,
  CFReadStreamRef    requestBody)                             AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  CFHTTPReadStreamSetRedirectsAutomatically()   *** DEPRECATED ***
 *  
 *  Deprecated:
 *    Use the kCFStreamPropertyHTTPShouldAutoredirect property above
 *    instead.
 *  
 *  Discussion:
 *    Sets the redirection property on the http stream.
 *  
 *  Parameters:
 *    
 *    httpStream:
 *      A pointer to the CFHTTPStream to be set.
 *    
 *    shouldAutoRedirect:
 *      A boolean indicating whether to redirect or not.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework but deprecated in 10.3
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFHTTPReadStreamSetRedirectsAutomatically(
  CFReadStreamRef   httpStream,
  Boolean           shouldAutoRedirect)                       AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_3;


/*
 *  CFHTTPReadStreamSetProxy()   *** DEPRECATED ***
 *  
 *  Deprecated:
 *    Use the kCFStreamPropertyHTTPProxy above instead.
 *  
 *  Discussion:
 *    Sets the redirection property on the http stream.
 *  
 *  Parameters:
 *    
 *    httpStream:
 *      A pointer to the CFHTTPStream to be set.
 *    
 *    proxyHost:
 *      The proxy hostname. A CFString value.
 *    
 *    proxyPort:
 *      The port number. A CFNumber value.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework but deprecated in 10.3
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFHTTPReadStreamSetProxy(
  CFReadStreamRef   httpStream,
  CFStringRef       proxyHost,
  CFIndex           proxyPort)                                AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_3;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFHTTPSTREAM__ */

