/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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

/*!
    @header		CFOpenDirectory
    @abstract   OpenDirectory is built as a CFRuntimeBase object to facilitate calling Open Directory APIs
    @discussion The goal of these APIs is to supply a common library with functions for CF-style usage to
                manipulate data in Open Directory supported Directories.  All API are completely thread safe due 
                to the nature of shared objects implementation.
*/

#ifndef __CFOPENDIRECTORY_H
#define __CFOPENDIRECTORY_H

#include <dispatch/dispatch.h>
#include <CoreFoundation/CoreFoundation.h> 
#include <opendirectory/odconstants.h>

#pragma mark Typedefs

/*!
    @typedef	ODSessionRef
    @abstract   Opaque reference for ODSession object
*/
typedef struct _ODSessionRef    *ODSessionRef;
    
/*!
    @typedef    ODNodeRef
    @abstract   Opaque reference for the ODNode object
*/
typedef struct _ODNode          *ODNodeRef;

/*!
    @typedef    ODRecordRef
    @abstract   Opaque reference for the ODRecord object
*/
typedef struct _ODRecord        *ODRecordRef;

/*!
    @typedef    ODContextRef
    @abstract   Opaque reference for the ODContext object
*/
typedef struct _ODContext       *ODContextRef;

/*!
    @typedef    ODQueryRef
    @abstract   Opaque reference for the ODQuery object
*/
typedef struct _ODQuery         *ODQueryRef;

/*!
    @typedef    ODQueryCallback
    @abstract   Is called as results are returned from a query.  The incoming result must be retained or copied.
    @discussion Is called as results are returned from an CFRunLoop-based query.  These results are only partial
                and the callback is called repeatedly as results are available.  The incoming result must be retained or copied.  The 
                array will be released by the CFRunLoop upon return.  Incoming results do not include previous results,
                only the most resent results are returned.  inResults can be NULL if an error occurs or the query is complete.  If 
                inError and inResults are NULL then the query has completed.
*/
typedef void (*ODQueryCallback)( ODQueryRef inQuery, CFArrayRef inResults, CFErrorRef inError, void *inUserInfo );

#pragma mark -
#pragma mark Const

/*!
    @const      kODSessionDefault
    @abstract   is the default type of ODSessionRef used if there is no need to create a specific reference
    @discussion is the default type of ODSessionRef used if there is no need to create a specific reference
*/
CF_EXPORT 
ODSessionRef kODSessionDefault;

/*!
    @const      kODSessionProxyAddress
    @abstract   the address to connect to via proxy, used when making the options dictionary
    @discussion the address to connect to via proxy, used when making the options dictionary
*/
CF_EXPORT
const CFStringRef kODSessionProxyAddress;

/*!
    @const      kODSessionProxyPort
    @abstract   the port to connect to via proxy, used when making the options dictionary
    @discussion the port to connect to via proxy, used when making the options dictionary.  This parameter
                is optional and should not be passed normally.
*/
CF_EXPORT
const CFStringRef kODSessionProxyPort;

/*!
    @const      kODSessionProxyUsername
    @abstract   the username to connect with via proxy, used when making the options dictionary
    @discussion the username to connect with via proxy, used when making the options dictionary
*/
CF_EXPORT
const CFStringRef kODSessionProxyUsername;

/*!
    @const      kODSessionProxyPassword
    @abstract   the password to connect with via proxy, used when making the options dictionary
    @discussion the password to connect with via proxy, used when making the options dictionary
*/
CF_EXPORT
const CFStringRef kODSessionProxyPassword;

/*!
    @const      kODErrorDomainFramework
    @abstract   the error domain for OpenDirectory.framework details
    @discussion the error domain for OpenDirectory.framework details
*/
CF_EXPORT
const CFStringRef kODErrorDomainFramework;


#pragma mark -
#pragma mark ODContext object functions

__BEGIN_DECLS

/*!
    @function   ODContextGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODContext object
    @result     a valid CFTypeID for the ODContext object
*/
CF_EXPORT
CFTypeID ODContextGetTypeID( void );

#pragma mark -
#pragma mark ODQuery object function calls

/*!
    @function   ODQueryGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODQuery object
    @result     a valid CFTypeID for the ODQuery object
*/
CF_EXPORT
CFTypeID ODQueryGetTypeID( void );

