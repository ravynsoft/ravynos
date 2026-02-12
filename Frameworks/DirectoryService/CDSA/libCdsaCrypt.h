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

#ifndef	_LIB_CDSA_CRYPT_H_
#define _LIB_CDSA_CRYPT_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <Security/cssm.h>

/*
 * Initialize CDSA and attach to the CSP.
 */
CSSM_RETURN cdsaCspAttach(
	CSSM_CSP_HANDLE		*cspHandle);
	
/*
 * Detach from CSP. To be called when app is finished with this 
 * library.
 */
CSSM_RETURN cdsaCspDetach(
	CSSM_CSP_HANDLE		cspHandle);
 
#pragma mark ------ Key generation ------

/*
 * Free resources allocated in cdsaDhGenerateKeyPair().
 */
CSSM_RETURN cdsaFreeKey(
	CSSM_CSP_HANDLE		cspHandle,		// from cdsaCspAttach()
	CSSM_KEY_PTR		key);			// from cdsaDeriveKey() 

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
	const CSSM_DATA	*inParams,			// optional 
	CSSM_DATA_PTR	outParams);			// optional, we malloc

/*
 * Perform Diffie-Hellman key exchange. 
 * Given "our" private key (in the form of a CSSM_KEY) and "their" public
 * key (in the form of a raw blob of bytes), cook up a symmetric key.
 */
CSSM_RETURN cdsaDhKeyExchange(
	CSSM_CSP_HANDLE	cspHandle,
	CSSM_KEY_PTR	myPrivateKey,		// from cdsaDhGenerateKeyPair
	const void		*theirPubKey,
	uint32			theirPubKeyLen,
	CSSM_KEY_PTR	derivedKey,			// RETURNED
	uint32			deriveKeySizeInBits,
	CSSM_ALGORITHMS	derivedKeyAlg);		// e.g., CSSM_ALGID_AES

#pragma mark ------ Simple encrypt/decrypt routines ------

/* 
 * These routines are used to perform simple "one-shot"
 * encryption and decryption oprtations. Use them when 
 * all of the data to be encrypted or decrypted is 
 * available at once.
 */
 
/*
 * Encrypt.
 * cipherText->Data is allocated by the CSP and must be freed (via
 * free()) by caller.
 */
CSSM_RETURN cdsaEncrypt(
	CSSM_CSP_HANDLE		cspHandle,		// from cdsaCspAttach()
	const CSSM_KEY		*key,			// from cdsaDeriveKey()
	const CSSM_DATA		*plainText,
	CSSM_DATA_PTR		cipherText);
	
/*
 * Decrypt.
 * plainText->Data is allocated by the CSP and must be freed (via
 * free()) by caller.
 */
CSSM_RETURN cdsaDecrypt(
	CSSM_CSP_HANDLE		cspHandle,		// from cdsaCspAttach()
	const CSSM_KEY		*key,			// from cdsaDeriveKey()
	const CSSM_DATA		*cipherText,
	CSSM_DATA_PTR		plainText);
	
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
	CSSM_DATA_PTR		digestData);


#ifdef	__cplusplus
}
#endif

#endif	/* _LIB_CDSA_CRYPT_H_ */
