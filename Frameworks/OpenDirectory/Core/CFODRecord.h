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

#if !defined(__OPENDIRECTORY_CFODRECORD__)
#define __OPENDIRECTORY_CFODRECORD__

#include <CFOpenDirectory/CFOpenDirectory.h>

__BEGIN_DECLS

/*!
    @function   ODRecordGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODRecord object
    @result     a valid CFTypeID for the ODRecord object
*/
CF_EXPORT
CFTypeID ODRecordGetTypeID(void) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

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
                OpenDirectory daemon, which could cause errors logged into /var/log/system.log if a threshold is reached.
    @param      record an ODRecordRef to use
    @param      username a CFStringRef of the username used to authenticate
    @param      password a CFStringRef of the password used to authenticate
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details.  Upon failure the original node
                will still be intact.
*/
CF_EXPORT
bool ODRecordSetNodeCredentials(ODRecordRef record, CFStringRef username, CFStringRef password, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSetNodeCredentialsExtended
    @abstract   Similar to calling ODNodeSetCredentialsExtended except credentials are only set for this particular record's
                node
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.  Same behavior as ODRecordSetNodeCredentials.
    @param      record an ODRecordRef to use
    @param      recordType a ODRecordTypeRef of the type of record to do the authentication with
    @param      authType a ODAuthenticationTypeRef of the type of authentication to be used (e.g., kDSStdAuthNTLMv2)
    @param      authItems a CFArrayRef of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems a pointer to CFArrayRef that will be assigned to a CFArrayRef of CFData items if the
                call returned any values followup values
    @param      outContext a pointer to ODContextRef if the call requires further calls for response-request auths.
    @param      error an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.  Upon failure the original node will still be intact.
*/
CF_EXPORT
bool ODRecordSetNodeCredentialsExtended(ODRecordRef record, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSetNodeCredentialsUsingKerberosCache
    @abstract   Unsupported function.
    @discussion Unsupported function.
*/
CF_EXPORT
bool ODRecordSetNodeCredentialsUsingKerberosCache(ODRecordRef record, CFStringRef cacheName, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_6, __MAC_10_7, __IPHONE_NA, __IPHONE_NA);

/*!
    @function   ODRecordCopyPasswordPolicy
    @abstract   Returns a CFDictionaryRef of the effective policy for the user if available
    @discussion Returns a CFDictionaryRef of the effective policy for the user if available
    @param      allocator a CFAllocatorRef to use
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef of the password policies for the supplied record, or NULL if no policy set
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyPasswordPolicy(CFAllocatorRef allocator, ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordVerifyPassword
    @abstract   Verifies the password provided is valid for the record
    @discussion Verifies the password provided is valid for the record.
    @param      record an ODRecordRef to use
    @param      password a CFStringRef of the password that is being verified
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordVerifyPassword(ODRecordRef record, CFStringRef password, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordVerifyPasswordExtended
    @abstract   Allows use of other Open Directory types of authentications to verify a record password
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may 
                require response-request loops, etc.
    @param      record an ODRecordRef to use
    @param      authType a ODAuthenticationTypeRef of the type of authentication to be used (e.g., kODAuthenticationTypeCRAM_MD5)
    @param      authItems a CFArrayRef of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems a pointer to CFArrayRef that will be assigned to a CFArrayRef of CFData items if the
                call returned any values followup values
    @param      outContext a pointer to ODContextRef if the call requires further calls for response-request auths.
    @param      error an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.  Some ODNodes may not support the call so an error of
                eNotHandledByThisNode or eNotYetImplemented may be returned.
*/
CF_EXPORT
bool ODRecordVerifyPasswordExtended(ODRecordRef record, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordChangePassword
    @abstract   Changes the password of an ODRecord
    @discussion Changes the password of an ODRecord.  If NULL is passed into inOldPassword, then an attempt to set
                the password will be tried.  If changing a password, then both old and new passwords should be supplied.
    @param      record an ODRecordRef to use
    @param      oldPassword a CFString of the record's old password (NULL is optional).
    @param      newPassword a CFString of the record's new password
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordChangePassword(ODRecordRef record, CFStringRef oldPassword, CFStringRef newPassword, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordGetRecordType
    @abstract   Returns the record type of an ODRecordRef
    @discussion Returns the record type of an ODRecordRef
    @param      record an ODRecordRef to use
    @result     a CFStringRef of the record type for this ODRecordRef
*/
CF_EXPORT
CFStringRef ODRecordGetRecordType(ODRecordRef record) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordGetRecordName
    @abstract   Returns the official record name of an ODRecordRef
    @discussion Returns the official record name of an ODRecordRef which typically corresponds to the first value
                of the kODAttributeTypeRecordName attribute, but not always.  This name should be a valid name in either case.
    @param      record an ODRecordRef to use
    @result     a CFStringRef of the record name for this ODRecordRef
*/
CF_EXPORT
CFStringRef ODRecordGetRecordName(ODRecordRef record) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordCopyValues
    @abstract   Returns the value of an attribute as a CFArrayRef of CFStringRef or CFDataRef types
    @discussion Returns the value of an attribute as a CFArrayRef of CFStringRef or CFDataRef, depending on
                whether the data is Binary or not.  If the value has been fetched from the directory previously
                a copy of the internal storage will be returned without going to the directory.  If it has not been fetched
                previously, then it will be fetched at that time.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef or ODAttributeType of the attribute (e.g., kODAttributeTypeRecordName, etc.)
    @param      error an optional CFErrorRef reference for error details
    @result     a CFArrayRef of the attribute requested if possible, or NULL if the attribute doesn't exist
*/
CF_EXPORT
CFArrayRef ODRecordCopyValues(ODRecordRef record, ODAttributeType attribute, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSetValue
    @abstract   Will take a CFDataRef or CFStringRef or a CFArrayRef of either type and set it for the attribute
    @discussion Will take a CFDataRef or CFStringRef or a CFArrayRef of either type and set it for the attribute.
                Any mixture of the types CFData and CFString are accepted.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef of the attribute for values to be added too
    @param      valueOrValues a CFArrayRef of CFStringRef or CFDataRef types or either of the individual types, passing
                an empty CFArray deletes the attribute.  The underlying implementation will do this in the most efficient manner,
                either by adding only new values or completely replacing the values depending on the capabilities of the
                particular plugin.
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordSetValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef valueOrValues, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordAddValue
    @abstract   Adds a value to an attribute
    @discussion Adds a value to an attribute.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef of the attribute for values to be added too
    @param      value a CFTypeRef of the value to be added to the attribute, either CFStringRef or CFDataRef
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordAddValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordRemoveValue
    @abstract   Removes a particular value from an attribute.
    @discussion Removes a particular value from an attribute.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef of the attribute to remove the value from
    @param      value a CFTypeRef of the value to be removed from the attribute.  Either CFStringRef or CFDataRef.
                If the value does not exist, true will be returned and no error will be set.
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordRemoveValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordCopyDetails
    @abstract   Returns the attributes and values in the form of a key-value pair set for this record.
    @discussion Returns the attributes and values in the form of a key-value pair set for this record.  The key is a 
                CFStringRef or ODAttributeType of the attribute name (e.g., kODAttributeTypeRecordName, etc.) and the 
				value is an CFArrayRef of either CFDataRef or CFStringRef depending on the type of data.  Binary data will 
				be returned as CFDataRef.
    @param      record an ODRecordRef to use
    @param      attributes a CFArrayRef of attributes.  If an attribute has not been fetched previously, it will be
                fetched in order to return the value.  If this parameter is NULL then all currently fetched attributes 
                will be returned.
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef of the attributes for the record
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyDetails(ODRecordRef record, CFArrayRef attributes, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSynchronize
    @abstract   Synchronizes the record from the Directory in order to get current data and commit pending changes
    @discussion Synchronizes the record from the Directory in order to get current data.  Any previously fetched attributes
                will be refetched from the Directory.  This will not refetch the entire record, unless the entire record
                has been accessed.  Additionally, any changes made to the record will be committed to the directory
                if the node does not do immediate commits.
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
*/
CF_EXPORT
bool ODRecordSynchronize(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordDelete
    @abstract   Deletes the record from the node and invalidates the record.
    @discussion Deletes the record from the node and invalidates the record.  The ODRecordRef should be
                released after deletion.
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordDelete(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordAddMember
    @abstract   Will add the record as a member of the group record that is provided
    @discussion Will add the record as a member of the group record that is provided in an appopriate manner 
                based on what the directory will store.  An error will be returned if the record is not a group record.  
                Additionally, if the member record is not an appropriate type allowed as part of a group an
                error will be returned.
    @param      group an ODRecordRef of the group record to modify
    @param      member an ODRecordRef of the record to add to the group record
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordAddMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordRemoveMember
    @abstract   Will remove the record as a member from the group record that is provided
    @discussion Will remove the record as a member from the group record that is provided.  If the record type
                of group is not a Group, false will be returned with an appropriate error.
    @param      group an ODRecordRef of the group record to modify
    @param      member an ODRecordRef of the record to remove from the group record
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordRemoveMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordContainsMember
    @abstract   Will use membership APIs to resolve group membership based on Group and Member record combination
    @discussion Will use membership APIs to resolve group membership based on Group and Member record combination.
				This API does not check attributes values directly, instead uses system APIs to deal with nested
				memberships.
    @param      group an ODRecordRef of the group to be checked for membership
    @param      member an ODRecordRef of the member to be checked against the group
    @param      error an optional CFErrorRef reference for error details
    @result     returns true or false depending on result
*/
CF_EXPORT
bool ODRecordContainsMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

__END_DECLS

#endif /* ! __OPENDIRECTORY_CFODRECORD__ */
