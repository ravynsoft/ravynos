/*
 * Copyright (c) 1996-98, 2000-2002 Apple Computer, Inc. All rights reserved.
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
 * @header CString
 * Implementation of a Reasonable string class.
 */

// sys
#include <string.h>
#include <stdio.h>		// for sprintf()
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>

#include "CString.h"
#include "PrivateTypes.h"


// ----------------------------------------------------------------------------
//	* CString Instance Methods
// ----------------------------------------------------------------------------
#pragma mark **** CString Public Instance Methods ****

// ctor & dtor
// Unless an exception is thrown, the ctors must always call Grow() so mData
// is always valid.
CString::CString ( int sz )
{
	mSize	= 0;
	mLength	= 0;
	mData	= NULL;

	Grow( sz );
}


// ---------------------------------------------------------------------------
//	* CString ()
//
// ---------------------------------------------------------------------------

CString::CString ( const char *str )
{
	mSize	= 0;
	mLength	= 0;
	mData	= NULL;

	// let's not throw if NULL string...

	// Set will not call Grow() if passed an empty string.
	if ( ( str != NULL ) && *str )
	{
		Set( str );
	}
	else
	{
		Grow();
	}
}


// ---------------------------------------------------------------------------
//	* CString()
//
// ---------------------------------------------------------------------------

CString::CString ( const char *str, int len )
{
	mSize	= 0;
	mLength	= 0;
	mData	= NULL;

	// Set will not call Grow() if passed an empty string.
	if ( ( str != NULL ) && *str && len )
	{
		Set( str, len );
	}
	else
	{
		Grow();
	}
} // CString


// ---------------------------------------------------------------------------
//	* CString()
//
// ---------------------------------------------------------------------------

CString::CString ( const CString& cs )
{
	mSize	= 0;
	mLength	= 0;
	mData	= NULL;

	// CString should always have a valid size.
	if ( cs.mSize <= 0 ) throw( (SInt32)eParameterError );

	Grow( cs.mSize );

	if ( !cs.mLength )
	{
		return;
	}

	mLength = cs.mLength;
	::strcpy( mData, cs.mData );

} // CString


// ---------------------------------------------------------------------------
//	* CString()
//
// ---------------------------------------------------------------------------

CString::CString ( const char *pattern, va_list args )
{
	mSize	= 0;
	mLength	= 0;
	mData	= NULL;

	// let's not throw if NULL string...
	if ( pattern == nil ) throw((SInt32)eParameterError);

	// Allocate some space then vsprintf into the buffer.
	Grow();
	Vsprintf( pattern, args );

} // CString


// ---------------------------------------------------------------------------
//	* ~CString()
//
// ---------------------------------------------------------------------------

CString::~CString()
{
	if ( mData == NULL )
		return;

	delete [] mData;
	mData = NULL;
	mLength = 0;
	mSize = 0;
} // ~CString


// ---------------------------------------------------------------------------
//	* GetLength()
//
// ---------------------------------------------------------------------------

int CString::GetLength ( void ) const
{ 
	if ( mData != NULL )
	{
		return( (int) ::strlen( mData ) );
	}
	else
	{
		return( 0 );
	}
} // GetLength


// ---------------------------------------------------------------------------
//	* GetPascal()
//
//		 - pascal string conversion
//
// ---------------------------------------------------------------------------

void CString::GetPascal ( unsigned char *pstr ) const
{
	if ( pstr == nil ) throw( (SInt32)eParameterError );
	if ( mLength > 255 ) throw( (SInt32)eParameterError );
	*pstr++ = ( unsigned char ) mLength;
	::memcpy( pstr, mData, mLength );
} // GetPascal


// ---------------------------------------------------------------------------
//	* Sprintf()
//
//		- string replacement routines ( also called by ctors )
//
// ---------------------------------------------------------------------------

void CString::Set ( const char *str )
{
	if ( str == nil ) throw((SInt32)eParameterError);

	// Clear mLength to avoid a copy during the grow.
	mLength = 0;

	// Handle the corner cases.
	if ( !*str )
	{
		*mData = '\0';
		return;
	}

	int	len = (int) ::strlen ( str );

	Grow( len + 1 );
	strcpy( mData, str );
	mLength = len;
} // Set


// ---------------------------------------------------------------------------
//	* Set()
//
// ---------------------------------------------------------------------------

