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
 * @header CFile
 * Versions of fstreams that use async file system calls.
 * To provide high-performance file I/O calls.
 */

#include "CFile.h"
#include "PrivateTypes.h"		// for eDSNoErr
#include <stdio.h>				// for statfs() and structs
#include <stdlib.h>				// for malloc()
#include <string.h>				// for strlen()
#include <sys/mount.h>			// for statfs() and structs
#include <fcntl.h>				// for open() flags
#include <errno.h>				// for errno

enum {
	kiIOAbort					= -27
};

//--------------------------------------------------------------------------------------------------
//	* CFile ()
//
//--------------------------------------------------------------------------------------------------

CFile::CFile ( void ) throw()
:		fLock("CFile::fLock"),
		fFilePath( nil ),
		fFileRef( kBadFileRef ),
		fRollLog( false ),
		fReadPos( 0 ),
		fWritePos( 0 ),
		fReadPosOK( false ),
		fWritePosOK( false ),
		fWroteData( false )
{
} // CFile


//--------------------------------------------------------------------------------------------------
//	* CFile ()
//
//--------------------------------------------------------------------------------------------------

CFile::CFile (	const char *inFilePath, const Boolean inCreate, const Boolean inRoll ) throw( SInt16 )
	:	fLock("CFile::fLock"),
		fFilePath( nil ),
		fFileRef( kBadFileRef ),
		fRollLog( inRoll ),
		fReadPos( 0 ),
		fWritePos( 0 ),
		fReadPosOK( false ),
		fWritePosOK( false ),
		fWroteData( false )
{
	this->open( inFilePath, inCreate );
} // CFile


//--------------------------------------------------------------------------------------------------
//	* ~CFile
//
//--------------------------------------------------------------------------------------------------

CFile::~CFile ( void )
{
	this->close();
	if ( fFilePath != nil )
	{
		free( fFilePath );
		fFilePath = nil;
	}
} // ~CFile


//--------------------------------------------------------------------------------------------------
//	* CFile
//
//--------------------------------------------------------------------------------------------------

void CFile::open ( const char *inFilePath, const Boolean inCreate )	throw ( SInt16 )
{
	register FILE	   *aFileRef		= kBadFileRef;
	char			   *pTmpFilePath	= nil;
	bool				bNewPath		= true;
	
	if ( inCreate == true )
	{
		if ( ::stat( inFilePath, &fStatStruct ) != -1 )
		{
			// file already exists, open it for read/write
			if ( kBadFileRef != (aFileRef = ::fopen( inFilePath, "r+" )) )
			{
				::rewind( aFileRef );
			}
		}
		else
		{
			// file does not exist, create it and open for read/write
			if ( kBadFileRef != (aFileRef = ::fopen( inFilePath, "w+" )) )
			{
				::stat( inFilePath, &fStatStruct );
				::rewind( aFileRef );
			}
		}
	}
	else
	{
		if ( ::stat( inFilePath, &fStatStruct ) != -1 )
			aFileRef = ::fopen( inFilePath, "r+" );
	}
	
	if ( fFilePath != nil )
	{
		if ( ::strcmp( fFilePath, inFilePath ) == 0 )
		{
			bNewPath = false;
		}
	}

	if ( bNewPath == true )
	{
		pTmpFilePath = (char *)::malloc( ::strlen( inFilePath ) + 1 );
		if ( pTmpFilePath != nil )
		{
			::strcpy( pTmpFilePath, inFilePath );
			if ( fFilePath != nil )
			{
				free( fFilePath );
				fFilePath = nil;
			}
			fFilePath = pTmpFilePath;
		}
		else
		{
			throw( (SInt16)eMemoryAllocError );
		}
	}

	fOpenTime	= ::time( nil );
	fLastChecked= ::time( nil );

	if ( kBadFileRef == aFileRef )
	{
		return;
	}
	
	fFileRef	= aFileRef;
	fReadPos	= 0;
	fWritePos	= 0;
	fReadPosOK	= true;
	fWritePosOK = true;

} // open


//--------------------------------------------------------------------------------------------------
//	* seteof ()
//
//--------------------------------------------------------------------------------------------------

