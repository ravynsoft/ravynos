/* DIGEST-MD5 SASL plugin
 * Ken Murchison
 * Rob Siemborski
 * Tim Martin
 * Alexey Melnikov 
 * $Id: digestmd5.c,v 1.6 2006/02/28 23:28:44 snsimon Exp $
 */
/* 
 * Copyright (c) 1998-2003 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any other legal
 *    details, please contact  
 *      Office of Technology Transfer
 *      Carnegie Mellon University
 *      5000 Forbes Avenue
 *      Pittsburgh, PA  15213-3890
 *      (412) 268-4387, fax: (412) 268-7395
 *      tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University ( http://www.cmu.edu/computing/ )."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <syslog.h>
#include "digestmd5.h"
#include <CommonCrypto/CommonDigest.h>

#define bool int
static const unsigned char *COLON = (const unsigned char *)":";
#if 0
static const char *SEALING_CLIENT_SERVER="Digest H(A1) to client-to-server sealing key magic constant";
static const char *SEALING_SERVER_CLIENT="Digest H(A1) to server-to-client sealing key magic constant";

static const char *SIGNING_CLIENT_SERVER="Digest session key to client-to-server signing key magic constant";
static const char *SIGNING_SERVER_CLIENT="Digest session key to server-to-client signing key magic constant";
#endif

#define kDumpablePrefix		"Digest "
#define kUserIDTag			",userid="


/*****************************  Common Section  *****************************/

void CvtHex(const HASH Bin, HASHHEX Hex)
{
    unsigned short  i;
    unsigned char   j;
    
    for (i = 0; i < HASHLEN; i++) {
	j = (Bin[i] >> 4) & 0xf;
	if (j <= 9)
	    Hex[i * 2] = (j + '0');
	else
	    Hex[i * 2] = (j + 'a' - 10);
	j = Bin[i] & 0xf;
	if (j <= 9)
	    Hex[i * 2 + 1] = (j + '0');
	else
	    Hex[i * 2 + 1] = (j + 'a' - 10);
    }
    Hex[HASHHEXLEN] = '\0';
}

/*
 * calculate request-digest/response-digest as per HTTP Digest spec
 */

void
DigestCalcOldResponse(
		   HASHHEX HA1,	/* H(A1) */
		   unsigned char *pszNonce,	/* nonce from server */
		   unsigned char *pszDigestUri,	/* requested URL */
		   const unsigned char *pszMethod,
		   HASHHEX HEntity,	/* H(entity body) if qop="auth-int" */
		   HASHHEX Response	/* request-digest or response-digest */
    )
{
    CC_MD5_CTX		Md5Ctx;
    HASH            HA2;
    HASH            RespHash;
    HASHHEX         HA2Hex;
    
    /* calculate H(A2) */
    CC_MD5_Init(&Md5Ctx);
	if ( pszMethod && pszMethod[0] == 0 )
		pszMethod = NULL;
	
    if (pszMethod != NULL) {
		CC_MD5_Update(&Md5Ctx, pszMethod, strlen((char *) pszMethod));
    }
    CC_MD5_Update(&Md5Ctx, (unsigned char *) COLON, 1);
    
    /* CC_MD5_Update(&Md5Ctx, (unsigned char *) "AUTHENTICATE:", 13); */
    CC_MD5_Update(&Md5Ctx, pszDigestUri, strlen((char *) pszDigestUri));
    CC_MD5_Final(HA2, &Md5Ctx);
    CvtHex(HA2, HA2Hex);
    
    /* calculate response */
    CC_MD5_Init(&Md5Ctx);
    CC_MD5_Update(&Md5Ctx, HA1, HASHHEXLEN);
    CC_MD5_Update(&Md5Ctx, COLON, 1);
    CC_MD5_Update(&Md5Ctx, pszNonce, strlen((char *) pszNonce));
    CC_MD5_Update(&Md5Ctx, COLON, 1);
	CC_MD5_Update(&Md5Ctx, HA2Hex, HASHHEXLEN);
    CC_MD5_Final(RespHash, &Md5Ctx);
    CvtHex(RespHash, Response);
}


void
DigestCalcResponse(
		   HASHHEX HA1,	/* H(A1) */
		   unsigned char *pszNonce,	/* nonce from server */
		   unsigned int pszNonceCount,	/* 8 hex digits */
		   unsigned char *pszCNonce,	/* client nonce */
		   unsigned char *pszQop,	/* qop-value: "", "auth", "auth-int" */
		   unsigned char *pszDigestUri,	/* requested URL */
		   const char *pszMethod,
		   HASHHEX HEntity,	/* H(entity body) if qop="auth-int" */
		   HASHHEX Response	/* request-digest or response-digest */
    )
{
    CC_MD5_CTX		Md5Ctx;
    HASH            HA2;
    HASH            RespHash;
    HASHHEX         HA2Hex;
    char ncvalue[10];
    
	if ( pszMethod && pszMethod[0] == 0 )
		pszMethod = strdup("AUTHENTICATE");

    /* calculate H(A2) */
    CC_MD5_Init(&Md5Ctx);
    
    if (pszMethod != NULL) {
	CC_MD5_Update(&Md5Ctx, pszMethod, strlen((char *) pszMethod));
    }
    CC_MD5_Update(&Md5Ctx, (unsigned char *) COLON, 1);
    
    /* CC_MD5_Update(&Md5Ctx, (unsigned char *) "AUTHENTICATE:", 13); */
    CC_MD5_Update(&Md5Ctx, pszDigestUri, strlen((char *) pszDigestUri));
    if (strcasecmp((char *) pszQop, "auth") != 0) {
	/* append ":00000000000000000000000000000000" */
	CC_MD5_Update(&Md5Ctx, COLON, 1);
	CC_MD5_Update(&Md5Ctx, HEntity, HASHHEXLEN);
    }
    CC_MD5_Final(HA2, &Md5Ctx);
    CvtHex(HA2, HA2Hex);
    
    /* calculate response */
    CC_MD5_Init(&Md5Ctx);
    CC_MD5_Update(&Md5Ctx, HA1, HASHHEXLEN);
    CC_MD5_Update(&Md5Ctx, COLON, 1);
    CC_MD5_Update(&Md5Ctx, pszNonce, strlen((char *) pszNonce));
    CC_MD5_Update(&Md5Ctx, COLON, 1);
    if (*pszQop) {
	sprintf(ncvalue, "%08x", pszNonceCount);
	CC_MD5_Update(&Md5Ctx, ncvalue, strlen(ncvalue));
	CC_MD5_Update(&Md5Ctx, COLON, 1);
	CC_MD5_Update(&Md5Ctx, pszCNonce, strlen((char *) pszCNonce));
	CC_MD5_Update(&Md5Ctx, COLON, 1);
	CC_MD5_Update(&Md5Ctx, pszQop, strlen((char *) pszQop));
	CC_MD5_Update(&Md5Ctx, COLON, 1);
    }
    CC_MD5_Update(&Md5Ctx, HA2Hex, HASHHEXLEN);
    CC_MD5_Final(RespHash, &Md5Ctx);
    CvtHex(RespHash, Response);
}

