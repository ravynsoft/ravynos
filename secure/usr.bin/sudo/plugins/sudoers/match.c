/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <sys/stat.h>
#ifdef HAVE_SYS_SYSTEMINFO_H
# include <sys/systeminfo.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#ifdef HAVE_NETGROUP_H
# include <netgroup.h>
#else
# include <netdb.h>
#endif /* HAVE_NETGROUP_H */
#include <dirent.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#ifdef HAVE_FNMATCH
# include <fnmatch.h>
#else
# include <compat/fnmatch.h>
#endif /* HAVE_FNMATCH */

#include <sudoers.h>
#include <gram.h>

/*
 * Check whether user described by pw matches member.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
user_matches(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const struct member *m)
{
    const struct sudoers_context *ctx = parse_tree->ctx;
    const char *lhost = parse_tree->lhost ? parse_tree->lhost : ctx->runas.host;
    const char *shost = parse_tree->shost ? parse_tree->shost : ctx->runas.shost;
    int matched = UNSPEC;
    struct alias *a;
    debug_decl(user_matches, SUDOERS_DEBUG_MATCH);

    switch (m->type) {
	case ALL:
	    matched = m->negated ? DENY : ALLOW;
	    break;
	case NETGROUP:
	    if (netgr_matches(parse_tree->nss, m->name,
		def_netgroup_tuple ? lhost : NULL,
		def_netgroup_tuple ? shost : NULL, pw->pw_name) == ALLOW)
		matched = m->negated ? DENY : ALLOW;
	    break;
	case USERGROUP:
	    if (usergr_matches(m->name, pw->pw_name, pw) == ALLOW)
		matched = m->negated ? DENY : ALLOW;
	    break;
	case ALIAS:
	    if ((a = alias_get(parse_tree, m->name, USERALIAS)) != NULL) {
		/* XXX */
		const int rc = userlist_matches(parse_tree, pw, &a->members);
		if (SPECIFIED(rc)) {
		    if (m->negated) {
			matched = rc == ALLOW ? DENY : ALLOW;
		    } else {
			matched = rc;
		    }
		}
		alias_put(a);
		break;
	    }
	    FALLTHROUGH;
	case WORD:
	    if (userpw_matches(m->name, pw->pw_name, pw) == ALLOW)
		matched = m->negated ? DENY : ALLOW;
	    break;
    }
    debug_return_int(matched);
}

/*
 * Check for user described by pw in a list of members.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
userlist_matches(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const struct member_list *list)
{
    struct member *m;
    int matched = UNSPEC;
    debug_decl(userlist_matches, SUDOERS_DEBUG_MATCH);

    TAILQ_FOREACH_REVERSE(m, list, member_list, entries) {
	matched = user_matches(parse_tree, pw, m);
	if (SPECIFIED(matched))
	    break;
    }
    debug_return_int(matched);
}

struct gid_list *
runas_getgroups(const struct sudoers_context *ctx)
{
    const struct passwd *pw;
    debug_decl(runas_getgroups, SUDOERS_DEBUG_MATCH);

    if (def_preserve_groups) {
	sudo_gidlist_addref(ctx->user.gid_list);
	debug_return_ptr(ctx->user.gid_list);
    }

    /* Only use results from a group db query, not the front end. */
    pw = ctx->runas.pw ? ctx->runas.pw : ctx->user.pw;
    debug_return_ptr(sudo_get_gidlist(pw, ENTRY_TYPE_QUERIED));
}

/*
 * Check whether the requested runas user matches user_list, the
 * user portion of a sudoers runaslist.  If user_list is NULL, a
 * list containing runas_default is used.
 * Returns ALLOW, DENY or UNSPEC.
 */
