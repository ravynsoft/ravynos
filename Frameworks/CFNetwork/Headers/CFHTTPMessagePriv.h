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
     File:       CFNetwork/CFHTTPMessagePriv.h
 
     Contains:   CoreFoundation Network HTTP message private header
 
     Copyright:  © 2001-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file contains unreleased SPI's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHTTPMessagePriv.i
                     Revision:        1.12
                     Dated:           2005/01/18 22:12:43
                     Last change by:  jwyld
                     Last comment:    3663096 Make NTLM work for Safari, WebDAV, AB, and hopefully all others.
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHTTPMESSAGEPRIV__
#define __CFHTTPMESSAGEPRIV__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __COREFOUNDATION__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifndef __CFHTTPMESSAGE__
#include <CFNetwork/CFHTTPMessage.h>
#endif

#ifndef __CFHTTPAUTHENTICATION__
#include <CFNetwork/CFHTTPAuthentication.h>
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
 *  _CFHTTPMessageSetResponseURL()
 *  
 *  Discussion:
 *    Adds the given url to the response.  Responses don't have URL's
 *    associated with them.  This function allows the association to be
 *    made.  This is required for Digest authentication. This function
 *    is provided primarily for hand-rolled responses.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The API's to CFHTTPMessageRef are thread-safe so long as multiple
 *    threads are not altering the same CFHTTPMessageRef at the same
 *    time.
 *  
 *  Parameters:
 *    
 *    response:
 *      HTTP message which is to get the url
 *    
 *    url:
 *      URL to be associated with the response.
 *  
 */
extern void 
_CFHTTPMessageSetResponseURL(
  CFHTTPMessageRef   response,
  CFURLRef           url)                                     AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  _CFHTTPAuthenticationUpdateFromResponse()
 *  
 *  Discussion:
 *    Updates an authentication object with carry-over information from
 *    a HTTP response.  Some authentication types use information from
 *    the previous response in order to formulate new request
 *    authorization.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    auth:
 *      Authentication object to have updated
 *    
 *    response:
 *      HTTP message with the carry-over information
 *    
 *    conn:
 *      Connection identifier to which this response is associated.
 *  
 */
extern void 
_CFHTTPAuthenticationUpdateFromResponse(
  CFHTTPAuthenticationRef   auth,
  CFHTTPMessageRef          response,
  const void *              conn)                             AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  CFHTTPAuthenticationAllowsSingleSignOn()
 *  
 *  Discussion:
 *    Returns TRUE if the chosen authentication scheme requires a
 *    username and password.  Currently, this will return FALSE for
 *    "Negotiate" and TRUE for the other methods.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The API's to CFHTTPAuthenticationRef are thread-safe so long as
 *    multiple threads are not altering the same
 *    CFHTTPAuthenticationRef at the same time.
 *  
 *  Parameters:
 *    
 *    auth:
 *      The authentication information being queried.
 *  
 *  Result:
 *    Returns TRUE if the chosen authentication scheme requires a
 *    username and password.
 *  
 */
extern Boolean 
CFHTTPAuthenticationAllowsSingleSignOn(CFHTTPAuthenticationRef auth) AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  _CFHTTPAuthenticationPasswordInClear()
 *  
 *  Discussion:
 *    Returns TRUE if the password is sent in an unencrypted manner. 
 *    Currently, this will return TRUE for Basic authentication and
 *    FALSE for Digest Authentication.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The API's to CFHTTPAuthenticationRef are thread-safe so long as
 *    multiple threads are not altering the same
 *    CFHTTPAuthenticationRef at the same time.
 *  
 *  Parameters:
 *    
 *    auth:
 *      The authentication information being queried.
 *  
 *  Result:
 *    Returns TRUE if the password is sent in an unencrypted manner.
 *  
 */
extern Boolean 
_CFHTTPAuthenticationPasswordInClear(CFHTTPAuthenticationRef auth) AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  _CFHTTPAuthenticationCopyServerSupportedSchemes()
 *  
 *  Discussion:
 *    Returns a CFArray containing the names of the methods of
 *    authentication that the server supports.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    auth:
 *      The authentication information being queried.
 *  
 *  Result:
 *    Returns a CFArray of CFStringRef's which are the names of the
 *    supported schemes.  NULL will be returned if the authentication
 *    object is not valid.
 *  
 */
extern CFArrayRef 
_CFHTTPAuthenticationCopyServerSupportedSchemes(CFHTTPAuthenticationRef auth) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFHTTPAuthenticationSetPreferredScheme()
 *  
 *  Discussion:
 *    Forces this authentication object to use the given scheme instead
 *    of its chosen scheme.  This call must be made prior to first use,
 *    so preferrably right after the authentication object is created.
 *    Keep in mind some authentication objects are carried over from
 *    one response to another.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    auth:
 *      The authentication information to be adjusted.
 *    
 *    scheme:
 *      The name of the new preferred scheme.
 *  
 *  Result:
 *    Returns TRUE on success and FALSE on failure.
 *  
 */
extern Boolean 
_CFHTTPAuthenticationSetPreferredScheme(
  CFHTTPAuthenticationRef   auth,
  CFStringRef               scheme)                           AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFHTTPAuthenticationApplyHeaderToRequest()
 *  
 *  Discussion:
 *    Sets the required authorization or proxy authorization header on
 *    the request which is to be sent out on the given request.  This
 *    is needed for connection-based authentication schemes since the
 *    connection association for a request is not known until late.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    auth:
 *      The authentication information to be used.
 *    
 *    request:
 *      The request to gain the authorization headers.
 *    
 *    connection:
 *      The connection to which the request is associated.  This
 *      represents the pipe on which the request will be sent.
 *  
 *  Result:
 *    Returns a CFStreamError.  The error field will be zero if there
 *    is no error.
 *  
 */
