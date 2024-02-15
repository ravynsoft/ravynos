/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
# include <sys/prctl.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include <sudo.h>

/*
 * Avoid using RLIM_INFINITY for the nofile soft limit to prevent
 * closefrom_fallback() from closing too many file descriptors.
 */
#if defined(OPEN_MAX) && OPEN_MAX > 256
# define SUDO_OPEN_MAX	OPEN_MAX
#else
# define SUDO_OPEN_MAX	256
#endif

#ifdef __LP64__
# define SUDO_STACK_MIN	(4 * 1024 * 1024)
#else
# define SUDO_STACK_MIN	(2 * 1024 * 1024)
#endif

#ifdef HAVE_SETRLIMIT64
# define getrlimit(a, b) getrlimit64((a), (b))
# define setrlimit(a, b) setrlimit64((a), (b))
# define rlimit rlimit64
# define rlim_t rlim64_t
# undef RLIM_INFINITY
# define RLIM_INFINITY RLIM64_INFINITY
#endif /* HAVE_SETRLIMIT64 */

/* Older BSD systems have RLIMIT_VMEM, not RLIMIT_AS. */
#if !defined(RLIMIT_AS) && defined(RLIMIT_VMEM)
# define RLIMIT_AS RLIMIT_VMEM
#endif

/*
 * macOS doesn't allow nofile soft limit to be infinite or
 * the stack hard limit to be infinite.
 * Linux containers have a problem with an infinite stack soft limit.
 */
static struct rlimit stack_fallback = { SUDO_STACK_MIN, 65532 * 1024 };

static struct saved_limit {
    const char *name;		/* rlimit_foo in lower case */
    int resource;		/* RLIMIT_FOO definition */
    bool override;		/* override limit while sudo executes? */
    bool saved;			/* true if we were able to get the value */
    bool policy;		/* true if policy specified an rlimit */
    bool preserve;		/* true if policy says to preserve user limit */
    rlim_t minlimit;		/* only modify limit if less than this value */
    struct rlimit *fallback;	/* fallback if we fail to set to newlimit */
    struct rlimit newlimit;	/* new limit to use if override is true */
    struct rlimit oldlimit;	/* original limit, valid if saved is true */
    struct rlimit policylimit;	/* limit from policy, valid if policy is true */
} saved_limits[] = {
#ifdef RLIMIT_AS
    {
	"rlimit_as",
	RLIMIT_AS,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	1 * 1024 * 1024 * 1024,			/* minlimit */
	NULL,					/* fallback */
	{ RLIM_INFINITY, RLIM_INFINITY }	/* newlimit */
    },
#endif
    {
	"rlimit_core",
	RLIMIT_CORE,
	false					/* override */
    },
    {
	"rlimit_cpu",
	RLIMIT_CPU,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	RLIM_INFINITY,				/* minlimit */
	NULL,
	{ RLIM_INFINITY, RLIM_INFINITY }
    },
    {
	"rlimit_data",
	RLIMIT_DATA,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	1 * 1024 * 1024 * 1024,			/* minlimit */
	NULL,
	{ RLIM_INFINITY, RLIM_INFINITY }
    },
    {
	"rlimit_fsize",
	RLIMIT_FSIZE,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	RLIM_INFINITY,				/* minlimit */
	NULL,
	{ RLIM_INFINITY, RLIM_INFINITY }
    },
#ifdef RLIMIT_LOCKS
    {
	"rlimit_locks",
	RLIMIT_LOCKS,
	false					/* override */
    },
#endif
#ifdef RLIMIT_MEMLOCK
    {
	"rlimit_memlock",
	RLIMIT_MEMLOCK,
	false					/* override */
    },
#endif
    {
	"rlimit_nofile",
	RLIMIT_NOFILE,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	SUDO_OPEN_MAX,				/* minlimit */
	NULL,
	{ SUDO_OPEN_MAX, RLIM_INFINITY }
    },
#ifdef RLIMIT_NPROC
    {
	"rlimit_nproc",
	RLIMIT_NPROC,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	RLIM_INFINITY,				/* minlimit */
	NULL,
	{ RLIM_INFINITY, RLIM_INFINITY }
    },
#endif
#ifdef RLIMIT_RSS
    {
	"rlimit_rss",
	RLIMIT_RSS,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	RLIM_INFINITY,				/* minlimit */
	NULL,
	{ RLIM_INFINITY, RLIM_INFINITY }
    },
#endif
    {
	"rlimit_stack",
	RLIMIT_STACK,
	true,					/* override */
	false,					/* saved */
	false,					/* policy */
	false,					/* preserve */
	SUDO_STACK_MIN,				/* minlimit */
	&stack_fallback,
	{ SUDO_STACK_MIN, RLIM_INFINITY }
    }
};