static bool UTF8_In_8859_1(const unsigned char *base, int len)
{
    const unsigned char *scan, *end;
    
    end = base + len;
    for (scan = base; scan < end; ++scan) {
	if (*scan > 0xC3)
	    break;			/* abort if outside 8859-1 */
	if (*scan >= 0xC0 && *scan <= 0xC3) {
	    if (++scan == end || *scan < 0x80 || *scan > 0xBF)
		break;
	}
    }
    
    /* if scan >= end, then this is a 8859-1 string. */
    return (scan >= end);
}

/*
 * if the string is entirely in the 8859-1 subset of UTF-8, then translate to
 * 8859-1 prior to MD5
 */
void MD5_UTF8_8859_1(
		     CC_MD5_CTX * ctx,
		     bool In_ISO_8859_1,
		     const unsigned char *base,
		     int len)
{
    const unsigned char *scan, *end;
    unsigned char   cbuf;
    
    end = base + len;
    
    /* if we found a character outside 8859-1, don't alter string */
    if (!In_ISO_8859_1) {
	CC_MD5_Update(ctx, base, len);
	return;
    }
    /* convert to 8859-1 prior to applying hash */
    do {
	for (scan = base; scan < end && *scan < 0xC0; ++scan);
	if (scan != base)
	    CC_MD5_Update(ctx, base, scan - base);
	if (scan + 1 >= end)
	    break;
	cbuf = ((scan[0] & 0x3) << 6) | (scan[1] & 0x3f);
	CC_MD5_Update(ctx, &cbuf, 1);
	base = scan + 2;
    }
    while (base < end);
}


static void DigestCalcSecret(
			     unsigned char *pszUserName,
			     unsigned char *pszRealm,
			     unsigned char *Password,
			     int PasswordLen,
			     HASH HA1)
{
    bool            In_8859_1;
    CC_MD5_CTX         Md5Ctx;
    
    /* Chris Newman clarified that the following text in DIGEST-MD5 spec
       is bogus: "if name and password are both in ISO 8859-1 charset"
       We shoud use code example instead */
    
    CC_MD5_Init(&Md5Ctx);
    
    /* We have to convert UTF-8 to ISO-8859-1 if possible */
    In_8859_1 = UTF8_In_8859_1(pszUserName, strlen((char *) pszUserName));
    MD5_UTF8_8859_1(&Md5Ctx, In_8859_1,
		    pszUserName, strlen((char *) pszUserName));
    
    CC_MD5_Update(&Md5Ctx, COLON, 1);
    
    if (pszRealm != NULL && pszRealm[0] != '\0') {
		/* a NULL realm is equivalent to the empty string */
		CC_MD5_Update(&Md5Ctx, pszRealm, strlen((char *) pszRealm));
    }      
    
    CC_MD5_Update(&Md5Ctx, COLON, 1);
    
    /* We have to convert UTF-8 to ISO-8859-1 if possible */
    In_8859_1 = UTF8_In_8859_1(Password, PasswordLen);
    MD5_UTF8_8859_1( &Md5Ctx, In_8859_1, Password, PasswordLen);
    
    CC_MD5_Final(HA1, &Md5Ctx);
}


static char *skip_lws (char *s)
{
    if(!s) return NULL;
    
    /* skipping spaces: */
    while (s[0] == ' ' || s[0] == HT || s[0] == CR || s[0] == LF) {
	if (s[0]=='\0') break;
	s++;
    }  
    
    return s;
}

static char *skip_token (char *s, int caseinsensitive)
{
    if(!s) return NULL;
    
    while (s[0]>SP) {
	if (s[0]==DEL || s[0]=='(' || s[0]==')' || s[0]=='<' || s[0]=='>' ||
	    s[0]=='@' || s[0]==',' || s[0]==';' || s[0]==':' || s[0]=='\\' ||
	    s[0]=='\'' || s[0]=='/' || s[0]=='[' || s[0]==']' || s[0]== '?' ||
	    s[0]=='=' || s[0]== '{' || s[0]== '}') {
	    if (caseinsensitive == 1) {
		if (!isupper((unsigned char) s[0]))
		    break;
	    } else {
		break;
	    }
	}
	s++;
    }  
    return s;
}

/* NULL - error (unbalanced quotes), 
   otherwise pointer to the first character after value */
static char *unquote (char *qstr)
{
    char *endvalue;
    int   escaped = 0;
    char *outptr;
    
    if(!qstr) return NULL;
    
    if (qstr[0] == '"') {
	qstr++;
	outptr = qstr;
	
	for (endvalue = qstr; endvalue[0] != '\0'; endvalue++, outptr++) {
	    if (escaped) {
		outptr[0] = endvalue[0];
		escaped = 0;
	    }
	    else if (endvalue[0] == '\\') {
		escaped = 1;
		outptr--; /* Will be incremented at the end of the loop */
	    }
	    else if (endvalue[0] == '"') {
		break;
	    }      
	    else {
		outptr[0] = endvalue[0];      
	    }
	}
	
	if (endvalue[0] != '"') {
	    return NULL;
	}
	
	while (outptr <= endvalue) {
	    outptr[0] = '\0';
	    outptr++;
	}
	endvalue++;
    }
    else { /* not qouted value (token) */
	endvalue = skip_token(qstr,0);
    };
    
    return endvalue;  
} 

static void get_pair(char **in, char **name, char **value)
{
    char  *endpair;
    /* int    inQuotes; */
    char  *curp = *in;
    *name = NULL;
    *value = NULL;
    
    if (curp == NULL) return;
    if (curp[0] == '\0') return;
    
    /* skipping spaces: */
    curp = skip_lws(curp);
    
    *name = curp;
    
    curp = skip_token(curp,1);
    
    /* strip wierd chars */
    if (curp[0] != '=' && curp[0] != '\0') {
	*curp++ = '\0';
    };
    
    curp = skip_lws(curp);
    
    if (curp[0] != '=') { /* No '=' sign */ 
	*name = NULL;
	return;
    }
    
    curp[0] = '\0';
    curp++;
    
    curp = skip_lws(curp);  
    
    *value = (curp[0] == '"') ? curp+1 : curp;
    
    endpair = unquote (curp);
    if (endpair == NULL) { /* Unbalanced quotes */ 
	*name = NULL;
	return;
    }
    if (endpair[0] != ',') {
	if (endpair[0]!='\0') {
	    *endpair++ = '\0'; 
	}
    }
    
    endpair = skip_lws(endpair);
    
    /* syntax check: MUST be '\0' or ',' */  
    if (endpair[0] == ',') {
	endpair[0] = '\0';
	endpair++; /* skipping <,> */
    } else if (endpair[0] != '\0') { 
	*name = NULL;
	return;
    }
    
    *in = endpair;
}

#ifdef WITH_DES
/******************************
 *
 * 3DES functions
 *
 *****************************/


