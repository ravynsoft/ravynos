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
 * @header DSTCPEndpoint
 * Interface to TCP Socket endpoint class.
 */

#ifndef __DSTCPEndpoint_h__
#define __DSTCPEndpoint_h__ 1

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>		// struct sockaddr_in

#include "DSNetworkUtilities.h"		// for some constants
#include "SharedConsts.h"
#include "libCdsaCrypt.h"

#define DH_KEY_SIZE		512		/* size of Diffie-Hellman key in bits */
#define DERIVE_KEY_SIZE	128		/* size of derived key in bits */
#define DERIVE_KEY_ALG	CSSM_ALGID_AES
#define DSTCPAuthTag	'DHN2'

enum eKeyState {
	eKeyStateSendPublicKey		= 0,
	eKeyStateGenerateChallenge,
	eKeyStateAcceptResponse,
	eKeyStateAcceptClientKey,
	eKeyStateGenerateResponse,
	
	eKeyStateValidKey
};

// specific to DSTCPEndpoint
const UInt32	kTCPOpenTimeout			= 120;
//KW need to revisit
#ifdef DSSERVERTCP
const UInt32	kTCPRWTimeout			= 60*60*24; //server 24 hour timeout
#else
const UInt32	kTCPRWTimeout			= 60*5; //client 5 min timeout like mach
#endif
const UInt32	kTCPMaxListenBackLog	= 1024;
const UInt32	kTCPErrorBufferLen		= 256;

const UInt32 kDSTCPEndpointMaxMessageSize	= 1024; //used for searching for the TCP message tag
const UInt32 kDSTCPEndpointMessageTagSize	= 4;	//for "DSPX" tag

// ----------------------------------------------------------------------------
// DSTCPEndpoint: implementation of endpoint based on BSD sockets.
// ----------------------------------------------------------------------------

class DSTCPEndpoint : public CIPCVirtualClass
{

public:

	// TCP related exception error codes
	enum eExceptions
	{
		kConnectionLostWarning	= eDSCannotAccessSession,		// We've lost our connection
		kTimeoutError			= eDSServerTimeout,		// Connection timed out
	};

	// timeout constant types
	enum eTimeoutType
	{
        kOpenTimeoutType = 1,
        kRWTimeoutType,
		kDefaultTimeoutType
    };

					DSTCPEndpoint		( const UInt32 inOpenTimeOut = kTCPOpenTimeout, const UInt32 inRdWrTimeOut = kTCPRWTimeout, int inSocket = -1 );
    virtual			~DSTCPEndpoint		( void );

	virtual SInt32	SendMessage			( sComData *inMessage );
	virtual SInt32	GetReplyMessage		( sComData **outMessage );
	SInt32			ClientNegotiateKey	( void );
	SInt32			ServerNegotiateKey	( void *dataBuff, UInt32 dataBuffLen );

	SInt32		ProcessData				( bool bEncrypt, void *inBuffer, UInt32 inBufferLen, void *&outBuffer, UInt32 &outBufferLen );

	// Inline accessors.
	UInt32		GetReverseAddress		( void ) const			{ return mRemoteHostIPAddr; }
	const char *GetReverseAddressString	( void ) const			{ return mRemoteHostIPString; }
	int			GetCurrentConnection	( void ) const			{ return mConnectFD; }
	inline bool	Negotiated				( void )				{ return (fKeyState == eKeyStateValidKey); }

	SInt32		SyncToMessageBody		( const Boolean inStripLeadZeroes, UInt32 *outBuffLen );
	
	SInt32		SendBuffer				( void *inBuffer, UInt32 inLength );
	
	Boolean		Connected				( void ) const ;
	SInt32		ConnectTo ( struct addrinfo *inAddrInfo ); //for client side
	void		CloseConnection			( void );
	void		GetReverseAddressString	( char *ioBuffer, const int inBufferSize ) const ;
	UInt32		GetRemoteHostIPAddress	( void ) { return mRemoteHostIPAddr; }
	
	in_port_t	GetRemoteHostPort		( void ) { return ( ntohs(mRemoteSockAddr.sin_port) ); }
	sockaddr *	GetRemoteSockAddr		( void ) { return (sockaddr *) &mRemoteSockAddr; }

	sComProxyData*  AllocToProxyStruct  ( sComData *inDataMsg );
	sComData*		AllocFromProxyStruct( sComProxyData *inProxyDataMsg );

protected:
	UInt32			DoTCPRecvFrom			( void *ioBuffer, const UInt32 inBufferSize );

private:
		
	/**** Instance methods accessible only to class. ****/
	int			DoTCPOpenSocket			( void );
	int			SetSocketOption			( const int inSocket, const int inSocketOption);
	int			DoTCPCloseSocket		( const int inSockFD );

protected:
	// network information
	struct sockaddr_in	mMySockAddr;	

	// remote host network information
	UInt32				mRemoteHostIPAddr;		// in network order
	IPAddrStr 			mRemoteHostIPString;	// IP address string
	struct sockaddr_in	mRemoteSockAddr;

	int					mConnectFD;
		
	// buffers
	char			   *mErrorBuffer;

	// states
	Boolean				mWeHaveClosed;

	// Timeouts
	int					mOpenTimeout;	// time out for opening connection
	int					mRWTimeout;		// time out for reading and writing
	int					mDefaultTimeout;
	
private:
	CSSM_CSP_HANDLE 	fcspHandle;
	
	eKeyState			fKeyState;
	
	CSSM_DATA			fParamBlock;
	CSSM_KEY			fPrivateKey;
	CSSM_KEY			fPublicKey;
	CSSM_KEY			fDerivedKey;
	uint32_t			fChallengeValue;
	
	static int32_t		mMessageID;		// this is used to track per-message ID globally for all remote messages
};

#endif // __DSTCPEndpoint_h__
