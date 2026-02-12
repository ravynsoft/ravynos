/*
 * chap.h - Challenge Handshake Authentication Protocol definitions.
 *
 * Copyright (c) 1993 The Australian National University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Australian National University.  The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Copyright (c) 1991 Gregory M. Christy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the author.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: chap.h,v 1.1 2004/09/10 21:17:27 snsimon Exp $
 */

#ifndef __CHAP_INCLUDE__

/* Code + ID + length */
#define CHAP_HEADERLEN		4

/*
 * CHAP codes.
 */

#define CHAP_DIGEST_MD5		5	/* use MD5 algorithm */
#define MD5_SIGNATURE_SIZE	16	/* 16 bytes in a MD5 message digest */
#define CHAP_MICROSOFT		0x80	/* use Microsoft-compatible alg. */
#define CHAP_MICROSOFT_V2	0x81	/* use Microsoft-compatible alg. */

/*
 * Digest type and selection.
 */

/* bitmask of supported algorithms */
#define MDTYPE_MICROSOFT_V2	0x1
#define MDTYPE_MICROSOFT	0x2
#define MDTYPE_MD5		0x4

#ifdef CHAPMS
#define MDTYPE_ALL (MDTYPE_MICROSOFT_V2 | MDTYPE_MICROSOFT | MDTYPE_MD5)
#else
#define MDTYPE_ALL (MDTYPE_MD5)
#endif
#define MDTYPE_NONE 0

/* Return the digest alg. ID for the most preferred digest type. */
#define CHAP_DIGEST(mdtype) \
    ((mdtype) & MDTYPE_MICROSOFT_V2)? CHAP_MICROSOFT_V2: \
    ((mdtype) & MDTYPE_MICROSOFT)? CHAP_MICROSOFT: \
    ((mdtype) & MDTYPE_MD5)? CHAP_DIGEST_MD5: \
    0

/* Return the bit flag (lsb set) for our most preferred digest type. */
#define CHAP_MDTYPE(mdtype) ((mdtype) ^ ((mdtype) - 1)) & (mdtype)

/* Return the bit flag for a given digest algorithm ID. */
#define CHAP_MDTYPE_D(digest) \
    ((digest) == CHAP_MICROSOFT_V2)? MDTYPE_MICROSOFT_V2: \
    ((digest) == CHAP_MICROSOFT)? MDTYPE_MICROSOFT: \
    ((digest) == CHAP_DIGEST_MD5)? MDTYPE_MD5: \
    0

/* Can we do the requested digest? */
#define CHAP_CANDIGEST(mdtype, digest) \
    ((digest) == CHAP_MICROSOFT_V2)? (mdtype) & MDTYPE_MICROSOFT_V2: \
    ((digest) == CHAP_MICROSOFT)? (mdtype) & MDTYPE_MICROSOFT: \
    ((digest) == CHAP_DIGEST_MD5)? (mdtype) & MDTYPE_MD5: \
    0

#define CHAP_CHALLENGE		1
#define CHAP_RESPONSE		2
#define CHAP_SUCCESS		3
#define CHAP_FAILURE    	4

/*
 *  Challenge lengths (for challenges we send) and other limits.
 */
#define MIN_CHALLENGE_LENGTH	16
#define MAX_CHALLENGE_LENGTH	24	/* sufficient for MS-CHAP Peer Chal. */
#define MAX_RESPONSE_LENGTH	64	/* sufficient for MD5 or MS-CHAP */
#define MS_AUTH_RESPONSE_LENGTH	40	/* MS-CHAPv2 authenticator response, */
					/* as ASCII */

/*
 * Each interface is described by a chap structure.
 */

typedef struct chap_state {
    int unit;			/* Interface unit number */
    int clientstate;		/* Client state */
    int serverstate;		/* Server state */
    u_char challenge[MAX_CHALLENGE_LENGTH]; /* last challenge string sent */
    u_char chal_len;		/* challenge length */
    u_char chal_id;		/* ID of last challenge */
    u_char chal_type;		/* hash algorithm for challenges */
    u_char id;			/* Current id */
    char *chal_name;		/* Our name to use with challenge */
    int chal_interval;		/* Time until we challenge peer again */
    int timeouttime;		/* Timeout time in seconds */
    int max_transmits;		/* Maximum # of challenge transmissions */
    int chal_transmits;		/* Number of transmissions of challenge */
    int resp_transmits;		/* Number of transmissions of response */
    u_char response[MAX_RESPONSE_LENGTH];	/* Response to send */
    char saresponse[MS_AUTH_RESPONSE_LENGTH+1];	/* Auth response to send */
    char earesponse[MS_AUTH_RESPONSE_LENGTH+1];	/* Auth response expected */
						/* +1 for null terminator */
    u_char resp_flags;		/* flags from MS-CHAPv2 auth response */
    u_char resp_length;		/* length of response */
    u_char resp_id;		/* ID for response messages */
    u_char resp_type;		/* hash algorithm for responses */
    char *resp_name;		/* Our name to send with response */
} chap_state;

/* We need the declaration of chap_state to use this prototype */
extern int (*chap_auth_hook) __P((char *user, u_char *remmd,
				  int remmd_len, chap_state *cstate));

/*
 * Client (peer) states.
 */
#define CHAPCS_INITIAL		0	/* Lower layer down, not opened */
#define CHAPCS_CLOSED		1	/* Lower layer up, not opened */
#define CHAPCS_PENDING		2	/* Auth us to peer when lower up */
#define CHAPCS_LISTEN		3	/* Listening for a challenge */
#define CHAPCS_RESPONSE		4	/* Sent response, waiting for status */
#define CHAPCS_OPEN		5	/* We've received Success */

/*
 * Server (authenticator) states.
 */
#define CHAPSS_INITIAL		0	/* Lower layer down, not opened */
#define CHAPSS_CLOSED		1	/* Lower layer up, not opened */
#define CHAPSS_PENDING		2	/* Auth peer when lower up */
#define CHAPSS_INITIAL_CHAL	3	/* We've sent the first challenge */
#define CHAPSS_OPEN		4	/* We've sent a Success msg */
#define CHAPSS_RECHALLENGE	5	/* We've sent another challenge */
#define CHAPSS_BADAUTH		6	/* We've sent a Failure msg */

/*
 * Timeouts.
 */
#define CHAP_DEFTIMEOUT		3	/* Timeout time in seconds */
#define CHAP_DEFTRANSMITS	10	/* max # times to send challenge */

extern chap_state chap[];

void ChapAuthWithPeer __P((int, char *, int));
void ChapAuthPeer __P((int, char *, int));

extern struct protent chap_protent;

#define __CHAP_INCLUDE__
#endif /* __CHAP_INCLUDE__ */
