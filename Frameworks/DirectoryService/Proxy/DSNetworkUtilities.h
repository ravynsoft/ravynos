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
 * @header DSNetworkUtilities
 */

#ifndef __DSNetworkUtilities_h__
#define __DSNetworkUtilities_h__ 1

#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/nameser.h>	// DNS 
#include <resolv.h>			// resolver global state

#include <DirectoryServiceCore/DSMutexSemaphore.h>
#include <DirectoryServiceCore/PrivateTypes.h>

#define	 MAXIPADDRSTRLEN	 32

// --------------------------------------------------------------------------------------------
//	* Typedefs and enums
// --------------------------------------------------------------------------------------------

// IP address string
typedef char		IPAddrStr[ MAXIPADDRSTRLEN ]; // define a type for "ddd.ddd.ddd.ddd\0"

enum { kMaxHostNameLength = 255 };

typedef	 UInt16		InetPort;
typedef	 UInt32		InetHost;
typedef	 char		InetDomainName[ kMaxHostNameLength + 1 ];

typedef	 struct MSInetHostInfo
{
	InetDomainName	name;
	InetHost		addrs[ 32 ];
} MSInetHostInfo;

typedef struct MultiHomeIPInfo {
	InetHost			IPAddress;				// our IP address
	IPAddrStr			IPAddressString;		// ASCII IP address string ddd.ddd.ddd.ddd\0
	InetDomainName		DNSName;				// our host name
	MultiHomeIPInfo    *pNext;
} MultiHomeIPInfo;

	
typedef struct IPAddressInfo {
	InetHost	addr;
	char		addrStr[MAXIPADDRSTRLEN];
} IPAddressInfo;


// --------------------------------------------------------------------------------------------
//	* Constants
// --------------------------------------------------------------------------------------------

const	short	kPrimaryIPAddr	= 0;
const	short	kMaxIPAddrs		= 32;

const	SInt32 kMAPRBLKnownSpammerResult = -10101;
const	SInt32 kNoValidIPAddress4ThisName = -10102;

class DSNetworkUtilities 
{
public:
	enum {	kDNSQuerySuccess = 0,
			kDNSQueryFailure,
			kDNSNotAvailable,
			kTCPNotAvailable
	};
	
	static SInt32		Initialize				( void );
	static void			DeInitialize			( void );

	// local host information
	static Boolean		IsTCPAvailable			( void ) { return sTCPAvailable; }

	static int			GetOurIPAddressCount	( void ) { return sIPAddrCount; }
	static InetHost		GetOurIPAddress 		( short inIndex = kPrimaryIPAddr );
	static const char *	GetOurIPAddressString	( short inIndex = kPrimaryIPAddr );

	static Boolean		DoesIPAddrMatch			( InetHost inIPAddr );

	// generic network routines
	static Boolean		IsValidAddressString	( const char *inAddrStr, InetHost *outInetHost );
	static InetHost		GetIPAddressByName		( const char *inName );	
	static int			IPAddrToString			( const InetHost inAddr, char *ioNameBuffer, const int inBufferSize);
	static int			StringToIPAddr			( const char *inAddrStr, InetHost *ioIPAddr );

	static SInt32		ResolveToIPAddress 		( const InetDomainName inDomainName, InetHost* outInetHost );

protected:
	static int				InitializeTCP		( void );
	static void		 		Signal				( void );
	static void             Wait				( void );


	// Data members

	static Boolean			sNetworkInitialized;
	static Boolean			sTCPAvailable;

	static short			sIPAddrCount;		// count of IP addresses for this server

	static MultiHomeIPInfo	*sIPInfo;

	static DSMutexSemaphore	*sNetSemaphore;
			
}; // class DSNetworkUtilities


#endif // __DSNetworkUtilities_h__