static struct rlimit corelimit;
static bool coredump_disabled;
#ifdef __linux__
static struct rlimit nproclimit;
static int dumpflag;
#endif

/*
 * Disable core dumps to avoid dropping a core with user password in it.
 * Not all operating systems disable core dumps for setuid processes.
 */
void
disable_coredump(void)
{
    debug_decl(disable_coredump, SUDO_DEBUG_UTIL);

    if (getrlimit(RLIMIT_CORE, &corelimit) == 0) {
	/*
	 * Set the soft limit to 0 but leave the existing hard limit.
	 * On Linux, we need CAP_SYS_RESOURCE to raise the hard limit
	 * which may not be the case in, e.g. an unprivileged container.
	 */
	struct rlimit rl = corelimit;
	rl.rlim_cur = 0;
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "RLIMIT_CORE [%lld, %lld] -> [%lld, %lld]",
	    (long long)corelimit.rlim_cur, (long long)corelimit.rlim_max,
	    (long long)rl.rlim_cur, (long long)rl.rlim_max);
	if (setrlimit(RLIMIT_CORE, &rl) == -1) {
	    sudo_warn("setrlimit(RLIMIT_CORE)");
	} else {
	    coredump_disabled = true;
#ifdef __linux__
	    /* On Linux, also set PR_SET_DUMPABLE to zero (reset by execve). */
	    if ((dumpflag = prctl(PR_GET_DUMPABLE, 0, 0, 0, 0)) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "prctl(PR_GET_DUMPABLE, 0, 0, 0, 0)");
		dumpflag = 0;
	    }
	    if (prctl(PR_SET_DUMPABLE, 0, 0, 0, 0) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "prctl(PR_SET_DUMPABLE, 0, 0, 0, 0)");
	    }
#endif /* __linux__ */
	}
    } else {
	sudo_warn("getrlimit(RLIMIT_CORE)");
    }

    debug_return;
}

/*
 * Restore core resource limit before executing the command.
 */
static void
restore_coredump(void)
{
    debug_decl(restore_coredump, SUDO_DEBUG_UTIL);

    if (coredump_disabled) {
	/*
	 * Do not warn about a failure to restore the core dump size limit.
	 * This is mostly harmless and should not happen in practice.
	 */
	if (setrlimit(RLIMIT_CORE, &corelimit) == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		"setrlimit(RLIMIT_CORE, [%lld, %lld])",
		(long long)corelimit.rlim_cur, (long long)corelimit.rlim_max);
	}
#ifdef __linux__
	if (prctl(PR_SET_DUMPABLE, dumpflag, 0, 0, 0) == -1) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		"prctl(PR_SET_DUMPABLE, %d, 0, 0, 0)", dumpflag);
	}
#endif /* __linux__ */
    }
    debug_return;
}

/*
 * Unlimit the number of processes since Linux's setuid() will
 * apply resource limits when changing uid and return EAGAIN if
 * nproc would be exceeded by the uid switch.
 *
 * This function is called *after* session setup and before the
 * final setuid() call.
 */
