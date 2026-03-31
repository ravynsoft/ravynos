/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.2 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*!
 * @header CInternalDispatch
 */

#include "CInternalDispatch.h"
#include "CLog.h"

pthread_key_t	CInternalDispatch::fThreadKey	= NULL;

CInternalDispatch::CInternalDispatch( void )
{
	fInternalDispatchStackHeight = -1;
	bzero( fInternalMsgDataList, sizeof(fInternalMsgDataList) );
}

CInternalDispatch::~CInternalDispatch()
{
	for ( int idx = 0; idx < kMaxInternalDispatchRecursion; idx++ )
	{
		if ( idx > 0 && fInternalMsgDataList[idx] != NULL )
			DbgLog( kLogError, "CInternalDispatch::~CInternalDispatch - buffer was not popped for level <%d>", idx );
		
		DSFree( fInternalMsgDataList[idx] );
	}
}

void CInternalDispatch::SwapCurrentMessageBuffer( sComData* inOldMsgData, sComData* inNewMsgData )
{
	if ( fInternalDispatchStackHeight > -1 && fInternalDispatchStackHeight < kMaxInternalDispatchRecursion &&
		 inOldMsgData != NULL && inNewMsgData != NULL &&
		 fInternalMsgDataList[fInternalDispatchStackHeight] == inOldMsgData )
	{
		fInternalMsgDataList[fInternalDispatchStackHeight] = inNewMsgData;
	}
}

sComData* CInternalDispatch::GetCurrentMessageBuffer( void )
{
	// we defer creating the internal buffer until we actually need it since it may not be used
	if ( fInternalDispatchStackHeight > -1 && fInternalDispatchStackHeight < kMaxInternalDispatchRecursion ) {
		return fInternalMsgDataList[fInternalDispatchStackHeight];
	}
	else if ( fInternalDispatchStackHeight == -1 ) {
		PushCurrentMessageBuffer();
		return fInternalMsgDataList[fInternalDispatchStackHeight];
	}
	
	return NULL;
}

void CInternalDispatch::PopCurrentMessageBuffer( void )
{
	if ( fInternalDispatchStackHeight > -1 && fInternalDispatchStackHeight < kMaxInternalDispatchRecursion ) {
		DSFree( fInternalMsgDataList[fInternalDispatchStackHeight] );
	}
	
	if ( fInternalDispatchStackHeight > -1 ) {
		fInternalDispatchStackHeight--;
	}
}

void CInternalDispatch::PushCurrentMessageBuffer( void )
{
	if ( fInternalDispatchStackHeight < kMaxInternalDispatchRecursion ) {
		fInternalDispatchStackHeight++;
		fInternalMsgDataList[fInternalDispatchStackHeight] = (sComData *) calloc( 1, sizeof(sComData) + kMsgBlockSize );
		
		if ( fInternalMsgDataList[fInternalDispatchStackHeight] != NULL ) {
			fInternalMsgDataList[fInternalDispatchStackHeight]->fDataSize	= kMsgBlockSize;
			fInternalMsgDataList[fInternalDispatchStackHeight]->fDataLength	= 0;
		}
	}
	else
	{
		DbgLog( kLogError, "CInternalDispatch::PushCurrentMessageBuffer - internal dispatch stack overrun - aborting" );
		abort();
	}
}

#pragma mark Static member functions

void CInternalDispatch::CreateThreadKey( void )
{
	pthread_key_create( &fThreadKey, DeleteThreadKey );
}

void CInternalDispatch::DeleteThreadKey( void *key )
{
	CInternalDispatch *thread = (CInternalDispatch *) key;
	
	DSDelete( thread );
}

void CInternalDispatch::AddCapability( void )
{
	static pthread_once_t once_init = PTHREAD_ONCE_INIT;
	
	pthread_once( &once_init, CreateThreadKey );
	
	if ( pthread_getspecific(fThreadKey) == NULL ) {
		pthread_setspecific( fThreadKey, new CInternalDispatch );
		DbgLog( kLogDebug, "CInternalDispatch::AddCapability has made thread %X internally dispatchable", pthread_self() );
	}
}

CInternalDispatch *CInternalDispatch::GetThreadInternalDispatch( void )
{
	return (CInternalDispatch *) pthread_getspecific( fThreadKey );
}