/*!
    @function   ODQueryCreateWithNode
    @abstract   Creates a query with the node using the parameters provided
    @discussion Creates a query with the node using the supplied query parameters.  Some parameters can either be CFString or
                CFData or a CFArray of either CFString or CFData.
    @param      inAllocator a memory allocator to use for this object
    @param      inNode an ODNodeRef to use
    @param      inRecordTypeOrList a CFString of a type or CFArray with a list of record types
    @param      inAttribute a CFStringRef of the attribute name to query
    @param      inMatchType an ODMatchType value that signifies the type of query
    @param      inQueryValueOrList a CFStringRef, CFDataRef or CFArrayRef of either type for values to query in attribute
    @param      inReturnAttributeOrList a CFStringRef or CFArrayRef of CFStrings with the list of attributes to be returned
                from the query.  Passing NULL is equivalent to passing kODAttributeTypeStandardOnly.
    @param      inMaxResults a CFIndex of the total number of values the caller wants to be returned
    @param      outError an optional CFErrorRef reference for error details
    @result     an ODQueryRef which should be passed into ODQueryCopyResults for immediate results or
                ODQueryScheduleWithRunLoop for background behavior
*/
CF_EXPORT
ODQueryRef ODQueryCreateWithNode( CFAllocatorRef inAllocator, ODNodeRef inNode, CFTypeRef inRecordTypeOrList, 
                                  ODAttributeType inAttribute, ODMatchType inMatchType, CFTypeRef inQueryValueOrList, 
                                  CFTypeRef inReturnAttributeOrList, CFIndex inMaxResults, CFErrorRef *outError );

/*!
    @function   ODQueryCreateWithNodeType
    @abstract   Creates a query object that is initialized to a particular node type.
    @discussion Creates a query object that is initialized to a particular node type using the supplied
                query options.
    @param      inAllocator a memory allocator to use for this object
    @param      inType an ODNodeType to use when doing a query
    @param      inRecordTypeOrList a ODRecordType of a type or CFArray with a list of record types
    @param      inAttribute a ODAttributeType or CFStringRef of the attribute name to query
    @param      inMatchType an ODMatchType value that signifies the type of query
    @param      inQueryValueOrList a CFStringRef, CFDataRef or CFArrayRef of either type for values to query in attribute
    @param      inReturnAttributeOrList a CFStringRef or CFArrayRef of CFStrings with the list of attributes to be returned
                from the query.  Passing NULL is equivalent to passing kODAttributeTypeStandardOnly.
    @param      inMaxResults a CFIndex of the total number of values the caller wants to be returned
    @param      outError an optional CFErrorRef reference for error details
    @result     an ODQueryRef which should be passed into ODQueryCopyResults for immediate results or
                ODQueryScheduleWithRunLoop for background behavior, see ODQueryCallback for details on RunLoop
                behavior.
*/
CF_EXPORT
ODQueryRef ODQueryCreateWithNodeType( CFAllocatorRef inAllocator, ODNodeType inType, CFTypeRef inRecordTypeOrList,
									  ODAttributeType inAttribute, ODMatchType inMatchType, CFTypeRef inQueryValueOrList, 
									  CFTypeRef inReturnAttributeOrList, CFIndex inMaxResults, CFErrorRef *outError );

/*!
    @function   ODQueryCopyResults
    @abstract   Returns results from a provided ODQueryRef synchronously
    @discussion Returns results from a provided ODQueryRef synchronously.  Passing false to inAllowPartialResults
                will block the call until all results are returned or an error occurs.  true can be passed at any time
                even if previous calls were made with false.
    @param      inQuery an ODQueryRef to use
    @param      inAllowPartialResults a bool, passing true to retrieve any currently available results, or false to
                wait for all results
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFArrayRef comprised of ODRecord objects.  If partial results were requested but are complete, then
                NULL will be returned with outError set to NULL. If an error occurs, NULL will be returned and 
                outError should be checked accordingly.
*/
CF_EXPORT
CFArrayRef ODQueryCopyResults( ODQueryRef inQuery, bool inAllowPartialResults, CFErrorRef *outError );

/*!
    @function   ODQuerySynchronize
    @abstract   Will dispose of any results and restart the query.
    @discussion Will dispose of any results and restart the query for subsequent ODQueryCopyResults.  If the query
                is currently scheduled on a RunLoop, then the callback function will be called with inResults == NULL and
                inError.error == kODErrorQuerySynchronize and inError.domain == kODErrorDomainFramework, signifying that
                all existing results should be thrown away in preparation for new results.
    @param      inQuery an ODQueryRef to use
*/
CF_EXPORT
void ODQuerySynchronize( ODQueryRef inQuery );

/*!
    @function   ODQuerySetCallback
    @abstract   This call is used to set the callback function for an asynchronous query
    @discussion This call is used to set the callback function for an asynchronous query, using a
                CFRunLoop or a dispatch queue.
    @param      inQuery an ODQueryRef to use
    @param      inCallback a function to call when a query has results to return
    @param      inUserInfo a user-defined pointer to be passed back to the Query callback function
*/
CF_EXPORT
void ODQuerySetCallback( ODQueryRef inQuery, ODQueryCallback inCallback, void *inUserInfo );

