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
     File:       CFNetwork/CFHTTPMessage.h
 
     Contains:   CoreFoundation Network socket streams header
 
     Copyright:  © 2001-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file may contain unreleased API's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFHTTPMessage.i
                     Revision:        1.4
                     Dated:           2004/08/03 22:57:05
                     Last change by:  jiarocci
                     Last comment:    Update Interface files to include headerdoc and remove warnings.
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFHTTPMESSAGE__
#define __CFHTTPMESSAGE__

#ifndef __CFNETWORKDEFS__
#include <CFNetwork/CFNetworkDefs.h>
#endif

#ifndef __CFSTRING__
#include <CoreFoundation/CFString.h>
#endif

#ifndef __CFURL__
#include <CoreFoundation/CFURL.h>
#endif



#include <AvailabilityMacros.h>

#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  kCFHTTPVersion1_0
 *  
 *  Discussion:
 *    Version string for HTTP 1.0.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFHTTPVersion1_0                           AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  kCFHTTPVersion1_1
 *  
 *  Discussion:
 *    Version string for HTTP 1.1.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFHTTPVersion1_1                           AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  kCFHTTPAuthenticationSchemeBasic
 *  
 *  Discussion:
 *    Basic HTTP authentication scheme.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFHTTPAuthenticationSchemeBasic            AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  kCFHTTPAuthenticationSchemeDigest
 *  
 *  Discussion:
 *    Digest HTTP authentication scheme.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.2 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFHTTPAuthenticationSchemeDigest           AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER;


/*
 *  CFHTTPMessageRef
 *  
 *  Discussion:
 *    This is the type of a reference to an HTTP message. An HTTP
 *    message can be a request or a response.
 */
typedef struct __CFHTTPMessage*         CFHTTPMessageRef;

