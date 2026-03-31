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
 * @header DSSwapUtils
 * Provides routines to byte swap DSProxy buffers.
 */
 
#include "DSSwapUtils.h"
#include <string.h>
#include <errno.h>			// system call error numbers
#include <unistd.h>			// for select call 
#include <stdlib.h>			// for calloc()
#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#include <Security/Authorization.h>
#include "CSharedData.h"
#include "SharedConsts.h"

UInt32 DSGetLong( void* ptr, eSwapDirection inSwapDir )
{
	UInt32	retval = *((UInt32 *) ptr);
	
	if ( inSwapDir == kDSSwapNetworkToHostOrder )
		retval = ntohl( retval );
	
	return retval;
}

UInt32 DSGetAndSwapLong( void* ptr, eSwapDirection inSwapDir )
{
	UInt32	retval = 0;
	UInt32	*value = (UInt32 *) ptr;
	
	switch ( inSwapDir )
	{
		case kDSSwapHostToNetworkOrder:
			retval = (*value);
			(*value) = htonl( retval );
			break;
		case kDSSwapNetworkToHostOrder:
			(*value) = retval = ntohl( *value );
			break;
	}
	
	return retval;
}

UInt16 DSGetAndSwapShort( void* ptr, eSwapDirection inSwapDir )
{
	UInt16	retval = 0;
	UInt16	*value = (UInt16 *) ptr;
	
	switch ( inSwapDir )
	{
		case kDSSwapHostToNetworkOrder:
			retval = (*value);
			(*value) = ntohs( retval );
			break;
		case kDSSwapNetworkToHostOrder:
			(*value) = retval = htons( *value );
			break;
	}
	
	return retval;
}

void DSSwapRecordEntry(char* data, UInt32 type, eSwapDirection inSwapDir)
{
	short i = 0;
	short j = 0;
	
    // if at any point we see data that doesn't make sense, we just bail
	bool bStandardA = (type != 'StdB' && type != 'DbgB');
    
    // start with initial length, then type and name strings (each with two byte length)
    UInt32 entryLen = DSGetAndSwapLong(data, inSwapDir);
    char* dataEnd = data + 4 + entryLen;
    data += 4;
    UInt16 tempLen = DSGetAndSwapShort(data, inSwapDir);
    data += 2 + tempLen;
    if (data > dataEnd) return;
    tempLen = DSGetAndSwapShort(data, inSwapDir);
    data += 2 + tempLen;
    if (data > dataEnd) return;
    
    // next is the attribute count (2 bytes)
    UInt16 attrCount = DSGetAndSwapShort(data, inSwapDir);
    data += 2;
    
    // go through each attribute entry
    for (i = 0; i < attrCount; i++)
    {
        UInt32 attrLen;
        if ( bStandardA )
        {
            // 4 byte attribute length
            attrLen = DSGetAndSwapLong(data, inSwapDir);
            data += 4;
        }
        else
        {
            // 2 byte attribute length
            attrLen = DSGetAndSwapShort(data, inSwapDir);
            data += 2;
        }
        
        char* attrEnd = data + attrLen;
        if (attrEnd > dataEnd) return;
        
        // next is attr type
        tempLen = DSGetAndSwapShort(data, inSwapDir);
        data += 2 + tempLen;
        if (data > attrEnd) return;
        
        // next is attr value count
        UInt16 attrValCount = DSGetAndSwapShort(data, inSwapDir);
        data += 2;

        // go through each attribute value
        for (j = 0; j < attrValCount; j++)
        {
            UInt32 attrValueLen;
			if ( bStandardA )
            {
                // 4 byte attribute length
                attrValueLen = DSGetAndSwapLong(data, inSwapDir);
                data += 4;
            }
            else
            {
                // 2 byte attribute length
                attrValueLen = DSGetAndSwapShort(data, inSwapDir);
                data += 2;
            }
            
            data += attrValueLen;
        }
    }
}

