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

#import <OpenDirectory/OpenDirectory.h>

/*!
    @class       ODRecord
    @abstract    This class is used to read, update and modify records within the directory
    @discussion  This class is used to read, update and modify records within the directory.  outError is optional parameter,
                 nil can be passed if error details are not needed.
*/
@interface ODRecord : NSObject

/*!
    @method     setNodeCredentials:password:error:
    @abstract   Similar to calling -[ODNode setCredentials:] except credentials are only set for this particular
                record's node
    @discussion Sets the credentials if necessary on the ODNode referenced by this ODRecord.  Very similar to
                calling -[ODNode setCredentials:] except other records referencing the underlying node will not get
                authenticated, therefore inadvertant changes cannot occur.  If all records referencing a particular 
                node need to be updated, then use -[ODNode setCredentials:] on the original node instead.  If the
                node is already authenticated with the same name and password, it will be a NOOP call.  The original
                ODNode held by an ODRecord will be released when the credentials are changed for the connection
                associated with the record.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)setNodeCredentials:(NSString *)inUsername password:(NSString *)inPassword error:(NSError **)outError;

/*!
    @method     setNodeCredentialsWithRecordType:authenticationType:authenticationItems:continueItems:context:error:
    @abstract   Similar to calling -[ODNode setCredentialsWithRecordType:] except credentials are only set for this particular record's
                node
    @discussion Allows the caller to use other types of authentications that are available in OpenDirectory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.  Same behavior as ODRecordSetNodeCredentials.  outError 
				is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)setNodeCredentialsWithRecordType:(ODRecordType)inRecordType authenticationType:(ODAuthenticationType)inType 
                     authenticationItems:(NSArray *)inItems continueItems:(NSArray **)outItems
                                 context:(id *)outContext error:(NSError **)outError;

/*!
    @method     setNodeCredentialsUsingKerberosCache:error:
    @abstract   Unsupported method.
    @discussion Unsupported method.
*/
- (BOOL)setNodeCredentialsUsingKerberosCache:(NSString *)inCacheName error:(NSError **)outError NS_DEPRECATED_MAC(10_6, 10_7);

/*!
    @method     passwordPolicyAndReturnError:
    @abstract   Returns a dictionary containing the password policy for the record if available.
    @discussion Returns a dictionary containing the password policy for the record if available.  If no policy for record
                nil will be returned.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (NSDictionary *)passwordPolicyAndReturnError:(NSError **)outError;

/*!
    @method     verifyPassword:error:
    @abstract   Verifies the password provided is valid for the record
    @discussion Verifies the password provided is valid for the record.  outError is optional parameter, nil can be passed if 
                error details are not needed.
*/
- (BOOL)verifyPassword:(NSString *)inPassword error:(NSError **)outError;

/*!
    @method     verifyExtendedWithAuthenticationType:authenticationItems:continueItems:context:error:
    @abstract   Allows use of other OpenDirectory types of authentications
    @discussion Allows the caller to use other types of authentications that are available in OpenDirectory, that may 
                require response-request loops, etc.  A bool with the result of the operation.  
                If it fails, outError can be checked for more specific error.  Some ODNodes may not support the call
                so an error code of kODErrorCredentialsMethodNotSupported may be returned.  outError is optional 
                parameter, nil can be passed if error details are not needed.
*/
- (BOOL)verifyExtendedWithAuthenticationType:(ODAuthenticationType)inType authenticationItems:(NSArray *)inItems 
                               continueItems:(NSArray **)outItems context:(id *)outContext error:(NSError **)outError;

/*!
    @method     changePassword:toPassword:error:
    @abstract   Changes the password for a record
    @discussion Changes the password for a record.  The oldPassword can be nil if password is being set assuming the appropriate
                privileges are in place.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)changePassword:(NSString *)oldPassword toPassword:(NSString *)newPassword error:(NSError **)outError;

/*!
    @method     synchronizeAndReturnError:
    @abstract   Synchronizes the record from the Directory in order to get current data and/or commit pending changes
    @discussion Synchronizes the record from the Directory in order to get current data.  Any previously fetched attributes
                will be re-fetch from the Directory.  This will not re-fetch the entire record, unless the entire record
                has been accessed.  Additionally, any changes made to the record will be committed to the directory,
                if the node does not do immediate commits.  outError is optional parameter, nil can be passed if error details
                are not needed.
*/
- (BOOL)synchronizeAndReturnError:(NSError **)outError;

