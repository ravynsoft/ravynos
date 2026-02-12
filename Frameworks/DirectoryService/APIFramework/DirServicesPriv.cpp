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
 * @header DirServicesPriv
 */

// app
#include "DirServicesPriv.h"
#include "DirServicesUtils.h"
#include "DirServicesTypesPriv.h"
#include "PrivateTypes.h"
#include "CDSRefMap.h"
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "CRCCalc.h"

extern pid_t		gProcessPID;
extern CDSRefMap	gFWRefMap;
extern CDSRefTable	gFWRefTable;

static const	UInt32	kFWBuffPad	= 16;

//--------------------------------------------------------------------------------------------------
//
//	Name:	VerifyTDataBuff
//
//--------------------------------------------------------------------------------------------------

tDirStatus VerifyTDataBuff ( tDataBuffer *inBuff, tDirStatus inNullErr, tDirStatus inEmptyErr )
{
	if ( inBuff == nil )
	{
		return( inNullErr );
	}
	if ( inBuff->fBufferSize == 0 )
	{
		return( inEmptyErr );
	}

	return( eDSNoErr );

} // VerifyTDataBuff


//--------------------------------------------------------------------------------------------------
//
//	Name:	VerifyTNodeList
//
//--------------------------------------------------------------------------------------------------

tDirStatus VerifyTNodeList ( tDataList *inDataList, tDirStatus inNullErr, tDirStatus inEmptyErr )
{
	if ( inDataList == nil )
	{
		return( inNullErr );
	}
	if ( inDataList->fDataNodeCount == 0 )
	{
		return( inEmptyErr );
	}
	if ( dsGetDataLength( inDataList ) == 0 )
	{
		return( inEmptyErr );
	}

	return( eDSNoErr );

} // VerifyTNodeList


//------------------------------------------------------------------------------------
//	Name: IsStdBuffer
//------------------------------------------------------------------------------------

tDirStatus IsStdBuffer ( tDataBufferPtr inOutDataBuff )

{
	tDirStatus	outResult	= eDSNoErr;
	SInt32		siResult	= eDSNoErr;
	CBuff		inBuff;
    UInt32		bufTag		= 0;

    //check to determine whether we employ client side buffer parsing
    //ie. look for 'StdA' OR 'StdB' tags

	try
	{
		if ( inOutDataBuff == nil ) throw( (SInt32)eDSEmptyBuffer );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		siResult = inBuff.Initialize( inOutDataBuff );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = inBuff.GetBuffType( &bufTag );
		if ( siResult != eDSNoErr ) throw( siResult );
        
        if ( (bufTag != 'StdA') && (bufTag != 'StdB') && (bufTag != 'DbgA') && (bufTag != 'DbgB') )  
		//KW should make 'StdB' and 'StdA" readily available constants but not 'DbgA' and 'DbgB'
        {
            outResult = eDSInvalidTag;
        }

    }
    
	catch( SInt32 err )
	{
		outResult = (tDirStatus)err;
	}

	return( outResult );
    
} // IsStdBuffer

//------------------------------------------------------------------------------------
//	Name: IsFWReference
//------------------------------------------------------------------------------------

tDirStatus IsFWReference ( UInt32 inRef )

{
	tDirStatus	outResult	= eDSInvalidReference;
	
    //check to determine whether this is a FW reference
	//ie. 0x0030000 bits are set

	if ((inRef & 0x00300000) != 0)
	{
		outResult = eDSNoErr;
	}

	return( outResult );
    
} // IsFWReference

//------------------------------------------------------------------------------------
//	Name: ExtractRecordEntry
//------------------------------------------------------------------------------------

tDirStatus ExtractRecordEntry ( tDataBufferPtr		inOutDataBuff,
								UInt32				inRecordEntryIndex,
								tAttributeListRef	*outAttributeListRef,
								tRecordEntryPtr		*outRecEntryPtr )

