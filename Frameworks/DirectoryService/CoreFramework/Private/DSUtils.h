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
 * @header DSUtils
 */

#ifndef __DSUtils_h__
#define __DSUtils_h__	1

#include <sys/types.h>
#include <DirectoryServiceCore/PrivateTypes.h>
#include <CoreFoundation/CoreFoundation.h>

//files and directories
#define kDSLDAPPrefsDirPath						"/Library/Preferences/DirectoryService"
#define kDSLDAPPrefsFilePath					kDSLDAPPrefsDirPath "/DSLDAPv3PlugInConfig"
#define kDSLDAPPrefsTempFilePath				kDSLDAPPrefsDirPath "/DSLDAPv3PlugInConfig.plist.XXXXXXXXXX"

#define kDSNodeEvent							"com.apple.DirectoryService.node.event"

__BEGIN_DECLS

tDataBufferPtr			dsDataBufferAllocatePriv			( UInt32 inBufferSize );
tDirStatus				dsDataBufferDeallocatePriv			( tDataBufferPtr inDataBufferPtr );
tDataListPtr			dsDataListAllocatePriv				( void );
tDirStatus				dsDataListDeallocatePriv			( tDataListPtr inDataList );
char*					dsGetPathFromListPriv				( tDataListPtr inDataList, const char *inDelimiter );
tDataListPtr			dsBuildFromPathPriv					( const char *inPathCString, const char *inPathSeparatorCString );
tDataListPtr			dsBuildListFromStringsPriv			( const char *in1stCString, ... );
tDirStatus				dsAppendStringToListAllocPriv		( tDataList *inOutDataList, const char *inCString );
tDataNodePtr			dsAllocListNodeFromStringPriv		( const char *inString );
tDataNodePtr			dsGetThisNodePriv					( tDataNode *inFirsNode, const UInt32 inIndex );
tDataNodePtr			dsGetLastNodePriv					( tDataNode *inFirsNode );
tDirStatus				dsAppendStringToListPriv			( tDataList *inDataList, const char *inCString );
tDirStatus				dsDeleteLastNodePriv				( tDataList *inDataList );
UInt32					dsDataListGetNodeCountPriv			( tDataList *inDataList );
UInt32					dsGetDataLengthPriv					( tDataList *inDataList );
tDirStatus				dsDataListGetNodePriv				( tDataList *inDataList, UInt32 inNodeIndex, tDataNodePtr *outDataListNode );
char*					dsDataListGetNodeStringPriv			( tDataListPtr inDataList, UInt32 inNodeIndex );
tDirStatus				dsDataListGetNodeAllocPriv			( const tDataList *inDataList, const UInt32 inNodeIndex, tDataNode **outDataNode );
tDataListPtr			dsAuthBufferGetDataListAllocPriv	( tDataBufferPtr inAuthBuff );
tDirStatus				dsAuthBufferGetDataListPriv			( tDataBufferPtr inAuthBuff, tDataListPtr inOutDataList );
char*					dsCStrFromCharacters				( const char *inChars, size_t inLen );

void					BinaryToHexConversion				( const unsigned char *inBinary, UInt32 inLength, char *outHexStr );
void					HexToBinaryConversion				( const char *inHexStr, UInt32 *outLength, unsigned char *outBinary );

double					dsTimestamp							( void );
tDirStatus				dsGetAuthMethodEnumValue			( tDataNode *inData, UInt32 *outAuthMethod );
const char*				dsGetPatternMatchName				( tDirPatternMatch inPatternMatchEnum );

CFArrayRef				dsConvertAuthBufferToCFArray		( tDataBufferPtr inAuthBuff );
CFArrayRef				dsConvertDataListToCFArray			( tDataList *inDataList );
tDataListPtr			dsConvertCFArrayToDataList			( CFArrayRef inArray );

char*					dsGetNameForProcessID				( pid_t inPID );
CFMutableDictionaryRef	dsCreateEventLogDict				( CFStringRef inEventType, const char *inUser, CFDictionaryRef inDetails );

int						dsCreatePrefsDirectory				( void );
CFStringRef				dsCreatePrefsFilename				( const char *inFileNameBase );
	
void					dsPostNodeEvent						( void );
CFArrayRef				dsCopyKerberosServiceList			( void ); // NOTE: this is not in the framework it is only for plugins

void					*dsRetainObject						( void *object, volatile int32_t *refcount );
bool					dsReleaseObject						( void *object, volatile int32_t *refcount, bool bFree );

tDirStatus				dsGetRecordReferenceInfoInternal	( tRecordReference recordRef, tRecordEntryPtr *recordEntry );
bool					dsIsRecordDisabledInternal			( tRecordReference recordRef );

__END_DECLS

#endif
