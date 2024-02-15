/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2003-2023 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * This code is derived from software contributed by Aaron Spangler.
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#ifdef HAVE_LBER_H
# include <lber.h>
#endif
#include <ldap.h>
#if defined(HAVE_LDAPSSL_H)
# include <ldapssl.h>
#elif defined(HAVE_LDAP_SSL_H)
# include <ldap_ssl.h>
#elif defined(HAVE_MPS_LDAP_SSL_H)
# include <mps/ldap_ssl.h>
#endif
#ifdef HAVE_LDAP_SASL_INTERACTIVE_BIND_S
# ifdef HAVE_SASL_SASL_H
#  include <sasl/sasl.h>
# else
#  include <sasl.h>
# endif
#endif /* HAVE_LDAP_SASL_INTERACTIVE_BIND_S */

#include <sudoers.h>
#include <sudo_lbuf.h>
#include <sudo_ldap.h>
#include <sudo_ldap_conf.h>
#include <sudo_dso.h>

#if defined(HAVE_LDAP_SASL_INTERACTIVE_BIND_S) && !defined(LDAP_SASL_QUIET)
# define LDAP_SASL_QUIET	0
#endif

#ifndef HAVE_LDAP_UNBIND_EXT_S
#define ldap_unbind_ext_s(a, b, c)	ldap_unbind_s(a)
#endif

/* The TIMEFILTER_LENGTH is the length of the filter when timed entries
   are used. The length is computed as follows:
       81       for the filter itself
       + 2 * 17 for the now timestamp
*/
#define TIMEFILTER_LENGTH	115

/*
 * The ldap_search structure implements a linked list of ldap and
 * search result pointers, which allows us to remove them after
 * all search results have been combined in memory.
 */
struct ldap_search_result {
    STAILQ_ENTRY(ldap_search_result) entries;
    LDAP *ldap;
    LDAPMessage *searchresult;
};
STAILQ_HEAD(ldap_search_list, ldap_search_result);

/*
 * The ldap_entry_wrapper structure is used to implement sorted result entries.
 * A double is used for the order to allow for insertion of new entries
 * without having to renumber everything.
 * Note: there is no standard floating point type in LDAP.
 *       As a result, some LDAP servers will only allow an integer.
 */
struct ldap_entry_wrapper {
    LDAPMessage	*entry;
    double order;
};

/*
 * The ldap_result structure contains the list of matching searches as
 * well as an array of all result entries sorted by the sudoOrder attribute.
 */
struct ldap_result {
    struct ldap_search_list searches;
    struct ldap_entry_wrapper *entries;
    unsigned int allocated_entries;
    unsigned int nentries;
};
#define	ALLOCATION_INCREMENT	100

/*
 * The ldap_netgroup structure implements a singly-linked tail queue of
 * netgroups a user is a member of when querying netgroups directly.
 */
struct ldap_netgroup {
    STAILQ_ENTRY(ldap_netgroup) entries;
    char *name;
};
STAILQ_HEAD(ldap_netgroup_list, ldap_netgroup);

/*
 * LDAP sudo_nss handle.
 * We store the connection to the LDAP server and the passwd struct of the
 * user the last query was performed for.
 */
struct sudo_ldap_handle {
    LDAP *ld;
    struct passwd *pw;
    struct sudoers_parse_tree parse_tree;
};

