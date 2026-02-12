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
 * @header CDSPluginUtils
 */


#include "CDSPluginUtils.h"
#include "DSUtils.h"
#include "CLog.h"
#include <DirectoryServiceCore/ServerModuleLib.h>
#include <DirectoryServiceCore/DSLThread.h>
#include <CommonCrypto/CommonCryptor.h>
#include <servers/bootstrap.h>
#include <syslog.h>
#include <mach/mach_error.h>
#include <mach/mach.h>
#include "SMBAuth.h"
#include <DirectoryService/DirServicesConstPriv.h>
#include <notify.h>

const char* CStrFromCFString( CFStringRef inCFStr, char** ioCStr, size_t* ioCStrSize, bool* outCStrAllocated )
{
	size_t	cStrSize = (ioCStrSize ? *ioCStrSize : 0);
	
	if ( outCStrAllocated != NULL )
		*outCStrAllocated = false;

	const char* cStrPtr = CFStringGetCStringPtr( inCFStr, kCFStringEncodingUTF8 );
	if ( cStrPtr != NULL )
		return cStrPtr;
	
	CFIndex maxCStrLen = CFStringGetMaximumSizeForEncoding( CFStringGetLength(inCFStr), kCFStringEncodingUTF8 ) + 1;
	if ( (size_t)maxCStrLen > cStrSize )
	{
		if( *ioCStr != NULL )
			free( *ioCStr );
		
		cStrSize = maxCStrLen;

		*ioCStr = (char*)malloc( cStrSize );
		if ( ioCStrSize != NULL )
			*ioCStrSize = cStrSize;
		if( outCStrAllocated != NULL )
			*outCStrAllocated = true;
	}
	
	if( !CFStringGetCString( inCFStr, *ioCStr, cStrSize, kCFStringEncodingUTF8 ) )
	{
		DbgLog( kLogPlugin, "CStrFromCFString(): CFStringGetCString() failed!" );
		return NULL;
	}
	
	return *ioCStr;
}

// ---------------------------------------------------------------------------
//	* CFDebugLog
// ---------------------------------------------------------------------------

void CFDebugLog( SInt32 lType, const char* format, ... )
{
	va_list ap;
	
	if ( LoggingEnabled(lType) ) {
		va_start( ap, format );
		
		CFDebugLogV( lType, format, ap );
		
		va_end( ap );
	}
}

// ---------------------------------------------------------------------------
//	* CFDebugLogV
// ---------------------------------------------------------------------------

void CFDebugLogV( SInt32 lType, const char* format, va_list ap )
{
	CFStringRef formatString = ::CFStringCreateWithCString( NULL, format, kCFStringEncodingUTF8 );

	CFStringRef logString = ::CFStringCreateWithFormatAndArguments( NULL, NULL, formatString, ap );
	CFMutableStringRef mutableLogString = ::CFStringCreateMutableCopy( NULL, 0, logString );
	CFStringFindAndReplace( mutableLogString, CFSTR( "%" ), CFSTR( "<percent>" ), CFRangeMake( 0, CFStringGetLength( mutableLogString ) ), 0 );
	
	const char *dbgStr;
	char* cStr = NULL;
	size_t cStrSize = 0;

	CFArrayRef dstrArray = CFStringCreateArrayBySeparatingStrings(NULL, mutableLogString, CFSTR("\n"));
	if ( dstrArray != NULL )
	{
		CFIndex aryCount = CFArrayGetCount( dstrArray );
		CFIndex index;
		CFStringRef lineString;
		
		for ( index = 0; index < aryCount; index++ )
		{
			lineString = (CFStringRef)CFArrayGetValueAtIndex( dstrArray, index );
			dbgStr = CStrFromCFString( lineString, &cStr, &cStrSize, NULL );
			DbgLog( lType, dbgStr, NULL );
		}
		
		DSCFRelease( dstrArray );
	}
	
	DSFreeString( cStr );
	DSCFRelease( logString );
	DSCFRelease( formatString );
	DSCFRelease( mutableLogString );
}


