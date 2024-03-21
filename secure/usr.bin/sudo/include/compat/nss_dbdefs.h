/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef COMPAT_NSS_DBDEFS_H
#define COMPAT_NSS_DBDEFS_H

/*
 * Bits of nss_dbdefs.h and nss_common.h needed to implement
 * getgrouplist(3) using nss_search(3).
 *
 * HP-UX does not ship those headers so we need this compatibility header.
 * It may also work on other systems that use a Solaris-derived nsswitch
 * API.
 */

#ifdef NEED_HPUX_MUTEX
# include <synch.h>
#endif

typedef enum {
    NSS_SUCCESS,
    NSS_NOTFOUND,
    NSS_UNAVAIL,
    NSS_TRYAGAIN
} nss_status_t;

typedef struct nss_db_params {
    const char *name;
    const char *config_name;
    const char *default_config;
    unsigned int max_active_per_src;
    unsigned int max_dormant_per_src;  
    int flags;
    void *finders;
    void *private;
    void (*cleanup)(struct nss_db_params *);
} nss_db_params_t;

struct nss_groupsbymem {
    const char *username;
    gid_t *gid_array;
    int maxgids;
    int force_slow_way;
    int (*str2ent)(const char *instr, int instr_len, void *ent, char *buffer, int buflen);
    nss_status_t (*process_cstr)(const char *instr, int instr_len, struct nss_groupsbymem *);
    int numgids;
};

typedef struct {
    void *result;	/* group struct to fill in. */
    char *buffer;	/* string buffer for above */
    int buflen;		/* string buffer size */
} nss_XbyY_buf_t;

struct nss_db_state;
typedef struct {
    struct nss_db_state *s;
#ifdef NEED_HPUX_MUTEX
    lwp_mutex_t lock;
#endif
} nss_db_root_t;

#ifdef NEED_HPUX_MUTEX
# define NSS_DB_ROOT_INIT		{ 0, LWP_MUTEX_INITIALIZER }
#else
# define NSS_DB_ROOT_INIT		{ 0 }
#endif
#define DEFINE_NSS_DB_ROOT(name)	nss_db_root_t name = NSS_DB_ROOT_INIT

/* Backend function to find all groups a user belongs to for initgroups(). */
#define NSS_DBOP_GROUP_BYMEMBER		6

/* str2ent function return values */
#define NSS_STR_PARSE_SUCCESS	0
#define NSS_STR_PARSE_PARSE	1
#define NSS_STR_PARSE_ERANGE	2

/* Max length for an /etc/group file line. */
#define NSS_BUFLEN_GROUP		8192

/* HP-UX uses an extra underscore for these functions. */
#ifdef HAVE___NSS_INITF_GROUP
# define _nss_initf_group       __nss_initf_group
#endif
#ifdef HAVE___NSS_XBYY_BUF_ALLOC
# define _nss_XbyY_buf_alloc    __nss_XbyY_buf_alloc
# define _nss_XbyY_buf_free     __nss_XbyY_buf_free
#endif

typedef void (*nss_db_initf_t)(nss_db_params_t *);
extern nss_status_t nss_search(nss_db_root_t *, nss_db_initf_t, int search_fnum, void *search_args);
extern nss_XbyY_buf_t *_nss_XbyY_buf_alloc(int struct_size, int buffer_size);
extern void _nss_XbyY_buf_free(nss_XbyY_buf_t *);

#endif /* COMPAT_NSS_DBDEFS_H */
