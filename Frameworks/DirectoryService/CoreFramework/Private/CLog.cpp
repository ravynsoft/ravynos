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
 * @header CLog
 * Implementation of the logging mechanism.
 */

#include <string.h>					// for memset() and strcpy()

#include <NSSystemDirectories.h>	// for NSSearchPath*()
#include <sys/types.h>				// for mode_t
#include <sys/stat.h>				// for mkdir() and stat()
#include <libkern/OSAtomic.h>

#include "CLog.h"
#include "COSUtils.h"
#include "DSMutexSemaphore.h"

const char *kgStringMessageEndOfLine = "\r\n";

// ----------------------------------------------------------------------------
//	* CLog Class Globals
// ----------------------------------------------------------------------------

OptionBits	CLog::fSrvrLogFlags		= kLogMeta;
OptionBits	CLog::fErrLogFlags		= kLogMeta | kLogError;
OptionBits	CLog::fDbgLogFlags		= kLogEverything;
OptionBits	CLog::fInfoLogFlags		= kLogMeta;

static passthru_logging_fn	passthru_log_message = NULL;

//--------------------------------------------------------------------------------------------------
//	* Initialize()
//
//--------------------------------------------------------------------------------------------------

SInt32 CLog::Initialize (	OptionBits srvrFlags,	OptionBits errFlags,
							OptionBits debugFlags,	OptionBits infoFlags,
							bool inOpenDbgLog, bool inOpenInfoLog,
							bool inLocalOnlyMode, passthru_logging_fn passthru )
{
	fSrvrLogFlags	= srvrFlags;
	fErrLogFlags	= errFlags;
	fDbgLogFlags	= debugFlags;
	fInfoLogFlags	= infoFlags;
	passthru_log_message = passthru;
	
	return eDSNoErr;

} // Initialize


//--------------------------------------------------------------------------------------------------
//	* Deinitialize()
//
//--------------------------------------------------------------------------------------------------

void CLog::Deinitialize ( void )
{
} // Deinitialize


//--------------------------------------------------------------------------------------------------
//	* StartLogging()
//
//--------------------------------------------------------------------------------------------------

void CLog::StartLogging ( eLogType inWhichLog, UInt32 inFlag )
{
	switch ( inWhichLog )
	{
		case keServerLog:
			fSrvrLogFlags |= inFlag;
			break;

		case keErrorLog:
			fErrLogFlags |= inFlag;
			break;

		case keDebugLog:
			fDbgLogFlags |= inFlag;
			break;

		case keInfoLog:
			fInfoLogFlags |= inFlag;
			break;
	}
} // StartLogging


//--------------------------------------------------------------------------------------------------
//	* StopLogging()
//
//--------------------------------------------------------------------------------------------------

void CLog::StopLogging ( eLogType inWhichLog, UInt32 inFlag )
{
	switch ( inWhichLog )
	{
		case keServerLog:
			fSrvrLogFlags &= ~inFlag;
			break;

		case keErrorLog:
			fErrLogFlags &= ~inFlag;
			break;

		case keDebugLog:
			fDbgLogFlags &= ~inFlag;
			break;

		case keInfoLog:
			fInfoLogFlags &= ~inFlag;
			break;
	}
} // StopLogging


//--------------------------------------------------------------------------------------------------
//	* SetLoggingPriority()
//
//--------------------------------------------------------------------------------------------------