{
	SInt32					siResult		= eDSNoErr;
	UInt32					uiIndex			= 0;
	UInt32					uiCount			= 0;
	UInt32					uiOffset		= 0;
	UInt32					uberOffset		= 0;
	char 				   *pData			= nil;
	tRecordEntryPtr			pRecEntry		= nil;
	CBuff					inBuff;
	UInt32					offset			= 0;
	UInt16					usTypeLen		= 0;
	char				   *pRecType		= nil;
	UInt16					usNameLen		= 0;
	char				   *pRecName		= nil;
	UInt16					usAttrCnt		= 0;
	UInt32					buffLen			= 0;
    UInt32					bufTag			= 0;

	try
	{
		if ( inOutDataBuff == nil ) throw( (SInt32)eDSNullDataBuff );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		siResult = inBuff.Initialize( inOutDataBuff );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = inBuff.GetBuffType( &bufTag );
		if ( siResult != eDSNoErr ) throw( siResult );
        
		siResult = inBuff.GetDataBlockCount( &uiCount );
		if ( siResult != eDSNoErr ) throw( siResult );

		uiIndex = inRecordEntryIndex;
		if ( uiIndex == 0 ) throw( (SInt32)eDSInvalidIndex );
		
		if ( uiIndex > uiCount ) throw( (SInt32)eDSIndexOutOfRange );

		pData = inBuff.GetDataBlock( uiIndex, &uberOffset );
		if ( pData == nil ) throw( (SInt32)eDSCorruptBuffer );

		//assume that the length retrieved is valid
		buffLen = inBuff.GetDataBlockLength( uiIndex );
		
		// Skip past this same record length obtained from GetDataBlockLength
		pData	+= 4;
		offset	= 0; //buffLen does not include first four bytes

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the length for the record type
		::memcpy( &usTypeLen, pData, 2 );
		
		pData	+= 2;
		offset	+= 2;

		pRecType = pData;
		
		pData	+= usTypeLen;
		offset	+= usTypeLen;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the length for the record name
		::memcpy( &usNameLen, pData, 2 );
		
		pData	+= 2;
		offset	+= 2;

		pRecName = pData;
		
		pData	+= usNameLen;
		offset	+= usNameLen;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the attribute count
		::memcpy( &usAttrCnt, pData, 2 );

		pRecEntry = (tRecordEntry *)::calloc( 1, sizeof( tRecordEntry ) + usNameLen + usTypeLen + 4 + kFWBuffPad );

		pRecEntry->fRecordNameAndType.fBufferSize	= usNameLen + usTypeLen + 4 + kFWBuffPad;
		pRecEntry->fRecordNameAndType.fBufferLength	= usNameLen + usTypeLen + 4;

		// Add the record name length
		::memcpy( pRecEntry->fRecordNameAndType.fBufferData, &usNameLen, 2 );
		uiOffset += 2;

		// Add the record name
		::memcpy( pRecEntry->fRecordNameAndType.fBufferData + uiOffset, pRecName, usNameLen );
		uiOffset += usNameLen;

		// Add the record type length
		::memcpy( pRecEntry->fRecordNameAndType.fBufferData + uiOffset, &usTypeLen, 2 );

		// Add the record type
		uiOffset += 2;
		::memcpy( pRecEntry->fRecordNameAndType.fBufferData + uiOffset, pRecType, usTypeLen );

		pRecEntry->fRecordAttributeCount = usAttrCnt;

        //create a reference here
        siResult = gFWRefTable.NewAttrListRef( outAttributeListRef, 0, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		
		if ( (bufTag == 'DbgA') || (bufTag == 'DbgB') )
		{
			syslog(LOG_CRIT, "DS:dsGetRecordEntry:ExtractRecordEntry:gFWRefTable.NewAttrListRef ref = %d", *outAttributeListRef);
		}
		        
		//uberOffset + offset + 4;	// context used by next calls of GetAttributeEntry
									// include the four bytes of the buffLen
		siResult = gFWRefTable.SetOffset( *outAttributeListRef, eAttrListRefType, uberOffset + offset + 4, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		siResult = gFWRefTable.SetBufTag( *outAttributeListRef, eAttrListRefType, bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		*outRecEntryPtr = pRecEntry;
        pRecEntry = nil;
	}

	catch( SInt32 err )
	{
		siResult = err;
	}
    
    //clean up pRecEntry if throwing an error
    if (pRecEntry != nil)
    {
        dsDeallocRecordEntry(0,pRecEntry);
        pRecEntry = nil;
    }

	return( (tDirStatus)siResult );

} // ExtractRecordEntry


//------------------------------------------------------------------------------------
//	Name: ExtractAttributeEntry
//------------------------------------------------------------------------------------

tDirStatus ExtractAttributeEntry (	tDataBufferPtr			inOutDataBuff,
									tAttributeListRef		inAttrListRef,
									UInt32					inAttrInfoIndex,
									tAttributeValueListRef	*outAttrValueListRef,
									tAttributeEntryPtr		*outAttrInfoPtr )
{
	SInt32					siResult			= eDSNoErr;
	UInt16					usAttrTypeLen		= 0;
	UInt16					usAttrCnt			= 0;
	UInt16					usAttrLen16			= 0;
	UInt32					usAttrLen			= 0;
	UInt16					usValueCnt			= 0;
	UInt16					usValueLen16		= 0;
	UInt32					usValueLen			= 0;
	UInt32					i					= 0;
	UInt32					uiIndex				= 0;
	UInt32					uiAttrEntrySize		= 0;
	UInt32					uiOffset			= 0;
	UInt32					uiTotalValueSize	= 0;
	UInt32					offset				= 0;
	UInt32					buffSize			= 0;
	UInt32					buffLen				= 0;
	char				   *p			   		= nil;
	char				   *pAttrType	   		= nil;
	tAttributeEntryPtr		pAttribInfo			= nil;
	UInt32					attrListOffset		= 0;
	UInt32					bufTag				= 0;

	try
	{
	
		siResult = gFWRefTable.GetOffset( inAttrListRef, eAttrListRefType, &attrListOffset, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = gFWRefTable.GetBufTag( inAttrListRef, eAttrListRefType, &bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		uiIndex = inAttrInfoIndex;
		if (uiIndex == 0) throw( (SInt32)eDSInvalidIndex );
				
		if ( inOutDataBuff == nil ) throw( (SInt32)eDSNullDataBuff );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		buffSize	= inOutDataBuff->fBufferSize;
		//here we can't use fBufferLength for the buffLen SINCE the buffer is packed at the END of the data block
		//and the fBufferLength is the overall length of the data for all blocks at the end of the data block
		//the value ALSO includes the bookkeeping data at the start of the data block
		//so we need to read it here

		p		= inOutDataBuff->fBufferData + attrListOffset;
		offset	= attrListOffset;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
				
		// Get the attribute count
		::memcpy( &usAttrCnt, p, 2 );
		if (uiIndex > usAttrCnt) throw( (SInt32)eDSIndexOutOfRange );

		// Move 2 bytes
		p		+= 2;
		offset	+= 2;

		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			// Skip to the attribute that we want
			for ( i = 1; i < uiIndex; i++ )
			{
				// Do record check, verify that offset is not past end of buffer, etc.
				if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
			
				// Get the length for the attribute
				::memcpy( &usAttrLen16, p, 2 );
	
				// Move the offset past the length word and the length of the data
				p		+= 2 + usAttrLen16;
				offset	+= 2 + usAttrLen16;
			}
	
			// Get the attribute offset
			uiOffset = offset;
	
			// Do record check, verify that offset is not past end of buffer, etc.
			if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
			
			// Get the length for the attribute block
			::memcpy( &usAttrLen16, p, 2 );
	
			// Skip past the attribute length
			p		+= 2;
			offset	+= 2;
			
			usAttrLen = (UInt32)usAttrLen16;
		}
		else
		{
			// Skip to the attribute that we want
			for ( i = 1; i < uiIndex; i++ )
			{
				// Do record check, verify that offset is not past end of buffer, etc.
				if (4 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
			
				// Get the length for the attribute
				::memcpy( &usAttrLen, p, 4 );
	
				// Move the offset past the length word and the length of the data
				p		+= 4 + usAttrLen;
				offset	+= 4 + usAttrLen;
			}
	
			// Get the attribute offset
			uiOffset = offset;
	
			// Do record check, verify that offset is not past end of buffer, etc.
			if (4 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
			
			// Get the length for the attribute block
			::memcpy( &usAttrLen, p, 4 );
	
			// Skip past the attribute length
			p		+= 4;
			offset	+= 4;
		}

		//set the bufLen to stricter range
		buffLen = offset + usAttrLen;
		if ( buffLen > buffSize ) throw ( (SInt32)eDSInvalidBuffFormat );

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the length for the attribute type
		::memcpy( &usAttrTypeLen, p, 2 );
		
		pAttrType = p + 2;
		p		+= 2 + usAttrTypeLen;
		offset	+= 2 + usAttrTypeLen;
		
		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get number of values for this attribute
		::memcpy( &usValueCnt, p, 2 );
		
		p		+= 2;
		offset	+= 2;
			
		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			for ( i = 0; i < usValueCnt; i++ )
			{
				// Do record check, verify that offset is not past end of buffer, etc.
				if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
				// Get the length for the value
				::memcpy( &usValueLen16, p, 2 );
				
				p		+= 2 + usValueLen16;
				offset	+= 2 + usValueLen16;
				
				uiTotalValueSize += usValueLen16;
			}
		}
		else
		{
			for ( i = 0; i < usValueCnt; i++ )
			{
				// Do record check, verify that offset is not past end of buffer, etc.
				if (4 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
				// Get the length for the value
				::memcpy( &usValueLen, p, 4 );
				
				p		+= 4 + usValueLen;
				offset	+= 4 + usValueLen;
				
				uiTotalValueSize += usValueLen;
			}
		}

		uiAttrEntrySize = sizeof( tAttributeEntry ) + usAttrTypeLen + kFWBuffPad;
		pAttribInfo = (tAttributeEntry *)::calloc( 1, uiAttrEntrySize );

		pAttribInfo->fAttributeValueCount				= usValueCnt;
		pAttribInfo->fAttributeDataSize					= uiTotalValueSize;
		pAttribInfo->fAttributeValueMaxSize				= 512;				// KW this is not used anywhere
		pAttribInfo->fAttributeSignature.fBufferSize	= usAttrTypeLen + kFWBuffPad;
		pAttribInfo->fAttributeSignature.fBufferLength	= usAttrTypeLen;
		::memcpy( pAttribInfo->fAttributeSignature.fBufferData, pAttrType, usAttrTypeLen );

        //create a reference here
        siResult = gFWRefTable.NewAttrValueRef( outAttrValueListRef, 0, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		
		if ( (bufTag == 'DbgA') || (bufTag == 'DbgB') )
		{
			syslog(LOG_CRIT, "DS:dsGetAttributeEntry:ExtractAttributeEntry:gFWRefTable.NewAttrValueRef ref = %d", *outAttrValueListRef);
		}
		        
		siResult = gFWRefTable.SetOffset( *outAttrValueListRef, eAttrValueListRefType, uiOffset, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		siResult = gFWRefTable.SetBufTag( *outAttrValueListRef, eAttrValueListRefType, bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		*outAttrInfoPtr = pAttribInfo;
		pAttribInfo = nil;
	}

	catch( SInt32 err )
	{
		siResult = err;
	}

    //clean up pRecEntry if throwing an error
    if (pAttribInfo != nil)
    {
        dsDeallocAttributeEntry(0,pAttribInfo);
        pAttribInfo = nil;
    }

	return( (tDirStatus)siResult );

} // ExtractAttributeEntry


//------------------------------------------------------------------------------------
//	Name: ExtractAttributeValue
//------------------------------------------------------------------------------------

tDirStatus ExtractAttributeValue (	tDataBufferPtr			 inOutDataBuff,
									tAttributeValueListRef	 inAttrValueListRef,
									UInt32					 inAttrValueIndex,
									tAttributeValueEntryPtr	*outAttrValue )
{
	SInt32						siResult		= eDSNoErr;
	UInt16						usValueCnt		= 0;
	UInt16						usValueLen16	= 0;
	UInt32						usValueLen		= 0;
	UInt16						usAttrNameLen	= 0;
	UInt32						i				= 0;
	UInt32						uiIndex			= 0;
	UInt32						offset			= 0;
	char					   *p				= nil;
	tAttributeValueEntry	   *pAttrValue		= nil;
	UInt32						buffSize		= 0;
	UInt32						buffLen			= 0;
	UInt16						attrLen16		= 0;
	UInt32						attrLen			= 0;
	UInt32						attrValueOffset	= 0;
	UInt32						bufTag			= 0;

	try
	{
		siResult = gFWRefTable.GetOffset( inAttrValueListRef, eAttrValueListRefType, &attrValueOffset, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = gFWRefTable.GetBufTag( inAttrValueListRef, eAttrValueListRefType, &bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		uiIndex = inAttrValueIndex;
		if (uiIndex == 0) throw( (SInt32)eDSInvalidIndex );

		if ( inOutDataBuff == nil ) throw( (SInt32)eDSNullDataBuff );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		buffSize	= inOutDataBuff->fBufferSize;
		//here we can't use fBufferLength for the buffLen SINCE the buffer is packed at the END of the data block
		//and the fBufferLength is the overall length of the data for all blocks at the end of the data block
		//the value ALSO includes the bookkeeping data at the start of the data block
		//so we need to read it here

		p		= inOutDataBuff->fBufferData + attrValueOffset;
		offset	= attrValueOffset;

		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
					
			// Get the buffer length
			::memcpy( &attrLen16, p, 2 );
	
			//now add the offset to the attr length for the value of buffLen to be used to check for buffer overruns
			//AND add the length of the buffer length var as stored ie. 2 bytes
			buffLen		= attrLen16 + attrValueOffset + 2;
			if (buffLen > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
	
			// Skip past the attribute length
			p		+= 2;
			offset	+= 2;
		}
		else
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (4 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
					
			// Get the buffer length
			::memcpy( &attrLen, p, 4 );
	
			//now add the offset to the attr length for the value of buffLen to be used to check for buffer overruns
			//AND add the length of the buffer length var as stored ie. 4 bytes
			buffLen		= attrLen + attrValueOffset + 4;
			if (buffLen > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
	
			// Skip past the attribute length
			p		+= 4;
			offset	+= 4;
		}

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the attribute name length
		::memcpy( &usAttrNameLen, p, 2 );
		
		p		+= 2 + usAttrNameLen;
		offset	+= 2 + usAttrNameLen;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the value count
		::memcpy( &usValueCnt, p, 2 );
		
		p		+= 2;
		offset	+= 2;

		if (uiIndex > usValueCnt)  throw( (SInt32)eDSIndexOutOfRange );

		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			// Skip to the value that we want
			for ( i = 1; i < uiIndex; i++ )
			{
				// Do record check, verify that offset is not past end of buffer, etc.
				if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
				// Get the length for the value
				::memcpy( &usValueLen16, p, 2 );
				
				p		+= 2 + usValueLen16;
				offset	+= 2 + usValueLen16;
			}
	
			// Do record check, verify that offset is not past end of buffer, etc.
			if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
			::memcpy( &usValueLen16, p, 2 );
			
			p		+= 2;
			offset	+= 2;
			
			usValueLen = (UInt32)usValueLen16;
		}
		else
		{
			// Skip to the value that we want
			for ( i = 1; i < uiIndex; i++ )
			{
				// Do record check, verify that offset is not past end of buffer, etc.
				if (4 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
				// Get the length for the value
				::memcpy( &usValueLen, p, 4 );
				
				p		+= 4 + usValueLen;
				offset	+= 4 + usValueLen;
			}
	
			// Do record check, verify that offset is not past end of buffer, etc.
			if (4 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
			::memcpy( &usValueLen, p, 4 );
			
			p		+= 4;
			offset	+= 4;
		}

		//if (usValueLen == 0)  throw( (SInt32)eDSInvalidBuffFormat ); //if zero is it okay?

		pAttrValue = (tAttributeValueEntry *)::calloc( 1, sizeof( tAttributeValueEntry ) + usValueLen + kFWBuffPad );

		pAttrValue->fAttributeValueData.fBufferSize		= usValueLen + kFWBuffPad;
		pAttrValue->fAttributeValueData.fBufferLength	= usValueLen;
		
		// Do record check, verify that offset is not past end of buffer, etc.
		if ( usValueLen + offset > buffLen ) throw( (SInt32)eDSInvalidBuffFormat );
		
		::memcpy( pAttrValue->fAttributeValueData.fBufferData, p, usValueLen );

		// Set the attribute value ID
		pAttrValue->fAttributeValueID = CalcCRCWithLength( pAttrValue->fAttributeValueData.fBufferData, usValueLen );

		*outAttrValue = pAttrValue;
		pAttrValue = nil;
	}

	catch( SInt32 err )
	{
		siResult = err;
	}

    //clean up pRecEntry if throwing an error
    if (pAttrValue != nil)
    {
        dsDeallocAttributeValueEntry(0,pAttrValue);
        pAttrValue = nil;
    }

	return( (tDirStatus)siResult );

} // ExtractAttributeValue

//------------------------------------------------------------------------------------
//	Name: ExtractNextAttributeEntry
//------------------------------------------------------------------------------------

tDirStatus ExtractNextAttributeEntry (	tDataBufferPtr				inOutDataBuff,
										tAttributeListRef			inAttrListRef,
										UInt32						inAttrInfoIndex,
										SInt32					   *inOutOffset,
										tAttributeValueListRef	   *outAttrValueListRef,
										tAttributeEntryPtr		   *outAttrInfoPtr )
{
	SInt32					siResult			= eDSNoErr;
	UInt16					usAttrTypeLen		= 0;
	UInt16					usAttrCnt			= 0;
	UInt16					usAttrLen16			= 0;
	UInt32					usAttrLen			= 0;
	UInt16					usValueCnt			= 0;
	UInt32					uiIndex				= 0;
	UInt32					uiAttrEntrySize		= 0;
	UInt32					uiOffset			= 0;
	UInt32					uiTotalValueSize	= 0;
	UInt32					offset				= 0;
	UInt32					buffSize			= 0;
	UInt32					buffLen				= 0;
	char				   *p			   		= nil;
	char				   *pAttrType	   		= nil;
	tAttributeEntryPtr		pAttribInfo			= nil;
	UInt32					attrListOffset		= 0;
	UInt32					bufTag				= 0;

	try
	{
		siResult = gFWRefTable.GetOffset( inAttrListRef, eAttrListRefType, &attrListOffset, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = gFWRefTable.GetBufTag( inAttrListRef, eAttrListRefType, &bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		uiIndex = inAttrInfoIndex;
		if (uiIndex == 0) throw( (SInt32)eDSInvalidIndex );
				
		if ( inOutDataBuff == nil ) throw( (SInt32)eDSNullDataBuff );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		buffSize	= inOutDataBuff->fBufferSize;
		//here we can't use fBufferLength for the buffLen SINCE the buffer is packed at the END of the data block
		//and the fBufferLength is the overall length of the data for all blocks at the end of the data block
		//the value ALSO includes the bookkeeping data at the start of the data block
		//so we need to read it here

		p		= inOutDataBuff->fBufferData + attrListOffset;
		offset	= attrListOffset;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
				
		// Get the attribute count
		::memcpy( &usAttrCnt, p, 2 );
		if (uiIndex > usAttrCnt) throw( (SInt32)eDSIndexOutOfRange );

		// Move 2 bytes
		p		+= 2;
		offset	+= 2;
		
		//add the inOutOffset which should place us at the correct attribute
		p		+= *inOutOffset;
		offset	+= *inOutOffset;
		//set the offset to the length of attribute value block for ExtractNextAttributeValue
		uiOffset = offset;

		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
			
			// Get the length for the attribute block
			::memcpy( &usAttrLen16, p, 2 );
	
			// Skip past the attribute length
			p		+= 2;
			offset	+= 2;
			
			usAttrLen = (UInt32)usAttrLen16;
		}
		else
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (4 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
			
			// Get the length for the attribute block
			::memcpy( &usAttrLen, p, 4 );
	
			// Skip past the attribute length
			p		+= 4;
			offset	+= 4;
		}

		//set the bufLen to stricter range
		buffLen = offset + usAttrLen;
		
		//now start calculating the attr value(s) length instead of iterating acrosss the values
		uiTotalValueSize += usAttrLen;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the length for the attribute type
		::memcpy( &usAttrTypeLen, p, 2 );
		
		pAttrType = p + 2;
		p		+= 2 + usAttrTypeLen;
		offset	+= 2 + usAttrTypeLen;
		uiTotalValueSize -= 2;
		uiTotalValueSize -= usAttrTypeLen;
		
		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get number of values for this attribute
		::memcpy( &usValueCnt, p, 2 );
		
		p		+= 2;
		offset	+= 2;
		uiTotalValueSize -= 2;
		
		//decrement the total value length
		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			uiTotalValueSize -= (2 * usValueCnt);
		}
		else
		{
			uiTotalValueSize -= (4 * usValueCnt);
		}

		uiAttrEntrySize = sizeof( tAttributeEntry ) + usAttrTypeLen + kFWBuffPad;
		pAttribInfo = (tAttributeEntry *)::calloc( 1, uiAttrEntrySize );

		pAttribInfo->fAttributeValueCount				= usValueCnt;
		pAttribInfo->fAttributeDataSize					= uiTotalValueSize;	// KW this is likely never used by any client but it is the actual length of all the values
		pAttribInfo->fAttributeValueMaxSize				= 512;				// KW this is not used anywhere
		pAttribInfo->fAttributeSignature.fBufferSize	= usAttrTypeLen + kFWBuffPad;
		pAttribInfo->fAttributeSignature.fBufferLength	= usAttrTypeLen;
		::memcpy( pAttribInfo->fAttributeSignature.fBufferData, pAttrType, usAttrTypeLen );

        //create a reference here
        siResult = gFWRefTable.NewAttrValueRef( outAttrValueListRef, 0, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		
		if ( (bufTag == 'DbgA') || (bufTag == 'DbgB') )
		{
			syslog(LOG_CRIT, "DS:dsGetAttributeEntry:ExtractNextAttributeEntry:gFWRefTable.NewAttrValueRef ref = %d", *outAttrValueListRef);
		}
		        
		siResult = gFWRefTable.SetOffset( *outAttrValueListRef, eAttrValueListRefType, uiOffset, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		siResult = gFWRefTable.SetBufTag( *outAttrValueListRef, eAttrValueListRefType, bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		*outAttrInfoPtr = pAttribInfo;
		pAttribInfo = nil;

		//check if we have more attrs
		if ( (uiIndex + 1) > usAttrCnt )
		{
			*inOutOffset = -1;
		}
		else
		{
			*inOutOffset += usAttrLen;
			if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
			{
				*inOutOffset += 2;
			}
			else
			{
				*inOutOffset += 4;
			}
		}
	}

	catch( SInt32 err )
	{
		siResult = err;
	}

    //clean up pRecEntry if throwing an error
    if (pAttribInfo != nil)
    {
        dsDeallocAttributeEntry(0,pAttribInfo);
        pAttribInfo = nil;
    }

	return( (tDirStatus)siResult );

} // ExtractNextAttributeEntry


//------------------------------------------------------------------------------------
//	Name: ExtractNextAttributeValue
//------------------------------------------------------------------------------------

tDirStatus ExtractNextAttributeValue (	tDataBufferPtr				inOutDataBuff,
										tAttributeValueListRef		inAttrValueListRef,
										UInt32						inAttrValueIndex,
										SInt32					   *inOutOffset,
										tAttributeValueEntryPtr	   *outAttrValue )
{
	SInt32						siResult		= eDSNoErr;
	UInt16						usValueCnt		= 0;
	UInt16						usValueLen16	= 0;
	UInt32						usValueLen		= 0;
	UInt16						usAttrNameLen	= 0;
	UInt32						uiIndex			= 0;
	UInt32						offset			= 0;
	char					   *p				= nil;
	tAttributeValueEntry	   *pAttrValue		= nil;
	UInt32						buffSize		= 0;
	UInt32						buffLen			= 0;
	UInt16						attrLen16		= 0;
	UInt32						attrLen			= 0;
	UInt32						attrValueOffset	= 0;
	UInt32						bufTag			= 0;

	try
	{
		siResult = gFWRefTable.GetOffset( inAttrValueListRef, eAttrValueListRefType, &attrValueOffset, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = gFWRefTable.GetBufTag( inAttrValueListRef, eAttrValueListRefType, &bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );

		uiIndex = inAttrValueIndex;
		if (uiIndex == 0) throw( (SInt32)eDSInvalidIndex );

		if ( inOutDataBuff == nil ) throw( (SInt32)eDSNullDataBuff );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		buffSize	= inOutDataBuff->fBufferSize;
		//here we can't use fBufferLength for the buffLen SINCE the buffer is packed at the END of the data block
		//and the fBufferLength is the overall length of the data for all blocks at the end of the data block
		//the value ALSO includes the bookkeeping data at the start of the data block
		//so we need to read it here

		p		= inOutDataBuff->fBufferData + attrValueOffset;
		offset	= attrValueOffset;

		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
					
			// Get the buffer length
			::memcpy( &attrLen16, p, 2 );
	
			//now add the offset to the attr length for the value of buffLen to be used to check for buffer overruns
			//AND add the length of the buffer length var as stored ie. 2 bytes
			buffLen		= attrLen16 + attrValueOffset + 2;
			if (buffLen > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
	
			// Skip past the attribute length
			p		+= 2;
			offset	+= 2;
		}
		else
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (4 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
					
			// Get the buffer length
			::memcpy( &attrLen, p, 4 );
	
			//now add the offset to the attr length for the value of buffLen to be used to check for buffer overruns
			//AND add the length of the buffer length var as stored ie. 4 bytes
			buffLen		= attrLen + attrValueOffset + 4;
			if (buffLen > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
	
			// Skip past the attribute length
			p		+= 4;
			offset	+= 4;
		}

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the attribute name length
		::memcpy( &usAttrNameLen, p, 2 );
		
		p		+= 2 + usAttrNameLen;
		offset	+= 2 + usAttrNameLen;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the value count
		::memcpy( &usValueCnt, p, 2 );
		
		p		+= 2;
		offset	+= 2;

		if (uiIndex > usValueCnt)  throw( (SInt32)eDSIndexOutOfRange );
		
		//use the passed in offset
		offset	+= *inOutOffset;
		p		+= *inOutOffset;

		if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
			::memcpy( &usValueLen16, p, 2 );
			
			p		+= 2;
			offset	+= 2;
			
			usValueLen = (UInt32)usValueLen16;
		}
		else
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if (4 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
			
			::memcpy( &usValueLen, p, 4 );
			
			p		+= 4;
			offset	+= 4;
		}

		//if (usValueLen == 0)  throw( (SInt32)eDSInvalidBuffFormat ); //if zero is it okay?

		pAttrValue = (tAttributeValueEntry *)::calloc( 1, sizeof( tAttributeValueEntry ) + usValueLen + kFWBuffPad );

		pAttrValue->fAttributeValueData.fBufferSize		= usValueLen + kFWBuffPad;
		pAttrValue->fAttributeValueData.fBufferLength	= usValueLen;
		
		// Do record check, verify that offset is not past end of buffer, etc.
		if ( usValueLen + offset > buffLen ) throw( (SInt32)eDSInvalidBuffFormat );
		
		::memcpy( pAttrValue->fAttributeValueData.fBufferData, p, usValueLen );

		// Set the attribute value ID
		pAttrValue->fAttributeValueID = CalcCRC( pAttrValue->fAttributeValueData.fBufferData );

		*outAttrValue = pAttrValue;
		pAttrValue = nil;
		
		if ( (uiIndex + 1) > usValueCnt )
		{
			*inOutOffset = -1;
		}
		else
		{
			*inOutOffset += usValueLen;
			if ( (bufTag == 'StdB') || (bufTag == 'DbgB') )
			{
				*inOutOffset += 2;
			}
			else
			{
				*inOutOffset += 4;
			}
		}
	}

	catch( SInt32 err )
	{
		siResult = err;
	}

    //clean up pRecEntry if throwing an error
    if (pAttrValue != nil)
    {
        dsDeallocAttributeValueEntry(0,pAttrValue);
        pAttrValue = nil;
    }

	return( (tDirStatus)siResult );

} // ExtractNextAttributeValue

//------------------------------------------------------------------------------------
//	Name: IsNodePathStrBuffer
//------------------------------------------------------------------------------------

tDirStatus IsNodePathStrBuffer ( tDataBufferPtr inOutDataBuff )

{
	tDirStatus	outResult	= eDSNoErr;
	SInt32		siResult	= eDSNoErr;
	CBuff		inBuff;
    UInt32		bufTag		= 0;

    //check to determine whether we can use client side buffer parsing of node path strings
    //ie. look for 'npss' tag

	try
	{
		if ( inOutDataBuff == nil ) throw( (SInt32)eDSEmptyBuffer );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		siResult = inBuff.Initialize( inOutDataBuff );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = inBuff.GetBuffType( &bufTag );
		if ( siResult != eDSNoErr ) throw( siResult );
        
        if (bufTag != 'npss')  //KW should make 'npss' a private available constant
        {
            outResult = eDSInvalidTag;
        }

    }
    
	catch( SInt32 err )
	{
		outResult = (tDirStatus)err;
	}

	return( outResult );
    
} // IsNodePathStrBuffer


//------------------------------------------------------------------------------------
//	Name: ExtractDirNodeName
//------------------------------------------------------------------------------------

tDirStatus ExtractDirNodeName (	tDataBufferPtr	inOutDataBuff,
								UInt32			inDirNodeIndex,
								tDataListPtr   *outDataList )
{
	SInt32						siResult		= eDSNoErr;
	UInt16						usValueLen		= 0;
	UInt32						iSegment		= 0;
	UInt32						uiIndex			= 0;
	UInt32						uiCount			= 0;
	UInt32						offset			= 0;
	char					   *p				= nil;
	UInt32						buffSize		= 0;
	char					   *outNodePathStr	= nil;
	UInt16						segmentCount	= 0;

	try
	{
		//buffer format ie. only used by FW in dsGetDirNodeName
		//4 byte tag
		//4 byte count of node paths
		//repeated:
		// 2 byte count of string segments - ttt
		// sub repeated:
		// 2 byte segment string length - ttt
		// actual segment string - ttt
		// ..... blank space
		// offsets for each node path in reverse order from end of buffer
		// note that buffer length is length of data aboved tagged with ttt
		uiIndex = inDirNodeIndex;
		if (uiIndex == 0) throw( (SInt32)eDSInvalidIndex );

		if ( inOutDataBuff == nil ) throw( (SInt32)eDSNullDataBuff );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );
		
		if ( outDataList == nil ) throw( (SInt32)eDSNullDataList );

		//validate count and tag together are not past end of buffer
		if (inOutDataBuff->fBufferSize < 8) throw( (SInt32)eDSInvalidBuffFormat );

		buffSize	= inOutDataBuff->fBufferSize;

		p			= inOutDataBuff->fBufferData + 4; //already know tag is correct
		offset		= 4;

		// Get the buffer node path string count
		::memcpy( &uiCount, p, 4 );

		//validate count contains number
		if (uiCount == 0) throw( (SInt32)eDSEmptyBuffer );

		//validate that index requested is in this buffer
		if (uiIndex > uiCount)  throw( (SInt32)eDSIndexOutOfRange );
		
		//retrieve the offset into the data for this index
		::memcpy(&offset,inOutDataBuff->fBufferData + buffSize - (uiIndex * 4) , 4);
		p = inOutDataBuff->fBufferData + offset;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffSize)  throw( (SInt32)eDSInvalidBuffFormat );
		
		//retrieve number of segments in node path string
		::memcpy( &segmentCount, p, 2 );
		
		p		+= 2;
		offset	+= 2;

		//build the tDataList here
		*outDataList = dsDataListAllocate(0);

		//retrieve each segment and add it to the data list
		for (iSegment = 1; iSegment <= segmentCount; iSegment++)
		{
			// Do record check, verify that offset is not past end of buffer, etc.
			if ( 2 + offset > buffSize ) throw( (SInt32)eDSInvalidBuffFormat );
		
			//retrieve the segment's length
			::memcpy( &usValueLen, p, 2 );
		
			p		+= 2;
			offset	+= 2;

			// Do record check, verify that offset is not past end of buffer, etc.
			if ( usValueLen > (SInt32)(buffSize - offset) ) throw( (SInt32)eDSInvalidBuffFormat );
		
			outNodePathStr = (char *) calloc( 1, usValueLen + 1);
			::memcpy( outNodePathStr, p, usValueLen );
	
			//add to the tDataList here
			dsAppendStringToListAlloc( 0, *outDataList, outNodePathStr );
			
			free(outNodePathStr);
			outNodePathStr = nil;

			p		+= usValueLen;
			offset	+= usValueLen;

		}
		
	}

	catch( SInt32 err )
	{
		siResult = err;
	}

	return( (tDirStatus)siResult );

} // ExtractDirNodeName

//------------------------------------------------------------------------------------
//	Name: IsRemoteReferenceMap
//------------------------------------------------------------------------------------

tDirStatus IsRemoteReferenceMap ( UInt32 inRef )

{
	tDirStatus	outResult	= eDSInvalidReference;
	
    //check to determine whether this is a remote reference map
	//ie. 0x00C0000 bits are set

	if ((inRef & 0x00C00000) != 0)
	{
		outResult = eDSNoErr;
	}

	return( outResult );
    
} // IsRemoteReferenceMap


//------------------------------------------------------------------------------------
//	Name: MakeGDNIFWRef
//------------------------------------------------------------------------------------

tDirStatus MakeGDNIFWRef (	tDataBufferPtr		inOutDataBuff,
							tAttributeListRef	*outAttributeListRef )

{
	SInt32					siResult		= eDSNoErr;
	char 				   *pData			= nil;
	CBuff					inBuff;
	UInt32					offset			= 0;
	UInt16					usTypeLen		= 0;
	UInt16					usNameLen		= 0;
	UInt32					buffLen			= 0;
    UInt32					bufTag			= 0;
	UInt32					uiOffset		= 0;

	try
	{
		if ( inOutDataBuff == nil ) throw( (SInt32)eDSNullDataBuff );
		if (inOutDataBuff->fBufferSize == 0) throw( (SInt32)eDSEmptyBuffer );

		siResult = inBuff.Initialize( inOutDataBuff );
		if ( siResult != eDSNoErr ) throw( siResult );

		siResult = inBuff.GetBuffType( &bufTag );
		if ( siResult != eDSNoErr ) throw( siResult );
        
		buffLen = inBuff.GetDataBlockLength(1);
		
		pData = inBuff.GetDataBlock( 1, &uiOffset );
		
		//add to the offset for the attr list the length of the usually GetDirNodeInfo fixed record labels
//		record length = 4
//		aRecData->AppendShort( ::strlen( recordType ) ); = 2
//		aRecData->AppendString( recordType ); = ??
//		aRecData->AppendShort( ::strlen( recordName ) ); = 2
//		aRecData->AppendString( recordName ); = ??
//		total adjustment = 4 + 2 + ?? + 2 + ?? = ?? (60 for all known DS plugins using std buffer)

		// Skip past the record length
		pData	+= 4;
		offset	= 4;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the length for the record type
		::memcpy( &usTypeLen, pData, 2 );
		
		pData	+= 2;
		offset	+= 2;

		pData	+= usTypeLen;
		offset	+= usTypeLen;

		// Do record check, verify that offset is not past end of buffer, etc.
		if (2 + offset > buffLen)  throw( (SInt32)eDSInvalidBuffFormat );
		
		// Get the length for the record name
		::memcpy( &usNameLen, pData, 2 );
		
		pData	+= 2;
		offset	+= 2;

		pData	+= usNameLen;
		offset	+= usNameLen;

        //create a reference here
        siResult = gFWRefTable.NewAttrListRef( outAttributeListRef, 0, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		
		if ( (bufTag == 'DbgA') || (bufTag == 'DbgB') )
		{
			syslog(LOG_CRIT, "DS:dsGetDirNodeInfo:MakeGDNIFWRef:gFWRefTable.NewAttrListRef ref = %d", *outAttributeListRef);
		}
		        
		//uiOffset + offset;	// context used by next calls of GetAttributeEntry
								// include the four bytes of the buffLen
		siResult = gFWRefTable.SetOffset( *outAttributeListRef, eAttrListRefType, offset + uiOffset, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
		siResult = gFWRefTable.SetBufTag( *outAttributeListRef, eAttrListRefType, bufTag, gProcessPID );
		if ( siResult != eDSNoErr ) throw( siResult );
	}

	catch( SInt32 err )
	{
		siResult = err;
	}
    
	return( (tDirStatus)siResult );

} // MakeGDNIFWRef

const char *dsGetPluginNamePriv( UInt32 inNodeRefNum, UInt32 inPID )
{
	return gFWRefMap.GetPluginName( inNodeRefNum, inPID );
}
