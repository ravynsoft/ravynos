/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#include "CDSPluginUtils.h"
#include "CAuthAuthority.h"
#include <DirectoryService/DirServicesConst.h>
#include <DirectoryService/DirServicesUtilsPriv.h>
#include <DirectoryServiceCore/PrivateTypes.h>

#include "SharedConsts.h"

CAuthAuthority::CAuthAuthority()
{
	mValueArray = NULL;
	mDisabledAuthorityStorage = NULL;
}


CAuthAuthority::CAuthAuthority(CAuthAuthority &inAuthAuthority)
{
	mValueArray = CFArrayCreateMutableCopy( kCFAllocatorDefault, 0, inAuthAuthority.mValueArray );
	mDisabledAuthorityStorage = NULL;
}


CAuthAuthority::~CAuthAuthority()
{
	DSCFRelease( mValueArray );
	DSCFRelease( mDisabledAuthorityStorage );
}


bool
CAuthAuthority::AddValue( const char *inAuthAuthorityStr )
{
	bool added = false;
	CFMutableDictionaryRef aaDict = dsConvertAuthAuthorityToCFDict( inAuthAuthorityStr );
	if ( aaDict != NULL )
	{
		if ( mValueArray == NULL )
			mValueArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
		
		if ( mValueArray != NULL ) {
			CFArrayAppendValue( mValueArray, aaDict );
			added = true;
		}
		CFRelease( aaDict );
	}
	
	return added;
}


bool
CAuthAuthority::AddValue( CFStringRef inAuthAuthorityString )
{
	char aaBuffer[1024];
	
	if ( CFStringGetCString(inAuthAuthorityString, aaBuffer, sizeof(aaBuffer), kCFStringEncodingUTF8) )
		return AddValue( aaBuffer );
	
	return false;
}


bool
CAuthAuthority::AddValues( CFArrayRef inAuthAuthorityArray )
{
	bool added = false;
	CFIndex index = 0;
	CFIndex aryCount = 0;
	CFStringRef aaString = NULL;
	CFMutableDictionaryRef aaDict = NULL;
	char aaBuffer[1024];
	
	if ( inAuthAuthorityArray == NULL )
		return false;
	
	if ( mValueArray == NULL ) {
		mValueArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
		if ( mValueArray == NULL )
			return false;
	}
	
	aryCount = CFArrayGetCount( inAuthAuthorityArray );
	for ( index = 0; index < aryCount; index++ )
	{
		aaString = (CFStringRef) CFArrayGetValueAtIndex( inAuthAuthorityArray, index );
		if ( aaString != NULL )
		{
			if ( CFStringGetCString(aaString, aaBuffer, sizeof(aaBuffer), kCFStringEncodingUTF8) )
			{
				aaDict = dsConvertAuthAuthorityToCFDict( aaBuffer );
				if ( aaDict != NULL )
				{
					CFArrayAppendValue( mValueArray, aaDict );
					CFRelease( aaDict );
					added = true;
				}
			}
		}
	}
	
	return added;
}


CFIndex
CAuthAuthority::GetValueCount( void )
{
	CFIndex theResult = 0;
	
	if ( mValueArray != NULL )
		theResult = CFArrayGetCount( mValueArray );
	
	return theResult;
}


char *
CAuthAuthority::GetValueAtIndex( int inIndex )
{
	if ( mValueArray == NULL )
		return NULL;
	
	return dsConvertCFDictToAuthAuthority( (CFMutableDictionaryRef) CFArrayGetValueAtIndex(mValueArray, inIndex) );
}


char *
CAuthAuthority::GetValueForTag( const char *inTagStr )
{
	return dsConvertCFDictToAuthAuthority( this->GetValueForTagAsCFDict(inTagStr) );
}