/*!
    @function   ODQueryScheduleWithRunLoop
    @abstract   Allows a query to run off of a runloop, though it will spawn a thread to handle the work
    @discussion Allows a query to run off of a runloop, though it will spawn a thread to handle the work.
                When query is complete or stopped the callback function will be called with NULL results 
                and inError set to NULL.  ODQueryUnscheduleFromRunLoop() must be called to remove this query
                from Runloops if necessary.
    @param      inQuery an ODQueryRef to put on the runloop
    @param      inRunLoop a CFRunLoopRef to put the ODQueryRef source onto
    @param      inRunLoopMode a CFStringRef with the runloop mode to add the ODQueryRef to
*/
CF_EXPORT
void ODQueryScheduleWithRunLoop( ODQueryRef inQuery, CFRunLoopRef inRunLoop, CFStringRef inRunLoopMode );

/*!
    @function   ODQueryUnscheduleFromRunLoop
    @abstract   Removes the ODQueryRef from the provided runloop
    @discussion Removes the ODQueryRef from the provided runloop
    @param      inQuery an ODQueryRef to remove from the runloop
    @param      inRunLoop a CFRunLoopRef to remove the ODQuery source from
    @param      inRunLoopMode a CFStringRef of the mode to remove the ODQuery from
*/
CF_EXPORT
void ODQueryUnscheduleFromRunLoop( ODQueryRef inQuery, CFRunLoopRef inRunLoop, CFStringRef inRunLoopMode );

/*!
    @function   ODQuerySetDispatchQueue
    @abstract   Performs the query and sends the results using the specified dispatch queue
    @discussion Schedule the query to run and deliver its results using the specified dispatch queue.
                The previously set callback will be called using the same semantics as
                ODQueryScheduleWithRunLoop
    @param      inQuery an ODQueryRef to perform
    @param      inQueue a dispatch queue to receive the query results
*/
CF_EXPORT
void ODQuerySetDispatchQueue( ODQueryRef inQuery, dispatch_queue_t inQueue );

/* private calls */
void ODQueryCancel(ODQueryRef inQueryRef);
bool ODQueryCancelled(ODQueryRef inQueryRef);
bool ODQueryRestarted(ODQueryRef inQueryRef);

void ODQuerySetRequestID(ODQueryRef inQueryRef, uint64_t reqid);
uint64_t ODQueryGetRequestID(ODQueryRef inQueryRef);

dispatch_queue_t ODQueryGetDispatchQueue(ODQueryRef inQueryRef);

CFDataRef ODQueryGetQueryID(ODQueryRef inQueryRef);
void ODQuerySetQueryID(ODQueryRef inQueryRef, CFDataRef queryID);

mach_port_t	ODQueryGetReplyPort(ODQueryRef inQueryRef);
void ODQuerySetReplyPort(ODQueryRef inQueryRef, mach_port_t replyPort);

void ODQuerySetPID(ODQueryRef inQueryRef, pid_t pid);
pid_t ODQueryGetPID(ODQueryRef inQueryRef);

void ODQuerySetExternalSession(ODQueryRef inQueryRef, bool externalSession);
bool ODQueryIsExternalSession(ODQueryRef inQueryRef);

#pragma mark -
#pragma mark ODSession object functions

/*!
    @function	ODSessionGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for ODSession objects
    @result     a valid CFTypeID for the ODSession object
*/
CF_EXPORT
CFTypeID ODSessionGetTypeID( void );

/*!
    @function   ODSessionCreate
    @abstract   Creates an ODSession object to be passed to ODNode functions
    @discussion Creates an ODSession object to be passed to ODNode functions.
    @param      inAllocator a memory allocator to use for this object
    @param      inOptions a CFDictionary of options associated with this ODSession.  This is typically NULL
                unless caller needs to proxy to another host.  
 
                If proxy is required then a dictionary with keys should be:
                            Key                             Value
                    kODSessionProxyAddress        CFString(hostname or IP)
                    kODSessionProxyPort           CFNumber(IP port, should not be set as it will default)
                    kODSessionProxyUsername       CFString(username)
                    kODSessionProxyPassword       CFString(password)
    @param      outError an optional CFErrorRef reference for error details
    @result     a valid ODSessionRef object or NULL if it cannot be created. Pass reference to CFErrorRef to
                get error details
*/
CF_EXPORT
ODSessionRef ODSessionCreate( CFAllocatorRef inAllocator, CFDictionaryRef inOptions, CFErrorRef *outError );

/*!
    @function   ODSessionCopyNodeNames
    @abstract   Returns the node names that are registered on this ODSession
    @discussion Returns the node names that are registered on this ODSession
    @param      inAllocator a memory allocator to use for this object
    @param      inSession an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      outError an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of node names that can be opened on the session reference
*/
CF_EXPORT
CFArrayRef ODSessionCopyNodeNames( CFAllocatorRef inAllocator, ODSessionRef inSession, CFErrorRef *outError );

#pragma mark -
#pragma mark ODNode object function calls

/*!
    @function   ODNodeGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODNode objects
    @result     a valid CFTypeID for the ODNode object
*/
CF_EXPORT
CFTypeID ODNodeGetTypeID( void );