//--------------------------------------------------------------------------------------------------
// * PWOpenDirNode ()
//--------------------------------------------------------------------------------------------------

SInt32 PWOpenDirNode( tDirNodeReference fDSRef, char *inNodeName, tDirNodeReference *outNodeRef )
{
	SInt32			error		= eDSNoErr;
	SInt32			error2		= eDSNoErr;
	tDataList	   *pDataList	= nil;

	pDataList = ::dsBuildFromPathPriv( inNodeName, "/" );
    if ( pDataList != nil )
    {
        error = ::dsOpenDirNode( fDSRef, pDataList, outNodeRef );
        error2 = ::dsDataListDeallocatePriv( pDataList );
        free( pDataList );
    }

    return( error );

} // PWOpenDirNode


// ---------------------------------------------------------------------------
//	* DoesThisMatch
// ---------------------------------------------------------------------------

bool DoesThisMatch (			const char		   *inString,
								const char		   *inPatt,
								tDirPatternMatch	inHow )
{
	bool		bOutResult	= false;
	CFMutableStringRef	strRef	= CFStringCreateMutable(NULL, 0);
	CFMutableStringRef	patRef	= CFStringCreateMutable(NULL, 0);
	CFRange		range;

	if ( (inString == nil) || (inPatt == nil) || (strRef == nil) || (patRef == nil) )
	{
		return( false );
	}

	CFStringAppendCString( strRef, inString, kCFStringEncodingUTF8 );
	CFStringAppendCString( patRef, inPatt, kCFStringEncodingUTF8 );	
	if ( (inHow >= eDSiExact) && (inHow <= eDSiRegularExpression) )
	{
		CFStringUppercase( strRef, NULL );
		CFStringUppercase( patRef, NULL );
	}

	switch ( inHow )
	{
		case eDSExact:
		case eDSiExact:
		{
			if ( CFStringCompare( strRef, patRef, 0 ) == kCFCompareEqualTo )
			{
				bOutResult = true;
			}
		}
		break;

		case eDSStartsWith:
		case eDSiStartsWith:
		{
			if ( CFStringHasPrefix( strRef, patRef ) )
			{
				bOutResult = true;
			}
		}
		break;

		case eDSEndsWith:
		case eDSiEndsWith:
		{
			if ( CFStringHasSuffix( strRef, patRef ) )
			{
				bOutResult = true;
			}
		}
		break;

		case eDSContains:
		case eDSiContains:
		{
			range = CFStringFind( strRef, patRef, 0 );
			if ( range.location != kCFNotFound )
			{
				bOutResult = true;
			}
		}
		break;

		case eDSLessThan:
		case eDSiLessThan:
		{
			if ( CFStringCompare( strRef, patRef, 0 ) == kCFCompareLessThan )
			{
				bOutResult = true;
			}
		}
		break;

		case eDSGreaterThan:
		case eDSiGreaterThan:
		{
			if ( CFStringCompare( strRef, patRef, 0 ) == kCFCompareGreaterThan )
			{
				bOutResult = true;
			}
		}
		break;

		case eDSLessEqual:
		case eDSiLessEqual:
		{
			if ( CFStringCompare( strRef, patRef, 0 ) != kCFCompareGreaterThan )
			{
				bOutResult = true;
			}
		}
		break;

		case eDSGreaterEqual:
		case eDSiGreaterEqual:
		{
			if ( CFStringCompare( strRef, patRef, 0 ) != kCFCompareLessThan )
			{
				bOutResult = true;
			}
		}
		break;

		default:
			break;
	}

	CFRelease( strRef );
	strRef = nil;
	CFRelease( patRef );
	patRef = nil;

	return( bOutResult );

} // DoesThisMatch


// ---------------------------------------------------------------------------
//	* MSCHAPv2ChangePass
// ---------------------------------------------------------------------------

