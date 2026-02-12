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
 * @header CClientEndPoint
 * Mach endpoint for DirectoryService Framework.
 */
#include <stdio.h>
#include <stdlib.h>				// for malloc()
#include <string.h>
#include <unistd.h>			   	// for getpid()
#include <time.h>				// for time()
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include <syslog.h>
#include <bsm/libbsm.h>

#include "CClientEndPoint.h"
#include "DirServicesTypes.h"
#include "DirServicesConst.h"
#include "DirServicesPriv.h"
#include "PrivateTypes.h"
#include "DSCThread.h"
#ifndef DISABLE_SEARCH_PLUGIN
#include "DirectoryServiceMIG.h"
#endif

#define kMsgSize	sizeof( sComData )
#define kIPCMsgSize	sizeof( sIPCMsg )

//------------------------------------------------------------------------------
//	* CClientEndPoint:
//
//------------------------------------------------------------------------------

CClientEndPoint::CClientEndPoint ( const char *inSrvrName )
{
	if ( inSrvrName != nil )
		fServiceName = strdup( inSrvrName );
	else
		fServiceName = strdup( kDSStdMachPortName );
	
	fReplyMsg = NULL;
	fServicePort = MACH_PORT_NULL;
	fSessionPort = MACH_PORT_NULL;
	
} // CClientEndPoint


//------------------------------------------------------------------------------
//	* ~CClientEndPoint:
//
//------------------------------------------------------------------------------

CClientEndPoint::~CClientEndPoint ( void )
{
	DSFree( fServiceName );
	DSFree( fReplyMsg );
	
	Disconnect();
	
	if ( fServicePort != MACH_PORT_NULL )
	{
		mach_port_deallocate( mach_task_self(), fServicePort );
		fServicePort = MACH_PORT_NULL;
	}
	
} // ~CClientEndPoint

//------------------------------------------------------------------------------
//	* Connect
//
//------------------------------------------------------------------------------

SInt32 CClientEndPoint::Connect( void )
{
#ifndef DISABLE_SEARCH_PLUGIN
	SInt32			siResult	= eDSNoErr;
	kern_return_t	kr			= KERN_FAILURE;
	
tryAgain:
	
	if ( fServicePort == MACH_PORT_NULL )
	{
		kr = bootstrap_look_up( bootstrap_port, fServiceName, &fServicePort );
		if ( kr != KERN_SUCCESS ) {
			siResult = eServerNotRunning;
			LOG3( kStdErr, "*** failed bootstrap_look_up: %s at: %d: Msg = %s\n", __FILE__, __LINE__, mach_error_string( kr ) );
		}
	}
	
	if ( siResult == eDSNoErr && fSessionPort == MACH_PORT_NULL )
	{
		audit_token_t atoken;
		uid_t euid = -1;
		
		kr = dsmig_create_api_session( fServicePort, &fSessionPort, &atoken );
		
		switch ( kr )
		{
			case MACH_SEND_INVALID_DEST:
				// means bootstrap port for server is no longer valid, may have been unloaded via launchctl
				mach_port_deallocate( mach_task_self(), fServicePort );
				fServicePort = MACH_PORT_NULL;
				goto tryAgain;
			case KERN_SUCCESS:
				audit_token_to_au32( atoken, NULL, &euid, NULL, NULL, NULL, NULL, NULL, NULL );
				if ( euid != 0 ) {
					dsmig_close_api_session( fSessionPort );
					mach_port_deallocate( mach_task_self(), fSessionPort );
					fSessionPort = MACH_PORT_NULL;
					siResult = eServerSendError;
					syslog( LOG_ALERT, "Attempt to connect to a daemon that is not running as root (euid != 0)" );
				}
				break;
			default:
				siResult = eServerSendError;
				break;
		}
	}
	
	return siResult;
#else
	return eServerSendError;
#endif
}

//------------------------------------------------------------------------------
//	* Disconnect
//
//------------------------------------------------------------------------------

