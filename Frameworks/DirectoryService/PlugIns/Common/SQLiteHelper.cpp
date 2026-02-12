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

#include "SQLiteHelper.h"

#include <sys/stat.h>
#include <DirectoryServiceCore/CLog.h>

SQLiteHelper::SQLiteHelper( const char *inDatabasePath, uint32_t inExpectedVersion )
{
	fDatabase = NULL;
	fDatabasePath = strdup( inDatabasePath );
	fVersion = inExpectedVersion;
	fNewDatabase = false;
}

SQLiteHelper::~SQLiteHelper( void )
{
	CloseDatabase();
	DSFree( fDatabasePath );
}

bool SQLiteHelper::OpenDatabase( bool inIntegrityCheck )
{
	struct stat statBuffer	= { 0 };
	
	fMutex.WaitLock();
	
	if ( fDatabase == NULL ) {
		if ( stat(fDatabasePath, &statBuffer) == 0 && statBuffer.st_size == 0 ) {
			inIntegrityCheck = false;
			CreateDatabase();
		}
		else {
			// if it fails to open here, we'll remove the file and attempt to open it again in the next if
			int status = sqlite3_open( fDatabasePath, &fDatabase );
			if ( SQLITE_OK != status ) {
				inIntegrityCheck = false;
				CreateDatabase();
			}
		}
	}
	
	if ( fDatabase != NULL ) {
		if ( (true == inIntegrityCheck && IntegrityCheck() == false) || IsDatabaseVersionCurrent() == false ) {
			CreateDatabase();
		}
	}
	
	// we lower the cache size to bare minimum, none of our tables are big enough and we have no need for cache
	// we don't do complex queries that require multiple rows in memory
	// let the disk cache for the system do most of the work as our indexes shouldn't be too large
	ExecSync( "PRAGMA cache_size = 2" ); // 4k is plenty for anything we do
	
	fMutex.SignalLock();
	
	return (fDatabase != NULL);
}
			
void SQLiteHelper::CreateDatabase( void )
{
	fMutex.WaitLock();
	
	if ( fDatabase != NULL ) {
		RemoveDatabase();
	}
	
	int status = sqlite3_open( fDatabasePath, &fDatabase );
	if ( SQLITE_OK == status ) {
		
		char	pCommand[256];
		
		DbgLog( kLogDebug, "SQLiteHelper::CreateDatabase - opened the DB at '%s'", fDatabasePath );
		snprintf( pCommand, sizeof(pCommand), "PRAGMA user_version = %d", fVersion );
		
		status = ExecSync( pCommand );
	}
	else {
		DbgLog( kLogError, "SQLiteHelper::CreateDatabase - failed to open the DB for creation at '%s' (%d)", fDatabasePath, status );
	}
	
	if ( SQLITE_OK != status && SQLITE_DONE != status ) {
		DbgLog( kLogDebug, "SQLiteHelper::CreateDatabase - removing the DB for '%s' due to failure to create (%d)", fDatabasePath, status );
		RemoveDatabase();
	}
	
	fNewDatabase = true;
	
	fMutex.SignalLock();
}

void SQLiteHelper::CloseDatabase( void )
{
	fMutex.WaitLock();

	if ( fDatabase != NULL ) {
		sqlite3_close( fDatabase );
		fDatabase = NULL;
		
		DbgLog( kLogPlugin, "SQLiteHelper::CloseDatabase is closing database '%s'", fDatabasePath );
	}
	
	fMutex.SignalLock();
}

void SQLiteHelper::RemoveDatabase( void )
{
	char	journal[PATH_MAX];
	
	fMutex.WaitLock();

	CloseDatabase();
	
	strlcpy( journal, fDatabasePath, sizeof(journal) );
	strlcat( journal, "-journal", sizeof(journal) );
	
	unlink( fDatabasePath );
	unlink( journal );

	fMutex.SignalLock();

	fNewDatabase = true;

	DbgLog( kLogNotice, "SQLiteHelper::RemoveDatabase - Removed database '%s' and journal file", fDatabasePath );
}

