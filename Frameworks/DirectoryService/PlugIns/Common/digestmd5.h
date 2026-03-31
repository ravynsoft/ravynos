/* DIGEST-MD5 SASL plugin
 * Rob Siemborski
 * Tim Martin
 * Alexey Melnikov 
 * $Id: digestmd5.h,v 1.6 2006/02/28 23:28:44 snsimon Exp $
 */
/* 
 * Copyright (c) 2001 Carnegie Mellon University.  All rights reserved.
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

#ifndef __digestmd5_h__
#define __digestmd5_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#ifndef macintosh
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <ctype.h>
#include <sasl/sasl.h>

/* DES support */
#ifdef WITH_DES
# ifdef WITH_SSL_DES
#  include <openssl/des.h>
# else /* system DES library */
#  include <des.h>
# endif
#endif /* WITH_DES */

#ifdef WIN32
# include <winsock.h>
#else /* Unix */
# include <netinet/in.h>
#endif /* WIN32 */

#ifdef WIN32
/* This must be after sasl.h, saslutil.h */
#include "saslDIGESTMD5.h"
#else /* Unix */
extern int      strcasecmp(const char *s1, const char *s2);
#endif /* end WIN32 */

#ifdef macintosh
//#include <sasl_md5_plugin_decl.h>
#endif

/* external definitions */

#ifdef sun
/* gotta define gethostname ourselves on suns */
extern int      gethostname(char *, int);
#endif

#ifndef TRUE
#define TRUE  (1)
#define FALSE (0)
#endif

#define kDumpablePrefix		"Digest "
/* Definitions */
#define NONCE_SIZE (32)		/* arbitrary */

/* Layer Flags */
#define DIGEST_NOLAYER    (1)
#define DIGEST_INTEGRITY  (2)
#define DIGEST_PRIVACY    (4)

/* defines */
#define HASHLEN 16
typedef unsigned char HASH[HASHLEN + 1];
#define HASHHEXLEN 32
typedef unsigned char HASHHEX[HASHHEXLEN + 1];

#define MAC_SIZE 10
#define MAC_OFFS 2


#define HT	(9)
#define CR	(13)
#define LF	(10)
#define SP	(32)
#define DEL	(127)

/* function definitions for cipher encode/decode */
typedef int cipher_function_t(void *,
			      const char *,
			      unsigned,
			      unsigned char[],
			      char *,
			      unsigned *);

typedef int cipher_init_t(void *, char [16], char [16]);
typedef void cipher_free_t(void *);

enum Context_type { DIGEST_SERVER_TYPE = 0, DIGEST_CLIENT_TYPE = 1 };

#ifdef WITH_RC4
typedef struct rc4_context_s rc4_context_t;
#endif

typedef enum DigestAlgorithm {
	kDigestAlgorithmMD5_RFC2069 = 0,
	kDigestAlgorithmMD5			= 1,
	kDigestAlgorithmMD5_sess	= 2
} DigestAlgorithm;

typedef struct buffer_info {
    char *data;
    unsigned curlen;   /* Current length of data in buffer */
    unsigned reallen;  /* total length of buffer (>= curlen) */
} buffer_info_t;

typedef struct global_context {
    /* common stuff */
    char *authid;
    char *realm;
    char *method;
    unsigned char *nonce;
    unsigned int nonce_count;
    unsigned char *cnonce;

	char isreplay;
	DigestAlgorithm algorithm;
	
    /* server stuff */
    time_t timeout;
    time_t timestamp;

    /* client stuff */
    char *serverFQDN;
    char *qop;
    struct digest_cipher *bestcipher;
    unsigned int server_maxbuf;
} global_context_t;

/* context that stores info */
typedef struct context {
    int state;			/* state in the authentication we are in */
    enum Context_type i_am;	/* are we the client or server? */
    
    global_context_t *global;
	
	/* params gathered from the client response */
	char *username;
    char *authorization_id;
    char *realm;
    unsigned char *cnonce;
    unsigned int noncecount;
    char *qop;
	char *algorithmStr;
    char *digesturi;
    unsigned int client_maxbuf;
    int maxbuf_count;  /* How many maxbuf instances were found */
    char *charset;
    char *userid;
    char *cipher;
    unsigned int n;
	char *response;
	
    unsigned char *response_value;
    
    unsigned int seqnum;
    unsigned int rec_seqnum;	/* for checking integrity */
    
    HASH Ki_send;
    HASH Ki_receive;
    
    HASH HA1;		/* Kcc or Kcs */
        
    /* For general use */
    char *out_buf;
    uint32_t out_buf_len;
    
    /* for encoding/decoding */
    buffer_info_t *enc_in_buf;
    char *encode_buf, *decode_buf, *decode_once_buf;
    unsigned encode_buf_len, decode_buf_len, decode_once_buf_len;
    char *decode_tmp_buf;
    unsigned decode_tmp_buf_len;
    char *MAC_buf;
    unsigned MAC_buf_len;
    
    char *buffer;
    char sizebuf[4];
    int cursize;
    int size;
    int needsize;
    
    /* Server MaxBuf for Client or Client MaxBuf For Server */
    unsigned int maxbuf;
    
    /* if privacy mode is used use these functions for encode and decode */
    cipher_function_t *cipher_enc;
    cipher_function_t *cipher_dec;
    cipher_init_t *cipher_init;
    cipher_free_t *cipher_free;
    
#ifdef WITH_DES
    des_key_schedule keysched_enc;   /* key schedule for des initialization */
    des_cblock ivec_enc;	     /* initial vector for encoding */
    des_key_schedule keysched_dec;   /* key schedule for des initialization */
    des_cblock ivec_dec;	     /* init vec for decoding */
    
    des_key_schedule keysched_enc2;  /* key schedule for 3des initialization */
    des_key_schedule keysched_dec2;  /* key schedule for 3des initialization */
#endif
    
#ifdef WITH_RC4
    rc4_context_t *rc4_enc_context;
    rc4_context_t *rc4_dec_context;
#endif /* WITH_RC4 */
} digest_context_t;

struct digest_cipher {
    char *name;
    unsigned ssf;
    int n; /* bits to make privacy key */
    int flag; /* a bitmask to make things easier for us */
    
    cipher_function_t *cipher_enc;
    cipher_function_t *cipher_dec;
    cipher_init_t *cipher_init;
    cipher_free_t *cipher_free;
};

void digest_dispose(void *conn_context);

int digest_server_parse(
	const char *serverChallengeStr,
	unsigned serverChallengeStrLen,
	const char *clientin,
	digest_context_t *outContext);

int digest_verify(digest_context_t *inContext,
	const char *inPassword,
	unsigned int inPasswordLength,
	char **serverout,
	unsigned *serveroutlen);

#ifdef __cplusplus
};
#endif

#endif /* __digestmd5_h__ */
