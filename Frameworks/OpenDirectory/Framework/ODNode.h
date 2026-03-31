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

#import <OpenDirectory/OpenDirectory.h>

@class ODRecord;

/*!
    @class       ODNode
    @abstract    This class is used to work with OpenDirectory nodes.
    @discussion  OpenDirectory uses nodes to represent different sources of directory information, via the local disk, LDAP, etc.
*/
@interface ODNode : NSObject {
	@private
	void *_internal;
}

/*!
    @method     nodeWithSession:type:error:
    @abstract   Create an autoreleased ODNode of the given type, optionally in a specific session.
    @discussion Autoreleased instance of an ODNode with a provided ODSession and ODNodeType.  outError is 
                optional parameter, nil can be passed if error details are not needed.
*/
+ (id)nodeWithSession:(ODSession *)inSession type:(ODNodeType)inType error:(NSError **)outError;

/*!
    @method     nodeWithSession:name:error:
    @abstract   Create an autoreleased ODNode with the given name, optionally in a specific session.
    @discussion autoreleased instance of an ODNode with a provided ODSession and node name.  outError is 
                optional parameter, nil can be passed if error details are not needed.
*/
+ (id)nodeWithSession:(ODSession *)inSession name:(NSString *)inName error:(NSError **)outError;

/*!
    @method     initWithSession:type:error:
    @abstract   Initialize an ODNode instance of the given type, optionally in a specific session.
    @discussion initialize instance of an ODNode with a provided ODSession and ODNodeType.  outError is 
                optional parameter, nil can be passed if error details are not needed.
*/
- (id)initWithSession:(ODSession *)inSession type:(ODNodeType)inType error:(NSError **)outError;

/*!
    @method     initWithSession:name:error:
    @abstract   Initialize an ODNode instance with the given name, optionally in a specific session.
    @discussion initialize instance of an ODNode with a provided ODSession and node name.  outError is optional
                parameter, nil can be passed if error details are not needed.
*/
- (id)initWithSession:(ODSession *)inSession name:(NSString *)inName error:(NSError **)outError;

/*!
    @method     subnodeNamesAndReturnError:
    @abstract   Returns NSArray of node names for this node, which may contain sub-nodes or search policy nodes
    @discussion Returns NSArray of node names for this node, which may contain sub-nodes or search policy nodes.
                Commonly used with Search policy nodes.  outError is optional parameter, nil can be passed if error
                details are not needed.
*/
- (NSArray *)subnodeNamesAndReturnError:(NSError **)outError;

/*!
    @method     unreachableSubnodeNamesAndReturnError:
    @abstract   Will return NSArray of names of subnodes that are not currently reachable.
    @discussion Will return NSArray of names of subnodes that are not currently reachable.  Commonly used with Search policy 
                nodes to determine if any nodes are currently unreachable, but may also return other subnodes if the
                OpenDirectory plugin supports.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (NSArray *)unreachableSubnodeNamesAndReturnError:(NSError **)outError;

/*!
    @property   nodeName
    @abstract   The node name.
    @discussion The node name, corresponding to its path in OpenDirectory.
*/
@property (nonatomic, readonly, copy) NSString *nodeName;

/*!
    @method     nodeDetails:error:
    @abstract   Returns a dictionary of information about the instance of ODNode
    @discussion Returns a dictionary of information about the instance of ODNode.  Details such as Trust information
                (kODAttributeTypeTrustInformation) or other Node details can be retrieved.  outError is optional parameter,
                nil can be passed if error details are not needed.
                
*/
- (NSDictionary *)nodeDetailsForKeys:(NSArray *)inKeys error:(NSError **)outError;

/*!
    @method     supportedRecordTypesAndReturnError:
    @abstract   Returns a NSArray of the record types supported by this node.
    @discussion Returns a NSArray of the record types supported by this node.  If node does not support the check
                then all possible types will be returned.  outError is optional parameter, nil can be passed if error details
                are not needed.
*/
- (NSArray *)supportedRecordTypesAndReturnError:(NSError **)outError;

/*!
    @method     supportedAttributesForRecordType:error:
    @abstract   Will return a list of attribute types supported for that attribute if possible
    @discussion Will return a list of attribute types supported for that attribute if possible.  If no specific
                types are available, then all possible values will be returned instead.  outError is optional parameter,
                nil can be passed if error details are not needed.
*/
- (NSArray *)supportedAttributesForRecordType:(ODRecordType)inRecordType error:(NSError **)outError;

/*!
    @method     setCredentialsWithRecordType:recordName:password:error:
    @abstract   Sets the credentials for interaction with the ODNode
    @discussion Sets the credentials for interaction with the ODNode.  Record references, etc. will use these credentials
                to query or change data.  Setting the credentials on a node referenced by other OD object types will
                change the credentials for all for all references.  outError is optional parameter, nil can be passed if error
                details are not needed.
*/
- (BOOL)setCredentialsWithRecordType:(ODRecordType)inRecordType recordName:(NSString *)inRecordName password:(NSString *)inPassword
                               error:(NSError **)outError;

/*!
    @method     setCredentialsWithRecordType:authType:authItems:outAuthItems:context:error:
    @abstract   Allows use of other OpenDirectory types of authentications to set the credentials for an ODNode
    @discussion Allows the caller to use other types of authentications that are available in OpenDirectory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.  outError is optional parameter, nil can be passed if 
				error details is not needed.
*/
- (BOOL)setCredentialsWithRecordType:(ODRecordType)inRecordType authenticationType:(ODAuthenticationType)inType 
                 authenticationItems:(NSArray *)inItems continueItems:(NSArray **)outItems
                             context:(id *)outContext error:(NSError **)outError;

/*!
    @method     setCredentialsUsingKerberosCache:error:
    @abstract   Unsupported method.
    @discussion Unsupported method.
*/
- (BOOL)setCredentialsUsingKerberosCache:(NSString *)inCacheName error:(NSError **)outError NS_DEPRECATED_MAC(10_6, 10_7);

/*!
    @method     createRecordWithRecordType:name:attributes:error:
    @abstract   Creates a record in this node, using the given name and attributes.
    @discussion Takes all the provided attributes and type to create an entire record.  The function will assign a
                UUID to the record automatically.  This UUID can be overwritten by the client by passing with the
                other attributes.  inAttributes is optional, nil can be passed if no other attributes are to be set.
*/
- (ODRecord *)createRecordWithRecordType:(ODRecordType)inRecordType name:(NSString *)inRecordName 
                              attributes:(NSDictionary *)inAttributes error:(NSError **)outError;

/*!
    @method     recordWithRecordType:name:attributes:error:
    @abstract   Returns an ODRecord object that references the requested type and name
    @discussion Returns an ODRecord object that references the requested type and name.  The record will have cached the
                attributes requested.  Further attributes can be requested via ODRecord APIs.  For performance it is best
                to ask for as many attributes that are needed as possible up front.
*/
- (ODRecord *)recordWithRecordType:(ODRecordType)inRecordType name:(NSString *)inRecordName attributes:(id)inAttributes
                             error:(NSError **)outError;

/*!
    @method     customCall:sendData:error:
    @abstract   Sends a custom code to the node; input and output data formats are specific to the call.
    @discussion Sends a custom code to the node; input and output data formats are specific to the call.  outError is 
                optional parameter, nil can be passed if error details are not needed.
*/
- (NSData *)customCall:(NSInteger)inCustomCode sendData:(NSData *)inSendData error:(NSError **)outError;

@end