int SQLiteHelper::ExecSync( const char *command, int length )
{
	int				status	= SQLITE_ERROR;
	sqlite3_stmt	*pStmt	= NULL;
	
	fMutex.WaitLock();

	if ( fDatabase != NULL )
	{
		status = sqlite3_prepare_v2( fDatabase, command, length, &pStmt, NULL );	
		if ( SQLITE_OK == status )
		{
			status = sqlite3_step( pStmt );
			sqlite3_finalize( pStmt );
		}
	}
	
	fMutex.SignalLock();

	return (status == SQLITE_DONE ? SQLITE_OK : status);
}

int SQLiteHelper::ExecSyncWithTypes( const char *command, int length, ... )
{
	int				status	= SQLITE_ERROR;
	sqlite3_stmt	*pStmt	= NULL;
	
	fMutex.WaitLock();
	
	if ( fDatabase != NULL )
	{
		status = sqlite3_prepare_v2( fDatabase, command, length, &pStmt, NULL );	
		if ( SQLITE_OK == status )
		{
			int				argIndex;
			va_list			args;
			int				argType;
			const char *	textValue;
			int				intValue;
			sqlite_int64	int64Value;
			double			doubleValue;
			
			va_start( args, length );

			do
			{
				argType = va_arg( args, int );
				if ( argType == kSQLTypeDone )
					break;
				
				argIndex = va_arg( args, int );
				
				switch ( argType )
				{
					case kSQLTypeDouble:
						doubleValue = va_arg( args, double );
						status = sqlite3_bind_double( pStmt, argIndex, doubleValue );
						break;
					case kSQLTypeInt:
						intValue = va_arg( args, int );
						status = sqlite3_bind_int( pStmt, argIndex, intValue );
						break;
					case kSQLTypeInt64:
						int64Value = va_arg( args, sqlite_int64 );
						status = sqlite3_bind_int64( pStmt, argIndex, int64Value );
						break;
					case kSQLTypeText:
						textValue = va_arg( args, const char * );
						status = sqlite3_bind_text( pStmt, argIndex, textValue, strlen(textValue), SQLITE_TRANSIENT );
						break;
					case kSQLTypeDone:
						break;
				}
			} while ( status == SQLITE_OK );
			
			va_end( args );

			// if status is OK, then let's execute the command
			if ( status == SQLITE_OK )
				status = sqlite3_step( pStmt );
			
			sqlite3_finalize( pStmt );
		}
	}
	
	fMutex.SignalLock();
	
	return (status == SQLITE_DONE ? SQLITE_OK : status);
}

int SQLiteHelper::Prepare( const char *command, int length, sqlite3_stmt **stmt, const char **pzTail )
{
	int result = SQLITE_ERROR;
	
	fMutex.WaitLock();
	
	if ( fDatabase != NULL ) {
		result = sqlite3_prepare_v2( fDatabase, command, length, stmt, pzTail );	
	}

	fMutex.SignalLock();

	return (result == SQLITE_DONE ? SQLITE_OK : result);
}

int SQLiteHelper::Step( sqlite3_stmt *inStmt )
{
	int	status	= SQLITE_ERROR;
	
	fMutex.WaitLock();
	
	if ( fDatabase != NULL && inStmt != NULL ) {
		status = sqlite3_step( inStmt );
	}
	
	fMutex.SignalLock();
	
	return (status == SQLITE_DONE ? SQLITE_OK : status);
}

int SQLiteHelper::Finalize( sqlite3_stmt *&inStmt )
{
	int	status	= SQLITE_ERROR;
	
	fMutex.WaitLock();
	
	if ( fDatabase != NULL && inStmt != NULL ) {
		status = sqlite3_finalize( inStmt );
		inStmt = NULL;
	}
	
	fMutex.SignalLock();
	
	return status;
}

