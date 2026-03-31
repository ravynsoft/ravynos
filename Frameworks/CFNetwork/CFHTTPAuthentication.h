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
     File:       CFNetwork/CFHTTPAuthentication.h
 
     Contains:   CoreFoundation Network HTTP authentication header
 
     Copyright:  © 2001-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file may contain unreleased API's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHTTPAuthentication.i
                     Revision:        1.3
                     Dated:           2004/06/08 21:27:18
                     Last change by:  jwyld
                     Last comment:    Comment changes/cleanup
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHTTPAUTHENTICATION__
#define __CFHTTPAUTHENTICATION__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __COREFOUNDATION__
#include <CoreFoundation/CoreFoundation.h>
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
 *  CFHTTPAuthenticationRef
 *  
 *  Discussion:
 *    This is the type of a reference to HTTP authentication
 *    information.
 */
typedef struct _CFHTTPAuthentication*   CFHTTPAuthenticationRef;

/*
 *  CFStreamErrorHTTPAuthentication
 *  
 *  Discussion:
 *    Authentication errors which may be returned as a result of trying
 *    to apply authentication to a request.  These errors are in the
 *    kCFStreamErrorDomainHTTP domain.
 */
enum CFStreamErrorHTTPAuthentication {

  /*
   * The type of authentication to be applied to a request is not
   * supported.
   */
  kCFStreamErrorHTTPAuthenticationTypeUnsupported = -1000,

  /*
   * The username was in a format not suitable for applying to the
   * request.
   */
  kCFStreamErrorHTTPAuthenticationBadUserName = -1001,

  /*
   * The password was in a format not suitable for applying to the
   * request.
   */
  kCFStreamErrorHTTPAuthenticationBadPassword = -1002
};
typedef enum CFStreamErrorHTTPAuthentication CFStreamErrorHTTPAuthentication;


/*
 *  kCFHTTPAuthenticationUsername
 *  
 *  Discussion:
 *    CFDictionary key, for CFHTTPMessageApplyCredentialDictionary. The
 *    username for authentication as a CFString.  Needs to be added if
 *    CFHTTPAuthenticationRequiresUserNameAndPassword returns TRUE.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFHTTPAuthenticationUsername               AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFHTTPAuthenticationPassword
 *  
 *  Discussion:
 *    CFDictionary key, for CFHTTPMessageApplyCredentialDictionary. The
 *    password for authentication as a CFString.  Needs to be added if
 *    CFHTTPAuthenticationRequiresUserNameAndPassword returns TRUE.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFHTTPAuthenticationPassword               AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  kCFHTTPAuthenticationAccountDomain
 *  
 *  Discussion:
 *    CFDictionary key, for CFHTTPMessageApplyCredentialDictionary. The
 *    domain for authentication as a CFString.  Needs to be added if
 *    CFHTTPAuthenticationRequiresAccountDomain returns TRUE.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFHTTPAuthenticationAccountDomain          AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;

/*
 *  CFHTTPAuthenticationGetTypeID()
 *  
 *  Discussion:
 *    Returns the type identifier of all CFHTTPAuthentication instances.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFTypeID 
CFHTTPAuthenticationGetTypeID(void)                           AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPAuthenticationCreateFromResponse()
 *  
 *  Discussion:
 *    Based on a response of 401 or 407, this function will create a
 *    new authentication object which can be used for adding
 *    credentials to future requests.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The API's to CFHTTPAuthenticationRef are thread-safe so long as
 *    multiple threads are not altering the same
 *    CFHTTPAuthenticationRef at the same time.
 *  
 *  Parameters:
 *    
 *    alloc:
 *      Allocator to use for creating authentication object
 *    
 *    response:
 *      Failed response.
 *  
 *  Result:
 *    A freshly created authentication object useful for applying
 *    authentication credentials to new requests.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHTTPAuthenticationRef 
CFHTTPAuthenticationCreateFromResponse(
  CFAllocatorRef     alloc,
  CFHTTPMessageRef   response)                                AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPAuthenticationIsValid()
 *  
 *  Discussion:
 *    Returns TRUE if the given authentication information was
 *    instantiated correctly and contains enough information in order
 *    to be applied to a request. If FALSE is returned, the object may
 *    still contain information which is useful to the user, e.g.
 *    unsupported method name.  An invalid object may be queried for
 *    information but may not be applied to a request.
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
 *    error:
 *      Reference to a CFStreamError which will be populated in the
 *      case of an error in creation.  Pass NULL if not interested in
 *      the failure reason.  The error domain will be
 *      kCFStreamErrorDomainHTTP, and the error code will be one of
 *      those defined in CFHTTPStream.h or one of those listed as
 *      CFStreamErrorHTTPAuthentication.
 *  
 *  Result:
 *    TRUE or FALSE depending on whether the authentication object is
 *    good for applying credentials to further requests.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPAuthenticationIsValid(
  CFHTTPAuthenticationRef   auth,
  CFStreamError *           error)       /* can be NULL */    AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPAuthenticationAppliesToRequest()
 *  
 *  Discussion:
 *    Returns TRUE if the given request requires credentials based upon
 *    the given authentication information.
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
 *    request:
 *      The request which is believed to need the given authentication.
 *  
 *  Result:
 *    TRUE if the given authentication information should be applied to
 *    the request, otherwise FALSE is returned.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPAuthenticationAppliesToRequest(
  CFHTTPAuthenticationRef   auth,
  CFHTTPMessageRef          request)                          AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPAuthenticationRequiresOrderedRequests()
 *  
 *  Discussion:
 *    Some authentication methods require that future requests must be
 *    performed in an ordered manner, so that information from a
 *    response can be added to a following request.
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
 *    Returns TRUE if the given authentication method requires ordered
 *    requests.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPAuthenticationRequiresOrderedRequests(CFHTTPAuthenticationRef auth) AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPMessageApplyCredentials()
 *  
 *  Discussion:
 *    Perform the authentication method required on the request using
 *    the given username and password.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The API's to CFHTTPAuthenticationRef are thread-safe so long as
 *    multiple threads are not altering the same
 *    CFHTTPAuthenticationRef at the same time.
 *  
 *  Parameters:
 *    
 *    request:
 *      The request which is to receive the credentials.
 *    
 *    auth:
 *      The authentication information for the given request.
 *    
 *    username:
 *      The username to use for performing the authentication.
 *    
 *    password:
 *      The password to use for performing the authentication.
 *    
 *    error:
 *      Reference to a CFStreamError which will be populated with the
 *      error information should one occurr during the application of
 *      the credentials. Pass NULL if not interested in the failure
 *      reason.  The error domain will be kCFStreamErrorDomainHTTP, and
 *      the error code will be one of those define in CFHTTPStream.h or
 *      one of those listed as CFStreamErrorHTTPAuthentication.
 *  
 *  Result:
 *    TRUE will be returned if the application of the credentials to
 *    the request was successful, otherwise FALSE is returned.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPMessageApplyCredentials(
  CFHTTPMessageRef          request,
  CFHTTPAuthenticationRef   auth,
  CFStringRef               username,
  CFStringRef               password,
  CFStreamError *           error)          /* can be NULL */ AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPMessageApplyCredentialDictionary()
 *  
 *  Discussion:
 *    Perform the authentication method required on the request using
 *    the given credential information.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *    The API's to CFHTTPAuthenticationRef are thread-safe so long as
 *    multiple threads are not altering the same
 *    CFHTTPAuthenticationRef at the same time.
 *  
 *  Parameters:
 *    
 *    request:
 *      The request which is to receive the credentials.
 *    
 *    auth:
 *      The authentication information for the given request.
 *    
 *    dict:
 *      A dictionary containing credentials to be applied to the
 *      request.  Valid keys are declared above.
 *    
 *    error:
 *      Reference to a CFStreamError which will be populated with the
 *      error information should one occurr during the application of
 *      the credentials. Pass NULL if not interested in the failure
 *      reason.  The error domain will be kCFStreamErrorDomainHTTP, and
 *      the error code will be one of those define in CFHTTPStream.h or
 *      one of those listed as CFStreamErrorHTTPAuthentication.
 *  
 *  Result:
 *    TRUE will be returned if the application of the credentials to
 *    the request was successful, otherwise FALSE is returned.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPMessageApplyCredentialDictionary(
  CFHTTPMessageRef          request,
  CFHTTPAuthenticationRef   auth,
  CFDictionaryRef           dict,
  CFStreamError *           error)         /* can be NULL */  AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



