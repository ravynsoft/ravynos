/*
 * chap_ms.c - Microsoft MS-CHAP compatible implementation.
 *
 * Copyright (c) 1995 Eric Rosenquist, Strata Software Limited.
 * http://www.strataware.com/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Eric Rosenquist.  The name of the author may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Modifications by Lauri Pesonen / lpesonen@clinet.fi, april 1997
 *
 *   Implemented LANManager type password response to MS-CHAP challenges.
 *   Now pppd provides both NT style and LANMan style blocks, and the
 *   prefered is set by option "ms-lanman". Default is to use NT.
 *   The hash text (StdText) was taken from Win95 RASAPI32.DLL.
 *
 *   You should also use DOMAIN\\USERNAME as described in README.MSCHAP80
 */

/*
 * Modifications by Frank Cusack, frank@google.com, March 2002.
 *
 *   Implemented MS-CHAPv2 functionality, heavily based on sample
 *   implementation in RFC 2759.  Implemented MPPE functionality,
 *   heavily based on sample implementation in RFC 3079.
 *   Copyright (c) 2002 Google, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

//#include "pppd.h"
#include "chap.h"
#include "chap_ms.h"
//#include "md4.h"
#ifdef __APPLE__
#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDigest.h>
#include "SMBAuth.h"
#define SHA1_SIGNATURE_SIZE CC_SHA1_DIGEST_LENGTH
#else
#include "sha1.h"
#endif

#define MAX(A,B)	((A)>(B)) ? (A) : (B)

static void	ascii2unicode __P((char[], int, u_char[]));
static void	DesEncrypt __P((const u_char *, const u_char *, u_char[8]));
static void	ChapMS_NT __P((u_char *, char *, int, u_char[24]));

#ifdef MSLANMAN
static void	ChapMS_LANMan __P((u_char *, char *, int, MS_ChapResponse *));
#endif

#ifdef USE_CRYPT
static void	Expand __P((u_char *, u_char *));
static void	Collapse __P((u_char *, u_char *));
#endif

#ifdef MPPE
static void	Set_Start_Key __P((u_char *, char *, int));
static void	SetMasterKeys __P((char *, int, u_char[24], int));
#endif

extern double drand48 __P((void));

#ifdef MSLANMAN
bool	ms_lanman = 0;    	/* Use LanMan password instead of NT */
			  	/* Has meaning only with MS-CHAP challenges */
#endif

#ifdef MPPE
u_char mppe_send_key[MPPE_MAX_KEY_LEN];
u_char mppe_recv_key[MPPE_MAX_KEY_LEN];
#endif

void
ChallengeResponse(const u_char *challenge,
		  const u_char PasswordHash[MD4_SIGNATURE_SIZE],
		  u_char response[24])
{
    unsigned char ZPasswordHash[21];
	
    bzero(ZPasswordHash, sizeof(ZPasswordHash));
    bcopy(PasswordHash, ZPasswordHash, MD4_SIGNATURE_SIZE);
	
#if 0
    dbglog("ChallengeResponse - ZPasswordHash %.*B",
	   sizeof(ZPasswordHash), ZPasswordHash);
#endif

    DesEncrypt(challenge, (unsigned char *)(ZPasswordHash +  0), &response[0]);
    DesEncrypt(challenge, (unsigned char *)(ZPasswordHash +  7), &response[8]);
    DesEncrypt(challenge, (unsigned char *)(ZPasswordHash + 14), &response[16]);

#if 0
    dbglog("ChallengeResponse - response %.24B", response);
#endif
}


static void
DesEncrypt(const u_char *clear, const u_char *key, u_char cipher[8])
{
	CCCryptorStatus status = kCCSuccess;
	unsigned char des_key[8] = {0};
	size_t dataMoved = 0;
	
	str_to_key((unsigned char *)key, des_key);
	status = CCCrypt( kCCEncrypt, kCCAlgorithmDES, 0,
						des_key, sizeof(des_key),
						NULL,
						clear, 8,
						cipher, 8, &dataMoved );
}


#ifdef USE_CRYPT

