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
 * @header CSrvrMessaging
 */

#include "CSrvrMessaging.h"
#include "PrivateTypes.h"
#include "DirServicesTypes.h"
#include "DSUtils.h"
#include "CLog.h"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>		// for offsetof()
#include <unistd.h>		// for sleep()


//------------------------------------------------------------------------------------
//	* CSrvrMessaging
//------------------------------------------------------------------------------------

CSrvrMessaging::CSrvrMessaging ( void )
{
} // CSrvrMessaging


//------------------------------------------------------------------------------------
//	* ~CSrvrMessaging
//------------------------------------------------------------------------------------

CSrvrMessaging::~CSrvrMessaging ( void )
{
} // ~CSrvrMessaging


//------------------------------------------------------------------------------------
//	* Add_tDataBuff_ToMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Add_tDataBuff_ToMsg ( sComData **inMsg, tDataBuffer *inBuff, eValueType inType )
{
	SInt32			result	= eDSNoErr;
	sObject		   *pObj	= nil;
	UInt32			offset	= 0;

	result = GetEmptyObj( (*inMsg), inType, &pObj );
	if ( result == eDSNoErr )
	{
		pObj->type		= inType; // == ktDataBuff
		pObj->count		= 1;
		if ( inBuff != nil )
		{
			pObj->used		= inBuff->fBufferLength;
			pObj->length	= inBuff->fBufferSize;
			offset			= pObj->offset;

			Grow( inMsg, offset, inBuff->fBufferSize );
			//can't use pObj after the Grow call since pObj and inMsg pointers may have changed

			::memcpy( (char *)(*inMsg) + offset, inBuff->fBufferData, inBuff->fBufferSize );
			(*inMsg)->fDataLength += inBuff->fBufferSize;
		}
		else
		{
			pObj->length = 0;
		}
	}

	return( result );

} // Add_tDataBuff_ToMsg


//------------------------------------------------------------------------------------
//	* Add_tDataList_ToMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Add_tDataList_ToMsg ( sComData **inMsg, tDataList *inList, eValueType inType )
{
	SInt32				result		= eDSNoErr;
	bool				done		= false;
	UInt32				offset		= 0;
	UInt32				length		= 0;
	UInt32				len			= 0;
	tDataNodePtr		pCurrNode	= nil;
	tDataBufferPriv	   *pPrivData	= nil;
	sObject			   *pObj		= nil;

	if ( ((*inMsg) != nil) && (inList != nil) )
	{
		result = GetEmptyObj( (*inMsg), inType, &pObj );
		if ( result == eDSNoErr )
		{
			pObj->type		= inType;  // == ktDataList
			pObj->count		= inList->fDataNodeCount;
			pObj->length	= ::dsGetDataLengthPriv( inList ) + (inList->fDataNodeCount * sizeof( UInt32 ));
			length			= pObj->length;
			offset			= pObj->offset;

			Grow( inMsg, offset, length );
			//can't use pObj after the Grow call since pObj and inMsg pointers may have changed

			pCurrNode = inList->fDataListHead;

			// Get the list total length
			while ( !done )
			{
				pPrivData = (tDataBufferPriv *)pCurrNode;
				len = pPrivData->fBufferLength;		// <- should use fBufferSize
				::memcpy( (char *)(*inMsg) + offset, &len, 4 );
				(*inMsg)->fDataLength += 4;
				offset += 4;

				::memcpy( (char *)(*inMsg) + offset, pPrivData->fBufferData, len );
				(*inMsg)->fDataLength += len;
				offset += len;

				if ( pPrivData->fNextPtr == nil )
				{
					done = true;
				}
				else
				{
					pCurrNode = pPrivData->fNextPtr;
				}
			}
		}
	}

	return( result );

} // Add_tDataList_ToMsg