static int dec_3des(void *v,
		    const char *input,
		    unsigned inputlen,
		    unsigned char digest[16],
		    char *output,
		    unsigned *outputlen)
{
    digest_context_t *text = (digest_context_t *) v;
    int padding, p;
    
    des_ede2_cbc_encrypt((void *) input,
			 (void *) output,
			 inputlen,
			 text->keysched_dec,
			 text->keysched_dec2,
			 &text->ivec_dec,
			 DES_DECRYPT);
    
    /* now chop off the padding */
    padding = output[inputlen - 11];
    if (padding < 1 || padding > 8) {
	/* invalid padding length */
	return -1;
    }
    /* verify all padding is correct */
    for (p = 1; p <= padding; p++) {
	if (output[inputlen - 10 - p] != padding) {
	    return -1;
	}
    }
    
    /* chop off the padding */
    *outputlen = inputlen - padding - 10;
    
    /* copy in the HMAC to digest */
    memcpy(digest, output + inputlen - 10, 10);
    
    return 0;
}

static int enc_3des(void *v,
		    const char *input,
		    unsigned inputlen,
		    unsigned char digest[16],
		    char *output,
		    unsigned *outputlen)
{
    digest_context_t *text = (digest_context_t *) v;
    int len;
    int paddinglen;
    
    /* determine padding length */
    paddinglen = 8 - ((inputlen + 10) % 8);
    
    /* now construct the full stuff to be ciphered */
    memcpy(output, input, inputlen);                /* text */
    memset(output+inputlen, paddinglen, paddinglen);/* pad  */
    memcpy(output+inputlen+paddinglen, digest, 10); /* hmac */
    
    len=inputlen+paddinglen+10;
    
    des_ede2_cbc_encrypt((void *) output,
			 (void *) output,
			 len,
			 text->keysched_enc,
			 text->keysched_enc2,
			 &text->ivec_enc,
			 DES_ENCRYPT);
    
    *outputlen=len;
    
    return 0;
}

static int init_3des(void *v, 
		     char enckey[16],
		     char deckey[16])
    
    
    
{
    digest_context_t *text = (digest_context_t *) v;
    
    if(des_key_sched((des_cblock *) enckey, text->keysched_enc) < 0)
	return -1;
    if(des_key_sched((des_cblock *) deckey, text->keysched_dec) < 0)
	return -1;
    
    if(des_key_sched((des_cblock *) (enckey+7), text->keysched_enc2) < 0)
	return -1;
    if(des_key_sched((des_cblock *) (deckey+7), text->keysched_dec2) < 0)
	return -1;
    
    memcpy(text->ivec_enc, ((char *) enckey) + 8, 8);
    memcpy(text->ivec_dec, ((char *) deckey) + 8, 8);
    
    return 0;
}


/******************************
 *
 * DES functions
 *
 *****************************/

static int dec_des(void *v, 
		   const char *input,
		   unsigned inputlen,
		   unsigned char digest[16],
		   char *output,
		   unsigned *outputlen)
{
    digest_context_t *text = (digest_context_t *) v;
    int p,padding = 0;

    des_cbc_encrypt((void *) input,
		    (void *) output,
		    inputlen,
		    text->keysched_dec,
		    &text->ivec_dec,
		    DES_DECRYPT);

    /* Update the ivec (des_cbc_encrypt implementations tend to be broken in
       this way) */
    memcpy(text->ivec_dec, input + (inputlen - 8), 8);
    
    /* now chop off the padding */
    padding = output[inputlen - 11];
    if (padding < 1 || padding > 8) {
	/* invalid padding length */
	return -1;
    }
    /* verify all padding is correct */
    for (p = 1; p <= padding; p++) {
	if (output[inputlen - 10 - p] != padding) {
	    return -1;
	}
    }
    
    /* chop off the padding */
    *outputlen = inputlen - padding - 10;
    
    /* copy in the HMAC to digest */
    memcpy(digest, output + inputlen - 10, 10);
    
    return 0;
}

static int enc_des(void *v, 
		   const char *input,
		   unsigned inputlen,
		   unsigned char digest[16],
		   char *output,
		   unsigned *outputlen)
{
  digest_context_t *text = (digest_context_t *) v;
  int len;
  int paddinglen;
  
  /* determine padding length */
  paddinglen= 8 - ((inputlen+10)%8);

  /* now construct the full stuff to be ciphered */
  memcpy(output, input, inputlen);                /* text */
  memset(output+inputlen, paddinglen, paddinglen);/* pad  */
  memcpy(output+inputlen+paddinglen, digest, 10); /* hmac */

  len=inputlen+paddinglen+10;

  des_cbc_encrypt((void *) output,
		  (void *) output,
		  len,
		  text->keysched_enc,
		  &text->ivec_enc,
		  DES_ENCRYPT);

  /* Update the ivec (des_cbc_encrypt implementations tend to be broken in
     this way) */
  memcpy(text->ivec_enc, output + (len - 8), 8);

  *outputlen=len;

  return 0;
}

static int init_des(void *v,
		    char enckey[16],
		    char deckey[16])
{
    digest_context_t *text = (digest_context_t *) v;
    
    des_key_sched((des_cblock *) enckey, text->keysched_enc);
    memcpy(text->ivec_enc, ((char *) enckey) + 8, 8);
    
    des_key_sched((des_cblock *) deckey, text->keysched_dec);
    memcpy(text->ivec_dec, ((char *) deckey) + 8, 8);
    
    memcpy(text->ivec_enc, ((char *) enckey) + 8, 8);
    memcpy(text->ivec_dec, ((char *) deckey) + 8, 8);
    
    return 0;
}

#endif /* WITH_DES */

#ifdef WITH_RC4
/* quick generic implementation of RC4 */
struct rc4_context_s {
    unsigned char sbox[256];
    int i, j;
};

static void rc4_init(rc4_context_t *text,
		     const unsigned char *key,
		     unsigned keylen)
{
    int i, j;
    
    /* fill in linearly s0=0 s1=1... */
    for (i=0;i<256;i++)
	text->sbox[i]=i;
    
    j=0;
    for (i = 0; i < 256; i++) {
	unsigned char tmp;
	/* j = (j + Si + Ki) mod 256 */
	j = (j + text->sbox[i] + key[i % keylen]) % 256;
	
	/* swap Si and Sj */
	tmp = text->sbox[i];
	text->sbox[i] = text->sbox[j];
	text->sbox[j] = tmp;
    }
    
    /* counters initialized to 0 */
    text->i = 0;
    text->j = 0;
}

static void rc4_encrypt(rc4_context_t *text,
			const char *input,
			char *output,
			unsigned len)
{
    int tmp;
    int i = text->i;
    int j = text->j;
    int t;
    int K;
    const char *input_end = input + len;
    
    while (input < input_end) {
	i = (i + 1) % 256;
	
	j = (j + text->sbox[i]) % 256;
	
	/* swap Si and Sj */
	tmp = text->sbox[i];
	text->sbox[i] = text->sbox[j];
	text->sbox[j] = tmp;
	
	t = (text->sbox[i] + text->sbox[j]) % 256;
	
	K = text->sbox[t];
	
	/* byte K is Xor'ed with plaintext */
	*output++ = *input++ ^ K;
    }
    
    text->i = i;
    text->j = j;
}