void CString::Set ( const char *str, int len )
{
	if ( str == nil ) throw( (SInt32)eParameterError );
	if ( len < 0 ) throw( (SInt32)eParameterError );

	mLength = 0;

	// Handle the corner cases.
	if ( !len || !*str )
	{
		*mData = '\0';
		return;
	}

	Grow( len + 1 );

	int	strLen = (int) ::strlen ( str );
	mLength = ( ( strLen < len ) ? strLen : len );
	::memcpy( mData, str, mLength );
	mData[mLength] = '\0';
} // Set


// ---------------------------------------------------------------------------
//	* Set()
//
// ---------------------------------------------------------------------------

void CString::Set ( const unsigned char *pstr )
{
	if ( pstr == nil ) throw((SInt32)eParameterError);

	int	len = ( int ) *pstr++;

	mLength = 0;

	// Handle the corner case.
	if ( !len )
	{
		*mData = '\0';
		return;
	}

	Grow( len + 1 );
	::memcpy( mData, pstr, len );
	mLength = len;
	mData[len] = '\0';

} // Set


// ---------------------------------------------------------------------------
//	* Set()
//
// ---------------------------------------------------------------------------

void CString::Set ( const CString &cs )
{
	int	len = cs.mLength;

	mLength = 0;

	// Handle the corner case.
	if ( !len )
	{
		*mData = '\0';
		return;
	}

	Grow( len + 1 );
	// strcpy will add the terminator.
	::strcpy( mData, cs.mData );
	mLength = len;

} // Set


// ----------------------------------------------------------------------------
//	* Append()
// ----------------------------------------------------------------------------

void CString::Append ( char inChar )
{
	Grow( mLength + 2 );
	// Append the char and the terminator.
	mData[mLength++] = inChar;
	mData[mLength] = '\0';
} // Append


// ---------------------------------------------------------------------------
//	* Append()
//
// ---------------------------------------------------------------------------

void CString::Append ( const char *str )
{
	if ( str == nil ) throw((SInt32)eParameterError);
	// Handle the corner case.
	if ( !*str )
		return;

	int	len = (int) ::strlen ( str );
	Grow( mLength + len + 1 );
	// strcpy will add the terminator.
	::strcpy( &mData[mLength], str );
	mLength += len;
} // Append


// ---------------------------------------------------------------------------
//	* Append()
//
// ---------------------------------------------------------------------------

void CString::Append ( const char *str, int arglen )
{
	if ( str == nil ) throw( (SInt32)eParameterError );
	if ( arglen < 0 ) throw( (SInt32)eParameterError );
	// Handle the corner cases.
	if ( !arglen || !*str )
		return;

	int len = (int) ::strlen ( str );
	if ( arglen < len )
	{
		len = arglen;
	}

	Grow( mLength + len + 1 );

	::memcpy( &mData[mLength], str, len );
	mLength += len;
	mData[mLength] = '\0';

} // Append


// ---------------------------------------------------------------------------
//	* Append()
//
// ---------------------------------------------------------------------------

void CString::Append ( const unsigned char *pstr )
{
	if ( pstr == nil ) throw((SInt32)eParameterError);

	int	len = ( int ) *pstr++;

	// Handle the corner case.
	if ( !len )
	{
		return;
	}

	Grow( mLength + len + 1 );
	::memcpy( &mData[mLength], pstr, len );
	mLength += len;
	mData[mLength] = '\0';

} // Append


// ---------------------------------------------------------------------------
//	* Append()
//
// ---------------------------------------------------------------------------

void CString::Append ( const CString &cs )
{
	int	len = cs.mLength;

	// Handle the corner case.
	if ( !len )
		return;

	Grow( mLength + len + 1 );
	// strcpy will add the terminator.
	::strcpy( &mData[mLength], cs.mData );
	mLength += len;
} // Append



// ----------------------------------------------------------------------------
//	* Prepend()
//
//		- Prepending is an expensive operation because the string must be copied
//			completely. To avoid multiple moves, this function -- and this
//			function alone -- duplicates the memory management in Grow().
// ----------------------------------------------------------------------------

