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

#ifndef __CFile_h__
#define __CFile_h__	1

#include <DirectoryServiceCore/DSCThread.h>	// for CThread::Yield()
#include <DirectoryServiceCore/DSMutexSemaphore.h>

typedef		long long		sInt64;

#include <unistd.h>		// for sync()
#include <limits.h>		// for PATH_MAX
#include <stdio.h>		// for PATH_MAX, FILE, fopen(), etc.
#include <fstream>	// for classes fstream and ios
#include <sys/stat.h>			// for fstat(), stat() and structs

using namespace std;

static FILE	* const		kBadFileRef = NULL;
static const int		kMaxFiles	= 8;

//typedef ios		ios_base;
typedef char	CFileSpec[ PATH_MAX ];
typedef char   *CFileSpecPtr;

#define		kRollLogMessageStartStr	"-- Start: Server rolled log on: %s --\n"
#define		kRollLogMessageEndStr	"-- End: Server rolled log on: %s --\n"
#define		kMemFullErrStr			"*** Error:  Could not roll file, memory full ***\n"
#define		kRenameErrorStr			"*** Error:  Received error %d during rename ***\n"

class CFile
{
public:
				CFile	( void )	throw();
				CFile	( const char *inFileSpec, const Boolean inCreate = false, const Boolean inRoll = false )	throw( SInt16 );
	virtual	   ~CFile	( void );

	virtual	void open	( const char *inFileSpec, const Boolean inCreate = false )	throw( SInt16 );

	// filesystem operations
	virtual	sInt64		freespace	( void ) const	throw( SInt16 );
	virtual	void		syncdisk	( void ) const	throw();

	virtual	int			is_open		( void ) const	throw();
	virtual	CFile&		seteof		( sInt64 lEOF )	throw( SInt16 );
	virtual	CFile&		flush		( void )		throw( SInt16 );
	virtual	void		close		( void )		throw( SInt16 );

	// block io
	virtual	ssize_t	ReadBlock		( void *s, streamsize n )					throw( SInt16 );
	virtual	CFile&	Read			( void *s, streamsize n )					throw( SInt16 );
			CFile&	read			( char *s, streamsize n )					throw( SInt16 );
			CFile&	read			( unsigned char *s, streamsize n )			throw( SInt16 );
			CFile&	read			( signed char *s, streamsize n )			throw( SInt16 );

	virtual	CFile& write			( const void *s, streamsize n )				throw( SInt16 );
			CFile& write			( const char *s, streamsize n )				throw( SInt16 );
			CFile& write			( const unsigned char *s, streamsize n )	throw( SInt16 );
			CFile& write			( const signed char *s, streamsize n )		throw( SInt16 );

	// positioning
	virtual	CFile&	seekg		( sInt64 lOffset, ios::seekdir inMark = ios::beg )	throw( SInt16 );
	virtual	sInt64	tellg		( void )	throw( SInt16 );

	virtual	CFile&	seekp		( sInt64 lOffset, ios::seekdir inMark = ios::beg )	throw( SInt16 );
	virtual	sInt64	tellp		( void )	throw( SInt16 );

	virtual	sInt64	FileSize	( void )	throw( SInt16 );
	virtual void	ModDate		( struct timespec *outModTime );
	
protected:
	DSMutexSemaphore	fLock;
	char		   *fFilePath;
	FILE		   *fFileRef;
	time_t			fOpenTime;
	time_t			fLastChecked;
	bool			fRollLog;
	sInt64			fReadPos;
	sInt64			fWritePos;
	bool			fReadPosOK;
	bool			fWritePosOK;
	struct stat		fStatStruct;
	bool			fWroteData;
};

inline CFile& CFile::flush ( void ) throw( SInt16 )
#if USE_UNIXIO
{ return *this; }	// A no-op, because I'm using unbuffered I/O.
#else
{ ::fflush ( fFileRef ); return *this; }
#endif

inline CFile&	CFile::read	 ( char *s, streamsize n )					throw( SInt16 ) { return this->Read( (void *)s, n ); }
inline CFile&	CFile::read	 ( unsigned char *s, streamsize n )			throw( SInt16 ) { return this->Read( (void *)s, n ); }
inline CFile&	CFile::read	 ( signed char *s, streamsize n )			throw( SInt16 ) { return this->Read( (void *)s, n ); }
inline CFile&	CFile::write ( const char *s, streamsize n )			throw( SInt16 ) { return this->write( (void *)s, n ); }
inline CFile&	CFile::write ( const unsigned char *s, streamsize n )	throw( SInt16 ) { return this->write( (void *)s, n ); }
inline CFile&	CFile::write ( const signed char *s, streamsize n )		throw( SInt16 ) { return this->write( (void *)s, n ); }
inline sInt64	CFile::tellg ( void )									throw( SInt16 ) { return( fReadPos ); }
inline sInt64	CFile::tellp ( void )									throw( SInt16 ) { return( fWritePos ); }

#endif	// __CFile_h__
