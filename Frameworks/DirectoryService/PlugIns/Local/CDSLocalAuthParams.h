/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef DISABLE_LOCAL_PLUGIN

#ifndef _CDSLocalAuthParams_
#define _CDSLocalAuthParams_	1

#include <openssl/evp.h>
#include <PasswordServer/AuthFile.h>
#include "DirServices.h"
#include "DirServicesUtils.h"
#include "DirServicesConst.h"
#include "CDSAuthDefs.h"
#include "CDSAuthParams.h"
#include "chap.h"
#include "digestmd5.h"

class CDSLocalAuthParams : public CDSAuthParams
{
	public:
										CDSLocalAuthParams();
		virtual							~CDSLocalAuthParams();
		
		virtual tDirStatus				LoadDSLocalParamsForAuthMethod(
											UInt32 inAuthMethod,
											UInt32 inUserLevelHashList,
											const char* inGUIDString,
											bool inAuthedUserIsAdmin,
											tDataBufferPtr inAuthData,
											tDataBufferPtr inAuthStepData );
		
		virtual bool					PolicyStateChanged( void );
		
		// members
		sHashState					state;
		sHashState					initialState;
		sHashState					targetUserState;
		char						*stateFilePath;
		char						*targetUserStateFilePath;
		bool						bFetchHashFiles;
		
	protected:
		
	private:
};

#endif

#endif // DISABLE_LOCAL_PLUGIN