bool SQLiteHelper::BeginTransaction( const char *inName )
{
	char	command[256];
	bool	result;

	fMutex.WaitLock();
	
	if ( inName != NULL ) {
		snprintf( command, sizeof(command), "BEGIN TRANSACTION %s", inName );
	}
	else {
		strlcpy( command, "BEGIN TRANSACTION", sizeof(command) );
	}
	
	result = ( ExecSync(command) == SQLITE_DONE );

	return result;
}

void SQLiteHelper::EndTransaction( const char *inName )
{
	char	command[256];
	
	if ( inName != NULL ) {
		snprintf( command, sizeof(command), "END TRANSACTION %s", inName );
	}
	else {
		strlcpy( command, "END TRANSACTION", sizeof(command) );
	}
	
	ExecSync( command );
	
	fMutex.SignalLock();
}

void SQLiteHelper::RollbackTransaction( const char *inName )
{
	char	command[256];
	
	fMutex.WaitLock();
	
	if ( inName != NULL ) {
		snprintf( command, sizeof(command), "ROLLBACK TRANSACTION %s", inName );
	}
	else {
		strlcpy( command, "ROLLBACK TRANSACTION", sizeof(command) );
	}
	
	ExecSync( command );
	
	fMutex.SignalLock();
}

bool SQLiteHelper::IntegrityCheck( void )
{
	sqlite3_stmt	*pStmt		= NULL;
	bool			bValidDB	= false;	// default to invalid DB
	
	if ( fDatabase != NULL )
	{
		ExecSync( "PRAGMA cache_size = 2" ); // 4k is plenty for this
		
		int status = sqlite3_prepare( fDatabase, "PRAGMA integrity_check", -1, &pStmt, NULL );	
		if ( SQLITE_OK == status ) {
			status = sqlite3_step( pStmt );
			
			// we will loop looking for "ok", in case SQL decides to add some verbosity for good DBs
			while ( SQLITE_ROW == status ) {
				if ( sqlite3_column_type(pStmt, 0) == SQLITE_TEXT ) {
					const char *text = (const char *) sqlite3_column_text( pStmt, 0 );
					if ( strcmp(text, "ok") == 0 ) {
						bValidDB = true;
					}
				}
				
				status = sqlite3_step( pStmt );
			}
			
			sqlite3_finalize( pStmt );
		}
		
		if ( bValidDB == false )
			DbgLog( kLogCritical, "Database '%s' failed integrity check", fDatabasePath );
		else
			SrvrLog( kLogApplication, "Database '%s' passed integrity check", fDatabasePath );
	}
	
	return bValidDB;
}

bool SQLiteHelper::IsDatabaseVersionCurrent( void )
{
	bool			bIsCurrent	= false;
	uint32_t		tempVers	= 0;
	sqlite3_stmt	*pStmt		= NULL;

	if ( fDatabase != NULL )
	{
		int status = sqlite3_prepare( fDatabase, "PRAGMA user_version", -1, &pStmt, NULL );	
		if ( SQLITE_OK == status ) {
			
			status = sqlite3_step( pStmt );
			if ( SQLITE_ROW == status ) {
				
				if ( sqlite3_column_type(pStmt, 0) == SQLITE_INTEGER ) {

					tempVers = sqlite3_column_int( pStmt, 0 );
					if ( tempVers == fVersion ) {
						DbgLog( kLogDebug, "SQLiteHelper::IsDatabaseVersionCurrent - Database '%s' hash %u", fDatabasePath, tempVers );
						bIsCurrent = true;
					}
				}
			}
			
			sqlite3_finalize( pStmt );
		}
		
		if ( bIsCurrent == false && tempVers > 0)
			DbgLog( kLogError, "SQLiteHelper::IsDatabaseVersionCurrent - Database '%s' expected hash %u got %u", fDatabasePath, 
				    fVersion, tempVers );
	}
	
	return bIsCurrent;
}

