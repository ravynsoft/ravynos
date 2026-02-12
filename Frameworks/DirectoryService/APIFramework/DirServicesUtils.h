/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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
 * @header DirServicesUtils
 */

#ifndef __DirServicesUtils_h__
#define	__DirServicesUtils_h__	1

// App

#include <stdarg.h>

#include <AvailabilityMacros.h>
#include <DirectoryService/DirServicesTypes.h>
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------

/*!
 * @function dsDataBufferAllocate
 */
tDataBufferPtr	dsDataBufferAllocate		(	tDirReference		inDirReference,
												UInt32				inBufferSize )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataBufferDeAllocate
 */
tDirStatus		dsDataBufferDeAllocate		(	tDirReference	inDirReference,
												tDataBufferPtr	inDataBufferPtr )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
// Data Node Routines

//-----------------------------------------------

/*!
 * @function dsDataNodeAllocateBlock
 */
tDataNodePtr	dsDataNodeAllocateBlock		(	tDirReference		inDirReference,
												UInt32				inDataNodeSize,
												UInt32				inDataNodeLength,
												tBuffer				inDataNodeBuffer )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataNodeAllocateString
 */
tDataNodePtr	dsDataNodeAllocateString	(	tDirReference	inDirReference,
												const char		*inCString )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataNodeDeAllocate
 */
tDirStatus		dsDataNodeDeAllocate		(	tDirReference	inDirReference,
												tDataNodePtr	inDataNodePtr )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataNodeSetLength
 */
tDirStatus		dsDataNodeSetLength			(	tDataNodePtr		inDataNodePtr,
												UInt32				inDataNodeLength )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataNodeGetLength
 */
UInt32	dsDataNodeGetLength					(	tDataNodePtr	inDataNodePtr )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataNodeGetSize
 */
UInt32	dsDataNodeGetSize					(	tDataNodePtr	inDataNodePtr )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;



//-----------------------------------------------
//-----------------------------------------------
//-----------------------------------------------
// Data list Routines
//-----------------------------------------------

/*!
 * @function dsDataListAllocate
 */
tDataListPtr	dsDataListAllocate			(	tDirReference	inDirReference )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataListDeallocate
 * @discussion Note that if the tDataListPtr is a heap based tDataList rather than
 *		stack based, that you must also call free() to release the memory for the
 *		head of the list after calling dsDataListDeallocate().
 */
tDirStatus		dsDataListDeallocate		(	tDirReference	inDirReference,
												tDataListPtr	inDataList )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataListDeAllocate
 * @discussion Included only for backward compatibility. Equivalent to dsDataListDeallocate.
 */
tDirStatus		dsDataListDeAllocate		(	tDirReference	inDirReference,
												tDataListPtr	inDataList,
												dsBool			inDeAllocateNodesFlag )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

//-----------------------------------------------

/*!
 * @function dsGetPathFromList
 */
char*			dsGetPathFromList			(	tDirReference	inDirReference,
												const tDataList	*inDataList,
												const char		*inDelimiter )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsBuildFromPath
 */
tDataListPtr	dsBuildFromPath				(	tDirReference	inDirReference,
												const char		*inPathCString,
												const char		*inPathSeparatorCString )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsBuildListFromPathAlloc
 */
tDirStatus		dsBuildListFromPathAlloc	(	tDirReference	inDirReference,
												tDataListPtr	inDataList,
												const char		*inPathCString,
												const char		*inPathSeparatorCString )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


/*!
 * @function dsBuildListFromNodesAlloc
 */
tDirStatus		dsBuildListFromNodesAlloc	(	tDirReference	inDirReferences,
												tDataListPtr	inDataList,
												tDataNodePtr	in1stDataNodePtr,
												... )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsBuildListFromStrings
 */
tDataListPtr	dsBuildListFromStrings		(	tDirReference	inDirReference,
												const char		*in1stCString,
												... )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsBuildListFromStringsAlloc
 */
tDirStatus		dsBuildListFromStringsAlloc	(	tDirReference	inDirReferences,
												tDataListPtr	inDataList,
												const char		*in1stCString,
												... )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsBuildListFromStringsAllocV
 */
tDirStatus		dsBuildListFromStringsAllocV ( tDirReference	inDirRef,
												tDataList		*inDataList,
												const char		*in1stCString,
												va_list			args )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsAppendStringToListAlloc
 */