void CLog::SetLoggingPriority ( eLogType inWhichLog, UInt32 inPriority )
{
	switch ( inWhichLog )
	{
		case keServerLog:
			fSrvrLogFlags = kLogEverything;
			break;

		case keErrorLog:
			fErrLogFlags = kLogEverything;
			break;

		case keDebugLog:
			fDbgLogFlags = kLogEmergency;
			switch ( inPriority )
			{
				case 9:
				case 8:
					fDbgLogFlags |= kLogMeta;
				case 7:
					fDbgLogFlags |= kLogDebug;
				case 6:
					fDbgLogFlags |= kLogInfo;
				case 5:
					fDbgLogFlags |= kLogNotice | kLogPlugin | kLogPerformanceStats;
				case 4:
					fDbgLogFlags |= kLogWarning | kLogAPICalls;
				case 3:
					fDbgLogFlags |= kLogError | kLogThreads | kLogEndpoint | kLogConnection | kLogTCPEndpoint;
				case 2:
					fDbgLogFlags |= kLogCritical | kLogHandler | kLogMsgQueue | kLogMsgTrans;
				case 1:
					fDbgLogFlags |= kLogAlert | kLogApplication | kLogAssert | kLogListener; 
					break;
				case 0:	// we by default do emergency
					break;
			}
			
			DbgLog( kLogMeta, "Debug Logging priority set to %d - filter %X", inPriority, fDbgLogFlags );
			break;

		case keInfoLog:
			fInfoLogFlags = kLogEverything;
			break;
	}
} // SetLoggingPriority


//--------------------------------------------------------------------------------------------------
//	* ToggleLogging()
//
//--------------------------------------------------------------------------------------------------

void CLog::ToggleLogging ( eLogType inWhichLog, UInt32 inFlag )
{
	switch ( inWhichLog )
	{
		case keServerLog:
			if ( fSrvrLogFlags & inFlag )
			{
				fSrvrLogFlags &= ~inFlag;
			}
			else
			{
				fSrvrLogFlags |= inFlag;
			}
			break;

		case keErrorLog:
			if ( fErrLogFlags & inFlag )
			{
				fErrLogFlags &= ~inFlag;
			}
			else
			{
				fErrLogFlags |= inFlag;
			}
			break;

		case keDebugLog:
			if ( fDbgLogFlags & inFlag )
			{
				fDbgLogFlags &= ~inFlag;
			}
			else
			{
				fDbgLogFlags |= inFlag;
			}
			break;

		case keInfoLog:
			if ( fInfoLogFlags & inFlag )
			{
				fInfoLogFlags &= ~inFlag;
			}
			else
			{
				fInfoLogFlags |= inFlag;
			}
			break;
	}
} // ToggleLogging


//--------------------------------------------------------------------------------------------------
//	* IsLogging()
//
//--------------------------------------------------------------------------------------------------

bool CLog::IsLogging ( eLogType inWhichLog, UInt32 inFlag )
{
	switch ( inWhichLog )
	{
		case keServerLog:
			return( fSrvrLogFlags & inFlag );

		case keErrorLog:
			return( fErrLogFlags & inFlag );

		case keDebugLog:
			return( fDbgLogFlags & inFlag );

		case keInfoLog:
			return( fInfoLogFlags & inFlag );
	}

	return( false );

} // IsLogging


//--------------------------------------------------------------------------------------------------
//	* StartDebugLog ()
//
//--------------------------------------------------------------------------------------------------

void CLog::StartDebugLog ( void )
{
} // StartDebugLog


//--------------------------------------------------------------------------------------------------
//	* StopDebugLog ()
//
//--------------------------------------------------------------------------------------------------

void CLog::StopDebugLog ( void )
{
	fDbgLogFlags = kLogMeta;
} // StopDebugLog


//--------------------------------------------------------------------------------------------------
//	* StartErrorLog ()
//
//--------------------------------------------------------------------------------------------------

void CLog::StartErrorLog ( void )
{
} // StartErrorLog


//--------------------------------------------------------------------------------------------------
//	* StopErrorLog ()
//
//--------------------------------------------------------------------------------------------------

void CLog::StopErrorLog ( void )
{
	fErrLogFlags = kLogMeta;
} // StopErrorLog


//--------------------------------------------------------------------------------------------------
//	* StartInfoLog ()
//
//--------------------------------------------------------------------------------------------------

void CLog::StartInfoLog ( void )
{
	fInfoLogFlags = kLogEverything;
} // StartInfoLog


//--------------------------------------------------------------------------------------------------
//	* StopInfoLog ()
//
//--------------------------------------------------------------------------------------------------

void CLog::StopInfoLog ( void )
{
	fInfoLogFlags = kLogMeta;
} // StopInfoLog


