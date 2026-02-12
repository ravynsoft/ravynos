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
 * Implementation of TCP Socket endpoint class.
 */

/*
	Note: all network addresses in method parameters and return values
	are in host byte order - they are converted to network byte order
	inside the methods for socket calls.
		 
	Note2: need to be aware of which routines are FW or Server exclusive
	for what type of logging
*/

#include <string.h>
#include <errno.h>			// system call error numbers
#include <unistd.h>			// for select call 
#include <stdlib.h>			// for calloc()
#include <poll.h>
#include <sys/time.h>		// for struct timeval
#include <libkern/OSAtomic.h>
#include <sys/socket.h>
#include <netdb.h>

#include "DSCThread.h"		// for GetCurThreadRunState()
#include "DSTCPEndpoint.h"
#ifdef DSSERVERTCP
	#include "CLog.h"
#else
	#define DbgLog(...)
#endif
#include "SharedConsts.h"	// for sComData
#include "DirServicesConst.h"
#include "DSTCPEndian.h"
#include "DSSwapUtils.h"

int32_t			DSTCPEndpoint::mMessageID	= 0;
static uint8	paramBlob[]	= { \
								0x30, 0x52, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x03, 0x30, 0x46, 0x02, 0x41,\
								0x00, 0xa0, 0xd4, 0x42, 0xd5, 0x68, 0x08, 0x94, 0xc9, 0xef, 0xb7, 0x18, 0x9c, 0x0b, 0x72, 0x53,\
								0xac, 0x8a, 0x7b, 0xc2, 0x40, 0x17, 0x96, 0x29, 0xd1, 0xf2, 0x96, 0xe8, 0x2b, 0x4e, 0x48, 0xaf,\
								0x59, 0xbe, 0x29, 0xc4, 0x9b, 0x52, 0xda, 0x05, 0x18, 0x29, 0x73, 0xff, 0xd5, 0x26, 0x47, 0x53,\
								0x54, 0x79, 0xf4, 0x39, 0x96, 0x6f, 0x61, 0x5e, 0xe6, 0xfc, 0x92, 0x7d, 0xf4, 0x20, 0x6e, 0xa9,\
								0xa3, 0x02, 0x01, 0x02 };

// ----------------------------------------------------------------------------
//	* DSTCPEndpoint Class (static) Methods
// ----------------------------------------------------------------------------
#pragma mark **** Class Methods ****

// ----------------------------------------------------------------------------
//	* DSTCPEndpoint Instance Methods
// ----------------------------------------------------------------------------
#pragma mark **** Instance Methods ****

// ----------------------------------------------------------------------------
//	* DSTCPEndpoint ()
//
// ----------------------------------------------------------------------------

DSTCPEndpoint::DSTCPEndpoint (	const UInt32	inOpenTimeout,
								const UInt32	inRWTimeout,
								int				inSocket ) :
	mRemoteHostIPAddr (0),
	mConnectFD (inSocket),
	mWeHaveClosed (false),
	mOpenTimeout (inOpenTimeout),
	mRWTimeout (inRWTimeout),
	mDefaultTimeout(inRWTimeout),
	fKeyState(eKeyStateAcceptClientKey)

{
	memset( &mMySockAddr, 0, sizeof(mMySockAddr) );
	mRemoteHostIPString[0] = '\0';
	memset( &mRemoteSockAddr, 0, sizeof(mRemoteSockAddr) );
	
	bzero(&fPrivateKey, sizeof(fPrivateKey));
	bzero(&fPublicKey, sizeof(fPublicKey));
	bzero(&fDerivedKey, sizeof(fDerivedKey));
		  
	if ( cdsaCspAttach(&fcspHandle) == CSSM_OK )
	{
		//set the param block
		fParamBlock.Data	= paramBlob;
		fParamBlock.Length	= sizeof(paramBlob);
	}

} // DSTCPEndpoint Constructor


// ----------------------------------------------------------------------------
//	* ~DSTCPEndpoint ()
//
// ----------------------------------------------------------------------------

DSTCPEndpoint::~DSTCPEndpoint ( void )
{
	// make sure we safely close the connection

	try
	{
		if ( mWeHaveClosed == false )
		{
			DoTCPCloseSocket( mConnectFD );
		}
	}
	catch( ... )
	{
	}

	cdsaFreeKey( fcspHandle, &fPrivateKey );
	cdsaFreeKey( fcspHandle, &fPublicKey );
	cdsaFreeKey( fcspHandle, &fDerivedKey );
	cdsaCspDetach( fcspHandle );	

} // ~DSTCPEndpoint


// ----------------------------------------------------------------------------
//	* ConnectTo () *****ONLY used by CMessaging class
// 
//		- Make a connection to another socket defined by the IP address and
//			port number
// ----------------------------------------------------------------------------