void DSSwapStandardBuf(char* data, UInt32 size, eSwapDirection inSwapDir)
{
    // check if this buffer is in one of the known formats
    // Must be at least 12 bytes big
    if (size < 12)
        return;
        
    UInt32 type = DSGetLong(data, inSwapDir);
    if ( (type == 'StdA') || (type == 'DbgA') || (type == 'StdB') || (type == 'DbgB') )
    {
        // these buffers contain an array of offsets at the beginning, with the record data
        // packed in reverse order at the end of the buffer        
        
        // swap the type
        DSSwapLong(data, inSwapDir);
        UInt32 recordCount = DSGetAndSwapLong(data + 4, inSwapDir);
        
        // now swap record entries
		UInt32 j = 0;
        for (j = 0; j < recordCount; j++)
        {
            UInt32 offset = DSGetAndSwapLong(data + (j * 4) + 8, inSwapDir);
            if (offset > size)	return; // bad buff, so bail
            DSSwapRecordEntry(data + offset, type, inSwapDir);
        }
        
        // swap the end tag
        DSSwapLong(data + (recordCount * 4) + 8, inSwapDir);
    }
    else if (type == 'npss')
    {
        // this is similar to the buffer format above, although for some reason the offsets are
        // in reverse order at the end, and the data is at the beginning
        
        // swap the type
        DSSwapLong(data, inSwapDir);
        UInt32 nodeCount = DSGetAndSwapLong(data + 4, inSwapDir);
        
		UInt32 i = 0;
        for (i = 0; i < nodeCount; i++)
        {
            UInt32 offset = DSGetAndSwapLong(data + size - (4 * i) - 4, inSwapDir);
            if (offset > size) return;
            char* tempPtr = data + offset;
            UInt16 numSegments = DSGetAndSwapShort(tempPtr, inSwapDir);
            tempPtr += 2;
            
			short j = 0;
            for (j = 0; j < numSegments; j++)
            {
                UInt16 segmentLen = DSGetAndSwapShort(tempPtr, inSwapDir);
                tempPtr += 2 + segmentLen;
                if (tempPtr - data > (SInt32)size) return;
            }
        }
    }
}

