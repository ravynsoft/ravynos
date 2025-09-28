/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <time.h>
#include <ctype.h>
#ifdef HAVE_LBER_H
# include <lber.h>
#endif
#include <ldap.h>

#include <sudoers.h>
#include <sudo_ldap.h>
#include <sudo_ldap_conf.h>

/*
 * Compare str to netgroup string ngstr of length nglen where str is a
 * NUL-terminated string and ngstr is part of a netgroup triple string.
 * Uses innetgr(3)-style matching rules.
 * Returns true if the strings match, else false.
 */
static bool
sudo_ldap_netgroup_match_str(const char *str, const char *ngstr, size_t nglen,
    bool ignore_case)
{
    debug_decl(sudo_ldap_netgroup_match_str, SUDOERS_DEBUG_LDAP);

    /* Skip leading whitespace. */
    while (isspace((unsigned char)*ngstr) && nglen > 0) {
	ngstr++;
	nglen--;
    }
    /* Skip trailing whitespace. */
    while (nglen > 0 && isspace((unsigned char)ngstr[nglen - 1])) {
	nglen--;
    }

    sudo_debug_printf(SUDO_DEBUG_DEBUG, "%s: compare \"%s\" to \"%.*s\"",
	__func__, str ? str : "", (int)nglen, ngstr);

    if (nglen == 0 || str == NULL) {
	/* An empty string is a wildcard. */
	debug_return_bool(true);
    }
    if (*ngstr == '-' && nglen == 1) {
	/* '-' means no valid value. */
	debug_return_bool(false);
    }
    if (ignore_case) {
	if (strncasecmp(str, ngstr, nglen) == 0 && str[nglen] == '\0')
	    debug_return_bool(true);
    } else {
	if (strncmp(str, ngstr, nglen) == 0 && str[nglen] == '\0')
	    debug_return_bool(true);
    }
    debug_return_bool(false);
}

/*
 * Match the specified netgroup triple using the given host,
 * user and domain.  Matching rules as per innetgr(3).
 * Returns 1 on match, else 0.
 */
static int
sudo_ldap_match_netgroup(const char *triple, const char *host,
    const char *user, const char *domain)
{
    const char *cp, *ep;
    debug_decl(sudo_ldap_match_netgroup, SUDOERS_DEBUG_LDAP);

    /* Trim leading space, check for opening paren. */
    while (isspace((unsigned char)*triple))
	triple++;
    if (*triple != '(') {
	sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: invalid triple: %s",
	    __func__, triple);
	debug_return_int(0);
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: matching (%s,%s,%s) against %s",
	__func__, host ? host : "", user ? user : "", domain ? domain : "",
	triple);

    /* Parse host. */
    cp = triple + 1;
    ep = strchr(cp, ',');
    if (ep == NULL || !sudo_ldap_netgroup_match_str(host, cp, (size_t)(ep - cp), true))
	debug_return_int(0);

    /* Parse user. */
    cp = ep + 1;
    ep = strchr(cp, ',');
    if (ep == NULL || !sudo_ldap_netgroup_match_str(user, cp, (size_t)(ep - cp), def_case_insensitive_user))
	debug_return_int(0);

    /* Parse domain. */
    cp = ep + 1;
    ep = strchr(cp, ')');
    if (ep == NULL || !sudo_ldap_netgroup_match_str(domain, cp, (size_t)(ep - cp), true))
	debug_return_int(0);

    debug_return_int(1);
}

#define MAX_NETGROUP_DEPTH 128
struct netgroups_seen {
    const char *groups[MAX_NETGROUP_DEPTH];
    size_t len;
};