/* in == 8-byte string (expanded version of the 56-bit key)
 * out == 64-byte string where each byte is either 1 or 0
 * Note that the low-order "bit" is always ignored by by setkey()
 */
static void Expand(u_char *in, u_char *out)
{
        int j, c;
        int i;

        for(i = 0; i < 64; in++){
		c = *in;
                for(j = 7; j >= 0; j--)
                        *out++ = (c >> j) & 01;
                i += 8;
        }
}

/* The inverse of Expand
 */
static void Collapse(u_char *in, u_char *out)
{
        int j;
        int i;
	unsigned int c;

	for (i = 0; i < 64; i += 8, out++) {
	    c = 0;
	    for (j = 7; j >= 0; j--, in++)
		c |= *in << j;
	    *out = c & 0xff;
	}
}
#endif


void ChallengeHash(const unsigned char PeerChallenge[16], const unsigned char *rchallenge, const char *username, unsigned char Challenge[8])
{
    CC_SHA1_CTX	sha1Context;
    u_char	sha1Hash[SHA1_SIGNATURE_SIZE];
    const char	*user;

    /* remove domain from "domain\username" */
    if ((user = strrchr(username, '\\')) != NULL)
		++user;
    else
		user = username;
	
    CC_SHA1_Init(&sha1Context);
    CC_SHA1_Update(&sha1Context, PeerChallenge, 16);
    CC_SHA1_Update(&sha1Context, rchallenge, 16);
    CC_SHA1_Update(&sha1Context, user, strlen(user));
    CC_SHA1_Final(sha1Hash, &sha1Context);

    bcopy(sha1Hash, Challenge, 8);
}

/*
 * Convert the ASCII version of the password to Unicode.
 * This implicitly supports 8-bit ISO8859/1 characters.
 * This gives us the little-endian representation, which
 * is assumed by all M$ CHAP RFCs.  (Unicode byte ordering
 * is machine-dependent.)
 */
static void
ascii2unicode(char ascii[], int ascii_len, u_char unicode[])
{
    int i;

    bzero(unicode, ascii_len * 2);
    for (i = 0; i < ascii_len; i++)
	unicode[i * 2] = (u_char) ascii[i];
}

void
NTPasswordHash(char *secret, int secret_len, u_char hash[MD4_SIGNATURE_SIZE])
{
//#ifdef __NetBSD__
    /* NetBSD uses the libc md4 routines which take bytes instead of bits */
    int			mdlen = secret_len;
//#else
//    int			mdlen = secret_len * 8;
//#endif
    CC_MD4_CTX		md4Context;

	/*
    MD4Init(&md4Context);
    MD4Update(&md4Context, secret, mdlen);
    MD4Final(hash, &md4Context);
	*/
    CC_MD4_Init(&md4Context);
    CC_MD4_Update(&md4Context, secret, mdlen);
    CC_MD4_Final(hash, &md4Context);
}