void DSSwapObjectData(	UInt32 type,
						char* data,
						UInt32 size,
						bool swapAuth,
						bool isCustomCall,
						UInt32 inCustomRequestNum,
						const char* inPluginName,
						bool isAPICallResponse,
						eSwapDirection inSwapDir)
{
    // first swap the contents of certain object types
    switch (type)
    {
        // all of these types are really just data buffers
        case ktDataBuff:
        {
			if (isCustomCall && (inPluginName != nil))
			{
				if (strcmp(inPluginName,"Configure") == 0)
				{
					if (inCustomRequestNum == eDSCustomCallConfigureGetAuthRef)
					{
						if (!isAPICallResponse)
						{
							UInt32 totalLen = 0;
							while (totalLen < size)
							{
								UInt32 length = DSGetAndSwapLong(data, inSwapDir);
								totalLen += length + 4;
								data += length + 4;
							}
							break;
						}
					}
					else if ( inCustomRequestNum == eDSCustomCallConfigureCheckVersion)
					{
						if (isAPICallResponse)
						{
							UInt32 totalLen = 0;
							while (totalLen < size)
							{
								UInt32 length = DSGetAndSwapShort(data, inSwapDir);
								totalLen += length + 2;
								data += length + 2;
							}
							break;
						}
					}
					else if ( inCustomRequestNum == eDSCustomCallConfigureSCGetKeyPathValueSize )
					{
						if (isAPICallResponse)
						{
							DSSwapLong(data, inSwapDir);
						}
					}
					else if ( inCustomRequestNum == eDSCustomCallConfigureSCGetKeyValueSize )
					{
						if (isAPICallResponse)
						{
							DSSwapLong(data, inSwapDir);
						}
					}
				}
				else if (strcmp(inPluginName,"Search") == 0)
				{
					if ( inCustomRequestNum == eDSCustomCallSearchReadDHCPLDAPSize )
					{
						if (isAPICallResponse)
						{
							DSSwapLong(data, inSwapDir);
						}
					}
				}
				else if (strcmp(inPluginName,"LDAPv3") == 0)
				{
					if (inCustomRequestNum == eDSCustomCallLDAPv3WriteServerMappings)
					{
						if (!isAPICallResponse)
						{
							UInt32 totalLen = 0;
							while (totalLen < size)
							{
								UInt32 length = DSGetAndSwapLong(data, inSwapDir);
								totalLen += length + 4;
								data += length + 4;
							}
							break;
						}
					}
					else if ( inCustomRequestNum == eDSCustomCallLDAPv3ReadConfigSize )
					{
						if (isAPICallResponse)
						{
							DSSwapLong(data, inSwapDir);
						}
					}
					else if ( inCustomRequestNum == 1003 ) // eRecordDeleteAndCredentials
					{
						if (!isAPICallResponse)
						{
							DSSwapLong(data, inSwapDir);
						}
					}
				}
				else if (strcmp(inPluginName,"SMB") == 0)
				{
					if ( inCustomRequestNum == 'xmls' )
					{
						if (isAPICallResponse)
						{
							DSSwapLong(data, inSwapDir);
						}
					}
					else if (inCustomRequestNum == 'read' )
					{
						if (isAPICallResponse)
						{
							UInt32 totalLen = 0;
							while (totalLen < size)
							{
								UInt32 length = DSGetAndSwapLong(data, inSwapDir);
								totalLen += length + 4;
								data += length + 4;
							}
							break;
						}
					}
					else if (inCustomRequestNum == 'writ' )
					{
						if (isAPICallResponse)
						{
							data += sizeof( AuthorizationExternalForm );
							UInt32 totalLen = 0;
							while (totalLen < size)
							{
								UInt32 length = DSGetAndSwapLong(data, inSwapDir);
								totalLen += length + 4;
								data += length + 4;
							}
							break;
						}
					}
				}
			}
			else
			{
				DSSwapStandardBuf(data, size, inSwapDir);
			}
        }
        case kAttrMatch:
        case kAttrType:
        case kAuthResponseBuff:
        case kAuthMethod:
        case kAttrTypeBuff:
        case kAttrValueBuff:
        case kFirstAttrBuff:
        case kNewAttrBuff:
        case kRecNameBuff:
        case kRecTypeBuff:
            break;
        case kAuthStepBuff:
		case kAuthStepDataResponse:
        {
            if (!swapAuth)
            {
                // two way random calls just have a blob of data,
                // no swapping required
                break;
            }

            // every call of every other type of auth has its buffer packed just like
            // a tDataList, so just fall through to the case below (no break).
        }
        // all of these types are data lists
        case ktDataList:
        case kDirNodeName:
        case kAttrTypeRequestList:
        case kRecTypeList:
        case kAttrTypeList:
        case kRecNameList:
        case kNodeInfoTypeList:
        case kNodeNamePatt:
        case kAttrMatches:
        case kAttrValueList:
        {
            UInt32 totalLen = 0;
            while (totalLen < size)
            {
                UInt32 length = DSGetAndSwapLong(data, inSwapDir);
                totalLen += length + 4;
                data += length + 4;
            }
        
            break;
        }
        case ktAttrEntry:
        {
            tAttributeEntry* entry = (tAttributeEntry*)data;
            DSSwapLong(&entry->fAttributeValueCount, inSwapDir);
            DSSwapLong(&entry->fAttributeDataSize, inSwapDir);
            DSSwapLong(&entry->fAttributeValueMaxSize, inSwapDir);
            DSSwapLong(&entry->fAttributeSignature.fBufferSize, inSwapDir);
            DSSwapLong(&entry->fAttributeSignature.fBufferLength, inSwapDir);
            break;
        }
        case ktAttrValueEntry:
        {
            tAttributeValueEntry* entry = (tAttributeValueEntry*)data;
            DSSwapLong(&entry->fAttributeValueID, inSwapDir);
            DSSwapLong(&entry->fAttributeValueData.fBufferSize, inSwapDir);
            DSSwapLong(&entry->fAttributeValueData.fBufferLength, inSwapDir);
            break;
        }
        case ktRecordEntry:
        {
            short tempLen;
            tRecordEntry* entry = (tRecordEntry*)data;
            DSSwapLong(&entry->fRecordAttributeCount, inSwapDir);
            DSSwapLong(&entry->fRecordNameAndType.fBufferSize, inSwapDir);
            DSSwapLong(&entry->fRecordNameAndType.fBufferLength, inSwapDir);

            // fRecordNameAndType has some embedded lengths
            char* ptr = (char*)&entry->fRecordNameAndType.fBufferData[0];
            tempLen = DSGetAndSwapShort(ptr, inSwapDir);
            ptr += tempLen + 2;
            DSGetAndSwapShort(ptr, inSwapDir);
            break;
        }
        
        default:
            // everything else should be a simple value with a 0 length
            if (size != 0)
            {
                syslog( LOG_INFO, "*** DS Error in: %s at: %d: Unexpected pObj\n of type %d", __FILE__, __LINE__, type );
            }
            break;
    }
}