tDirStatus MSCHAPv2ChangePass(
	const char *inUsername,
	const uint8_t *inLMHash,
	const char *inEncoding,
	const uint8_t *inData,
	UInt32 inDataLen,
	uint8_t **outPassword,
	UInt32 *outPasswordLen )
{
	tDirStatus			status			= eDSNoErr;
	int					result			= -1;
    UInt32				pwLen			= 0;
	int32_t				encodeVal		= -1;
	CCCryptorStatus		ccStatus		= kCCSuccess;
	size_t				dataOutMoved	= 0;
    char				pwDataStr[1024];
    UInt8				data[1024];
    
	// decrypt message
	ccStatus = CCCrypt( kCCDecrypt, kCCAlgorithmRC4, 0, inLMHash, 16, NULL, inData, inDataLen,
						(uint8_t *)pwDataStr, sizeof(pwDataStr), &dataOutMoved );
	if ( ccStatus != kCCSuccess )
		return eDSAuthFailed;
	
	try
	{		
		// get the pw length
		pwLen = (UInt32) LittleEndianCharsToInt32( &pwDataStr[512] );
		
		// check the password length for > 512. It's the only integrity checking that the change password
		// method can do. Don't print password lengths to log files, just whether or not the length was the
		// reason for a rejection.
		if ( pwLen > 512 )
			throw( (tDirStatus)eDSAuthPasswordTooLong );
		if ( pwLen == 0 )
			throw( (tDirStatus)eDSAuthPasswordTooShort );
		
		memcpy( data, pwDataStr + 512 - pwLen, pwLen );
		data[pwLen] = '\0';
		data[pwLen + 1] = '\0';
		
		// get the password in UTF8 encoding
		sscanf( inEncoding, "%u", &encodeVal );
		if ( encodeVal == 0 && strcmp( inEncoding, "0" ) != 0 )
			throw( (tDirStatus)eDSInvalidBuffFormat );
		
		result = FALSE;
		switch ( encodeVal )
		{
			case 0:
				// already UTF8
				strcpy( pwDataStr, (char *)data );
				result = TRUE;
				break;
			
			case 1:
                {
                    // unicode
                    LittleEndianUnicodeToUnicode((u_int16_t *)data, pwLen/2, (u_int16_t *)data);
                    CFStringRef pwString = CFStringCreateWithCharacters( kCFAllocatorDefault, (UniChar *)data, pwLen/2 );
                    if ( pwString != NULL )
                    {
                        result = CFStringGetCString( pwString, pwDataStr, sizeof(pwDataStr), kCFStringEncodingUTF8 );
                        CFRelease( pwString );
                    }
                }
				break;
			
			default:
				// code-page
				result = FALSE;
                break;
		}
	
		if ( result == FALSE )
			throw( (tDirStatus)eDSAuthFailed );
		
		*outPasswordLen = strlen( pwDataStr );
		*outPassword = (uint8_t *)strdup( pwDataStr );
	}
 	catch ( tDirStatus catchStatus )
	{
		status = catchStatus;
	}
	
	bzero( pwDataStr, sizeof(pwDataStr) );
	
	return status;
}


// ---------------------------------------------------------------------------
//	GetLocalKDCRealm
//
//	@discussion
//		Located at /Config/KerberosKDC
// ---------------------------------------------------------------------------