/*!
    @property   recordType
    @abstract   Type of the record.
    @discussion The record type.
*/
@property (nonatomic, readonly, copy) NSString *recordType;

/*!
    @property   recordName
    @abstract   Name of the record.
    @discussion This is the official record name.
*/
@property (nonatomic, readonly, copy) NSString *recordName;

/*!
    @method     recordDetailsForAttributes:error:
    @abstract   Returns the attributes and values in the form of a key-value pair set.
    @discussion Returns the attributes and values in the form of a key-value pair set for this record.  The key is a 
                NSString of the attribute name (e.g., kODAttributeTypeRecordName, etc.) and the value is an NSArray
                of either NSData or NSString depending on the type of data.  Binary data will be returned as NSData.
                If nil is passed, then all currently retrieved attributes will be returned.  outError is optional parameter, 
                nil can be passed if error details are not needed.
*/
- (NSDictionary *)recordDetailsForAttributes:(NSArray *)inAttributes error:(NSError **)outError;

/*!
    @method     valuesForAttribute:error:
    @abstract   Returns an NSArray of NSString or NSData values of the attribute
    @discussion Returns an NSArray of NSString or NSData depending on the type of data.  Binary data will be 
                returned as NSData.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (NSArray *)valuesForAttribute:(ODAttributeType)inAttribute error:(NSError **)outError;

/*!
	@method     setValue:forAttribute:error:
	@abstract   Will take a mixture of NSData or NSString or an NSArray of either type when setting the values of an attribute
	@discussion Will take a mixture of NSData or NSString or an NSArray of either type when setting the values of an attribute.
				outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)setValue:(id)inValueOrValues forAttribute:(ODAttributeType)inAttribute error:(NSError **)outError;

/*!
    @method     removeValuesForAttribute:error:
    @abstract   Removes all the values for an attribute.
    @discussion Removes all the values for an attribute.  outError is optional parameter, nil can be passed if 
                error details are not needed.
*/
- (BOOL)removeValuesForAttribute:(ODAttributeType)inAttribute error:(NSError **)outError;

/*!
    @method     addValue:toAttribute:error:
    @abstract   Will add a value to an attribute
    @discussion Will add a value to an attribute.  Should be either NSData or NSString type.  outError is optional 
                parameter, nil can be passed if error details are not needed.
*/
- (BOOL)addValue:(id)inValue toAttribute:(ODAttributeType)inAttribute error:(NSError **)outError;

/*!
    @method     removeValue:fromAttribute:error:
    @abstract   Will remove a value from an attribute
    @discussion Will remove a value from an attribute.  Should be either NSData or NSString type.  outError is optional 
                parameter, nil can be passed if error details are not needed.
*/
- (BOOL)removeValue:(id)inValue fromAttribute:(ODAttributeType)inAttribute error:(NSError **)outError;

/*!
    @method     deleteRecordAndReturnError:
    @abstract   Deletes the record from the node and invalidates the record.
    @discussion Deletes the record from the node and invalidates the record.  The ODRecord should be
                released after deletion.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)deleteRecordAndReturnError:(NSError **)outError;

@end

/*!
    @category    ODRecord (ODRecordGroupExtensions)
    @discussion  Record extensions for checking and modifying group membership.
*/
@interface ODRecord (ODRecordGroupExtensions)

/*!
    @method     addMemberRecord:error:
    @abstract   Will add the record as a member of the group record
    @discussion Will add the record as a member of the group record.  An error will be returned if the record is not
                a group record.  Additionally, if the member record is not an appropriate type allowed as part of a group
                an error will be returned.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)addMemberRecord:(ODRecord *)inRecord error:(NSError **)outError;

/*!
    @method     removeMemberRecord:error:
    @abstract   Will remove the record as a member from the group record
    @discussion Will remove the record as a member from the group record. An error will be returned if the record is not
                a group record.  Additionally, if the member record is not an appropriate type allowed as part of a group
                an error will be returned.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)removeMemberRecord:(ODRecord *)inRecord error:(NSError **)outError;

/*!
    @method     isMemberRecord:error:
    @abstract   Will use membership APIs to determine if inRecord is a member of the group
    @discussion Will use membership APIs to determine if inRecord is a member of the group.  If the receiving
                object is not a group then NO will still be returned.  outError is optional parameter, nil can be passed if 
                error details are not needed.
*/
- (BOOL)isMemberRecord:(ODRecord *)inRecord error:(NSError **)outError;

@end
