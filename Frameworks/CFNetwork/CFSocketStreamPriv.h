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
     File:       CFNetwork/CFSocketStreamPriv.h
 
     Contains:   CoreFoundation Socket Stream SPI
 
     Copyright:  © 2002-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFSocketStreamPriv.i
                     Revision:        1.19
                     Dated:           2005/03/17 20:54:24
                     Last change by:  jwyld
                     Last comment:    4042459 ntlm authenticating proxies were not getting connect tunneling authorization for https
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFSOCKETSTREAMPRIV__
#define __CFSOCKETSTREAMPRIV__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __CFSTREAM__
#include <CoreFoundation/CFStream.h>
#endif





#include <AvailabilityMacros.h>

#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  kCFStreamSocketSecurityLevelTLSv1SSLv3
 *  
 *  Discussion:
 *    Stream property value, for both set and copy operations.
 *    Indicates to attempt TLSv1 with fallback to SSLv3.  SSLv2 is not
 *    attempted.
 *  
 */
extern const CFStringRef kCFStreamSocketSecurityLevelTLSv1SSLv3      AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFStreamPropertyUseAddressCache
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations. 
 *    CFBooleanRef to allow hostname lookup to use CFSocketStream's
 *    built-in address cache.  The value is kCFBooleanTrue by default.
 *  
 */
extern const CFStringRef kCFStreamPropertyUseAddressCache            AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

/*
 *  kCFStreamPropertyCONNECTProxy
 *  
 *  Discussion:
 *    Stream property key, for set operations.  To set a stream to use
 *    a CONNECT proxy, call CFReadStreamSetProperty or
 *    CFWriteStreamSetProperty with the property name set to
 *    kCFStreamPropertyCONNECTProxy and the value being a dictionary
 *    with at least the following two keys:
 *    kCFStreamPropertyCONNECTProxyHost. The dictionary returned by
 *    SystemConfiguration for proxies will work without alteration.
 *  
 */
extern const CFStringRef kCFStreamPropertyCONNECTProxy               AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFStreamPropertyCONNECTProxyHost
 *  
 *  Discussion:
 *    CFDictionary key for CONNECT proxy information.  The key
 *    kCFStreamPropertyCONNECTProxyHost should contain a CFStringRef
 *    value representing the CONNECT proxy host.  Defined to match
 *    kSCPropNetProxiesHTTPSProxy
 *  
 */
extern const CFStringRef kCFStreamPropertyCONNECTProxyHost           AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFStreamPropertyCONNECTProxyPort
 *  
 *  Discussion:
 *    CFDictionary key for CONNECT proxy information.  The key
 *    kCFStreamPropertyCONNECTProxyPort should contain a CFNumberRef
 *    which itself is of type kCFNumberSInt32Type.  This value should
 *    represent the port on which the proxy is listening.  Defined to
 *    match kSCPropNetProxiesHTTPSPort
 *  
 */
extern const CFStringRef kCFStreamPropertyCONNECTProxyPort           AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFStreamPropertyCONNECTVersion
 *  
 *  Discussion:
 *    CFDictionary key for CONNECT proxy information.  By default,
 *    kCFHTTPVersion1_0 will be used unless there is a
 *    kCFStreamPropertyCONNECTVersion key in the dictionary. Its value
 *    should be a CFStringRef representing the HTTP version to use for
 *    the CONNECT request.
 *  
 */
extern const CFStringRef kCFStreamPropertyCONNECTVersion             AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFStreamPropertyCONNECTAdditionalHeaders
 *  
 *  Discussion:
 *    CFDictionary key for CONNECT proxy information.  Its value should
 *    be a CFDictionary of header fields and their respective values. 
 *    This dictionary will be iterated and added to the CONNECT request.
 *  
 */
extern const CFStringRef kCFStreamPropertyCONNECTAdditionalHeaders   AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFStreamPropertyCONNECTResponse
 *  
 *  Discussion:
 *    Stream property key, for copy operations.  CFHTTPMessage holding
 *    the proxy server's response.
 *  
 */