#ifdef HAVE_LDAP_INITIALIZE
static char *
sudo_ldap_join_uri(struct ldap_config_str_list *uri_list)
{
    struct ldap_config_str *uri;
    size_t len = 0;
    char *buf = NULL;
    debug_decl(sudo_ldap_join_uri, SUDOERS_DEBUG_LDAP);

    STAILQ_FOREACH(uri, uri_list, entries) {
	if (ldap_conf.ssl_mode == SUDO_LDAP_STARTTLS) {
	    if (strncasecmp(uri->val, "ldaps://", 8) == 0) {
		sudo_warnx("%s", U_("starttls not supported when using ldaps"));
		ldap_conf.ssl_mode = SUDO_LDAP_SSL;
	    }
	}
	len += strlen(uri->val) + 1;
    }
    if (len == 0 || (buf = malloc(len)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    } else {
	char *cp = buf;

	STAILQ_FOREACH(uri, uri_list, entries) {
	    cp += strlcpy(cp, uri->val, len - (size_t)(cp - buf));
	    *cp++ = ' ';
	}
	cp[-1] = '\0';
    }
    debug_return_str(buf);
}
#endif /* HAVE_LDAP_INITIALIZE */

/*
 * Wrapper for ldap_create() or ldap_init() that handles
 * SSL/TLS initialization as well.
 * Returns LDAP_SUCCESS on success, else non-zero.
 */
static int
sudo_ldap_init(const struct sudoers_context *ctx, LDAP **ldp, const char *host,
    int port)
{
    LDAP *ld;
    int ret;
    debug_decl(sudo_ldap_init, SUDOERS_DEBUG_LDAP);

#ifdef HAVE_LDAPSSL_INIT
    if (ldap_conf.ssl_mode != SUDO_LDAP_CLEAR) {
	const int defsecure = ldap_conf.ssl_mode == SUDO_LDAP_SSL;
	DPRINTF2("ldapssl_clientauth_init(%s, %s)",
	    ldap_conf.tls_certfile ? ldap_conf.tls_certfile : "NULL",
	    ldap_conf.tls_keyfile ? ldap_conf.tls_keyfile : "NULL");
	ret = ldapssl_clientauth_init(ldap_conf.tls_certfile, NULL,
	    ldap_conf.tls_keyfile != NULL, ldap_conf.tls_keyfile, NULL);
	/*
	 * Starting with version 5.0, Mozilla-derived LDAP SDKs require
	 * the cert and key paths to be a directory, not a file.
	 * If the user specified a file and it fails, try the parent dir.
	 */
	if (ret != LDAP_SUCCESS) {
	    bool retry = false;
	    if (ldap_conf.tls_certfile != NULL) {
		char *cp = strrchr(ldap_conf.tls_certfile, '/');
		if (cp != NULL && strncmp(cp + 1, "cert", 4) == 0) {
		    *cp = '\0';
		    retry = true;
		}
	    }
	    if (ldap_conf.tls_keyfile != NULL) {
		char *cp = strrchr(ldap_conf.tls_keyfile, '/');
		if (cp != NULL && strncmp(cp + 1, "key", 3) == 0) {
		    *cp = '\0';
		    retry = true;
		}
	    }
	    if (retry) {
		DPRINTF2("retry ldapssl_clientauth_init(%s, %s)",
		    ldap_conf.tls_certfile ? ldap_conf.tls_certfile : "NULL",
		    ldap_conf.tls_keyfile ? ldap_conf.tls_keyfile : "NULL");
		ret = ldapssl_clientauth_init(ldap_conf.tls_certfile, NULL,
		    ldap_conf.tls_keyfile != NULL, ldap_conf.tls_keyfile, NULL);
	    }
	}
	if (ret != LDAP_SUCCESS) {
	    sudo_warnx(U_("unable to initialize SSL cert and key db: %s"),
		ldapssl_err2string(ret));
	    if (ldap_conf.tls_certfile == NULL)
		sudo_warnx(U_("you must set TLS_CERT in %s to use SSL"),
		    ctx->settings.ldap_conf);
	    goto done;
	}

	DPRINTF2("ldapssl_init(%s, %d, %d)", host, port, defsecure);
	if ((ld = ldapssl_init(host, port, defsecure)) != NULL)
	    ret = LDAP_SUCCESS;
    } else
#elif defined(HAVE_LDAP_SSL_INIT) && defined(HAVE_LDAP_SSL_CLIENT_INIT)
    if (ldap_conf.ssl_mode == SUDO_LDAP_SSL) {
	int sslrc;
	ret = ldap_ssl_client_init(ldap_conf.tls_keyfile, ldap_conf.tls_keypw,
	    0, &sslrc);
	if (ret != LDAP_SUCCESS) {
	    sudo_warnx("ldap_ssl_client_init(): %s: %s",
		ldap_err2string(ret), ssl_err2string(sslrc));
	    goto done;
	}
	DPRINTF2("ldap_ssl_init(%s, %d, NULL)", host, port);
	if ((ld = ldap_ssl_init((char *)host, port, NULL)) != NULL)
	    ret = LDAP_SUCCESS;
    } else
#endif
    {
#ifdef HAVE_LDAP_CREATE
	DPRINTF2("ldap_create()");
	if ((ret = ldap_create(&ld)) != LDAP_SUCCESS)
	    goto done;
	DPRINTF2("ldap_set_option(LDAP_OPT_HOST_NAME, %s)", host);
	ret = ldap_set_option(ld, LDAP_OPT_HOST_NAME, host);
#else
	DPRINTF2("ldap_init(%s, %d)", host, port);
	if ((ld = ldap_init((char *)host, port)) == NULL) {
	    ret = LDAP_LOCAL_ERROR;
	    goto done;
	}
	ret = LDAP_SUCCESS;
#endif
    }

    *ldp = ld;
done:
    debug_return_int(ret);
}

/*
 * Wrapper for ldap_get_values_len() that fills in the response code
 * on error.
 */
static struct berval **
sudo_ldap_get_values_len(LDAP *ld, LDAPMessage *entry, const char *attr, int *rc)
{
    struct berval **bval;

    bval = ldap_get_values_len(ld, entry, attr);
    if (bval == NULL) {
	const int optrc = ldap_get_option(ld, LDAP_OPT_RESULT_CODE, rc);
	if (optrc != LDAP_OPT_SUCCESS)
	    *rc = optrc;
    } else {
	*rc = LDAP_SUCCESS;
    }
    return bval;
}

/*
 * Walk through search results and return true if we have a matching
 * non-Unix group (including netgroups), else false.
 * A matching entry that is negated will always return false.
 */
static int
sudo_ldap_check_non_unix_group(struct sudoers_context *ctx,
    const struct sudo_nss *nss, LDAPMessage *entry, struct passwd *pw)
{
    struct sudo_ldap_handle *handle = nss->handle;
    LDAP *ld = handle->ld;
    struct berval **bv, **p;
    bool ret = false;
    int rc;
    debug_decl(sudo_ldap_check_non_unix_group, SUDOERS_DEBUG_LDAP);

    if (!entry)
	debug_return_bool(false);

    /* get the values from the entry */
    bv = sudo_ldap_get_values_len(ld, entry, "sudoUser", &rc);
    if (bv == NULL) {
	if (rc == LDAP_NO_MEMORY)
	    debug_return_int(-1);
	debug_return_bool(false);
    }

    /* walk through values */
    for (p = bv; *p != NULL && !ret; p++) {
	bool negated = false;
	const char *val = (*p)->bv_val;

	if (*val == '!') {
	    val++;
	    negated = true;
	}
	if (*val == '+') {
	    if (netgr_matches(nss, val,
		def_netgroup_tuple ? ctx->runas.host : NULL,
		def_netgroup_tuple ? ctx->runas.shost : NULL, pw->pw_name) == ALLOW)
		ret = true;
	    DPRINTF2("ldap sudoUser netgroup '%s%s' ... %s",
		negated ? "!" : "", val, ret ? "MATCH!" : "not");
	} else {
	    if (group_plugin_query(pw->pw_name, val + 2, pw))
		ret = true;
	    DPRINTF2("ldap sudoUser non-Unix group '%s%s' ... %s",
		negated ? "!" : "", val, ret ? "MATCH!" : "not");
	}
	/* A negated match overrides all other entries. */
	if (ret && negated) {
	    ret = false;
	    break;
	}
    }

    ldap_value_free_len(bv);	/* cleanup */

    debug_return_bool(ret);
}

/*
 * Extract the dn from an entry and return the first rdn from it.
 */
static char *
sudo_ldap_get_first_rdn(LDAP *ld, LDAPMessage *entry, int *rc)
{
#ifdef HAVE_LDAP_STR2DN
    char *dn, *rdn = NULL;
    LDAPDN tmpDN;
    debug_decl(sudo_ldap_get_first_rdn, SUDOERS_DEBUG_LDAP);

    if ((dn = ldap_get_dn(ld, entry)) == NULL) {
	int optrc = ldap_get_option(ld, LDAP_OPT_RESULT_CODE, rc);
	if (optrc != LDAP_OPT_SUCCESS)
	    *rc = optrc;
	debug_return_str(NULL);
    }
    *rc = ldap_str2dn(dn, &tmpDN, LDAP_DN_FORMAT_LDAP);
    if (*rc == LDAP_SUCCESS) {
	ldap_rdn2str(tmpDN[0], &rdn, LDAP_DN_FORMAT_UFN);
	ldap_dnfree(tmpDN);
    }
    ldap_memfree(dn);
    debug_return_str(rdn);
#else
    char *dn, **edn;
    debug_decl(sudo_ldap_get_first_rdn, SUDOERS_DEBUG_LDAP);

    if ((dn = ldap_get_dn(ld, entry)) == NULL) {
	int optrc = ldap_get_option(ld, LDAP_OPT_RESULT_CODE, rc);
	if (optrc != LDAP_OPT_SUCCESS)
	    *rc = optrc;
	debug_return_str(NULL);
    }
    edn = ldap_explode_dn(dn, 1);
    ldap_memfree(dn);
    if (edn == NULL) {
	*rc = LDAP_NO_MEMORY;
	debug_return_str(NULL);
    }
    *rc = LDAP_SUCCESS;
    debug_return_str(edn[0]);
#endif
}

/*
 * Read sudoOption and fill in the defaults list.
 * This is used to parse the cn=defaults entry.
 */
static bool
sudo_ldap_parse_options(LDAP *ld, LDAPMessage *entry, struct defaults_list *defs)
{
    struct berval **p, **bv = NULL;
    char *cp, *cn = NULL, *source = NULL;
    bool ret = false;
    int rc;
    debug_decl(sudo_ldap_parse_options, SUDOERS_DEBUG_LDAP);

    bv = sudo_ldap_get_values_len(ld, entry, "sudoOption", &rc);
    if (bv == NULL) {
	if (rc == LDAP_NO_MEMORY)
	    goto oom;
	debug_return_bool(true);
    }

    /* Use sudoRole in place of file name in defaults. */
    cn = sudo_ldap_get_first_rdn(ld, entry, &rc);
    if (cn == NULL) {
	if (rc == LDAP_NO_MEMORY)
	    goto oom;
    }
    if (asprintf(&cp, "sudoRole %s", cn ? cn : "UNKNOWN") == -1)
	goto oom;
    source = sudo_rcstr_dup(cp);
    free(cp);
    if (source == NULL)
	goto oom;

    /* Walk through options, appending to defs. */
    for (p = bv; *p != NULL; p++) {
	char *var, *val;
	int op;

	op = sudo_ldap_parse_option((*p)->bv_val, &var, &val);
	if (!append_default(var, val, op, source, defs))
	    goto oom;
    }

    ret = true;
    goto done;

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

done:
    sudo_rcstr_delref(source);
    if (cn)
	ldap_memfree(cn);
    ldap_value_free_len(bv);

    debug_return_bool(ret);
}

/*
 * Build an LDAP timefilter.
 *
 * Stores a filter in the buffer that makes sure only entries
 * are selected that have a sudoNotBefore in the past and a
 * sudoNotAfter in the future, i.e. a filter of the following
 * structure (spaced out a little more for better readability:
 *
 * (&
 *   (|
 *	(!(sudoNotAfter=*))
 *	(sudoNotAfter>__now__)
 *   )
 *   (|
 *	(!(sudoNotBefore=*))
 *	(sudoNotBefore<__now__)
 *   )
 * )
 *
 * If either the sudoNotAfter or sudoNotBefore attributes are missing,
 * no time restriction shall be imposed.
 */
static bool
sudo_ldap_timefilter(char *buffer, size_t buffersize)
{
    char timebuffer[sizeof("20120727121554.0Z")];
    bool ret = false;
    struct tm gmt;
    time_t now;
    size_t tblen;
    int buflen;
    debug_decl(sudo_ldap_timefilter, SUDOERS_DEBUG_LDAP);

    /* Make sure we have a formatted timestamp for __now__. */
    time(&now);
    if (gmtime_r(&now, &gmt) == NULL) {
	sudo_warn("%s", U_("unable to get GMT time"));
	goto done;
    }

    /* Format the timestamp according to the RFC. */
    timebuffer[sizeof(timebuffer) - 1] = '\0';
    tblen = strftime(timebuffer, sizeof(timebuffer), "%Y%m%d%H%M%S.0Z", &gmt);
    if (tblen == 0 || timebuffer[sizeof(timebuffer) - 1] != '\0') {
	sudo_warnx("%s", U_("unable to format timestamp"));
	goto done;
    }

    /* Build filter. */
    buflen = snprintf(buffer, buffersize, "(&(|(!(sudoNotAfter=*))(sudoNotAfter>=%s))(|(!(sudoNotBefore=*))(sudoNotBefore<=%s)))",
	timebuffer, timebuffer);
    if (buflen < 0 || (size_t)buflen >= buffersize) {
	sudo_warnx(U_("internal error, %s overflow"), __func__);
	errno = EOVERFLOW;
	goto done;
    }

    ret = true;

done:
    debug_return_bool(ret);
}

/*
 * Builds up a filter to search for default settings
 */
static char *
sudo_ldap_build_default_filter(void)
{
    char *filt;
    debug_decl(sudo_ldap_build_default_filter, SUDOERS_DEBUG_LDAP);

    if (!ldap_conf.search_filter)
	debug_return_str(strdup("cn=defaults"));

    if (asprintf(&filt, "(&%s(cn=defaults))", ldap_conf.search_filter) == -1)
	debug_return_str(NULL);

    debug_return_str(filt);
}

/*
 * Check the netgroups list beginning at "start" for nesting.
 * Parent nodes with a memberNisNetgroup that match one of the
 * netgroups are added to the list and checked for further nesting.
 * Return true on success or false if there was an internal overflow.
 */
static bool
sudo_netgroup_lookup_nested(struct sudoers_context *ctx, LDAP *ld, char *base,
    struct timeval *timeout, struct ldap_netgroup_list *netgroups,
    struct ldap_netgroup *start)
{
    LDAPMessage *entry, *result;
    size_t filt_len;
    char *filt;
    int rc;
    debug_decl(sudo_netgroup_lookup_nested, SUDOERS_DEBUG_LDAP);

    DPRINTF1("Checking for nested netgroups from netgroup_base '%s'", base);
    do {
	struct ldap_netgroup *ng, *old_tail;

	result = NULL;
	old_tail = STAILQ_LAST(netgroups, ldap_netgroup, entries);
	filt_len = strlen(ldap_conf.netgroup_search_filter) + 7;
	for (ng = start; ng != NULL; ng = STAILQ_NEXT(ng, entries)) {
	    filt_len += sudo_ldap_value_len(ng->name) + 20;
	}
	if ((filt = malloc(filt_len)) == NULL)
	    goto oom;
	CHECK_STRLCPY(filt, "(&", filt_len);
	CHECK_STRLCAT(filt, ldap_conf.netgroup_search_filter, filt_len);
	CHECK_STRLCAT(filt, "(|", filt_len);
	for (ng = start; ng != NULL; ng = STAILQ_NEXT(ng, entries)) {
	    CHECK_STRLCAT(filt, "(memberNisNetgroup=", filt_len);
	    CHECK_LDAP_VCAT(filt, ng->name, filt_len);
	    CHECK_STRLCAT(filt, ")", filt_len);
	}
	CHECK_STRLCAT(filt, "))", filt_len);
	DPRINTF1("ldap netgroup search filter: '%s'", filt);
	rc = ldap_search_ext_s(ld, base, LDAP_SCOPE_SUBTREE, filt,
	    NULL, 0, NULL, NULL, timeout, 0, &result);
	free(filt);
	if (rc == LDAP_SUCCESS) {
	    LDAP_FOREACH(entry, ld, result) {
		struct berval **bv;

		bv = sudo_ldap_get_values_len(ld, entry, "cn", &rc);
		if (bv == NULL) {
		    if (rc == LDAP_NO_MEMORY)
			goto oom;
		} else {
		    /* Don't add a netgroup twice. */
		    STAILQ_FOREACH(ng, netgroups, entries) {
			/* Assumes only one cn per entry. */
			if (strcasecmp(ng->name, (*bv)->bv_val) == 0)
			    break;
		    }
		    if (ng == NULL) {
			ng = malloc(sizeof(*ng));
			if (ng == NULL ||
			    (ng->name = strdup((*bv)->bv_val)) == NULL) {
			    free(ng);
			    ldap_value_free_len(bv);
			    goto oom;
			}
#ifdef __clang_analyzer__
			/* clang analyzer false positive */
			if (__builtin_expect(netgroups->stqh_last == NULL, 0))
			    __builtin_trap();
#endif
			STAILQ_INSERT_TAIL(netgroups, ng, entries);
			DPRINTF1("Found new netgroup %s for %s", ng->name, base);
		    }
		    ldap_value_free_len(bv);
		}
	    }
	}
	ldap_msgfree(result);

	/* Check for nested netgroups in what we added. */
	start = old_tail ? STAILQ_NEXT(old_tail, entries) : STAILQ_FIRST(netgroups);
    } while (start != NULL);

    debug_return_bool(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    ldap_msgfree(result);
    debug_return_bool(false);
overflow:
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    free(filt);
    debug_return_bool(false);
}

/*
 * Look up netgroups that the specified user is a member of.
 * Appends new entries to the netgroups list.
 * Return true on success or false if there was an internal overflow.
 */
static bool
sudo_netgroup_lookup(struct sudoers_context *ctx, LDAP *ld, struct passwd *pw,
    struct ldap_netgroup_list *netgroups)
{
    struct ldap_config_str *base;
    struct ldap_netgroup *ng, *old_tail;
    struct timeval tv, *tvp = NULL;
    LDAPMessage *entry, *result = NULL;
    const char *domain;
    char *escaped_domain = NULL, *escaped_user = NULL;
    char *escaped_host = NULL, *escaped_shost = NULL, *filt = NULL;
    int filt_len, rc;
    bool ret = false;
    debug_decl(sudo_netgroup_lookup, SUDOERS_DEBUG_LDAP);

    if (ldap_conf.timeout > 0) {
	tv.tv_sec = ldap_conf.timeout;
	tv.tv_usec = 0;
	tvp = &tv;
    }

    /* Use NIS domain if set, else wildcard match. */
    domain = sudo_getdomainname();

    /* Escape the domain, host names, and user name per RFC 4515. */
    if (domain != NULL) {
	if ((escaped_domain = sudo_ldap_value_dup(domain)) == NULL)
	    goto oom;
    }
    if ((escaped_user = sudo_ldap_value_dup(pw->pw_name)) == NULL)
	    goto oom;
    if (def_netgroup_tuple) {
	escaped_host = sudo_ldap_value_dup(ctx->runas.host);
	if (ctx->runas.host == ctx->runas.shost)
	    escaped_shost = escaped_host;
	else
	    escaped_shost = sudo_ldap_value_dup(ctx->runas.shost);
	if (escaped_host == NULL || escaped_shost == NULL)
	    goto oom;
    }

    /* Build query, using NIS domain if it is set. */
    if (domain != NULL) {
	if (escaped_host != escaped_shost) {
	    filt_len = asprintf(&filt, "(&%s(|"
		"(nisNetgroupTriple=\\28,%s,%s\\29)"
		"(nisNetgroupTriple=\\28%s,%s,%s\\29)"
		"(nisNetgroupTriple=\\28%s,%s,%s\\29)"
		"(nisNetgroupTriple=\\28,%s,\\29)"
		"(nisNetgroupTriple=\\28%s,%s,\\29)"
		"(nisNetgroupTriple=\\28%s,%s,\\29)))",
		ldap_conf.netgroup_search_filter, escaped_user, escaped_domain,
		escaped_shost, escaped_user, escaped_domain,
		escaped_host, escaped_user, escaped_domain, escaped_user,
		escaped_shost, escaped_user, escaped_host, escaped_user);
	} else if (escaped_shost != NULL) {
	    filt_len = asprintf(&filt, "(&%s(|"
		"(nisNetgroupTriple=\\28,%s,%s\\29)"
		"(nisNetgroupTriple=\\28%s,%s,%s\\29)"
		"(nisNetgroupTriple=\\28,%s,\\29)"
		"(nisNetgroupTriple=\\28%s,%s,\\29)))",
		ldap_conf.netgroup_search_filter, escaped_user, escaped_domain,
		escaped_shost, escaped_user, escaped_domain,
		escaped_user, escaped_shost, escaped_user);
	} else {
	    filt_len = asprintf(&filt, "(&%s(|"
		"(nisNetgroupTriple=\\28*,%s,%s\\29)"
		"(nisNetgroupTriple=\\28*,%s,\\29)))",
		ldap_conf.netgroup_search_filter, escaped_user, escaped_domain,
		escaped_user);
	}
    } else {
	if (escaped_host != escaped_shost) {
	    filt_len = asprintf(&filt, "(&%s(|"
		"(nisNetgroupTriple=\\28,%s,*\\29)"
		"(nisNetgroupTriple=\\28%s,%s,*\\29)"
		"(nisNetgroupTriple=\\28%s,%s,*\\29)))",
		ldap_conf.netgroup_search_filter, escaped_user,
		escaped_shost, escaped_user, escaped_host, escaped_user);
	} else if (escaped_shost != NULL) {
	    filt_len = asprintf(&filt, "(&%s(|"
		"(nisNetgroupTriple=\\28,%s,*\\29)"
		"(nisNetgroupTriple=\\28%s,%s,*\\29)))",
		ldap_conf.netgroup_search_filter, escaped_user,
		escaped_shost, escaped_user);
	} else {
	    filt_len = asprintf(&filt,
		"(&%s(|(nisNetgroupTriple=\\28*,%s,*\\29)))",
		ldap_conf.netgroup_search_filter, escaped_user);
	}
    }
    if (filt_len == -1)
	goto oom;
    DPRINTF1("ldap netgroup search filter: '%s'", filt);

    STAILQ_FOREACH(base, &ldap_conf.netgroup_base, entries) {
	DPRINTF1("searching from netgroup_base '%s'", base->val);
	rc = ldap_search_ext_s(ld, base->val, LDAP_SCOPE_SUBTREE, filt,
	    NULL, 0, NULL, NULL, tvp, 0, &result);
	if (rc != LDAP_SUCCESS) {
	    DPRINTF1("ldap netgroup search failed: %s", ldap_err2string(rc));
	    ldap_msgfree(result);
	    result = NULL;
	    continue;
	}

	old_tail = STAILQ_LAST(netgroups, ldap_netgroup, entries);
	LDAP_FOREACH(entry, ld, result) {
	    struct berval **bv;

	    bv = sudo_ldap_get_values_len(ld, entry, "cn", &rc);
	    if (bv == NULL) {
		if (rc == LDAP_NO_MEMORY)
		    goto oom;
	    } else {
		/* Don't add a netgroup twice. */
		STAILQ_FOREACH(ng, netgroups, entries) {
		    /* Assumes only one cn per entry. */
		    if (strcasecmp(ng->name, (*bv)->bv_val) == 0)
			break;
		}
		if (ng == NULL) {
		    ng = malloc(sizeof(*ng));
		    if (ng == NULL ||
			(ng->name = strdup((*bv)->bv_val)) == NULL) {
			free(ng);
			ldap_value_free_len(bv);
			goto oom;
		    }
		    STAILQ_INSERT_TAIL(netgroups, ng, entries);
		    DPRINTF1("Found new netgroup %s for %s", ng->name,
			base->val);
		}
		ldap_value_free_len(bv);
	    }
	}
	ldap_msgfree(result);
	result = NULL;

	/* Check for nested netgroups in what we added. */
	ng = old_tail ? STAILQ_NEXT(old_tail, entries) : STAILQ_FIRST(netgroups);
	if (ng != NULL) {
	    if (!sudo_netgroup_lookup_nested(ctx, ld, base->val, tvp, netgroups, ng))
		goto done;
	}
    }
    ret = true;
    goto done;

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
done:
    free(escaped_domain);
    free(escaped_user);
    free(escaped_host);
    if (escaped_host != escaped_shost)
	free(escaped_shost);
    free(filt);
    ldap_msgfree(result);
    debug_return_bool(ret);
}

/*
 * Builds up a filter to check against LDAP.
 */
static char *
sudo_ldap_build_pass1(struct sudoers_context *ctx, LDAP *ld, struct passwd *pw)
{
    char idbuf[STRLEN_MAX_UNSIGNED(uid_t) + 1];
    char timebuffer[TIMEFILTER_LENGTH + 1];
    char *buf, *notbuf;
    struct ldap_netgroup_list netgroups;
    struct ldap_netgroup *ng = NULL;
    struct gid_list *gidlist;
    struct group_list *grlist;
    struct group *grp;
    size_t sz = 0;
    int i;
    debug_decl(sudo_ldap_build_pass1, SUDOERS_DEBUG_LDAP);

    STAILQ_INIT(&netgroups);

    if (ldap_conf.timed || ldap_conf.search_filter) {
	/* Allocate space for the global AND. */
	sz += 3;

	/* Add LDAP search filter if present. */
	if (ldap_conf.search_filter)
	    sz += strlen(ldap_conf.search_filter);

	/* If timed, add space for time limits. */
	if (ldap_conf.timed)
	    sz += TIMEFILTER_LENGTH;
    }

    /* Add space for the global OR clause + (sudoUser=ALL) + NOT + NUL. */
    sz += sizeof("(|(sudoUser=ALL)(!(|)))");

    /* Add space for username and uid, including the negated versions. */
    sz += ((sizeof("(sudoUser=)(sudoUser=#)") - 1 +
	sudo_ldap_value_len(pw->pw_name) + sizeof(idbuf) - 1) * 2) + 2;

    /* Add space for primary and supplementary groups and gids */
    if ((grp = sudo_getgrgid(pw->pw_gid)) != NULL) {
	sz += ((sizeof("(sudoUser=%)") - 1 +
	    sudo_ldap_value_len(grp->gr_name)) * 2) + 1;
    }
    sz += ((sizeof("(sudoUser=%#)") - 1 + sizeof(idbuf) - 1) * 2) + 1;
    if ((grlist = sudo_get_grlist(pw)) != NULL) {
	for (i = 0; i < grlist->ngroups; i++) {
	    if (grp != NULL && strcasecmp(grlist->groups[i], grp->gr_name) == 0)
		continue;
	    sz += ((sizeof("(sudoUser=%)") - 1 +
		sudo_ldap_value_len(grlist->groups[i])) * 2) + 1;
	}
    }
    if ((gidlist = sudo_get_gidlist(pw, ENTRY_TYPE_ANY)) != NULL) {
	for (i = 0; i < gidlist->ngids; i++) {
	    if (pw->pw_gid == gidlist->gids[i])
		continue;
	    sz += ((sizeof("(sudoUser=%#)") - 1 + sizeof(idbuf) - 1) * 2) + 1;
	}
    }

    /* Add space for user netgroups if netgroup_base specified. */
    if (ldap_conf.netgroup_query) {
	DPRINTF1("Looking up netgroups for %s", pw->pw_name);
	if (sudo_netgroup_lookup(ctx, ld, pw, &netgroups)) {
	    STAILQ_FOREACH(ng, &netgroups, entries) {
		sz += ((sizeof("(sudoUser=+)") - 1 + strlen(ng->name)) * 2) + 1;
	    }
	} else {
	    /* sudo_netgroup_lookup() failed, clean up. */
	    while ((ng = STAILQ_FIRST(&netgroups)) != NULL) {
		STAILQ_REMOVE_HEAD(&netgroups, entries);
		free(ng->name);
		free(ng);
	    }
	}
    }

    buf = malloc(sz);
    notbuf = malloc(sz);
    if (buf == NULL || notbuf == NULL)
	goto bad;
    *buf = '\0';
    *notbuf = '\0';

    /*
     * If timed or using a search filter, start a global AND clause to
     * contain the search filter, search criteria, and time restriction.
     */
    if (ldap_conf.timed || ldap_conf.search_filter)
	CHECK_STRLCPY(buf, "(&", sz);

    if (ldap_conf.search_filter)
	CHECK_STRLCAT(buf, ldap_conf.search_filter, sz);

    /* Global OR + sudoUser=user_name filter */
    CHECK_STRLCAT(buf, "(|(sudoUser=", sz);
    CHECK_LDAP_VCAT(buf, pw->pw_name, sz);
    CHECK_STRLCAT(buf, ")", sz);
    CHECK_STRLCAT(notbuf, "(sudoUser=!", sz);
    CHECK_LDAP_VCAT(notbuf, pw->pw_name, sz);
    CHECK_STRLCAT(notbuf, ")", sz);

    /* Append user-ID */
    (void) snprintf(idbuf, sizeof(idbuf), "%u", (unsigned int)pw->pw_uid);
    CHECK_STRLCAT(buf, "(sudoUser=#", sz);
    CHECK_STRLCAT(buf, idbuf, sz);
    CHECK_STRLCAT(buf, ")", sz);
    CHECK_STRLCAT(notbuf, "(sudoUser=!#", sz);
    CHECK_STRLCAT(notbuf, idbuf, sz);
    CHECK_STRLCAT(notbuf, ")", sz);

    /* Append primary group and group-ID */
    if (grp != NULL) {
	CHECK_STRLCAT(buf, "(sudoUser=%", sz);
	CHECK_LDAP_VCAT(buf, grp->gr_name, sz);
	CHECK_STRLCAT(buf, ")", sz);
	CHECK_STRLCAT(notbuf, "(sudoUser=!%", sz);
	CHECK_LDAP_VCAT(notbuf, grp->gr_name, sz);
	CHECK_STRLCAT(notbuf, ")", sz);
    }
    (void) snprintf(idbuf, sizeof(idbuf), "%u", (unsigned int)pw->pw_gid);
    CHECK_STRLCAT(buf, "(sudoUser=%#", sz);
    CHECK_STRLCAT(buf, idbuf, sz);
    CHECK_STRLCAT(buf, ")", sz);
    CHECK_STRLCAT(notbuf, "(sudoUser=!%#", sz);
    CHECK_STRLCAT(notbuf, idbuf, sz);
    CHECK_STRLCAT(notbuf, ")", sz);

    /* Append supplementary groups and group-IDs */
    if (grlist != NULL) {
	for (i = 0; i < grlist->ngroups; i++) {
	    if (grp != NULL && strcasecmp(grlist->groups[i], grp->gr_name) == 0)
		continue;
	    CHECK_STRLCAT(buf, "(sudoUser=%", sz);
	    CHECK_LDAP_VCAT(buf, grlist->groups[i], sz);
	    CHECK_STRLCAT(buf, ")", sz);
	    CHECK_STRLCAT(notbuf, "(sudoUser=!%", sz);
	    CHECK_LDAP_VCAT(notbuf, grlist->groups[i], sz);
	    CHECK_STRLCAT(notbuf, ")", sz);
	}
    }
    if (gidlist != NULL) {
	for (i = 0; i < gidlist->ngids; i++) {
	    if (pw->pw_gid == gidlist->gids[i])
		continue;
	    (void) snprintf(idbuf, sizeof(idbuf), "%u",
		(unsigned int)gidlist->gids[i]);
	    CHECK_STRLCAT(buf, "(sudoUser=%#", sz);
	    CHECK_STRLCAT(buf, idbuf, sz);
	    CHECK_STRLCAT(buf, ")", sz);
	    CHECK_STRLCAT(notbuf, "(sudoUser=!%#", sz);
	    CHECK_STRLCAT(notbuf, idbuf, sz);
	    CHECK_STRLCAT(notbuf, ")", sz);
	}
    }

    /* Done with groups. */
    if (gidlist != NULL)
	sudo_gidlist_delref(gidlist);
    if (grlist != NULL)
	sudo_grlist_delref(grlist);
    if (grp != NULL)
	sudo_gr_delref(grp);

    /* Add netgroups (if any), freeing the list as we go. */
    while ((ng = STAILQ_FIRST(&netgroups)) != NULL) {
	STAILQ_REMOVE_HEAD(&netgroups, entries);
	CHECK_STRLCAT(buf, "(sudoUser=+", sz);
	CHECK_LDAP_VCAT(buf, ng->name, sz);
	CHECK_STRLCAT(buf, ")", sz);
	CHECK_STRLCAT(notbuf, "(sudoUser=!+", sz);
	CHECK_LDAP_VCAT(notbuf, ng->name, sz);
	CHECK_STRLCAT(notbuf, ")", sz);
	free(ng->name);
	free(ng);
    }

    /* Add ALL to list. */
    CHECK_STRLCAT(buf, "(sudoUser=ALL))", sz);

    /* Add filter for negated entries. */
    CHECK_STRLCAT(buf, "(!(|", sz);
    CHECK_STRLCAT(buf, notbuf, sz);
    CHECK_STRLCAT(buf, ")", sz);

    /* Add the time restriction, or simply end the global OR. */
    if (ldap_conf.timed) {
	CHECK_STRLCAT(buf, ")", sz); /* closes the global OR */
	if (!sudo_ldap_timefilter(timebuffer, sizeof(timebuffer)))
	    goto bad;
	CHECK_STRLCAT(buf, timebuffer, sz);
    } else if (ldap_conf.search_filter) {
	CHECK_STRLCAT(buf, ")", sz); /* closes the global OR */
    }

    CHECK_STRLCAT(buf, ")", sz); /* closes the global OR or the global AND */

    free(notbuf);
    debug_return_str(buf);
overflow:
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    if (ng != NULL) {
	/* Overflow while traversing netgroups. */
	free(ng->name);
	free(ng);
    }
    errno = EOVERFLOW;
bad:
    while ((ng = STAILQ_FIRST(&netgroups)) != NULL) {
	STAILQ_REMOVE_HEAD(&netgroups, entries);
	free(ng->name);
	free(ng);
    }
    free(buf);
    free(notbuf);
    debug_return_str(NULL);
}

/*
 * Builds up a filter to check against non-Unix group
 * entries in LDAP, including netgroups.
 */
static char *
sudo_ldap_build_pass2(void)
{
    char *filt, timebuffer[TIMEFILTER_LENGTH + 1];
    bool query_netgroups = def_use_netgroups;
    int len;
    debug_decl(sudo_ldap_build_pass2, SUDOERS_DEBUG_LDAP);

    /*
     * If we can query nisNetgroupTriple using netgroup_base, there is
     * no need to match all netgroups in pass 2.  If netgroups are not
     * natively supported, netgroup_base must be set.
     */
    if (ldap_conf.netgroup_query)
	query_netgroups = false;
#ifndef HAVE_INNETGR
    else if (STAILQ_EMPTY(&ldap_conf.netgroup_base))
	query_netgroups = false;
#endif

    /* Short circuit if no netgroups and no non-Unix groups. */
    if (!query_netgroups && !def_group_plugin) {
	errno = ENOENT;
	debug_return_str(NULL);
    }

    if (ldap_conf.timed) {
	if (!sudo_ldap_timefilter(timebuffer, sizeof(timebuffer)))
	    debug_return_str(NULL);
    }

    /*
     * Match all sudoUsers beginning with '+' or '%:'.
     * If a search filter or time restriction is specified,
     * those get ANDed in to the expression.
     */
    if (query_netgroups && def_group_plugin) {
	len = asprintf(&filt, "%s%s(|(sudoUser=+*)(sudoUser=!+*)(sudoUser=%%:*)(sudoUser=!%%:*))%s%s",
	    (ldap_conf.timed || ldap_conf.search_filter) ? "(&" : "",
	    ldap_conf.search_filter ? ldap_conf.search_filter : "",
	    ldap_conf.timed ? timebuffer : "",
	    (ldap_conf.timed || ldap_conf.search_filter) ? ")" : "");
    } else {
	len = asprintf(&filt, "%s%s(|(sudoUser=%s*)(sudoUser=!%s*))%s%s",
	    (ldap_conf.timed || ldap_conf.search_filter) ? "(&" : "",
	    ldap_conf.search_filter ? ldap_conf.search_filter : "",
	    query_netgroups ? "+" : "%:", query_netgroups ? "+" : "%:",
	    ldap_conf.timed ? timebuffer : "",
	    (ldap_conf.timed || ldap_conf.search_filter) ? ")" : "");
    }
    if (len == -1)
	filt = NULL;

    debug_return_str(filt);
}

static char *
berval_iter(void **vp)
{
    struct berval **bv = *vp;

    *vp = bv + 1;
    return *bv ? (*bv)->bv_val : NULL;
}

/*
 * Wrapper for sudo_ldap_role_to_priv() that takes an LDAPMessage.
 * Returns a struct privilege on success or NULL on failure.
 */
static struct privilege *
ldap_entry_to_priv(LDAP *ld, LDAPMessage *entry, int *rc_out)
{
    struct berval **cmnds = NULL, **hosts = NULL;
    struct berval **runasusers = NULL, **runasgroups = NULL;
    struct berval **opts = NULL, **notbefore = NULL, **notafter = NULL;
    struct privilege *priv = NULL;
    char *cn = NULL;
    int rc;
    debug_decl(ldap_entry_to_priv, SUDOERS_DEBUG_LDAP);

    /* Ignore sudoRole without sudoCommand or sudoHost. */
    cmnds = sudo_ldap_get_values_len(ld, entry, "sudoCommand", &rc);
    if (cmnds == NULL)
	goto cleanup;
    hosts = sudo_ldap_get_values_len(ld, entry, "sudoHost", &rc);
    if (hosts == NULL)
	goto cleanup;

    /* Get the entry's dn for long format printing. */
    if ((cn = sudo_ldap_get_first_rdn(ld, entry, &rc)) == NULL)
	goto cleanup;

    /* Get sudoRunAsUser / sudoRunAsGroup */
    runasusers = sudo_ldap_get_values_len(ld, entry, "sudoRunAsUser", &rc);
    if (runasusers == NULL) {
	if (rc != LDAP_NO_MEMORY)
	    runasusers = sudo_ldap_get_values_len(ld, entry, "sudoRunAs", &rc);
	if (rc == LDAP_NO_MEMORY)
	    goto cleanup;
    }
    runasgroups = sudo_ldap_get_values_len(ld, entry, "sudoRunAsGroup", &rc);
    if (rc == LDAP_NO_MEMORY)
	goto cleanup;

    /* Get sudoNotBefore / sudoNotAfter */
    if (ldap_conf.timed) {
	notbefore = sudo_ldap_get_values_len(ld, entry, "sudoNotBefore", &rc);
	if (rc == LDAP_NO_MEMORY)
	    goto cleanup;
	notafter = sudo_ldap_get_values_len(ld, entry, "sudoNotAfter", &rc);
	if (rc == LDAP_NO_MEMORY)
	    goto cleanup;
    }

    /* Parse sudoOptions. */
    opts = sudo_ldap_get_values_len(ld, entry, "sudoOption", &rc);
    if (rc == LDAP_NO_MEMORY)
	goto cleanup;

    priv = sudo_ldap_role_to_priv(cn, hosts, runasusers, runasgroups,
	cmnds, opts, notbefore ? notbefore[0]->bv_val : NULL,
	notafter ? notafter[0]->bv_val : NULL, false, true, berval_iter);
    if (priv == NULL) {
	rc = LDAP_NO_MEMORY;
	goto cleanup;
    }

cleanup:
    if (cn != NULL)
	ldap_memfree(cn);
    if (cmnds != NULL)
	ldap_value_free_len(cmnds);
    if (hosts != NULL)
	ldap_value_free_len(hosts);
    if (runasusers != NULL)
	ldap_value_free_len(runasusers);
    if (runasgroups != NULL)
	ldap_value_free_len(runasgroups);
    if (opts != NULL)
	ldap_value_free_len(opts);
    if (notbefore != NULL)
	ldap_value_free_len(notbefore);
    if (notafter != NULL)
	ldap_value_free_len(notafter);

    *rc_out = rc;
    debug_return_ptr(priv);
}

static bool
ldap_to_sudoers(LDAP *ld, struct ldap_result *lres,
    struct userspec_list *ldap_userspecs)
{
    struct userspec *us;
    struct member *m;
    unsigned int i;
    int rc;
    debug_decl(ldap_to_sudoers, SUDOERS_DEBUG_LDAP);

    /* We only have a single userspec */
    if ((us = calloc(1, sizeof(*us))) == NULL)
	goto oom;
    us->file = sudo_rcstr_dup("LDAP");
    TAILQ_INIT(&us->users);
    TAILQ_INIT(&us->privileges);
    STAILQ_INIT(&us->comments);
    TAILQ_INSERT_TAIL(ldap_userspecs, us, entries);

    /* The user has already matched, use ALL as wildcard. */
    if ((m = sudo_ldap_new_member_all()) == NULL)
	goto oom;
    TAILQ_INSERT_TAIL(&us->users, m, entries);

    /* Treat each entry as a separate privilege. */
    for (i = 0; i < lres->nentries; i++) {
	struct privilege *priv;

	priv = ldap_entry_to_priv(ld, lres->entries[i].entry, &rc);
	if (priv == NULL) {
	    if (rc == LDAP_NO_MEMORY)
		goto oom;
	    continue;
	}
	TAILQ_INSERT_TAIL(&us->privileges, priv, entries);
    }

    debug_return_bool(true);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    free_userspecs(ldap_userspecs);
    debug_return_bool(false);
}

#ifdef HAVE_LDAP_SASL_INTERACTIVE_BIND_S
typedef unsigned int (*sudo_gss_krb5_ccache_name_t)(unsigned int *minor_status, const char *name, const char **old_name);
static sudo_gss_krb5_ccache_name_t sudo_gss_krb5_ccache_name;

static int
sudo_set_krb5_ccache_name(const char *name, const char **old_name)
{
    int ret = 0;
    unsigned int junk;
    static bool initialized;
    debug_decl(sudo_set_krb5_ccache_name, SUDOERS_DEBUG_LDAP);

    if (!initialized) {
	sudo_gss_krb5_ccache_name = (sudo_gss_krb5_ccache_name_t)
	    sudo_dso_findsym(SUDO_DSO_DEFAULT, "gss_krb5_ccache_name");
	initialized = true;
    }

    /*
     * Try to use gss_krb5_ccache_name() if possible.
     * We also need to set KRB5CCNAME since some LDAP libs may not use
     * gss_krb5_ccache_name().
     */
    if (sudo_gss_krb5_ccache_name != NULL) {
	ret = (int)sudo_gss_krb5_ccache_name(&junk, name, old_name);
    } else {
	/* No gss_krb5_ccache_name(), fall back on KRB5CCNAME. */
	if (old_name != NULL)
	    *old_name = sudo_getenv("KRB5CCNAME");
    }
    if (name != NULL && *name != '\0') {
	if (sudo_setenv("KRB5CCNAME", name, true) == -1)
	    ret = -1;
    } else {
	if (sudo_unsetenv("KRB5CCNAME") == -1)
	    ret = -1;
    }

    debug_return_int(ret);
}

/*
 * Make a copy of the credential cache file specified by KRB5CCNAME
 * which must be readable by the user.  The resulting cache file
 * is root-owned and will be removed after authenticating via SASL.
 */
static char *
sudo_krb5_copy_cc_file(struct sudoers_context *ctx)
{
    static char new_ccname[] = _PATH_TMP "sudocc_XXXXXXXX";
    const char *old_ccname = ctx->user.ccname;
    ssize_t nread, nwritten = -1;
    char buf[10240], *ret = NULL;
    int nfd, ofd = -1;
    debug_decl(sudo_krb5_copy_cc_file, SUDOERS_DEBUG_LDAP);

    old_ccname = sudo_krb5_ccname_path(old_ccname);
    if (old_ccname != NULL) {
	/* Open credential cache as user to prevent stolen creds. */
	if (!set_perms(ctx, PERM_USER))
	    goto done;
	ofd = open(old_ccname, O_RDONLY|O_NONBLOCK);
	if (!restore_perms())
	    goto done;

	if (ofd != -1) {
	    (void) fcntl(ofd, F_SETFL, 0);
	    if (sudo_lock_file(ofd, SUDO_LOCK)) {
		nfd = mkstemp(new_ccname);
		if (nfd != -1) {
		    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
			"copy ccache %s -> %s", old_ccname, new_ccname);
		    while ((nread = read(ofd, buf, sizeof(buf))) > 0) {
			ssize_t off = 0;
			do {
			    nwritten = write(nfd, buf + off,
				(size_t)(nread - off));
			    if (nwritten == -1) {
				sudo_warn("error writing to %s", new_ccname);
				goto write_error;
			    }
			    off += nwritten;
			} while (off < nread);
		    }
		    if (nread == -1)
			sudo_warn("unable to read %s", new_ccname);
write_error:
		    close(nfd);
		    if (nread != -1 && nwritten != -1) {
			ret = new_ccname;	/* success! */
		    } else {
			unlink(new_ccname);	/* failed */
		    }
		} else {
		    sudo_warn("unable to create temp file %s", new_ccname);
		}
	    }
	} else {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
		"unable to open %s", old_ccname);
	}
    }
done:
    if (ofd != -1)
	close(ofd);
    debug_return_str(ret);
}