CFile& CFile::seteof ( sInt64 lEOF ) throw ( SInt16 )
{
	SInt16		nError;

	if ( fFileRef == kBadFileRef )
	{
		throw( (SInt16) ds_fnOpnErr );
	}

	fLock.WaitLock();

	nError = ::ftruncate( fileno( fFileRef ), lEOF );
	fReadPosOK	= false;
	fWritePosOK	= false;

	fLock.SignalLock();
	if ( nError )
	{
		// ********* Put a proper error code here!
		throw( (SInt16) ds_fnOpnErr );
	}

	return( *this );

} // seteof


//--------------------------------------------------------------------------------------------------
//	* close
//
//--------------------------------------------------------------------------------------------------

void CFile::close ( void ) throw ( SInt16 )
{
	if ( fFileRef == kBadFileRef )
	{
		return;
	}

	::fflush( fFileRef );
	
	// only sync if we've written data
	if( fWroteData )
	{
		::fsync( fileno(fFileRef) );
		fWroteData = false;
	}
	::fclose( fFileRef );
	
	fFileRef = kBadFileRef;
	this->syncdisk();

} // close


//--------------------------------------------------------------------------------------------------
//	* freespace
//
//--------------------------------------------------------------------------------------------------

sInt64 CFile::freespace ( void ) const throw ( SInt16 )
{
	struct statfs	ssStats;

	if ( fFileRef == kBadFileRef )
	{
		throw( (SInt16)ds_fnOpnErr );
	}

	::fstatfs( fileno(fFileRef), &ssStats );

	return( (sInt64)ssStats.f_bsize * (sInt64)ssStats.f_bavail );

} // freespace


//--------------------------------------------------------------------------------------------------
//	* ReadBlock ()
//
//		block read that returns some useful info like the number of bytes read
//--------------------------------------------------------------------------------------------------

ssize_t CFile::ReadBlock ( void *pData, streamsize nBytes ) throw ( SInt16 )
{
	register ssize_t	lRead	= 0;
			 off_t		offset	= 0;

	if ( fFileRef == kBadFileRef )
	{
		throw( (SInt16)ds_fnOpnErr );
	}

	if ( !fReadPosOK )
	{
		offset = ::lseek( fileno( fFileRef ), fReadPos, SEEK_SET );
		if ( -1 == offset )
		{
			throw( (SInt16) ds_gfpErr );
		}
	}

	lRead = ::read( fileno( fFileRef ), pData, nBytes );
	if ( -1 == lRead )
	{
		throw( (SInt16) ds_readErr );
	}

	// Update the position marker.
	fReadPos   += (sInt64)lRead;
	fReadPosOK	= true;
	fWritePosOK	= false;

	return( lRead );

} // ReadBlock


//--------------------------------------------------------------------------------------------------
//	* Read ()
//
//		block io
//--------------------------------------------------------------------------------------------------

CFile& CFile::Read ( void *pData, streamsize nBytes ) throw ( SInt16 )
{
	register ssize_t	lRead;
			 off_t		offset;

	if ( fFileRef == kBadFileRef )
	{
		throw( (SInt16)ds_fnOpnErr );
	}

	if ( !fReadPosOK )
	{
		offset = ::lseek( fileno( fFileRef ), fReadPos, SEEK_SET );
		if ( -1 == offset )
		{
			throw( (SInt16)ds_gfpErr );
		}
	}

	lRead = ::read( fileno( fFileRef ), pData, nBytes );
	if ( -1 == lRead )
	{
		throw( (SInt16)ds_readErr );
	}

	// Update the position marker.
	fReadPos   += (sInt64)lRead;
	fReadPosOK	= true;
	fWritePosOK	= false;

	return( *this );

} // Read


//--------------------------------------------------------------------------------------------------
//	* write
//
//--------------------------------------------------------------------------------------------------