//------------------------------------------------------------------------------------
//	* Add_Value_ToMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Add_Value_ToMsg ( sComData *inMsg, UInt32 inValue, eValueType inType )
{
	SInt32			result		= eDSNoErr;
	sObject		   *pObj		= nil;

	result = GetEmptyObj( inMsg, inType, &pObj );
	if ( result == eDSNoErr )
	{
		pObj->type		= inType;
		pObj->count		= inValue; //this is many times an SInt32 assignment of siResult to the UInt32 count
		pObj->length	= 0;
	}

	return( result );

} // Add_Value_ToMsg


//------------------------------------------------------------------------------------
//	* Add_tAttrEntry_ToMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Add_tAttrEntry_ToMsg ( sComData **inMsg, tAttributeEntry *inData )
{
	SInt32			result	= eDSNoErr;
	sObject		   *pObj	= nil;
	UInt32			offset	= 0;
	UInt32			length	= 0;

	result = GetEmptyObj( (*inMsg), ktAttrValueEntry, &pObj );
	if ( result == eDSNoErr )
	{
		pObj->type		= ktAttrEntry;
		pObj->count		= 1;

		if ( inData != nil )
		{
			pObj->length	= sizeof( tAttributeEntry ) + inData->fAttributeSignature.fBufferSize;
			length			= pObj->length;
			offset			= pObj->offset;

			Grow( inMsg, offset, length );
			//can't use pObj after the Grow call since pObj and inMsg pointers may have changed

			::memcpy( (char *)(*inMsg) + offset, inData, length );
			(*inMsg)->fDataLength += length;
		}
		else
		{
			pObj->length = 0;
		}
	}

	return( result );

} // Add_tAttrEntry_ToMsg


//------------------------------------------------------------------------------------
//	* Add_tAttrValueEntry_ToMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Add_tAttrValueEntry_ToMsg ( sComData **inMsg, tAttributeValueEntry *inData )
{
	SInt32			result	= eDSNoErr;
	sObject		   *pObj	= nil;
	UInt32			offset	= 0;
	UInt32			length	= 0;

	result = GetEmptyObj( (*inMsg), ktAttrValueEntry, &pObj );
	if ( result == eDSNoErr )
	{
		pObj->type	= ktAttrValueEntry;
		pObj->count	= 1;

		if ( inData != nil )
		{
			pObj->length	= sizeof( tAttributeValueEntry ) + inData->fAttributeValueData.fBufferSize;
			length			= pObj->length;
			offset			= pObj->offset;

			Grow( inMsg, offset, length );
			//can't use pObj after the Grow call since pObj and inMsg pointers may have changed

			::memcpy( (char *) (*inMsg) + offset, inData, length );
			(*inMsg)->fDataLength += length;
		}
		else
		{
			pObj->count		= 0;
			pObj->length	= 0;
		}
	}

	return( result );

} // Add_tAttrValueEntry_ToMsg


//------------------------------------------------------------------------------------
//	* Add_tRecordEntry_ToMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Add_tRecordEntry_ToMsg ( sComData **inMsg, tRecordEntry *inData )
{
	SInt32			result	= eDSNoErr;
	sObject		   *pObj	= nil;
	UInt32			offset	= 0;
	UInt32			length	= 0;

	result = GetEmptyObj( (*inMsg), ktRecordEntry, &pObj );
	if ( result == eDSNoErr )
	{
		pObj->type		= ktRecordEntry;
		pObj->count		= 1;
		if ( inData != nil )
		{
			pObj->length	= sizeof( tRecordEntry ) + inData->fRecordNameAndType.fBufferSize;
			length			= pObj->length;
			offset			= pObj->offset;

			Grow( inMsg, offset, length );
			//can't use pObj after the Grow call since pObj and inMsg pointers may have changed

			::memcpy( (char *)(*inMsg) + offset, inData, length );
			(*inMsg)->fDataLength += length;
		}
		else
		{
			pObj->length = 0;
		}
	}

	return( result );

} // Add_tRecordEntry_ToMsg