static void rc4_decrypt(rc4_context_t *text,
			const char *input,
			char *output,
			unsigned len)
{
    int tmp;
    int i = text->i;
    int j = text->j;
    int t;
    int K;
    const char *input_end = input + len;
    
    while (input < input_end) {
	i = (i + 1) % 256;
	
	j = (j + text->sbox[i]) % 256;
	
	/* swap Si and Sj */
	tmp = text->sbox[i];
	text->sbox[i] = text->sbox[j];
	text->sbox[j] = tmp;
	
	t = (text->sbox[i] + text->sbox[j]) % 256;
	
	K = text->sbox[t];
	
	/* byte K is Xor'ed with plaintext */
	*output++ = *input++ ^ K;
    }
    
    text->i = i;
    text->j = j;
}

static void free_rc4(void *v) 
{
    digest_context_t *text = (digest_context_t *) v;
    
    /* allocate rc4 context structures */
    if(text->rc4_enc_context) free(text->rc4_enc_context);
    if(text->rc4_dec_context) free(text->rc4_dec_context);
}

static int init_rc4(void *v, 
		    char enckey[16],
		    char deckey[16])
{
    digest_context_t *text = (digest_context_t *) v;
    
    /* allocate rc4 context structures */
    text->rc4_enc_context=
	(rc4_context_t *) malloc(sizeof(rc4_context_t));
    if (text->rc4_enc_context==NULL) return SASL_NOMEM;
    
    text->rc4_dec_context=
	(rc4_context_t *) malloc(sizeof(rc4_context_t));
    if (text->rc4_dec_context==NULL) return SASL_NOMEM;
    
    /* initialize them */
    rc4_init(text->rc4_enc_context,(const unsigned char *) enckey, 16);
    rc4_init(text->rc4_dec_context,(const unsigned char *) deckey, 16);
    
    return 0;
}

static int dec_rc4(void *v,
		   const char *input,
		   unsigned inputlen,
		   unsigned char digest[16],
		   char *output,
		   unsigned *outputlen)
{
    digest_context_t *text = (digest_context_t *) v;
    
    /* decrypt the text part */
    rc4_decrypt(text->rc4_dec_context, input, output, inputlen-10);
    
    /* decrypt the HMAC part */
    rc4_decrypt(text->rc4_dec_context, 
		input+(inputlen-10), (char *) digest, 10);
    
    /* no padding so we just subtract the HMAC to get the text length */
    *outputlen = inputlen - 10;
    
    return 0;
}

static int enc_rc4(void *v,
		   const char *input,
		   unsigned inputlen,
		   unsigned char digest[16],
		   char *output,
		   unsigned *outputlen)
{
    digest_context_t *text = (digest_context_t *) v;
    
    /* pad is zero */
    *outputlen = inputlen+10;
    
    /* encrypt the text part */
    rc4_encrypt(text->rc4_enc_context, (const char *) input, output, inputlen);
    
    /* encrypt the HMAC part */
    rc4_encrypt(text->rc4_enc_context, (const char *) digest, 
		(output)+inputlen, 10);
    
    return 0;
}

#endif /* WITH_RC4 */

struct digest_cipher available_ciphers[] =
{
#ifdef WITH_RC4
    { "rc4-40", 40, 5, 0x01, &enc_rc4, &dec_rc4, &init_rc4, &free_rc4 },
    { "rc4-56", 56, 7, 0x02, &enc_rc4, &dec_rc4, &init_rc4, &free_rc4 },
    { "rc4", 128, 16, 0x04, &enc_rc4, &dec_rc4, &init_rc4, &free_rc4 },
#endif
#ifdef WITH_DES
    { "des", 55, 16, 0x08, &enc_des, &dec_des, &init_des, NULL },
    { "3des", 112, 16, 0x10, &enc_3des, &dec_3des, &init_3des, NULL },
#endif
    { NULL, 0, 0, 0, NULL, NULL, NULL, NULL }
};

#if 0
static int create_layer_keys(digest_context_t *text,
			     HASH key, int keylen,
			     char enckey[16], char deckey[16])
{
    CC_MD5_CTX Md5Ctx;
    
    CC_MD5_Init(&Md5Ctx);
    CC_MD5_Update(&Md5Ctx, key, keylen);
    if (text->i_am == DIGEST_SERVER_TYPE) {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *) SEALING_SERVER_CLIENT, 
			 strlen(SEALING_SERVER_CLIENT));
    } else {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *) SEALING_CLIENT_SERVER,
			 strlen(SEALING_CLIENT_SERVER));
    }
    CC_MD5_Final((unsigned char *) enckey, &Md5Ctx);
    
    CC_MD5_Init(&Md5Ctx);
    CC_MD5_Update(&Md5Ctx, key, keylen);
    if (text->i_am != DIGEST_SERVER_TYPE) {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *)SEALING_SERVER_CLIENT, 
			 strlen(SEALING_SERVER_CLIENT));
    } else {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *)SEALING_CLIENT_SERVER,
			 strlen(SEALING_CLIENT_SERVER));
    }
    CC_MD5_Final((unsigned char *) deckey, &Md5Ctx);
    
    /* create integrity keys */
    /* sending */
    CC_MD5_Init(&Md5Ctx);
    CC_MD5_Update(&Md5Ctx, text->HA1, HASHLEN);
    if (text->i_am == DIGEST_SERVER_TYPE) {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *)SIGNING_SERVER_CLIENT, 
			 strlen(SIGNING_SERVER_CLIENT));
    } else {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *)SIGNING_CLIENT_SERVER,
			 strlen(SIGNING_CLIENT_SERVER));
    }
    CC_MD5_Final(text->Ki_send, &Md5Ctx);
    
    /* receiving */
    CC_MD5_Init(&Md5Ctx);
    CC_MD5_Update(&Md5Ctx, text->HA1, HASHLEN);
    if (text->i_am != DIGEST_SERVER_TYPE) {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *)SIGNING_SERVER_CLIENT, 
			 strlen(SIGNING_SERVER_CLIENT));
    } else {
	CC_MD5_Update(&Md5Ctx, (const unsigned char *)SIGNING_CLIENT_SERVER,
			 strlen(SIGNING_CLIENT_SERVER));
    }
    CC_MD5_Final(text->Ki_receive, &Md5Ctx);
    
    return 0;
}
#endif

static void
clear_global_context(global_context_t *glob_context)
{
    time_t timeout;
	
	if ( glob_context == NULL )
		return;
	
    if (glob_context->authid) free(glob_context->authid);
    if (glob_context->nonce) free(glob_context->nonce);
    if (glob_context->cnonce) free(glob_context->cnonce);
    if (glob_context->realm) free(glob_context->realm);
	if (glob_context->method) free(glob_context->method);

    if (glob_context->serverFQDN) free(glob_context->serverFQDN);
    if (glob_context->qop) free(glob_context->qop);

    /* zero everything except for the reauth timeout */
    timeout = glob_context->timeout;
    bzero(glob_context, sizeof(global_context_t));
    glob_context->timeout = timeout;
}

