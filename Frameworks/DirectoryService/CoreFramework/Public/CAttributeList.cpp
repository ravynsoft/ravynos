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
 * @header CAttributeList
 */

#include "CAttributeList.h"
#include "DSUtils.h"

//------------------------------------------------------------------------------------
//	* CAttributeList
//------------------------------------------------------------------------------------

CAttributeList::CAttributeList ( tDataListPtr inNodeList )
{
	fNodeList = inNodeList;
	bCleanData = false;
} // CAttributeList


//------------------------------------------------------------------------------------
//	* CAttributeList
//------------------------------------------------------------------------------------

CAttributeList::CAttributeList ( char *inNode )
{   
	if (inNode != nil)
	{
		fNodeList = dsBuildListFromStringsPriv( inNode, nil );
		bCleanData = true;
	}
} // CAttributeList


//------------------------------------------------------------------------------------
//	* CAttributeList
//------------------------------------------------------------------------------------

CAttributeList::~CAttributeList ( void )
{
	if ( (bCleanData) && (fNodeList != nil) )
	{
		dsDataListDeallocatePriv( fNodeList );
		//need to free the header as well
		free( fNodeList );
		fNodeList = nil;
	}
} // ~CAttributeList


//------------------------------------------------------------------------------------
//	* GetCount
//------------------------------------------------------------------------------------

UInt32 CAttributeList::GetCount ( void )
{
	if ( fNodeList != nil )
	{
		return( fNodeList->fDataNodeCount );
	}

	return( 0 );

} // GetCount


//------------------------------------------------------------------------------------
//	* GetAttribute
//------------------------------------------------------------------------------------

SInt32 CAttributeList::GetAttribute( UInt32 inIndex, char **outData )
{
	tDataNodePtr		pCurrNode	= nil;
	tDataBufferPriv	   *pPrivData	= nil;
	
	if ( outData == NULL )
		return eParameterError;
	*outData = NULL;
	
	if ( inIndex == 0 )
		return eDSInvalidIndex;
	
	if ( fNodeList == NULL )
		return eDSNullAttributeTypeList;

	pCurrNode = fNodeList->fDataListHead;
	if ( pCurrNode == NULL || inIndex > fNodeList->fDataNodeCount )
		return eDSAttrListError;
	
	for ( UInt32 idx = 1; idx <= fNodeList->fDataNodeCount; idx++ )
	{
		pPrivData = (tDataBufferPriv *)pCurrNode;
		if ( idx == inIndex )
		{
			*outData = pPrivData->fBufferData;
			return eDSNoErr;
		}
		
		pCurrNode = pPrivData->fNextPtr;
	}
	
	return eDSAttrListError;

} // GetAttribute