void
unlimit_nproc(void)
{
#ifdef __linux__
    struct rlimit rl = { RLIM_INFINITY, RLIM_INFINITY };
    debug_decl(unlimit_nproc, SUDO_DEBUG_UTIL);

    if (getrlimit(RLIMIT_NPROC, &nproclimit) != 0)
	sudo_warn("getrlimit(RLIMIT_NPROC)");
    sudo_debug_printf(SUDO_DEBUG_INFO, "RLIMIT_NPROC [%lld, %lld] -> [inf, inf]",
	(long long)nproclimit.rlim_cur, (long long)nproclimit.rlim_max);
    if (setrlimit(RLIMIT_NPROC, &rl) == -1) {
	rl.rlim_cur = rl.rlim_max = nproclimit.rlim_max;
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "RLIMIT_NPROC [%lld, %lld] -> [%lld, %lld]",
	    (long long)nproclimit.rlim_cur, (long long)nproclimit.rlim_max,
	    (long long)rl.rlim_cur, (long long)rl.rlim_max);
	if (setrlimit(RLIMIT_NPROC, &rl) != 0)
	    sudo_warn("setrlimit(RLIMIT_NPROC)");
    }
    debug_return;
#endif /* __linux__ */
}

/*
 * Restore saved value of RLIMIT_NPROC before execve().
 */
void
restore_nproc(void)
{
#ifdef __linux__
    debug_decl(restore_nproc, SUDO_DEBUG_UTIL);

    if (setrlimit(RLIMIT_NPROC, &nproclimit) != 0) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "setrlimit(RLIMIT_NPROC, [%lld, %lld])",
	    (long long)nproclimit.rlim_cur, (long long)nproclimit.rlim_max);
    }

    debug_return;
#endif /* __linux__ */
}

/*
 * Unlimit resource limits so sudo is not limited by, e.g.
 * stack, data or file table sizes.
 */
void
unlimit_sudo(void)
{
    unsigned int idx;
    int pass, rc;
    debug_decl(unlimit_sudo, SUDO_DEBUG_UTIL);

    /* Set resource limits to unlimited and stash the old values. */
    for (idx = 0; idx < nitems(saved_limits); idx++) {
	struct saved_limit *lim = &saved_limits[idx];
	if (getrlimit(lim->resource, &lim->oldlimit) == -1)
	    continue;
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "getrlimit(%s) -> [%lld, %lld]", lim->name,
	    (long long)lim->oldlimit.rlim_cur,
	    (long long)lim->oldlimit.rlim_max);
	lim->saved = true;

	/* Only override the existing limit if it is smaller than minlimit. */
	if (lim->minlimit != RLIM_INFINITY) {
	    if (lim->oldlimit.rlim_cur >= lim->minlimit)
		lim->override = false;
	}
	if (!lim->override)
	    continue;

	for (pass = 0; pass < 2; pass++) {
	    if (lim->newlimit.rlim_cur != RLIM_INFINITY) {
		/* Don't reduce the soft resource limit. */
		if (lim->oldlimit.rlim_cur == RLIM_INFINITY ||
			lim->oldlimit.rlim_cur > lim->newlimit.rlim_cur)
		    lim->newlimit.rlim_cur = lim->oldlimit.rlim_cur;
	    }
	    if (lim->newlimit.rlim_max != RLIM_INFINITY) {
		/* Don't reduce the hard resource limit. */
		if (lim->oldlimit.rlim_max == RLIM_INFINITY ||
			lim->oldlimit.rlim_max > lim->newlimit.rlim_max)
		    lim->newlimit.rlim_max = lim->oldlimit.rlim_max;
	    }
	    if ((rc = setrlimit(lim->resource, &lim->newlimit)) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "setrlimit(%s, [%lld, %lld])", lim->name,
		    (long long)lim->newlimit.rlim_cur,
		    (long long)lim->newlimit.rlim_max);
		if (pass == 0 && lim->fallback != NULL) {
		    /* Try again using fallback values. */
		    lim->newlimit.rlim_cur = lim->fallback->rlim_cur;
		    lim->newlimit.rlim_max = lim->fallback->rlim_max;
		    continue;
		}
	    }
	    break;
	}
	if (rc == -1) {
	    /* Try setting new rlim_cur to old rlim_max. */
	    lim->newlimit.rlim_cur = lim->oldlimit.rlim_max;
	    lim->newlimit.rlim_max = lim->oldlimit.rlim_max;
	    if ((rc = setrlimit(lim->resource, &lim->newlimit)) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "setrlimit(%s, [%lld, %lld])", lim->name,
		    (long long)lim->newlimit.rlim_cur,
		    (long long)lim->newlimit.rlim_max);
	    }
	}
	if (rc == -1)
	    sudo_warn("setrlimit(%s)", lim->name);
    }

    debug_return;
}