SInt32 DSTCPEndpoint::ConnectTo ( struct addrinfo *inAddrInfo )
{
	int					err = eDSNoErr;
	int					result = 0;
	int					sockfd;
	time_t				timesUp;
	struct sockaddr		*serverAddr = inAddrInfo->ai_addr;
	int					rc = eDSNoErr;
	bool				releaseZeroFD = false;

	do //this is an INTENTIONAL temporary leak of the socket if it is zero since sockfd zero seems to always fail eventually
	{
		sockfd = DoTCPOpenSocket();
		if ( sockfd < 0 )
		{
			return( eDSTCPSendError );
		}

		mConnectFD = sockfd;
	
		// although connect has its own timeout, to enable longer time out we use mOpenTimeout
		timesUp = ::time(NULL) + mOpenTimeout;
		while ( ::time(NULL) < timesUp )
		{
			result = ::connect( mConnectFD, serverAddr, serverAddr->sa_len );
	
			if ( result == -1 )
			{
				err = errno;
				switch ( err )
				{
					case ETIMEDOUT:
						continue;	// returned from connect's timeout, keep trying until we time out
						break;
	
					case ECONNREFUSED:
						LOG2( kStdErr, "ConnectTo: connect() error: %d, %s", err, strerror(err) );
						return( eDSIPUnreachable );					
						break;
						
					default:	// other errors are serious
						LOG2( kStdErr, "ConnectTo: connect() error: %d, %s", err, strerror(err) );
						return( eDSTCPSendError );
						break;
				} // switch
			}
			else
			{ // connect succeeded
				if ( (sockfd != 0) && (releaseZeroFD) ) //cleanup the intentional temporary leak of the zero FD
				{
					int rcSock = 0;
					rcSock = ::close( 0 );
					if ( rcSock == -1 )
					{
						err = errno;
#ifdef DSSERVERTCP
						DbgLog( kLogTCPEndpoint, "DoTCPCloseSocket: close() on unused socket 0 failed with error %d: %s", err, strerror(err) );
#else
						LOG2( kStdErr, "DoTCPCloseSocket: close() on unused socket 0 failed with error %d: %s", err, strerror(err) );
#endif
					}
					else
					{
#ifdef DSSERVERTCP
						DbgLog( kLogTCPEndpoint, "DoTCPCloseSocket: close() on unused socket 0" );
#else
						LOG( kStdErr, "DoTCPCloseSocket: close() on unused socket 0" );
#endif
					}
				}
				break;
			}
		}
		if (sockfd == 0)
		{
			releaseZeroFD = true;
		}
	} while (sockfd == 0);

	if ( result == 0 )
	{ 
		// connection established, now we can safely copy the network information data members
		// mActive = true;
		memcpy(&mRemoteSockAddr, &serverAddr, sizeof(mRemoteSockAddr));
		rc = this->SetSocketOption( mConnectFD, SO_NOSIGPIPE );
		if ( rc != 0 )
		{
			return( eDSTCPSendError );
		}
		LOG2( kStdErr, "Established TCP connection to %d on port %d.", inIPAddress, inPort );
		return(eDSNoErr);
	}
	else
	{
		// may have got to here by timeout
		LOG2( kStdErr, "Unable to connect to %d on port %d.", inIPAddress, inPort );
		return(eDSServerTimeout);
	}
} // ConnectTo

// ----------------------------------------------------------------------------
//	* GetReverseAddressString ()
//
// ----------------------------------------------------------------------------

void DSTCPEndpoint::GetReverseAddressString (	char	*ioBuffer,
												const int	inBufferLen) const
{
	if ( ioBuffer != NULL )
	{
		::strncpy (ioBuffer, mRemoteHostIPString, inBufferLen);
	}
} // GetReverseAddressString


// ----------------------------------------------------------------------------
//	* Connected ()
//
//		- Is the socket connection still open?
// ----------------------------------------------------------------------------

Boolean DSTCPEndpoint::Connected ( void ) const
{
	struct pollfd fdToPoll;
	int result;

	if ( mConnectFD == -1 )
		return false;
	
	fdToPoll.fd = mConnectFD;
	fdToPoll.events = POLLSTANDARD;
	fdToPoll.revents = 0;
	result = poll( &fdToPoll, 1, 0 );
	if ( result == -1 )
		return false;
	return ( (fdToPoll.revents & POLLHUP) == 0 );
} // Connected


// ----------------------------------------------------------------------------
//	* CloseConnection()
//
// ----------------------------------------------------------------------------

void DSTCPEndpoint::CloseConnection ( void )
{
	if ( mConnectFD > 0 )
	{
		int err = this->DoTCPCloseSocket( mConnectFD );
		if ( err == eDSNoErr )
		{
			mConnectFD = 0;
			mWeHaveClosed = true;
		}
	}
}


// ----------------------------------------------------------------------------
//	Private Methods
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//	* DoTCPOpenSocket ()
//
//		- Open a new socket
// ----------------------------------------------------------------------------

