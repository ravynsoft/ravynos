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
     File:       CFNetwork/CFFTPStream.h
 
     Contains:   CoreFoundation FTP stream header
 
     Copyright:  © 2001-2005 by Apple Computer, Inc., all rights reserved
 
     Warning:    *** APPLE INTERNAL USE ONLY ***
                 This file may contain unreleased API's
 
     BuildInfo:  Built by:            anonymous
                 On:                  Wed Apr 27 10:45:36 2005
                 With Interfacer:     3.0d46   (Mac OS X for PowerPC)
                 From:                CFFTPStream.i
                     Revision:        1.9
                     Dated:           2004/06/01 17:53:05
                     Last change by:  rew
                     Last comment:    Updating all copyrights to include 2004
 
     Bugs:       Report bugs to Radar component "System Interfaces", "Latest"
                 List the version information (from above) in the Problem Description.
 
*/
#ifndef __CFFTPSTREAM__
#define __CFFTPSTREAM__

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
 *  kCFStreamErrorDomainFTP
 *  
 *  Discussion:
 *    Result code returned by FTP server
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const SInt32 kCFStreamErrorDomainFTP                          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
FTP Stream Property keys.  These keys can be passed to the stream
property "set/get" functions, such as CFReadStreamSetProperty/
CFReadStreamCopyProperty, or to a CFDictionary creator or an item
accessor/mutator.  The comment before each key declaration (treated
as definition) indicates the value type of the property.
*/


/*
 *  kCFStreamPropertyFTPUserName
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFString
 *    type to hold login user name.  Don't set this property if you
 *    want anonymous FTP.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPUserName                AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPPassword
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations. CFString
 *    type to hold login password.  Don't set this property if you want
 *    anonymous FTP.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPPassword                AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPUsePassiveMode
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations. CFBoolean
 *    type. kCFBooleanTrue means use passive mode, kCFBooleanFalse
 *    otherwise
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPUsePassiveMode          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPResourceSize
 *  
 *  Discussion:
 *    Stream property key, for read stream copy operations.  CFNumber
 *    of kCFNumberLongLongType to hold resource size in bytes.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPResourceSize            AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPFetchResourceInfo
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFBoolean
 *    type.  TRUE means that resource info, such as size, must be
 *    provided before download starts at higher cost.  Don't set if
 *    resource size/other info is unnecessary.  Initially, only
 *    resource size is implemented.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPFetchResourceInfo       AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPFileTransferOffset
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFNumber
 *    of kCFNumberLongLongType for the file offset to start transfer at.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPFileTransferOffset      AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPAttemptPersistentConnection
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations.  CFBoolean
 *    type.  TRUE by default, set to FALSE to avoid reusing existing
 *    server connections.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPAttemptPersistentConnection AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPProxy
 *  
 *  Discussion:
 *    Stream property key, for both set and copy operations. 
 *    CFDictionary type that holds key-value pairs of proxy dictionary.
 *     The dictionary returned by SystemConfiguration can also be
 *    passed directly as the value.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPProxy                   AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPProxyHost
 *  
 *  Discussion:
 *    Stream property key or FTP Proxy dictionary key, for both set and
 *    copy operations.  It matches kSCPropNetProxiesFTPProxy defined in
 *    SCSchemaDefinitions.h.  CFString for proxy server host name. 
 *    This property can be set and copied individually or via a
 *    CFDictionary.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPProxyHost               AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFStreamPropertyFTPProxyPort
 *  
 *  Discussion:
 *    Stream property key or FTP Proxy dictionary key, for both set and
 *    copy operations.  It matches kSCPropNetProxiesFTPPort defined in
 *    SCSchemaDefinitions.h.  CFNumber of kCFNumberIntType for proxy
 *    server port number.  This property can be set and copied
 *    individually or via a CFDictionary.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPProxyPort               AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;

/*
 *  kCFStreamPropertyFTPProxyUser
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPProxyUser               AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;
/*
 *  kCFStreamPropertyFTPProxyPassword
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFStreamPropertyFTPProxyPassword           AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
CFDictionary keys for resource information.  The information is
extracted from a line of the directory list by function
CFFTPCreateParsedResourceListing.
*/


