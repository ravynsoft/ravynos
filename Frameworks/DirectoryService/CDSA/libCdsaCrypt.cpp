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
 * @header libCdsaCrypt
 * Simple high-level CDSA access routines.
 */

#include "libCdsaCrypt.h"
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#pragma mark --- static session data and private functions ----

static CSSM_VERSION vers = {2, 0};
static const CSSM_GUID testGuid = { 0xFADE, 0, 0, { 1,2,3,4,5,6,7,0 }};

/*
 * Standard app-level memory functions required by CDSA.
 */
void * appMalloc (CSSM_SIZE size, void *allocRef) {
	return( malloc(size) );
}
void appFree (void *mem_ptr, void *allocRef) {
	free(mem_ptr);
 	return;
}
void * appRealloc (void *ptr, CSSM_SIZE size, void *allocRef) {
	return( realloc( ptr, size ) );
}
void * appCalloc (uint32 num, CSSM_SIZE size, void *allocRef) {
	return( calloc( num, size ) );
}
static CSSM_API_MEMORY_FUNCS memFuncs = {
	appMalloc,
	appFree,
	appRealloc,
 	appCalloc,
 	NULL
 };

/*
 * Init CSSM; returns CSSM_FALSE on error. Reusable.
 */
static CSSM_BOOL cssmInitd = CSSM_FALSE;
CSSM_RETURN cssmStartup()
{
	if(cssmInitd) {
		return CSSM_OK;
	}  

	CSSM_RETURN  crtn;
    CSSM_PVC_MODE pvcPolicy = CSSM_PVC_NONE;
	
	crtn = CSSM_Init (&vers, 
		CSSM_PRIVILEGE_SCOPE_NONE,
		&testGuid,
		CSSM_KEY_HIERARCHY_NONE,
		&pvcPolicy,
		NULL /* reserved */);
	if(crtn != CSSM_OK) {
		return crtn;
	}
	else {
		cssmInitd = CSSM_TRUE;
		return CSSM_OK;
	}
}

/*
 * Cook up a symmetric encryption context for the specified key,
 * inferring all needed attributes solely from the key algorithm.
 * This is obviously not a one-size-fits all function, but rather
 * the "most common case". If you need to encrypt/decrypt with other
 * padding, mode, etc., do it yourself.
 */
static CSSM_RETURN genCryptHandle(
	CSSM_CSP_HANDLE cspHandle,
	const CSSM_KEY	*key,
	const CSSM_DATA	*ivPtr,
	CSSM_CC_HANDLE	*ccHandle)
{
	CSSM_ALGORITHMS		keyAlg = key->KeyHeader.AlgorithmId;
	CSSM_ALGORITHMS		encrAlg;
	CSSM_ENCRYPT_MODE	encrMode = CSSM_ALGMODE_NONE;
	CSSM_PADDING 		encrPad = CSSM_PADDING_NONE;
	CSSM_RETURN			crtn;
	CSSM_CC_HANDLE		ccHand = 0;
	CSSM_ACCESS_CREDENTIALS	creds;
	CSSM_BOOL			isSymmetric = CSSM_TRUE;
	
	/* 
	 * Infer algorithm - ususally it's the same as in the key itself
	 */
	switch(keyAlg) {
		case CSSM_ALGID_3DES_3KEY:
			encrAlg = CSSM_ALGID_3DES_3KEY_EDE;
			break;
		default:
			encrAlg = keyAlg;
			break;
	}
	
	/* infer mode and padding */
	switch(encrAlg) {
		/* 8-byte block ciphers */
		case CSSM_ALGID_DES:
		case CSSM_ALGID_3DES_3KEY_EDE:
		case CSSM_ALGID_RC5:
		case CSSM_ALGID_RC2:
			encrMode = CSSM_ALGMODE_CBCPadIV8;
			encrPad = CSSM_PADDING_PKCS5;
			break;
		
		/* 16-byte block ciphers */
		case CSSM_ALGID_AES:
			encrMode = CSSM_ALGMODE_CBCPadIV8;
			encrPad = CSSM_PADDING_PKCS7;
			break;
			
		/* stream ciphers */
		case CSSM_ALGID_ASC:
		case CSSM_ALGID_RC4:
			encrMode = CSSM_ALGMODE_NONE;
			encrPad = CSSM_PADDING_NONE;
			break;
			
		/* RSA asymmetric */
		case CSSM_ALGID_RSA:
			/* encrMode not used */
			encrPad = CSSM_PADDING_PKCS1;
			isSymmetric = CSSM_FALSE;
			break;
		default:
			/* don't wing it - abort */
			return CSSMERR_CSP_INTERNAL_ERROR;
	}
	
	memset(&creds, 0, sizeof(CSSM_ACCESS_CREDENTIALS));
	if(isSymmetric) {
		crtn = CSSM_CSP_CreateSymmetricContext(cspHandle,
			encrAlg,
			encrMode,
			NULL,			// access cred
			key,
			ivPtr,			// InitVector
			encrPad,
			NULL,			// Params
			&ccHand);
	}
	else {
		crtn = CSSM_CSP_CreateAsymmetricContext(cspHandle,
			encrAlg,
			&creds,			// access
			key,
			encrPad,
			&ccHand);
	
	}
	if(crtn) {
		return crtn;
	}
	*ccHandle = ccHand;
	return CSSM_OK;
}

