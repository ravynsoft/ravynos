/*
 * sdbm - ndbm work-alike hashed database library
 * based on Per-Ake Larson's Dynamic Hashing algorithms. BIT 18 (1978).
 * author: oz@nexus.yorku.ca
 * status: public domain. 
 */
#ifndef PERL_SDBM_FILE_SDBM_H_
#define PERL_SDBM_FILE_SDBM_H_

#define DBLKSIZ 4096
#define PBLKSIZ 1024
#define PAIRMAX 1008			/* arbitrary on PBLKSIZ-N */
#define SPLTMAX	10			/* maximum allowed splits */
                                        /* for a single insertion */
#ifdef VMS
#define DIRFEXT	".sdbm_dir"
#else
#define DIRFEXT	".dir"
#endif
#define PAGFEXT	".pag"

typedef struct {
        int dirf;		       /* directory file descriptor */
        int pagf;		       /* page file descriptor */
        int flags;		       /* status/error flags, see below */
        long maxbno;		       /* size of dirfile in bits */
        long curbit;		       /* current bit number */
        long hmask;		       /* current hash mask */
        long blkptr;		       /* current block for nextkey */
        int keyptr;		       /* current key for nextkey */
        long blkno;		       /* current page to read/write */
        long pagbno;		       /* current page in pagbuf */
        char pagbuf[PBLKSIZ];	       /* page file block buffer */
        long dirbno;		       /* current block in dirbuf */
        char dirbuf[DBLKSIZ];	       /* directory file block buffer */
} DBM;

#define DBM_RDONLY	0x1	       /* data base open read-only */
#define DBM_IOERR	0x2	       /* data base I/O error */

/*
 * utility macros
 */
#define sdbm_rdonly(db)		((db)->flags & DBM_RDONLY)
#define sdbm_error(db)		((db)->flags & DBM_IOERR)

#define sdbm_clearerr(db)	((db)->flags &= ~DBM_IOERR)  /* ouch */

#define sdbm_dirfno(db)	((db)->dirf)
#define sdbm_pagfno(db)	((db)->pagf)

typedef struct {
        const char *dptr;
        int dsize;
} datum;

extern const datum nullitem;

/*
 * flags to sdbm_store
 */
#define DBM_INSERT	0
#define DBM_REPLACE	1

/*
 * ndbm interface
 */
extern DBM *sdbm_open(char *, int, int);
extern void sdbm_close(DBM *);
extern datum sdbm_fetch(DBM *, datum);
extern int sdbm_delete(DBM *, datum);
extern int sdbm_store(DBM *, datum, datum, int);
extern datum sdbm_firstkey(DBM *);
extern datum sdbm_nextkey(DBM *);
extern int sdbm_exists(DBM *, datum);

/*
 * other
 */
extern DBM *sdbm_prep(char *, char *, int, int);
extern long sdbm_hash(const char *, int);

#ifndef SDBM_ONLY
#define dbm_open sdbm_open
#define dbm_close sdbm_close
#define dbm_fetch sdbm_fetch
#define dbm_store sdbm_store
#define dbm_delete sdbm_delete
#define dbm_firstkey sdbm_firstkey
#define dbm_nextkey sdbm_nextkey
#define dbm_error sdbm_error
#define dbm_clearerr sdbm_clearerr
#endif

/* Most of the following is stolen from perl.h.  We don't include
   perl.h here because we just want the portability parts of perl.h,
   not everything else.
*/
#ifndef H_PERL  /* Include guard */
#include "embed.h"  /* Follow all the global renamings. */

/*
 * The following contortions are brought to you on behalf of all the
 * standards, semi-standards, de facto standards, not-so-de-facto standards
 * of the world, as well as all the other botches anyone ever thought of.
 * The basic theory is that if we work hard enough here, the rest of the
 * code can be a lot prettier.  Well, so much for theory.  Sorry, Henry...
 */

#include <errno.h>
#ifdef HAS_SOCKET
#   ifdef I_NET_ERRNO
#     include <net/errno.h>
#   endif
#endif

#include <stdio.h>
#include <ctype.h>
#include <setjmp.h>

#if defined(I_UNISTD)
#include <unistd.h>
#endif

#ifdef VMS
#  include <file.h>
#  include <unixio.h>
#endif

#ifdef I_SYS_PARAM
#   if !defined(WIN32) && !defined(VMS)
#       ifdef PARAM_NEEDS_TYPES
#	    include <sys/types.h>
#       endif
#       include <sys/param.h>
#   endif
#endif

#ifndef _TYPES_		/* If types.h defines this it's easy. */
#   ifndef major		/* Does everyone's types.h define this? */
#	include <sys/types.h>
#   endif
#endif

#include <sys/stat.h>

#ifndef SEEK_SET
# ifdef L_SET
#  define SEEK_SET	L_SET
# else
#  define SEEK_SET	0  /* Wild guess. */
# endif
#endif

/* Use all the "standard" definitions */
#include <stdlib.h>

#define MEM_SIZE Size_t

/* This comes after <stdlib.h> so we don't try to change the standard
 * library prototypes; we'll use our own instead. */

#if defined(MYMALLOC) && !defined(PERL_POLLUTE_MALLOC)
#  define malloc  Perl_malloc
#  define calloc  Perl_calloc
#  define realloc Perl_realloc
#  define free    Perl_mfree

#ifdef __cplusplus
extern "C" {
#endif

Malloc_t Perl_malloc(MEM_SIZE nbytes);
Malloc_t Perl_calloc(MEM_SIZE elements, MEM_SIZE size);
Malloc_t Perl_realloc(Malloc_t where, MEM_SIZE nbytes);
Free_t   Perl_mfree(Malloc_t where);

#ifdef __cplusplus
}
#endif

#endif /* MYMALLOC */

#include <string.h>

#define memzero(d,l) memset(d,0,l)

#ifdef BUGGY_MSC
#  pragma function(memcmp)
#endif

#define memNE(s1,s2,l) (memcmp(s1,s2,l))
#define memEQ(s1,s2,l) (!memcmp(s1,s2,l))

#ifdef I_NETINET_IN
#  ifdef VMS
#    include <in.h>
#  else
#    include <netinet/in.h>
#  endif
#endif

#endif /* Include guard */

#endif /* PERL_SDBM_FILE_SDBM_H_ */