/*
 *  CFHTTPAuthenticationCopyRealm()
 *  
 *  Discussion:
 *    Some authentication techniques provide for namespaces on top of
 *    domains. This call will return the authentication information's
 *    namespace if there is one, otherwise it will return NULL.  This
 *    namespace is usually used for prompting the application user for
 *    a name and password.
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
 *    This call will return the authentication information's namespace
 *    if there is one, otherwise it will return NULL.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFStringRef 
CFHTTPAuthenticationCopyRealm(CFHTTPAuthenticationRef auth)   AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPAuthenticationCopyDomains()
 *  
 *  Discussion:
 *    Returns a list of domain URL's on which the given authentication
 *    should be applied. This function is provided mostly for
 *    informational purposes. CFHTTPAuthenticationAppliesToRequest
 *    should be used in order to check whether a request requires the
 *    authentication.
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
 *    Returns a list of domain URL's on which the given authentication
 *    should be applied.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFArrayRef 
CFHTTPAuthenticationCopyDomains(CFHTTPAuthenticationRef auth) AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPAuthenticationCopyMethod()
 *  
 *  Discussion:
 *    Returns the method of authentication which will be performed when
 *    applying credentials.  The strongest method of authentication
 *    will be chosen in the case of multiple choices.
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
 *    Returns the method of authentication which will be performed when
 *    applying credentials.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFStringRef 
CFHTTPAuthenticationCopyMethod(CFHTTPAuthenticationRef auth)  AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;



/*
 *  CFHTTPAuthenticationRequiresUserNameAndPassword()
 *  
 *  Discussion:
 *    Returns TRUE if the chosen authentication scheme requires a
 *    username and password.
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
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPAuthenticationRequiresUserNameAndPassword(CFHTTPAuthenticationRef auth) AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  CFHTTPAuthenticationRequiresAccountDomain()
 *  
 *  Discussion:
 *    Returns TRUE if the chosen authentication scheme requires a
 *    domain for authentication.  Currently, this will return TRUE for
 *    "NTLM" and FALSE for the other methods.
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
 *    domain for authentication.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.4 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPAuthenticationRequiresAccountDomain(CFHTTPAuthenticationRef auth) AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER;



#if PRAGMA_ENUM_ALWAYSINT
    #pragma enumsalwaysint reset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __CFHTTPAUTHENTICATION__ */