void CString::Prepend ( const char *str )
{
	if ( str == nil ) throw((SInt32)eParameterError);

	// Handle the corner case.
	if ( !*str )
		return;
	// Optimization when prepending to an empty string.
	if ( !mLength )
	{
		Append ( str );
		return;
	}

	size_t	len = ::strlen ( str );
	int	newlen = (int) (mLength + len + 1);

	// Handle the easy case first: allocate a new block and copy into it.
	if ( newlen > mSize )
	{
		int	pow2 = 16;
		while ( pow2 < newlen )
			pow2 <<= 1;
		char	*newData = new char [pow2];
		if ( newData == nil ) throw((SInt32)eMemoryAllocError);
		::strcpy( newData, str );
		::strcpy( &newData[len], mData );
		delete []mData;
		mSize = pow2;
		mLength += (UInt32) len;
		mData = newData;
	}
	else
	{
		char	*cpNew = &mData[--newlen];
		char	*cpOld = &mData[( newlen = mLength )];
		// Copy the string backwards, starting with the terminator.
		for ( newlen++; newlen--; )
			*cpNew-- = *cpOld--;
		// Copy the prepended string; using memcpy() to avoid the terminator.
		::memcpy( mData, str, len );
		mLength += (UInt32) len;
	}
} // Prepend


// ----------------------------------------------------------------------------
//	* Clear()
//
//		 - nulls the string and sets the size to inNewSize
//
// ----------------------------------------------------------------------------

void CString::Clear ( int inNewSize )
{
	*mData = '\0';
	mLength = 0;

	if ( inNewSize != 0 )
	{
		Grow( inNewSize );
	}
}


// ---------------------------------------------------------------------------
//	* Sprintf()
//
// ---------------------------------------------------------------------------

void CString::Sprintf ( const char *inPattern, ... )
{
	va_list		args;
	va_start	( args, inPattern );

	this->Vsprintf( inPattern, args );
} // Sprintf


// ---------------------------------------------------------------------------
//	* Vsprintf()
//
// ---------------------------------------------------------------------------