/*!
    @function   ODNodeCreateWithNodeType
    @abstract   Creates an ODNodeRef based on a specific node type
    @discussion Creates an ODNodeRef based on a specific node type
    @param      inAllocator a memory allocator to use for this object
    @param      inSession an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      inType an ODNodeType of the node to open
    @param      outError an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL.  outError can be checked for details upon
                failure.
*/
CF_EXPORT
ODNodeRef ODNodeCreateWithNodeType( CFAllocatorRef inAllocator, ODSessionRef inSession, ODNodeType inType, 
                                    CFErrorRef *outError );

/*!
    @function   ODNodeCreateWithName
    @abstract   Creates an ODNodeRef based on a partciular node name
    @discussion Creates an ODNodeRef based on a particular node name
    @param      inAllocator a memory allocator to use for this object
    @param      inSession an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      inNodeName a CFStringRef of the name of the node to open
    @param      outError an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL. outError can be checked for specific
                error
*/
CF_EXPORT
ODNodeRef ODNodeCreateWithName( CFAllocatorRef inAllocator, ODSessionRef inSession, CFStringRef inNodeName, 
                                CFErrorRef *outError );

/*!
    @function   ODNodeCreateCopy
    @abstract   Creates a copy, including any remote credentials used for Proxy and/or Node authentication
    @discussion Creates a copy of the object including all credentials used for the original.  Can be used for future
                references to the same node setup.
    @param      inAllocator a memory allocator to use for this object
    @param      inNode an ODNodeRef to make a copy of
    @param      outError an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL, with outError set to a CFErrorRef
*/
CF_EXPORT
ODNodeRef ODNodeCreateCopy( CFAllocatorRef inAllocator, ODNodeRef inNode, CFErrorRef *outError );

/*!
    @function   ODNodeCopySubnodeNames
    @abstract   Returns a CFArray of subnode names for this node, which may contain sub-nodes or search policy nodes
    @discussion Returns a CFArray of subnode names for this node, which may contain sub-nodes or search policy nodes.
                Commonly used with Search policy nodes.
    @param      inNode an ODNodeRef to use
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFArrayRef with the list of nodes, otherwise NULL, with outError set to a CFErrorRef
*/
CF_EXPORT
CFArrayRef ODNodeCopySubnodeNames( ODNodeRef inNode, CFErrorRef *outError );

/*!
    @function   ODNodeCopyUnreachableSubnodeNames
    @abstract   Will return names of subnodes that are not currently reachable.
    @discussion Will return names of subnodes that are not currently reachable.  Commonly used with Search policy nodes
                to determine if any nodes are currently unreachable, but may also return other subnodes if the
                Open Directory plugin supports.
    @param      inNode an ODNodeRef to use
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFArrayRef with the list of unreachable nodes or NULL if no bad nodes
*/
CF_EXPORT
CFArrayRef ODNodeCopyUnreachableSubnodeNames( ODNodeRef inNode, CFErrorRef *outError );

/*!
    @function   ODNodeGetName
    @abstract   Returns the node name of the node that was opened
    @discussion Returns the node name of the node that was opened
    @param      inNode an ODNodeRef to use
    @result     a CFStringRef of the node name that is current or NULL if no open node
*/
CF_EXPORT
CFStringRef ODNodeGetName( ODNodeRef inNode );

/*!
    @function   ODNodeCopyDetails
    @abstract   Returns a dictionary with details about the node in dictionary form
    @discussion Returns a dictionary with details about the node in dictionary form.
    @param      inNode an ODNodeRef to use
    @param      inKeys a CFArrayRef listing the keys the user wants returned, such as 
                kODAttributeTypeStreet
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing the requested key and values in form of a CFArray
*/
CF_EXPORT
CFDictionaryRef ODNodeCopyDetails( ODNodeRef inNode, CFArrayRef inKeys, CFErrorRef *outError );

/*!
    @function   ODNodeCopySupportedRecordTypes
    @abstract   Returns a CFArrayRef of the record types supported by this node.
    @discussion Returns a CFArrayRef of the record types supported by this node.  If node does not support the check
                then all possible types will be returned.
    @param      inNode an ODNodeRef to use
    @param      outError an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of CFStrings listing the supported Record types on this node.
*/
CF_EXPORT
CFArrayRef ODNodeCopySupportedRecordTypes( ODNodeRef inNode, CFErrorRef *outError );

/*!
    @function   ODNodeCopySupportedAttributes
    @abstract   Will return a list of attribute types supported for that attribute if possible
    @discussion Will return a list of attribute types supported for that attribute if possible.  If no specific
                types are available, then all possible values will be returned instead.
    @param      inNode an ODNodeRef to use
    @param      inRecordType a ODRecordTypeRef with the type of record to check attribute types.  If NULL is passed it will
                return all possible attributes that are available.
    @param      outError an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of CFStrings listing the attributes supported for the requested record type
*/
CF_EXPORT
CFArrayRef ODNodeCopySupportedAttributes( ODNodeRef inNode, ODRecordType inRecordType, CFErrorRef *outError );