int DSTCPEndpoint::DoTCPOpenSocket (void)
{
	int		err;
	int		sockfd;

#ifdef DSSERVERTCP
	DbgLog( kLogTCPEndpoint, "Open socket." );
#else
	LOG( kStdErr, "Open socket." );
#endif
	sockfd = ::socket( AF_INET, SOCK_STREAM, 0 );

	if ( sockfd == -1 )
	{
		err = errno;
#ifdef DSSERVERTCP
		DbgLog( kLogTCPEndpoint, "DoTCPOpenSocket: socket() error %d: %s", err, strerror(err) );
#else
		LOG2( kStdErr, "DoTCPOpenSocket: Unable to open a socket with error %d: %s", err, strerror(err) );
#endif
	}
	err = errno;
	if (err != 0)
	{
#ifdef DSSERVERTCP
		DbgLog( kLogTCPEndpoint, "DoTCPOpenSocket: socket error %d: %s with sockfd %d", err, strerror(err), sockfd );
#else
		LOG3( kStdErr, "DoTCPOpenSocket: socket error %d: %s with sockfd %d", err, strerror(err), sockfd );
#endif
	}

	return( sockfd );
}

// ----------------------------------------------------------------------------
//	* SetSocketOption ()
//
//		- Set the socket level option
// ----------------------------------------------------------------------------

int DSTCPEndpoint::SetSocketOption ( const int inSocket, const int inSocketOption )
{
	int rc		= 0;
	int err		= 0;
	int val		= 1;
	int len		= sizeof(val);

	if ( inSocket != 0 )
	{
		if ( inSocket != mConnectFD )
		{
#ifdef DSSERVERTCP
			ErrLog( kLogTCPEndpoint, "SetSocketOption: invalid socket: %d", inSocket );
#else
			LOG1( kStdErr, "SetSocketOption: invalid socket: %d", inSocket );
#endif
			return( -1 );
		}

		rc = ::setsockopt( inSocket, SOL_SOCKET, inSocketOption, &val, len );
		if ( rc != 0 ) 
		{
			err = errno;

#ifdef DSSERVERTCP
			DbgLog( kLogError, "Unable to set socket option: Message: \"%s\", Error: %d", strerror(err), err );
#else
			LOG2( kStdErr, "Unable to set socket option: Message: \"%s\", Error: %d", strerror(err), err );
#endif
		}
	}

	return( 0 );

} // SetSocketOption


// ----------------------------------------------------------------------------
//	* DoTCPCloseSocket ()
//
// ----------------------------------------------------------------------------

int DSTCPEndpoint::DoTCPCloseSocket ( const int inSockFD )
{
	int err = eDSNoErr;
	int rc	= 0;

	if ( inSockFD <= 0 )
	{
		return( eDSNoErr );
	}

#ifdef DSSERVERTCP
	DbgLog( kLogTCPEndpoint, "Close socket." );
#endif
	rc = ::close( inSockFD );
	if ( rc == -1 )
	{
		err = errno;
#ifdef DSSERVERTCP
		DbgLog( kLogTCPEndpoint, "DoTCPCloseSocket: close() on socket %d failed with error %d: %s", inSockFD, err, strerror(err) );
#else
		LOG3( kStdErr, "DoTCPCloseSocket: close() on socket %d failed with error %d: %s", inSockFD, err, strerror(err) );
#endif
	}

	return( err );

} // DoTCPCloseSocket


// ----------------------------------------------------------------------------
//	* DoTCPRecvFrom ()
// ----------------------------------------------------------------------------