static int
sudo_ldap_sasl_interact(LDAP *ld, unsigned int flags, void *_auth_id,
    void *_interact)
{
    char *auth_id = (char *)_auth_id;
    sasl_interact_t *interact = (sasl_interact_t *)_interact;
    int ret = LDAP_SUCCESS;
    debug_decl(sudo_ldap_sasl_interact, SUDOERS_DEBUG_LDAP);

    for (; interact->id != SASL_CB_LIST_END; interact++) {
	if (interact->id != SASL_CB_USER) {
	    sudo_warnx("sudo_ldap_sasl_interact: unexpected interact id %lu",
		interact->id);
	    ret = LDAP_PARAM_ERROR;
	    break;
	}

	if (auth_id != NULL)
	    interact->result = auth_id;
	else if (interact->defresult != NULL)
	    interact->result = interact->defresult;
	else
	    interact->result = "";

	interact->len = (unsigned int)strlen(interact->result);
#if SASL_VERSION_MAJOR < 2
	interact->result = strdup(interact->result);
	if (interact->result == NULL) {
	    ret = LDAP_NO_MEMORY;
	    break;
	}
#endif /* SASL_VERSION_MAJOR < 2 */
	DPRINTF2("sudo_ldap_sasl_interact: SASL_CB_USER %s",
	    (const char *)interact->result);
    }
    debug_return_int(ret);
}
#endif /* HAVE_LDAP_SASL_INTERACTIVE_BIND_S */

