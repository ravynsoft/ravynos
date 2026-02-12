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
 * @header DSTCPEndian
 * Provides routines to byte swap DSProxy buffers.
 */
 
#if __LITTLE_ENDIAN__

#include "DSTCPEndian.h"
#include <string.h>
#include <errno.h>			// system call error numbers
#include <unistd.h>			// for select call 
#include <stdlib.h>			// for calloc()
#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#include <Security/Authorization.h>
#include <DirectoryService/DirServicesConst.h>
#include "SharedConsts.h"	// for sComProxyData

#ifdef DSSERVERTCP

#include "CServerPlugin.h"
#include "CLog.h"
#include "CRefTable.h"

extern CRefTable gRefTable;

#else

#include "DirServicesPriv.h"

#endif

#include "CSharedData.h"

//#define DUMP_BUFFER
#ifdef DUMP_BUFFER

#include <libkern/OSAtomic.h>

char* objectTypes[] =
{
	"kResult",
	"ktDirRef",
	"ktNodeRef",
	"ktRecRef",
	"ktAttrListRef",
	"ktAttrValueListRef",
	"ktDataBuff",
	"ktDataList",
	"ktDirPattMatch",
	"kAttrPattMatch",
	"kAttrMatch",
	"kMatchRecCount",
	"kNodeNamePatt",
	"ktAccessControlEntry",
	"ktAttrEntry",
	"ktAttrValueEntry",
	"kOpenRecBool",
	"kAttrInfoOnly",
	"kRecFlags",
	"kAttrFlags",
	"kRecEntryIndex",
	"kAttrInfoIndex",
	"kAttrValueIndex",
	"kAttrValueID",
	"kOutBuffLen",
	"kAuthStepDataLen",
	"kAuthOnlyBool",
	"kDirNodeName",
	"kAuthMethod",
	"kNodeInfoTypeList",
	"kRecNameList",
	"kRecTypeList",
	"kAttrTypeList",
	"kRecTypeBuff",
	"kRecNameBuff",
	"kAttrType",
	"kAttrTypeBuff",
	"kAttrValueBuff",
	"kNewAttrBuff",
	"kFirstAttrBuff",
    "unknown",
	"kAttrBuff",
	"kAuthStepBuff",
	"kAuthResponseBuff",
	"kAttrTypeRequestList",
	"kCustomRequestCode",
	"kPluginName",

	"kNodeCount",
	"kNodeIndex",
	"kAttrInfoCount",
	"kAttrRecEntryCount",
	"ktRecordEntry",
	"kAuthStepDataResponse",

	"kContextData",
	"ktPidRef",
	"ktGenericRef",
	"kNodeChangeToken",
	"ktEffectiveUID",
	"ktUID",
	
	"kAttrMatches",
	"kAttrValueList"
};

FILE* gDumpFile = NULL;

void DumpBuf(char* buf, UInt32 len)
{
    char acsiiBuf[17];
    
    for (UInt32 i = 0; i < len; i++)
    {
        if ((i % 16) == 0) ::memset(acsiiBuf, 0, 17);
        fprintf(gDumpFile, "%02X ", buf[i]);
        if (::isprint(buf[i]))
            acsiiBuf[i % 16] = buf[i];
        else
            acsiiBuf[i % 16] = '.';
        if ((i % 16) == 15) fprintf(gDumpFile, "   %s\n", acsiiBuf);
    }
}
#endif


