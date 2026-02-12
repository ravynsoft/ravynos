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
 * @header PluginData
 */

#ifndef __PluginData_H__
#define __PluginData_H__	1


#include <DirectoryServiceCore/PrivateTypes.h>
#include <sys/types.h>

typedef enum {
	keAttrReadOnly		= 0x00000001,
	keAttrReadWrite		= 0x00000002
} eAttributeFlags;

typedef enum {
	kUnknownState	= 0x00000000,
	kActive			= 0x00000001,
	kInactive		= 0x00000002,
	kInitalized		= 0x00000004,
	kInitialized	= 0x00000004,
	kUninitialized	= 0x00000008,
	kFailedToInit	= 0x00000010
} ePluginState;

//-------------------------------------------------
// dsGetDirNodeList

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirReference			fInDirRef;
	tDataBufferPtr			fOutDataBuff;
	UInt32					fOutNodeCount;
	tContextData			fIOContinueData;
} sGetDirNodeList;


//-------------------------------------------------
// dsReleaseContinueData

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirReference			fInDirReference;
	tContextData			fInContinueData;
} sReleaseContinueData;


//-------------------------------------------------
// dsFindDirNodes

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirReference			fInDirRef;
	tDataBufferPtr			fOutDataBuff;
	tDataListPtr			fInNodeNamePattern;
	tDirPatternMatch		fInPatternMatchType;
	UInt32					fOutDirNodeCount;
	tContextData			fOutContinueData;
} sFindDirNodes;


//-------------------------------------------------
// dsOpenDirNode

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirReference			fInDirRef;
	tDataListPtr			fInDirNodeName;
	tDirNodeReference		fOutNodeRef;
	uid_t					fInUID;
	uid_t					fInEffectiveUID;
} sOpenDirNode;


//-------------------------------------------------
// dsCloseDirNode

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirReference			fInNodeRef;
} sCloseDirNode;


//-------------------------------------------------
// dsGetDirNodeInfo

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataListPtr			fInDirNodeInfoTypeList;
	tDataBufferPtr			fOutDataBuff;
	bool					fInAttrInfoOnly;
	UInt32					fOutAttrInfoCount;
	tAttributeListRef		fOutAttrListRef;
	tContextData			fOutContinueData;
} sGetDirNodeInfo;


//-------------------------------------------------
// dsGetRecordList

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fInDataBuff;
	tDataListPtr			fInRecNameList;
	tDirPatternMatch		fInPatternMatch;
	tDataListPtr			fInRecTypeList;
	tDataListPtr			fInAttribTypeList;
	bool					fInAttribInfoOnly;
	UInt32					fOutRecEntryCount;
	tContextData			fIOContinueData;
} sGetRecordList;


//-------------------------------------------------
// dsGetRecordEntry

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fInOutDataBuff;
	UInt32					fInRecEntryIndex;
	tAttributeListRef		fOutAttrListRef;
	tRecordEntryPtr			fOutRecEntryPtr;
} sGetRecordEntry;


//-------------------------------------------------
// dsGetAttributeEntry

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fInOutDataBuff;
	tAttributeListRef		fInAttrListRef;
	UInt32					fInAttrInfoIndex;
	tAttributeValueListRef	fOutAttrValueListRef;
	tAttributeEntryPtr		fOutAttrInfoPtr;
} sGetAttributeEntry;


//-------------------------------------------------
// dsGetAttributeValue

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fInOutDataBuff;
	UInt32					fInAttrValueIndex;
	tAttributeValueListRef	fInAttrValueListRef;
	tAttributeValueEntryPtr	fOutAttrValue;
} sGetAttributeValue;


//-------------------------------------------------
// dsCloseAttributeList

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tAttributeListRef		fInAttributeListRef;
} sCloseAttributeList;


//-------------------------------------------------
// dsCloseAttributeValueList

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tAttributeValueListRef	fInAttributeValueListRef;
} sCloseAttributeValueList;


//-------------------------------------------------
// dsOpenRecord

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataNodePtr			fInRecType;
	tDataNodePtr			fInRecName;
	tRecordReference		fOutRecRef;
} sOpenRecord;


//-------------------------------------------------
// dsGetRecordReferenceInfo

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tRecordEntryPtr			fOutRecInfo;
} sGetRecRefInfo;


//-------------------------------------------------
// dsGetRecordAttributeInfo

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	tAttributeEntryPtr		fOutAttrInfoPtr;
} sGetRecAttribInfo;



//-------------------------------------------------
// dsGetRecordAttributeValueByID

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	UInt32					fInValueID;
	tAttributeValueEntryPtr	fOutEntryPtr;
} sGetRecordAttributeValueByID;


//-------------------------------------------------
// dsGetRecordAttributeValueByIndex

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	UInt32					fInAttrValueIndex;
	tAttributeValueEntryPtr	fOutEntryPtr;
} sGetRecordAttributeValueByIndex;


//-------------------------------------------------
// dsGetRecordAttributeValueByValue

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	tDataNodePtr			fInAttrValue;
	tAttributeValueEntryPtr	fOutEntryPtr;
} sGetRecordAttributeValueByValue;