/*
 * Create a new sudo_ldap_result structure.
 */
static struct ldap_result *
sudo_ldap_result_alloc(void)
{
    struct ldap_result *result;
    debug_decl(sudo_ldap_result_alloc, SUDOERS_DEBUG_LDAP);

    result = calloc(1, sizeof(*result));
    if (result != NULL)
	STAILQ_INIT(&result->searches);

    debug_return_ptr(result);
}

/*
 * Free the ldap result structure
 */
static void
sudo_ldap_result_free(struct ldap_result *lres)
{
    struct ldap_search_result *s;
    debug_decl(sudo_ldap_result_free, SUDOERS_DEBUG_LDAP);

    if (lres != NULL) {
	if (lres->nentries) {
	    free(lres->entries);
	    lres->entries = NULL;
	}
	while ((s = STAILQ_FIRST(&lres->searches)) != NULL) {
	    STAILQ_REMOVE_HEAD(&lres->searches, entries);
	    ldap_msgfree(s->searchresult);
	    free(s);
	}
	free(lres);
    }
    debug_return;
}

/*
 * Add a search result to the ldap_result structure.
 */
static struct ldap_search_result *
sudo_ldap_result_add_search(struct ldap_result *lres, LDAP *ldap,
    LDAPMessage *searchresult)
{
    struct ldap_search_result *news;
    debug_decl(sudo_ldap_result_add_search, SUDOERS_DEBUG_LDAP);

    /* Create new entry and add it to the end of the chain. */
    news = calloc(1, sizeof(*news));
    if (news != NULL) {
	news->ldap = ldap;
	news->searchresult = searchresult;
	STAILQ_INSERT_TAIL(&lres->searches, news, entries);
    }

    debug_return_ptr(news);
}