//------------------------------------------------------------------------------------
//	* Get_tDataBuff_FromMsg		ktDataBuff
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Get_tDataBuff_FromMsg ( sComData *inMsg, tDataBuffer **outBuff, eValueType inType )
{
	SInt32		result	= eDSNoErr;
	UInt32		offset	= 0;
	UInt32		length	= 0;
	sObject	   *pObj	= nil;

	result = GetThisObj( inMsg, inType, &pObj );
	if ( result == eDSNoErr )
	{
		offset = pObj->offset;
		length = pObj->length;

		if ( length >= 0 )
		{
			if ( *outBuff == nil )
			{
				*outBuff = ::dsDataBufferAllocatePriv( length );
			}

			if ( *outBuff != nil )
			{
				// Null out the buffer
				::memset( (*outBuff)->fBufferData, 0, (*outBuff)->fBufferSize );

				if ( (*outBuff)->fBufferSize >= pObj->length )
				{
					::memcpy( (*outBuff)->fBufferData, (char *)inMsg + offset, length );
					(*outBuff)->fBufferLength = pObj->used;
				}
				else
				{
					::memcpy( (*outBuff)->fBufferData, (char *)inMsg + offset, (*outBuff)->fBufferSize );
					(*outBuff)->fBufferLength = pObj->used;
					result = eDSBufferTooSmall;
				}
			}
			else
			{
				result = eDSNullDataBuff;
			}
		}
	}

	return( result );

} // Get_tDataBuff_FromMsg


//------------------------------------------------------------------------------------
//	* Get_tDataList_FromMsg		ktDataList
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Get_tDataList_FromMsg ( sComData *inMsg, tDataList **outList, eValueType inType )
{
	SInt32		siResult	= eDSNoErr;
	UInt32		offset		= 0;
	UInt32		length		= 0;
	UInt32		count		= 0;
	UInt32		cntr		= 0;
	sObject	   *pObj		= nil;
	tDataList  *pOutList	= nil;

	try
	{
		siResult = GetThisObj( inMsg, inType, &pObj );
		if ( siResult == eDSNoErr )
		{
			if ( outList != nil )
			{
				pOutList = ::dsDataListAllocatePriv();
				if ( pOutList != nil )
				{
					tDataBufferPriv    *pCurNodeData    = NULL;
					char               *pMsg            = (char *) inMsg;
					
					offset	= pObj->offset;
					count	= pObj->count;
					
					while ( cntr < count )
					{
						tDataBufferPriv    *pNewNodeData;
						
						::bcopy( pMsg + offset, &length, 4 );
						offset += 4;
						
						// if the buffer is more that 1 gig, something is wrong.
						if( length > 1024 * 1024 * 1024 )
						{
							siResult = eMemoryError;
							break;
						}
						
						pNewNodeData = (tDataBufferPriv *)::calloc( sizeof(tDataBufferPriv) + length, sizeof(char) );
						if ( pNewNodeData != nil )
						{
							pNewNodeData->fBufferSize = length;
							pNewNodeData->fBufferLength = length;
							
							bcopy( pMsg + offset, pNewNodeData->fBufferData, length );
							
							// Get the new node's header and point it to the prevous end
							pNewNodeData->fPrevPtr	= (tDataNodePtr) pCurNodeData;
							
							// Set the script code to ASCII
							pNewNodeData->fScriptCode = kASCIICodeScript;
						}
						else
						{
							siResult = eMemoryError;
							break;
						}
						
						// if we have a current node, means we are appending to the list
						if( pCurNodeData != nil )
							pCurNodeData->fNextPtr = (tDataNodePtr) pNewNodeData;
						else
							pOutList->fDataListHead = (tDataNodePtr) pNewNodeData;
						
						// increment offset
						offset += length;
						cntr++;
						
						pCurNodeData = pNewNodeData;
						
						pOutList->fDataNodeCount++;
					}
					*outList = pOutList;
				}
				else
				{
					siResult = eMemoryError;
				}
			}
			else
			{
				siResult = eDSNullDataList;
			}
		}
	}

	catch( SInt32 err )
	{
		DbgLog( 0x00FF, "***CSrvrMessaging::Get_tDataList_FromMsg with error %l", err ); 

		if ( pOutList != nil )
		{
			::dsDataListDeallocatePriv( pOutList );
			//need to free the header as well
			free( pOutList );
			pOutList = nil;
		}
		siResult = err;
	}

	return( siResult );

} // Get_tDataList_FromMsg