CFMutableDictionaryRef
CAuthAuthority::GetValueForTagAsCFDict( const char *inTagStr )
{
	CFMutableDictionaryRef theDict = NULL;
	CFMutableDictionaryRef aDict = NULL;
	CFIndex arrayCount = 0;
	CFIndex index = 0;
	CFStringRef tagValueString = NULL;
	CFStringRef searchTagValueString = NULL;
	
	if ( mValueArray == NULL )
		return NULL;
	
	searchTagValueString = CFStringCreateWithCString( kCFAllocatorDefault, inTagStr, kCFStringEncodingUTF8 );
	if ( searchTagValueString == NULL )
		return NULL;
	
	arrayCount = CFArrayGetCount( mValueArray );
	for ( index = 0; index < arrayCount; index++ )
	{
		aDict = (CFMutableDictionaryRef) CFArrayGetValueAtIndex( mValueArray, index );
		if ( aDict != NULL )
		{
			tagValueString = (CFStringRef) CFDictionaryGetValue( aDict, CFSTR("tag") );
			if ( tagValueString != NULL &&
				 CFStringCompare(tagValueString, searchTagValueString, kCFCompareCaseInsensitive) == kCFCompareEqualTo )
			{
				theDict = aDict;
				break;
			}
		}
	}
	
	CFRelease( searchTagValueString );
	
	return theDict;
}


char *
CAuthAuthority::GetDataForTag( const char *inTagStr, CFIndex inDataSegmentIndex )
{
	char *retStr = NULL;
	CFArrayRef aaDataArray = NULL;
	CFMutableDictionaryRef aaDict = this->GetValueForTagAsCFDict( inTagStr );
	if ( aaDict != NULL )
	{
		aaDataArray = (CFArrayRef) CFDictionaryGetValue( aaDict, CFSTR("data") );
		if ( aaDataArray != NULL )
		{
			CFIndex arrayCount = CFArrayGetCount( aaDataArray );
			if ( inDataSegmentIndex >= arrayCount )
				return NULL;
			
			CFStringRef aaDataString = (CFStringRef) CFArrayGetValueAtIndex( aaDataArray, inDataSegmentIndex );
			if ( aaDataString == NULL )
				return NULL;
			
			// include room for a zero-terminator
			size_t needSize = CFStringGetMaximumSizeForEncoding( CFStringGetLength(aaDataString), kCFStringEncodingUTF8 ) + 1;
			
			retStr = (char *) malloc( needSize );
			if ( retStr == NULL )
				return NULL;
			
			if ( !CFStringGetCString(aaDataString, retStr, needSize, kCFStringEncodingUTF8) )
				DSFreeString( retStr );
		}
	}
	
	return retStr;
}
		
		
bool
CAuthAuthority::SetValueForTag( const char *inTagStr, const char *inAuthAuthorityStr )
{
	this->RemoveValueForTag( inTagStr );
	return this->AddValue( inAuthAuthorityStr );
}