void CClientEndPoint::Disconnect( void )
{
#ifndef DISABLE_SEARCH_PLUGIN
	if ( fSessionPort != MACH_PORT_NULL )
	{
		dsmig_close_api_session( fSessionPort );
		mach_port_deallocate( mach_task_self(), fSessionPort );
		fSessionPort = MACH_PORT_NULL;
	}
#endif
}

//------------------------------------------------------------------------------
//	* SendServerMessage
//
//------------------------------------------------------------------------------

SInt32 CClientEndPoint::SendMessage( sComData *inMsg )
{
	SInt32		result		= eServerSendError;
	bool		bTryAgain	= false;
	
#ifndef DISABLE_SEARCH_PLUGIN
	do
	{
		kern_return_t			kr			= MACH_SEND_INVALID_DEST;
		mach_msg_type_name_t	serverPoly	= MACH_MSG_TYPE_COPY_SEND; 
		
		// let's do the call
		if( fSessionPort != MACH_PORT_NULL )
		{
			char					replyFixedBuffer[ kMaxFixedMsg ];
			
			sComDataPtr				replyFixedData	= (sComDataPtr) replyFixedBuffer;
			mach_msg_type_number_t	replyFixedLen	= 0;
			vm_address_t			sendData		= 0;
			mach_msg_type_number_t	sendLen			= sizeof(sComData) + inMsg->fDataLength - 1;
			vm_address_t			replyData		= 0;
			mach_msg_type_number_t	replyLen		= 0;
			
			if( inMsg->fDataLength <= kMaxFixedMsgData )
			{
				kr = dsmig_api_call( fSessionPort, serverPoly, inMsg, sendLen, 0, 0, replyFixedData, &replyFixedLen, &replyData, &replyLen );
			}
			else
			{
				vm_read( mach_task_self(), (vm_address_t)inMsg, sendLen, &sendData, &sendLen );
				
				kr = dsmig_api_call( fSessionPort, serverPoly, NULL, 0, sendData, sendLen, replyFixedData, &replyFixedLen, &replyData, &replyLen );
				
				// let's deallocate the memory we allocated... if we failed..
				if( kr == MACH_SEND_INVALID_DEST )
				{
					vm_deallocate( mach_task_self(), sendData, sendLen );
				}
			}
			
			if( kr == KERN_SUCCESS )
			{
				sComDataPtr		pComData = NULL;
				UInt32			uiLength = 0;
				
				// if we have OOL data, we need to copy it and deallocate it..
				if( replyFixedLen )
				{
					pComData = (sComDataPtr) replyFixedData;
					uiLength = replyFixedLen;
				}
				else if( replyLen ) 
				{
					pComData = (sComDataPtr) replyData;
					uiLength = replyLen;
				}
				
				// if this is a valid reply..
				if( pComData != NULL && pComData->fDataLength == (uiLength - (sizeof(sComData) - 1)) )
				{
					fReplyMsg = (sComData *) calloc( sizeof(char), sizeof(sComData) + pComData->fDataSize );
					
					bcopy( pComData, fReplyMsg, uiLength );
					
					result = eDSNoErr;
				}
				
				// if we had reply data OOL, let's free it appropriately...
				if( replyLen )
				{
					vm_deallocate( mach_task_self(), replyData, replyLen );			
				}
			}
		}
		
		if( bTryAgain == false && kr == MACH_SEND_INVALID_DEST )
		{
			Disconnect();
			
			if ( Connect() == eDSNoErr )
				bTryAgain = true;
		}
		else
		{
			bTryAgain = false;
		}
		
	} while( bTryAgain );
#endif
		
	return result;
} // SendServerMessage


//------------------------------------------------------------------------------
//	* GetReplyMessage
//
//------------------------------------------------------------------------------

SInt32 CClientEndPoint::GetReplyMessage( sComData **outMsg )
{
	SInt32	siResult = eServerReplyError;
	
	if ( fReplyMsg != NULL )
	{
		DSFree( *outMsg );

		*outMsg = fReplyMsg;
		fReplyMsg = NULL;
		
		siResult = eDSNoErr;
	}

	return siResult;

} // GetReplyMessage