#pragma mark --- start of public Functions ---

/*
 * Initialize CDSA and attach to the CSP.
 */
CSSM_RETURN cdsaCspAttach(
	CSSM_CSP_HANDLE		*cspHandle)
{
	CSSM_CSP_HANDLE cspHand;
	CSSM_RETURN		crtn;
	
	/* initialize CDSA (this is reusable) */
	crtn = cssmStartup();
	if(crtn) {
		return crtn;
	}
	
	/* Load the CSP bundle into this app's memory space */
	crtn = CSSM_ModuleLoad(&gGuidAppleCSP,
		CSSM_KEY_HIERARCHY_NONE,
		NULL,			// eventHandler
		NULL);			// AppNotifyCallbackCtx
	if(crtn) {
		return crtn;
	}
	
	/* obtain a handle which will be used to refer to the CSP */ 
	crtn = CSSM_ModuleAttach (&gGuidAppleCSP,
		&vers,
		&memFuncs,			// memFuncs
		0,					// SubserviceID
		CSSM_SERVICE_CSP,	
		0,					// AttachFlags
		CSSM_KEY_HIERARCHY_NONE,
		NULL,				// FunctionTable
		0,					// NumFuncTable
		NULL,				// reserved
		&cspHand);
	if(crtn) {
		return crtn;
	}
	*cspHandle = cspHand;
	return CSSM_OK;
}
	
/*
 * Detach from CSP. To be called when app is finished with this 
 * library.
 */
CSSM_RETURN cdsaCspDetach(
	CSSM_CSP_HANDLE		cspHandle)
{
	return CSSM_ModuleDetach(cspHandle);
}

/*
 * Free resources allocated in cdsaDeriveKey().
 */
CSSM_RETURN cdsaFreeKey(
	CSSM_CSP_HANDLE		cspHandle,
	CSSM_KEY_PTR		key)
{
	return CSSM_FreeKey(cspHandle, 
		NULL,			// access cred
		key,	
		CSSM_FALSE);	// don't delete since it wasn't permanent
}

#pragma mark ------ Diffie-Hellman key generation and derivation ------

/*
 * Generate a Diffie-Hellman key pair. Algorithm parameters are
 * either specified by caller via inParams, or are generated here
 * and returned to caller in outParams. Exactly one of (inParams,
 * outParams) must be non-NULL.
 */