//-------------------------------------------------
// dsFlushRecord

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
} sFlushRecord;


//-------------------------------------------------
// dsCloseRecord

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
} sCloseRecord;


//-------------------------------------------------
// dsSetRecordName

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInNewRecName;
} sSetRecordName;


//-------------------------------------------------
// dsSetRecordType

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInNewRecType;
} sSetRecordType;


//-------------------------------------------------
// dsDeleteRecord

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
} sDeleteRecord;


//-------------------------------------------------
// dsCreateRecord
// dsCreateRecordAndOpen

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataNodePtr			fInRecType;
	tDataNodePtr			fInRecName;
	bool					fInOpen;
	tRecordReference		fOutRecRef;
} sCreateRecord;


//-------------------------------------------------
// dsAddAttribute

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInNewAttr;
	tAccessControlEntryPtr	fInNewAttrAccess;
	tDataNodePtr			fInFirstAttrValue;
} sAddAttribute;


//-------------------------------------------------
// dsRemoveAttribute

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttribute;
} sRemoveAttribute;



//-------------------------------------------------
// dsAddAttributeValue

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	tDataNodePtr			fInAttrValue;
} sAddAttributeValue;


//-------------------------------------------------
// dsRemoveAttributeValue

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	UInt32					fInAttrValueID;
} sRemoveAttributeValue;


//-------------------------------------------------
// dsSetAttributeValue

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	tAttributeValueEntryPtr	fInAttrValueEntry;
} sSetAttributeValue;


//-------------------------------------------------
// dsSetAttributeValues

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tRecordReference		fInRecRef;
	tDataNodePtr			fInAttrType;
	tDataListPtr			fInAttrValueList;
} sSetAttributeValues;


//-------------------------------------------------
// dsDoDirNodeAuth

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataNodePtr			fInAuthMethod;
	bool					fInDirNodeAuthOnlyFlag;
	tDataBufferPtr			fInAuthStepData;
	tDataBufferPtr			fOutAuthStepDataResponse;
	tContextData			fIOContinueData;
} sDoDirNodeAuth;


//-------------------------------------------------
// dsDoDirNodeAuthOnRecordType

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataNodePtr			fInAuthMethod;
	bool					fInDirNodeAuthOnlyFlag;
	tDataBufferPtr			fInAuthStepData;
	tDataBufferPtr			fOutAuthStepDataResponse;
	tContextData			fIOContinueData;
	tDataNodePtr			fInRecordType;
} sDoDirNodeAuthOnRecordType;


//-------------------------------------------------
// dsDoAttributeValueSearch

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fOutDataBuff;
	tDataListPtr			fInRecTypeList;
	tDataNodePtr			fInAttrType;
	tDirPatternMatch		fInPattMatchType;
	tDataNodePtr			fInPatt2Match;
	UInt32					fOutMatchRecordCount;
	tContextData			fIOContinueData;
} sDoAttrValueSearch;


//-------------------------------------------------
// dsDoMultipleAttributeValueSearch

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fOutDataBuff;
	tDataListPtr			fInRecTypeList;
	tDataNodePtr			fInAttrType;
	tDirPatternMatch		fInPattMatchType;
	tDataListPtr			fInPatterns2MatchList;
	UInt32					fOutMatchRecordCount;
	tContextData			fIOContinueData;
} sDoMultiAttrValueSearch;


//-------------------------------------------------
// dsDoAttributeValueSearchWithData

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fOutDataBuff;
	tDataListPtr			fInRecTypeList;
	tDataNodePtr			fInAttrType;
	tDirPatternMatch		fInPattMatchType;
	tDataNodePtr			fInPatt2Match;
	UInt32					fOutMatchRecordCount;
	tContextData			fIOContinueData;
	tDataListPtr			fInAttrTypeRequestList;
	bool					fInAttrInfoOnly;
} sDoAttrValueSearchWithData;


//-------------------------------------------------
// dsDoMultipleAttributeValueSearchWithData

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	tDataBufferPtr			fOutDataBuff;
	tDataListPtr			fInRecTypeList;
	tDataNodePtr			fInAttrType;
	tDirPatternMatch		fInPattMatchType;
	tDataListPtr			fInPatterns2MatchList;
	UInt32					fOutMatchRecordCount;
	tContextData			fIOContinueData;
	tDataListPtr			fInAttrTypeRequestList;
	bool					fInAttrInfoOnly;
} sDoMultiAttrValueSearchWithData;


//-------------------------------------------------
// dsDoPlugInCustomCall

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	tDirNodeReference		fInNodeRef;
	UInt32					fInRequestCode;
	tDataBufferPtr			fInRequestData;
	tDataBufferPtr			fOutRequestResponse;
	tDirNodeReference		fInNodeRefMap; //used for endian byte swapping only 
} sDoPlugInCustomCall;

//-------------------------------------------------
// Internal Network Transition Call Thru

typedef struct {
	UInt32					fType;
	SInt32					fResult;
//future distinguishing of type of transition
} sNetworkTransitionValue;


//-------------------------------------------------
// Header

typedef struct {
	UInt32					fType;
	SInt32					fResult;
	void				   *fContextData;
} sHeader;

#endif // __PluginData_H__
