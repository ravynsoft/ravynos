/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013, 2016, 2018-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sudoers.h>
#include <interfaces.h>
#include <sudo_lbuf.h>
#include <sudo_ldap.h>
#include <sudo_digest.h>
#include <gram.h>

/*
 * Returns true if the string pointed to by valp begins with an
 * odd number of '!' characters.  Intervening blanks are ignored.
 * Stores the address of the string after '!' removal in valp.
 */
bool
sudo_ldap_is_negated(char **valp)
{
    char *val = *valp;
    bool ret = false;
    debug_decl(sudo_ldap_is_negated, SUDOERS_DEBUG_LDAP);

    while (*val == '!') {
	ret = !ret;
	do {
	    val++;
	} while (isblank((unsigned char)*val));
    }
    *valp = val;
    debug_return_bool(ret);
}

/*
 * Parse an option string into a defaults structure.
 * The members of def are pointers into optstr (which is modified).
 */
int
sudo_ldap_parse_option(char *optstr, char **varp, char **valp)
{
    char *cp, *val = NULL;
    char *var = optstr;
    int op;
    debug_decl(sudo_ldap_parse_option, SUDOERS_DEBUG_LDAP);

    /* check for equals sign past first char */
    cp = strchr(var, '=');
    if (cp != NULL && cp > var) {
	val = cp + 1;
	op = cp[-1];	/* peek for += or -= cases */
	if (op == '+' || op == '-') {
	    /* case var+=val or var-=val */
	    cp--;
	} else {
	    /* case var=val */
	    op = true;
	}
	/* Trim whitespace between var and operator. */
	while (cp > var && isblank((unsigned char)cp[-1]))
	    cp--;
	/* Truncate variable name. */
	*cp = '\0';
	/* Trim leading whitespace from val. */
	while (isblank((unsigned char)*val))
	    val++;
	/* Strip double quotes if present. */
	if (*val == '"') {
	    char *ep = val + strlen(val);
	    if (ep != val && ep[-1] == '"') {
		val++;
		ep[-1] = '\0';
	    }
	}
    } else {
	/* Boolean value, either true or false. */
	op = sudo_ldap_is_negated(&var) ? false : true;
    }
    *varp = var;
    *valp = val;

    debug_return_int(op);
}

/*
 * Convert an array of user/group names to a member list.
 * The caller is responsible for freeing the returned struct member_list.
 */
static struct member_list *
array_to_member_list(void *a, sudo_ldap_iter_t iter)
{
    struct member_list negated_members =
	TAILQ_HEAD_INITIALIZER(negated_members);
    struct member_list *members;
    struct member *m;
    char *val;
    debug_decl(bv_to_member_list, SUDOERS_DEBUG_LDAP);

    if ((members = calloc(1, sizeof(*members))) == NULL)
	return NULL;
    TAILQ_INIT(members);                      

    while ((val = iter(&a)) != NULL) {
	if ((m = calloc(1, sizeof(*m))) == NULL)
	    goto bad;
	m->negated = sudo_ldap_is_negated(&val);

	switch (val[0]) {
	case '\0':
	    /* Empty RunAsUser means run as the invoking user. */
	    m->type = MYSELF;
	    break;
	case '+':
	    m->type = NETGROUP;
	    break;
	case '%':
	    m->type = USERGROUP;
	    break;
	case 'A':
	    if (strcmp(val, "ALL") == 0) {
		m->type = ALL;
		break;
	    }
	    FALLTHROUGH;
	default:
	    m->type = WORD;
	    break;
	}
	if (m->type != ALL && m->type != MYSELF) {
	    if ((m->name = strdup(val)) == NULL) {
		free(m);
		goto bad;
	    }
	}
	if (m->negated)
	    TAILQ_INSERT_TAIL(&negated_members, m, entries);
	else
	    TAILQ_INSERT_TAIL(members, m, entries);
    }

    /* Negated members take precedence so we insert them at the end. */
    TAILQ_CONCAT(members, &negated_members, entries);
    debug_return_ptr(members);
bad:
    free_members(&negated_members);
    free_members(members);
    free(members);
    debug_return_ptr(NULL);
}

static bool
is_address(char *host)
{
    union sudo_in_addr_un addr;
    bool ret = false;
    char *slash;
    debug_decl(is_address, SUDOERS_DEBUG_LDAP);

    /* Check for mask, not currently parsed. */
    if ((slash = strchr(host, '/')) != NULL)
	*slash = '\0';

    if (inet_pton(AF_INET, host, &addr.ip4) == 1)
	ret = true;
#ifdef HAVE_STRUCT_IN6_ADDR
    else if (inet_pton(AF_INET6, host, &addr.ip6) == 1)
	ret = true;
#endif

    if (slash != NULL)
	*slash = '/';

    debug_return_bool(ret);
}