CFile& CFile::write ( const void *pData, streamsize nBytes ) throw ( SInt16 )
{
	int					i			= 0;
	register ssize_t	lWrite		= 0;
	register struct tm *tmPtr		= nil;
	char			   *pBuff_1 	= nil;
	char			   *pBuff_2 	= nil;
	time_t				seconds		= 0;
	int					error		= eDSNoErr;
	streamsize			strSize		= 0;
	streamsize			buffSize	= 0;
	bool				bRollLog	= false;
	char				dateStr	[ 256 ];

	fLock.WaitLock();

	try
	{
		if ( fFileRef == kBadFileRef )
		{
			throw( (SInt16)ds_fnOpnErr );
		}

		if ( !fWritePosOK )
		{
			if ( -1 == ::lseek( fileno( fFileRef ), fWritePos, SEEK_SET) )
			{
				throw( (SInt16)ds_gfpErr );
			}
		}
		::fflush( fFileRef );

		if ( -1 == (lWrite = ::fwrite( pData, sizeof( char ), nBytes, fFileRef )) )
		{
			throw( (SInt16) ds_writErr );
		}
		fWroteData = true;
		::fflush( fFileRef );

		if ( fRollLog == true )
		{
			seconds = (time_t)::time( nil );
			if ( seconds > (fLastChecked + 60) )
			{
				if ( this->FileSize() > 2048000 )
				{
					bRollLog = true;
				}
			}
			if ( seconds > (fOpenTime + 86400) )
			{
				bRollLog = true;
			}

			if ( bRollLog == true )
			{
				// Create temp buffers
				//	Name of the file plus the new extension plus more
				buffSize = ::strlen( fFilePath ) + 1024;

				pBuff_1 = (char *)::calloc( buffSize, sizeof( char ) );
				if ( pBuff_1 == nil )
				{
					throw( (SInt16)eMemoryAllocError );
				}

				pBuff_2 = (char *)::calloc( buffSize, sizeof( char ) );
				if ( pBuff_2 == nil )
				{
					throw( (SInt16)eMemoryAllocError );
				}

				// Remove the oldest
				::sprintf( pBuff_1, "%s.%u", fFilePath, kMaxFiles );

				// It may not exist so ignore the error
				(void)::remove( pBuff_1 );

				// Now we rename the files
				for ( i = (kMaxFiles - 1); i >= 0; i-- )
				{
					// New name
					::sprintf( pBuff_1, "%s.%u", fFilePath, i + 1 );

					// Old name
					if ( i == 0 )
					{
						::sprintf( pBuff_2, "%s", fFilePath );
					}
					else
					{
						::sprintf( pBuff_2, "%s.%u", fFilePath, i );
					}

					// Rename it
					// It may not exist so ignore the error except for the current file
					error = rename( pBuff_2, pBuff_1 );
					if ( (error != eDSNoErr) && (i == 0) )
					{
						// Log the error and bail
						::sprintf( pBuff_1, kRenameErrorStr, error );
						lWrite = ::fwrite( pBuff_1, sizeof( char ), ::strlen( pBuff_1 ), fFileRef );
						if ( lWrite == -1 )
						{
							free( pBuff_1 );
							free( pBuff_2 );
							throw( (SInt16)ds_writErr );
						}
						fWroteData = true;
						::fflush( fFileRef );

						free( pBuff_1 );
						free( pBuff_2 );
						throw( (SInt16) ds_permErr );
					}

					// Only tag the current log file
					if ( i == 0 )
					{
						// Log the end tag
						tmPtr = ::localtime( (time_t *)&seconds );
						::strftime( dateStr, 255, "%b %e %Y %X", tmPtr );	// Dec 25 1998 12:00:00
		
						::sprintf( pBuff_1, kRollLogMessageEndStr, dateStr );
						strSize = ::strlen( pBuff_1 );
		
						lWrite = ::fwrite( pBuff_1, sizeof( char ), ::strlen( pBuff_1 ), fFileRef );
						if ( lWrite == -1 )
						{
							free( pBuff_1 );
							free( pBuff_2 );
							throw( (SInt16)ds_writErr );
						}
						fWroteData = true;
						::fflush( fFileRef );
					}
				}

				// Close the old file and open a new one
				this->close();
				this->open( fFilePath, true );

				// Tag the head of the new log
				::sprintf( pBuff_1, kRollLogMessageStartStr, dateStr );
				strSize = ::strlen( pBuff_1 );

				lWrite = ::fwrite( pBuff_1, sizeof( char ), ::strlen( pBuff_1 ), fFileRef );
				if ( lWrite == -1 )
				{
					free( pBuff_1 );
					free( pBuff_2 );
					throw( (SInt16)ds_writErr );
				}
				fWroteData = true;
				::fflush( fFileRef );

				// Free up the memory
				free( pBuff_1 );
				free( pBuff_2 );
				pBuff_1 = nil;
				pBuff_2 = nil;
			}
		}

		// Update the position marker.
		fWritePos	+= (sInt64)lWrite;
		fWritePosOK	 = true;
		fReadPosOK	 = false;
	}

	catch ( SInt16 err )
	{
		fLock.SignalLock();
		throw( err );
	}

	catch ( ... )
	{
		fLock.SignalLock();
		throw( kiIOAbort );
	}

	fLock.SignalLock();

	return( *this );

} // write