UInt32 DSTCPEndpoint::DoTCPRecvFrom ( void *ioBuffer, const UInt32 inBufferSize )
{
	int				rc;
	int				err;
	int				bytesRead = 0;
	fd_set			readSet;
	struct timeval	tvTimeout		= { mRWTimeout, 0 };
	struct timeval	tvTimeoutTime	= { mRWTimeout, 0 };
	time_t			timeoutTime;

	timeoutTime = ::time( NULL ) + mRWTimeout;

	::gettimeofday (&tvTimeoutTime, NULL);
	tvTimeoutTime.tv_sec += mRWTimeout;
	
	do {
		FD_ZERO( &readSet );
		FD_SET( mConnectFD, &readSet );

		rc = ::select( mConnectFD+1, &readSet, NULL, NULL, &tvTimeout );

		// Recompute the timeout and break if timeout exceeded.
		if ( (rc == -1) && (EINTR == errno) )
		{
			struct timeval	tvNow;
			::gettimeofday( &tvNow, NULL );
			timersub( &tvTimeoutTime, &tvNow, &tvTimeout );
			if ( tvTimeout.tv_sec < 0 )
			{
#ifdef DSSERVERTCP
				DbgLog( kLogTCPEndpoint, "DoTCPRecvFrom: connection timeout?" );
#else
				LOG( kStdErr, "DoTCPRecvFrom: connection timeout?" );
#endif
				throw( (SInt32)eDSTCPReceiveError );
			}
		}
	} while ( (rc == -1) && (EINTR == errno) );


	if ( rc == 0 )
	{
#ifdef DSSERVERTCP
			DbgLog( kLogTCPEndpoint, "DoTCPRecvFrom: timed out waiting for response." );
#else
			LOG( kStdErr, "DoTCPRecvFrom: timed out waiting for response." );
#endif
			throw( (SInt32)kTimeoutError );
	}
	else if ( rc == -1 )
	{
 		err = errno;
#ifdef DSSERVERTCP
		DbgLog( kLogTCPEndpoint, "DoTCPRecvFrom: select() error %d: %s", err, strerror(err) );
#else
		LOG2( kStdErr, "DoTCPRecvFrom: select() error %d: %s", err, strerror(err) );
#endif
		throw((SInt32)eDSTCPReceiveError);
	} 
	else if ( FD_ISSET(mConnectFD, &readSet) )
	{
		// socket is ready for read - blocks until all read
		//KW need a socket level timeout for this read to complete ie. setsocketopt call with SO_RCVTIMEO
		//bytesRead = ::recvfrom( mConnectFD, ioBuffer, inBufferSize, MSG_DONTWAIT, NULL, NULL );
		do
		{
			bytesRead = ::recvfrom( mConnectFD, ioBuffer, inBufferSize, MSG_WAITALL, NULL, NULL );
	
		} while ( (bytesRead == -1) && (errno == EAGAIN) );
		
		if ( bytesRead == 0 )
		{
			// connection closed from the other side
			err = errno;
#ifdef DSSERVERTCP
			DbgLog( kLogTCPEndpoint, "DoTCPRecvFrom: connection closed by peer - error is %d", err );
#else
			LOG1( kStdErr, "DoTCPRecvFrom: connection closed by peer - error is %d", err );
#endif
			throw( (SInt32)eDSTCPReceiveError );
		}
		else if ( bytesRead == -1 )
		{
			err = errno;
#ifdef DSSERVERTCP
			DbgLog( kLogTCPEndpoint, "DoTCPRecvFrom: recvfrom error %d: %s", err, strerror(err) );
#else
			LOG2( kStdErr, "DoTCPRecvFrom: recvfrom error %d: %s", err, strerror(err) );
#endif
			throw( (SInt32)eDSTCPReceiveError );
		}
		else
		{
#ifdef DSSERVERTCP
			DbgLog( kLogTCPEndpoint, "DoTCPRecvFrom(): received %d bytes with endpoint %ld and connectFD %d", bytesRead, (long)this, mConnectFD );
#else
			LOG3( kStdErr, "DoTCPRecvFrom(): received %d bytes with endpoint %ld and connectFD %d", bytesRead, (long)this, mConnectFD );
#endif
		}
	}

	return( (UInt32)bytesRead );

} // DoTCPRecvFrom


// ----------------------------------------------------------------------------
// * SyncToMessageBody():	read tag and buffer length from the endpoint
//							returns the buffer length
// ----------------------------------------------------------------------------