static int
runas_userlist_matches(const struct sudoers_parse_tree *parse_tree,
    const struct member_list *user_list, struct member **matching_user)
{
    const struct sudoers_context *ctx = parse_tree->ctx;
    const char *lhost = parse_tree->lhost ? parse_tree->lhost : ctx->runas.host;
    const char *shost = parse_tree->shost ? parse_tree->shost : ctx->runas.shost;
    int user_matched = UNSPEC;
    struct member *m;
    struct alias *a;
    debug_decl(runas_userlist_matches, SUDOERS_DEBUG_MATCH);

    TAILQ_FOREACH_REVERSE(m, user_list, member_list, entries) {
	switch (m->type) {
	    case ALL:
		user_matched = m->negated ? DENY : ALLOW;
		break;
	    case NETGROUP:
		if (netgr_matches(parse_tree->nss, m->name,
		    def_netgroup_tuple ? lhost : NULL,
		    def_netgroup_tuple ? shost : NULL,
		    ctx->runas.pw->pw_name) == ALLOW)
		    user_matched = m->negated ? DENY : ALLOW;
		break;
	    case USERGROUP:
		if (usergr_matches(m->name, ctx->runas.pw->pw_name, ctx->runas.pw) == ALLOW)
		    user_matched = m->negated ? DENY : ALLOW;
		break;
	    case ALIAS:
		a = alias_get(parse_tree, m->name, RUNASALIAS);
		if (a != NULL) {
		    const int rc = runas_userlist_matches(parse_tree,
			&a->members, matching_user);
		    if (SPECIFIED(rc)) {
			if (m->negated) {
			    user_matched = rc == ALLOW ? DENY : ALLOW;
			} else {
			    user_matched = rc;
			}
		    }
		    alias_put(a);
		    break;
		}
		FALLTHROUGH;
	    case WORD:
		if (userpw_matches(m->name, ctx->runas.pw->pw_name, ctx->runas.pw) == ALLOW)
		    user_matched = m->negated ? DENY : ALLOW;
		break;
	    case MYSELF:
		/*
		 * Only match a rule with an empty runas user if a group
		 * was specified on the command line without a user _or_
		 * the user specified their own name on the command line.
		 */
		if ((!ISSET(ctx->settings.flags, RUNAS_USER_SPECIFIED) &&
			ISSET(ctx->settings.flags, RUNAS_GROUP_SPECIFIED)) ||
			strcmp(ctx->user.name, ctx->runas.pw->pw_name) == 0)
		    user_matched = m->negated ? DENY : ALLOW;
		break;
	}
	if (SPECIFIED(user_matched)) {
	    if (matching_user != NULL && m->type != ALIAS)
		*matching_user = m;
	    break;
	}
    }
    debug_return_int(user_matched);
}

/*
 * Check whether the requested runas group matches group_list, the
 * group portion of a sudoers runaslist, or the runas user's groups.
 * Returns ALLOW, DENY or UNSPEC.
 */
static int
runas_grouplist_matches(const struct sudoers_parse_tree *parse_tree,
    const struct member_list *group_list, struct member **matching_group)
{
    const struct sudoers_context *ctx = parse_tree->ctx;
    int group_matched = UNSPEC;
    struct member *m;
    struct alias *a;
    debug_decl(runas_grouplist_matches, SUDOERS_DEBUG_MATCH);

    if (group_list != NULL) {
	TAILQ_FOREACH_REVERSE(m, group_list, member_list, entries) {
	    switch (m->type) {
		case ALL:
		    group_matched = m->negated ? DENY : ALLOW;
		    break;
		case ALIAS:
		    a = alias_get(parse_tree, m->name, RUNASALIAS);
		    if (a != NULL) {
			const int rc = runas_grouplist_matches(parse_tree,
			    &a->members, matching_group);
			if (SPECIFIED(rc)) {
			    if (m->negated) {
				group_matched = rc == ALLOW ? DENY : ALLOW;
			    } else {
				group_matched = rc;
			    }
			}
			alias_put(a);
			break;
		    }
		    FALLTHROUGH;
		case WORD:
		    if (group_matches(m->name, ctx->runas.gr) == ALLOW)
			group_matched = m->negated ? DENY : ALLOW;
		    break;
	    }
	    if (SPECIFIED(group_matched)) {
		if (matching_group != NULL && m->type != ALIAS)
		    *matching_group = m;
		break;
	    }
	}
    }
    if (!SPECIFIED(group_matched)) {
	struct gid_list *runas_groups;
	/*
	 * The runas group was not explicitly allowed by sudoers.
	 * Check whether it is one of the target user's groups.
	 */
	if (ctx->runas.pw->pw_gid == ctx->runas.gr->gr_gid) {
	    group_matched = ALLOW;	/* runas group matches passwd db */
	} else if ((runas_groups = runas_getgroups(ctx)) != NULL) {
	    int i;

	    for (i = 0; i < runas_groups->ngids; i++) {
		if (runas_groups->gids[i] == ctx->runas.gr->gr_gid) {
		    group_matched = ALLOW;	/* matched aux group vector */
		    break;
		}
	    }
	    sudo_gidlist_delref(runas_groups);
	}
    }

    debug_return_int(group_matched);
}