char *GetLocalKDCRealm( void )
{
	char						*theRealmStr			= NULL;
	tDirStatus					siResult				= eDSNoErr;
	tDirReference				dsRef					= 0;
	tDataBufferPtr				dataBuffer				= dsDataBufferAllocatePriv( 1024 );
	UInt32						nodeCount				= 0;
	tContextData				context					= 0;
	tDataListPtr				nodeName				= NULL;
	tDirNodeReference			localNodeRef			= 0;
	tDataListPtr				recName					= NULL;
	tDataListPtr				recType					= NULL;
	tDataListPtr				attrTypes				= NULL;
	UInt32						recCount				= 1;
	tAttributeListRef			attrListRef				= 0;
	tRecordEntryPtr				pRecEntry				= NULL;
	tAttributeValueListRef		valueRef				= 0;
	tAttributeEntryPtr			pAttrEntry				= NULL;
	tAttributeValueEntryPtr		pValueEntry				= NULL;
	
	siResult = dsOpenDirService( &dsRef );
	if ( siResult != eDSNoErr )
		return NULL;

	siResult = dsFindDirNodes( dsRef, dataBuffer, NULL, eDSLocalNodeNames, &nodeCount, &context );
	if ( siResult != eDSNoErr || nodeCount == 0 )
		goto bail;
	
	siResult = dsGetDirNodeName( dsRef, dataBuffer, 1, &nodeName );
	if ( siResult != eDSNoErr )
		goto bail;
	
	siResult = dsOpenDirNode( dsRef, nodeName, &localNodeRef );
	if ( siResult != eDSNoErr )
		goto bail;
	
	recName = dsBuildListFromStringsPriv( "KerberosKDC", NULL );
	recType = dsBuildListFromStringsPriv( kDSStdRecordTypeConfig, NULL );
	attrTypes = dsBuildListFromStringsPriv( kDS1AttrDistinguishedName, NULL );

	do 
	{
		siResult = dsGetRecordList( localNodeRef, dataBuffer, recName, eDSExact, recType, attrTypes, false, &recCount,
									&context );
		if ( siResult == eDSBufferTooSmall )
		{
			UInt32 bufSize = dataBuffer->fBufferSize;
			dsDataBufferDeallocatePriv( dataBuffer );
			dataBuffer = dsDataBufferAllocatePriv( bufSize * 2 );
		}
	}
	while ( siResult == eDSBufferTooSmall || ((siResult == eDSNoErr) && (recCount == 0) && (context != 0)) );
	
	if ( siResult == eDSNoErr && recCount > 0 )
	{
		siResult = dsGetRecordEntry( localNodeRef, dataBuffer, 1, &attrListRef, &pRecEntry );
		if ( siResult == eDSNoErr && pRecEntry != NULL )
		{
			siResult = dsGetAttributeEntry( localNodeRef, dataBuffer, attrListRef, 1, &valueRef, &pAttrEntry );
			if ( siResult == eDSNoErr && pAttrEntry->fAttributeValueCount > 0 )
			{
				siResult = dsGetAttributeValue( localNodeRef, dataBuffer, 1, valueRef, &pValueEntry );
				if ( siResult == eDSNoErr )
					theRealmStr = dsCStrFromCharacters( pValueEntry->fAttributeValueData.fBufferData,
														pValueEntry->fAttributeValueData.fBufferLength );
									
				if ( pValueEntry != NULL )
					dsDeallocAttributeValueEntry( dsRef, pValueEntry );
			}
			dsCloseAttributeValueList( valueRef );
			if ( pAttrEntry != NULL )
				dsDeallocAttributeEntry( dsRef, pAttrEntry );
		}
		dsCloseAttributeList(attrListRef);
		if ( pRecEntry != NULL )
			dsDeallocRecordEntry( dsRef, pRecEntry );
	}
	
bail:
	if ( recName != NULL ) {
		dsDataListDeallocatePriv( recName );
		free( recName );
	}
	if ( recType != NULL ) {
		dsDataListDeallocatePriv( recType );
		free( recType );
	}
	if ( attrTypes != NULL ) {
		dsDataListDeallocatePriv( attrTypes );
		free( attrTypes );
	}
	if ( nodeName != NULL ) {
		dsDataListDeallocatePriv( nodeName );
		free( nodeName );
	}
	
	if ( dataBuffer != NULL )
		dsDataBufferDeallocatePriv( dataBuffer );
	if ( localNodeRef != 0 )
		dsCloseDirNode( localNodeRef );
	if ( dsRef != 0 )
		dsCloseDirService( dsRef );
	
	return theRealmStr;
}