static struct member *
host_to_member(char *host)
{
    struct member *m;
    debug_decl(host_to_member, SUDOERS_DEBUG_LDAP);

    if ((m = calloc(1, sizeof(*m))) == NULL)
	goto oom;
    m->negated = sudo_ldap_is_negated(&host);
    switch (*host) {
    case '+':
	m->type = NETGROUP;
	break;
    case 'A':
	if (strcmp(host, "ALL") == 0) {
	    m->type = ALL;
	    break;
	}
	FALLTHROUGH;
    default:
	if (is_address(host)) {
	    m->type = NTWKADDR;
	} else {
	    m->type = WORD;
	}
	break;
    }
    if (m->type != ALL) {
	if ((m->name = strdup(host)) == NULL)
	    goto oom;
    }

    debug_return_ptr(m);
oom:
    free(m);
    debug_return_ptr(NULL);
}

/*
 * If a digest prefix is present, add it to struct command_digest_list
 * and update cmnd to point to the command after the digest.
 * Returns 1 if a digest was parsed, 0 if not and -1 on error.
 */
static int
sudo_ldap_extract_digest(const char *cmnd, char **endptr,
    struct command_digest_list *digests)
{
    const char *ep, *cp = cmnd;
    struct command_digest *digest;
    unsigned int digest_type = SUDO_DIGEST_INVALID;
    debug_decl(sudo_ldap_extract_digest, SUDOERS_DEBUG_LDAP);

    /*
     * Check for and extract a digest prefix, e.g.
     * sha224:d06a2617c98d377c250edd470fd5e576327748d82915d6e33b5f8db1 /bin/ls
     */
    if (cp[0] == 's' && cp[1] == 'h' && cp[2] == 'a') {
	switch (cp[3]) {
	case '2':
	    if (cp[4] == '2' && cp[5] == '4')
		digest_type = SUDO_DIGEST_SHA224;
	    else if (cp[4] == '5' && cp[5] == '6')
		digest_type = SUDO_DIGEST_SHA256;
	    break;
	case '3':
	    if (cp[4] == '8' && cp[5] == '4')
		digest_type = SUDO_DIGEST_SHA384;
	    break;
	case '5':
	    if (cp[4] == '1' && cp[5] == '2')
		digest_type = SUDO_DIGEST_SHA512;
	    break;
	}
	if (digest_type != SUDO_DIGEST_INVALID) {
	    cp += 6;
	    while (isblank((unsigned char)*cp))
		cp++;
	    if (*cp == ':') {
		cp++;
		while (isblank((unsigned char)*cp))
		    cp++;
		ep = cp;
		while (*ep != '\0' && !isblank((unsigned char)*ep) && *ep != ',')
		    ep++;
		if (isblank((unsigned char)*ep) || *ep == ',') {
		    if ((digest = malloc(sizeof(*digest))) == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			debug_return_int(-1);
		    }
		    digest->digest_type = digest_type;
		    digest->digest_str = strndup(cp, (size_t)(ep - cp));
		    if (digest->digest_str == NULL) {
			sudo_warnx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
			free(digest);
			debug_return_int(-1);
		    }
		    while (isblank((unsigned char)*ep))
			ep++;
		    *endptr = (char *)ep;
		    sudo_debug_printf(SUDO_DEBUG_INFO,
			"%s digest %s for %s",
			digest_type_to_name(digest_type),
			digest->digest_str, cp);
		    TAILQ_INSERT_TAIL(digests, digest, entries);
		    debug_return_int(1);
		}
	    }
	}
    }
    debug_return_int(0);
}

/*
 * If a digest list is present, fill in struct command_digest_list
 * and update cmnd to point to the command after the digest.
 * Returns false on error, else true.
 */
static bool
sudo_ldap_extract_digests(char **cmnd, struct command_digest_list *digests)
{
    char *cp = *cmnd;
    int rc;
    debug_decl(sudo_ldap_extract_digests, SUDOERS_DEBUG_LDAP);

    for (;;) {
	rc = sudo_ldap_extract_digest(cp, &cp, digests);
	if (rc != 1)
	    break;

	/* Check for additional digestspecs, separated by a comma. */
	if (*cp != ',')
	    break;
	do {
	    cp++;
	} while (isblank((unsigned char)*cp));
    }
    *cmnd = cp;

    debug_return_bool(rc != -1);
}