/*
 * Connect to the LDAP server specified by ld.
 * Returns LDAP_SUCCESS on success, else non-zero.
 */
static int
sudo_ldap_bind_s(struct sudoers_context *ctx, LDAP *ld)
{
    int ret;
    debug_decl(sudo_ldap_bind_s, SUDOERS_DEBUG_LDAP);

#ifdef HAVE_LDAP_SASL_INTERACTIVE_BIND_S
    if (ldap_conf.rootuse_sasl == true ||
	(ldap_conf.rootuse_sasl != false && ldap_conf.use_sasl == true)) {
	const char *old_ccname = NULL;
	const char *new_ccname = ldap_conf.krb5_ccname;
	const char *tmp_ccname = NULL;
	void *auth_id = ldap_conf.rootsasl_auth_id ?
	    ldap_conf.rootsasl_auth_id : ldap_conf.sasl_auth_id;
	int rc;

	/* Make temp copy of the user's credential cache as needed. */
	if (ldap_conf.krb5_ccname == NULL && ctx->user.ccname != NULL) {
	    new_ccname = tmp_ccname = sudo_krb5_copy_cc_file(ctx);
	    if (tmp_ccname == NULL) {
		/* XXX - fatal error */
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "unable to copy user ccache %s", ctx->user.ccname);
	    }
	}

	if (new_ccname != NULL) {
	    rc = sudo_set_krb5_ccache_name(new_ccname, &old_ccname);
	    if (rc == 0) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "set ccache name %s -> %s",
		    old_ccname ? old_ccname : "(none)", new_ccname);
	    } else {
		sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		    "sudo_set_krb5_ccache_name() failed: %d", rc);
	    }
	}
	ret = ldap_sasl_interactive_bind_s(ld, ldap_conf.binddn,
	    ldap_conf.sasl_mech, NULL, NULL, LDAP_SASL_QUIET,
	    sudo_ldap_sasl_interact, auth_id);
	if (new_ccname != NULL) {
	    rc = sudo_set_krb5_ccache_name(old_ccname ? old_ccname : "", NULL);
	    if (rc == 0) {
		sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		    "restore ccache name %s -> %s", new_ccname,
		    old_ccname ? old_ccname : "(none)");
	    } else {
		sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		    "sudo_set_krb5_ccache_name() failed: %d", rc);
	    }
	    /* Remove temporary copy of user's credential cache. */
	    if (tmp_ccname != NULL)
		unlink(tmp_ccname);
	}
	if (ret != LDAP_SUCCESS) {
	    sudo_warnx("ldap_sasl_interactive_bind_s(): %s",
		ldap_err2string(ret));
	    goto done;
	}
	DPRINTF1("ldap_sasl_interactive_bind_s() ok");
    } else