/*
 * Check whether the sudoers runaslist, composed of user_list and
 * group_list, matches the runas user/group requested by the user.
 * Either (or both) user_list and group_list may be NULL.
 * If user_list is NULL, a list containing runas_default is used.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
runaslist_matches(const struct sudoers_parse_tree *parse_tree,
    const struct member_list *user_list, const struct member_list *group_list,
    struct member **matching_user, struct member **matching_group)
{
    const struct sudoers_context *ctx = parse_tree->ctx;
    struct member_list _user_list = TAILQ_HEAD_INITIALIZER(_user_list);
    int user_matched, group_matched = UNSPEC;
    struct member m_user;
    debug_decl(runaslist_matches, SUDOERS_DEBUG_MATCH);

    /* If no runas user listed in sudoers, use the default value.  */
    if (user_list == NULL) {
	m_user.name = def_runas_default;
	m_user.type = WORD;
	m_user.negated = false;
	TAILQ_INSERT_HEAD(&_user_list, &m_user, entries);
	user_list = &_user_list;
	matching_user = NULL;
    }

    user_matched = runas_userlist_matches(parse_tree, user_list, matching_user);
    if (ISSET(ctx->settings.flags, RUNAS_GROUP_SPECIFIED)) {
	group_matched = runas_grouplist_matches(parse_tree, group_list,
	    matching_group);
    }

    if (user_matched == DENY || group_matched == DENY)
	debug_return_int(DENY);
    if (user_matched == group_matched || ctx->runas.gr == NULL)
	debug_return_int(user_matched);
    debug_return_int(UNSPEC);
}

/*
 * Check for lhost and shost in a list of members.
 * Returns ALLOW, DENY or UNSPEC.
 */
static int
hostlist_matches_int(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const char *lhost, const char *shost,
    const struct member_list *list)
{
    struct member *m;
    int matched = UNSPEC;
    debug_decl(hostlist_matches, SUDOERS_DEBUG_MATCH);

    TAILQ_FOREACH_REVERSE(m, list, member_list, entries) {
	matched = host_matches(parse_tree, pw, lhost, shost, m);
	if (SPECIFIED(matched))
	    break;
    }
    debug_return_int(matched);
}

/*
 * Check for ctx->runas.host and ctx->runas.shost in a list of members.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
hostlist_matches(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const struct member_list *list)
{
    const struct sudoers_context *ctx = parse_tree->ctx;
    const char *lhost = parse_tree->lhost ? parse_tree->lhost : ctx->runas.host;
    const char *shost = parse_tree->shost ? parse_tree->shost : ctx->runas.shost;

    return hostlist_matches_int(parse_tree, pw, lhost, shost, list);
}

/*
 * Check whether host or shost matches member.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
host_matches(const struct sudoers_parse_tree *parse_tree,
    const struct passwd *pw, const char *lhost, const char *shost,
    const struct member *m)
{
    struct alias *a;
    int ret = UNSPEC;
    debug_decl(host_matches, SUDOERS_DEBUG_MATCH);

    switch (m->type) {
	case ALL:
	    ret = m->negated ? DENY : ALLOW;
	    break;
	case NETGROUP:
	    if (netgr_matches(parse_tree->nss, m->name, lhost, shost,
		def_netgroup_tuple ? pw->pw_name : NULL) == ALLOW)
		ret = m->negated ? DENY : ALLOW;
	    break;
	case NTWKADDR:
	    if (addr_matches(m->name) == ALLOW)
		ret = m->negated ? DENY : ALLOW;
	    break;
	case ALIAS:
	    a = alias_get(parse_tree, m->name, HOSTALIAS);
	    if (a != NULL) {
		/* XXX */
		const int rc = hostlist_matches_int(parse_tree, pw, lhost,
		    shost, &a->members);
		if (SPECIFIED(rc)) {
		    if (m->negated) {
			ret = rc == ALLOW ? DENY : ALLOW;
		    } else {
			ret = rc;
		    }
		}
		alias_put(a);
		break;
	    }
	    FALLTHROUGH;
	case WORD:
	    if (hostname_matches(shost, lhost, m->name) == ALLOW)
		ret = m->negated ? DENY : ALLOW;
	    break;
    }
    sudo_debug_printf(SUDO_DEBUG_DEBUG,
	"host %s (%s) matches sudoers host %s%s: %s", lhost, shost,
	m->negated ? "!" : "", m->name ? m->name : "ALL",
	ret == ALLOW ? "ALLOW" : "DENY");
    debug_return_int(ret);
}