/*
 * Restore resource limits modified by unlimit_sudo() and disable_coredump().
 */
void
restore_limits(void)
{
    unsigned int idx;
    debug_decl(restore_limits, SUDO_DEBUG_UTIL);

    /* Restore resource limits to saved values. */
    for (idx = 0; idx < nitems(saved_limits); idx++) {
	struct saved_limit *lim = &saved_limits[idx];
	if (lim->override && lim->saved) {
	    struct rlimit rl = lim->oldlimit;
	    int i, rc;

	    for (i = 0; i < 10; i++) {
		rc = setrlimit(lim->resource, &rl);
		if (rc != -1 || errno != EINVAL)
		    break;

		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "setrlimit(%s, [%lld, %lld])", lim->name,
		    (long long)rl.rlim_cur, (long long)rl.rlim_max);

		/*
		 * Soft limit could be lower than current resource usage.
		 * This can be an issue on NetBSD with RLIMIT_STACK and ASLR.
		 */
		if (rl.rlim_cur > LLONG_MAX / 2)
		    break;
		rl.rlim_cur *= 2;
		if (lim->newlimit.rlim_cur != RLIM_INFINITY &&
			rl.rlim_cur > lim->newlimit.rlim_cur) {
		    rl.rlim_cur = lim->newlimit.rlim_cur;
		}
		if (rl.rlim_max != RLIM_INFINITY &&
			rl.rlim_cur > rl.rlim_max) {
		    rl.rlim_max = rl.rlim_cur;
		}
		rc = setrlimit(lim->resource, &rl);
		if (rc != -1 || errno != EINVAL)
		    break;
	    }
	    if (rc == -1)
		sudo_warn("setrlimit(%s)", lim->name);
	}
    }
    restore_coredump();

    debug_return;
}

static bool
store_rlimit(const char *str, rlim_t *val, bool soft)
{
    const size_t inflen = sizeof("infinity") - 1;
    debug_decl(store_rlimit, SUDO_DEBUG_UTIL);

    if (isdigit((unsigned char)*str)) {
	unsigned long long ullval = 0;
	char *ep;

	errno = 0;
#ifdef HAVE_STRTOULL
	ullval = strtoull(str, &ep, 10);
	if (str == ep || (errno == ERANGE && ullval == ULLONG_MAX))
	    debug_return_bool(false);
#else
	ullval = strtoul(str, &ep, 10);
	if (str == ep || (errno == ERANGE && ullval == ULONG_MAX))
	    debug_return_bool(false);
#endif
	if (*ep == '\0' || (soft && *ep == ',')) {
	    *val = ullval;
	    debug_return_bool(true);
	}
	goto done;
    }
    if (strncmp(str, "infinity", inflen) == 0) {
	if (str[inflen] == '\0' || (soft && str[inflen] == ',')) {
	    *val = RLIM_INFINITY;
	    debug_return_bool(true);
	}
    }
done:
    debug_return_bool(false);
}

