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
 * @header SharedConsts
 */

#ifndef __SharedConsts_h__
#define	__SharedConsts_h__	1

#include <mach/message.h>
#include <bsm/audit.h>
#include <sys/socket.h>

#include <DirectoryServiceCore/PrivateTypes.h>

//Auth methods used for SPI support of enabling Windows sharing authentication
/*!
 * @defined kDSStdAuthSetShadowHashSecure
 * @discussion Auth method that allows a user to self opt out of windows sharing authentication.
 * NT hash and Lan Manager authentication are disabled
 */
#define		kDSStdAuthSetShadowHashSecure				"dsAuthMethodStandard:dsAuthSetShadowHashSecure"

/*!
 * @defined kDSStdAuthSetShadowHashWindows
 * @discussion Auth method that allows a user to self opt in to windows sharing authentication.
 * NT hash and Lan Manager authentication are enabled
 */
#define		kDSStdAuthSetShadowHashWindows				"dsAuthMethodStandard:dsAuthSetShadowHashWindows"

//SPI constants
/*!
 * @defined kDSStdRecordTypeDirectoryNodeInfo
 * @discussion Identifies the dsGetDirNodeInfo record type.
 */
#define		kDSStdRecordTypeDirectoryNodeInfo		"dsRecTypeStandard:DirectoryNodeInfo"

/*!
 * @defined kDSStdRecordTypeSearchNodeInfo
 * @discussion Identifies the dsGetDirNodeInfo record type for the Search Nodes.
 */
#define		kDSStdRecordTypeSearchNodeInfo		"dsRecTypeStandard:SearchNodeInfo"

/*!
 * @defined kDBLocalDefaultPath
 * @discussion Identifies the default file path for the default DS local node.
 */
#define		kDBLocalDefaultPath					"/var/db/dslocal/nodes/Default"

typedef struct {
	unsigned int	msgt_name : 8,
					msgt_size : 8,
					msgt_number : 8,
					msgt_translate : 8;
} mach_msg_type_t;

typedef struct sObject
{
	UInt32		type;
	UInt32		count;
	UInt32		offset;
	UInt32		used;
	UInt32		length;
} sObject;

#pragma pack(4)			/* Make sure the structure stays as we defined it otherwise IPC will break */
typedef struct sComData
{
	mach_msg_type_t		type;
	UInt32				fDataSize;
	UInt32				fDataLength;
	// ---- Mach information
	mach_port_t			fMachPort;
	pid_t				fPID;
	// ---- Proxy information
	UInt32					fMsgID;
	int						fSocket;
	struct sockaddr_storage	fIPAddress;
	// ----
	uid_t				fUID;
	uid_t				fEffectiveUID;
	uid_t				fAuditUID;
	gid_t				fEffectiveGID;
	gid_t				fGID;
	au_asid_t			fAuditSID;
	au_tid_t			fTerminalID;
	// ----
	sObject				obj[ 10 ];
	char				data[ 1 ];
} sComData;
#pragma pack()

typedef struct sComProxyData
{
	mach_msg_header_t	head;
	mach_msg_type_t		type;
	UInt32				fDataSize;
	UInt32				fDataLength;
	UInt32				fMsgID;
	UInt32				fPID;
	UInt32				fUnused1; 	// not used
	UInt32				fIPAddress;
	sObject				obj[ 10 ];
	char				data[ 1 ];
} sComProxyData;

#ifdef __cplusplus
	const UInt32 kMsgBlockSize	= 1024 * 4;					// Set to average of 4k
	const UInt32 kObjSize		= (UInt32) sizeof(sObject) * 10;	// size of object struct
	//const UInt32 kIPCMsgLen		= kMsgBlockSize + kObjSize;	// IPC message block size
	const UInt32 kIPCMsgLen		= kMsgBlockSize;	// IPC message block size

	// data should be OOL, may need to be increased if we create more MIG definitions that require more inline data
    const UInt32 kMaxFixedMsg       = 16384;
    const UInt32 kMaxFixedMsgData	= (UInt32) (kMaxFixedMsg - sizeof(sComData));  // this is the max fixed data we'll send..
    const UInt32 kMaxMIGMsg         = kMaxFixedMsg + 256;               // padding for any additional data
#else
	#define kMsgBlockSize		1024 * 4
	#define kObjSize			sizeof(sObject) * 10
	#define kIPCMsgLen			kMsgBlockSize
	#define kMaxFixedMsg        16384
	#define kMaxFixedMsgData    kMaxFixedMsg - sizeof(sComData)
    #define kMaxMIGMsg          16384 + 256