tDirStatus		dsAppendStringToListAlloc	(	tDirReference	inDirReferences,
												tDataListPtr	inDataList,
												const char		* inCString )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

//-----------------------------------------------

/*!
 * @function dsDataListGetNodeCount
 */
UInt32	dsDataListGetNodeCount				(	const tDataList	*inDataList )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsAllocStringsFromList
 * @discussion Provides a char** of the strings contained within a tDataList.
 */
char** dsAllocStringsFromList( tDirReference inDirRef, const tDataList *inDataList )
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsGetDataLength
 */
UInt32	dsGetDataLength						(	const tDataList	*inDataList );


/*!
 * @function dsDataListInsertAfter
 * @param inNodeIndex One-based index of the existing node to insert the new node after.
 * 		If inNodeIndex is 0, then it is inserted at the head of the list.
 * @discussion The datanode is copied into the datalist.  The caller owns the original
 *		datanode, therefore must deallocate after adding to the list.
 */
tDirStatus		dsDataListInsertAfter		(	tDirReference	inDirReferences,
												tDataListPtr	inDataList,
												tDataNodePtr	inInsertDataNode,
												const UInt32	inNodeIndex )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataListMergeListAfter
 */
tDirStatus		dsDataListMergeListAfter	(	tDataListPtr	inTargetList,
												tDataListPtr	inSourceList,
												const UInt32	inNodeIndex )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataListCopyList
 */
tDataListPtr	dsDataListCopyList			(	tDirReference	inDirReference,
												const tDataList	*inDataListSource )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataListDeleteThisNode
 */
tDirStatus		dsDataListDeleteThisNode	(	tDirReference		inDirReference,
												tDataListPtr		inDataList,
												UInt32				inNodeIndex )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDataListGetNodeAlloc
 */
tDirStatus		dsDataListGetNodeAlloc		(	tDirReference		inDirReference,
												const tDataList		*inDataListPtr,
												UInt32				inNodeIndex,
												tDataNodePtr		*outDataNode )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

//-----------------------------------------------

/*!
 * @function dsAllocAttributeValueEntry
 */
tAttributeValueEntryPtr		dsAllocAttributeValueEntry		(	tDirReference			inDirRef,
																UInt32					inAttrValueID,
																void				   *inAttrValueData,
																UInt32					inAttrValueDataLen )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDeallocAttributeValueEntry
 */
tDirStatus					dsDeallocAttributeValueEntry	(	tDirReference			inDirRef,
																tAttributeValueEntryPtr	inAttrValueEntry )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDeallocAttributeEntry
 */
tDirStatus					dsDeallocAttributeEntry			(	tDirReference			inDirRef,
																tAttributeEntryPtr		inAttrEntry )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsDeallocRecordEntry
 */
tDirStatus					dsDeallocRecordEntry			(	tDirReference			inDirRef,
																tRecordEntryPtr			inRecEntry )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;


//-----------------------------------------------

/*!
 * @function dsGetRecordNameFromEntry
 * @param outRecName Used to return the record name to the client. Client is responsible for freeing
 * 		the resulting string.
 */
tDirStatus		dsGetRecordNameFromEntry	(	tRecordEntryPtr inRecEntryPtr, char **outRecName )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsGetRecordTypeFromEntry
 * @param outRecType Used to return the record name to the client. Client is responsible for freeing
 * 		the resulting string.
 */
tDirStatus		dsGetRecordTypeFromEntry	(	tRecordEntryPtr inRecEntryPtr, char **outRecType )
DEPRECATED_IN_MAC_OS_X_VERSION_10_6_AND_LATER;

/*!
 * @function dsParseAuthAuthority
 * Pass in a complete authentication authority attribute
 * format is version;tag;data
 * retrieve version, tag, and data as separate strings
 * Memory for the char** parameters is the responsibility of the client
 * and can be cleaned up using free().
 */
tDirStatus		dsParseAuthAuthority		(   const char *inAuthAuthority, char **outVersion, char **outAuthTag, char **outAuthData ) 
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsCopyDirStatusName
 * Pass in the tDirStatus enum and receive the string of the enum name.
 * Memory for the char* is the responsibility of the client
 * and can be cleaned up using free().
 */