/*
 * Check for cmnd and args in a list of members.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
cmndlist_matches(const struct sudoers_parse_tree *parse_tree,
    const struct member_list *list, const char *runchroot,
    struct cmnd_info *info)
{
    struct member *m;
    int matched;
    debug_decl(cmndlist_matches, SUDOERS_DEBUG_MATCH);

    TAILQ_FOREACH_REVERSE(m, list, member_list, entries) {
	matched = cmnd_matches(parse_tree, m, runchroot, info);
	if (SPECIFIED(matched))
	    debug_return_int(matched);
    }
    debug_return_int(UNSPEC);
}

/*
 * Check cmnd and args.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
cmnd_matches(const struct sudoers_parse_tree *parse_tree,
    const struct member *m, const char *runchroot, struct cmnd_info *info)
{
    struct alias *a;
    struct sudo_command *c;
    int rc, matched = UNSPEC;
    debug_decl(cmnd_matches, SUDOERS_DEBUG_MATCH);

    switch (m->type) {
	case ALL:
	case COMMAND:
	    c = (struct sudo_command *)m->name;
	    if (command_matches(parse_tree->ctx, c->cmnd, c->args, runchroot,
		    info, &c->digests) == ALLOW)
		matched = m->negated ? DENY : ALLOW;
	    break;
	case ALIAS:
	    a = alias_get(parse_tree, m->name, CMNDALIAS);
	    if (a != NULL) {
		rc = cmndlist_matches(parse_tree, &a->members, runchroot, info);
		if (SPECIFIED(rc)) {
		    if (m->negated) {
			matched = rc == ALLOW ? DENY : ALLOW;
		    } else {
			matched = rc;
		    }
		}
		alias_put(a);
	    }
	    break;
    }
    debug_return_int(matched);
}

/*
 * Like cmnd_matches() but only matches against the ALL command.
 * Returns ALLOW, DENY or UNSPEC.
 */
int
cmnd_matches_all(const struct sudoers_parse_tree *parse_tree,
    const struct member *m, const char *runchroot, struct cmnd_info *info)
{
    const bool negated = m->negated;
    struct sudo_command *c;
    int matched = UNSPEC;
    struct alias *a;
    debug_decl(cmnd_matches_all, SUDOERS_DEBUG_MATCH);

    switch (m->type) {
	case ALL:
	    c = (struct sudo_command *)m->name;
	    if (command_matches(parse_tree->ctx, c->cmnd, c->args, runchroot,
		    info, &c->digests) == ALLOW)
		matched = negated ? DENY : ALLOW;
	    break;
	case ALIAS:
	    a = alias_get(parse_tree, m->name, CMNDALIAS);
	    if (a != NULL) {
		TAILQ_FOREACH_REVERSE(m, &a->members, member_list, entries) {
		    matched = cmnd_matches_all(parse_tree, m, runchroot, info);
		    if (SPECIFIED(matched)) {
			if (negated)
			    matched = matched == ALLOW ? DENY : ALLOW;
			break;
		    }
		}
		alias_put(a);
	    }
	    break;
    }
    debug_return_int(matched);
}

/*
 * Returns ALLOW if the hostname matches the pattern, else DENY
 */