SInt32 DSTCPEndpoint::SyncToMessageBody(const Boolean inStripLeadZeroes, UInt32 *outBuffLen)
{
	UInt32			index = 0;
	UInt32			readBytes = 0;
	UInt32			newLen = 0;
	UInt32			curIndex = kDSTCPEndpointMessageTagSize;
	char		   *ourBuffer;
	UInt32			buffLen = 0;
	SInt32			result	= eDSNoErr;

	ourBuffer = (char *) calloc(kDSTCPEndpointMaxMessageSize, 1);
	
	try 
	{
		readBytes = DoTCPRecvFrom(ourBuffer, kDSTCPEndpointMessageTagSize);
		
		if (readBytes != kDSTCPEndpointMessageTagSize)
		{
			//couldn't read even the minimum tag size so return zero
			free(ourBuffer);
			*outBuffLen = 0;
#ifdef DSSERVERTCP
			DbgLog( kLogTCPEndpoint, "SyncToMessageBody: attempted read of %d bytes failed with %d bytes read", kDSTCPEndpointMessageTagSize, readBytes );
#else
			LOG2( kStdErr, "SyncToMessageBody: attempted read of %d bytes failed with %d bytes read", kDSTCPEndpointMessageTagSize, readBytes );
#endif
			return eDSTCPReceiveError;
		}
	}
	catch( SInt32 err )
	{
		if (ourBuffer != nil)
		{
			free(ourBuffer);
		}
#ifdef DSSERVERTCP
		DbgLog( kLogTCPEndpoint, "SyncToMessageBody: attempted read of %d bytes failed in DoTCPRecvFrom with error %d", kDSTCPEndpointMessageTagSize, err );
#else
		LOG2( kStdErr, "SyncToMessageBody: attempted read of %d bytes failed in DoTCPRecvFrom with error %d", kDSTCPEndpointMessageTagSize, err );
#endif
		return eDSTCPReceiveError;
	}
	
	//TODO need to handle corrupted data? ie. continue searching for tag?
	if (inStripLeadZeroes)
	{
		// strip any leading zeroes
		for ( index=0; (index < kDSTCPEndpointMessageTagSize) && (ourBuffer[index] == 0x00); index++ )
		{
			readBytes--;
		}
	
		try
		{
			//keep reading one at a time if we encounter any leading zeroes
			//don't expect this to ever happen
			while ( (readBytes < kDSTCPEndpointMessageTagSize) && (curIndex < kDSTCPEndpointMaxMessageSize) )
			{
				newLen = DoTCPRecvFrom(ourBuffer+curIndex, 1);
				if (newLen != 1)
				{
					//couldn't read even one byte so return zero
					free(ourBuffer);
					*outBuffLen = 0;
#ifdef DSSERVERTCP
					DbgLog( kLogTCPEndpoint, "SyncToMessageBody: align frame by skipping leading zeroes - attempted read of one byte failed with %d bytes read", newLen );
#else
					LOG1( kStdErr, "SyncToMessageBody: align frame by skipping leading zeroes - attempted read of one byte failed with %d bytes read", newLen );
#endif
					return eDSTCPReceiveError;
				}
				if (ourBuffer[curIndex] != 0x00)
				{
					readBytes++;
				}
				curIndex++;
			}
		}		
		catch( SInt32 err )
		{
			if (ourBuffer != nil)
			{
				free(ourBuffer);
			}
#ifdef DSSERVERTCP
			DbgLog( kLogTCPEndpoint, "SyncToMessageBody: align frame by skipping leading zeroes - failed in DoTCPRecvFrom with error %l", err );
#else
			LOG1( kStdErr, "SyncToMessageBody: align frame by skipping leading zeroes - failed in DoTCPRecvFrom with error %l", err );
#endif
			return eDSTCPReceiveError;
		}
	}

	//check if we found the tag we are looking for
	if ( (readBytes == kDSTCPEndpointMessageTagSize) && (strncmp(ourBuffer+curIndex-kDSTCPEndpointMessageTagSize,"DSPX",kDSTCPEndpointMessageTagSize) == 0) )
	{
		try
		{
			//now get the buffer length
			//check here to determine if buffLen is at least sizeof(sComData)
			newLen = DoTCPRecvFrom(&buffLen , 4);
			if (newLen != 4) //|| (buffLen < sizeof(sComData)) )
			{
#ifdef DSSERVERTCP
				DbgLog( kLogTCPEndpoint, "SyncToMessageBody: get the buffer length - attempted read of four bytes failed with %d bytes read", newLen );
#else
				LOG1( kStdErr, "SyncToMessageBody: get the buffer length - attempted read of four bytes failed with %d bytes read", newLen );
#endif
				*outBuffLen = 0;
			}
			else
			{
				*outBuffLen = ntohl(buffLen);
			}
		}		
		catch( SInt32 err )
		{
			if (ourBuffer != nil)
			{
				free(ourBuffer);
			}
#ifdef DSSERVERTCP
			DbgLog( kLogTCPEndpoint, "SyncToMessageBody: get the buffer length - failed in DoTCPRecvFrom with error %l", err );
#else
			LOG1( kStdErr, "SyncToMessageBody: get the buffer length - failed in DoTCPRecvFrom with error %l", err );
#endif
			return eDSTCPReceiveError;
		}
	}
	
	free(ourBuffer);
	return result;

} // SyncToMessageBody


//------------------------------------------------------------------------------
//	* SendBuffer
//
//------------------------------------------------------------------------------

SInt32 DSTCPEndpoint::SendBuffer ( void *inBuffer, UInt32 inLength )
{
	SInt32				result		= eDSNoErr;
	UInt32				sendBuffLen = sizeof("DSPX") + sizeof(UInt32) + inLength;
	char				*sendBuff	= (char *) calloc( sendBuffLen, sizeof(char) );
	uint32_t			offset		= 0;
	
	bcopy( "DSPX", sendBuff, kDSTCPEndpointMessageTagSize );
	*((UInt32 *) (sendBuff + kDSTCPEndpointMessageTagSize)) = htonl( inLength );
	bcopy( inBuffer, sendBuff + kDSTCPEndpointMessageTagSize + sizeof(UInt32), inLength);

	// TODO: use dispatch, but not yet (wait until we redo this class to use it completely)
	do
	{
		ssize_t sentBytes = send( mConnectFD, sendBuff + offset, sendBuffLen - offset, 0 );
		if ( sentBytes < 0 ) {
			switch ( errno ) {
				case EINTR:
				case EAGAIN:
					break;
				default:
					DSFree( sendBuff );
					return eDSTCPSendError;
			}
		}
		else {
			offset += sentBytes;
		}
		
		if ( offset < sendBuffLen ) {
			
			fd_set	writeSet;
			struct timeval tvTimeout = { 10, 0 };
			
			FD_ZERO( &writeSet );
			FD_SET( mConnectFD, &writeSet );
			
			select( mConnectFD+1, NULL, &writeSet, NULL, &tvTimeout );
			continue;
		}
		
		break;
	} while ( 1 );
	
	DSFree( sendBuff );

	return result;

} // SendBuffer


//------------------------------------------------------------------------------
//	* SendMessage
//
//------------------------------------------------------------------------------