//------------------------------------------------------------------------------------
//	* Get_Value_FromMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Get_Value_FromMsg ( sComData *inMsg, UInt32 *outValue, eValueType inType )
{
	UInt32		result	= eDSNoErr;
	sObject	   *pObj	= nil;

	result = GetThisObj( inMsg, inType, &pObj );
	if ( result == eDSNoErr )
	{
		*outValue = pObj->count;
	}

	return( result );

} // Get_Value_FromMsg

//------------------------------------------------------------------------------------
//	* Get_tAttrEntry_FromMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Get_tAttrEntry_FromMsg ( sComData *inMsg, tAttributeEntry **outAttrEntry, eValueType inType )
{
	SInt32				result		= eDSNoErr;
	sObject			   *pObj		= nil;
	tAttributeEntry	   *pAttrEntry	= nil;

	result = GetThisObj( inMsg, inType, &pObj );
	if ( result == eDSNoErr )
	{
		pAttrEntry = (tAttributeEntry *)::calloc( 1, pObj->length );
		if ( pAttrEntry != nil )
		{
			// Fill the struct
			::memcpy( pAttrEntry, (char *)inMsg + pObj->offset, pObj->length );

			*outAttrEntry = pAttrEntry;
		}
		else
		{
			result = eDSNullAttribute;
		}
	}

	return( result );

} // Get_tAttrEntry_FromMsg



//------------------------------------------------------------------------------------
//	* Get_tAttrValueEntry_FromMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Get_tAttrValueEntry_FromMsg ( sComData				*inMsg,
												tAttributeValueEntry **outAttrValue,
												eValueType				inType )
{
	SInt32						result			= eDSNoErr;
	sObject					   *pObj			= nil;
	tAttributeValueEntry	   *pAttrValueEntry	= nil;

	result = GetThisObj( inMsg, inType, &pObj );
	if ( result == eDSNoErr )
	{
		pAttrValueEntry = (tAttributeValueEntry *)::calloc( 1, pObj->length );
		if ( pAttrValueEntry != nil )
		{
			// Fill the struct
			::memcpy( pAttrValueEntry, (char *)inMsg + pObj->offset, pObj->length );

			*outAttrValue = pAttrValueEntry;

		}
		else
		{
			result = eDSNullAttributeValue;
		}
	}

	return( result );

} // Get_tAttrValueEntry_FromMsg


//------------------------------------------------------------------------------------
//	* Get_tRecordEntry_FromMsg
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::Get_tRecordEntry_FromMsg ( sComData *inMsg, tRecordEntry **outRecEntry, eValueType inType )
{
	SInt32		   		result			= eDSNoErr;
	sObject		   	   *pObj			= nil;
	tRecordEntry	   *pRecordEntry	= nil;

	result = GetThisObj( inMsg, inType, &pObj );
	if ( result == eDSNoErr )
	{
		pRecordEntry = (tRecordEntry *)::calloc( 1, pObj->length );
		if ( pRecordEntry != nil )
		{
			// Fill the struct
			::memcpy( pRecordEntry, (char *)inMsg + pObj->offset, pObj->length );

			*outRecEntry = pRecordEntry;

		}
		else
		{
			result = eDSNullRecEntryPtr;
		}
	}

	return( result );

} // Get_tRecordEntry_FromMsg