void SwapProxyMessage( sComProxyData* inMessage, eSwapDirection inDirection )
{
    short i;
    sObject* object;
    bool isTwoWay = false;
    
    if (inMessage == NULL) return;
    
#ifdef DUMP_BUFFER
    UInt32 bufSize = 0;
    if ( gDumpFile == NULL )
	{
		FILE *tempFile = fopen( "/Library/Logs/DirectoryService/PacketDump", "w" );
		if ( OSAtomicCompareAndSwapPtrBarrier(NULL, tempFile, (void **)&gDumpFile) == false )
		{
			// close the file.
			fclose( tempFile );
		}
	}
	
    if (inDirection == kDSSwapHostToNetworkOrder)
        fprintf(gDumpFile, "\n\n\nBuffer in host order (preSwap)\n");
    else
        fprintf(gDumpFile, "\n\n\nBuffer in network order (preSwap)\n");
	
    for (i=0; i< 10; i++)
    {
        object = &inMessage->obj[i];
        UInt32 objType = DSGetLong(&object->type, inDirection);
        UInt32 offset = DSGetLong(&object->offset, inDirection);
        UInt32 length = DSGetLong(&object->length, inDirection);
        char* type = "unknown";
        if (objType >= kResult && objType <= kAttrValueList)
            type = objectTypes[objType - kResult];
        if (objType != 0)
        {
            if (length > 0)
                    fprintf(gDumpFile, "Object %d, type %s, offset %ld, size %ld\n", i, type, DSGetLong(&object->offset, inDirection), DSGetLong(&object->length, inDirection));
            else
                    fprintf(gDumpFile, "Object %d, type %s, value %ld\n", i, type, DSGetLong(&object->count, inDirection));
        }
        if (length > 0)
        {
            UInt32 size = offset + length - sizeof(sComProxyData) + 4;
            if (size > bufSize) bufSize = size;
        }
    }
    DumpBuf(inMessage->data, bufSize);
#endif
    
    DSSwapLong(&inMessage->fDataSize, inDirection);
    DSSwapLong(&inMessage->fDataLength, inDirection);
    DSSwapLong(&inMessage->fMsgID, inDirection);
    DSSwapLong(&inMessage->fPID, inDirection);
    
	UInt32 aNodeRef = 0;
	UInt32 aNodeRefMap = 0;
	//handle CustomCall endian issues - need to determine which plugin is being used
	bool bCustomCall = false;
	UInt32 aCustomRequestNum = 0;
	bool bIsAPICallResponse = false;
	const char* aPluginName = NULL;
	
    // if this is the auth case, we need to do some checks
    for (i=0; i< 10; i++)
    {
        object = &inMessage->obj[i];
        UInt32 objType = DSGetLong(&object->type, inDirection);
		char* method;
		
		switch ( objType )
		{
			case kAuthMethod:
				// check for two-way random special case before we start swapping stuff
				method = (char *)inMessage + DSGetLong( &object->offset, inDirection );
				if ( strcmp(method, kDSStdAuth2WayRandom) == 0 )
					isTwoWay = true;
				break;
			case kCustomRequestCode:
				//check for Custom Call special casing
				bCustomCall = true;
				aCustomRequestNum = DSGetLong( &object->count, inDirection );
				break;
			case ktNodeRef:
				// need to determine the nodename for discrimination of duplicate custom call codes - server
				aNodeRef = (UInt32)DSGetLong( &object->count, inDirection );
				break;
			case ktNodeRefMap:
				// need to determine the nodename for discrimination of duplicate custom call codes - FW
				aNodeRefMap = (UInt32)DSGetLong( &object->count, inDirection );
				break;
			case kResult:
				bIsAPICallResponse = true;
				break;
		}
    } // for (i=0; i< 10; i++)
	
	if ( bCustomCall && aCustomRequestNum != 0 )
	{
#ifdef DSSERVERTCP
		if ( aNodeRef != 0 )
		{
			CServerPlugin *aPluginPtr = NULL;

			if ( gRefTable.VerifyReference(aNodeRef, eRefTypeDirNode, &aPluginPtr, -1, inMessage->fPID) == eDSNoErr )
			{
				aPluginName = aPluginPtr->GetPluginName();
				DbgLog( kLogDebug, "SwapProxyMessage - Custom code %d - module %s", aCustomRequestNum, aPluginName );
			}
			else
			{
				DbgLog( kLogError, "SwapProxyMessage - Custom code %d - Cannot find plugin - Ref: %d, PID: %d, Address: %A", 
						aCustomRequestNum, aNodeRef, inMessage->fPID, inMessage->fIPAddress );
			}
		}
#else
		if ( aNodeRefMap != 0 )
			aPluginName = dsGetPluginNamePriv( aNodeRefMap, getpid() );
#endif
	}

    // swap objects
    for (i=0; i< 10; i++)
    {
        object = &inMessage->obj[i];

        if (object->type == 0)
            continue;
            
        UInt32 objType = DSGetAndSwapLong(&object->type, inDirection);
        DSSwapLong(&object->count, inDirection);
        UInt32 objOffset = DSGetAndSwapLong(&object->offset, inDirection);
        DSSwapLong(&object->used, inDirection);
        UInt32 objLength = DSGetAndSwapLong(&object->length, inDirection);
            
        DSSwapObjectData(objType, (char *)inMessage + objOffset, objLength, (!isTwoWay), bCustomCall, aCustomRequestNum, 
						 (const char*)aPluginName, bIsAPICallResponse, inDirection);
    }
    
#ifdef DUMP_BUFFER
    if ( inDirection == kDSSwapHostToNetworkOrder )
        fprintf(gDumpFile, "\n\nBuffer in network order (post Swap)\n");
    else
        fprintf(gDumpFile, "\n\nBuffer in host order (post Swap)\n");
    DumpBuf(inMessage->data, bufSize);
    fflush(stdout);
#endif
}

#endif