extern const CFStringRef kCFStreamPropertyCONNECTResponse            AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFStreamPropertyPreviousCONNECTResponse
 *  
 *  Discussion:
 *    Stream property key, for copy operations.  CFHTTPMessage holding
 *    the proxy server's last response until the current one is fully
 *    valid again.  This is used for a CONNECT resume after a 407, for
 *    example.
 *  
 */
extern const CFStringRef kCFStreamPropertyPreviousCONNECTResponse    AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

#ifdef __MACH__
/*
 *  kCFStreamPropertySocketSSLContext
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFDataRef
 *    containing a reference to the SecureTransport SecureContext
 *    structure.
 *  
 */
extern const CFStringRef kCFStreamPropertySocketSSLContext;
/*
 *  _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations. 
 *    CFBooleanRef to set whether SSL authenticates the server's
 *    certificate or not.  The value is kCFBooleanTrue by default.
 *  
 */
extern const CFStringRef _kCFStreamPropertySocketSecurityAuthenticatesServerCertificate;
/*
 *  _kCFStreamPropertySSLClientCertificates
 *  
 *  Discussion:
 *    Stream property key for copy operations.  CFArrayRef containing
 *    SecCertificateRefs (except for element 0 of the array, which is a
 *    SecIdentityRef.) See SSLGetCertificate in
 *    Security/SecureTransportPriv.h for more information.
 *  
 */
extern const CFStringRef _kCFStreamPropertySSLClientCertificates     AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
/*
 *  _kCFStreamPropertySSLClientCertificateState
 *  
 *  Discussion:
 *    Stream property key for copy operations.  CFNumberRef wrapping a
 *    SSLClientCertificateState value. See SSLGetClientCertificateState
 *    in Security/SecureTransport.h for more information.
 *  
 */
extern const CFStringRef _kCFStreamPropertySSLClientCertificateState AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;
#endif  /* defined(__MACH__) */

/*
 *  kCFStreamPropertyProxyExceptionsList
 *  
 *  Discussion:
 *    CFDictionary key for proxy information, for both set and copy
 *    operations.  The value is a CFArray of hostname expressions for
 *    which we should bypass the proxy server. This key is used within
 *    the dictionary that serves as the value of the properties
 *    kCFStreamPropertySOCKSProxy, kCFStreamPropertyHTTPProxy or
 *    kCFStreamPropertyFTPProxy.
 *  
 */
extern const CFStringRef kCFStreamPropertyProxyExceptionsList;
/* matches kSCPropNetProxiesExceptionsList */
/*
 *  _kCFStreamPropertySocketPeerName
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  The value
 *    is a CFStringRef indicating the peer's host name.  This is to be
 *    set when doing SSL through a proxy, since the SocketStream's
 *    hostname will be the proxy instead of the peer.
 *  
 */
extern const CFStringRef _kCFStreamPropertySocketPeerName;
/*
 *  CFStreamCreatePairWithNetServicePieces()
 *  
 *  Discussion:
 *    Creates a pair of streams to a net service using the individual
 *    pieces of the net service instead of the net service itself.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The allocator to use for creation.
 *    
 *    domain:
 *      The network domain in which the service is registered.
 *    
 *    serviceType:
 *      The type of service being resolved on the network.
 *    
 *    name:
 *      The name of the machine or application advertising the service.
 *    
 *    readStream:
 *      Reference to a CFReadStreamRef which will be filled with the
 *      created read stream.  Pass in NULL if the read stream is not
 *      required.
 *    
 *    writeStream:
 *      Reference to a CFWriteStreamRef which will be filled with the
 *      created write stream.  Pass in NULL if the write stream is not
 *      required.
 *  
 */
extern void 
CFStreamCreatePairWithNetServicePieces(
  CFAllocatorRef      alloc,
  CFStringRef         domain,
  CFStringRef         serviceType,
  CFStringRef         name,
  CFReadStreamRef *   readStream,        /* can be NULL */
  CFWriteStreamRef *  writeStream)       /* can be NULL */    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



#ifdef __cplusplus
}
#endif

#endif /* __CFSOCKETSTREAMPRIV__ */