/*!
    @function   ODNodeSetCredentials
    @abstract   Sets the credentials for interaction with the ODNode
    @discussion Sets the credentials for interaction with the ODNode.  Record references, etc. will use these credentials
                to query or change data.  Setting the credentials on a node referenced by other OD object types will
                change the credentials for all for all references.
    @param      inNode an ODNodeRef to use
    @param      inRecordType a ODRecordTypeRef of the Record Type to use, if NULL is passed, defaults to a 
                kODRecordTypeUsers
    @param      inRecordName a CFString of the username to be used for this node authentication
    @param      inPassword a CFString of the password to be used for this node authentication
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details.  If the authentication failed, 
                the previous credentials are used.
*/
CF_EXPORT
bool ODNodeSetCredentials( ODNodeRef inNode, ODRecordType inRecordType, CFStringRef inRecordName, 
						   CFStringRef inPassword, CFErrorRef *outError );

/*!
    @function   ODNodeSetCredentialsExtended
    @abstract   Allows use of other Open Directory types of authentications to set the credentials for an ODNode
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.
    @param      inNode an ODNodeRef to use
    @param      inRecordType a ODRecordType of the type of record to do the authentication with
    @param      inAuthType a ODAuthenticationType of the type of authentication to be used (e.g., kDSStdAuthNTLMv2)
    @param      inAuthItems a CFArray of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems will be assigned to a pointer of a CFArray of CFData items if there are returned values
    @param      outContext will return a pointer to a context if caller supplies a container, and the call requires a
                context.  If a non-NULL value is returned, then more calls must be made with the Context to continue
                the authorization.
    @param      outError an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.
*/
CF_EXPORT
bool ODNodeSetCredentialsExtended( ODNodeRef inNode, ODRecordType inRecordType, ODAuthenticationType inAuthType, 
								   CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
								   CFErrorRef *outError );

CF_EXPORT
bool ODNodeVerifyCredentialsExtended(ODNodeRef inNode, ODRecordType inRecordType, ODAuthenticationType inAuthType, 
									 CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
									 CFErrorRef *outError);

/*!
    @function   ODNodeSetCredentialsUsingKerberosCache
    @abstract   Sets the credentials of the node using the current kerberos cache
    @discussion Sets the credentials of the node using the current kerberos cache.  Will not interact with the user.
                If no cache name is supplied, it defaults to the currently active Kerberos cache.  If the existing
                cache will not allow use, it will return false and an error.
    @param      inNode an ODNodeRef to use
    @param      inCacheName an optional CFStringRef of the cache name to use for tickets.  NULL can be passed if the 
                currently active cache is to be used.
    @param      outError an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation
*/
CF_EXPORT
bool ODNodeSetCredentialsUsingKerberosCache( ODNodeRef inNode, CFStringRef inCacheName, CFErrorRef *outError );

/*!
    @function   ODNodeCreateRecord
    @abstract   Takes a record and all of the provided attributes and creates the record in the node
    @discussion Takes all the provided attributes and type to create an entire record.  The function will assign a
                UUID to the record automatically.  This UUID can be overwritten by the client by passing with the
                other attributes.
    @param      inNode an ODNodeRef to use
    @param      inRecordType a ODRecordTypeRef of the type of record (e.g., kODRecordTypeUsers, etc.)
    @param      inRecordName a CFStringRef of the name of record
    @param      inAttributes a CFDictionaryRef of key-value pairs for attribute values.  The key is a CFStringRef of the
                attribute name or ODRecordType constant such as kODAttributeTypeRecordName.  The value must be a CFArrayRef of
                CFDataRef or CFStringRef.  If additional kODAttributeTypeRecordName are to be set, they can be passed in the 
                inAttributes list.  This parameter is optional and can be NULL.  If any of the attributes passed
                fail to be set, the record will be deleted and outError will be set with the appropriate error.
                If a password is not supplied with a user account, then a random password will be set automatically.  If
                an empty password is expected, then the kODAttributeTypePassword should be set to an empty CFArray.
    @param      outError an optional CFErrorRef reference for error details
    @result     returns a valid ODRecordRef.  If the add fails, outError can be checked for details.
*/
CF_EXPORT
ODRecordRef ODNodeCreateRecord( ODNodeRef inNode, ODRecordType inRecordType, CFStringRef inRecordName,
                                CFDictionaryRef inAttributes, CFErrorRef *outError );