static void
free_global_context(void *glob_context)
{
    global_context_t *glob_text = (global_context_t *) glob_context;
    
    clear_global_context(glob_text);

    free(glob_text);
}

void
digest_dispose(void *conn_context)
{
    digest_context_t *text = (digest_context_t *) conn_context;
    
    if (text == NULL)
		return;
    
	if (text->global != NULL)
		free_global_context(text->global);
	
    if (text->cipher_free)
		text->cipher_free(text);
    
    /* free the stuff in the context */
    if (text->response_value)
		free(text->response_value);
    
    if (text->buffer) free(text->buffer);
    if (text->encode_buf) free(text->encode_buf);
    if (text->decode_buf) free(text->decode_buf);
    if (text->decode_once_buf) free(text->decode_once_buf);
    if (text->decode_tmp_buf) free(text->decode_tmp_buf);
    if (text->out_buf) free(text->out_buf);
    if (text->MAC_buf) free(text->MAC_buf);
	if (text->digesturi) free(text->digesturi);

	if (text->username) free(text->username);
	if (text->authorization_id) free(text->authorization_id);
	if (text->cnonce) free(text->cnonce);
	if (text->realm) free(text->realm);
	if (text->qop) free(text->qop);
	if (text->algorithmStr) free(text->algorithmStr);
	if (text->response) free(text->response);
	if (text->cipher) free(text->cipher);
	if (text->charset) free(text->charset);
	if (text->userid) free(text->userid);

    if (text->enc_in_buf) {
		if (text->enc_in_buf->data)
			free(text->enc_in_buf->data);
		free(text->enc_in_buf);
    }
	
	bzero(conn_context, sizeof(digest_context_t));
}


/*****************************  Server Section  *****************************/

static void
DigestCalcHA1FromSecret(digest_context_t * text,
			HASH HA1,
			unsigned char *authorization_id,
			unsigned char *pszNonce,
			unsigned char *pszCNonce,
			HASHHEX SessionKey)
{
    CC_MD5_CTX Md5Ctx;
    
    /* calculate session key */
    CC_MD5_Init(&Md5Ctx);
    CC_MD5_Update(&Md5Ctx, HA1, HASHLEN);
    CC_MD5_Update(&Md5Ctx, COLON, 1);
    CC_MD5_Update(&Md5Ctx, pszNonce, strlen((char *) pszNonce));
    CC_MD5_Update(&Md5Ctx, COLON, 1);
    CC_MD5_Update(&Md5Ctx, pszCNonce, strlen((char *) pszCNonce));
    if (authorization_id != NULL) {
		CC_MD5_Update(&Md5Ctx, COLON, 1);
		CC_MD5_Update(&Md5Ctx, authorization_id, strlen((char *) authorization_id));
    }
    CC_MD5_Final(HA1, &Md5Ctx);
    
    CvtHex(HA1, SessionKey);
    
    /* save HA1 because we need it to make the privacy and integrity keys */
    memcpy(text->HA1, HA1, sizeof(HASH));
}

static char *
create_MD5_response(digest_context_t * text,
			     unsigned char *nonce,
			     char *digesturi,
			     HASH Secret,
			     char *authorization_id,
				 char *method,
			     unsigned char **response_value)
{
    HASHHEX         HEntity = "00000000000000000000000000000000";
    HASHHEX         SessionKey;
    HASHHEX         Response;
    char           *result;
    
	CvtHex((const unsigned char *)Secret, SessionKey);
	//utils->seterror(utils->conn, 0, "HA1 = %s", SessionKey);
	
	DigestCalcOldResponse(
		       SessionKey,	/* H(A1) */
		       nonce,	/* nonce from server */
		       (unsigned char *) digesturi,	/* requested URL */
		       (unsigned char *) (method ? method : "GET"),	/* OPTIONS, PROPFIND, GET */
		       HEntity,	/* H(entity body) if qop="auth-int" */
		       Response	/* request-digest or response-digest */
    );
    
    result = malloc(HASHHEXLEN + 1);
    memcpy(result, Response, HASHHEXLEN);
    result[HASHHEXLEN] = 0;
    
    /* response_value (used for reauth i think */
    if (response_value != NULL) {
	DigestCalcOldResponse(
		       SessionKey,	/* H(A1) */
		       nonce,	/* nonce from server */
		       (unsigned char *) digesturi,	/* requested URL */
		       NULL,
		       HEntity,	/* H(entity body) if qop="auth-int" */
		       Response	/* request-digest or response-digest */
    );
	
	*response_value = malloc(HASHHEXLEN + 1);
	if (*response_value == NULL)
	    return NULL;
	memcpy(*response_value, Response, HASHHEXLEN);
	(*response_value)[HASHHEXLEN] = 0;
    }
    return result;
}

static char *
create_response(digest_context_t * text,
			     unsigned char *nonce,
			     unsigned int ncvalue,
			     unsigned char *cnonce,
			     char *qop,
			     char *digesturi,
			     HASH Secret,
			     char *authorization_id,
				 char *method,
	unsigned char **response_value)
{
    HASHHEX         SessionKey;
    HASHHEX         HEntity = "00000000000000000000000000000000";
    HASHHEX         Response;
    char           *result;
    
	if ( text->global->algorithm == kDigestAlgorithmMD5_sess )
	{
		DigestCalcHA1FromSecret(text,
					Secret,
					(unsigned char *) authorization_id,
					nonce,
					cnonce,
					SessionKey);
	}
	else
	{
		CvtHex(Secret, SessionKey);
	}
	
	if (qop == NULL)
		qop = "auth";
    
    DigestCalcResponse(
		       SessionKey,/* H(A1) */
		       nonce,	/* nonce from server */
		       ncvalue,	/* 8 hex digits */
		       cnonce,	/* client nonce */
		       (unsigned char *) qop,	/* qop-value: "", "auth", "auth-int" */
		       (unsigned char *) digesturi,	/* requested URL */
		       method ? method : "AUTHENTICATE",
		       HEntity,	/* H(entity body) if qop="auth-int" */
		       Response	/* request-digest or response-digest */
	);
    
    result = malloc(HASHHEXLEN + 1);
    memcpy(result, Response, HASHHEXLEN);
    result[HASHHEXLEN] = 0;
    
    /* response_value (used for reauth i think */
    if (response_value != NULL) {
		DigestCalcResponse(
				   SessionKey,	/* H(A1) */
				   nonce,	/* nonce from server */
				   ncvalue,	/* 8 hex digits */
				   cnonce,	/* client nonce */
					   (unsigned char *) qop,	/* qop-value: "", "auth", * "auth-int" */
				   (unsigned char *) digesturi,	/* requested URL */
				   NULL,
				   HEntity,	/* H(entity body) if qop="auth-int" */
				   Response	/* request-digest or response-digest */
			);
		
		*response_value = malloc(HASHHEXLEN + 1);
		if (*response_value == NULL)
			return NULL;
		memcpy(*response_value, Response, HASHHEXLEN);
		(*response_value)[HASHHEXLEN] = 0;
    }
    return result;
}