#endif /* HAVE_LDAP_SASL_INTERACTIVE_BIND_S */
#ifdef HAVE_LDAP_SASL_BIND_S
    {
	struct berval bv;

	bv.bv_val = ldap_conf.bindpw ? ldap_conf.bindpw : (char *)"";
	bv.bv_len = strlen(bv.bv_val);

	ret = ldap_sasl_bind_s(ld, ldap_conf.binddn, LDAP_SASL_SIMPLE, &bv,
	    NULL, NULL, NULL);
	if (ret != LDAP_SUCCESS) {
	    sudo_warnx("ldap_sasl_bind_s(): %s", ldap_err2string(ret));
	    goto done;
	}
	DPRINTF1("ldap_sasl_bind_s() ok");
    }
#else
    {
	ret = ldap_simple_bind_s(ld, ldap_conf.binddn, ldap_conf.bindpw);
	if (ret != LDAP_SUCCESS) {
	    sudo_warnx("ldap_simple_bind_s(): %s", ldap_err2string(ret));
	    goto done;
	}
	DPRINTF1("ldap_simple_bind_s() ok");
    }
#endif
done:
    debug_return_int(ret);
}

/*
 * Shut down the LDAP connection.
 */
static int
sudo_ldap_close(struct sudoers_context *ctx, struct sudo_nss *nss)
{
    struct sudo_ldap_handle *handle = nss->handle;
    debug_decl(sudo_ldap_close, SUDOERS_DEBUG_LDAP);

    if (handle != NULL) {
	/* Unbind and close the LDAP connection. */
	if (handle->ld != NULL) {
	    ldap_unbind_ext_s(handle->ld, NULL, NULL);
	    handle->ld = NULL;
	}

	/* Free the handle container. */
	if (handle->pw != NULL)
	    sudo_pw_delref(handle->pw);
	free_parse_tree(&handle->parse_tree);
	free(handle);
	nss->handle = NULL;
    }
    debug_return_int(0);
}