#endif

typedef struct sIPCMsg
{
	mach_msg_header_t	fHeader;
	UInt32				fMsgType;
	UInt32				fCount;
	UInt32				fOf;
	UInt32				fMsgID;
	UInt32				fPID;
	UInt32				fUnused1; 	// not used
	sObject				obj[ 10 ];
	char				fData[ kIPCMsgLen ];
	mach_msg_audit_trailer_t	fTail;	//this is the largest trailer struct
										//we never set this and we never send it
										//but we have the bucket large enough to receive it
} sIPCMsg;



typedef enum {
	kResult					= 4460, //need to keep this as first for endian swapping code
	ktDirRef				= 4461,
	ktNodeRef				= 4462,
	ktRecRef				= 4463,
	ktAttrListRef			= 4464,
	ktAttrValueListRef		= 4465,
	ktDataBuff				= 4466,
	ktDataList				= 4467,
	ktDirPattMatch			= 4468,
	kAttrPattMatch			= 4469,
	kAttrMatch				= 4470,
	kMatchRecCount			= 4471,
	kNodeNamePatt			= 4472,
	ktAccessControlEntry	= 4473,
	ktAttrEntry				= 4474,
	ktAttrValueEntry		= 4475,
	kOpenRecBool			= 4476,
	kAttrInfoOnly			= 4477,
	kRecFlags				= 4478,
	kAttrFlags				= 4479,
	kRecEntryIndex			= 4480,
	kAttrInfoIndex			= 4481,
	kAttrValueIndex			= 4482,
	kAttrValueID			= 4483,
	kOutBuffLen				= 4484,
	kAuthStepDataLen		= 4485,
	kAuthOnlyBool			= 4486,
	kDirNodeName			= 4487,
	kAuthMethod				= 4488,
	kNodeInfoTypeList		= 4489,
	kRecNameList			= 4490,
	kRecTypeList			= 4491,
	kAttrTypeList			= 4492,
	kRecTypeBuff			= 4493,
	kRecNameBuff			= 4494,
	kAttrType				= 4495,
	kAttrTypeBuff			= 4496,
	kAttrValueBuff			= 4497,
	kNewAttrBuff			= 4498,
	kFirstAttrBuff			= 4499,
	kAttrBuff				= 4501,
	kAuthStepBuff			= 4502,
	kAuthResponseBuff		= 4503,
	kAttrTypeRequestList	= 4504,
	kCustomRequestCode		= 4505,
	kPluginName				= 4506,

	kNodeCount				= 4507,
	kNodeIndex				= 4508,
	kAttrInfoCount			= 4509,
	kAttrRecEntryCount		= 4510,
	ktRecordEntry			= 4511,
	kAuthStepDataResponse	= 4512,

	kContextData			= 4513,
	ktPidRef				= 4514,
	ktGenericRef			= 4515,
	kNodeChangeToken		= 4516,
	ktEffectiveUID			= 4517,
	ktUID					= 4518,
	
	kAttrMatches			= 4519,
	kAttrValueList			= 4520,
	ktNodeRefMap			= 4521,			//special mapped node ref in FW for Custom Calls to retrieve plugin name
	kEnd					= 0xFFFFFFFF
} eValueType;

//Note any time an entry point reference is added it must be ensured that
//the DSPlugInStub, and DSPlugInStubC plugins all have the corresponding changes made.
//Also note that the enums combined for Server and PlugIn calls cannot exceed 255 since
//unsigned int	msgt_name : 8 --> is the used field in the mach_msg_type_t used in sComData to send the type of call
//Allow server calls to be 1 to 127 ****Only add numbers on to the end of this list - DON'T change any for backward compatibility****
enum eDSServerCalls {
	/*  1 */ kOpenDirService			= 1,
	/*    */ kCloseDirService,
	/*    */ kGetDirNodeName,
	/*    */ kGetDirNodeCount,
	/*  5 */ kGetDirNodeChangeToken,
	/*    */ kGetDirNodeList,
	/*    */ kFindDirNodes,
	/*    */ kVerifyDirRefNum,
	/*    */ kCheckUserNameAndPassword,
	/* 10 */ kAddChildPIDToReference,
	/*    */ kOpenDirServiceProxy,
	/*    */ kOpenDirServiceLocal,
	/* 13 */ kDSServerCallsEnd
};