int
hostname_matches(const char *shost, const char *lhost, const char *pattern)
{
    const char *host;
    int ret;
    debug_decl(hostname_matches, SUDOERS_DEBUG_MATCH);

    host = strchr(pattern, '.') != NULL ? lhost : shost;
    ret = DENY;
    if (has_meta(pattern)) {
	if (fnmatch(pattern, host, FNM_CASEFOLD) == 0)
	    ret = ALLOW;
    } else {
	if (strcasecmp(host, pattern) == 0)
	    ret = ALLOW;
    }
    debug_return_int(ret);
}

/*
 * Returns ALLOW if the user/uid from sudoers matches the specified user/uid,
 * else returns DENY.
 */
int
userpw_matches(const char *sudoers_user, const char *user, const struct passwd *pw)
{
    const char *errstr;
    int ret = DENY;
    uid_t uid;
    debug_decl(userpw_matches, SUDOERS_DEBUG_MATCH);

    if (pw != NULL && *sudoers_user == '#') {
	uid = (uid_t) sudo_strtoid(sudoers_user + 1, &errstr);
	if (errstr == NULL && uid == pw->pw_uid) {
	    ret = ALLOW;
	    goto done;
	}
    }
    if (def_case_insensitive_user) {
	if (strcasecmp(sudoers_user, user) == 0)
	    ret = ALLOW;
    } else {
	if (strcmp(sudoers_user, user) == 0)
	    ret = ALLOW;
    }
done:
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"user %s matches sudoers user %s: %s",
	user, sudoers_user, ret == ALLOW ? "ALLOW" : "DENY");
    debug_return_int(ret);
}

/*
 * Returns ALLOW if the group/gid from sudoers matches the specified group/gid,
 * else returns DENY.
 */
int
group_matches(const char *sudoers_group, const struct group *gr)
{
    const char *errstr;
    int ret = DENY;
    gid_t gid;
    debug_decl(group_matches, SUDOERS_DEBUG_MATCH);

    if (*sudoers_group == '#') {
	gid = (gid_t) sudo_strtoid(sudoers_group + 1, &errstr);
	if (errstr == NULL && gid == gr->gr_gid) {
	    ret = ALLOW;
	    goto done;
	}
    }
    if (def_case_insensitive_group) {
	if (strcasecmp(sudoers_group, gr->gr_name) == 0)
	    ret = ALLOW;
    } else {
	if (strcmp(sudoers_group, gr->gr_name) == 0)
	    ret = ALLOW;
    }
done:
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"group %s matches sudoers group %s: %s",
	gr->gr_name, sudoers_group, ret == ALLOW ? "ALLOW" : "DENY");
    debug_return_int(ret);
}

/*
 * Returns true if the given user belongs to the named group,
 * else returns false.
 */
int
usergr_matches(const char *group, const char *user, const struct passwd *pw)
{
    struct passwd *pw0 = NULL;
    int ret = DENY;
    debug_decl(usergr_matches, SUDOERS_DEBUG_MATCH);

    /* Make sure we have a valid usergroup, sudo style */
    if (*group++ != '%') {
	sudo_debug_printf(SUDO_DEBUG_DIAG, "user group %s has no leading '%%'",
	    group);
	goto done;
    }

    /* Query group plugin for %:name groups. */
    if (*group == ':' && def_group_plugin) {
	if (group_plugin_query(user, group + 1, pw) == true)
	    ret = ALLOW;
	goto done;
    }

    /* Look up user's primary gid in the passwd file. */
    if (pw == NULL) {
	if ((pw0 = sudo_getpwnam(user)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_DIAG, "unable to find %s in passwd db",
		user);
	    goto done;
	}
	pw = pw0;
    }

    if (user_in_group(pw, group)) {
	ret = ALLOW;
	goto done;
    }

    /* Query the group plugin for Unix groups too? */
    if (def_group_plugin && def_always_query_group_plugin) {
	if (group_plugin_query(user, group, pw) == true) {
	    ret = ALLOW;
	    goto done;
	}
    }

done:
    if (pw0 != NULL)
	sudo_pw_delref(pw0);

    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"user %s matches group %s: %s", user, group,
	ret == ALLOW ? "ALLOW" : "DENY");
    debug_return_int(ret);
}

#if defined(HAVE_GETDOMAINNAME) || defined(SI_SRPC_DOMAIN)
/*
 * Check the domain for invalid characters.
 * Linux getdomainname(2) returns (none) if no domain is set.
 */