SInt32 DSTCPEndpoint::SendMessage( sComData *inMsg )
{
	UInt32			messageSize = 0;
	sComProxyData  *inProxyMsg  = nil;
	SInt32			sendResult  = eDSNoErr;
	void			*outBuffer	= NULL;
	UInt32			outLength	= 0;
	
	inProxyMsg = AllocToProxyStruct( (sComData *)inMsg );
	
	//let us only send the data that is present and not the entire buffer
	inProxyMsg->fDataSize = inProxyMsg->fDataLength;
	messageSize = sizeof(sComProxyData) + inProxyMsg->fDataLength;
	
	inProxyMsg->fIPAddress = mRemoteHostIPAddr;
	inProxyMsg->fPID = ntohs( mRemoteSockAddr.sin_port );
	inProxyMsg->fMsgID = OSAtomicIncrement32( &mMessageID );
	
	if ( inProxyMsg->type.msgt_translate != 2 ) {
		SwapProxyMessage( inProxyMsg, kDSSwapHostToNetworkOrder );
	}

	ProcessData( true, inProxyMsg, messageSize, outBuffer, outLength );

	sendResult = SendBuffer( outBuffer, outLength );
	
	DSFree( inProxyMsg );
	DSFree( outBuffer );
	
	return sendResult;
} // SendMessage


//------------------------------------------------------------------------------
//	* GetReplyMessage
//------------------------------------------------------------------------------

SInt32 DSTCPEndpoint::GetReplyMessage( sComData **outMsg )
{
	SInt32					siResult		= eDSNoErr;
	UInt32					buffLen			= 0;
	UInt32					readBytes 		= 0;
	void				   *inBuffer		= nil;
	UInt32					inLength		= 0;
	sComProxyData		   *outProxyMsg		= nil;

	//need to read a tag and then a buffer length
	siResult = SyncToMessageBody(true, &inLength);
	
	if ( (siResult == eDSNoErr) && (inLength != 0) )
	{
		try
		{
			//go ahead and read the message body of length inLength
			//put the message data into inBuffer
			inBuffer = (void *)calloc(1,inLength);
			readBytes = DoTCPRecvFrom(inBuffer, inLength);
			if (readBytes != inLength)
			{
				//TODO need to recover somehow
				LOG( kStdErr, "GetServerReply: Couldn't read entire message block" );
				siResult = eDSTCPReceiveError;
			}
			else
			{
				void *tmpOutMsg = nil;
				ProcessData( false, inBuffer, inLength, tmpOutMsg, buffLen );
				outProxyMsg = (sComProxyData *)tmpOutMsg;
				if (buffLen == 0)
				{
					free(outProxyMsg);
					outProxyMsg	= (sComProxyData *)inBuffer;
					inBuffer	= nil;
					buffLen		= inLength;
				}
			}
		}
		catch( SInt32 err )
		{
			siResult = eDSTCPReceiveError;
		}
	}
	
	if (inBuffer != nil)
	{
		free(inBuffer);
		inBuffer = nil;
	}
	
    if (outProxyMsg != nil)
    {
		if ( outProxyMsg->type.msgt_translate != 2 ) {
			SwapProxyMessage( outProxyMsg, kDSSwapNetworkToHostOrder );
		}
		
		*outMsg = AllocFromProxyStruct( outProxyMsg );
		free(outProxyMsg);
		outProxyMsg = nil;
    }

	return( siResult );

} // GetReplyMessage

//------------------------------------------------------------------------------
//	* ClientNegotiateKey
//------------------------------------------------------------------------------

SInt32 DSTCPEndpoint::ClientNegotiateKey( void )
{
	SInt32	result;
	void	*recvBuff		= NULL;
	UInt32	recvBuffLen		= 0;
	void	*sendBuff		= NULL;
	UInt32	sendBuffLen		= 0;
	
	fKeyState = eKeyStateSendPublicKey;
	
	do
	{
		result = ProcessData( true, recvBuff, recvBuffLen, sendBuff, sendBuffLen );
		DSFree( recvBuff );
		
		if ( fKeyState == eKeyStateValidKey )
			break;
		
		// send the response
		if ( result == eDSNoErr ) {
			result = SendBuffer( sendBuff, sendBuffLen );
			DSFree( sendBuff );
		}
		
		// read the buffer len
		if ( result == eDSNoErr ) {
			result = SyncToMessageBody( true, &recvBuffLen );
		}
		
		// read the payload
		if ( result == eDSNoErr ) {
			recvBuff = (UInt8 *) calloc( recvBuffLen, sizeof(char) );
			
			UInt32 readBytes = DoTCPRecvFrom( recvBuff, recvBuffLen );
			if ( readBytes != recvBuffLen ) {
				result = eDSCorruptBuffer;
			}
		}
		
	} while ( result == eDSNoErr );
	
	DSFree( sendBuff );
	DSFree( recvBuff );
	
	return result;
} // ClientNegotiateKey


//------------------------------------------------------------------------------
//	* ServerNegotiateKey
//------------------------------------------------------------------------------

SInt32 DSTCPEndpoint::ServerNegotiateKey( void *dataBuff, UInt32 dataBuffLen )
{
	void	*sendBuff	= NULL;
	UInt32	sendBuffLen	= 0;
	SInt32	result		= ProcessData( true, dataBuff, dataBuffLen, sendBuff, sendBuffLen );
	
	if ( result == eDSNoErr ) {
		if ( sendBuffLen > 0 ) {
			result = SendBuffer( sendBuff, sendBuffLen );
		}
	}

	DSFree( sendBuff );

	return result;
} // ServerNegotiateKey