//Note any time an entry point reference is added it must be ensured that
//the DSPlugInStub, and DSPlugInStubC plugins all have the corresponding changes made.
//Also note that the enums combined for Server and PlugIn calls cannot exceed 255 since
//unsigned int	msgt_name : 8 --> is the used field in the mach_msg_type_t used in sComData to send the type of call
//allow plugin calls to be 128 to 255 ****Only add numbers on to the end of this list - DON'T change any for backward compatibility****
enum eDSPluginCalls {
	/* 128 */ kDSPlugInCallsBegin	=  128,
	/*     */ kReleaseContinueData,
	/* 130 */ kOpenDirNode,
	/*     */ kCloseDirNode,
	/*     */ kGetDirNodeInfo,
	/*     */ kGetRecordList,
	/*     */ kGetRecordEntry,
	/* 135 */ kGetAttributeEntry,
	/*     */ kGetAttributeValue,
	/*     */ kOpenRecord,
	/*     */ kGetRecordReferenceInfo,
	/*     */ kGetRecordAttributeInfo,
	/* 140 */ kGetRecordAttributeValueByID,
	/*     */ kGetRecordAttributeValueByIndex,
	/*     */ kFlushRecord,
	/*     */ kCloseRecord,
	/*     */ kSetRecordName,
	/* 145 */ kSetRecordType,
	/*     */ kDeleteRecord,
	/*     */ kCreateRecord,
	/*     */ kCreateRecordAndOpen,
	/*     */ kAddAttribute,
	/* 150 */ kRemoveAttribute,
	/*     */ kAddAttributeValue,
	/*     */ kRemoveAttributeValue,
	/*     */ kSetAttributeValue,
	/*     */ kDoDirNodeAuth,
	/* 155 */ kDoAttributeValueSearch,
	/*     */ kDoAttributeValueSearchWithData,
	/*     */ kDoPlugInCustomCall,
	/*     */ kCloseAttributeList,
	/*     */ kCloseAttributeValueList,
	/* 160 */ kHandleNetworkTransition,
	/*     */ kServerRunLoop,
	/*     */ kDoDirNodeAuthOnRecordType,
	/*     */ kCheckNIAutoSwitch,
	/*     */ kGetRecordAttributeValueByValue,
	/* 165 */ kDoMultipleAttributeValueSearch,
	/*     */ kDoMultipleAttributeValueSearchWithData,
	/*     */ kSetAttributeValues,
	/*     */ kKerberosMutex,
	/*     */ kHandleSystemWillSleep,
	/* 170 */ kHandleSystemWillPowerOn,
	/* 171 */ kDSPlugInCallsEnd
};

//-----------------------------------------------------------------------------
//	* Logging Flags
//
//		The user may turn logging on or off, and select from the type of logging
//		desired. Certain types of logging information are NOOPs if the
//		compile-time symbol DEBUG is not defined.
//
//-----------------------------------------------------------------------------

enum eLogFlags {
	kLogNone		= 0x00000000,			// Log no information
	kLogMeta		= 0x00000001,			// Info about the log itself
	kLogApplication	= 0x00000002,			// P1
	
	// Assert
	kLogAssert		= 0x00000004,			// P1
	
	// Listener
	kLogListener	= 0x00000010,			// P1
	
	// Handler
	kLogHandler		= 0x00000100,			// P2
	kLogMsgQueue	= 0x00000200,			// P2
	
	// Threads
	kLogThreads		= 0x00001000,			// P3
	
	// Endpoint
	kLogEndpoint	= 0x00002000,			// P3
	
	// Plugin
	kLogPlugin		= 0x00004000,			// P5
	
	// Proxy
	kLogConnection	= 0x00008000,			// P3
	
	// Transactions
	kLogMsgTrans			= 0x00010000,	// P2
	
	// TCP Endpoint
	kLogTCPEndpoint			= 0x00020000,	// P3
	
	// Performance Stats
	kLogPerformanceStats	= 0x00040000,	// P5
	
	// Priority logging scheme
	kLogEmergency		= 0x00100000,		// P0
	kLogAlert			= 0x00200000,		// P1
	kLogCritical		= 0x00400000,		// P2
	kLogError			= 0x00800000,		// P3 -- redirects to error log
	kLogWarning			= 0x01000000,		// P4
	kLogNotice			= 0x02000000,		// P5
	kLogInfo			= 0x04000000,		// P6
	kLogDebug			= 0x08000000,		// P7
	
	// Specific logging scheme
	kLogAPICalls	= 0x40000000,			// P4 - API call logging
	
	// Everything
	kLogEverything	= 0x7FFFFFFF,			
	
	//logging outside of "everything"
	//dupe enum to set DBG header
	kLogDebugHeader = 0x80000000			//for setting of kDebugHdr in CLog
};

// log types


#endif // __SharedConsts_h__