/*
 * Open a connection to the LDAP server.
 * Returns 0 on success and non-zero on failure.
 */
static int
sudo_ldap_open(struct sudoers_context *ctx, struct sudo_nss *nss)
{
    LDAP *ld;
    int rc = -1;
    bool ldapnoinit = false;
    struct sudo_ldap_handle *handle;
    debug_decl(sudo_ldap_open, SUDOERS_DEBUG_LDAP);

    if (nss->handle != NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: called with non-NULL handle %p", __func__, nss->handle);
	sudo_ldap_close(ctx, nss);
    }

    if (!sudo_ldap_read_config(ctx))
	goto done;

    /* Prevent reading of user ldaprc and system defaults. */
    if (sudo_getenv("LDAPNOINIT") == NULL) {
	if (sudo_setenv("LDAPNOINIT", "1", true) == 0)
	    ldapnoinit = true;
    }

    /* Set global LDAP options */
    if (sudo_ldap_set_options_global() != LDAP_SUCCESS)
	goto done;

    /* Connect to LDAP server */
#ifdef HAVE_LDAP_INITIALIZE
    if (!STAILQ_EMPTY(&ldap_conf.uri)) {
	char *buf = sudo_ldap_join_uri(&ldap_conf.uri);
	if (buf == NULL)
	    goto done;
	DPRINTF2("ldap_initialize(ld, %s)", buf);
	rc = ldap_initialize(&ld, buf);
	free(buf);
    } else
#endif
	rc = sudo_ldap_init(ctx, &ld, ldap_conf.host, ldap_conf.port);
    if (rc != LDAP_SUCCESS) {
	sudo_warnx(U_("unable to initialize LDAP: %s"), ldap_err2string(rc));
	goto done;
    }

    /* Set LDAP per-connection options */
    rc = sudo_ldap_set_options_conn(ld);
    if (rc != LDAP_SUCCESS)
	goto done;

    if (ldapnoinit)
	(void) sudo_unsetenv("LDAPNOINIT");

    if (ldap_conf.ssl_mode == SUDO_LDAP_STARTTLS) {
#if defined(HAVE_LDAP_START_TLS_S)
	rc = ldap_start_tls_s(ld, NULL, NULL);
	if (rc != LDAP_SUCCESS) {
	    sudo_warnx("ldap_start_tls_s(): %s", ldap_err2string(rc));
	    goto done;
	}
	DPRINTF1("ldap_start_tls_s() ok");
#elif defined(HAVE_LDAP_SSL_CLIENT_INIT) && defined(HAVE_LDAP_START_TLS_S_NP)
	int sslrc;
	rc = ldap_ssl_client_init(ldap_conf.tls_keyfile, ldap_conf.tls_keypw,
	    0, &sslrc);
	if (rc != LDAP_SUCCESS) {
	    sudo_warnx("ldap_ssl_client_init(): %s: %s",
		ldap_err2string(rc), ssl_err2string(sslrc));
	    goto done;
	}
	rc = ldap_start_tls_s_np(ld, NULL);
	if (rc != LDAP_SUCCESS) {
	    sudo_warnx("ldap_start_tls_s_np(): %s", ldap_err2string(rc));
	    goto done;
	}
	DPRINTF1("ldap_start_tls_s_np() ok");
#else
	sudo_warnx("%s",
	    U_("start_tls specified but LDAP libs do not support ldap_start_tls_s() or ldap_start_tls_s_np()"));
#endif /* !HAVE_LDAP_START_TLS_S && !HAVE_LDAP_START_TLS_S_NP */
    }

    /* Actually connect */
    rc = sudo_ldap_bind_s(ctx, ld);
    if (rc != LDAP_SUCCESS)
	goto done;

    /* Create a handle container. */
    handle = calloc(1, sizeof(struct sudo_ldap_handle));
    if (handle == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	rc = -1;
	goto done;
    }
    handle->ld = ld;
    /* handle->pw = NULL; */
    init_parse_tree(&handle->parse_tree, NULL, NULL, ctx, nss);
    nss->handle = handle;

done:
    debug_return_int(rc == LDAP_SUCCESS ? 0 : -1);
}

static int
sudo_ldap_getdefs(struct sudoers_context *ctx, const struct sudo_nss *nss)
{
    struct sudo_ldap_handle *handle = nss->handle;
    struct timeval tv, *tvp = NULL;
    struct ldap_config_str *base;
    LDAPMessage *entry, *result = NULL;
    char *filt = NULL;
    int rc, ret = -1;
    static bool cached;
    debug_decl(sudo_ldap_getdefs, SUDOERS_DEBUG_LDAP);

    if (handle == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: called with NULL handle", __func__);
	debug_return_int(-1);
    }

    /* Use cached result if present. */
    if (cached)
	debug_return_int(0);

    filt = sudo_ldap_build_default_filter();
    if (filt == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }
    DPRINTF1("Looking for cn=defaults: %s", filt);

    STAILQ_FOREACH(base, &ldap_conf.base, entries) {
	LDAP *ld = handle->ld;

	if (ldap_conf.timeout > 0) {
	    tv.tv_sec = ldap_conf.timeout;
	    tv.tv_usec = 0;
	    tvp = &tv;
	}
	ldap_msgfree(result);
	result = NULL;
	rc = ldap_search_ext_s(ld, base->val, LDAP_SCOPE_SUBTREE,
	    filt, NULL, 0, NULL, NULL, tvp, 0, &result);
	if (rc == LDAP_SUCCESS && (entry = ldap_first_entry(ld, result))) {
	    DPRINTF1("found:%s", ldap_get_dn(ld, entry));
	    if (!sudo_ldap_parse_options(ld, entry, &handle->parse_tree.defaults))
		goto done;
	} else {
	    DPRINTF1("no default options found in %s", base->val);
	}
    }
    cached = true;
    ret = 0;

done:
    ldap_msgfree(result);
    free(filt);

    debug_return_int(ret);
}

/*
 * Comparison function for ldap_entry_wrapper structures, ascending order.
 * This should match role_order_cmp() in parse_ldif.c.
 */
static int
ldap_entry_compare(const void *a, const void *b)
{
    const struct ldap_entry_wrapper *aw = a;
    const struct ldap_entry_wrapper *bw = b;
    debug_decl(ldap_entry_compare, SUDOERS_DEBUG_LDAP);

    debug_return_int(aw->order < bw->order ? -1 :
	(aw->order > bw->order ? 1 : 0));
}

/*
 * Return the last entry in the list of searches, usually the
 * one currently being used to add entries.
 */
static struct ldap_search_result *
sudo_ldap_result_last_search(struct ldap_result *lres)
{
    debug_decl(sudo_ldap_result_last_search, SUDOERS_DEBUG_LDAP);

    debug_return_ptr(STAILQ_LAST(&lres->searches, ldap_search_result, entries));
}

/*
 * Add an entry to the result structure.
 */
