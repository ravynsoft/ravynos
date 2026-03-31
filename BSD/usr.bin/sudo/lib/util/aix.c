/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2008, 2010-2016 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/resource.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <usersec.h>
#include <uinfo.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_util.h>

#ifdef HAVE_GETUSERATTR

#ifndef HAVE_SETRLIMIT64
# define setrlimit64(a, b) setrlimit(a, b)
# define rlimit64 rlimit
# define rlim64_t rlim_t
# define RLIM64_INFINITY RLIM_INFINITY
#endif /* HAVE_SETRLIMIT64 */

#ifndef RLIM_SAVED_MAX
# define RLIM_SAVED_MAX	RLIM64_INFINITY
#endif

struct aix_limit {
    int resource;
    const char *soft;
    const char *hard;
    int factor;
};

static struct aix_limit aix_limits[] = {
    { RLIMIT_FSIZE, S_UFSIZE, S_UFSIZE_HARD, 512 },
    { RLIMIT_CPU, S_UCPU, S_UCPU_HARD, 1 },
    { RLIMIT_DATA, S_UDATA, S_UDATA_HARD, 512 },
    { RLIMIT_STACK, S_USTACK, S_USTACK_HARD, 512 },
    { RLIMIT_RSS, S_URSS, S_URSS_HARD, 512 },
    { RLIMIT_CORE, S_UCORE, S_UCORE_HARD, 512 },
    { RLIMIT_NOFILE, S_UNOFILE, S_UNOFILE_HARD, 1 }
};

static int
aix_getlimit(const char *user, const char *lim, int *valp)
{
    debug_decl(aix_getlimit, SUDO_DEBUG_UTIL);

    if (getuserattr((char *)user, (char *)lim, valp, SEC_INT) != 0)
	debug_return_int(-1);
    debug_return_int(0);
}

static int
aix_setlimits(char *user)
{
    struct rlimit64 rlim;
    int val;
    size_t n;
    debug_decl(aix_setlimits, SUDO_DEBUG_UTIL);

    if (setuserdb(S_READ) != 0) {
	sudo_warn("%s", U_("unable to open userdb"));
	debug_return_int(-1);
    }

    /*
     * For each resource limit, get the soft/hard values for the user
     * and set those values via setrlimit64().  Must be run as euid 0.
     */
    for (n = 0; n < nitems(aix_limits); n++) {
	/*
	 * We have two strategies, depending on whether or not the
	 * hard limit has been defined.
	 */
	if (aix_getlimit(user, aix_limits[n].hard, &val) == 0) {
	    rlim.rlim_max = val == -1 ? RLIM64_INFINITY : (rlim64_t)val * aix_limits[n].factor;
	    if (aix_getlimit(user, aix_limits[n].soft, &val) == 0)
		rlim.rlim_cur = val == -1 ? RLIM64_INFINITY : (rlim64_t)val * aix_limits[n].factor;
	    else
		rlim.rlim_cur = rlim.rlim_max;	/* soft not specd, use hard */
	} else {
	    /* No hard limit set, try soft limit, if it exists. */
	    if (aix_getlimit(user, aix_limits[n].soft, &val) == -1)
		continue;
	    rlim.rlim_cur = val == -1 ? RLIM64_INFINITY : (rlim64_t)val * aix_limits[n].factor;

	    /* Set default hard limit as per limits(4). */
	    switch (aix_limits[n].resource) {
		case RLIMIT_CPU:
		case RLIMIT_FSIZE:
		    rlim.rlim_max = rlim.rlim_cur;
		    break;
		case RLIMIT_STACK:
		    rlim.rlim_max = 4194304UL * aix_limits[n].factor;
		    break;
		default:
		    rlim.rlim_max = RLIM64_INFINITY;
		    break;
	    }
	}
	(void)setrlimit64(aix_limits[n].resource, &rlim);
    }
    enduserdb();
    debug_return_int(0);
}

#ifdef HAVE_SETAUTHDB

# ifndef HAVE_AUTHDB_T
typedef char authdb_t[16];
# endif

/* The empty string means to access all defined authentication registries. */
static authdb_t old_registry;

# if defined(HAVE_DECL_SETAUTHDB) && !HAVE_DECL_SETAUTHDB
int setauthdb(authdb_t new, authdb_t old);
int getauthdb(authdb_t val);
# endif
# if defined(HAVE_DECL_USRINFO) && !HAVE_DECL_USRINFO
int usrinfo(int cmd, char *buf, int count);
# endif

