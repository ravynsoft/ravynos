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

/*!
 * @header DirServices
 * APIs.
 */

/*!
 * @header DirectoryServices
 * Directory Services introduction text.
 */

#ifndef __DirServices_h__
#define __DirServices_h__

#include <AvailabilityMacros.h>
#include <DirectoryService/DirServicesTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------
// Directory Services Calls

#pragma mark Calls

/*!
 * @function dsOpenDirService
 * @discussion Opens Directory Services API reference. Must be called before any other
 * 		Directory Services API calls because this reference is needed for any other call.
 * @param outDirReference reference to use in subsequent Directory Services API calls
 */
tDirStatus	dsOpenDirService			(	tDirReference	   *outDirReference	) 
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsOpenDirServiceProxy
 * @discussion Opens Directory Services API reference via TCP. Must be called before any other
 * 		Directory Services API calls because this reference is needed for any other call.
 * @param outDirRef reference to use in subsequent Directory Services API calls
 * @param inHostOrIPAddress either the hostname or dotted IP address of the remote DirectoryService machine
 * @param inIPPort either the client defined port or "0" which then allows use of the default
 */
#define DSPROXY 1
tDirStatus	dsOpenDirServiceProxy 		(	tDirReference		   *outDirRef,
											const char			   *inHostOrIPAddress,
											UInt32					inIPPort,
											tDataNodePtr			inAuthMethod,
											tDataBufferPtr			inAuthStepData,
											tDataBufferPtr			outAuthStepDataResponse,
											tContextData		   *ioContinueData )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
	
/*!
 * @function dsOpenDirServiceLocal
 * @discussion Opens Directory Services API reference to Local Access only daemon. Must be called before any other
 * 		Directory Services API calls because this reference is needed for any other call.
 * @param outDirRef reference to use in subsequent Directory Services API calls
 * @param inFilePath unix file path for a ds local node database [if either NULL or "Default" input then the default node is used]
 */
tDirStatus	dsOpenDirServiceLocal 		(	tDirReference		   *outDirRef,
											const char			   *inFilePath )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
	
/*!
 * @function dsCloseDirService
 * @param inDirReference Directory Services API reference to be closed
 */