//------------------------------------------------------------------------------------
//	* GetEmptyObj
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::GetEmptyObj ( sComData *inMsg, eValueType inType, sObject **outObj )
{
	SInt32		siResult	= eDSIndexNotFound;
	UInt32		i;

	for ( i = 0; i < 10; i++ )
	{
		if ( inMsg->obj[ i ].type == 0 )
		{
			*outObj = &inMsg->obj[ i ];
			if ( i == 0 )
			{
				//offset was incorrect ie. missing 2 more UInt32 fixed below ie. now 3*
				// but should really use offsetof here since it is a fixed size object
				(*outObj)->offset = offsetof(struct sComData, data);
				//(*outObj)->offset = sizeof( mach_msg_header_t ) +
				//					sizeof( mach_msg_type_t ) +
				//					3*sizeof( UInt32 ) +
				//				   (sizeof( sObject ) * 10);
			}
			else
			{
				(*outObj)->offset = inMsg->obj[ i - 1 ].offset + inMsg->obj[ i - 1 ].length;
			}
			siResult = eDSNoErr;
			break;
		}
		else if ( inMsg->obj[ i ].type == (UInt32)inType )
		{
			//siResult = kDupInList;
			break;
		}
	}

	return( siResult );

} // GetEmptyObj


//------------------------------------------------------------------------------------
//	* GetThisObj
//------------------------------------------------------------------------------------

SInt32 CSrvrMessaging::GetThisObj ( sComData *inMsg, eValueType inType, sObject **outObj )
{
	SInt32		siResult	= eDSIndexNotFound;
	UInt32		i;

	for ( i = 0; i < 10; i++ )
	{
		if ( inMsg->obj[ i ].type == (UInt32)inType )
		{
			*outObj = &inMsg->obj[ i ];
			siResult = eDSNoErr;
			break;
		}
	}

	return( siResult );

} // GetThisObj


//------------------------------------------------------------------------------------
//	* ClearDataBlock
//------------------------------------------------------------------------------------

void CSrvrMessaging::ClearDataBlock ( sComData *inMsg )
{
	if ( inMsg != nil )
	{
		::memset( inMsg->obj, 0, (sizeof( sObject ) * 10 ) );
		::memset( inMsg->data, 0, inMsg->fDataSize );
		inMsg->fDataLength	= 0;
	}
} // ClearDataBlock


//------------------------------------------------------------------------------------
//	* ClearMessageBlock
//------------------------------------------------------------------------------------

void CSrvrMessaging::ClearMessageBlock ( sComData *inMsg )
{
	if ( inMsg != nil )
	{
		::memset( inMsg->obj, 0, (sizeof( sObject ) * 10 ) );
	}
} // ClearMessageBlock

//------------------------------------------------------------------------------------
//	* Grow
//------------------------------------------------------------------------------------

void CSrvrMessaging::Grow ( sComData **inMsg, UInt32 inOffset, UInt32 inSize )
{
	UInt32			newSize		= 0;
	sComData	   *pNewPtr		= nil;

	// Is there anything to do
	if ( inSize == 0 )
	{
		return;
	}

	// Do we need a bigger object
	if ( ((*inMsg)->fDataLength + inSize) > (*inMsg)->fDataSize )
	{
		newSize = (*inMsg)->fDataSize;
		//idea is to make the newSize a multiple of kMsgBlockSize
		while( newSize < ((*inMsg)->fDataLength + inSize) )
		{
			newSize += kMsgBlockSize;
		}

		// Create the new pointer
		pNewPtr = (sComData *)::calloc( 1, sizeof( sComData ) + newSize );
		if ( pNewPtr == nil )
		{
			throw( (SInt32)eMemoryAllocError );
		}

		// Copy the old data to the new destination
		::memcpy( pNewPtr, (*inMsg), sizeof( sComData ) + (*inMsg)->fDataLength );

		// Dump the old data block
		::free( (*inMsg) );
		(*inMsg) = nil;

		// Assign the new data block
		(*inMsg) = (sComData *)pNewPtr;
		pNewPtr = nil;

		// Set the new data size
		(*inMsg)->fDataSize = newSize;
	}
} // Grow