/*!
    @function   ODNodeCopyRecord
    @abstract   Simple API to open / create a references to a particular record on a Node
    @discussion Simple API to open / create a references to a particular record on a Node
    @param      inNode an ODNodeRef to use
    @param      inRecordType a ODRecordTypeRef of the record type to copy
    @param      inRecordName a CFStringRef of the record name to copy
    @param      inAttributes (optional) a CFArrayRef of the attributes to copy from the directory.  Can be NULL when no 
                attributes are needed.  Any DS standard types can be passed, for example passing a CFArray with
                kODAttributeTypeAllAttributes will fetch all attributes up front.  If just standard attributes are needed, then
                kODAttributeTypeStandardOnly can be put into a CFArray and passed.
    @param      outError an optional CFErrorRef reference for error details
    @result     returns a valid ODRecordRef.  If the record copy fails, outError can be checked for details.
*/
CF_EXPORT
ODRecordRef ODNodeCopyRecord( ODNodeRef inNode, ODRecordType inRecordType, CFStringRef inRecordName, 
                              CFArrayRef inAttributes, CFErrorRef *outError );

/*!
    @function   ODNodeCustomCall
    @abstract   This will send a custom call to a node along with the inSendData and response in outResponseData
    @discussion This will send a custom call to a node along with the inSendData and response in outResponseData.
                The outResponseData should be a CFMutableDataRef with a data object set to the size of the expected
                response.  If the responseData is not large enough for the custom call, unexpected results may occurr.
    @param      inNode an ODNodeRef to use
    @param      inCustomCode the custom code to be sent to the node
    @param      inSendData a data blob expected by the custom code, can be NULL of no send data
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFDataRef with the result of the operation, otherwise outError can be checked for specific details
*/
CF_EXPORT
CFDataRef ODNodeCustomCall( ODNodeRef inNode, CFIndex inCustomCode, CFDataRef inSendData, CFErrorRef *outError );

#pragma mark -
#pragma mark ODRecord object function calls

/*!
    @function   ODRecordGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODRecord object
    @result     a valid CFTypeID for the ODRecord object
*/
CF_EXPORT
CFTypeID ODRecordGetTypeID( void );

/*!
    @function   ODRecordSetNodeCredentials
    @abstract   Similar to calling ODNodeSetCredentials except credentials are only set for this particular record's node
    @discussion Sets the credentials if necessary on the ODNodeRef referenced by this ODRecordRef.  Very similar to
                calling ODNodeSetCredentials except other records referencing the underlying ODNodeRef will not get
                authenticated, therefore inadvertant changes cannot occur.  If all records referencing a particular 
                ODNodeRef need to be updated, then use ODNodeSetCredentials on the original ODNodeRef instead.  If the
                ODNodeRef is already authenticated with the same name and password, this will be a NOOP call.  The original
                ODNodeRef held by an ODRecordRef will be released and a new ODNodeRef will be created when the credentials
                are set for this ODRecordRef.  Calling this on multiple records could result in multiple References into the 
                DirectoryService daemon, which could cause errors logged into /var/log/system.log if a threshold is reached.
    @param      inRecord an ODRecordRef to use
    @param      inUsername a CFStringRef of the username used to authenticate
    @param      inPassword a CFStringRef of the password used to authenticate
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details.  Upon failure the original node
                will still be intact.
*/
CF_EXPORT
bool ODRecordSetNodeCredentials( ODRecordRef inRecord, CFStringRef inUsername, CFStringRef inPassword, CFErrorRef *outError );

/*!
    @function   ODRecordSetNodeCredentialsExtended
    @abstract   Similar to calling ODNodeSetCredentialsExtended except credentials are only set for this particular record's
                node
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.  Same behavior as ODRecordSetNodeCredentials.
    @param      inRecord an ODRecordRef to use
    @param      inRecordType a ODRecordTypeRef of the type of record to do the authentication with
    @param      inAuthType a ODAuthenticationTypeRef of the type of authentication to be used (e.g., kDSStdAuthNTLMv2)
    @param      inAuthItems a CFArrayRef of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems a pointer to CFArrayRef that will be assigned to a CFArrayRef of CFData items if the
                call returned any values followup values
    @param      outContext a pointer to ODContextRef if the call requires further calls for response-request auths.
    @param      outError an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.  Upon failure the original node will still be intact.
*/
CF_EXPORT
bool ODRecordSetNodeCredentialsExtended( ODRecordRef inRecord, ODRecordType inRecordType, ODAuthenticationType inAuthType, 
										 CFArrayRef inAuthItems, CFArrayRef *outAuthItems, ODContextRef *outContext,
										 CFErrorRef *outError );

/*!
    @function   ODRecordSetNodeCredentialsUsingKerberosCache
    @abstract   Sets the credentials of the node using the current kerberos cache
    @discussion Sets the credentials of the node using the current kerberos cache.  Will not interact with the user.
                If no cache name is supplied, it defaults to the currently active Kerberos cache.  If the existing
                cache will not allow use, it will return false and an error.
    @param      inRecord an ODRecordRef to use
    @param      inCacheName an optional CFStringRef of the cache name to use for tickets.  NULL can be passed if the 
                currently active cache is to be used.
    @param      outError an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation
*/
CF_EXPORT
bool ODRecordSetNodeCredentialsUsingKerberosCache( ODRecordRef inRecord, CFStringRef inCacheName, CFErrorRef *outError );