static bool
set_policy_rlimit(int resource, const char *val)
{
    unsigned int idx;
    debug_decl(set_policy_rlimit, SUDO_DEBUG_UTIL);

    for (idx = 0; idx < nitems(saved_limits); idx++) {
	struct saved_limit *lim = &saved_limits[idx];
	const char *hard, *soft = val;

	if (lim->resource != resource)
	    continue;

	if (strcmp(val, "default") == 0) {
	    /* Use system-assigned limit set by begin_session(). */
	    lim->policy = false;
	    lim->preserve = false;
	    debug_return_bool(true);
	}
	if (strcmp(val, "user") == 0) {
	    /* Preserve invoking user's limit. */
	    lim->policy = false;
	    lim->preserve = true;
	    debug_return_bool(true);
	}

	/*
	 * Expect limit in the form "soft,hard" or "limit" (both soft+hard).
	 */
	hard = strchr(val, ',');
	if (hard != NULL)
	    hard++;
	else
	    hard = soft;

	if (store_rlimit(soft, &lim->policylimit.rlim_cur, true) &&
		store_rlimit(hard, &lim->policylimit.rlim_max, false)) {
	    lim->policy = true;
	    lim->preserve = false;
	    debug_return_bool(true);
	}
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "%s: invalid rlimit: %s", lim->name, val);
	debug_return_bool(false);
    }
    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	"invalid resource limit: %d", resource);
    debug_return_bool(false);
}

bool
parse_policy_rlimit(const char *str)
{
    bool ret = false;
    debug_decl(parse_policy_rlimit, SUDO_DEBUG_UTIL);

#ifdef RLIMIT_AS
    if (strncmp(str, "as=", sizeof("as=") - 1) == 0) {
	str += sizeof("as=") - 1;
	ret = set_policy_rlimit(RLIMIT_AS, str);
    } else
#endif
#ifdef RLIMIT_CORE
    if (strncmp(str, "core=", sizeof("core=") - 1) == 0) {
	str += sizeof("core=") - 1;
	ret = set_policy_rlimit(RLIMIT_CORE, str);
    } else
#endif
#ifdef RLIMIT_CPU
    if (strncmp(str, "cpu=", sizeof("cpu=") - 1) == 0) {
	str += sizeof("cpu=") - 1;
	ret = set_policy_rlimit(RLIMIT_CPU, str);
    } else
#endif
#ifdef RLIMIT_DATA
    if (strncmp(str, "data=", sizeof("data=") - 1) == 0) {
	str += sizeof("data=") - 1;
	ret = set_policy_rlimit(RLIMIT_DATA, str);
    } else
#endif
#ifdef RLIMIT_FSIZE
    if (strncmp(str, "fsize=", sizeof("fsize=") - 1) == 0) {
	str += sizeof("fsize=") - 1;
	ret = set_policy_rlimit(RLIMIT_FSIZE, str);
    } else
#endif
#ifdef RLIMIT_LOCKS
    if (strncmp(str, "locks=", sizeof("locks=") - 1) == 0) {
	str += sizeof("locks=") - 1;
	ret = set_policy_rlimit(RLIMIT_LOCKS, str);
    } else
#endif
#ifdef RLIMIT_MEMLOCK
    if (strncmp(str, "memlock=", sizeof("memlock=") - 1) == 0) {
	str += sizeof("memlock=") - 1;
	ret = set_policy_rlimit(RLIMIT_MEMLOCK, str);
    } else
#endif
#ifdef RLIMIT_NOFILE
    if (strncmp(str, "nofile=", sizeof("nofile=") - 1) == 0) {
	str += sizeof("nofile=") - 1;
	ret = set_policy_rlimit(RLIMIT_NOFILE, str);
    } else
#endif
#ifdef RLIMIT_NPROC
    if (strncmp(str, "nproc=", sizeof("nproc=") - 1) == 0) {
	str += sizeof("nproc=") - 1;
	ret = set_policy_rlimit(RLIMIT_NPROC, str);
    } else
#endif
#ifdef RLIMIT_RSS
    if (strncmp(str, "rss=", sizeof("rss=") - 1) == 0) {
	str += sizeof("rss=") - 1;
	ret = set_policy_rlimit(RLIMIT_RSS, str);
    } else
#endif
#ifdef RLIMIT_STACK
    if (strncmp(str, "stack=", sizeof("stack=") - 1) == 0) {
	str += sizeof("stack=") - 1;
	ret = set_policy_rlimit(RLIMIT_STACK, str);
    }
#endif
    debug_return_bool(ret);
}