bool
CAuthAuthority::SetValueDisabledForTag( const char *inTagStr )
{
	bool theResult = false;
	CFMutableArrayRef dataArray = NULL;
	CFStringRef tagString = NULL;
	
	CFMutableDictionaryRef aaDict = this->GetValueForTagAsCFDict( inTagStr );
	if ( aaDict != NULL )
	{
		dataArray = (CFMutableArrayRef) CFDictionaryGetValue( aaDict, CFSTR("data") );
		if ( dataArray == NULL )
		{
			dataArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
			if ( dataArray == NULL )
				return false;
				
			CFDictionaryAddValue( aaDict, CFSTR("data"), dataArray );
			CFRelease( dataArray );
		}
		
		// Insert the current tag into the data array
		tagString = (CFStringRef) CFDictionaryGetValue( aaDict, CFSTR("tag") );
		CFArrayInsertValueAtIndex( dataArray, 0, tagString );
		CFArrayInsertValueAtIndex( dataArray, 0, CFSTR("") );
		
		// replace the primary tag
		CFDictionarySetValue( aaDict, CFSTR("tag"), CFSTR(kDSTagAuthAuthorityDisabledUser) );
		theResult = true;
	}
	else
	{
		aaDict = this->GetValueForTagAsCFDict( kDSTagAuthAuthorityDisabledUser );
		if ( aaDict != NULL )
		{
			// There's already a disabled Auth Authority, so check the first
			// data item to see if it's a match for the tag
			
			CFStringRef searchTagValueString = CFStringCreateWithCString( kCFAllocatorDefault, inTagStr, kCFStringEncodingUTF8 );
			if ( searchTagValueString != NULL )
			{
				dataArray = (CFMutableArrayRef) CFDictionaryGetValue( aaDict, CFSTR("data") );
				if ( dataArray != NULL )
				{
					tagString = (CFStringRef) CFArrayGetValueAtIndex( dataArray, 0 );
					if ( tagString != NULL &&
						 CFStringCompare(searchTagValueString, tagString, kCFCompareCaseInsensitive) == kCFCompareEqualTo )
					{
						// already disabled
						theResult = true;
					}
				}
				
				CFRelease( searchTagValueString );
			}
		}
		
		// if not disabled yet, disable
		if ( theResult == false )
		{
			char disabledAAStr[256] = {0};
			
			snprintf( disabledAAStr, sizeof(disabledAAStr), "%s;%s;", kDSValueAuthAuthorityDisabledUser, inTagStr );
			theResult = this->AddValue( disabledAAStr );
		}
	}
	
	return theResult;
}


bool
CAuthAuthority::SetDataForTag( const char *inTagStr, const char *inDataSegment, CFIndex inDataSegmentIndex )
{
	CFIndex dataArrayCount = 0;
	CFStringRef dataSegString = NULL;
	CFMutableDictionaryRef aaDict = this->GetValueForTagAsCFDict( inTagStr );
	if ( aaDict == NULL )
		return false;
	
	CFMutableArrayRef dataArray = (CFMutableArrayRef)CFDictionaryGetValue( aaDict, CFSTR("data") );
	if ( dataArray == NULL )
	{
		if ( inDataSegmentIndex != 0 )
			return false;
		
		dataSegString = CFStringCreateWithCString( NULL, inDataSegment, kCFStringEncodingUTF8 );
		dataArray = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		CFArrayAppendValue( dataArray, dataSegString );
		CFRelease( dataSegString );
		
		CFDictionaryAddValue( aaDict, CFSTR("data"), dataArray );
		CFRelease( dataArray );
	}
	else
	{
		// Example: inDataSegmentIndex == 0, CFArrayGetCount(dataArray) == 1, then
		// 0 (replace) & 1 (append) are valid positions, 2+ are not.
		if ( inDataSegmentIndex > (dataArrayCount = CFArrayGetCount(dataArray)) )
			return false;
		
		dataSegString = CFStringCreateWithCString( NULL, inDataSegment, kCFStringEncodingUTF8 );
		if ( inDataSegmentIndex == dataArrayCount )
		{
			CFArrayAppendValue( dataArray, dataSegString );
		}
		else
		{
			CFArraySetValueAtIndex( dataArray, inDataSegmentIndex, dataSegString );
		}
		CFRelease( dataSegString );
	}
	
	return true;
}


void
CAuthAuthority::RemoveValueForTag( const char *inTagStr )
{
	CFIndex arrayCount = 0;
	CFIndex index = 0;
	CFDictionaryRef aDict = NULL;
	CFStringRef tagValueString = NULL;
	CFStringRef searchTagValueString = NULL;
	
	if ( mValueArray == NULL )
		return;
	
	searchTagValueString = CFStringCreateWithCString( kCFAllocatorDefault, inTagStr, kCFStringEncodingUTF8 );
	if ( searchTagValueString == NULL )
		return;

	arrayCount = CFArrayGetCount( mValueArray );
	for ( index = arrayCount - 1; index >= 0; index-- )
	{
		aDict = (CFDictionaryRef) CFArrayGetValueAtIndex( mValueArray, index );
		if ( aDict != NULL )
		{
			tagValueString = (CFStringRef) CFDictionaryGetValue( aDict, CFSTR("tag") );
			if ( tagValueString != NULL &&
				 CFStringCompare(tagValueString, searchTagValueString, kCFCompareCaseInsensitive) == kCFCompareEqualTo )
			{
				CFArrayRemoveValueAtIndex( mValueArray, index );
			}
		}
	}
	
	CFRelease( searchTagValueString );
}


