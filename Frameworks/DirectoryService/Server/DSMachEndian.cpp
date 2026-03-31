/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
 * @header DSMachEndian
 * Provides routines to byte swap Mach buffers.
 */
 
#if __LITTLE_ENDIAN__

#include "DSMachEndian.h"
#include <string.h>
#include <errno.h>			// system call error numbers
#include <unistd.h>			// for select call 
#include <stdlib.h>			// for calloc()
#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#include <Security/Authorization.h>
#include <DirectoryService/DirServicesConst.h>
#include "DirServicesPriv.h"

#include "CServerPlugin.h"
#include "CLog.h"
#include "CReftable.h"
#include "CSharedData.h"
#include "SharedConsts.h"	// for sComData

// uncomment the line below to dump the mach buffers to a file
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

static FILE* gDumpFile = NULL;

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

void SwapMachMessage( sComData *message, eSwapDirection direction )
{
    short i;
    sObject* object;
    bool isTwoWay = false;
    
    if (message == nil) return;
    
#ifdef DUMP_BUFFER
    UInt32 bufSize = 0;
    if ( gDumpFile == NULL )
	{
		FILE *tempFile = fopen( "/Library/Logs/DirectoryService/MachMsgDump", "w" );
		if ( OSAtomicCompareAndSwapPtrBarrier(NULL, tempFile, (void **) &gDumpFile) == false )
		{
			// close the file.
			fclose( tempFile );
		}
	}
        
    if (direction == kDSSwapNetworkToHostOrder)
        fprintf(gDumpFile, "\n\n\nBuffer in Network order (preSwap)\n");
    else
        fprintf(gDumpFile, "\n\n\nBuffer in Host order (preSwap)\n");
	
    for (i=0; i< 10; i++)
    {
        object = &message->obj[i];
        UInt32 objType = DSGetLong(&object->type, direction);
        UInt32 offset = DSGetLong(&object->offset, direction);
        UInt32 length = DSGetLong(&object->length, direction);
        char* type = "unknown";
        if (objType >= kResult && objType <= kAttrValueList)
            type = objectTypes[objType - kResult];
        if (objType != 0)
        {
            if (length > 0)
                    fprintf(gDumpFile, "Object %d, type %s, offset %ld, size %ld\n", i, type, DSGetLong(&object->offset, direction), DSGetLong(&object->length, direction));
            else
                    fprintf(gDumpFile, "Object %d, type %s, value %ld\n", i, type, DSGetLong(&object->count, direction));
        }
        if (length > 0)
        {
            UInt32 size = offset + length - sizeof(sComData) + 4;
            if (size > bufSize) bufSize = size;
        }
    }
    DumpBuf(message->data, bufSize);
#endif
    
    DSSwapLong(&message->fDataSize, direction);
    DSSwapLong(&message->fDataLength, direction);
    DSSwapLong(&message->fMsgID, direction);
    
	UInt32 aNodeRef = 0;

	//handle CustomCall endian issues - need to determine which plugin is being used
	bool bCustomCall = false;
	UInt32 aCustomRequestNum = 0;
	bool bIsAPICallResponse = false;
	const char* aPluginName = nil;
    // if this is the auth case or custom call case, we need to do some checks
    for (i=0; i< 10; i++)
    {
        object = &message->obj[i];
        UInt32 objType = DSGetLong( &object->type, direction );
		char* method;
		
		switch ( objType )
		{
			case kAuthMethod:
				// check for two-way random special case before we start swapping stuff
				method = (char *)message + DSGetLong( &object->offset, direction );
				if ( strcmp(method, kDSStdAuth2WayRandom) == 0 )
					isTwoWay = true;
				break;
			case kCustomRequestCode:
				//check for Custom Call special casing
				bCustomCall = true;
				aCustomRequestNum = DSGetLong( &object->count, direction );
				break;
			case ktNodeRef:
				//need to determine the nodename for discrimination of duplicate custom call codes - server
				aNodeRef = (UInt32)DSGetLong( &object->count, direction );
				break;
			case kResult:
				bIsAPICallResponse = true;
				break;
		}
    } // for (i=0; i< 10; i++)
	
	if ( bCustomCall && aCustomRequestNum != 0 && aNodeRef != 0 )
	{
		aPluginName = dsGetPluginNamePriv( aNodeRef, getpid() );
	}

    // swap objects
    for (i=0; i< 10; i++)
    {
        object = &message->obj[i];

        if (object->type == 0)
            continue;
            
        UInt32 objType = DSGetAndSwapLong(&object->type, direction);
        DSSwapLong(&object->count, direction);
        UInt32 objOffset = DSGetAndSwapLong(&object->offset, direction);
        DSSwapLong(&object->used, direction);
        UInt32 objLength = DSGetAndSwapLong(&object->length, direction);
            
        DSSwapObjectData(objType, (char *)message + objOffset, objLength, (!isTwoWay), bCustomCall, aCustomRequestNum, (const char*)aPluginName, bIsAPICallResponse, direction);
    }
    
#ifdef DUMP_BUFFER
    if (direction == kDSSwapNetworkToHostOrder)
        fprintf(gDumpFile, "\n\nBuffer in Host order (post Swap)\n");
    else
        fprintf(gDumpFile, "\n\nBuffer in Network order (post Swap)\n");
    DumpBuf(message->data, bufSize);
    fflush(stdout);
#endif
}

#endif
