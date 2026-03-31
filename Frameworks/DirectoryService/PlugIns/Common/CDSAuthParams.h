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

#ifndef _CDSAuthParams_
#define _CDSAuthParams_	1

#include <openssl/evp.h>
#include <PasswordServer/AuthFile.h>
#include <DirectoryService/DirServices.h>
#include <DirectoryService/DirServicesUtils.h>
#include <DirectoryService/DirServicesConst.h>
#include <DirectoryServiceCore/CDSAuthDefs.h>
#include <DirectoryServiceCore/chap.h>
#include <DirectoryServiceCore/digestmd5.h>
#include <DirectoryServiceCore/pps.h>

class CDSAuthParams
{
	public:
										CDSAuthParams();
		virtual							~CDSAuthParams();
		
		virtual void					ZeroHashes( void );
		
		virtual tDirStatus				LoadParamsForAuthMethod(
											tDataNodePtr inAuthMethod,
											tDataBufferPtr inAuthData,
											tDataBufferPtr inAuthStepData );
		
		virtual void					PostEvent( tDirStatus inAuthResult );
		
		virtual CFMutableDictionaryRef	DictionaryFromAuthItems( void );
		virtual void					SetParamsFromDictionary( CFDictionaryRef inKeyedAuthItems );

		virtual tDirStatus				ExtractServiceInfo( tDataBufferPtr inAuthStepData );
		
		// members
		UInt32						uiAuthMethod;
		char						*mAuthMethodStr;
		char						*pUserName;
		char						*pNewPassword;
		char						*pOldPassword;
		unsigned char				*pNTLMDigest;
		UInt32						ntlmDigestLen;
		UInt32						ntlmHashType;
		unsigned char				*pCramResponse;
		UInt32						cramResponseLen;
		char						*pSambaName;
		char						*pDomain;
		char						*pAdminUser;
		char						*pAdminPassword;
		unsigned char				P21[kHashShadowKeyLength];
		unsigned char				C8[kHashShadowChallengeLength];
		unsigned char				C16[16];
		unsigned char				*PeerC16;
		unsigned char				P24[kHashShadowResponseLength];
		unsigned char				P24Input[kHashShadowResponseLength];
		unsigned char				GeneratedNTLM[EVP_MAX_MD_SIZE];
		char						MSCHAP2Response[MS_AUTH_RESPONSE_LENGTH+1];
		tDataListPtr				dataList;
		char						*path;
		unsigned char				hashes[kHashTotalLength];
		unsigned char				generatedHashes[kHashTotalLength];
		UInt32						hashLength;
		SInt32						hashesLengthFromFile;					
		tDataNodePtr				secureHashNode;
		unsigned char				secureHash[kHashSaltedSHA1Length];
		unsigned int				itemCount;	
		char						*nativeAttrType;
		char						*policyStr;	
		struct timespec				modDateOfPassword;						
		struct timeval				modDateAssist;
		PWGlobalAccessFeatures		globalAccess;
		PWGlobalMoreAccessFeatures	globalMoreAccess;		
		UInt32						policyStrLen;
		digest_context_t			digestContext;
		int							keySize;
		char						*challenge;
		char						*apopResponse;
		char						*aaData;
		char						*aaDataLocalCacheUser;
		CFDictionaryRef				serviceInfoDict;
		bool						mPostAuthEvent;
		
	protected:
		
	private:
};

#endif
