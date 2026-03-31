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
 * @header CDataBuff
 */

// sys
#include <strings.h>
#include <string.h>
#include <stdlib.h> // for calloc and free

// app
#include "CDataBuff.h"

// ---------------------------------------------------------------------------
//	CDataBuff ()
// ---------------------------------------------------------------------------

CDataBuff::CDataBuff ( UInt32 inSize )
	: fSize( 0 ), fLength( 0 ), fData( nil )
{
	GrowBuff( inSize );
} // CDataBuff


// ---------------------------------------------------------------------------
//	~CDataBuff ()
// ---------------------------------------------------------------------------

CDataBuff::~CDataBuff ( void )
{
	if ( fData != nil )
	{
		::free( fData );
		fData = nil;
	}
} // ~CDataBuff


// ---------------------------------------------------------------------------
//	AppendString ()
// ---------------------------------------------------------------------------

void CDataBuff::AppendString ( const char *inStr )
{
	size_t	len	= 0;

	if ( inStr != nil )
	{
		len = ::strlen( inStr );
		GrowBuff( (UInt32) (fLength + len + 1) );

		::strcpy ( &fData[ fLength ], inStr );
		fLength += (UInt32) len;
	}
} // AppendString


// ---------------------------------------------------------------------------
//	AppendLong ()
// ---------------------------------------------------------------------------

void CDataBuff::AppendLong ( UInt32 inLong )
{
	UInt32	len	= 4;

   	GrowBuff( fLength + len + 1 );

	::memcpy( &fData[ fLength ], &inLong, 4 );

	fLength += len;

} // AppendLong


// ---------------------------------------------------------------------------
//	AppendShort ()
// ---------------------------------------------------------------------------

void CDataBuff::AppendShort ( UInt16 inShort )
{
	UInt32	len	= 2;

   	GrowBuff( fLength + len + 1 );

	::memcpy( &fData[ fLength ], &inShort, 2 );

	fLength += len;

} // AppendShort


// ---------------------------------------------------------------------------
//	AppendBlock ()
// ---------------------------------------------------------------------------

void CDataBuff::AppendBlock ( const void *inData, UInt32 inLength )
{
   	GrowBuff( fLength + inLength + 1 );

	::memcpy( &fData[ fLength ], inData, inLength );

	fLength += inLength;

} // AppendBlock


// ---------------------------------------------------------------------------
//	Clear ()
// ---------------------------------------------------------------------------

void CDataBuff::Clear ( UInt32 inSize )
{
	fLength = 0;

	if ( inSize == 0 ) //if simple clear then just clear and leave size as is since we already allocated space
	{
		bzero(fData, fSize);
	}
	else if ( inSize < fSize )
	{
		::free( fData );
		fData = nil;

		GrowBuff( inSize ); // will calloc
	}
	else
	{
		bzero( fData, fSize );
	}
} // Clear


// ---------------------------------------------------------------------------
//	GetSize ()
// ---------------------------------------------------------------------------

UInt32 CDataBuff::GetSize ( void )
{
	return( fSize );
} // GetSize


// ---------------------------------------------------------------------------
//	GetLength ()
// ---------------------------------------------------------------------------

UInt32 CDataBuff::GetLength ( void )
{
	return( fLength );
} // GetLength


// ---------------------------------------------------------------------------
//	GetData ()
// ---------------------------------------------------------------------------

char* CDataBuff::GetData ( void )
{
	return( fData );
} // GetData


// ---------------------------------------------------------------------------
//	GrowBuff ()
// ---------------------------------------------------------------------------

void CDataBuff::GrowBuff ( UInt32 inNewSize )
{
	UInt32	newSize	= inNewSize;

	// Allocate the default length if requested.
	if ( newSize == 0 )
	{
		newSize = kDefaultSize;
	}

	// Don't bother reallocating if there is already enough room.
	if ( (newSize <= fSize) && (fData != nil) )
	{
		return;
	}

	// Round the requested size to the nearest power of two (> 16).
	//	The comparison is an optimization for the most common case.
	if ( newSize != kDefaultSize )
	{
		register UInt32 pow2 = 16;
		while ( pow2 < newSize )
		{
			pow2 <<= 1;
		}
		newSize = pow2;
	}

	register char *newData = (char *)::calloc( 1, newSize );

	if ( fData != nil )
	{
		if ( fLength != 0 )
		{
			// Copy the old data to the new block
			::memcpy( newData, fData, fLength );
		}

		::free( fData );
		fData = nil;
	}

	fSize = newSize;
	fData = newData;

} // Grow


SInt32 dsCDataBuffFromAttrTypeAndStringValue( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char* inAttrValue )
{
	if ( (inOutAttrDataBuff == nil) || (inOutDataBuff == nil) )
	{
		return((SInt32)eDSNullDataBuff);
	}
	
	if (inAttrType == nil)
	{
		return((SInt32)eDSNullAttributeType);
	}

	inOutDataBuff->Clear();
	
	// Append the attribute type
	inOutDataBuff->AppendShort( ::strlen( inAttrType ) );
	inOutDataBuff->AppendString( inAttrType );

	if ( ( inbAttrInfoOnly == false ) && (inAttrValue != nil) )
	{
		// Attribute value count
		inOutDataBuff->AppendShort( 1 );

		// Append the attribute value
		inOutDataBuff->AppendLong( (UInt32) ::strlen( inAttrValue ) );
		inOutDataBuff->AppendString( inAttrValue );

	}
	else
	{
		inOutDataBuff->AppendShort( 0 );
	}

	inOutAttrDataBuff->AppendLong( inOutDataBuff->GetLength() );
	inOutAttrDataBuff->AppendBlock( inOutDataBuff->GetData(), inOutDataBuff->GetLength() );

	return((SInt32)eDSNoErr);
}