CSSM_RETURN cdsaDhGenerateKeyPair(
	CSSM_CSP_HANDLE	cspHandle,
	CSSM_KEY_PTR	publicKey,
	CSSM_KEY_PTR	privateKey,
	uint32			keySizeInBits,
	const CSSM_DATA	*inParams,		// optional 
	CSSM_DATA_PTR	outParams)		// optional, we malloc
{
	CSSM_RETURN		crtn;
	CSSM_CC_HANDLE 	ccHandle;
	CSSM_DATA		labelData = {8, (uint8 *)"tempKey"};
	
	/* Caller must specify either inParams or outParams, not both */
	if(inParams && outParams) {
		return CSSMERR_CSSM_INVALID_POINTER;
	}
	if(!inParams && !outParams) {
		return CSSMERR_CSSM_INVALID_POINTER;
	}
	memset(publicKey, 0, sizeof(CSSM_KEY));
	memset(privateKey, 0, sizeof(CSSM_KEY));
	
	crtn = CSSM_CSP_CreateKeyGenContext(cspHandle,
		CSSM_ALGID_DH,
		keySizeInBits,
		NULL,					// Seed
		NULL,					// Salt
		NULL,					// StartDate
		NULL,					// EndDate
		inParams,				// Params, may be NULL
		&ccHandle);
	if(crtn) {
		return crtn;
	}
	
	if(outParams) {
		/* explicitly generate params and return them to caller */
		outParams->Data = NULL;
		outParams->Length = 0;
		crtn = CSSM_GenerateAlgorithmParams(ccHandle, 
			keySizeInBits, outParams);
		if(crtn) {
			CSSM_DeleteContext(ccHandle);
			return crtn;
		}
	}
	
	crtn = CSSM_GenerateKeyPair(ccHandle,
		CSSM_KEYUSE_DERIVE,		// only legal use of a Diffie-Hellman key 
		CSSM_KEYATTR_RETURN_DATA | CSSM_KEYATTR_EXTRACTABLE,
		&labelData,
		publicKey,
		/* private key specification */
		CSSM_KEYUSE_DERIVE,
		CSSM_KEYATTR_RETURN_REF,
		&labelData,				// same labels
		NULL,					// CredAndAclEntry
		privateKey);
	CSSM_DeleteContext(ccHandle);
	return crtn;
}

/*
 * Perform Diffie-Hellman key exchange. 
 * Given "our" private key (in the form of a CSSM_KEY) and "their" public
 * key (in the form of a raw blob of bytes), cook up a symmetric key.
 */
CSSM_RETURN cdsaDhKeyExchange(
	CSSM_CSP_HANDLE	cspHandle,
	CSSM_KEY_PTR	myPrivateKey,			// from cdsaDhGenerateKeyPair
	const void		*theirPubKey,
	uint32			theirPubKeyLen,
	CSSM_KEY_PTR	derivedKey,				// RETURNED
	uint32			deriveKeySizeInBits,
	CSSM_ALGORITHMS	derivedKeyAlg)			// e.g., CSSM_ALGID_AES
{
	CSSM_RETURN 			crtn;
	CSSM_ACCESS_CREDENTIALS	creds;
	CSSM_CC_HANDLE			ccHandle;
	CSSM_DATA				labelData = {8, (uint8 *)"tempKey"};
	
	memset(&creds, 0, sizeof(CSSM_ACCESS_CREDENTIALS));
	memset(derivedKey, 0, sizeof(CSSM_KEY));
	
	crtn = CSSM_CSP_CreateDeriveKeyContext(cspHandle,
		CSSM_ALGID_DH,
		derivedKeyAlg,
		deriveKeySizeInBits,
		&creds,
		myPrivateKey,	// BaseKey
		0,				// IterationCount
		0,				// Salt
		0,				// Seed
		&ccHandle);
	if(crtn) {
		return crtn;
	}
	
	/* public key passed in as CSSM_DATA *Param */
	CSSM_DATA theirPubKeyData = { theirPubKeyLen, (uint8 *)theirPubKey };
	
	crtn = CSSM_DeriveKey(ccHandle,
		&theirPubKeyData,
		CSSM_KEYUSE_ANY, 
		CSSM_KEYATTR_RETURN_DATA | CSSM_KEYATTR_EXTRACTABLE,
		&labelData,
		NULL,				// cread/acl
		derivedKey);
	CSSM_DeleteContext(ccHandle);
	return crtn;
}

#pragma mark ------ Simple encrypt/decrypt routines ------

/* 
 * Common initialization vector shared by encrypt and decrypt.
 * Some applications may wish to specify a different IV for
 * each encryption op (e.g., disk block number, IP packet number,
 * etc.) but that is outside the scope of this library.
 */