//------------------------------------------------------------------------------
//     * AllocToProxyStruct
//
//------------------------------------------------------------------------------

sComProxyData* DSTCPEndpoint::AllocToProxyStruct ( sComData *inDataMsg )
{
	sComProxyData      *outProxyDataMsg = nil;
	int					objIndex;

	
	if (inDataMsg != nil)
	{
		outProxyDataMsg = (sComProxyData *)calloc( 1, sizeof(sComProxyData) + inDataMsg->fDataSize );
		
		outProxyDataMsg->type = inDataMsg->type;
		outProxyDataMsg->fMsgID = inDataMsg->fMsgID;
		outProxyDataMsg->fDataSize = inDataMsg->fDataSize;
		outProxyDataMsg->fDataLength = inDataMsg->fDataLength;

		// this copies the sObject and the actual data
		bcopy( inDataMsg->obj, outProxyDataMsg->obj, kObjSize + inDataMsg->fDataSize );
		
		//need to adjust the offsets since they are relative to the start of the message
		for ( objIndex = 0; objIndex < 10; objIndex++ )
		{
			if ( outProxyDataMsg->obj[ objIndex ].offset != 0 )
			{
				// sComData is larger than proxy struct
				outProxyDataMsg->obj[ objIndex ].offset -= sizeof(sComData) - sizeof(sComProxyData);
			}
		}
	}

	return ( outProxyDataMsg );
}

//------------------------------------------------------------------------------
//     * AllocFromProxyStruct
//
//------------------------------------------------------------------------------

sComData* DSTCPEndpoint::AllocFromProxyStruct ( sComProxyData *inProxyDataMsg )
{
	sComData                   *outDataMsg          = nil;
	int							objIndex;

	if (inProxyDataMsg != nil)
	{
		outDataMsg = (sComData *)calloc( 1, sizeof(sComData) + inProxyDataMsg->fDataSize );
		
		outDataMsg->type = inProxyDataMsg->type;
		outDataMsg->fMsgID = inProxyDataMsg->fMsgID;
		outDataMsg->fPID = inProxyDataMsg->fPID;
		outDataMsg->fDataSize = inProxyDataMsg->fDataSize;
		outDataMsg->fDataLength = inProxyDataMsg->fDataLength;

		// this copies the sObject and the actual data
		bcopy( inProxyDataMsg->obj, outDataMsg->obj, kObjSize + inProxyDataMsg->fDataSize );
		
		//need to adjust the offsets since they are relative to the start of the message
		for ( objIndex = 0; objIndex < 10; objIndex++ )
		{
			if ( outDataMsg->obj[ objIndex ].offset != 0 )
			{
				// sComData is larger than proxy struct
				outDataMsg->obj[ objIndex ].offset += sizeof(sComData) - sizeof(sComProxyData);
			}
		}
		
		// set the effective UIDs to -2...
		outDataMsg->fUID = outDataMsg->fEffectiveUID = (uid_t) -2;
	}

	return ( outDataMsg );
}

