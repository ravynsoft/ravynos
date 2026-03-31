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

#ifndef __CInternalDispatch_h__
#define __CInternalDispatch_h__ 1

#include "DSCThread.h"
#include "PrivateTypes.h"
#include "SharedConsts.h"

class CInternalDispatch
{
	public:
						CInternalDispatch			( void );
		virtual		   ~CInternalDispatch			( void );
		
		//handles internal dispatch message buffers
		void			PushCurrentMessageBuffer	( void );
		void			PopCurrentMessageBuffer		( void );
		sComData	   *GetCurrentMessageBuffer		( void );
		void			SwapCurrentMessageBuffer	( sComData* inOldMsgData, sComData* inNewMsgData);
		
		static void					CreateThreadKey				( void );
		static void					DeleteThreadKey				( void *key );
		static void					AddCapability				( void );
		static CInternalDispatch	*GetThreadInternalDispatch	( void );

	private:
		int32_t			fInternalDispatchStackHeight;
		sComData	   *fInternalMsgDataList[kMaxInternalDispatchRecursion];
	
		static pthread_key_t	fThreadKey;
};

#endif