/*
 * Look up authentication registry for user (SYSTEM in /etc/security/user) and
 * set it as the default for the process.  This ensures that password and
 * group lookups are made against the correct source (files, NIS, LDAP, etc).
 * Does not modify errno even on error since callers do not check return value.
 */
int
aix_getauthregistry_v1(char *user, char *saved_registry)
{
    int serrno = errno;
    int ret = -1;
    debug_decl(aix_getauthregistry, SUDO_DEBUG_UTIL);

    saved_registry[0] = '\0';
    if (user != NULL) {
	char *registry;

	if (setuserdb(S_READ) != 0) {
	    sudo_warn("%s", U_("unable to open userdb"));
	    goto done;
	}
	ret = getuserattr(user, (char *)S_REGISTRY, &registry, SEC_CHAR);
	if (ret == 0) {
	    /* sizeof(authdb_t) is guaranteed to be 16 */
	    if (strlcpy(saved_registry, registry, 16) >= 16) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "registry for user %s too long: %s", user, registry);
	    }
	    sudo_debug_printf(SUDO_DEBUG_INFO,
		"%s: saved authentication registry for user %s is %s",
		__func__, user, saved_registry);
	}
	enduserdb();
    } else {
	/* Get the process-wide registry. */
	ret = getauthdb(saved_registry);
    }
done:
    errno = serrno;
    debug_return_int(ret);
}

/*
 * Set the specified authentication registry for user (SYSTEM in
 * /etc/security/user) and set it as the default for the process.
 * This ensures that password and group lookups are made against
 * the correct source (files, NIS, LDAP, etc).
 * If registry is NULL, look it up based on the user name.
 * Does not modify errno even on error since callers do not check return value.
 */
int
aix_setauthdb_v1(char *user)
{
    return aix_setauthdb_v2(user, NULL);
}

int
aix_setauthdb_v2(char *user, char *registry)
{
    authdb_t regbuf;
    int serrno = errno;
    int ret = -1;
    debug_decl(aix_setauthdb, SUDO_DEBUG_UTIL);

    if (user != NULL) {
	/* Look up authentication registry if one is not provided. */
	if (registry == NULL) {
	    if (aix_getauthregistry(user, regbuf) != 0)
		goto done;
	    registry = regbuf;
	}
	ret = setauthdb(registry, old_registry);
	if (ret != 0) {
	    sudo_warn(U_("unable to switch to registry \"%s\" for %s"),
		registry, user);
	} else {
		sudo_debug_printf(SUDO_DEBUG_INFO,
		    "%s: setting authentication registry to %s",
		    __func__, registry);
	}
    }
done:
    errno = serrno;
    debug_return_int(ret);
}

/*
 * Restore the saved authentication registry, if any.
 * Does not modify errno even on error since callers do not check return value.
 */
int
aix_restoreauthdb_v1(void)
{
    int serrno = errno;
    int ret = 0;
    debug_decl(aix_setauthdb, SUDO_DEBUG_UTIL);

    if (setauthdb(old_registry, NULL) != 0) {
	sudo_warn("%s", U_("unable to restore registry"));
	ret = -1;
    } else {
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: setting authentication registry to %s",
	    __func__, old_registry);
    }
    errno = serrno;
    debug_return_int(ret);
}
#endif

int
aix_prep_user_v1(char *user, const char *tty)
{
    char *info;
    int len;
    debug_decl(aix_setauthdb, SUDO_DEBUG_UTIL);

    /* set usrinfo, like login(1) does */
    len = asprintf(&info, "NAME=%s%cLOGIN=%s%cLOGNAME=%s%cTTY=%s%c",
	user, '\0', user, '\0', user, '\0', tty ? tty : "", '\0');
    if (len == -1) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }
    (void)usrinfo(SETUINFO, info, len);
    free(info);

#ifdef HAVE_SETAUTHDB
    /* set authentication registry */
    if (aix_setauthdb(user, NULL) != 0)
	debug_return_int(-1);
#endif

    /* set resource limits */
    if (aix_setlimits(user) != 0)
	debug_return_int(-1);

    debug_return_int(0);
}
#endif /* HAVE_GETUSERATTR */