/*
 * Convert hex string to int
 */
static int htoi(unsigned char *hexin, unsigned int *res)
{
    int             lup, inlen;
    inlen = strlen((char *) hexin);
    
    *res = 0;
    for (lup = 0; lup < inlen; lup++) {
		switch (hexin[lup]) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				*res = (*res << 4) + (hexin[lup] - '0');
				break;
				
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				*res = (*res << 4) + (hexin[lup] - 'a' + 10);
				break;
				
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				*res = (*res << 4) + (hexin[lup] - 'A' + 10);
				break;
				
			default:
				return SASL_BADPARAM;
		}	
    }
    
    return 0;
}


    /*
     * username         = "username" "=" <"> username-value <">
     * username-value   = qdstr-val cnonce           = "cnonce" "=" <">
     * cnonce-value <"> cnonce-value     = qdstr-val nonce-count      = "nc"
     * "=" nc-value nc-value         = 8LHEX qop              = "qop" "="
     * qop-value digest-uri = "digest-uri" "=" digest-uri-value
     * digest-uri-value  = serv-type "/" host [ "/" serv-name ] serv-type
     * = 1*ALPHA host             = 1*( ALPHA | DIGIT | "-" | "." ) service
     * = host response         = "response" "=" <"> response-value <">
     * response-value   = 32LHEX LHEX = "0" | "1" | "2" | "3" | "4" | "5" |
     * "6" | "7" | "8" | "9" | "a" | "b" | "c" | "d" | "e" | "f" cipher =
     * "cipher" "=" cipher-value
     */