extern CFStreamError 
_CFHTTPAuthenticationApplyHeaderToRequest(
  CFHTTPAuthenticationRef   auth,
  CFHTTPMessageRef          request,
  const void *              connection)                       AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFHTTPAuthenticationDisassociateConnection()
 *  
 *  Discussion:
 *    Breaks the association between an authentication object and a
 *    connection reference.  This association is first made with a call
 *    to _CFHTTPAuthenticationApplyHeaderToRequest.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    Thread safe.
 *  
 *  Parameters:
 *    
 *    auth:
 *      The authentication information to be used.
 *    
 *    connection:
 *      The connection to which the authentication object is referenced.
 *  
 */
extern void 
_CFHTTPAuthenticationDisassociateConnection(
  CFHTTPAuthenticationRef   auth,
  const void *              connection)                       AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFHTTPMessageCanRetry()
 *  
 *  Discussion:
 *    Upon a failure of a request which had credentials applied, this
 *    function will return TRUE if it believes that another attempt
 *    might succeed if the credentials are applied to a new request. 
 *    The caller should differentiate between 407 and 401 errors before
 *    calling this, since it is possible to get a 407 followed by a 401
 *    in a legal progress of fetching.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The API's to CFHTTPAuthenticationRef are thread-safe so long as
 *    multiple threads are not altering the same
 *    CFHTTPAuthenticationRef at the same time.
 *  
 *  Parameters:
 *    
 *    response:
 *      The failed response being queried.
 *  
 *  Result:
 *    Returns TRUE if it is believed that re-applying credentials might
 *    allow the request to succeed, otherwise FALSE is returned.
 *  
 */
extern Boolean 
_CFHTTPMessageCanRetry(CFHTTPMessageRef response)             AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;





/*
 *  _CFGregorianDateCreateWithBytes()
 *  
 *  Discussion:
 *    Parses RFC 850, RFC 1123, and asctime formatted date/time strings.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The allocator to be used for new allocations.
 *    
 *    bytes:
 *      Pointer to a byte buffer to be parsed.
 *    
 *    length:
 *      Number of bytes located at the bytes pointer location.
 *    
 *    date:
 *      Reference to a CFGregorianDate structure to be filled with the
 *      parse results.
 *    
 *    tz:
 *      Reference to a CFTimeZoneRef if the time zone information is to
 *      be parsed.  Pass NULL if the time zone is not to be parsed. 
 *      This will be allocated with the passed in allocator argument.
 *  
 *  Result:
 *    Returns the location in the buffer where parsing finished.  If
 *    nothing was parsed as a result of failure, the result will be
 *    bytes.  If the time zone could not parse, tz will be set to NULL
 *    but a result not equal to bytes will be returned.
 *  
 */
extern const UInt8 * 
_CFGregorianDateCreateWithBytes(
  CFAllocatorRef     alloc,
  const UInt8 *      bytes,
  CFIndex            length,
  CFGregorianDate *  date,
  CFTimeZoneRef *    tz)                                      AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFGregorianDateCreateWithString()
 *  
 *  Discussion:
 *    Parses RFC 850, RFC 1123, and asctime formatted date/time strings.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The allocator to be used for new allocations.
 *    
 *    str:
 *      CFStringRef to be parsed.
 *    
 *    date:
 *      Reference to a CFGregorianDate structure to be filled with the
 *      parse results.
 *    
 *    tz:
 *      Reference to a CFTimeZoneRef if the time zone information is to
 *      be parsed.  Pass NULL if the time zone is not to be parsed. 
 *      This will be allocated with the passed in allocator argument.
 *  
 *  Result:
 *    Same as _CFGregorianDateCreateWithBytes but returns the count of
 *    characters parsed.
 *  
 */
extern CFIndex 
_CFGregorianDateCreateWithString(
  CFAllocatorRef     alloc,
  CFStringRef        str,
  CFGregorianDate *  date,
  CFTimeZoneRef *    tz)                                      AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFStringCreateRFC1123DateStringWithGregorianDate()
 *  
 *  Discussion:
 *    Creates a RFC 1123 formatted date and time string.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The allocator to be used for the new allocation.
 *    
 *    date:
 *      Reference to a CFGregorianDate structure to be used.
 *    
 *    tz:
 *      Time zone reference if offset is different than GMT.  If NULL
 *      is passed, the offset is assumed to be +0000.
 *  
 *  Result:
 *    A string representing the passed in date and time in the RFC 1123
 *    format.
 *  
 */
extern CFStringRef 
_CFStringCreateRFC1123DateStringWithGregorianDate(
  CFAllocatorRef     alloc,
  CFGregorianDate *  date,
  CFTimeZoneRef      tz)                                      AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  _CFStringCreateRFC2616DateStringWithGregorianDate()
 *  
 *  Discussion:
 *    Creates a RFC 2616 formatted date and time string.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      The allocator to be used for the new allocation.
 *    
 *    date:
 *      Reference to a CFGregorianDate structure to be used.
 *    
 *    tz:
 *      Time zone reference if offset is different than GMT.  If NULL
 *      is passed, the time zone is assumed to be GMT.
 *  
 *  Result:
 *    A string representing the passed in date and time in the RFC 2616
 *    format.
 *  
 */
extern CFStringRef 
_CFStringCreateRFC2616DateStringWithGregorianDate(
  CFAllocatorRef     alloc,
  CFGregorianDate *  date,
  CFTimeZoneRef      tz)                                      AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFHTTPMESSAGEPRIV__ */

