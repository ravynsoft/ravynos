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

#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/errno.h>

#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>		// interface struture ifreq, ifconf
#include <net/if_dl.h>	// datalink structs
#include <net/if_types.h>
#include <unistd.h>

#include <arpa/inet.h>	// for inet_*
#include <arpa/nameser.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <ifaddrs.h>

#include "DSNetworkUtilities.h"
#ifdef DSSERVERTCP
#include "CLog.h"
#endif

#define IFR_NEXT(ifr)   \
    ((struct ifreq *) ((char *) (ifr) + sizeof(*(ifr)) + \
      MAX(0, (int) (ifr)->ifr_addr.sa_len - (int) sizeof((ifr)->ifr_addr))))

// ------------------------------------------------------------------------
// Static class variables
// ------------------------------------------------------------------------
Boolean				DSNetworkUtilities::sNetworkInitialized	= false;
Boolean				DSNetworkUtilities::sTCPAvailable		= false;

short				DSNetworkUtilities::sIPAddrCount		= 0;	// count of IP addresses for this server

MultiHomeIPInfo*	DSNetworkUtilities::sIPInfo				= nil;

DSMutexSemaphore   *DSNetworkUtilities::sNetSemaphore		= NULL;

// ------------------------------------------------------------------------
//	* Initialize ()
// ------------------------------------------------------------------------

SInt32 DSNetworkUtilities::Initialize ( void )
{
	register int rc = 0;

	if (sNetworkInitialized == true)
	{
		return eDSNoErr;
	}

	sNetSemaphore = new DSMutexSemaphore("DSNetworkUtilities::sNetSemaphore");
	sIPInfo = nil;

	try
	{
#ifdef DSSERVERTCP
		SrvrLog( kLogApplication, "Initializing TCP ..." );
#endif
		rc = InitializeTCP();
		if ( rc != 0 )
		{
#ifdef DSSERVERTCP
			ErrLog( kLogApplication, "*** Warning*** TCP is not available.  Error: %d", rc );
			DbgLog( kLogTCPEndpoint, "DSNetworkUtilities::Initialize(): TCP not available." );
#else
			LOG1( kStdErr, "*** Warning*** DSNetworkUtilities::Initialize(): TCP is not available.  Error: %d", rc );
#endif
			throw( (SInt32)rc );
		}

	}

	catch( SInt32 err )
	{
		sNetworkInitialized = false;
#ifdef DSSERVERTCP
		DbgLog( kLogTCPEndpoint, "DSNetworkUtilities::Initialize failed.  Error: %d", err );
#else
		LOG1( kStdErr, "DSNetworkUtilities::Initialize failed.  Error: %d", err );
#endif
		return err;
	}

	sNetworkInitialized = true;
#ifdef DSSERVERTCP
	DbgLog( kLogTCPEndpoint, "DSNetworkUtilities::Initialized." );
#endif

	return( eDSNoErr );

} // Initialize


// ------------------------------------------------------------------------
//	* ResolveToIPAddress ()
// ------------------------------------------------------------------------

SInt32 DSNetworkUtilities::ResolveToIPAddress ( const InetDomainName inDomainName, InetHost* outInetHost )
{
	register struct hostent *hp = NULL;

	DSNetworkUtilities::Wait();

	try
	{
		if ( inDomainName[0] != '\0' && outInetHost != NULL)
		{
			hp = ::gethostbyname( inDomainName );
			if ( hp == NULL )
			{
				throw( (SInt32)h_errno );
			}
			*outInetHost = ntohl( ((struct in_addr *)(hp->h_addr_list[0]))->s_addr );

			DSNetworkUtilities::Signal();
			return( eDSNoErr );
		}
	} // try

	catch( SInt32 err )
	{
#ifdef DSSERVERTCP
		ErrLog( kLogTCPEndpoint, "Unable to resolve the IP address for %s.", inDomainName );
#else
		LOG1( kStdErr, "Unable to resolve the IP address for %s.", inDomainName );
#endif
		DSNetworkUtilities::Signal();
		return( err );
	}

    return( eDSNoErr );

} // ResolveToIPAddress


// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------

InetHost DSNetworkUtilities::GetOurIPAddress ( short inIndex )
{
	MultiHomeIPInfo    *aIPInfo = sIPInfo;
	short				aIndex  = 0;
	
	if ( sNetworkInitialized == false )
	{
		if ( Initialize() != eDSNoErr )
		{
			return 0;
		}
	}

	if (inIndex  < sIPAddrCount )
	{
		while ( aIPInfo != nil)
		{
			if ( aIndex == inIndex)
			{
				return( aIPInfo->IPAddress );
			}
			aIndex++;
			aIPInfo = aIPInfo->pNext;
		}
	}

	return 0;

} // GetOurIPAddress


// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
const char *
DSNetworkUtilities::GetOurIPAddressString (short inIndex)
{
	MultiHomeIPInfo    *aIPInfo = sIPInfo;
	short				aIndex  = 0;

	if (inIndex  < sIPAddrCount )
	{
		while ( aIPInfo != nil)
		{
			if ( aIndex == inIndex)
			{
				return( aIPInfo->IPAddressString );
			}
			aIndex++;
			aIPInfo = aIPInfo->pNext;
		}
	}

	return NULL;
}


// ------------------------------------------------------------------------
//	* DoesIPAddrMatch ()
//
//		- Does this IP address match one of my addresses
// ------------------------------------------------------------------------

Boolean DSNetworkUtilities::DoesIPAddrMatch ( InetHost inIPAddr )
{
	MultiHomeIPInfo    *aIPInfo = sIPInfo;

	if ((inIPAddr != 0x00000000) && (inIPAddr != 0xFFFFFFFF) )
	{
		while ( aIPInfo != nil)
		{
			if ( inIPAddr == aIPInfo->IPAddress)
			{
				return( true );
			}
			aIPInfo = aIPInfo->pNext;
		}
	}
	return false;
}


//--------------------------------------------------------------------------------------------------
//	IsValidAddressString
//
//		- Check if the string passed in is a valid IP address string. Returns true and converts
//		 it to InetHost(IP address) or retunrs false if not a valid address string.
//
//--------------------------------------------------------------------------------------------
Boolean
DSNetworkUtilities::IsValidAddressString (const char *inString, InetHost *outInetHost)
{
	register const char	*cp;
	struct in_addr	hostaddr;

	if (inString == NULL)
		return false;

	/* Make sure it has only digits and dots. */
	for (cp = inString; *cp; ++cp) {
		if ((isdigit(*cp) !=0) && *cp != '.') {
			return false;
		}
	}

	/* If it has a trailing dot, don't treat it as an address. */
	if (*--cp != '.') {
		if (::inet_aton(inString, &hostaddr) == 1) {
			*outInetHost = ntohl(hostaddr.s_addr);
			return true;
		}
	}

	return false;
}


//--------------------------------------------------------------------------------------------------
//	GetIPAddressByName
//
//		- lookup the IP address of a given hostname
//
//--------------------------------------------------------------------------------------------
InetHost
DSNetworkUtilities::GetIPAddressByName(const char *inName)
{
	register struct hostent	*hp = NULL;
	register InetHost	IPAddr = 0;

	DSNetworkUtilities::Wait();

	try {

		hp = ::gethostbyname(inName);
		if (hp == NULL) {
			throw((SInt32)h_errno);
		}

		IPAddr = ntohl(((struct in_addr *)hp->h_addr)->s_addr);
		DSNetworkUtilities::Signal();
		return IPAddr;
	}

	catch( SInt32 err )
	{
#ifdef DSSERVERTCP
		ErrLog( kLogTCPEndpoint, "GetIPAddressByName for %s failed: %d.", inName, err );
#else
		LOG2( kStdErr, "GetIPAddressByName for %s failed: %d.", inName, err );
#endif
		DSNetworkUtilities::Signal();
		return IPAddr;
	}

}

//--------------------------------------------------------------------------
//	IPAddrToString
//
//	Takes a IP address convert it to the string format with dots
//	address passed in should be in host byte order
//---------------------------------------------------------------------------
int
DSNetworkUtilities::IPAddrToString(const InetHost inAddr, char *ioNameBuffer, const int inBufferSize)
{
	char	*result;
	struct in_addr	InetAddr;

	DSNetworkUtilities::Wait();

	InetAddr.s_addr = htonl(inAddr);
	result = ::inet_ntoa(InetAddr);
	if ( result != NULL )
		::strncpy(ioNameBuffer, result, inBufferSize);
	else {
#ifdef DSSERVERTCP
		ErrLog( kLogTCPEndpoint, "IPAddrToString for %u failed.", inAddr );
#else
		LOG1( kStdErr, "IPAddrToString for %u failed.", inAddr );
#endif
		ioNameBuffer[0] = '\0';
		DSNetworkUtilities::Signal();
		return -1;
	}

	DSNetworkUtilities::Signal();
	return 0;
}


//-------------------------------------------------------------------------------
//	StringToIPAddr
//
//	Takes a IP address string (with dots) and convert it to a real IP address
//	Address returned is in host byte order
//--------------------------------------------------------------------------------
int
DSNetworkUtilities::StringToIPAddr(const char *inAddrStr, InetHost *ioIPAddr)
{
	struct in_addr	InetAddr;
	int		rc;

	if ( inAddrStr  && ioIPAddr ) {
		DSNetworkUtilities::Wait();

		rc = ::inet_aton(inAddrStr, &InetAddr);
		if ( rc == 1 ) {
			*ioIPAddr = ntohl(InetAddr.s_addr);
			DSNetworkUtilities::Signal();
			return 0;
		}

#ifdef DSSERVERTCP
		ErrLog( kLogTCPEndpoint, "StringToIPAddr() failed for %s", inAddrStr );
#else
		LOG1( kStdErr, "StringToIPAddr() failed for %s", inAddrStr );
#endif
		DSNetworkUtilities::Signal();
	}
	return -1;
}


// ------------------------------------------------------------------------
// proctected member functions
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
//	* InitializeTCP ()
//
// ------------------------------------------------------------------------

int DSNetworkUtilities::InitializeTCP ( void )
{
	// interface structures
	register struct sockaddr_in *sain;
	register int	sock = 0;
	register int	ipcount = 0;
	int rc = 0;
	int err = 0;
	MultiHomeIPInfo    *aIPInfo = nil;


	sTCPAvailable = false;

	DSNetworkUtilities::Wait();

	try {
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if ( sock == -1 )
		{
			err = errno;
#ifdef DSSERVERTCP
			ErrLog( kLogTCPEndpoint, "SOCKET: %d.", err );
#else
			LOG1( kStdErr, "SOCKET: %d.", err );
#endif
			throw((SInt32)err);
		}

		struct ifaddrs *ifa_list = NULL, *ifa = NULL;
		
		rc = getifaddrs( &ifa_list );
		if ( rc == 0 )
		{
			for ( ifa = ifa_list; ifa; ifa = ifa->ifa_next )
			{
				if ( *ifa->ifa_name == 'e' && ifa->ifa_addr->sa_family == AF_INET )
				{
					// ethernet interface
					sain = (struct sockaddr_in *)ifa->ifa_addr;
					if (sIPInfo != nil)
					{
						aIPInfo = sIPInfo;
						while(aIPInfo->pNext != nil)
						{
							aIPInfo = aIPInfo->pNext;
						}
						aIPInfo->pNext = (MultiHomeIPInfo*) calloc(1, sizeof(MultiHomeIPInfo));
						aIPInfo = aIPInfo->pNext;
					}
					else
					{
						sIPInfo = (MultiHomeIPInfo*) calloc(1, sizeof(MultiHomeIPInfo));
						aIPInfo = sIPInfo;
						
					}
					
					aIPInfo->IPAddress = ntohl(sain->sin_addr.s_addr);
					IPAddrToString(aIPInfo->IPAddress, aIPInfo->IPAddressString, MAXIPADDRSTRLEN);
					ipcount ++;
				}
			}
			
			freeifaddrs(ifa_list);
		}
		else
		{
#ifdef DSSERVERTCP
			ErrLog( kLogTCPEndpoint, "getifaddrs: %d.", err );
#else
			LOG1( kStdErr, "getifaddrs: %d.", err );
#endif
			throw((SInt32)err);
		}
		
		close(sock);
		sIPAddrCount = ipcount;
		sTCPAvailable = true;
		DSNetworkUtilities::Signal();

	} // try

	catch( SInt32 someError )
	{
		DSNetworkUtilities::Signal();
#ifdef DSSERVERTCP
		ErrLog( kLogTCPEndpoint, "DSNetworkUtilities::InitializeTCP failed." );
#else
		LOG( kStdErr, "DSNetworkUtilities::InitializeTCP failed." );
#endif
		sTCPAvailable = false;
		return someError;
	}

	return eDSNoErr;

} // InitializeTCP


void DSNetworkUtilities::Signal ( void )
{
	if ( sNetSemaphore != nil )
	{
		sNetSemaphore->Signal();
	}
	else
	{
#ifdef DSSERVERTCP
		DbgLog( kLogApplication,"DSNetworkUtilities::Signal -- sNetSemaphore is NULL" );
#else
		LOG( kStdErr,"DSNetworkUtilities::Signal -- sNetSemaphore is NULL" );
#endif
	}
}

void DSNetworkUtilities::Wait ()
{
	if ( sNetSemaphore != nil )
	{
		sNetSemaphore->Wait();
	}
	else
	{
#ifdef DSSERVERTCP
		DbgLog( kLogApplication,"DSNetworkUtilities::Wait -- sNetSemaphore is NULL" );
#else
		LOG( kStdErr,"DSNetworkUtilities::Wait -- sNetSemaphore is NULL" );
#endif
	}
}