static mach_port_t	kerberosAutoPort	= MACH_PORT_NULL;

static void lookupKerberosAutoconfigPort( void )
{
	kern_return_t result = bootstrap_look_up( bootstrap_port, (char *)"com.apple.KerberosAutoConfig", &kerberosAutoPort );
	if ( result != KERN_SUCCESS ) {
		syslog( LOG_ALERT, "Error with bootstrap_look_up for com.apple.KerberosAutoConfig - Msg = %s\n", mach_error_string(result) );
	}
}

void LaunchKerberosAutoConfigTool( void )
{
	static pthread_once_t	initPort	= PTHREAD_ONCE_INIT;
	
	pthread_once( &initPort, lookupKerberosAutoconfigPort );
	
	if ( kerberosAutoPort != MACH_PORT_NULL )
	{
		// TODO: this can probably be simplified but for now leave as is
		sIPCMsg aMsg;
		
		aMsg.fHeader.msgh_bits			= MACH_MSGH_BITS( MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND );
		aMsg.fHeader.msgh_size			= sizeof(sIPCMsg) - sizeof( mach_msg_audit_trailer_t );
		aMsg.fHeader.msgh_id			= 0;
		aMsg.fHeader.msgh_remote_port	= kerberosAutoPort;
		aMsg.fHeader.msgh_local_port	= MACH_PORT_NULL;
		
		aMsg.fMsgType	= 0;
		aMsg.fCount		= 1;
		aMsg.fPID		= 0;
		aMsg.fMsgID		= 0;
		aMsg.fOf		= 1;
		
		// tickle the mach init port
		// TODO: should this really be required to start the daemon
		mach_msg( (mach_msg_header_t *)&aMsg, MACH_SEND_MSG | MACH_SEND_TIMEOUT, aMsg.fHeader.msgh_size, 0, MACH_PORT_NULL, 1, MACH_PORT_NULL );
	}
	
} // LaunchKerberosAutoConfigTool

void dsNotifyUpdatedRecord( const char *inModule, const char *inNodeName, const char *inRecType )
{
	char	tempBuffer[256];
	
	// first do the location one, then we'll do the global one.
	strlcpy( tempBuffer, kDSNotifyGlobalRecordUpdatePrefix, sizeof(tempBuffer) );
	strlcat( tempBuffer, inModule, sizeof(tempBuffer) );
	strlcat( tempBuffer, ".", sizeof(tempBuffer) );
	if ( inNodeName != NULL ) {
		strlcat( tempBuffer, inNodeName, sizeof(tempBuffer) );
		strlcat( tempBuffer, ".", sizeof(tempBuffer) );
	}
	strlcat( tempBuffer, inRecType, sizeof(tempBuffer) );
	
	notify_post( tempBuffer );

	// now do a global one only
	strlcpy( tempBuffer, kDSNotifyGlobalRecordUpdatePrefix, sizeof(tempBuffer) );
	strlcat( tempBuffer, inRecType, sizeof(tempBuffer) );

	notify_post( tempBuffer );
}


// ---------------------------------------------------------------------------
//	GenerateRandomComputerPassword
//
//	@discussion
//		Set the password for the record, 20 characters long, using a complex
//		character set.
// ---------------------------------------------------------------------------

char *
GenerateRandomComputerPassword( void )
{
	const int		iPassLength = 20;
	char			*pCompPassword = (char *) calloc( sizeof(char), iPassLength + 1 );
	register char	cTemp;
	
	for ( int iLen = 0; iLen < iPassLength; )
	{
		cTemp = (char)((arc4random() % (0x7e - 0x21)) + 0x21);
		
		// accept printable characters, but no spaces...
		if ( isprint(cTemp) && !isspace(cTemp) )
			pCompPassword[iLen++] = cTemp;
	}
	
	return pCompPassword;
}