int
digest_server_parse(
	const char *serverChallengeStr,
	unsigned serverChallengeStrLen,
	const char *clientin,
	digest_context_t *outContext)
{
    int				result				= SASL_OK;
    unsigned int    client_maxbuf		= 65536;
    int             maxbuf_count		= 0;		/* How many maxbuf instances was found */
	char			*tptr				= NULL;
	char			*name				= NULL;
	char			*value				= NULL;
	char			*outCopy			= NULL;
	unsigned int	dumpablePrefixLen	= sizeof(kDumpablePrefix) - 1;
	int				startPos			= 0;
	char			*in_start			= NULL;
	char			*in					= NULL;
	
    if (serverChallengeStrLen > 4096)
		return SASL_FAIL;
	
	/* parse the server challenge */
	if ( serverChallengeStr != NULL && serverChallengeStrLen > 0 )
	{
		if ( outContext->global == NULL )
			outContext->global = (global_context_t *)calloc( 1, sizeof(global_context_t) );
		else
			clear_global_context(outContext->global);
		outContext->global->isreplay = 1;
		
		if ( serverChallengeStrLen > dumpablePrefixLen &&
			 strncasecmp(serverChallengeStr, kDumpablePrefix, dumpablePrefixLen) == 0 )
		{
			startPos += dumpablePrefixLen;
		}
		
		outContext->out_buf = (char *) calloc( 1, serverChallengeStrLen - startPos + 1 );
		if ( outContext->out_buf == NULL )
			return SASL_NOMEM;
		outContext->out_buf_len = serverChallengeStrLen - startPos + 1;
		
		memcpy( outContext->out_buf, serverChallengeStr + startPos, serverChallengeStrLen - startPos );
		outContext->out_buf[serverChallengeStrLen-startPos] = '\0';
		
		/* parse this thing and fill out the context */
		outContext->global->timestamp = time(0);
		outContext->global->nonce_count = 0;
		outContext->global->nonce = NULL;
		
		/* get_pair() modifies the string so we must use a copy */
		outCopy = strdup(outContext->out_buf);
		if (outCopy == NULL)
			return SASL_FAIL;
		
		tptr = strstr(outCopy, "nonce");
		if ( tptr != NULL )
		{
			get_pair(&tptr, &name, &value);
			if ( value != NULL ) {
				outContext->global->nonce = (unsigned char *)strdup(value);
				outContext->global->nonce_count++;
			}
		}
		
		free( outCopy );
		
		if ( outContext->global->nonce_count == 0 ) {
			/* not going to work */
			return SASL_FAIL;
		}
		
		/* get_pair() modifies the string, get another fresh copy */
		outCopy = strdup(outContext->out_buf);
		if (outCopy == NULL)
			return SASL_FAIL;
		
		outContext->global->realm = NULL;
		tptr = strstr(outCopy, "realm");
		if ( tptr != NULL )
		{
			get_pair(&tptr, &name, &value);
			if ( value != NULL ) {
				outContext->global->realm = strdup(value);
			}
		}
		free( outCopy );
		
		/* get_pair() modifies the string, get another fresh copy */
		outCopy = strdup(outContext->out_buf);
		if (outCopy == NULL)
			return SASL_FAIL;
		
		outContext->global->method = NULL;
		tptr = strstr(outCopy, "method");
		if ( tptr != NULL )
		{
			get_pair(&tptr, &name, &value);
			if ( value != NULL ) {
				outContext->global->method = strdup(value);
			}
		}
		free( outCopy );
		
		if (result != 0)
			return result;
	}
	
	/* until we find out otherwise, the algorithm default is RFC 2069 */
	outContext->global->algorithm = kDigestAlgorithmMD5_RFC2069;
	
	/* Step 2: parse the client response */
	/* Note that <outContext> contains the server challenge parameters. The client's response */
	/* must be consistent with the original context. */
	
	in_start = (char *) malloc( strlen(clientin) + 1 );
	if ( in_start == NULL )
		return SASL_NOMEM;
	
	strcpy( in_start, clientin );
	in = in_start;
	
	if ( strncasecmp(in, kDumpablePrefix, dumpablePrefixLen) == 0 )
		in += dumpablePrefixLen;
	
	while (in[0] != '\0')
	{
		name = NULL;
		value = NULL;
		get_pair(&in, &name, &value);
		//sparams->utils->log(sparams->utils->conn, "name=%s, value=%s", name, value);
		
		if (name == NULL)
			break;
		
		/* Extracting parameters for step 2 */
		
		/*
		 * digest-response  = 1#( username | realm | nonce | cnonce |
		 * nonce-count | qop | digest-uri | response | maxbuf | charset |
		 * cipher | auth-param )
		 */
		
		if (strcasecmp(name, "username") == 0)
		{
			if (outContext->global->authid && (strcmp(value, outContext->global->authid) != 0)) {
				//SETERROR(sparams->utils, "username changed: authentication aborted");
				result = SASL_FAIL;
				break;
			}
			
			outContext->username = strdup(value);
		}
		else if (strcasecmp(name, "authzid") == 0)
		{
			outContext->authorization_id = strdup(value);
		}
		else if (strcasecmp(name, "cnonce") == 0)
		{
			if (outContext->global->cnonce && (strcmp(value, (char *)outContext->global->cnonce) != 0)) {
				//SETERROR(sparams->utils, "cnonce changed: authentication aborted");
				result = SASL_FAIL;
				break;
			}
			
			outContext->cnonce = (unsigned char *)strdup(value);
		}
		else if (strcasecmp(name, "nc") == 0)
		{
			if (htoi((unsigned char *) value, &outContext->noncecount) != SASL_OK) {
				//SETERROR(sparams->utils, "error converting hex to int");
				result = SASL_BADAUTH;
				break;
			}
		}
		else if (strcasecmp(name, "realm") == 0)
		{
			if (outContext->realm) {
				//SETERROR(sparams->utils, "duplicate realm: authentication aborted");
				result = SASL_FAIL;
				break;
			}
			else if (outContext->global->realm && (strcmp(value, outContext->global->realm) != 0)) {
				//SETERROR(sparams->utils, "realm changed: authentication aborted");
				result = SASL_FAIL;
				break;
			}
			
			outContext->realm = strdup(value);
		}
		else if (strcasecmp(name, "nonce") == 0)
		{
			if (strcmp(value, (char *) outContext->global->nonce) != 0) {
				/*
				 * Nonce changed: Abort authentication!!!
				 */
				//SETERROR(sparams->utils, "nonce changed: authentication aborted");
				result = SASL_BADAUTH;
				break;
			}
		}
		else if (strcasecmp(name, "qop") == 0)
		{
			outContext->qop = strdup(value);
		}
		else if (strcasecmp(name, "digest-uri") == 0 || strcasecmp(name, "uri") == 0)
		{
			/* XXX: verify digest-uri format */
			/*
			 * digest-uri-value  = serv-type "/" host [ "/" serv-name ]
			 */
			outContext->digesturi = strdup(value);
		}
		else if (strcasecmp(name, "response") == 0)
		{
			outContext->response = strdup(value);
		}
		else if (strcasecmp(name, "cipher") == 0)
		{
			outContext->cipher = strdup(value);
		}
		else if (strcasecmp(name, "maxbuf") == 0) 
		{
			maxbuf_count++;
			if (maxbuf_count != 1) {
				result = SASL_BADAUTH;
				//SETERROR(sparams->utils, "duplicate maxbuf: authentication aborted");
				break;
			}
			else if (sscanf(value, "%u", &client_maxbuf) != 1) {
				result = SASL_BADAUTH;
				//SETERROR(sparams->utils, "invalid maxbuf parameter");
				break;
			}
			else {
				if (client_maxbuf <= 16) {
					result = SASL_BADAUTH;
					//SETERROR(sparams->utils, "maxbuf parameter too small");
					break;
				}
			}
		}
		else if (strcasecmp(name, "charset") == 0) {
			if (strcasecmp(value, "utf-8") != 0) {
				//SETERROR(sparams->utils, "client doesn't support UTF-8");
				result = SASL_FAIL;
				break;
			}
			outContext->charset = strdup(value);
		}
		else if (strcasecmp(name, "userid") == 0) {
			outContext->userid = strdup(value);
		}
		else if (strcasecmp(name, "algorithm") == 0) {
			outContext->algorithmStr = strdup(value);
		}
		else {
			//sparams->utils->log(sparams->utils->conn, SASL_LOG_DEBUG, "DIGEST-MD5 unrecognized pair %s/%s: ignoring", name, value);
		}
	}

	do
	{
		if ( result == SASL_OK )
		{		
			/* defaulting qop to "" if not specified */
			/* we want to be able to distinguish NULL or "" from "auth" */
			/* to decide RFC 2069 or RFC 2617 */
			if (outContext->qop == NULL) {
				outContext->qop = (char *) calloc(1, 1);
				if (outContext->qop == NULL)
					result = SASL_NOMEM;
					break;
			}
					
			/* pick the algorithm to use */
			if (outContext->algorithmStr != NULL && outContext->qop[0] != '\0') {
				if (strcasecmp(outContext->algorithmStr, "md5") == 0)
					outContext->global->algorithm = kDigestAlgorithmMD5;
				else if (strcasecmp(outContext->algorithmStr, "md5-sess") == 0)
					outContext->global->algorithm = kDigestAlgorithmMD5_sess;
				else {
					// invalid
					result = SASL_BADPARAM;
					break;
				}
			}
			
			/* check which layer/cipher to use */
			if ((!strcasecmp(outContext->qop, "auth-conf")) && (outContext->cipher != NULL))
			{
				/* auth-conf and md5-sess go together but the algorithm is not included in the client's */
				/* response when using SASL in accordance with RFC 2831 */
				/* 
					RFC 2831                 Digest SASL Mechanism                  May 2000

				   Also, in the HTTP usage of Digest, several directives in the
				   "digest-challenge" sent by the server have to be returned by the
				   client in the "digest-response". These are:

					opaque
					algorithm

				   These directives are not needed when Digest is used as a SASL
				   mechanism (i.e., MUST NOT be sent, and MUST be ignored if received).
				*/
				
				outContext->global->algorithm = kDigestAlgorithmMD5_sess;
				
				#if 0
				/* see what cipher was requested */
				struct digest_cipher *cptr;
				
				cptr = available_ciphers;
				while (cptr->name)
				{
					/* find the cipher requested & make sure it's one we're happy
					   with by policy */
					if (!strcasecmp(cipher, cptr->name) && 
						outContext->requiressf <= cptr->ssf &&
						outContext->limitssf >= cptr->ssf)
					{
						/* found it! */
						break;
					}
					cptr++;
				}
				#endif
			}
			
			/* Verifing that all parameters were defined */
			if ((outContext->username == NULL) ||
				(outContext->noncecount == 0 && outContext->global->algorithm != kDigestAlgorithmMD5_RFC2069) ||
				(outContext->cnonce == NULL && outContext->global->algorithm != kDigestAlgorithmMD5_RFC2069) ||
				(outContext->digesturi == NULL) ||
				(outContext->response == NULL))
			{
				//SETERROR(sparams->utils, "required parameters missing");
				result = SASL_BADAUTH;
				break;
			}
		}
	}
	while (0);
	
	if ( in_start != NULL )
		free( in_start );
	
	return result;
}