/*
 *  kCFFTPResourceMode
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFNumber to hold the
 *    resource access permission defined in sys/types.h.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceMode                          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFFTPResourceName
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFString that holds
 *    the resource name.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceName                          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFFTPResourceOwner
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFString that holds
 *    the resource owner's name.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceOwner                         AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFFTPResourceGroup
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFString to hold the
 *    name of the group that shares the resource.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceGroup                         AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFFTPResourceLink
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFString to hold
 *    symbolic link information.  If the item is a symbolic link the
 *    string will contain the path to the item the link references.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceLink                          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFFTPResourceSize
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFNumber of
 *    kCFNumberLongLongType to hold the resource length in bytes.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceSize                          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFFTPResourceType
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFNumber to hold the
 *    resource type as defined in sys/dirent.h.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceType                          AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  kCFFTPResourceModDate
 *  
 *  Discussion:
 *    CFDictionary key, for get value operation.  CFDate to hold the
 *    last modification date and time information.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern const CFStringRef kCFFTPResourceModDate                       AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;


/*
 *  CFReadStreamCreateWithFTPURL()
 *  
 *  Discussion:
 *    Create an FTP read stream for downloading operation from an FTP
 *    URL. If the URL refers to a directory, the stream is a filtered
 *    line-at-a-time read stream corresponding to the listing results
 *    provided by the server. If it's a file, then the stream is a
 *    regular read stream providing the data for that file.
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
 *    ftpURL:
 *      A pointer to a CFURL structure created by CFURLCreateWithString
 *      function.  If this parameter is not a pointer to a valid CFURL
 *      structure, the behavior is undefined.
 *  
 *  Result:
 *    A pointer to the CF read stream created, or NULL if failed. It is
 *    caller's responsibilty to release the memory allocated for the
 *    read stream.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFReadStreamRef 
CFReadStreamCreateWithFTPURL(
  CFAllocatorRef   alloc,
  CFURLRef         ftpURL)                                    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  CFFTPCreateParsedResourceListing()
 *  
 *  Discussion:
 *    Parse a line of file or folder listing of Unix format, and store
 *    the extracted result in a CFDictionary.
 *  
 *  Mac OS X threading:
 *    Thread safe
 *  
 *  Parameters:
 *    
 *    alloc:
 *      A pointer to the CFAllocator which should be used to allocate
 *      memory for the CFDictionary to hold resource info.  If this
 *      reference is not a valid CFAllocator, the behavior is undefined.
 *    
 *    buffer:
 *      A pointer to a buffer that may hold lines of resource listing,
 *      but only the first line starting from buffer[0] will be parsed
 *      each call.
 *    
 *    bufferLength:
 *      The maximum buffer size in bytes started from the location
 *      pointed by "buffer."
 *    
 *    parsed:
 *      A pointer to a CFDictionary pointer.  The dictionary holds the
 *      extracted resource information.  When parsing fails, a NULL
 *      pointer will be returned.  It is caller's responsibilty to
 *      release the memory allocated for the dictionary.
 *  
 *  Result:
 *    The number of bytes consumed from buffer, 0 if there are not
 *    enough bytes, or -1 if a parse failure occurs.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFIndex 
CFFTPCreateParsedResourceListing(
  CFAllocatorRef     alloc,
  const UInt8 *      buffer,
  CFIndex            bufferLength,
  CFDictionaryRef *  parsed)                                  AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



/*
 *  CFWriteStreamCreateWithFTPURL()
 *  
 *  Discussion:
 *    Create an FTP write stream for uploading operation to a FTP URL.
 *    If the URL specifies a directory, the open will be followed by a
 *    close event/state and the directory will have been created. 
 *    Intermediary directory structure is not created.
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
 *    ftpURL:
 *      A pointer to a CFURL structure created by CFURLCreateWithString
 *      function.  If this parameter is not a pointer to a valid CFURL
 *      structure, the behavior is undefined.
 *  
 *  Result:
 *    A pointer to the CF write stream created, or NULL if failed. It
 *    is caller's responsibilty to release the memory allocated for the
 *    write stream.
 *  
 *  Availability:
 *    Mac OS X:         in version 10.3 and later in CoreServices.framework
 *    CarbonLib:        not available
 *    Non-Carbon CFM:   not available
 */
extern CFWriteStreamRef 
CFWriteStreamCreateWithFTPURL(
  CFAllocatorRef   alloc,
  CFURLRef         ftpURL)                                    AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER;



#ifdef __cplusplus
}
#endif

#endif /* __CFFTPSTREAM__ */