char*			dsCopyDirStatusName			(   SInt32 inDirStatus ) 
AVAILABLE_MAC_OS_X_VERSION_10_4_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function	dsFillAuthBuffer
 * @abstract	Fills a buffer with a list of items.
 * @discussion	Use this function as a convenient way to compose the buffer
 *				for calls to dsDoDirNodeAuth().
 * @param		inOutAuthBuffer pass in a preallocated buffer to be filled.
 * @param		inCount the number of length/data pairs on the stack
 * @param		inLen the length of one buffer item
 * @param		inData a pointer to inLen bytes of data
 */
tDirStatus	dsFillAuthBuffer			(	tDataBufferPtr inOutAuthBuffer,
												UInt32			inCount,
												UInt32			inLen,
												const void *inData, ... )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsAppendAuthBuffer
 * @abstract	Appends a list of items to an existing buffer.
 * @discussion	Use this function as a convenient way to compose the buffer
 *				for calls to dsDoDirNodeAuth().
 * @param		inOutAuthBuffer pass in a preallocated buffer.
 * @param		inCount the number of length/data pairs on the stack
 * @param		inLen the length of one buffer item
 * @param		inData a pointer to inLen bytes of data
 */
tDirStatus		dsAppendAuthBuffer				(	tDataBufferPtr inOutAuthBuffer,
												UInt32			inCount,
												UInt32			inLen,
													const void *inData, ... )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsAppendAuthBufferWithAuthorityAttribute
 * @abstract	Inserts a user name with authentication authority data into
 *				an existing buffer.
 * @discussion	Use this function for authentication methods that contain user
 *				or authenticator names and the authentication authority attribute
 *				has already been retrieved.
 * @param		inNodeRef a node reference for the record to parse
 * @param		inRecordListBuffPtr the data returned from dsGetDataList
 * @param		inAttributePtr an attribute with authentication authority data
 * @param		inValueRef the reference for the kDSNAttrAuthenticationAuthority
 *				attribute.
 * @param		inUserName the name of the user to authenticate
 * @param		inOutAuthBuffer pass in a preallocated buffer, returns with
 *				the user data appended.
 * @result		tDirStatus code
 */
tDirStatus	dsAppendAuthBufferWithAuthorityAttribute
												(	tDirNodeReference inNodeRef,
													tDataBufferPtr inRecordListBuffPtr,
													tAttributeEntryPtr inAttributePtr,
													tAttributeValueListRef inValueRef,
													const char *inUserName,
													tDataBufferPtr inOutAuthBuffer )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsAppendAuthBufferWithAuthorityStrings
 * @abstract	Inserts a user name with authentication authority data into
 *				an existing buffer.
 * @discussion	Use this function for authentication methods that contain user
 *				or authenticator names and the authentication authority attribute
 *				has already been retrieved.
 * @param		inUserName the name of the user to authenticate
 * @param		inAuthAuthority a NULL terminated array of C strings
 * @param		inOutAuthBuffer pass in a preallocated buffer, returns with
 *				the user data appended.
 * @result		tDirStatus code
 */

tDirStatus dsAppendAuthBufferWithAuthorityStrings
												(	const char *inUserName,
													const char *inAuthAuthority[],
													tDataBufferPtr inOutAuthBuffer )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

/*!
 * @function dsServiceInformationAllocate
 * @abstract	Allocate a buffer that contains the xml plist form of a
 *				CFDictionary.
 * @discussion	Services can use this function to obtain a buffer suitable for
 *				providing additional information to dsDoDirNodeAuth(). This buffer
 *				is also used to return data from an authentication method, so it
 *				needs to be large enough to handle the context information and
 *				the data returned by the authentication method.
 * @param		inServiceInfo A dictionary that contains context information
 *				from a service
 * @param		inBufferSize The desired size of the buffer. It is expanded if
 *				necessary to fit the context information. The buffer must be large
 *				enough to hold the data returned by the authentication method used.
 * @param		outPackedServiceInfo A constructed buffer containing the data
 *				from inServiceInfo.
 * @result		tDirStatus code
*/

tDirStatus dsServiceInformationAllocate			(	CFDictionaryRef inServiceInfo,
													UInt32 inBufferSize,
													tDataBufferPtr *outPackedServiceInfo )
AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED_IN_MAC_OS_X_VERSION_10_6;

#ifdef __cplusplus
}
#endif

#endif