void
CAuthAuthority::ToggleDisabledAuthority( bool enable )
{
	CFMutableDictionaryRef enableDict = NULL;
	CFMutableArrayRef dataArray = NULL;
	CFStringRef aString = NULL;
	
	if ( enable )
	{
		mDisabledAuthorityStorage = this->GetValueForTagAsCFDict( kDSTagAuthAuthorityDisabledUser );
		if ( mDisabledAuthorityStorage != NULL )
		{
			CFRetain( mDisabledAuthorityStorage );
			this->RemoveValueForTag( kDSTagAuthAuthorityDisabledUser );
			
			enableDict = CFDictionaryCreateMutableCopy( kCFAllocatorDefault, 0, mDisabledAuthorityStorage );
			if ( enableDict != NULL )
			{
				dataArray = (CFMutableArrayRef) CFDictionaryGetValue( enableDict, CFSTR("data") );
				if ( dataArray != NULL )
				{
					// remove the version, don't care
					CFArrayRemoveValueAtIndex( dataArray, 0 );
					
					// now, the top item is the tag
					aString = (CFStringRef) CFArrayGetValueAtIndex( dataArray, 0 );
					CFDictionarySetValue( enableDict, CFSTR("tag"), aString );
					CFArrayRemoveValueAtIndex( dataArray, 0 );
					
					// insert it into the authority list
					CFArrayAppendValue( mValueArray, enableDict );
					CFRelease( enableDict );
				}
			}
		}
	}
}


bool
CAuthAuthority::TagDisabled( const char *inTagStr )
{
	bool result = false;
	CFIndex arrayCount = 0;
	CFIndex index = 0;
	CFDictionaryRef aDict = NULL;
	CFStringRef tagValueString = NULL;
	CFStringRef searchTagValueString = NULL;
	CFMutableArrayRef dataArray = NULL;
	CFStringRef aString = NULL;
	
	if ( mValueArray == NULL )
		return false;
	
	searchTagValueString = CFStringCreateWithCString( kCFAllocatorDefault, inTagStr, kCFStringEncodingUTF8 );
	if ( searchTagValueString == NULL )
		return false;
	
	arrayCount = CFArrayGetCount( mValueArray );
	for ( index = arrayCount - 1; index >= 0; index-- )
	{
		aDict = (CFDictionaryRef) CFArrayGetValueAtIndex( mValueArray, index );
		if ( aDict != NULL )
		{
			tagValueString = (CFStringRef) CFDictionaryGetValue( aDict, CFSTR("tag") );
			if ( tagValueString != NULL &&
				 CFStringCompare(tagValueString, CFSTR(kDSTagAuthAuthorityDisabledUser), kCFCompareCaseInsensitive) == kCFCompareEqualTo )
			{
				dataArray = (CFMutableArrayRef) CFDictionaryGetValue( aDict, CFSTR("data") );
				if ( dataArray != NULL )
				{
					aString = (CFStringRef) CFArrayGetValueAtIndex( dataArray, 1 );
					if ( aString != NULL &&
						 CFStringCompare(aString, searchTagValueString, kCFCompareCaseInsensitive) == kCFCompareEqualTo )
					{
						result = true;
						break;
					}
				}
			}
		}
	}
	
	CFRelease( searchTagValueString );
	
	return result;
}

