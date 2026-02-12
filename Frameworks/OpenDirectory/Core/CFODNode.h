/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#if !defined(__OPENDIRECTORY_CFODNODE__)
#define __OPENDIRECTORY_CFODNODE__ 1

#include <CFOpenDirectory/CFOpenDirectory.h>

__BEGIN_DECLS

/*!
    @function   ODNodeGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODNode objects
    @result     a valid CFTypeID for the ODNode object
*/
CF_EXPORT
CFTypeID ODNodeGetTypeID(void) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCreateWithNodeType
    @abstract   Creates an ODNodeRef based on a specific node type
    @discussion Creates an ODNodeRef based on a specific node type
    @param      allocator a memory allocator to use for this object
    @param      session an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      nodeType an ODNodeType of the node to open
    @param      error an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL.  outError can be checked for details upon
                failure.
*/
CF_EXPORT
ODNodeRef ODNodeCreateWithNodeType(CFAllocatorRef allocator, ODSessionRef session, ODNodeType nodeType, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCreateWithName
    @abstract   Creates an ODNodeRef based on a partciular node name
    @discussion Creates an ODNodeRef based on a particular node name
    @param      allocator a memory allocator to use for this object
    @param      session an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      nodeName a CFStringRef of the name of the node to open
    @param      error an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL. outError can be checked for specific
                error
*/
CF_EXPORT
ODNodeRef ODNodeCreateWithName(CFAllocatorRef allocator, ODSessionRef session, CFStringRef nodeName, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCreateCopy
    @abstract   Creates a copy, including any remote credentials used for Proxy and/or Node authentication
    @discussion Creates a copy of the object including all credentials used for the original.  Can be used for future
                references to the same node setup.
    @param      allocator a memory allocator to use for this object
    @param      node an ODNodeRef to make a copy of
    @param      error an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL, with outError set to a CFErrorRef
*/
CF_EXPORT
ODNodeRef ODNodeCreateCopy(CFAllocatorRef allocator, ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopySubnodeNames
    @abstract   Returns a CFArray of subnode names for this node, which may contain sub-nodes or search policy nodes
    @discussion Returns a CFArray of subnode names for this node, which may contain sub-nodes or search policy nodes.
                Commonly used with Search policy nodes.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFArrayRef with the list of nodes, otherwise NULL, with outError set to a CFErrorRef
*/
CF_EXPORT
CFArrayRef ODNodeCopySubnodeNames(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopyUnreachableSubnodeNames
    @abstract   Will return names of subnodes that are not currently reachable.
    @discussion Will return names of subnodes that are not currently reachable.  Commonly used with Search policy nodes
                to determine if any nodes are currently unreachable, but may also return other subnodes if the
                Open Directory plugin supports.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFArrayRef with the list of unreachable nodes or NULL if no bad nodes
*/
CF_EXPORT
CFArrayRef ODNodeCopyUnreachableSubnodeNames(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeGetName
    @abstract   Returns the node name of the node that was opened
    @discussion Returns the node name of the node that was opened
    @param      node an ODNodeRef to use
    @result     a CFStringRef of the node name that is current or NULL if no open node
*/
CF_EXPORT
CFStringRef ODNodeGetName(ODNodeRef node) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopyDetails
    @abstract   Returns a dictionary with details about the node in dictionary form
    @discussion Returns a dictionary with details about the node in dictionary form.
    @param      node an ODNodeRef to use
    @param      keys a CFArrayRef listing the keys the user wants returned, such as 
                kODAttributeTypeStreet
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing the requested key and values in form of a CFArray
*/
CF_EXPORT
CFDictionaryRef ODNodeCopyDetails(ODNodeRef node, CFArrayRef keys, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopySupportedRecordTypes
    @abstract   Returns a CFArrayRef of the record types supported by this node.
    @discussion Returns a CFArrayRef of the record types supported by this node.  If node does not support the check
                then all possible types will be returned.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of CFStrings listing the supported Record types on this node.
*/
CF_EXPORT
CFArrayRef ODNodeCopySupportedRecordTypes(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopySupportedAttributes
    @abstract   Will return a list of attribute types supported for that attribute if possible
    @discussion Will return a list of attribute types supported for that attribute if possible.  If no specific
                types are available, then all possible values will be returned instead.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef with the type of record to check attribute types.  If NULL is passed it will
                return all possible attributes that are available.
    @param      error an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of CFStrings listing the attributes supported for the requested record type
*/
CF_EXPORT
CFArrayRef ODNodeCopySupportedAttributes(ODNodeRef node, ODRecordType recordType, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeSetCredentials
    @abstract   Sets the credentials for interaction with the ODNode
    @discussion Sets the credentials for interaction with the ODNode.  Record references, etc. will use these credentials
                to query or change data.  Setting the credentials on a node referenced by other OD object types will
                change the credentials for all for all references.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef of the Record Type to use, if NULL is passed, defaults to a 
                kODRecordTypeUsers
    @param      recordName a CFString of the username to be used for this node authentication
    @param      password a CFString of the password to be used for this node authentication
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details.  If the authentication failed, 
                the previous credentials are used.
*/
CF_EXPORT
bool ODNodeSetCredentials(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFStringRef password, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeSetCredentialsExtended
    @abstract   Allows use of other Open Directory types of authentications to set the credentials for an ODNode
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordType of the type of record to do the authentication with
    @param      authType a ODAuthenticationType of the type of authentication to be used (e.g., kDSStdAuthNTLMv2)
    @param      authItems a CFArray of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems will be assigned to a pointer of a CFArray of CFData items if there are returned values
    @param      outContext will return a pointer to a context if caller supplies a container, and the call requires a
                context.  If a non-NULL value is returned, then more calls must be made with the Context to continue
                the authorization.
    @param      error an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.
*/
CF_EXPORT
bool ODNodeSetCredentialsExtended(ODNodeRef node, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeSetCredentialsUsingKerberosCache
    @abstract   Unsupported function.
    @discussion Unsupported function.
*/
CF_EXPORT
bool ODNodeSetCredentialsUsingKerberosCache(ODNodeRef node, CFStringRef cacheName, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_6, __MAC_10_7, __IPHONE_NA, __IPHONE_NA);

/*!
    @function   ODNodeCreateRecord
    @abstract   Takes a record and all of the provided attributes and creates the record in the node
    @discussion Takes all the provided attributes and type to create an entire record.  The function will assign a
                UUID to the record automatically.  This UUID can be overwritten by the client by passing with the
                other attributes.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef of the type of record (e.g., kODRecordTypeUsers, etc.)
    @param      recordName a CFStringRef of the name of record
    @param      attributeDict a CFDictionaryRef of key-value pairs for attribute values.  The key is a CFStringRef of the
                attribute name or ODRecordType constant such as kODAttributeTypeRecordName.  The value must be a CFArrayRef of
                CFDataRef or CFStringRef.  If additional kODAttributeTypeRecordName are to be set, they can be passed in the 
                inAttributes list.  This parameter is optional and can be NULL.  If any of the attributes passed
                fail to be set, the record will be deleted and outError will be set with the appropriate error.
                If a password is not supplied with a user account, then a random password will be set automatically.  If
                an empty password is expected, then the kODAttributeTypePassword should be set to an empty CFArray.
    @param      error an optional CFErrorRef reference for error details
    @result     returns a valid ODRecordRef.  If the add fails, outError can be checked for details.
*/
CF_EXPORT
ODRecordRef ODNodeCreateRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFDictionaryRef attributeDict, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopyRecord
    @abstract   Simple API to open / create a references to a particular record on a Node
    @discussion Simple API to open / create a references to a particular record on a Node
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef of the record type to copy
    @param      recordName a CFStringRef of the record name to copy
    @param      attributes (optional) a CFArrayRef (or single ODAttributeType) of the attributes to copy from the directory.  Can be NULL when no 
                attributes are needed.  Any standard types can be passed, for example
                kODAttributeTypeAllAttributes will fetch all attributes up front.  If just standard attributes are needed, then
                kODAttributeTypeStandardOnly can be passed.
    @param      error an optional CFErrorRef reference for error details
    @result     returns a valid ODRecordRef.  If the record copy fails, outError can be checked for details.
*/
CF_EXPORT
ODRecordRef ODNodeCopyRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFTypeRef attributes, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCustomCall
    @abstract   This will send a custom call to a node along with the inSendData and response in outResponseData
    @discussion This will send a custom call to a node along with the inSendData and response in outResponseData.
                The outResponseData should be a CFMutableDataRef with a data object set to the size of the expected
                response.  If the responseData is not large enough for the custom call, unexpected results may occurr.
    @param      node an ODNodeRef to use
    @param      customCode the custom code to be sent to the node
    @param      data a data blob expected by the custom code, can be NULL of no send data
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDataRef with the result of the operation, otherwise outError can be checked for specific details
*/
CF_EXPORT
CFDataRef ODNodeCustomCall(ODNodeRef node, CFIndex customCode, CFDataRef data, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

__END_DECLS

#endif /* ! __OPENDIRECTORY_CFODNODE__ */