SInt32 dsCDataBuffFromAttrTypeAndStringValues( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char* inAttrValue, ... )
{
	va_list		args;
	SInt32		result = eDSNoErr;

	va_start( args, inAttrValue );
	result = dsCDataBuffFromAttrTypeAndStringArgValues(inOutAttrDataBuff, inOutDataBuff, inbAttrInfoOnly, inAttrType, inAttrValue, args);
	va_end( args );
	return(result);
}

SInt32 dsCDataBuffFromAttrTypeAndStringArgValues( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char* inAttrValue, va_list inAttrValues )
{
	if ( (inOutAttrDataBuff == nil) || (inOutDataBuff == nil) )
	{
		return((SInt32)eDSNullDataBuff);
	}
	
	if (inAttrType == nil)
	{
		return((SInt32)eDSNullAttributeType);
	}

	if ( (inAttrValue == nil) && !inbAttrInfoOnly )
	{
		return((SInt32)eDSNullAttributeValue);
	}
	
	inOutDataBuff->Clear();
	
	// Append the attribute type
	inOutDataBuff->AppendShort( ::strlen( inAttrType ) );
	inOutDataBuff->AppendString( inAttrType );

	if ( inbAttrInfoOnly == false )
	{
		CDataBuff* tmpDataBuff = new CDataBuff();
		tmpDataBuff->Clear();
		UInt32 numAttrValues = 0;
		const char* argString = inAttrValue;
		while (argString != nil)
		{
			numAttrValues++;
			// Append the attribute value
			tmpDataBuff->AppendLong( (UInt32) ::strlen( argString ) );
			tmpDataBuff->AppendString( argString );

			argString = va_arg( inAttrValues, char * );
		}
		
		// Attribute value count
		inOutDataBuff->AppendShort( numAttrValues );
		//add the attr values
		inOutDataBuff->AppendBlock( tmpDataBuff->GetData(), tmpDataBuff->GetLength() );
		
		delete(tmpDataBuff);
		tmpDataBuff = nil;
	}
	else
	{
		inOutDataBuff->AppendShort( 0 );
	}

	va_end( inAttrValues );
	
	inOutAttrDataBuff->AppendLong( inOutDataBuff->GetLength() );
	inOutAttrDataBuff->AppendBlock( inOutDataBuff->GetData(), inOutDataBuff->GetLength() );

	return((SInt32)eDSNoErr);
}


SInt32 dsCDataBuffFromAttrTypeAndStringValues( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char** inAttrValues )
{
	if ( (inOutAttrDataBuff == nil) || (inOutDataBuff == nil) )
	{
		return((SInt32)eDSNullDataBuff);
	}
	
	if (inAttrType == nil)
	{
		return((SInt32)eDSNullAttributeType);
	}

	inOutDataBuff->Clear();
	
	// Append the attribute type
	inOutDataBuff->AppendShort( ::strlen( inAttrType ) );
	inOutDataBuff->AppendString( inAttrType );

	if ( ( inbAttrInfoOnly == false ) && (inAttrValues != nil) && (inAttrValues[0] != nil) )
	{
		CDataBuff* tmpDataBuff = new CDataBuff();
		tmpDataBuff->Clear();
		UInt32 numAttrValues = 0;
		const char* argString = inAttrValues[numAttrValues];
		while (argString != nil) //expecting a null terminated char* list
		{
			numAttrValues++;
			// Append the attribute value
			tmpDataBuff->AppendLong( (UInt32) ::strlen( argString ) );
			tmpDataBuff->AppendString( argString );

			argString = inAttrValues[numAttrValues];
		}
		
		// Attribute value count
		inOutDataBuff->AppendShort( numAttrValues );
		//add the attr values
		inOutDataBuff->AppendBlock( tmpDataBuff->GetData(), tmpDataBuff->GetLength() );
		
		delete(tmpDataBuff);
		tmpDataBuff = nil;
	}
	else
	{
		inOutDataBuff->AppendShort( 0 );
	}

	inOutAttrDataBuff->AppendLong( inOutDataBuff->GetLength() );
	inOutAttrDataBuff->AppendBlock( inOutDataBuff->GetData(), inOutDataBuff->GetLength() );

	return((SInt32)eDSNoErr);
}


SInt32 dsCDataBuffFromAttrTypeAndData( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, 
                                       const char* inAttrValue, UInt32 inLength )
{
	if ( (inOutAttrDataBuff == nil) || (inOutDataBuff == nil) )
	{
		return((SInt32)eDSNullDataBuff);
	}
	
	if (inAttrType == nil)
	{
		return((SInt32)eDSNullAttributeType);
	}
    
	inOutDataBuff->Clear();
	
	// Append the attribute type
	inOutDataBuff->AppendShort( ::strlen( inAttrType ) );
	inOutDataBuff->AppendString( inAttrType );
    
	if ( ( inbAttrInfoOnly == false ) && (inAttrValue != nil) )
	{
		// Attribute value count
		inOutDataBuff->AppendShort( 1 );
        
		// Append the attribute value
		inOutDataBuff->AppendLong( inLength );
        inOutDataBuff->AppendBlock( inAttrValue, inLength );
	}
	else
	{
		inOutDataBuff->AppendShort( 0 );
	}
    
	inOutAttrDataBuff->AppendLong( inOutDataBuff->GetLength() );
	inOutAttrDataBuff->AppendBlock( inOutDataBuff->GetData(), inOutDataBuff->GetLength() );
    
	return((SInt32)eDSNoErr);    
}