tDirStatus	dsCloseDirService			(	tDirReference		inDirReference		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


/*!
 * @function dsAddChildPIDToReference
 */
tDirStatus dsAddChildPIDToReference		(	tDirReference			inDirRef,
											SInt32					inValidChildPID,
											UInt32					inValidAPIReferenceToGrantChild )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsIsDirServiceRunning
 */
tDirStatus dsIsDirServiceRunning		( )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsIsDirServiceLocalRunning
 */
tDirStatus dsIsDirServiceLocalRunning	( )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsGetDirNodeCount
 * @discussion Get the count of the total number of directory nodes in the system.
 * @param inDirReference Directory reference established with dsOpenDirService.
 * @param outDirectoryNodeCount Contains count of the total number of directory nodes.
 */
tDirStatus	dsGetDirNodeCount		(	tDirReference		inDirReference,
										UInt32			   *outDirectoryNodeCount	)
AVAILABLE_MAC_OS_X_VERSION_10_2_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsGetDirNodeCountWithInfo
 * @discussion Get the count of the total number of DirNodes in the Directory System with change information
 * 		ie. a token gets returned with a different value if there has been a change in the registered nodes
 * 		so the client can retain the first token and compare with the second to see if any registered nodes have changed
 * @param inDirReference Directory Reference Established with dsOpenDirService
 * @param outDirectoryNodeCount Contains count of the total number of nodes in the directory
 * @param outDirectoryNodeChangeToken Contains token that changes upon any registered node changes.
 */
tDirStatus	dsGetDirNodeCountWithInfo		(	tDirReference		inDirReference,
												UInt32			   *outDirectoryNodeCount,
												UInt32			   *outDirectoryNodeChangeToken	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetDirNodeList
 * @discussion Fill a buffer with the names of all the directory nodes.
 * @param inDirReference Directory reference established with dsOpenDirService
 * @param inOutDataBufferPtr Contains a client allocated buffer to store results..data is extracted with dsGetDirNodeName
 * @param outDirNodeCount Number of directory node names contained in dataBuffer
 * @param inOutContinueData Pointer to a tContextData variable, if (*inOutCountinueData == 0) there is no more data
 * 		otherwise can be used in a 2nd call to the same routine to get the remainder of the directory node list.
 * 		if client does not use  if (*inOutCountinueData != 0) and the client doesn't wish to continue
 * 		then dsReleaseContinueData should be called to clean up..
 */
tDirStatus	dsGetDirNodeList		(	tDirReference		inDirReference,
										tDataBufferPtr		inOutDataBufferPtr,
										UInt32			   *outDirNodeCount,
										tContextData	   *inOutContinueData		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsReleaseContinueData
 * @discussion If continue data from dsGetDirNodeList or any other Directory Services
 * 		function is non-NULL, then call this routine to release the continue data
 * 		if the client chooses not to continue the directory node listing or other operation.
 * @param inDirReference Directory reference established with dsOpenDirService if
 *		inContinueData was returned by dsGetDirNodeList or dsFindDirNodes, node reference 
 *		(type tDirNodeReference) established with dsOpenDirNode 
 *		if inContinueData was returned by a node specific API such as dsGetRecordList,
 *		dsDoAttributeValueSearch, dsDoAttributeValueSearchWithData, dsDoMultipleAttributeValueSearch,
 *		dsDoMultipleAttributeValueSearchWithData, dsGetDirNodeInfo, dsDoDirNodeAuth,
 *		or dsDoDirNodeAuthOnRecordType.
 * @param inContinueData Pointer to a tContextData variable which will be cleaned up by Directory Services
 */
tDirStatus	dsReleaseContinueData	(	tDirReference	inDirReference,	
										tContextData	inContinueData		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsFindDirNodes
 * @discussion Find directory nodes matching a certain pattern.
 * @param inDirReference Directory reference established with dsOpenDirService
 * @param inOutDataBufferPtr Contains a client allocated buffer to store results. Data is extracted with dsGetDirNodeName.
 * @param inNodeNamePattern A tDataList pointer, which contains the pattern to be matched.
 * @param inPatternMatchType What type of match to perform on inNodeNamePattern.
 * 		Valid values for this are: 	eDSExact, eDSStartsWith, eDSEndsWith, eDSContains.
 * 		Other match types will return an error.
 * @param outDirNodeCount Number of items in the client buffer.
 * @param inOutContinueData Pointer to a tContextData variable, if (*inOutCountinueData == 0) there is no more data
 * 		otherwise can be used in a 2nd call to the same routine to get the remainder of the directory node list.
 * 		if client does not use  if (*inOutCountinueData != 0) and the client doesn't wish to continue
 * 		then dsReleaseContinueData should be called to clean up.
 */
tDirStatus	dsFindDirNodes			(	tDirReference			inDirReference,
										tDataBufferPtr			inOutDataBufferPtr,
										tDataListPtr			inNodeNamePattern,
										tDirPatternMatch		inPatternMatchType,
										UInt32				   *outDirNodeCount,
										tContextData		   *inOutContinueData )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

										
/*!
 * @function dsGetDirNodeName
 * @discussion Parse the return Buffer from dsFindDirNodes or dsGetDirNodeList
 * 		and build a tDataList representing the directory node's name.
 * @param inDirReference Directory reference established with dsOpenDirService
 * @param inOutDataBuffer A buffer containing all the directory node names from
 * 		dsGetDirNodeList or dsFindDirNodes
 * @param inDirNodeIndex One-based index of directory node name to fetch/build.
 * @param inOutDataList Address of tDataListPtr that is built by this call.
 * 		The client is responsible for disposing of it with dsDataListDeAllocate.
 */
tDirStatus	dsGetDirNodeName		(	tDirReference		inDirReference,
										tDataBufferPtr		inOutDataBuffer,
										UInt32				inDirNodeIndex,
										tDataListPtr	   *inOutDataList			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsOpenDirNode
 * @discussion Establish a session for a particular directory node.
 * @param inDirReference Directory reference established with dsOpenDirService.
 * @param inDirNodeName Directory node name to open. Should be split into path
 * 		components, for example as a result of dsBuildListFromPath
 * @param outDirNodeReference Valid call with eDSNoErr, results in a directory node session reference.
 * 		This reference represents the client's session context for the contents of the given directory node.
 */
tDirStatus	dsOpenDirNode				(	tDirReference	inDirReference,
											tDataListPtr	inDirNodeName,
											tDirNodeReference	*outDirNodeReference	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsCloseDirNode
 * @discussion Tear down a directory node session.
 * @param inDirNodeReference Directory node reference obtained from dsOpenDirNode
 */
tDirStatus	dsCloseDirNode				(	tDirNodeReference	inDirNodeReference		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


/*!
 * @function dsGetDirNodeInfo
 * @discussion Get information about a directory node: authentication methods, unique ID's,
 * 		ICON information, access controls, record types contained in this node, plug-in information,
 * 		directory node/type/signature.
 * @param inDirNodeReference Directory node reference obtained from dsOpenDirNode.
 * @param inDirNodeInfoTypeList tDataList containing the types of requested data.
 * @param inOutDataBuffer Client-allocated buffer to hold the data results.
 * @param inAttributeInfoOnly This flag is set to true if the client wants attribute info only,
 * 		not attribute values.
 * @param outAttributeInfoCount A count of the number of data types present in the DataBuffer.
 * @param inOutContinueData Pointer to a tContextData variable. If (*inOutCountinueData == 0)
 * 		there is no more data. Otherwise can be used in the next call to the same routine to get
 * 		the remainder of the information. If client does not use non-NULL continue data,
 * 		then dsReleaseContinueData should be called to clean up.
 */
tDirStatus	dsGetDirNodeInfo		(	tDirNodeReference		inDirNodeReference,
										tDataListPtr			inDirNodeInfoTypeList,
										tDataBufferPtr			inOutDataBuffer,
										dsBool					inAttributeInfoOnly,
										UInt32				   *outAttributeInfoCount,
										tAttributeListRef	   *outAttributeListRef,
										tContextData		   *inOutContinueData			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetRecordList
 * @discussion Get a list of records, optionally: by name, by type, with or without attribute info,
 * 		with or without attribute value data.
 * @param inDirNodeReference Directory node reference obtained from dsOpenDirNode.
 * @param inOutDataBuffer A client-allocated buffer to hold the data results.
 * @param inRecordNameList A tDataList of Record names to be matched.
 * @param inPatternMatchType How is the pattern matched for the inRecordNameList.
 * @param inRecordTypeList What record types do we want returned?
 * @param inAttributeTypeList What type of attributes do we want for each record.
 * @param inAttributeInfoOnly Do we want attribute information only, or do we also want attribute values.
 * @param inOutRecordEntryCount How many record entries are there in the client buffer.
 * 		However, also a limit of the maximum records returned as provided by the client.
 * 		If zero or less, then assuming no limit on number of records to be returned.
 * @param inOutContinueData Pointer to a tContextData variable. If (*inOutCountinueData == 0)
 * 		there is no more data. Otherwise can be used in the next call to the same routine to get
 * 		the remainder of the information. If client does not use and continue data is non-NULL
 * 		then dsReleaseContinueData should be called to clean up.
 */
tDirStatus	dsGetRecordList		(	tDirNodeReference		inDirNodeReference,
									tDataBufferPtr			inOutDataBuffer,
									tDataListPtr			inRecordNameList,
									tDirPatternMatch		inPatternMatchType,
									tDataListPtr			inRecordTypeList,
									tDataListPtr			inAttributeTypeList,
									dsBool					inAttributeInfoOnly,
									UInt32				   *inOutRecordEntryCount,
									tContextData		   *inOutContinueData		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
	
/*!
 * @function dsGetRecordEntry
 * @discussion Get a record entry from a buffer.
 * @param inDirNodeReference Directory node reference obtained from dsOpenDirNode.
 * @param inOutDataBuffer A client-allocated buffer to hold the data results.
 */
tDirStatus	dsGetRecordEntry	(	tDirNodeReference	inDirNodeReference,
									tDataBufferPtr		inOutDataBuffer,
									UInt32				inRecordEntryIndex,

									tAttributeListRef	*outAttributeListRef,
									tRecordEntryPtr		*outRecordEntryPtr		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetAttributeEntry
 * @discussion Get an attribute entry from a buffer.
 */
tDirStatus	dsGetAttributeEntry	(	tDirNodeReference		inDirNodeReference,
									tDataBufferPtr			inOutDataBuffer,
									tAttributeListRef		inAttributeListRef,
									UInt32					inAttributeInfoIndex,
									tAttributeValueListRef	*outAttributeValueListRef,
									tAttributeEntryPtr		*outAttributeInfoPtr			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetAttributeValue
 * @discussion Get an attribute value from a buffer.
 */
tDirStatus	dsGetAttributeValue			(	tDirNodeReference		inDirNodeReference,
											tDataBufferPtr			inOutDataBuffer,
											UInt32					inAttributeValueIndex,
											tAttributeValueListRef	inAttributeValueListRef,
											tAttributeValueEntryPtr	*outAttributeValue		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


/*!
* @function dsGetNextAttributeEntry
* @discussion Get the next attribute entry from a buffer. Optimized buffer extraction with offset value.
* Intent is to only service this call in the FW unless falling through to old call.
*/
tDirStatus	dsGetNextAttributeEntry		(	tDirNodeReference			inDirNodeReference,
											tDataBufferPtr				inOutDataBuffer,
											tAttributeListRef			inAttributeListRef,
											UInt32						inAttributeInfoIndex,
											SInt32					   *inOutAttributeOffset,
											tAttributeValueListRef	   *outAttributeValueListRef,
											tAttributeEntryPtr		   *outAttributeInfoPtr )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;


/*!
* @function dsGetNextAttributeValue
* @discussion Get the next attribute value from a buffer. Optimized buffer extraction with offset value.
* Intent is to only service this call in the FW unless falling through to old call.
*/
tDirStatus	dsGetNextAttributeValue		(	tDirNodeReference			inDirNodeReference,
											tDataBufferPtr				inOutDataBuffer,
											UInt32						inAttributeValueIndex,
											SInt32					   *inOutAttributeValueOffset,
											tAttributeValueListRef		inAttributeValueListRef,
											tAttributeValueEntryPtr	   *outAttributeValue )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsCloseAttributeList
 */
tDirStatus	dsCloseAttributeList		(	tAttributeListRef		inAttributeListRef	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsCloseAttributeValueList
 */
tDirStatus	dsCloseAttributeValueList	(	tAttributeValueListRef	inAttributeValueListRef	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


/*!
 * @function dsOpenRecord
 */
tDirStatus	dsOpenRecord						(	tDirNodeReference	inDirNodeReference,
													tDataNodePtr		inRecordType,
													tDataNodePtr		inRecordName,
													tRecordReference	*outRecordReference	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetRecordReferenceInfo
 */
tDirStatus	dsGetRecordReferenceInfo			(	tRecordReference	inRecordReference,
													tRecordEntryPtr		*outRecordInfo			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetRecordAttributeInfo
 */
tDirStatus	dsGetRecordAttributeInfo			(	tRecordReference	inRecordReference,
													tDataNodePtr		inAttributeType,											
													tAttributeEntryPtr	*outAttributeInfoPtr	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetRecordAttributeValueByID
 */
tDirStatus	dsGetRecordAttributeValueByID		(	tRecordReference		inRecordReference,
													tDataNodePtr			inAttributeType,											
													UInt32					inValueID,
													tAttributeValueEntryPtr	*outEntryPtr		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
	
/*!
 * @function dsGetRecordAttributeValueByIndex
 */
tDirStatus	dsGetRecordAttributeValueByIndex	(	tRecordReference		inRecordReference,
													tDataNodePtr			inAttributeType,											
													UInt32					inValueIndex,
													tAttributeValueEntryPtr	*outEntryPtr		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetRecordAttributeValueByValue
 * @param inRecordReference Record reference from an opened record.
 * @param inAttributeType Attribute type to retrieve.
 * @param inAttributeValue Attribute value whose existence is to be confirmed.
 * @param outEntryPtr Output data structure.
 * @discussion
 * This routine verifies the existence of an attribute value within a record.
 */
tDirStatus	dsGetRecordAttributeValueByValue	(	tRecordReference		inRecordReference,
													tDataNodePtr			inAttributeType,											
													tDataNodePtr			inAttributeValue,
													tAttributeValueEntryPtr	*outEntryPtr		)
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsFlushRecord
 */
tDirStatus	dsFlushRecord						(	tRecordReference		inRecordReference	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsCloseRecord
 */
tDirStatus	dsCloseRecord						(	tRecordReference		inRecordReference	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


/*!
 * @function dsSetRecordName
 */
tDirStatus	dsSetRecordName			(	tRecordReference	inRecordReference,
										tDataNodePtr		inNewRecordName			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
										
/*!
 * @function dsSetRecordType
 */
tDirStatus	dsSetRecordType			(	tRecordReference	inRecordReference,
										tDataNodePtr		inNewRecordType			)
AVAILABLE_MAC_OS_X_VERSION_10_0_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_2;

/*!
 * @function dsDeleteRecord
 */
tDirStatus	dsDeleteRecord			(	tRecordReference	inRecordReference		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsCreateRecord
 */
tDirStatus	dsCreateRecord			(	tDirNodeReference		inDirNodeReference,
										tDataNodePtr			inRecordType,
										tDataNodePtr			inRecordName		)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsCreateRecordAndOpen
 */
tDirStatus	dsCreateRecordAndOpen	(	tDirNodeReference		inDirNodeReference,
										tDataNodePtr			inRecordType,
										tDataNodePtr			inRecordName,
										tRecordReference	   *outRecordReference	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsAddAttribute
 * @discussion Add an attribute type to a record.
 */
tDirStatus	dsAddAttribute			(	tRecordReference		inRecordReference,
										tDataNodePtr			inNewAttribute,
										tAccessControlEntryPtr	inNewAttributeAccess,
										tDataNodePtr			inFirstAttributeValue	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsRemoveAttribute
 */
tDirStatus	dsRemoveAttribute		(	tRecordReference		inRecordReference,
										tDataNodePtr			inAttribute			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsAddAttributeValue
 * @discussion Add data to a record.
 */
tDirStatus	dsAddAttributeValue		(	tRecordReference		inRecordReference,
										tDataNodePtr			inAttributeType,
										tDataNodePtr			inAttributeValue			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsRemoveAttributeValue
 */
tDirStatus	dsRemoveAttributeValue	(	tRecordReference		inRecordReference,
										tDataNodePtr			inAttributeType,
										UInt32					inAttributeValueID			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsSetAttributeValue
 */
tDirStatus	dsSetAttributeValue		(	tRecordReference		inRecordReference,
										tDataNodePtr			inAttributeType,
										tAttributeValueEntryPtr	inAttributeValuePtr			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
										
/*!
 * @function dsSetAttributeValues
 * @discussion sets an attribute to have the given list of values
 * @param inRecordReference the record reference of the record to modify
 * @param inAttributeType the attribute type to set values for
 * @param inAttributeValuesPtr the list of all values for the attribute
 */
tDirStatus dsSetAttributeValues		(   tRecordReference		inRecordReference,
										tDataNodePtr			inAttributeType,
										tDataListPtr			inAttributeValuesPtr )
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsDoDirNodeAuth
 * @discussion
 *		Do an authentication session with the given authentication type.
 *		When a authentication is successful the error code 'eDSNoErr' is returned
 * 		and the outAuthStepDataResponse parameter will contain a correctly formed
 * 		kDS1AttrAuthCredential value.  This AuthCredential can be used for future
 * 		authentications for this and other directory nodes in the directory system.
 * 		Not all directory nodes will support authenticating in this manner, but most
 * 		should.  In addition the current kDS1AttrAuthCredential value can always be
 * 		obtained via dsGetDirNodeInfo call with kDS1AttrAuthCredential as one of the
 * 		requested attributes.  Directory Nodes that support using a kDS1AttrAuthCredential
 * 		will list DSAuthCredential as a support authentication method.  Support
 * 		authentication methods can be determined by calling dsGetDirNodeInfo and requesting
 * 		the kDSNAttrAuthMethod attribute for that directory node.
 * 		NOTE: it is important to note that while some Directory Nodes may support
 * 		the attempt of using a kDS1AttrAuthCredential to authenticate, when the Directory
 * 		Node plug-in decodes the Credential the authentication attempt may still fail
 * 		for plug-in specific reasons (the plug-in may find the level of original authentication
 * 		insufficient for it's requirements or configuration, or the credential may have expired
 * 		and is no longer valid).  In addition when using a kDS1AttrAuthCredential to authentication
 * 		to a different directory node than the original kDS1AttrAuthCredential was generated, the
 * 		level of access granted by the directory node plug-in may not match the level in the
 * 		original directory node.  Access granted to the contents of a directory node is entirely
 * 		at the descretion of the directory node plug-in and the directory system it represents.
 * @param inDirNodeAuthOnlyFlag Indicates if the client wishes to use
 * 		the results of this authentication process as their identity for this directory session
 * 		(inDirNodeReference) for directory node access authorization.
 * 		If the flag value is "false", then at the completion of the auth process both the
 * 		Directory Services API and the Plug-in should use this "identity" to grant
 * 		or deny access for all future directory service calls.
 * 		If the flag value is "true", then at the completion of of the auth process no identity
 * 		information will be used by the directory services or Plug-in for authorization purposes.
 * 		A file server just wishing to authenticate a user, but not change how/who it is accessing
 * 		the directory as would set this parameter to "true".
 */
tDirStatus dsDoDirNodeAuth			(	tDirNodeReference	inDirNodeReference,
										tDataNodePtr		inDirNodeAuthName,
										dsBool				inDirNodeAuthOnlyFlag,
										tDataBufferPtr		inAuthStepData,
										tDataBufferPtr		outAuthStepDataResponse,
										tContextData		*inOutContinueData			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDoDirNodeAuthOnRecordType
 * @discussion
 *		Do an authentication session with the given authentication type on the specified record type. 
 *		When a authentication is successful the error code 'eDSNoErr' is returned
 * 		and the outAuthStepDataResponse parameter will contain a correctly formed
 * 		kDS1AttrAuthCredential value.  This AuthCredential can be used for future
 * 		authentications for this and other directory nodes in the directory system.
 * 		Not all directory nodes will support authenticating in this manner, but most
 * 		should.  In addition the current kDS1AttrAuthCredential value can always be
 * 		obtained via dsGetDirNodeInfo call with kDS1AttrAuthCredential as one of the
 * 		requested attributes.  Directory Nodes that support using a kDS1AttrAuthCredential
 * 		will list DSAuthCredential as a supported authentication method.  Support
 * 		authentication methods can be determined by calling dsGetDirNodeInfo and requesting
 * 		the kDSNAttrAuthMethod attribute for that directory node.
 * 		NOTE: it is important to note that while some Directory Nodes may support
 * 		the attempt of using a kDS1AttrAuthCredential to authenticate, when the Directory
 * 		Node plug-in decodes the Credential the authentication attempt may still fail
 * 		for plug-in specific reasons (the plug-in may find the level of original authentication
 * 		insufficient for it's requirements or configuration, or the credential may have expired
 * 		and is no longer valid).  In addition when using a kDS1AttrAuthCredential to authentication
 * 		to a different directory node than the original kDS1AttrAuthCredential was generated, the
 * 		level of access granted by the directory node plug-in may not match the level in the
 * 		original directory node.  Access granted to the contents of a directory node is entirely
 * 		at the descretion of the directory node plug-in and the directory system it represents.
 * @param inDirNodeAuthOnlyFlag Indicates if the client wishes to use
 * 		the results of this authentication process as their identity for this directory session
 * 		(inDirNodeReference) for directory node access authorization.
 * 		If the flag value is "false", then at the completion of the auth process both the
 * 		Directory Services API and the Plug-in should use this "identity" to grant
 * 		or deny access for all future directory service calls.
 * 		If the flag value is "true", then at the completion of of the auth process no identity
 * 		information will be used by the directory services or Plug-in for authorization purposes.
 * 		A file server just wishing to authenticate a user, but not change how/who it is accessing
 * 		the directory as would set this parameter to "true".
 * @param inRecordType The record type to perform the auth against for the inDirNodeAuthName.
 *		If this is passed in as NULL then call will be routed as dsDoDirNodeAuth() which assumes
 *		a record type of kDSStdRecordTypeUsers.
 */
tDirStatus dsDoDirNodeAuthOnRecordType	(	tDirNodeReference	inDirNodeReference,
											tDataNodePtr		inDirNodeAuthName,
											dsBool				inDirNodeAuthOnlyFlag,
											tDataBufferPtr		inAuthStepData,
											tDataBufferPtr		outAuthStepDataResponse,
											tContextData		*inOutContinueData,
											tDataNodePtr		inRecordType			)
AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsDoAttributeValueSearch
 * @discussion Use dsGetRecordEntry, dsGetAttributeEntry, and dsGetAttributeValue
 * 		to parse the inOutDataBuffer parameter for results.
 * @param inDirNodeReference Directory node reference obtained from dsOpenDirNode.
 * @param inOutDataBuffer A client-allocated buffer to hold the data results.
 * @param inRecordTypeList The list of record types to search over.
 * @param inAttributeType Which attribute type we are to match on.
 * @param inPatternMatchType The matching criteria used.
 * @param inPattern2Match Value to match for the above attribute type.
 * @param inOutMatchRecordCount How many records we found that met the match criteria.
 * 		However, also a limit of the maximum records returned as provided by the client.
 * 		If zero or less, then assuming no limit on number of records to be returned.
 * @param inOutContinueData Pointer to a tContextData variable. If (*inOutCountinueData == 0)
 * 		there is no more data. Otherwise can be used in the next call to the same routine to get
 * 		the remainder of the information. If client does not use non-NULL continue data,
 * 		then dsReleaseContinueData should be called to clean up.
 */
tDirStatus	dsDoAttributeValueSearch	(	tDirNodeReference	inDirNodeReference,
											tDataBufferPtr		inOutDataBuffer,
											tDataListPtr		inRecordTypeList,
											tDataNodePtr		inAttributeType,
											tDirPatternMatch	inPatternMatchType,
											tDataNodePtr		inPattern2Match,
											UInt32			   *inOutMatchRecordCount,
											tContextData	   *inOutContinueData			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


/*!
 * @function dsDoMultipleAttributeValueSearch
 * @discussion Use dsGetRecordEntry, dsGetAttributeEntry, and dsGetAttributeValue
 * 		to parse the inOutDataBuffer parameter for results.
 * @param inOutDataBuffer A client-allocated buffer to hold the data results.
 * @param inRecordTypeList The list of record types to search over.
 * @param inAttributeType Which attribute type we are to match on.
 * @param inPatternMatchType The matching criteria used.
 * @param inPatterns2Match The list of values to match for the above attribute type.
 * @param inOutMatchRecordCount How many records we found that met the match criteria.
 * 		However, also a limit of the maximum records returned as provided by the client.
 * 		If zero or less, then assuming no limit on number of records to be returned.
 */
tDirStatus	dsDoMultipleAttributeValueSearch	
										(	tDirNodeReference	inDirNodeReference,
											tDataBufferPtr		inOutDataBuffer,
											tDataListPtr		inRecordTypeList,
											tDataNodePtr		inAttributeType,
											tDirPatternMatch	inPatternMatchType,
											tDataListPtr		inPatterns2Match,
											UInt32			   *inOutMatchRecordCount,
											tContextData	   *inOutContinueData			)
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;



/*!
 * @function dsDoAttributeValueSearchWithData
 * @param inOutDataBuffer A client-allocated buffer to hold the data results.
 * @param inRecordTypeList The list of record types to search over.
 * @param inAttributeMatchType Which attribute type we are to match on.
 * @param inPatternMatchType The matching criteria used.
 * @param inPatternToMatch Value to match for the above attribute type.
 * @param inAttributeTypeRequestList What type of attributes do we want for each record.
 * @param inAttributeInfoOnly Do we want attribute information only, or values too.
 * @param inOutMatchRecordCount How many records we found that met the match criteria.
 * 		However, also a limit of the maximum records returned as provided by the client.
 * 		If zero or less, then assuming no limit on number of records to be returned.
 */
tDirStatus	dsDoAttributeValueSearchWithData	(	tDirNodeReference	inDirNodeReference,
													tDataBufferPtr		inOutDataBuffer,
													tDataListPtr		inRecordTypeList,
													tDataNodePtr		inAttributeMatchType,
													tDirPatternMatch	inPatternMatchType,
													tDataNodePtr		inPatternToMatch,
													tDataListPtr		inAttributeTypeRequestList,
													dsBool				inAttributeInfoOnly,
													UInt32			   *inOutMatchRecordCount,
													tContextData	   *inOutContinueData			)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDoMultipleAttributeValueSearchWithData
 * @param inOutDataBuffer A client-allocated buffer to hold the data results.
 * @param inRecordTypeList The list of record types to search over.
 * @param inAttributeMatchType Which attribute type we are to match on.
 * @param inPatternMatchType The matching criteria used.
 * @param inPatternsToMatch The list of values to match for the above attribute type.
 * @param inAttributeTypeRequestList What type of attributes do we want for each record.
 * @param inAttributeInfoOnly Do we want attribute information only, or values too.
 * @param inOutMatchRecordCount How many records we found that met the match criteria.
 * 		However, also a limit of the maximum records returned as provided by the client.
 * 		If zero or less, then assuming no limit on number of records to be returned.
 */
tDirStatus	dsDoMultipleAttributeValueSearchWithData
												(	tDirNodeReference	inDirNodeReference,
													tDataBufferPtr		inOutDataBuffer,
													tDataListPtr		inRecordTypeList,
													tDataNodePtr		inAttributeMatchType,
													tDirPatternMatch	inPatternMatchType,
													tDataListPtr		inPatternsToMatch,
													tDataListPtr		inAttributeTypeRequestList,
													dsBool				inAttributeInfoOnly,
													UInt32			   *inOutMatchRecordCount,
													tContextData	   *inOutContinueData			)
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsDoPlugInCustomCall
 * @discussion Used for custom direct communications with plugs-ins.
 */
tDirStatus	dsDoPlugInCustomCall		(	tDirNodeReference	inDirNodeReference,
											UInt32				inCustomRequestCode,
											tDataBufferPtr		inCustomRequestData,
											tDataBufferPtr		outCustomRequestResponse	)
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;
											

/*!
 * @function dsVerifyDirRefNum
 * @discussion Verify an existing tDirReference.
 */
tDirStatus		dsVerifyDirRefNum			(	tDirReference inDirReference	)
	DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

#ifdef __cplusplus
}
#endif

#endif