int
digest_verify(digest_context_t *inContext,
				const char *inPassword,
				unsigned int inPasswordLength,
				char **serverout,
			    unsigned *serveroutlen)
{
    sasl_secret_t			*sec			= NULL;
    int						result			= SASL_FAIL;
    char					*serverresponse	= NULL;
    unsigned int			client_maxbuf	= 65536;
    HASH					A1;
    
    /*
     * username         = "username" "=" <"> username-value <">
     * username-value   = qdstr-val cnonce           = "cnonce" "=" <">
     * cnonce-value <"> cnonce-value     = qdstr-val nonce-count      = "nc"
     * "=" nc-value nc-value         = 8LHEX qop              = "qop" "="
     * qop-value digest-uri = "digest-uri" "=" digest-uri-value
     * digest-uri-value  = serv-type "/" host [ "/" serv-name ] serv-type
     * = 1*ALPHA host             = 1*( ALPHA | DIGIT | "-" | "." ) service
     * = host response         = "response" "=" <"> response-value <">
     * response-value   = 32LHEX LHEX = "0" | "1" | "2" | "3" | "4" | "5" |
     * "6" | "7" | "8" | "9" | "a" | "b" | "c" | "d" | "e" | "f" cipher =
     * "cipher" "=" cipher-value
     */

    /* Verifing that all parameters were defined */
    if ((inContext->username == NULL) ||
		(inContext->noncecount == 0 && inContext->global->algorithm != kDigestAlgorithmMD5_RFC2069) ||
		(inContext->cnonce == NULL && inContext->global->algorithm != kDigestAlgorithmMD5_RFC2069) ||
		(inContext->digesturi == NULL) ||
		(inContext->response == NULL))
	{
		//SETERROR(sparams->utils, "required parameters missing");
		result = SASL_BADAUTH;
		goto FreeAllMem;
    }

	sec = (sasl_secret_t *) malloc(sizeof(sasl_secret_t) + inPasswordLength);
	if (sec == NULL) {
	    //SETERROR(sparams->utils, "unable to allocate secret");
	    result = SASL_NOMEM;
	    goto FreeAllMem;
	}
	
	sec->len = inPasswordLength;
	strncpy((char *)sec->data, inPassword, inPasswordLength + 1); 
	
	/*
	 * Verifying response obtained from client
	 * 
	 * H_URP = H({ username-value,":",realm-value,":",passwd}) sec->data
	 * contains H_URP
	 */
	
	/* Calculate the secret from the plaintext password */
	{
	    HASH HA1;
	    
	    DigestCalcSecret((unsigned char *)inContext->username, (unsigned char *)inContext->global->realm, sec->data, sec->len, HA1);
	    
	    /*
	     * A1 = { H( { username-value, ":", realm-value, ":", passwd } ),
	     * ":", nonce-value, ":", cnonce-value }
	     */
	    
	    memcpy(A1, HA1, HASHLEN);
	    A1[HASHLEN] = '\0';
	}
	
	if ( inContext->global->algorithm == kDigestAlgorithmMD5_RFC2069 )
	{
		serverresponse = create_MD5_response(inContext,
						inContext->global->nonce,
						inContext->digesturi,
						A1,
						inContext->authorization_id,
						inContext->global->method,
						&inContext->response_value);
	}
	else
	{
		serverresponse = create_response(inContext,
						inContext->global->nonce,
						inContext->noncecount,
						inContext->cnonce,
						inContext->qop,
						inContext->digesturi,
						A1,
						inContext->authorization_id,
						inContext->global->method,
						&inContext->response_value);
    }
	
    if (serverresponse == NULL) {
		//SETERROR(sparams->utils, "internal error: unable to create response");
		result = SASL_NOMEM;
		goto FreeAllMem;
    }
    
    /* if ok verified */
    if (strcmp(serverresponse, inContext->response) == 0)
	{
		result = SASL_OK;
	}
	else
	{
		//SETERROR(sparams->utils, "client response doesn't match what we generated");
		//sparams->utils->seterror(sparams->utils->conn, 0, "serverresponse = %s", serverresponse);
		result = SASL_BADAUTH;
		goto FreeAllMem;
    }
	
    inContext->seqnum = 0;		/* for integrity/privacy */
    inContext->rec_seqnum = 0;	/* for integrity/privacy */
    inContext->maxbuf = client_maxbuf;
    
    /* used by layers */
    inContext->size = -1;
    inContext->needsize = 4;
    inContext->buffer = NULL;
        
    /*
     * The server receives and validates the "digest-response". The server
     * checks that the nonce-count is "00000001". If it supports subsequent
     * authentication, it saves the value of the nonce and the nonce-count.
     */
    
    /*
     * The "username-value", "realm-value" and "passwd" are encoded according
     * to the value of the "charset" directive. If "charset=UTF-8" is
     * present, and all the characters of either "username-value" or "passwd"
     * are in the ISO 8859-1 character set, then it must be converted to
     * UTF-8 before being hashed. A sample implementation of this conversion
     * is in section 8.
     */
    
    /* add to challenge */
	{
		unsigned resplen = strlen((char *)inContext->response_value) + sizeof("rspauth") + 3;
		if ( inContext->out_buf != NULL ) {
			free( inContext->out_buf );
			inContext->out_buf = NULL;
		}
		inContext->out_buf = (char *) malloc( resplen );
		if ( inContext->out_buf == NULL ) {
			result = SASL_NOMEM;
			goto FreeAllMem;
		}
	}	
	
	inContext->out_buf_len = sprintf(inContext->out_buf, "rspauth=%s", inContext->response_value);
	
	/* self check */
	if (strlen(inContext->out_buf) > 2048) {
		free( inContext->out_buf );
		inContext->out_buf = NULL;
	    result = SASL_BUFOVER;
	    goto FreeAllMem;
	}

	/* increment the nonce count */
	inContext->global->nonce_count++;

	/* setup for a potential reauth */
	inContext->global->timestamp = time(0);

	if (!inContext->global->authid) {
	    inContext->global->authid = inContext->username;
	    inContext->username = NULL;
	}
	if (!inContext->global->cnonce) {
	    inContext->global->cnonce = inContext->cnonce;
	    inContext->cnonce = NULL;
	}
	
	*serveroutlen = strlen(inContext->out_buf);
	*serverout = inContext->out_buf;
	inContext->out_buf = NULL;	// memory is handed-off to the caller
	
	result = SASL_OK;
    
  FreeAllMem:
    /* free everything */
    /*
     * sparams->utils->free (authorization_id);
     */
	if ( serverresponse != NULL ) {
		free( serverresponse );
	}
	
	if ( sec != NULL ) {
		bzero( sec->data, sec->len );
		free( sec );
	}
	if ( inContext->username != NULL ) {
		free( inContext->username );
		inContext->username = NULL;
	}
	if ( inContext->authorization_id != NULL ) {
		free( inContext->authorization_id );
		inContext->authorization_id = NULL;
	}
	if ( inContext->cnonce != NULL ) {
		free( inContext->cnonce );
		inContext->cnonce = NULL;
	}
	if ( inContext->realm != NULL ) {
		free( inContext->realm );
		inContext->realm = NULL;
	}
	if ( inContext->qop != NULL ) {
		free( inContext->qop );
		inContext->qop = NULL;
	}
	if ( inContext->algorithmStr != NULL ) {
		free( inContext->algorithmStr );
		inContext->algorithmStr = NULL;
	}
	if ( inContext->digesturi != NULL ) {
		free( inContext->digesturi );
		inContext->digesturi = NULL;
	}
	if ( inContext->response != NULL ) {
		free( inContext->response );
		inContext->response = NULL;
	}
	if ( inContext->cipher != NULL ) {
		free( inContext->cipher );
		inContext->cipher = NULL;
	}
	if ( inContext->charset != NULL ) {
		free( inContext->charset );
		inContext->charset = NULL;
	}
	if ( inContext->userid != NULL ) {
		free( inContext->userid );
		inContext->userid = NULL;
	}
	
	return result;
}