static int
sudo_ldap_innetgr_base(LDAP *ld, const char *base,
    struct timeval *timeout, const char *netgr, const char *host,
    const char *user, const char *domain, struct netgroups_seen *seen)
{
    char *escaped_netgr = NULL, *filt = NULL;
    LDAPMessage *entry, *result = NULL;
    int rc, ret = 0;
    size_t n;
    debug_decl(sudo_ldap_innetgr_base, SUDOERS_DEBUG_LDAP);

    /* Cycle detection. */
    for (n = 0; n < seen->len; n++) {
	if (strcmp(netgr, seen->groups[n]) == 0) {
	    DPRINTF1("%s: cycle in netgroups", netgr);
	    goto done;
	}
    }
    if (seen->len + 1 > MAX_NETGROUP_DEPTH) {
	DPRINTF1("%s: too many nested netgroups", netgr);
	goto done;
    }
    seen->groups[seen->len++] = netgr;

    /* Escape the netgroup name per RFC 4515. */
    if ((escaped_netgr = sudo_ldap_value_dup(netgr)) == NULL)
	goto done;

    /* Build nisNetgroup query. */
    rc = asprintf(&filt, "(&%s(cn=%s))",
	ldap_conf.netgroup_search_filter, escaped_netgr);
    if (rc == -1)
	goto done;
    DPRINTF1("ldap netgroup search filter: '%s'", filt);

    /* Perform an LDAP query for nisNetgroup. */
    DPRINTF1("searching from netgroup_base '%s'", base);
    rc = ldap_search_ext_s(ld, base, LDAP_SCOPE_SUBTREE, filt,
	NULL, 0, NULL, NULL, timeout, 0, &result);
    free(filt);
    if (rc != LDAP_SUCCESS) {
	DPRINTF1("ldap netgroup search failed: %s", ldap_err2string(rc));
	goto done;
    }

    LDAP_FOREACH(entry, ld, result) {
	struct berval **bv, **p;

	/* Check all nisNetgroupTriple entries. */
	bv = ldap_get_values_len(ld, entry, "nisNetgroupTriple");
	if (bv == NULL) {
	    const int optrc = ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &rc);
	    if (optrc != LDAP_OPT_SUCCESS || rc == LDAP_NO_MEMORY)
		goto done;
	} else {
	    for (p = bv; *p != NULL && !ret; p++) {
		char *val = (*p)->bv_val;
		if (sudo_ldap_match_netgroup(val, host, user, domain)) {
		    ret = 1;
		    break;
		}
	    }
	    ldap_value_free_len(bv);
	    if (ret == 1)
		break;
	}

	/* Handle nested netgroups. */
	bv = ldap_get_values_len(ld, entry, "memberNisNetgroup");
	if (bv == NULL) {
	    const int optrc = ldap_get_option(ld, LDAP_OPT_RESULT_CODE, &rc);
	    if (optrc != LDAP_OPT_SUCCESS || rc == LDAP_NO_MEMORY)
		goto done;
	} else {
	    for (p = bv; *p != NULL && !ret; p++) {
		const char *val = (*p)->bv_val;
		const size_t saved_len = seen->len;
		ret = sudo_ldap_innetgr_base(ld, base, timeout, val, host,
		    user, domain, seen);
		/* Restore seen state to avoid use-after-free. */
		seen->len = saved_len;
	    }
	    ldap_value_free_len(bv);
	}
    }

done:
    ldap_msgfree(result);
    free(escaped_netgr);

    debug_return_int(ret);
}

int
sudo_ldap_innetgr_int(void *v, const char *netgr, const char *host,
    const char *user, const char *domain)
{
    LDAP *ld = v;
    struct timeval tv, *tvp = NULL;
    struct ldap_config_str *base;
    struct netgroups_seen seen;
    int ret = 0;
    debug_decl(sudo_ldap_innetgr, SUDOERS_DEBUG_LDAP);

    if (STAILQ_EMPTY(&ldap_conf.netgroup_base)) {
	/* LDAP netgroups not configured. */
	debug_return_int(-1);
    }

    if (ldap_conf.timeout > 0) {
	tv.tv_sec = ldap_conf.timeout;
	tv.tv_usec = 0;
	tvp = &tv;
    }

    /* Perform an LDAP query for nisNetgroup. */
    STAILQ_FOREACH(base, &ldap_conf.netgroup_base, entries) {
	seen.len = 0;
	ret = sudo_ldap_innetgr_base(ld, base->val, tvp, netgr, host,
	    user, domain, &seen);
	if (ret != 0)
	    break;
    }

    debug_return_int(ret);
}
