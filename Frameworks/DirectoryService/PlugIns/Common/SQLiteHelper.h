/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#ifndef __SQLITEHELPER_H
#define __SQLITEHELPER_H

#include <sqlite3.h>
#include <stdint.h>
#include <DirectoryServiceCore/DSSemaphore.h>
#include <DirectoryServiceCore/DSMutexSemaphore.h>

typedef enum
{
	kSQLTypeDouble = 1,
	kSQLTypeInt,
	kSQLTypeInt64,
	kSQLTypeText,
	kSQLTypeDone	= 99
} SQLValueType;

class SQLiteHelper
{
	public:
						SQLiteHelper( const char *inDatabasePath, uint32_t inExpectedVersion );
						~SQLiteHelper( void );

		// will return true if succeeds, note it can be an empty database if integrity check fails or an old version
		bool			OpenDatabase( bool inIntegrityCheck = false );
		void			CreateDatabase( void );
		void			CloseDatabase( void );
		void			RemoveDatabase( void );
	
#if defined(DEBUG_LOCKS)
	#define LockDatabase()		LockDatabaseDebug( __FILE__, __LINE__ )
	#define UnlockDatabase()	UnlockDatabaseDebug( __FILE__, __LINE__ )
		inline void		LockDatabaseDebug( const char *file, int line ) { fMutex.WaitDebug(file, line); }
		inline void		UnlockDatabaseDebug( const char *file, int line ) { fMutex.SignalDebug(file, line); }
#elif defined(DEBUG_LOCKS_HISTORY)
	#define LockDatabase()		LockDatabaseDebug( __FILE__, __LINE__ )
	#define UnlockDatabase()	UnlockDatabaseDebug( __FILE__, __LINE__ )
		inline void		LockDatabaseDebug( const char *file, int line ) { fMutex.WaitDebugHistory(file, line); }
		inline void		UnlockDatabaseDebug( const char *file, int line ) { fMutex.SignalDebugHistory(file, line); }
#else
		inline void		LockDatabase( void ) { fMutex.WaitLock(); }
		inline void		UnlockDatabase( void ) { fMutex.SignalLock(); }
#endif

		int				ExecSync( const char *command, int length = -1 );
		int				ExecSyncWithTypes( const char *command, int length, ... );	// SQLValueType, value ending with kTypeDone
	
		// must call BeginTransaction first
		int				Prepare( const char *command, int length, sqlite3_stmt **stmt, const char **pzTail = NULL );
		int				Step( sqlite3_stmt *inStmt );
		int				Finalize( sqlite3_stmt *&inStmt );

		bool			BeginTransaction( const char *inName = NULL );
		void			EndTransaction( const char *inName = NULL );
		void			RollbackTransaction( const char *inName = NULL );
	
		bool			IsNewDatabase( void ) { return fNewDatabase; }
	
	private:
		DSMutexSemaphore	fMutex;
		sqlite3				*fDatabase;
		char				*fDatabasePath;
		uint32_t			fVersion;
		bool				fNewDatabase;
	
	private:
		bool		IntegrityCheck( void );
		bool		IsDatabaseVersionCurrent( void );
};

#endif