static uint8 iv[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static const CSSM_DATA ivCommon = {16, iv};

/*
 * Encrypt.
 * cipherText->Data is allocated by the CSP and must be freed (via
 * free()) by caller.
 */
CSSM_RETURN cdsaEncrypt(
	CSSM_CSP_HANDLE		cspHandle,
	const CSSM_KEY		*key,
	const CSSM_DATA		*plainText,
	CSSM_DATA_PTR		cipherText)
{
	CSSM_RETURN 	crtn;
	CSSM_CC_HANDLE	ccHandle;
	CSSM_DATA		remData = {0, NULL};
	CSSM_SIZE		bytesEncrypted;
	
	crtn = genCryptHandle(cspHandle, key, &ivCommon, &ccHandle);
	if(crtn) {
		return crtn;
	}
	cipherText->Length = 0;
	cipherText->Data = NULL;
	crtn = CSSM_EncryptData(ccHandle,
		plainText,
		1,
		cipherText,
		1,
		&bytesEncrypted,
		&remData);
	CSSM_DeleteContext(ccHandle);
	if(crtn) {
		return crtn;
	}
	
	cipherText->Length = bytesEncrypted;
	if(remData.Length != 0) {
		/* append remaining data to cipherText */
		uint32 newLen = cipherText->Length + remData.Length;
		cipherText->Data = (uint8 *)appRealloc(cipherText->Data,
			newLen,
			NULL);
		memmove(cipherText->Data + cipherText->Length, 
			remData.Data, remData.Length);
		cipherText->Length = newLen;
		appFree(remData.Data, NULL);
	}
	return CSSM_OK;
}

/*
 * Decrypt.
 * plainText->Data is allocated by the CSP and must be freed (via
 * free()) by caller.
 */
CSSM_RETURN cdsaDecrypt(
	CSSM_CSP_HANDLE		cspHandle,
	const CSSM_KEY		*key,
	const CSSM_DATA		*cipherText,
	CSSM_DATA_PTR		plainText)
{
	CSSM_RETURN 	crtn;
	CSSM_CC_HANDLE	ccHandle;
	CSSM_DATA		remData = {0, NULL};
	CSSM_SIZE		bytesDecrypted;
	
	crtn = genCryptHandle(cspHandle, key, &ivCommon, &ccHandle);
	if(crtn) {
		return crtn;
	}
	plainText->Length = 0;
	plainText->Data = NULL;
	crtn = CSSM_DecryptData(ccHandle,
		cipherText,
		1,
		plainText,
		1,
		&bytesDecrypted,
		&remData);
	CSSM_DeleteContext(ccHandle);
	if(crtn) {
		return crtn;
	}
	
	plainText->Length = bytesDecrypted;
	if(remData.Length != 0) {
		/* append remaining data to plainText */
		uint32 newLen = plainText->Length + remData.Length;
		plainText->Data = (uint8 *)appRealloc(plainText->Data,
			newLen,
			NULL);
		memmove(plainText->Data + plainText->Length, 
			remData.Data, remData.Length);
		plainText->Length = newLen;
		appFree(remData.Data, NULL);
	}
	return CSSM_OK;
}

#pragma mark ------ Digest routines ------

/*
 * The simple one-shot digest routine, when all of the data to 
 * be processed is available at once.
 * digest->Data is allocated by the CSP and must be freed (via
 * free()) by caller.
 */
CSSM_RETURN cdsaDigest(
	CSSM_CSP_HANDLE		cspHandle,		// from cdsaCspAttach()
	CSSM_ALGORITHMS		digestAlg,		// e.g., CSSM_ALGID_SHA1
	const CSSM_DATA		*inData,
	CSSM_DATA_PTR		digestData)
{
	CSSM_RETURN 	crtn;
	CSSM_CC_HANDLE	ccHandle;
	
	digestData->Data = NULL;
	digestData->Length = 0;
	
	crtn = CSSM_CSP_CreateDigestContext(cspHandle, digestAlg, &ccHandle);
	if(crtn) {
		return crtn;
	}
	crtn = CSSM_DigestData(ccHandle, inData, 1, digestData);
	CSSM_DeleteContext(ccHandle);
	return crtn;
}
	