static void
ChapMS_NT(u_char *rchallenge, char *secret, int secret_len,
	  u_char NTResponse[24])
{
    u_char	unicodePassword[MAX_NT_PASSWORD * 2];
    u_char	PasswordHash[MD4_SIGNATURE_SIZE];

    /* Hash the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash((char *)unicodePassword, secret_len * 2, PasswordHash);
	
    ChallengeResponse(rchallenge, PasswordHash, NTResponse);
}

#ifdef MPPE
/*
 * Set mppe_xxxx_key from the NTPasswordHashHash.
 * RFC 2548 (RADIUS support) requires us to export this function (ugh).
 */
void
mppe_set_keys(u_char *rchallenge, u_char PasswordHashHash[MD4_SIGNATURE_SIZE])
{
    CC_SHA1_CTX	sha1Context;
    u_char	Digest[SHA1_SIGNATURE_SIZE];	/* >= MPPE_MAX_KEY_LEN */

    SHA1_Init(&sha1Context);
    SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
    SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
    SHA1_Update(&sha1Context, rchallenge, 8);
    SHA1_Final(Digest, &sha1Context);

    /* Same key in both directions. */
    bcopy(Digest, mppe_send_key, sizeof(mppe_send_key));
    bcopy(Digest, mppe_recv_key, sizeof(mppe_recv_key));
}

/*
 * Set mppe_xxxx_key from MS-CHAP credentials. (see RFC 3079)
 */
static void
Set_Start_Key(u_char *rchallenge, char *secret, int secret_len)
{
    u_char	unicodePassword[MAX_NT_PASSWORD * 2];
    u_char	PasswordHash[MD4_SIGNATURE_SIZE];
    u_char	PasswordHashHash[MD4_SIGNATURE_SIZE];

    /* Hash (x2) the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);
    NTPasswordHash(PasswordHash, sizeof(PasswordHash), PasswordHashHash);

    mppe_set_keys(rchallenge, PasswordHashHash);
}

/*
 * Set mppe_xxxx_key from MS-CHAPv2 credentials. (see RFC 3079)
 */
static void
SetMasterKeys(char *secret, int secret_len, u_char NTResponse[24], int IsServer)
{
    CC_SHA1_CTX	sha1Context;
    u_char	unicodePassword[MAX_NT_PASSWORD * 2];
    u_char	PasswordHash[MD4_SIGNATURE_SIZE];
    u_char	PasswordHashHash[MD4_SIGNATURE_SIZE];
    u_char	MasterKey[SHA1_SIGNATURE_SIZE];	/* >= MPPE_MAX_KEY_LEN */
    u_char	Digest[SHA1_SIGNATURE_SIZE];	/* >= MPPE_MAX_KEY_LEN */

    u_char SHApad1[40] =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    u_char SHApad2[40] =
	{ 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	  0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	  0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	  0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2 };

    /* "This is the MPPE Master Key" */
    u_char Magic1[27] =
	{ 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74,
	  0x68, 0x65, 0x20, 0x4d, 0x50, 0x50, 0x45, 0x20, 0x4d,
	  0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x4b, 0x65, 0x79 };
    /* "On the client side, this is the send key; "
       "on the server side, it is the receive key." */
    u_char Magic2[84] =
	{ 0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
	  0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
	  0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20, 0x6b, 0x65, 0x79,
	  0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
	  0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73, 0x69, 0x64, 0x65,
	  0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
	  0x6b, 0x65, 0x79, 0x2e };
    /* "On the client side, this is the receive key; "
       "on the server side, it is the send key." */
    u_char Magic3[84] =
	{ 0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
	  0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
	  0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
	  0x6b, 0x65, 0x79, 0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73,
	  0x69, 0x64, 0x65, 0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
	  0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20,
	  0x6b, 0x65, 0x79, 0x2e };
    u_char *s;

    /* Hash (x2) the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);
    NTPasswordHash(PasswordHash, sizeof(PasswordHash), PasswordHashHash);

    SHA1_Init(&sha1Context);
    SHA1_Update(&sha1Context, PasswordHashHash, sizeof(PasswordHashHash));
    SHA1_Update(&sha1Context, NTResponse, 24);
    SHA1_Update(&sha1Context, Magic1, sizeof(Magic1));
    SHA1_Final(MasterKey, &sha1Context);

    /*
     * generate send key
     */
    if (IsServer)
	s = Magic3;
    else
	s = Magic2;
    SHA1_Init(&sha1Context);
    SHA1_Update(&sha1Context, MasterKey, 16);
    SHA1_Update(&sha1Context, SHApad1, sizeof(SHApad1));
    SHA1_Update(&sha1Context, s, 84);
    SHA1_Update(&sha1Context, SHApad2, sizeof(SHApad2));
    SHA1_Final(Digest, &sha1Context);

    bcopy(Digest, mppe_send_key, sizeof(mppe_send_key));

    /*
     * generate recv key
     */
    if (IsServer)
	s = Magic2;
    else
	s = Magic3;
    SHA1_Init(&sha1Context);
    SHA1_Update(&sha1Context, MasterKey, 16);
    SHA1_Update(&sha1Context, SHApad1, sizeof(SHApad1));
    SHA1_Update(&sha1Context, s, 84);
    SHA1_Update(&sha1Context, SHApad2, sizeof(SHApad2));
    SHA1_Final(Digest, &sha1Context);

    bcopy(Digest, mppe_recv_key, sizeof(mppe_recv_key));
}

#endif /* MPPE */


void
ChapMS(chap_state *cstate, u_char *rchallenge, char *secret, int secret_len,
       MS_ChapResponse *response)
{
#if 0
    CHAPDEBUG((LOG_INFO, "ChapMS: secret is '%.*s'", secret_len, secret));
#endif
    bzero(response, sizeof(*response));

    ChapMS_NT(rchallenge, secret, secret_len, response->NTResp);

#ifdef MSLANMAN
    ChapMS_LANMan(rchallenge, secret, secret_len, response);

    /* preferred method is set by option  */
    response->UseNT[0] = !ms_lanman;
#else
    response->UseNT[0] = 1;
#endif

    cstate->resp_length = MS_CHAP_RESPONSE_LEN;

#ifdef MPPE
    Set_Start_Key(rchallenge, secret, secret_len);
#endif
}


/*
 * If PeerChallenge is NULL, one is generated and response->PeerChallenge
 * is filled in.  Call this way when generating a response.
 * If PeerChallenge is supplied, it is copied into response->PeerChallenge.
 * Call this way when verifying a response (or debugging).
 * Do not call with PeerChallenge = response->PeerChallenge.
 *
 * response->PeerChallenge is then used for calculation of the
 * Authenticator Response.
 */
/*
void
ChapMS2(chap_state *cstate, u_char *rchallenge, u_char *PeerChallenge,
	char *user, char *secret, int secret_len, MS_Chap2Response *response,
	u_char authResponse[MS_AUTH_RESPONSE_LENGTH+1], int authenticator)
*/


void
GenerateAuthenticatorResponse(const u_char PasswordHash[MD4_SIGNATURE_SIZE],
			      u_char NTResponse[24], const u_char Challenge[8],
			      char *authResponse)
{
    /*
     * "Magic" constants used in response generation, from RFC 2759.
     */
    u_char Magic1[39] = /* "Magic server to client signing constant" */
	{ 0x4D, 0x61, 0x67, 0x69, 0x63, 0x20, 0x73, 0x65, 0x72, 0x76,
	  0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x6C, 0x69, 0x65,
	  0x6E, 0x74, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x69, 0x6E, 0x67,
	  0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74 };
    u_char Magic2[41] = /* "Pad to make it do more than one iteration" */
	{ 0x50, 0x61, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B,
	  0x65, 0x20, 0x69, 0x74, 0x20, 0x64, 0x6F, 0x20, 0x6D, 0x6F,
	  0x72, 0x65, 0x20, 0x74, 0x68, 0x61, 0x6E, 0x20, 0x6F, 0x6E,
	  0x65, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F,
	  0x6E };

    int i;
    CC_SHA1_CTX sha1Context;
    u_char	PasswordHashHash[MD4_SIGNATURE_SIZE];
    u_char	Digest[SHA1_SIGNATURE_SIZE];

    /* Hash (x2) the Unicode version of the secret (== password). */
    NTPasswordHash((char *)PasswordHash, MD4_SIGNATURE_SIZE, PasswordHashHash);
	
    CC_SHA1_Init(&sha1Context);
    CC_SHA1_Update(&sha1Context, PasswordHashHash, sizeof(PasswordHashHash));
    CC_SHA1_Update(&sha1Context, NTResponse, 24);
    CC_SHA1_Update(&sha1Context, Magic1, sizeof(Magic1));
    CC_SHA1_Final(Digest, &sha1Context);
	
    CC_SHA1_Init(&sha1Context);
    CC_SHA1_Update(&sha1Context, Digest, 20);
    CC_SHA1_Update(&sha1Context, Challenge, 8);
    CC_SHA1_Update(&sha1Context, Magic2, sizeof(Magic2));
    CC_SHA1_Final(Digest, &sha1Context);
	
    /* Convert to ASCII hex string. */
    for (i = 0; i < (MS_AUTH_RESPONSE_LENGTH / 2); i++)
		sprintf(authResponse + (i * 2), "%02X", Digest[i]);
}