/*
 *  CFHTTPMessageGetTypeID()
 *  
 *  Discussion:
 *    Return the unique type for this class.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Result:
 *    A unique CFType for CFHTTPMessage.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFTypeID 
CFHTTPMessageGetTypeID(void)                                  AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCreateRequest()
 *  
 *  Discussion:
 *    Create an HTTPMessage from an HTTP method, url and version.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      A pointer to the CFAllocator which should be used to allocate
 *      memory for the CF read stream and its storage for values. If
 *      this reference is not a valid CFAllocator, the behavior is
 *      undefined.
 *    
 *    requestMethod:
 *      A pointer to a CFString indicating the method of request. For a
 *      "GET" request, for example, the value would be CFSTR("GET").
 *    
 *    url:
 *      A pointer to a CFURL structure created any of the several
 *      CFURLCreate... functions.  If this parameter is not a pointer
 *      to a valid CFURL structure, the behavior is undefined.
 *    
 *    httpVersion:
 *      A pointer to a CFString indicating the version of request.
 *  
 *  Result:
 *    A pointer to the CFHTTPMessage created, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    message.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHTTPMessageRef 
CFHTTPMessageCreateRequest(
  CFAllocatorRef   alloc,
  CFStringRef      requestMethod,
  CFURLRef         url,
  CFStringRef      httpVersion)                               AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCreateResponse()
 *  
 *  Discussion:
 *    Create an HTTPMessage from an HTTP status code, description and
 *    version.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      A pointer to the CFAllocator which should be used to allocate
 *      memory for the CF read stream and its storage for values. If
 *      this reference is not a valid CFAllocator, the behavior is
 *      undefined.
 *    
 *    statusCode:
 *      An integer status code for the response.
 *    
 *    statusDescription:
 *      A pointer to a CFString for the status. Pass NULL to use the
 *      standard description for the given status code, as found in RFC
 *      2616.
 *    
 *    httpVersion:
 *      A pointer to a CFString for the HTTP version.
 *  
 *  Result:
 *    A pointer to the CFHTTPMessage created, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    message.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHTTPMessageRef 
CFHTTPMessageCreateResponse(
  CFAllocatorRef   alloc,
  int              statusCode,
  CFStringRef      statusDescription,
  CFStringRef      httpVersion)                               AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCreateEmpty()
 *  
 *  Discussion:
 *    Creates an empty request or response, which you can then append
 *    bytes to via CFHTTPMessageAppendBytes().
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      A pointer to the CFAllocator which should be used to allocate
 *      memory for the CF read stream and its storage for values. If
 *      this reference is not a valid CFAllocator, the behavior is
 *      undefined.
 *    
 *    isRequest:
 *      A boolean. Pass kCFBooleanTrue if the message should be a
 *      request.
 *  
 *  Result:
 *    A pointer to the CFHTTPMessage created, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    message.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHTTPMessageRef 
CFHTTPMessageCreateEmpty(
  CFAllocatorRef   alloc,
  Boolean          isRequest)                                 AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCreateCopy()
 *  
 *  Discussion:
 *    Creates a copy of a CFHTTPMessage.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      A pointer to the CFAllocator which should be used to allocate
 *      memory for the CF read stream and its storage for values. If
 *      this reference is not a valid CFAllocator, the behavior is
 *      undefined.
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A pointer to the CFHTTPMessage created, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    message.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFHTTPMessageRef 
CFHTTPMessageCreateCopy(
  CFAllocatorRef     alloc,
  CFHTTPMessageRef   message)                                 AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageIsRequest()
 *  
 *  Discussion:
 *    Returns whether the CFHTTPMessage is a request or a response.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A Boolean. A value of kCFBooleanTrue indicates the message is a
 *    request. A value of kCFBooleanFalse indicates the message is a
 *    response.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPMessageIsRequest(CFHTTPMessageRef message)              AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCopyVersion()
 *  
 *  Discussion:
 *    Returns the HTTP version.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A pointer to a CFString, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the string.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFStringRef 
CFHTTPMessageCopyVersion(CFHTTPMessageRef message)            AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCopyBody()
 *  
 *  Discussion:
 *    Returns the body of the message.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A pointer to a CFData, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the data.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFDataRef 
CFHTTPMessageCopyBody(CFHTTPMessageRef message)               AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageSetBody()
 *  
 *  Discussion:
 *    Sets the body of the message from a CFData.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *    
 *    bodyData:
 *      A pointer to a CFData containing the body to be set. If the
 *      bodyData is NULL, the behavior is undefined.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFHTTPMessageSetBody(
  CFHTTPMessageRef   message,
  CFDataRef          bodyData)                                AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCopyHeaderFieldValue()
 *  
 *  Discussion:
 *    Returns the specified header field.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *    
 *    headerField:
 *      A pointer to the CFString. If the headerField is NULL, the
 *      behavior is undefined.
 *  
 *  Result:
 *    A pointer to a CFString, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the string.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFStringRef 
CFHTTPMessageCopyHeaderFieldValue(
  CFHTTPMessageRef   message,
  CFStringRef        headerField)                             AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCopyAllHeaderFields()
 *  
 *  Discussion:
 *    Returns a CFDictionary containing all of the header fields.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A pointer to a CFDictionary, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the dictionary.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFDictionaryRef 
CFHTTPMessageCopyAllHeaderFields(CFHTTPMessageRef message)    AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageSetHeaderFieldValue()
 *  
 *  Discussion:
 *    Sets the value of the specified header field.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *    
 *    headerField:
 *      A pointer to the CFString. If headerField is NULL, the behavior
 *      is undefined.
 *    
 *    value:
 *      A pointer to the CFString containing the value to set. Set the
 *      value to NULL to remove the header field.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern void 
CFHTTPMessageSetHeaderFieldValue(
  CFHTTPMessageRef   message,
  CFStringRef        headerField,
  CFStringRef        value)                                   AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageAppendBytes()
 *  
 *  Discussion:
 *    Appends the given bytes to the message given (parsing out any
 *    control information if appropriate).  Returns kCFBooleanFalse if
 *    a parsing error occurs while processing the new data.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *    
 *    newBytes:
 *      A pointer to the bytes. If newBytes is NULL, the behavior is
 *      undefined.
 *    
 *    numBytes:
 *      A CFIndex of the number of bytes to append.
 *  
 *  Result:
 *    A Boolean indicating success or failure.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPMessageAppendBytes(
  CFHTTPMessageRef   message,
  const UInt8 *      newBytes,
  CFIndex            numBytes)                                AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageIsHeaderComplete()
 *  
 *  Discussion:
 *    Returns whether further header data is expected by the message.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    message:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A Boolean. A value of kCFBooleanTrue indicates the header is
 *    complete and no further data is expected.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPMessageIsHeaderComplete(CFHTTPMessageRef message)       AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCopySerializedMessage()
 *  
 *  Discussion:
 *    Creates a self-contained copy of a CFHTTPMessage. This would be
 *    suitable for persistant storage or for transmitting over the
 *    network independently.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    request:
 *      A pointer to the CFHTTPMessage to be seralized.
 *  
 *  Result:
 *    A pointer to a CFData, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the data.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFDataRef 
CFHTTPMessageCopySerializedMessage(CFHTTPMessageRef request)  AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*********************/
/* Request functions */
/*********************/