SInt32 DSTCPEndpoint::ProcessData( bool bEncrypt, void *inBuffer, UInt32 inBufferLen, void *&outBuffer, UInt32 &outBufferLen )
{
	SInt32		result		= eDSCorruptBuffer;
	CSSM_DATA	plainText	= { 0, NULL };
	CSSM_DATA	cipherText	= { 0, NULL };
	
	switch ( fKeyState )
	{
		case eKeyStateSendPublicKey:
			// build the send buffer with the auth tag
			if ( cdsaDhGenerateKeyPair(fcspHandle, &fPublicKey, &fPrivateKey, DH_KEY_SIZE, &fParamBlock, NULL) == CSSM_OK )
			{
				outBufferLen = sizeof(FourCharCode) + fPublicKey.KeyData.Length;
				
				char *tempPtr = (char *) calloc( 1, outBufferLen );
				*((FourCharCode *) tempPtr) = htonl( DSTCPAuthTag );
				memcpy( tempPtr + sizeof(FourCharCode), fPublicKey.KeyData.Data, fPublicKey.KeyData.Length );
				outBuffer = tempPtr;
				result = eDSNoErr;
			}
			
			DbgLog( kLogDebug, "DSTCPEndpointProcessData - Send Public Key - generate key pair - %s", 
				   (result == eDSNoErr ? "succeeded" : "failed") );
			fKeyState = eKeyStateGenerateChallenge;
			break;
			
		case eKeyStateGenerateChallenge:
			if ( cdsaDhKeyExchange(fcspHandle, &fPrivateKey, inBuffer, inBufferLen, &fDerivedKey, DERIVE_KEY_SIZE, DERIVE_KEY_ALG) == CSSM_OK )
			{
				fChallengeValue = arc4random();
				
				uint32_t temp = htonl( fChallengeValue );
				
				plainText.Data = (uint8_t *) &temp;
				plainText.Length = sizeof(temp);
				
				if ( cdsaEncrypt(fcspHandle, &fDerivedKey, &plainText, &cipherText) == CSSM_OK )
				{
					outBuffer = cipherText.Data;
					outBufferLen = cipherText.Length;
					result = eDSNoErr;
				}
				
				fChallengeValue++; // we are expecting +1 as the response
			}
			
			DbgLog( kLogDebug, "DSTCPEndpointProcessData - Generate Challenge - challenge creation - %s", 
				   (result == eDSNoErr ? "succeeded" : "failed") );
			fKeyState = eKeyStateAcceptResponse;
			break;
			
		case eKeyStateAcceptResponse:
			cipherText.Data = (uint8_t *) inBuffer;
			cipherText.Length = inBufferLen;
			plainText.Data = NULL;
			plainText.Length = 0;
			
			if ( cdsaDecrypt(fcspHandle, &fDerivedKey, &cipherText, &plainText) == CSSM_OK )
			{
				if ( plainText.Data != NULL && plainText.Length == sizeof(uint32_t) && fChallengeValue == ntohl(*((uint32_t*) plainText.Data)) )
				{
					fKeyState = eKeyStateValidKey;
					result = eDSNoErr;
				}
				
				DSFree( plainText.Data );
			}
			
			DbgLog ( kLogDebug, "DSTCPEndpointProcessData - Accept Response - response was %s", 
					(result == eDSNoErr ? "correct" : "incorrect") );
			break;
			
		case eKeyStateAcceptClientKey:
			if ( inBufferLen > sizeof(FourCharCode) )
			{ 
				char *tempPtr = (char *) inBuffer;
				
				if ( DSTCPAuthTag == ntohl(*((FourCharCode *) tempPtr)) )
				{
					tempPtr += sizeof(FourCharCode);
					inBufferLen -= sizeof(FourCharCode);
					
					if ( cdsaDhGenerateKeyPair(fcspHandle, &fPublicKey, &fPrivateKey, DH_KEY_SIZE, &fParamBlock, NULL) == CSSM_OK )
					{
						if ( cdsaDhKeyExchange(fcspHandle, &fPrivateKey, tempPtr, inBufferLen, &fDerivedKey, DERIVE_KEY_SIZE, 
											   DERIVE_KEY_ALG) == CSSM_OK )
						{
							outBufferLen = fPublicKey.KeyData.Length;
							outBuffer = calloc( outBufferLen, sizeof(char) );
							bcopy( fPublicKey.KeyData.Data, outBuffer, outBufferLen );
							result = eDSNoErr;
						}
					}
				}
			}
			
			DbgLog( kLogDebug, "DSTCPEndpointProcessData - Accept Client Key - %s", (result == eDSNoErr ? "succeed" : "failed") );
			fKeyState = eKeyStateGenerateResponse;
			break;
			
		case eKeyStateGenerateResponse:
			if ( inBufferLen != 0 )
			{
				cipherText.Data		= (uint8_t *) inBuffer;
				cipherText.Length	= inBufferLen;
				if ( cdsaDecrypt(fcspHandle, &fDerivedKey, &cipherText, &plainText) == CSSM_OK )
				{
					if ( plainText.Data != NULL && plainText.Length == 4 )
					{
						//add one to test blob received
						uint32_t temp = ntohl( *((uint32_t *) plainText.Data) ) + 1;
						(*(uint32_t *) plainText.Data) = htonl( temp );
						
						cipherText.Data		= NULL;
						cipherText.Length	= 0;
						
						if ( cdsaEncrypt(fcspHandle, &fDerivedKey, &plainText, &cipherText) == CSSM_OK )
						{
							outBuffer = cipherText.Data;
							outBufferLen = cipherText.Length;
							result = eDSNoErr;
						}
						
						DSFree ( plainText.Data );
					}
				}
			}
			
			DbgLog( kLogDebug, "DSTCPEndpointProcessData - Generate Response - %s", (result == eDSNoErr ? "succeed" : "failed") );
			fKeyState = eKeyStateValidKey;
			break;
			
		case eKeyStateValidKey:
			outBufferLen = 0;
			if ( fDerivedKey.KeyData.Data != NULL )
			{
				if ( bEncrypt == true )
				{
					plainText.Data = (uint8_t *)inBuffer;
					plainText.Length = inBufferLen;
					
					if ( cdsaEncrypt(fcspHandle, &fDerivedKey, &plainText, &cipherText) == CSSM_OK )
					{
						outBuffer = cipherText.Data;
						outBufferLen = cipherText.Length;
						DbgLog( kLogDebug, "DSTCPEndpointProcessData - Encrypted data - length %d", outBufferLen );
						result = eDSNoErr;
					}
				}
				else
				{
					cipherText.Data = (uint8_t *) inBuffer;
					cipherText.Length = inBufferLen;
					
					if ( cdsaDecrypt(fcspHandle, &fDerivedKey, &cipherText, &plainText) == CSSM_OK )
					{
						outBuffer = plainText.Data;
						outBufferLen = plainText.Length;
						DbgLog( kLogDebug, "DSTCPEndpointProcessData - Decrypted data - length %d", outBufferLen );
						result = eDSNoErr;
					}
				}
			}
			break;
	}
	
	return result;
}