/*!
    @function   ODRecordCopyPasswordPolicy
    @abstract   Returns a CFDictionaryRef of the effective policy for the user if available
    @discussion Returns a CFDictionaryRef of the effective policy for the user if available
    @param      inAllocator a CFAllocatorRef to use
    @param      inRecord an ODRecordRef to use
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef of the password policies for the supplied record, or NULL if no policy set
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyPasswordPolicy( CFAllocatorRef inAllocator, ODRecordRef inRecord, CFErrorRef *outError );

/*!
    @function   ODRecordVerifyPassword
    @abstract   Verifies the password provided is valid for the record
    @discussion Verifies the password provided is valid for the record.
    @param      inRecord an ODRecordRef to use
    @param      inPassword a CFStringRef of the password that is being verified
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordVerifyPassword( ODRecordRef inRecord, CFStringRef inPassword, CFErrorRef *outError );

/*!
    @function   ODRecordVerifyPasswordExtended
    @abstract   Allows use of other Open Directory types of authentications to verify a record password
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may 
                require response-request loops, etc.
    @param      inRecord an ODRecordRef to use
    @param      inAuthType a ODAuthenticationTypeRef of the type of authentication to be used (e.g., kODAuthenticationTypeCRAM_MD5)
    @param      inAuthItems a CFArrayRef of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems a pointer to CFArrayRef that will be assigned to a CFArrayRef of CFData items if the
                call returned any values followup values
    @param      outContext a pointer to ODContextRef if the call requires further calls for response-request auths.
    @param      outError an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.  Some ODNodes may not support the call so an error of
                eNotHandledByThisNode or eNotYetImplemented may be returned.
*/
CF_EXPORT
bool ODRecordVerifyPasswordExtended( ODRecordRef inRecord, ODAuthenticationType inAuthType, CFArrayRef inAuthItems, 
									 CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *outError );

/*!
    @function   ODRecordChangePassword
    @abstract   Changes the password of an ODRecord
    @discussion Changes the password of an ODRecord.  If NULL is passed into inOldPassword, then an attempt to set
                the password will be tried.  If changing a password, then both old and new passwords should be supplied.
    @param      inRecord an ODRecordRef to use
    @param      inOldPassword a CFString of the record's old password (NULL is optional).
    @param      inNewPassword a CFString of the record's new password
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordChangePassword( ODRecordRef inRecord, CFStringRef inOldPassword, CFStringRef inNewPassword, 
							 CFErrorRef *outError );

/*!
    @function   ODRecordGetRecordType
    @abstract   Returns the record type of an ODRecordRef
    @discussion Returns the record type of an ODRecordRef
    @param      inRecord an ODRecordRef to use
    @result     a CFStringRef of the record type for this ODRecordRef
*/
CF_EXPORT
CFStringRef ODRecordGetRecordType( ODRecordRef inRecord );

/*!
    @function   ODRecordGetRecordName
    @abstract   Returns the official record name of an ODRecordRef
    @discussion Returns the official record name of an ODRecordRef which typically corresponds to the first value
                of the kODAttributeTypeRecordName attribute, but not always.  This name should be a valid name in either case.
    @param      inRecord an ODRecordRef to use
    @result     a CFStringRef of the record name for this ODRecordRef
*/
CF_EXPORT
CFStringRef ODRecordGetRecordName( ODRecordRef inRecord );

/*!
    @function   ODRecordCopyValues
    @abstract   Returns the value of an attribute as a CFArrayRef of CFStringRef or CFDataRef types
    @discussion Returns the value of an attribute as a CFArrayRef of CFStringRef or CFDataRef, depending on
                whether the data is Binary or not.  If the value has been fetched from the directory previously
                a copy of the internal storage will be returned without going to the directory.  If it has not been fetched
                previously, then it will be fetched at that time.
    @param      inRecord an ODRecordRef to use
    @param      inAttribute a CFStringRef or ODAttributeType of the attribute (e.g., kODAttributeTypeRecordName, etc.)
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFArrayRef of the attribute requested if possible, or NULL if the attribute doesn't exist
*/
CF_EXPORT
CFArrayRef ODRecordCopyValues( ODRecordRef inRecord, ODAttributeType inAttribute, CFErrorRef *outError );