/*
 *  CFHTTPMessageCopyRequestURL()
 *  
 *  Discussion:
 *    Creates a copy of the request URL.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    request:
 *      A pointer to the CFHTTPMessage.
 *  
 *  Result:
 *    A pointer to a CFURL, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the url.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFURLRef 
CFHTTPMessageCopyRequestURL(CFHTTPMessageRef request)         AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCopyRequestMethod()
 *  
 *  Discussion:
 *    Creates a copy of the request method.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    request:
 *      A pointer to the CFHTTPMessage.
 *  
 *  Result:
 *    A pointer to a CFString, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the string.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFStringRef 
CFHTTPMessageCopyRequestMethod(CFHTTPMessageRef request)      AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageAddAuthentication()
 *  
 *  Discussion:
 *    Adds authentication to the request. Tries to modify request to
 *    contain the authentication information requested by the failed
 *    response (which presumably is a 401 or 407 response).
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    request:
 *      A pointer to a CFHTTPMessage request.
 *    
 *    authenticationFailureResponse:
 *      A pointer to a CFHTTPMessage of the failed response.
 *    
 *    username:
 *      A pointer to a CFString containing the user name to
 *      authenticate.
 *    
 *    password:
 *      A pointer to a CFString containing the password of the user.
 *    
 *    authenticationScheme:
 *      A pointer to a CFString containing the authentication scheme to
 *      use to authenticate. If authenticationScheme is NULL, strongest
 *      supported scheme listed authenticationFailureResponse will be
 *      used.
 *    
 *    forProxy:
 *      A boolean indicating whether the authentication applies to a
 *      proxy or not.
 *  
 *  Result:
 *    A pointer to a CFString, or NULL if failed. It is caller's
 *    responsibilty to release the memory allocated for the string.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern Boolean 
CFHTTPMessageAddAuthentication(
  CFHTTPMessageRef   request,
  CFHTTPMessageRef   authenticationFailureResponse,
  CFStringRef        username,
  CFStringRef        password,
  CFStringRef        authenticationScheme,
  Boolean            forProxy)                                AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/**********************/
/* Response functions */
/**********************/

/*
 *  CFHTTPMessageGetResponseStatusCode()
 *  
 *  Discussion:
 *    Returns the status code for the response.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    response:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A UInt32 indicating the status code.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern UInt32 
CFHTTPMessageGetResponseStatusCode(CFHTTPMessageRef response) AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;


/*
 *  CFHTTPMessageCopyResponseStatusLine()
 *  
 *  Discussion:
 *    Returns the status line for the response.
 *  
 *  Mac OS X threading:
 *    Not thread safe
 *  
 *  Parameters:
 *    
 *    response:
 *      A pointer to the CFHTTPMessage to be copied. If the message is
 *      NULL, the behavior is undefined.
 *  
 *  Result:
 *    A CFString indicating the status code, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    string.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.1 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFStringRef 
CFHTTPMessageCopyResponseStatusLine(CFHTTPMessageRef response) AVAILABLE_MAC_OS_X_VERSION_10_1_AND_LATER;



#ifdef __cplusplus
}
#endif

#endif /* __CFHTTPMESSAGE__ */