//--------------------------------------------------------------------------------------------------
//	* GetServerLog ()
//
//--------------------------------------------------------------------------------------------------

CLog* CLog::GetServerLog ( void )
{
	return NULL;
} // GetServerLog


//--------------------------------------------------------------------------------------------------
//	* GetErrorLog ()
//
//--------------------------------------------------------------------------------------------------

CLog* CLog::GetErrorLog ( void )
{
	return NULL;
} // GetErrorLog


//--------------------------------------------------------------------------------------------------
//	* GetDebugLog ()
//
//--------------------------------------------------------------------------------------------------

CLog* CLog::GetDebugLog ( void )
{
	return NULL;
} // GetDebugLog


//--------------------------------------------------------------------------------------------------
//	* GetInfoLog ()
//
//--------------------------------------------------------------------------------------------------

CLog* CLog::GetInfoLog ( void )
{
	return NULL;
} // GetInfoLog


//--------------------------------------------------------------------------------------------------
//	* Lock()
//
//--------------------------------------------------------------------------------------------------

void CLog::Lock ( void )
{
	fLock->WaitLock();
} // Lock


//--------------------------------------------------------------------------------------------------
//	* UnLock()
//
//--------------------------------------------------------------------------------------------------

void CLog::UnLock ( void )
{
	fLock->SignalLock();
} // UnLock


// ----------------------------------------------------------------------------
//	* CLog Instance Methods
// ----------------------------------------------------------------------------
#pragma mark **** CLog Public Instance Methods ****

// ctor & dtor
CLog::CLog ( const char *inFile,
			 UInt32 inMaxLen,
			 OptionBits inFlags,
			 UInt32 type,
			 UInt32 creator )
{
	fFlags				= inFlags;
	fMaxLength			= inMaxLen;
	fOffset				= 0;
	fLength				= 0;

	::memset( fHooks, 0, sizeof( fHooks ) );

	// CFile throws.  We need to catch so we don't throw out of our constructor
	try {
		fFile = new CFile( inFile, true, (inFlags & kRollLog) );
		if ( !fFile->is_open() )
			throw(-1);
		
		if (fFile != nil)
		{
			// Get the length of the file and set the write pointer to EOF.
			fFile->seekp( 0, ios::end );
			fLength = fFile->tellp();
		}
	} catch ( ... ) {
		fFile = nil;
		fLength = 0;
	}

	fLock = new DSMutexSemaphore("CLog::fLock");
} // 


//--------------------------------------------------------------------------------------------------
//	* ~CLog()
//
//--------------------------------------------------------------------------------------------------

CLog::~CLog ( void )
{
	if ( fFile != nil )
	{
		delete( fFile );
		fFile = nil;
	}

	if ( fLock != nil )
	{
		delete( fLock );
		fLock = nil;
	}
} // ~CLog


//--------------------------------------------------------------------------------------------------
//	* SetMaxLength()
//
//--------------------------------------------------------------------------------------------------

void CLog::SetMaxLength ( UInt32 inMaxLen )
{
	fMaxLength = inMaxLen;
} // SetMaxLength


//--------------------------------------------------------------------------------------------------
//	* GetInfo()
//
//--------------------------------------------------------------------------------------------------

void CLog::GetInfo (	CFileSpec	&fileSpec,
						UInt32		&startOffset,
						UInt32		&dataLength,
						bool		&hasWrapped )
{
	::strcpy( fileSpec, fFileSpec );
	startOffset = (UInt32) fOffset;
	dataLength = (UInt32) fLength;
	hasWrapped = false;
} // GetInfo


//--------------------------------------------------------------------------------------------------
//	* ClearLog()
//
//--------------------------------------------------------------------------------------------------

SInt16 CLog::ClearLog ( void )
{
	try {
		if( fFile == nil ) throw ((SInt16) ds_fnfErr);
		
		fFile->seteof( 0 );
		fOffset = 0;
		fLength = 0;
	}
	
	catch ( SInt16 nErr )
	{
		return( nErr );
	}

	return( eDSNoErr );

} // ClearLog


//--------------------------------------------------------------------------------------------------
//	* AddHook()
//
//--------------------------------------------------------------------------------------------------