/*!
    @function   ODRecordSetValue
    @abstract   Will take a CFDataRef or CFStringRef or a CFArrayRef of either type and set it for the attribute
    @discussion Will take a CFDataRef or CFStringRef or a CFArrayRef of either type and set it for the attribute.
                Any mixture of the types CFData and CFString are accepted.
    @param      inRecord an ODRecordRef to use
    @param      inAttribute a CFStringRef of the attribute for values to be added too
    @param      inValueOrValues a CFArrayRef of CFStringRef or CFDataRef types or either of the individual types, passing
                an empty CFArray deletes the attribute.  The underlying implementation will do this in the most efficient manner,
                either by adding only new values or completely replacing the values depending on the capabilities of the
                particular plugin.
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordSetValue( ODRecordRef inRecord, ODAttributeType inAttribute, CFTypeRef inValueOrValues, CFErrorRef *outError );

/*!
    @function   ODRecordAddValue
    @abstract   Adds a value to an attribute
    @discussion Adds a value to an attribute.
    @param      inRecord an ODRecordRef to use
    @param      inAttribute a CFStringRef of the attribute for values to be added too
    @param      inValue a CFTypeRef of the value to be added to the attribute, either CFStringRef or CFDataRef
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordAddValue( ODRecordRef inRecord, ODAttributeType inAttribute, CFTypeRef inValue, CFErrorRef *outError );

/*!
    @function   ODRecordRemoveValue
    @abstract   Removes a particular value from an attribute.
    @discussion Removes a particular value from an attribute.
    @param      inRecord an ODRecordRef to use
    @param      inAttribute a CFStringRef of the attribute to remove the value from
    @param      inValue a CFTypeRef of the value to be removed from the attribute.  Either CFStringRef or CFDataRef.
                If the value does not exist, true will be returned and no error will be set.
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordRemoveValue( ODRecordRef inRecord, ODAttributeType inAttribute, CFTypeRef inValue, CFErrorRef *outError );

/*!
    @function   ODRecordCopyDetails
    @abstract   Returns the attributes and values in the form of a key-value pair set for this record.
    @discussion Returns the attributes and values in the form of a key-value pair set for this record.  The key is a 
                CFStringRef or ODAttributeType of the attribute name (e.g., kODAttributeTypeRecordName, etc.) and the 
				value is an CFArrayRef of either CFDataRef or CFStringRef depending on the type of data.  Binary data will 
				be returned as CFDataRef.
    @param      inRecord an ODRecordRef to use
    @param      inAttributes a CFArrayRef of attributes.  If an attribute has not been fetched previously, it will be
                fetched in order to return the value.  If this parameter is NULL then all currently fetched attributes 
                will be returned.
    @param      outError an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef of the attributes for the record
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyDetails( ODRecordRef inRecord, CFArrayRef inAttributes, CFErrorRef *outError );

/*!
    @function   ODRecordSynchronize
    @abstract   Synchronizes the record from the Directory in order to get current data and commit pending changes
    @discussion Synchronizes the record from the Directory in order to get current data.  Any previously fetched attributes
                will be refetched from the Directory.  This will not refetch the entire record, unless the entire record
                has been accessed.  Additionally, any changes made to the record will be committed to the directory
                if the node does not do immediate commits.
    @param      inRecord an ODRecordRef to use
    @param      outError an optional CFErrorRef reference for error details
*/
CF_EXPORT
bool ODRecordSynchronize( ODRecordRef inRecord, CFErrorRef *outError );

/*!
    @function   ODRecordDelete
    @abstract   Deletes the record from the node and invalidates the record.
    @discussion Deletes the recrod from the node and invalidates the record.  The ODRecordRef should be
                CFReleased after deletion.
    @param      inRecord an ODRecordRef to use
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordDelete( ODRecordRef inRecord, CFErrorRef *outError );

/*!
    @function   ODRecordAddMember
    @abstract   Will add the record as a member of the group record that is provided
    @discussion Will add the record as a member of the group record that is provided in an appopriate manner 
                based on what the directory will store.  An error will be returned if the record is not a group record.  
                Additionally, if the member record is not an appropriate type allowed as part of a group an
                error will be returned.
    @param      inGroup an ODRecordRef of the group record to modify
    @param      inMember an ODRecordRef of the record to add to the group record
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordAddMember( ODRecordRef inGroup, ODRecordRef inMember, CFErrorRef *outError );

/*!
    @function   ODRecordRemoveMember
    @abstract   Will remove the record as a member from the group record that is provided
    @discussion Will remove the record as a member from the group record that is provided.  If the record type
                of tine inGroup is not a Group false will be returned with an appropriate error.
    @param      inGroup an ODRecordRef of the group record to modify
    @param      inMember an ODRecordRef of the record to remove from the group record
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordRemoveMember( ODRecordRef inGroup, ODRecordRef inMember, CFErrorRef *outError );

/*!
    @function   ODRecordContainsMember
    @abstract   Will use membership APIs to resolve group membership based on Group and Member record combination
    @discussion Will use membership APIs to resolve group membership based on Group and Member record combination.
				This API does not check attributes values directly, instead uses system APIs to deal with nested
				memberships.
    @param      inGroup an ODRecordRef of the group to be checked for membership
    @param      inMember an ODRecordRef of the member to be checked against the group
    @param      outError an optional CFErrorRef reference for error details
    @result     returns true or false depending on result
*/
CF_EXPORT
bool ODRecordContainsMember( ODRecordRef inGroup, ODRecordRef inMember, CFErrorRef *outError );

__END_DECLS

#endif /* CFOpenDirectory */