/*
 * Convert an LDAP sudoRole to a sudoers privilege.
 * Pass in struct berval ** for LDAP or char *** for SSSD.
 */
struct privilege *
sudo_ldap_role_to_priv(const char *cn, void *hosts, void *runasusers,
    void *runasgroups, void *cmnds, void *opts, const char *notbefore,
    const char *notafter, bool warnings, bool store_options,
    sudo_ldap_iter_t iter)
{
    struct cmndspec_list negated_cmnds = TAILQ_HEAD_INITIALIZER(negated_cmnds);
    struct member_list negated_hosts = TAILQ_HEAD_INITIALIZER(negated_hosts);
    struct cmndspec *prev_cmndspec = NULL;
    struct privilege *priv;
    struct member *m;
    char *cmnd;
    debug_decl(sudo_ldap_role_to_priv, SUDOERS_DEBUG_LDAP);

    if ((priv = calloc(1, sizeof(*priv))) == NULL)
	goto oom;
    TAILQ_INIT(&priv->hostlist);
    TAILQ_INIT(&priv->cmndlist);
    TAILQ_INIT(&priv->defaults);

    priv->ldap_role = strdup(cn ? cn : "UNKNOWN");
    if (priv->ldap_role == NULL)
	goto oom;

    if (hosts == NULL) {
	/* The host has already matched, use ALL as wildcard. */
	if ((m = sudo_ldap_new_member_all()) == NULL)
	    goto oom;
	TAILQ_INSERT_TAIL(&priv->hostlist, m, entries);
    } else {
	char *host;
	while ((host = iter(&hosts)) != NULL) {
	    if ((m = host_to_member(host)) == NULL)
		goto oom;
	    if (m->negated)
		TAILQ_INSERT_TAIL(&negated_hosts, m, entries);
	    else
		TAILQ_INSERT_TAIL(&priv->hostlist, m, entries);
	}
	/* Negated hosts take precedence so we insert them at the end. */
	TAILQ_CONCAT(&priv->hostlist, &negated_hosts, entries);
    }

    /*
     * Parse sudoCommands and add to cmndlist.
     */
    while ((cmnd = iter(&cmnds)) != NULL) {
	bool negated = sudo_ldap_is_negated(&cmnd);
	struct sudo_command *c = NULL;
	struct cmndspec *cmndspec;

	/* Allocate storage upfront. */
	if ((cmndspec = calloc(1, sizeof(*cmndspec))) == NULL)
	    goto oom;
	if ((m = calloc(1, sizeof(*m))) == NULL) {
	    free(cmndspec);
	    goto oom;
	}
	if ((c = calloc(1, sizeof(*c))) == NULL) {
	    free(cmndspec);
	    free(m);
	    goto oom;
	}
	m->name = (char *)c;
	TAILQ_INIT(&c->digests);

	/* Negated commands have precedence so insert them at the end. */
	if (negated)
	    TAILQ_INSERT_TAIL(&negated_cmnds, cmndspec, entries);
	else
	    TAILQ_INSERT_TAIL(&priv->cmndlist, cmndspec, entries);

	/* Initialize cmndspec */
	TAGS_INIT(&cmndspec->tags);
	cmndspec->notbefore = UNSPEC;
	cmndspec->notafter = UNSPEC;
	cmndspec->timeout = UNSPEC;
	cmndspec->cmnd = m;

	if (prev_cmndspec != NULL) {
	    /* Inherit values from prior cmndspec (common to the sudoRole). */
	    cmndspec->runasuserlist = prev_cmndspec->runasuserlist;
	    cmndspec->runasgrouplist = prev_cmndspec->runasgrouplist;
	    cmndspec->notbefore = prev_cmndspec->notbefore;
	    cmndspec->notafter = prev_cmndspec->notafter;
	    cmndspec->timeout = prev_cmndspec->timeout;
	    cmndspec->runchroot = prev_cmndspec->runchroot;
	    cmndspec->runcwd = prev_cmndspec->runcwd;
#ifdef HAVE_SELINUX
	    cmndspec->role = prev_cmndspec->role;
	    cmndspec->type = prev_cmndspec->type;
#endif /* HAVE_SELINUX */
#ifdef HAVE_PRIV_SET
	    cmndspec->privs = prev_cmndspec->privs;
	    cmndspec->limitprivs = prev_cmndspec->limitprivs;
#endif /* HAVE_PRIV_SET */
	    cmndspec->tags = prev_cmndspec->tags;
	    if (cmndspec->tags.setenv == IMPLIED)
		cmndspec->tags.setenv = UNSPEC;
	} else {
	    /* Parse sudoRunAsUser / sudoRunAs */
	    if (runasusers != NULL) {
		cmndspec->runasuserlist =
		    array_to_member_list(runasusers, iter);
		if (cmndspec->runasuserlist == NULL)
		    goto oom;
	    }

	    /* Parse sudoRunAsGroup */
	    if (runasgroups != NULL) {
		cmndspec->runasgrouplist =
		    array_to_member_list(runasgroups, iter);
		if (cmndspec->runasgrouplist == NULL)
		    goto oom;
	    }

	    /* Parse sudoNotBefore / sudoNotAfter */
	    if (notbefore != NULL)
		cmndspec->notbefore = parse_gentime(notbefore);
	    if (notafter != NULL)
		cmndspec->notafter = parse_gentime(notafter);

	    /* Parse sudoOptions. */
	    if (opts != NULL) {
		char *opt, *source = NULL;

		if (store_options) {
		    /* Use sudoRole in place of file name in defaults. */
		    size_t slen = sizeof("sudoRole ") - 1 + strlen(priv->ldap_role);
		    if ((source = sudo_rcstr_alloc(slen)) == NULL)
			goto oom;
		    if ((size_t)snprintf(source, slen + 1, "sudoRole %s", priv->ldap_role) != slen) {
			sudo_warnx(U_("internal error, %s overflow"), __func__);
			sudo_rcstr_delref(source);
			goto bad;
		    }
		}

		while ((opt = iter(&opts)) != NULL) {
		    char *var, *val;
		    int op;

		    op = sudo_ldap_parse_option(opt, &var, &val);
		    if (strcmp(var, "command_timeout") == 0 && val != NULL) {
			if (cmndspec->timeout != UNSPEC) {
			    sudo_warnx(U_("duplicate sudoOption: %s%s%s"), var,
				op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
			}
			cmndspec->timeout = parse_timeout(val);
		    } else if (strcmp(var, "runchroot") == 0 && val != NULL) {
			if (cmndspec->runchroot != NULL) {
			    free(cmndspec->runchroot);
			    sudo_warnx(U_("duplicate sudoOption: %s%s%s"), var,
				op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
			}
			if ((cmndspec->runchroot = strdup(val)) == NULL)
			    break;
		    } else if (strcmp(var, "runcwd") == 0 && val != NULL) {
			if (cmndspec->runcwd != NULL) {
			    free(cmndspec->runcwd);
			    sudo_warnx(U_("duplicate sudoOption: %s%s%s"), var,
				op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
			}
			if ((cmndspec->runcwd = strdup(val)) == NULL)
			    break;
#ifdef HAVE_SELINUX
		    } else if (strcmp(var, "role") == 0 && val != NULL) {
			if (cmndspec->role != NULL) {
			    free(cmndspec->role);
			    sudo_warnx(U_("duplicate sudoOption: %s%s%s"), var,
				op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
			}
			if ((cmndspec->role = strdup(val)) == NULL)
			    break;
		    } else if (strcmp(var, "type") == 0 && val != NULL) {
			if (cmndspec->type != NULL) {
			    free(cmndspec->type);
			    sudo_warnx(U_("duplicate sudoOption: %s%s%s"), var,
				op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
			}
			if ((cmndspec->type = strdup(val)) == NULL)
			    break;
#endif /* HAVE_SELINUX */
#ifdef HAVE_PRIV_SET
		    } else if (strcmp(var, "privs") == 0 && val != NULL) {
			if (cmndspec->privs != NULL) {
			    free(cmndspec->privs);
			    sudo_warnx(U_("duplicate sudoOption: %s%s%s"), var,
				op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
			}
			if ((cmndspec->privs = strdup(val)) == NULL)
			    break;
		    } else if (strcmp(var, "limitprivs") == 0 && val != NULL) {
			if (cmndspec->limitprivs != NULL) {
			    free(cmndspec->limitprivs);
			    sudo_warnx(U_("duplicate sudoOption: %s%s%s"), var,
				op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
			}
			if ((cmndspec->limitprivs = strdup(val)) == NULL)
			    break;
#endif /* HAVE_PRIV_SET */
		    } else if (store_options) {
			if (!append_default(var, val, op, source,
			    &priv->defaults)) {
			    break;
			}
		    } else {
			/* Convert to tags. */
			bool converted = sudoers_defaults_to_tags(var, val, op,
			    &cmndspec->tags);
			if (!converted) {
			    if (warnings) {
				/* XXX - callback to process unsupported options. */
				if (val != NULL) {
				    sudo_warnx(U_("unable to convert sudoOption: %s%s%s"), var, op == '+' ? "+=" : op == '-' ? "-=" : "=", val);
				} else {
				    sudo_warnx(U_("unable to convert sudoOption: %s%s%s"), op == false ? "!" : "", var, "");
				}
			    }
			    continue;
			}
		    }
		}
		sudo_rcstr_delref(source);
		if (opt != NULL) {
		    /* Defer oom until we drop the ref on source. */
		    goto oom;
		}
	    }

	    /* So we can inherit previous values. */
	    prev_cmndspec = cmndspec;
	}

	/* Fill in command member now that options have been processed. */
	m->negated = negated;
	if (!sudo_ldap_extract_digests(&cmnd, &c->digests))
	    goto oom;
	if (strcmp(cmnd, "ALL") == 0) {
	    if (cmndspec->tags.setenv == UNSPEC)
		cmndspec->tags.setenv = IMPLIED;
	    m->type = ALL;
	} else {
	    char *args = strpbrk(cmnd, " \t");
	    if (args != NULL) {
		*args++ = '\0';
		if ((c->args = strdup(args)) == NULL)
		    goto oom;
	    }
	    if ((c->cmnd = strdup(cmnd)) == NULL)
		goto oom;
	    m->type = COMMAND;
	}
    }
    /* Negated commands take precedence so we insert them at the end. */
    TAILQ_CONCAT(&priv->cmndlist, &negated_cmnds, entries);

    debug_return_ptr(priv);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
bad:
    if (priv != NULL) {
	TAILQ_CONCAT(&priv->hostlist, &negated_hosts, entries);
	TAILQ_CONCAT(&priv->cmndlist, &negated_cmnds, entries);
	free_privilege(priv);
    }
    debug_return_ptr(NULL);
}

/* So ldap.c and sssd.c don't need to include gram.h */
struct member *
sudo_ldap_new_member_all(void)
{
    struct member *m;
    debug_decl(sudo_ldap_new_member_all, SUDOERS_DEBUG_LDAP);

    if ((m = calloc(1, sizeof(*m))) != NULL)
	m->type = ALL;
    debug_return_ptr(m);
}

/*
 * Determine length of query value after escaping characters
 * as per RFC 4515.
 */
size_t
sudo_ldap_value_len(const char *value)
{
    const char *s;
    size_t len = 0;

    for (s = value; *s != '\0'; s++) {
	switch (*s) {
	case '\\':
	case '(':
	case ')':
	case '*':
	    len += 2;
	    break;
	}
    }
    len += (size_t)(s - value);
    return len;
}

/*
 * Like strlcat() but escapes characters as per RFC 4515.
 */
size_t
sudo_ldap_value_cat(char * restrict dst, const char * restrict src, size_t size)
{
    char *d = dst;
    const char *s = src;
    size_t n = size;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
	d++;
    dlen = (size_t)(d - dst);
    n = size - dlen;

    if (n == 0)
	return dlen + strlen(s);
    while (*s != '\0') {
	switch (*s) {
	case '\\':
	    if (n < 3)
		goto done;
	    *d++ = '\\';
	    *d++ = '5';
	    *d++ = 'c';
	    n -= 3;
	    break;
	case '(':
	    if (n < 3)
		goto done;
	    *d++ = '\\';
	    *d++ = '2';
	    *d++ = '8';
	    n -= 3;
	    break;
	case ')':
	    if (n < 3)
		goto done;
	    *d++ = '\\';
	    *d++ = '2';
	    *d++ = '9';
	    n -= 3;
	    break;
	case '*':
	    if (n < 3)
		goto done;
	    *d++ = '\\';
	    *d++ = '2';
	    *d++ = 'a';
	    n -= 3;
	    break;
	default:
	    if (n < 1)
		goto done;
	    *d++ = *s;
	    n--;
	    break;
	}
	s++;
    }
done:
    *d = '\0';
    while (*s != '\0')
	s++;
    return dlen + (size_t)(s - src);	/* count does not include NUL */
}

/*
 * Like strdup() but escapes characters as per RFC 4515.
 */
char *
sudo_ldap_value_dup(const char *src)
{
    char *dst;
    size_t size;

    size = sudo_ldap_value_len(src) + 1;
    dst = malloc(size);
    if (dst == NULL)
	return NULL;

    *dst = '\0';
    if (sudo_ldap_value_cat(dst, src, size) >= size) {
	/* Should not be possible... */
	free(dst);
	dst = NULL;
    }
    return dst;
}