void CLog::AddHook ( AppendHook fpNewHook )
{
	AppendHook	*pHooks = fHooks;
	for ( ; *pHooks; pHooks++ )
	{
	}
	*pHooks = fpNewHook;
} // AddHook


//--------------------------------------------------------------------------------------------------
//	* Append()
//
//--------------------------------------------------------------------------------------------------

SInt16 CLog::Append ( const CString &line )
{
	CString			csTemp( 60 + line.GetLength() );

	fLock->WaitLock();

	if (fFlags & kThreadInfo)
	{
		csTemp.Sprintf( ( (fFlags & kDebugHdr) ? "%G %D %T - T[%X] - %S" : "%D %T - T[%X] - %S" ), pthread_self(), &line );
	}
	else
	{
		csTemp.Sprintf( ( (fFlags & kDebugHdr) ? "%G %D %T\t%S" : "%D %T\t%S" ) , &line);
	}

	// Append newline if necessary.
	if (csTemp[csTemp.GetLength () - 1] != '\n')
	{
		csTemp.Append ('\n');
	}

	try {
		if( fFile == nil ) throw ((SInt16) ds_fnfErr);

		// wrap up later
		fFile->write( csTemp.GetData(), csTemp.GetLength() );

		// Call all the hooks.
        AppendHook	*pHooks = fHooks ;
		for ( ; *pHooks ; pHooks++)
			(*pHooks) (csTemp);
	}

	catch ( SInt16 nErr )
	{
		fLock->SignalLock();
		return( nErr );
	}

	fLock->SignalLock();

	return( eDSNoErr );

} // Append

#pragma mark -
#pragma mark C functions

void SrvrLog ( SInt32 lType, const char *szpPattern, ... )
{
	// if log is not open, we start it now
	if ( szpPattern != nil )
	{
		va_list	args;
		va_start( args, szpPattern );
		
		bool isLogging = CLog::IsLogging(keDebugLog, lType); // no application log anymore, just direct to debug log
		if (passthru_log_message != NULL && isLogging == true) {
			CString message = CString(szpPattern, args);
			passthru_log_message(lType, message.GetData());
		}
	}
} // SrvrLog


void ErrLog ( SInt32 lType, const char *szpPattern, ... )
{
	// if log is not open, we start it now
	if ( szpPattern != nil )
	{
		va_list	args;
		va_start( args, szpPattern );
		
		bool isLogging = CLog::IsLogging(keDebugLog, kLogError); // no error log anymore, just direct to debug log
		if (passthru_log_message != NULL && isLogging == true) {
			CString message = CString(szpPattern, args);
			passthru_log_message(kLogError, message.GetData());
		}
	}
} // ErrLog


void DbgLog ( SInt32 lType, const char *szpPattern, ... )
{
	if (szpPattern == NULL)
		return;

	va_list	args;
	va_start( args, szpPattern );

	// certain types always get logged to error log
	if ((lType & (kLogEmergency | kLogAlert | kLogCritical | kLogError)) != 0) {
		bool isLogging = CLog::IsLogging(keErrorLog, lType);
		if (passthru_log_message != NULL && isLogging == true) {
			CString message = CString(szpPattern, args);
			passthru_log_message(lType, message.GetData());
		}
	}
	else {
		bool isLogging = CLog::IsLogging(keDebugLog, lType);
		if (passthru_log_message != NULL && isLogging == true) {
			CString message = CString(szpPattern, args);
			passthru_log_message(lType, message.GetData());
		}
	}
} // DbgLog


void InfoLog ( SInt32 lType, const char *szpPattern, ... )
{
	if ( (szpPattern != nil) && (CLog::GetInfoLog() != nil ) )
	{
		// Just in case it was deleted while I was waiting for it
		if ( CLog::GetInfoLog() != nil )
		{
			if ( CLog::IsLogging( keInfoLog, lType ) )
			{
				va_list	args;
				va_start( args, szpPattern );
				
				CLog::GetInfoLog()->Append( CString( szpPattern, args ) );
			}
		}
	}
} // GetInfoLog