void CString::Vsprintf ( const char *pattern, va_list args )
{
	// Use a temp buffer big enough to hold one 64-bit value more than
	// the default size.
	char			caTemp [kCStringDefSize + 32];
	char  *cpTemp = caTemp;
	unsigned long	ulArg;
	long			lArg;
	u_int32_t		uiArg;
	int32_t			iArg;
	double			dArg;
	int				nArg;
	char		   *szpArg;
	unsigned char  *spArg;
	FourCharCode	fccArg;
	CString		   *cspArg;
	in_addr_t		ipv4Address;

	this->Clear();
	
	for ( const char *p = pattern; *p; p++ )
	{
		// The threshold minimizes the calls to Grow() and is arbitrary.
		if ( ( cpTemp - caTemp ) > kCStringDefSize )
		{
			*cpTemp = '\0';
			Append ( caTemp );
			cpTemp = caTemp;
		}
		if ( *p != '%' )
		{
			*cpTemp++ = *p;
		}
		else
		{
			switch ( *++p )
			{
				// non-standard ( A = IP Address )!
				case 'A':
					ipv4Address = va_arg ( args, in_addr_t );
					cpTemp += ::sprintf ( cpTemp, "%d.%d.%d.%d",
											((ipv4Address >> 24) & 0xFF),
											((ipv4Address >> 16) & 0xFF),
											((ipv4Address >>  8) & 0xFF),
											(ipv4Address & 0xFF) );
					break;

				// non-standard! (D = Date in localtime ie. no longer UTC/GMT, no arg)
				case 'D':
					{
					time_t		tNow = ::time (NULL) ;
					struct tm	*tmTime = ::localtime(&tNow) ;
					cpTemp += ::sprintf (cpTemp, "%04d-%02d-%02d",
											tmTime->tm_year + 1900,
											tmTime->tm_mon + 1,
											tmTime->tm_mday) ;
					}
					break;

				// non-standard ( F = FourCharCode )!
				case 'F':
					fccArg = va_arg ( args, FourCharCode );
					*cpTemp++ = '\'';
					*cpTemp++ = ((fccArg >> 24) & 0xFF);
					*cpTemp++ = ((fccArg >> 16) & 0xFF);
					*cpTemp++ = ((fccArg >>  8) & 0xFF);
					*cpTemp++ = (fccArg        & 0xFF);
					*cpTemp++ = '\'';
					break;

				// non-standard ( P = Pascal string )!
				case 'P':
				case 'p':	// lower-case 'p' is deprecated usage
					spArg = va_arg ( args, unsigned char * );
					if ( cpTemp != caTemp )
					{
						*cpTemp = '\0';
						Append ( caTemp );
						cpTemp = caTemp;
					}
					Append ( spArg );
					break;

				// non-standard ( S = CString )!
				case 'S':
					cspArg = va_arg ( args, CString * );
					if ( cpTemp != caTemp )
					{
						*cpTemp = '\0';
						Append ( caTemp );
						cpTemp = caTemp;
					}
					if ( cspArg )
						Append ( *cspArg );
					break;

				// non-standard! (T = localtime with offset, no arg)
				case 'T':
					{
						time_t		tNow = ::time (NULL) ;
						struct tm	*tmTime = ::localtime(&tNow) ;
						cpTemp += ::sprintf (cpTemp, "%02d:%02d:%02d %s",
												tmTime->tm_hour,
												tmTime->tm_min,
												tmTime->tm_sec,
												tmTime->tm_zone) ;
					}
					break;

				// non-standard! (G = XCode Debug Output, no arg)
				case 'G':
					{
						cpTemp += ::sprintf (cpTemp, "DSDEBUG") ;
					}
					break;

				// non-standard ( long expected )!
				case 'X':
					ulArg = va_arg ( args, unsigned long );
#ifdef __LP64__
					cpTemp += ::sprintf ( cpTemp, "0x%016lX", ulArg );
#else
					cpTemp += ::sprintf ( cpTemp, "0x%08lX", ulArg );
#endif
					break;
					
				case 'u': // this is a 32-bit unsigned int
					uiArg = va_arg ( args, u_int32_t );
					cpTemp += ::sprintf ( cpTemp, "%u", uiArg );
					break;

				case 'U': // this is a unsigned long (64 or 32 depending on LP64)
					ulArg = va_arg ( args, unsigned long );
					cpTemp += ::sprintf ( cpTemp, "%lu", ulArg );
					break;
				
				case 'l': // this is a 32-bit signed int
					iArg = va_arg ( args, int32_t );
					cpTemp += ::sprintf ( cpTemp, "%d", iArg );
					break;

				case 'L': // this is a long (64 or 32 depending on LP64)
					lArg = va_arg ( args, long );
					cpTemp += ::sprintf ( cpTemp, "%ld", lArg );
					break;
					
				case 'c':
					*cpTemp++ = va_arg ( args, int );
					break;

				case 'd':
					nArg = va_arg ( args, int );
					cpTemp += ::sprintf ( cpTemp, "%d", nArg );
					break;

				case 's':
					szpArg = va_arg ( args, char * );
					if ( cpTemp != caTemp )
					{
						*cpTemp = '\0';
						Append ( caTemp );
						cpTemp = caTemp;
					}
					if ( szpArg )
						Append ( szpArg );
					break;
					
				case 'f':
				case 'g':
					dArg = va_arg ( args, double );
					cpTemp += ::sprintf ( cpTemp, "%f", dArg );
					break;

				default:
					cpTemp += ::sprintf ( cpTemp, "  *** bad control string ***  " );
			}
		}
	}

	if ( cpTemp != caTemp )
	{
		*cpTemp = '\0';
		Append ( caTemp );
	}
	va_end ( args );
} // Vsprintf


#pragma mark **** CString Protected Instance Methods ****

// ----------------------------------------------------------------------------
//	* Grow()
//
//		- All memory management ( even in the c'tors ) are handled in this method.
//			A newSize value of 0 implies the use of the default buffer size!
//			To leave the buffer alone, call with an argument value of 1.
//
//			IMPORTANT NOTE: Any changes to Grow() should be reflected in Prepend()
//				because it performs memory management for efficiency.
//
// ----------------------------------------------------------------------------

void CString::Grow( int newSize )
{
	if ( newSize < 0 ) throw( (SInt32)eParameterError );

	// Allocate the default length if requested.
	if ( !newSize )
	{
		newSize = kCStringDefSize;
	}

	// Don't bother reallocating if there is already enough room.
	if ( newSize <= mSize )
	{
		return;
	}

	// Round the requested size to the nearest power of two ( > 16 ).
	// The comparison is an optimization for the most common case.
	if ( newSize != kCStringDefSize )
	{
		int	pow2 = 16;
		while ( pow2 < newSize )
			pow2 <<= 1;
		newSize = pow2;
	}

	char	*newData = new char [newSize];
	if ( newData == nil ) throw((SInt32)eMemoryAllocError);

	if ( mLength )
	{
		::strcpy( newData, mData );
	}
	else
	{
		*newData = '\0';
	}

	if ( mData != NULL )
	{
		delete [] mData;
	}

	mSize = newSize;
	mData = newData;

} // Grow