/*
 * Set resource limits as specified by the security policy (if any).
 * This should be run as part of the session setup but after PAM,
 * login.conf, etc.
 */
void
set_policy_rlimits(void)
{
    unsigned int idx;
    debug_decl(set_policy_rlimits, SUDO_DEBUG_UTIL);

    for (idx = 0; idx < nitems(saved_limits); idx++) {
	struct saved_limit *lim = &saved_limits[idx];
	struct rlimit *rl;
	int rc;

	if (!lim->policy && (!lim->preserve || !lim->saved))
	    continue;

	rl = lim->preserve ? &lim->oldlimit : &lim->policylimit;
	if ((rc = setrlimit(lim->resource, rl)) == 0) {
	    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
		"setrlimit(%s, [%lld, %lld])", lim->name,
		(long long)rl->rlim_cur, (long long)rl->rlim_max);
	    continue;
	}

	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO|SUDO_DEBUG_ERRNO,
	    "setrlimit(%s, [%lld, %lld])", lim->name,
	    (long long)rl->rlim_cur, (long long)rl->rlim_max);

	if (rl->rlim_cur > lim->oldlimit.rlim_max || rl->rlim_max > lim->oldlimit.rlim_max) {
	    /* Try setting policy rlim_cur to old rlim_max. */
	    if (rl->rlim_cur > lim->oldlimit.rlim_max)
		rl->rlim_cur = lim->oldlimit.rlim_max;
	    if (rl->rlim_max > lim->oldlimit.rlim_max)
		rl->rlim_max = lim->oldlimit.rlim_max;
	    if ((rc = setrlimit(lim->resource, rl)) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "setrlimit(%s, [%lld, %lld])", lim->name,
		    (long long)rl->rlim_cur, (long long)rl->rlim_max);
	    }
	}
	if (rc == -1)
	    sudo_warn("setrlimit(%s)", lim->name);
    }

    debug_return;
}

size_t
serialize_rlimits(char **info, size_t info_max)
{
    char *str;
    size_t idx, nstored = 0;
    debug_decl(serialize_rlimits, SUDO_DEBUG_UTIL);

    for (idx = 0; idx < nitems(saved_limits); idx++) {
	const struct saved_limit *lim = &saved_limits[idx];
	const struct rlimit *rl = &lim->oldlimit;
	char curlim[STRLEN_MAX_UNSIGNED(unsigned long long) + 1];
	char maxlim[STRLEN_MAX_UNSIGNED(unsigned long long) + 1];

	if (!lim->saved)
	    continue;

	if (nstored == info_max)
	    goto oom;

	if (rl->rlim_cur == RLIM_INFINITY) {
	    strlcpy(curlim, "infinity", sizeof(curlim));
	} else {
	    snprintf(curlim, sizeof(curlim), "%llu",
		(unsigned long long)rl->rlim_cur);
	}
	if (rl->rlim_max == RLIM_INFINITY) {
	    strlcpy(maxlim, "infinity", sizeof(maxlim));
	} else {
	    snprintf(maxlim, sizeof(maxlim), "%llu",
		(unsigned long long)rl->rlim_max);
	}
	if (asprintf(&str, "%s=%s,%s", lim->name, curlim, maxlim) == -1)
	    goto oom;
	info[nstored++] = str;
    }
    debug_return_size_t(nstored);
oom:
    while (nstored)
	free(info[--nstored]);
    debug_return_size_t((size_t)-1);
}