static bool
valid_domain(const char *domain)
{
    const char *cp;
    debug_decl(valid_domain, SUDOERS_DEBUG_MATCH);

    for (cp = domain; *cp != '\0'; cp++) {
	/* Check for illegal characters, Linux may use "(none)". */
	if (*cp == '(' || *cp == ')' || *cp == ',' || *cp == ' ')
	    break;
    }
    if (cp == domain || *cp != '\0')
	debug_return_bool(false);
    debug_return_bool(true);
}

/*
 * Get NIS-style domain name and copy from static storage or NULL if none.
 */
const char *
sudo_getdomainname(void)
{
    static char *domain;
    static bool initialized;
    debug_decl(sudo_getdomainname, SUDOERS_DEBUG_MATCH);

    if (!initialized) {
	size_t host_name_max;
	int rc;

# ifdef _SC_HOST_NAME_MAX
	host_name_max = (size_t)sysconf(_SC_HOST_NAME_MAX);
	if (host_name_max == (size_t)-1)
# endif
	    host_name_max = 255;    /* POSIX and historic BSD */

	domain = malloc(host_name_max + 1);
	if (domain != NULL) {
	    domain[0] = '\0';
# ifdef SI_SRPC_DOMAIN
	    rc = sysinfo(SI_SRPC_DOMAIN, domain, host_name_max + 1);
# else
	    rc = getdomainname(domain, host_name_max + 1);
# endif
	    if (rc == -1 || !valid_domain(domain)) {
		/* Error or invalid domain name. */
		free(domain);
		domain = NULL;
	    }
	} else {
	    /* XXX - want to pass error back to caller */
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	}
	initialized = true;
    }
    debug_return_str(domain);
}
#else
const char *
sudo_getdomainname(void)
{
    debug_decl(sudo_getdomainname, SUDOERS_DEBUG_MATCH);
    debug_return_ptr(NULL);
}
#endif /* HAVE_GETDOMAINNAME || SI_SRPC_DOMAIN */

/*
 * Returns ALLOW if "host" and "user" belong to the netgroup "netgr",
 * else return DENY.  Either of "lhost", "shost" or "user" may be NULL
 * in which case that argument is not checked...
 */
int
netgr_matches(const struct sudo_nss *nss, const char *netgr,
    const char *lhost, const char *shost, const char *user)
{
    const char *domain;
    int ret = DENY;
    debug_decl(netgr_matches, SUDOERS_DEBUG_MATCH);

    if (!def_use_netgroups) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "netgroups are disabled");
	debug_return_int(DENY);
    }

    /* make sure we have a valid netgroup, sudo style */
    if (*netgr++ != '+') {
	sudo_debug_printf(SUDO_DEBUG_DIAG, "netgroup %s has no leading '+'",
	    netgr);
	debug_return_int(DENY);
    }

    /* get the domain name (if any) */
    domain = sudo_getdomainname();

    /* Use nss-specific innetgr() function if available. */
    if (nss != NULL && nss->innetgr != NULL) {
	switch (nss->innetgr(nss, netgr, lhost, user, domain)) {
	case 0:
	    if (lhost != shost) {
		if (nss->innetgr(nss, netgr, shost, user, domain) == 1)
		    ret = ALLOW;
	    }
	    goto done;
	case 1:
	    ret = ALLOW;
	    goto done;
	default:
	    /* Not supported, use system innetgr(3). */
	    break;
	}
    }

#ifdef HAVE_INNETGR
    /* Use system innetgr() function. */
    if (innetgr(netgr, lhost, user, domain) == 1) {
	ret = ALLOW;
    } else if (lhost != shost) {
	if (innetgr(netgr, shost, user, domain) == 1)
	    ret = ALLOW;
    }
#else
    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	"%s: no system netgroup support", __func__);
#endif /* HAVE_INNETGR */

done:
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"netgroup %s matches (%s|%s, %s, %s): %s", netgr, lhost ? lhost : "",
	shost ? shost : "", user ? user : "", domain ? domain : "",
	ret == ALLOW ? "ALLOW" : "DENY");

    debug_return_int(ret);
}