static struct ldap_entry_wrapper *
sudo_ldap_result_add_entry(struct ldap_result *lres, LDAPMessage *entry)
{
    struct ldap_search_result *last;
    struct berval **bv;
    double order = 0.0;
    char *ep;
    int rc;
    debug_decl(sudo_ldap_result_add_entry, SUDOERS_DEBUG_LDAP);

    /* Determine whether the entry has the sudoOrder attribute. */
    last = sudo_ldap_result_last_search(lres);
    if (last != NULL) {
	bv = sudo_ldap_get_values_len(last->ldap, entry, "sudoOrder", &rc);
	if (bv == NULL) {
	    if (rc == LDAP_NO_MEMORY)
		debug_return_ptr(NULL);
	} else {
	    if (ldap_count_values_len(bv) > 0) {
		/* Get the value of this attribute, 0 if not present. */
		DPRINTF2("order attribute raw: %s", (*bv)->bv_val);
		order = strtod((*bv)->bv_val, &ep);
		if (ep == (*bv)->bv_val || *ep != '\0') {
		    sudo_warnx(U_("invalid sudoOrder attribute: %s"),
			(*bv)->bv_val);
		    order = 0.0;
		}
		DPRINTF2("order attribute: %f", order);
	    }
	    ldap_value_free_len(bv);
	}
    }

    /*
     * Enlarge the array of entry wrappers as needed, preallocating blocks
     * of 100 entries to save on allocation time.
     */
    if (++lres->nentries > lres->allocated_entries) {
	unsigned int allocated_entries =
	    lres->allocated_entries + ALLOCATION_INCREMENT;
	struct ldap_entry_wrapper *entries = reallocarray(lres->entries,
	    allocated_entries, sizeof(lres->entries[0]));
	if (entries == NULL)
	    debug_return_ptr(NULL);
	lres->allocated_entries = allocated_entries;
	lres->entries = entries;
    }

    /* Fill in the new entry and return it. */
    lres->entries[lres->nentries - 1].entry = entry;
    lres->entries[lres->nentries - 1].order = order;

    debug_return_ptr(&lres->entries[lres->nentries - 1]);
}

/*
 * Perform the LDAP query for the user.  The caller is responsible for
 * freeing the result with sudo_ldap_result_free().
 */
static struct ldap_result *
sudo_ldap_result_get(struct sudoers_context *ctx, const struct sudo_nss *nss,
    struct passwd *pw)
{
    struct sudo_ldap_handle *handle = nss->handle;
    struct ldap_config_str *base;
    struct ldap_result *lres;
    struct timeval tv, *tvp = NULL;
    LDAPMessage *entry, *result;
    LDAP *ld = handle->ld;
    char *filt = NULL;
    int pass, rc;
    debug_decl(sudo_ldap_result_get, SUDOERS_DEBUG_LDAP);

    /*
     * Okay - time to search for anything that matches this user
     * Lets limit it to only two queries of the LDAP server
     *
     * The first pass will look by the username, groups, and
     * the keyword ALL.  We will then inspect the results that
     * came back from the query.  We don't need to inspect the
     * sudoUser in this pass since the LDAP server already scanned
     * it for us.
     *
     * The second pass will return all the entries that contain non-
     * Unix groups, including netgroups.  Then we take the non-Unix
     * groups returned and try to match them against the username.
     *
     * Since we have to sort the possible entries before we make a
     * decision, we perform the queries and store all of the results in
     * an ldap_result object.  The results are then sorted by sudoOrder.
     */
    lres = sudo_ldap_result_alloc();
    if (lres == NULL)
	goto oom;
    for (pass = 0; pass < 2; pass++) {
	filt = pass ? sudo_ldap_build_pass2() : sudo_ldap_build_pass1(ctx, ld, pw);
	if (filt != NULL) {
	    DPRINTF1("ldap search '%s'", filt);
	    STAILQ_FOREACH(base, &ldap_conf.base, entries) {
		DPRINTF1("searching from base '%s'",
		    base->val);
		if (ldap_conf.timeout > 0) {
		    tv.tv_sec = ldap_conf.timeout;
		    tv.tv_usec = 0;
		    tvp = &tv;
		}
		result = NULL;
		rc = ldap_search_ext_s(ld, base->val, LDAP_SCOPE_SUBTREE, filt,
		    NULL, 0, NULL, NULL, tvp, 0, &result);
		if (rc != LDAP_SUCCESS) {
		    DPRINTF1("ldap search pass %d failed: %s", pass + 1,
			ldap_err2string(rc));
		    continue;
		}

		/* Add the search result to list of search results. */
		DPRINTF1("adding search result");
		if (sudo_ldap_result_add_search(lres, ld, result) == NULL)
		    goto oom;
		LDAP_FOREACH(entry, ld, result) {
		    if (pass != 0) {
			/* Check non-unix group in 2nd pass. */
			switch (sudo_ldap_check_non_unix_group(ctx, nss, entry,
			    pw)) {
			case -1:
			    goto oom;
			case false:
			    continue;
			default:
			    break;
			}
		    }
		    if (sudo_ldap_result_add_entry(lres, entry) == NULL)
			goto oom;
		}
		DPRINTF1("result now has %d entries", lres->nentries);
	    }
	    free(filt);
	} else if (errno != ENOENT) {
	    /* Out of memory? */
	    goto oom;
	}
    }

    /* Sort the entries by the sudoOrder attribute. */
    if (lres->nentries != 0) {
	DPRINTF1("sorting remaining %d entries", lres->nentries);
	qsort(lres->entries, lres->nentries, sizeof(lres->entries[0]),
	    ldap_entry_compare);
    }

    debug_return_ptr(lres);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    free(filt);
    sudo_ldap_result_free(lres);
    debug_return_ptr(NULL);
}

/*
 * Perform LDAP query for user and host and convert to sudoers
 * parse tree.
 */
static int
sudo_ldap_query(struct sudoers_context *ctx, const struct sudo_nss *nss,
    struct passwd *pw)
{
    struct sudo_ldap_handle *handle = nss->handle;
    struct ldap_result *lres = NULL;
    int ret = -1;
    debug_decl(sudo_ldap_query, SUDOERS_DEBUG_LDAP);

    if (handle == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: called with NULL handle", __func__);
	debug_return_int(-1);
    }

    /* Use cached result if it matches pw. */
    if (handle->pw != NULL) {
	if (pw == handle->pw) {
	    ret = 0;
	    goto done;
	}
	sudo_pw_delref(handle->pw);
	handle->pw = NULL;
    }

    /* Free old userspecs, if any. */
    free_userspecs(&handle->parse_tree.userspecs);

    DPRINTF1("%s: ldap search user %s, host %s", __func__, pw->pw_name,
	ctx->runas.host);
    if ((lres = sudo_ldap_result_get(ctx, nss, pw)) == NULL)
	goto done;

    /* Convert to sudoers parse tree. */
    if (!ldap_to_sudoers(handle->ld, lres, &handle->parse_tree.userspecs))
	goto done;

    /* Stash a ref to the passwd struct in the handle. */
    sudo_pw_addref(pw);
    handle->pw = pw;

    ret = 0;

done:
    /* Cleanup. */
    sudo_ldap_result_free(lres);
    if (ret == -1)
	free_userspecs(&handle->parse_tree.userspecs);
    debug_return_int(ret);
}

/*
 * Return the initialized (but empty) sudoers parse tree.
 * The contents will be populated by the getdefs() and query() functions.
 */
static struct sudoers_parse_tree *
sudo_ldap_parse(struct sudoers_context *ctx, const struct sudo_nss *nss)
{
    struct sudo_ldap_handle *handle = nss->handle;
    debug_decl(sudo_ldap_parse, SUDOERS_DEBUG_LDAP);

    if (handle == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR,
	    "%s: called with NULL handle", __func__);
	debug_return_ptr(NULL);
    }

    debug_return_ptr(&handle->parse_tree);
}

static int
sudo_ldap_innetgr(const struct sudo_nss *nss, const char *netgr,
    const char *host, const char *user, const char *domain)
{
    const struct sudo_ldap_handle *handle = nss->handle;
    return sudo_ldap_innetgr_int(handle->ld, netgr, host, user, domain);
}

#if 0
/*
 * Create an ldap_result from an LDAP search result.
 *
 * This function is currently not used anywhere, it is left here as
 * an example of how to use the cached searches.
 */
static struct ldap_result *
sudo_ldap_result_from_search(LDAP *ldap, LDAPMessage *searchresult)
{
    struct ldap_search_result *last;
    struct ldap_result *result;
    LDAPMessage	*entry;

    /*
     * An ldap_result is built from several search results, which are
     * organized in a list. The head of the list is maintained in the
     * ldap_result structure, together with the wrappers that point
     * to individual entries, this has to be initialized first.
     */
    result = sudo_ldap_result_alloc();
    if (result == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_ptr(NULL);
    }

    /*
     * Build a new list node for the search result, this creates the
     * list node.
     */
    last = sudo_ldap_result_add_search(result, ldap, searchresult);

    /*
     * Now add each entry in the search result to the array of of entries
     * in the ldap_result object.
     */
    LDAP_FOREACH(entry, last->ldap, last->searchresult) {
	if (sudo_ldap_result_add_entry(result, entry) == NULL) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    sudo_ldap_result_free(result);
	    result = NULL;
	    break;
	}
    }
    DPRINTF1("sudo_ldap_result_from_search: %d entries found",
	result ? result->nentries : -1);
    return result;
}
#endif

/* sudo_nss implementation */
struct sudo_nss sudo_nss_ldap = {
    { NULL, NULL },
    "ldap",
    sudo_ldap_open,
    sudo_ldap_close,
    sudo_ldap_parse,
    sudo_ldap_query,
    sudo_ldap_getdefs,
    sudo_ldap_innetgr
};