//--------------------------------------------------------------------------------------------------
//	* FileSize
//
//		positioning
//--------------------------------------------------------------------------------------------------

sInt64 CFile::FileSize ( void ) throw ( SInt16 )
{
	struct stat		ssFile;

	if ( fFileRef == kBadFileRef )
	{
		throw( (SInt16)ds_fnOpnErr );
	}

	if ( -1 == ::fstat( fileno( fFileRef ), &ssFile ) )
	{
		throw( (SInt16)ds_gfpErr );
	}

	return( ssFile.st_size );

} // FileSize


//--------------------------------------------------------------------------------------------------
//	* ModDate
//--------------------------------------------------------------------------------------------------

void CFile::ModDate( struct	timespec *outModTime )
{
	if ( outModTime != NULL )
		memcpy( outModTime, &(fStatStruct.st_mtimespec), sizeof(fStatStruct.st_mtimespec) );
}

	
//--------------------------------------------------------------------------------------------------
//	* seekg ()
//
//--------------------------------------------------------------------------------------------------

CFile& CFile::seekg ( sInt64 lOffset, ios::seekdir inMark ) throw ( SInt16 )
{
	register sInt64	lEOF;

	if ( fFileRef == kBadFileRef )
	{
		throw( (SInt16)ds_fnOpnErr );
	}

	lEOF = FileSize();

	switch ( inMark )
	{
		case ios::beg:
			if ( fReadPos == lOffset )
			{
				return( *this );
			}
			if ( lOffset <= 0 )
			{
				fReadPos = 0;
			}
			else if ( lOffset > lEOF )
			{
				fReadPos = lEOF;
			}
			else
			{
				fReadPos = lOffset;
			}
			break;

		case ios::cur:
			if ( !lOffset )
			{
				return( *this );
			}

			fReadPos += lOffset;
			if ( fReadPos <= 0 )
			{
				fReadPos = 0;
			}
			else if ( fReadPos > lEOF )
			{
				fReadPos = lEOF;
			}
			break;

		case ios::end:
		default:
			if ( lOffset > 0 )
			{
				fReadPos = lEOF;
			}
			else	// Got EOF and lOffset <= 0
			{
				fReadPos = lEOF + lOffset;
			}
			break;
	}

	fReadPosOK	= false;
	fWritePosOK	= false;

	return( *this );

} // seekg


//--------------------------------------------------------------------------------------------------
//	* seekp
//
//--------------------------------------------------------------------------------------------------

CFile& CFile::seekp ( sInt64 lOffset, ios::seekdir inMark ) throw ( SInt16 )
{
	register sInt64	lEOF;

	if ( fFileRef == kBadFileRef )
	{
		throw( (SInt16)ds_fnOpnErr );
	}

	switch ( inMark )
	{
		case ios::beg:
			if ( fWritePos == lOffset )
			{
				return( *this );
			}
			fWritePos = lOffset;
			break;

		case ios::cur:
			if (!lOffset)
			{
				return( *this );
			}
			fWritePos += lOffset;
			break;

		case ios::end:
		default:
			lEOF = FileSize();
			fWritePos = lEOF + lOffset;
			break;
	}

	if ( fWritePos < 0 )
	{
		fWritePos = 0;
	}

	fReadPosOK = false;
	fWritePosOK = false;

	return( *this );

} // seekp


//--------------------------------------------------------------------------------------------------
//	* syncdisk
//
//--------------------------------------------------------------------------------------------------

void CFile::syncdisk ( void ) const throw()
{
	// sync() blocks RAIDs, switch to fsyncs before closing the file
} // syncdisk


//--------------------------------------------------------------------------------------------------
//	* is_open
//
//--------------------------------------------------------------------------------------------------

int CFile::is_open( void ) const throw()
{
	return ( fFileRef != kBadFileRef );
} // is_open


//--------------------------------------------------------------------------------------------------
//	* flush
//
//--------------------------------------------------------------------------------------------------

