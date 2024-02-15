/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2000-2005, 2007-2023
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#ifdef HAVE_LOGIN_CAP_H
# include <login_cap.h>
# ifndef LOGIN_SETENV
#  define LOGIN_SETENV	0
# endif
#endif /* HAVE_LOGIN_CAP_H */
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>

#include <sudoers.h>

/*
 * Flags used in rebuild_env()
 */
#undef DID_TERM
#define DID_TERM	0x00000001
#undef DID_PATH
#define DID_PATH	0x00000002
#undef DID_HOME
#define DID_HOME	0x00000004
#undef DID_SHELL
#define DID_SHELL	0x00000008
#undef DID_LOGNAME
#define DID_LOGNAME	0x00000010
#undef DID_USER
#define DID_USER    	0x00000020
#undef DID_LOGIN
#define DID_LOGIN   	0x00000040
#undef DID_MAIL
#define DID_MAIL   	0x00000080
#undef DID_MAX
#define DID_MAX    	0x0000ffff

#undef KEPT_TERM
#define KEPT_TERM	0x00010000
#undef KEPT_PATH
#define KEPT_PATH	0x00020000
#undef KEPT_HOME
#define KEPT_HOME	0x00040000
#undef KEPT_SHELL
#define KEPT_SHELL	0x00080000
#undef KEPT_LOGNAME
#define KEPT_LOGNAME	0x00100000
#undef KEPT_USER
#define KEPT_USER    	0x00200000
#undef KEPT_LOGIN
#define KEPT_LOGIN	0x00400000
#undef KEPT_MAIL
#define KEPT_MAIL	0x00800000
#undef KEPT_MAX
#define KEPT_MAX    	0xffff0000

/*
 * AIX sets the LOGIN environment variable too.
 */
#ifdef _AIX
# define KEPT_USER_VARIABLES (KEPT_LOGIN|KEPT_LOGNAME|KEPT_USER)
#else
# define KEPT_USER_VARIABLES (KEPT_LOGNAME|KEPT_USER)
#endif

/*
 * Functions to open, close and parse an environment file, either
 * a system file such as /etc/environment or one specified in sudoers.
 */
struct sudoers_env_file {
    void * (*open)(const char *);
    void   (*close)(void *);
    char * (*next)(void *, int *);
};

/*
 * State for a local environment file.
 */
struct env_file_local {
    FILE *fp;
    char *line;
    size_t linesize;
};

struct environment {
    char **envp;		/* pointer to the new environment */
    char **old_envp;		/* pointer the old environment we allocated */
    size_t env_size;		/* size of new_environ in char **'s */
    size_t env_len;		/* number of slots used, not counting NULL */
};

/*
 * Copy of the sudo-managed environment.
 */
static struct environment env;

/*
 * Default table of "bad" variables to remove from the environment.
 * XXX - how to omit TERMCAP if it starts with '/'?
 */
static const char *initial_badenv_table[] = {
    "IFS",
    "CDPATH",
    "LOCALDOMAIN",
    "RES_OPTIONS",
    "HOSTALIASES",
    "NLSPATH",
    "PATH_LOCALE",
    "LD_*",
    "_RLD*",
#ifdef __hpux
    "SHLIB_PATH",
#endif /* __hpux */
#ifdef _AIX
    "LDR_*",
    "LIBPATH",
    "AUTHSTATE",
#endif
#ifdef __APPLE__
    "DYLD_*",
#endif
#ifdef HAVE_KERB5
    "KRB5_CONFIG*",
    "KRB5_KTNAME",
#endif /* HAVE_KERB5 */
#ifdef HAVE_SECURID
    "VAR_ACE",
    "USR_ACE",
    "DLC_ACE",
#endif /* HAVE_SECURID */
    "TERMINFO",			/* terminfo, exclusive path to terminfo files */
    "TERMINFO_DIRS",		/* terminfo, path(s) to terminfo files */
    "TERMPATH",			/* termcap, path(s) to termcap files */
    "TERMCAP",			/* XXX - only if it starts with '/' */
    "ENV",			/* ksh, file to source before script runs */
    "BASH_ENV",			/* bash, file to source before script runs */
    "PS4",			/* bash, prefix for lines in xtrace mode */
    "GLOBIGNORE",		/* bash, globbing patterns to ignore */
    "BASHOPTS",			/* bash, initial "shopt -s" options */
    "SHELLOPTS",		/* bash, initial "set -o" options */
    "JAVA_TOOL_OPTIONS",	/* java, extra command line options */
    "PERLIO_DEBUG",		/* perl, debugging output file */
    "PERLLIB",			/* perl, search path for modules/includes */
    "PERL5LIB",			/* perl 5, search path for modules/includes */
    "PERL5OPT",			/* perl 5, extra command line options */
    "PERL5DB",			/* perl 5, command used to load debugger */
    "FPATH",			/* ksh, search path for functions */
    "NULLCMD",			/* zsh, command for null file redirection */
    "READNULLCMD",		/* zsh, command for null file redirection */
    "ZDOTDIR",			/* zsh, search path for dot files */
    "TMPPREFIX",		/* zsh, prefix for temporary files */
    "PYTHONHOME",		/* python, module search path */
    "PYTHONPATH",		/* python, search path */
    "PYTHONINSPECT",		/* python, allow inspection */
    "PYTHONUSERBASE",		/* python, per user site-packages directory */
    "RUBYLIB",			/* ruby, library load path */
    "RUBYOPT",			/* ruby, extra command line options */
    "*=()*",			/* bash functions */
    NULL
};

/*
 * Default table of variables to check for '%' and '/' characters.
 */
static const char *initial_checkenv_table[] = {
    "COLORTERM",
    "LANG",
    "LANGUAGE",
    "LC_*",
    "LINGUAS",
    "TERM",
    "TZ",
    NULL
};

/*
 * Default table of variables to preserve in the environment.
 */
static const char *initial_keepenv_table[] = {
    "COLORS",
    "DISPLAY",
    "HOSTNAME",
    "KRB5CCNAME",
    "LS_COLORS",
    "PATH",
    "PS1",
    "PS2",
    "XAUTHORITY",
    "XAUTHORIZATION",
    "XDG_CURRENT_DESKTOP",
    NULL
};

/*
 * Free our copy (or copies) of the environment.
 * This function is only safe to call after the command has executed.
 */
void
env_free(void)
{
    sudoers_gc_remove(GC_PTR, env.envp);
    free(env.envp);
    sudoers_gc_remove(GC_PTR, env.old_envp);
    free(env.old_envp);
    memset(&env, 0, sizeof(env));
}

/*
 * Initialize env based on envp.
 */
bool
env_init(char * const envp[])
{
    char * const *ep;
    size_t len;
    debug_decl(env_init, SUDOERS_DEBUG_ENV);

    if (envp == NULL) {
	/* Free the old envp we allocated, if any. */
	sudoers_gc_remove(GC_PTR, env.old_envp);
	free(env.old_envp);

	/*
	 * Reset to initial state but keep a pointer to what we allocated
	 * since it will be passed to execve(2).
	 */
	env.old_envp = env.envp;
	env.envp = NULL;
	env.env_size = 0;
	env.env_len = 0;
    } else {
	/* Make private copy of envp. */
	for (ep = envp; *ep != NULL; ep++)
	    continue;
	len = (size_t)(ep - envp);

	env.env_len = len;
	env.env_size = len + 1 + 128;
	env.envp = reallocarray(NULL, env.env_size, sizeof(char *));
	if (env.envp == NULL) {
	    env.env_size = 0;
	    env.env_len = 0;
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return_bool(false);
	}
	sudoers_gc_add(GC_PTR, env.envp);
#ifdef ENV_DEBUG
	memset(env.envp, 0, env.env_size * sizeof(char *));
#endif
	memcpy(env.envp, envp, len * sizeof(char *));
	env.envp[len] = NULL;

	/* Free the old envp we allocated, if any. */
	sudoers_gc_remove(GC_PTR, env.old_envp);
	free(env.old_envp);
	env.old_envp = NULL;
    }

    debug_return_bool(true);
}

/*
 * Getter for private copy of the environment.
 */
char **
env_get(void)
{
    return env.envp;
}

/*
 * Swap the old and new copies of the environment.
 */
bool
env_swap_old(void)
{
    char **old_envp;

    if (env.old_envp == NULL)
	return false;
    old_envp = env.old_envp;
    env.old_envp = env.envp;
    env.envp = old_envp;
    return true;
}

/*
 * Similar to putenv(3) but operates on sudo's private copy of the
 * environment (not environ) and it always overwrites.  The dupcheck param
 * determines whether we need to verify that the variable is not already set.
 * Will only overwrite an existing variable if overwrite is set.
 * Does not include warnings or debugging to avoid recursive calls.
 */
int
sudo_putenv_nodebug(char *str, bool dupcheck, bool overwrite)
{
    char **ep;
    const char *equal;
    bool found = false;

    /* Some putenv(3) implementations check for NULL. */
    if (str == NULL) {
	errno = EINVAL;
	return -1;
    }

    /* The string must contain a '=' char but not start with one. */
    equal = strchr(str, '=');
    if (equal == NULL || equal == str) {
	errno = EINVAL;
	return -1;
    }

    /* Make sure there is room for the new entry plus a NULL. */
    if (env.env_size > 2 && env.env_len > env.env_size - 2) {
	char **nenvp;
	size_t nsize;

	if (env.env_size > SIZE_MAX - 128) {
	    sudo_warnx_nodebug(U_("internal error, %s overflow"),
		"sudo_putenv_nodebug");
	    errno = EOVERFLOW;
	    return -1;
	}
	nsize = env.env_size + 128;
	if (nsize > SIZE_MAX / sizeof(char *)) {
	    sudo_warnx_nodebug(U_("internal error, %s overflow"),
		"sudo_putenv_nodebug");
	    errno = EOVERFLOW;
	    return -1;
	}
	sudoers_gc_remove(GC_PTR, env.envp);
	nenvp = reallocarray(env.envp, nsize, sizeof(char *));
	if (nenvp == NULL) {
	    sudoers_gc_add(GC_PTR, env.envp);
	    return -1;
	}
	sudoers_gc_add(GC_PTR, nenvp);
	env.envp = nenvp;
	env.env_size = nsize;
#ifdef ENV_DEBUG
	memset(env.envp + env.env_len, 0,
	    (env.env_size - env.env_len) * sizeof(char *));
#endif
    }

#ifdef ENV_DEBUG
    if (env.envp[env.env_len] != NULL) {
	errno = EINVAL;
	return -1;
    }
#endif

    if (dupcheck) {
	size_t len = (size_t)(equal - str) + 1;
	for (ep = env.envp; *ep != NULL; ep++) {
	    if (strncmp(str, *ep, len) == 0) {
		if (overwrite)
		    *ep = str;
		found = true;
		break;
	    }
	}
	/* Prune out extra instances of the variable we just overwrote. */
	if (found && overwrite) {
	    while (*++ep != NULL) {
		if (strncmp(str, *ep, len) == 0) {
		    char **cur = ep;
		    while ((*cur = *(cur + 1)) != NULL)
			cur++;
		    ep--;
		}
	    }
	    env.env_len = (size_t)(ep - env.envp);
	}
    }

    if (!found) {
	ep = env.envp + env.env_len;
	env.env_len++;
	*ep++ = str;
	*ep = NULL;
    }
    return 0;
}

/*
 * Similar to putenv(3) but operates on sudo's private copy of the
 * environment (not environ) and it always overwrites.  The dupcheck param
 * determines whether we need to verify that the variable is not already set.
 * Will only overwrite an existing variable if overwrite is set.
 */
static int
sudo_putenv(char *str, bool dupcheck, bool overwrite)
{
    int ret;
    debug_decl(sudo_putenv, SUDOERS_DEBUG_ENV);

    sudo_debug_printf(SUDO_DEBUG_INFO, "sudo_putenv: %s", str);

    ret = sudo_putenv_nodebug(str, dupcheck, overwrite);
    if (ret == -1) {
#ifdef ENV_DEBUG
	if (env.envp[env.env_len] != NULL) {
	    sudo_warnx("%s",
		U_("sudo_putenv: corrupted envp, length mismatch"));
	}
#endif
    }
    debug_return_int(ret);
}

/*
 * Similar to setenv(3) but operates on a private copy of the environment.
 * The dupcheck param determines whether we need to verify that the variable
 * is not already set.
 */
static int
sudo_setenv2(const char *var, const char *val, bool dupcheck, bool overwrite)
{
    char *estring;
    size_t esize;
    int ret = -1;
    debug_decl(sudo_setenv2, SUDOERS_DEBUG_ENV);

    esize = strlen(var) + 1 + strlen(val) + 1;
    if ((estring = malloc(esize)) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	debug_return_int(-1);
    }

    /* Build environment string and insert it. */
    if (strlcpy(estring, var, esize) >= esize ||
	strlcat(estring, "=", esize) >= esize ||
	strlcat(estring, val, esize) >= esize) {

	sudo_warnx(U_("internal error, %s overflow"), __func__);
	errno = EOVERFLOW;
    } else {
	ret = sudo_putenv(estring, dupcheck, overwrite);
    }
    if (ret == -1)
	free(estring);
    else
	sudoers_gc_add(GC_PTR, estring);
    debug_return_int(ret);
}

/*
 * Similar to setenv(3) but operates on a private copy of the environment.
 */
int
sudo_setenv(const char *var, const char *val, int overwrite)
{
    return sudo_setenv2(var, val, true, (bool)overwrite);
}

/*
 * Similar to unsetenv(3) but operates on a private copy of the environment.
 * Does not include warnings or debugging to avoid recursive calls.
 */
int
sudo_unsetenv_nodebug(const char *var)
{
    char **ep = env.envp;
    size_t len;

    if (ep == NULL || var == NULL || *var == '\0' || strchr(var, '=') != NULL) {
	errno = EINVAL;
	return -1;
    }

    len = strlen(var);
    while (*ep != NULL) {
	if (strncmp(var, *ep, len) == 0 && (*ep)[len] == '=') {
	    /* Found it; shift remainder + NULL over by one. */
	    char **cur = ep;
	    while ((*cur = *(cur + 1)) != NULL)
		cur++;
	    env.env_len--;
	    /* Keep going, could be multiple instances of the var. */
	} else {
	    ep++;
	}
    }
    return 0;
}

/*
 * Similar to unsetenv(3) but operates on a private copy of the environment.
 */
int
sudo_unsetenv(const char *name)
{
    int ret;
    debug_decl(sudo_unsetenv, SUDOERS_DEBUG_ENV);

    sudo_debug_printf(SUDO_DEBUG_INFO, "sudo_unsetenv: %s", name);

    ret = sudo_unsetenv_nodebug(name);

    debug_return_int(ret);
}

/*
 * Similar to getenv(3) but operates on a private copy of the environment.
 * Does not include warnings or debugging to avoid recursive calls.
 */
char *
sudo_getenv_nodebug(const char *name)
{
    char **ep, *val = NULL;
    size_t namelen = 0;

    if (env.env_len != 0) {
	/* For BSD compatibility, treat '=' in name like end of string. */
	while (name[namelen] != '\0' && name[namelen] != '=')
	    namelen++;
	for (ep = env.envp; *ep != NULL; ep++) {
	    if (strncmp(*ep, name, namelen) == 0 && (*ep)[namelen] == '=') {
		val = *ep + namelen + 1;
		break;
	    }
	}
    }
    return val;
}

/*
 * Similar to getenv(3) but operates on a private copy of the environment.
 */
char *
sudo_getenv(const char *name)
{
    char *val;
    debug_decl(sudo_getenv, SUDOERS_DEBUG_ENV);

    sudo_debug_printf(SUDO_DEBUG_INFO, "sudo_getenv: %s", name);

    val = sudo_getenv_nodebug(name);

    debug_return_str(val);
}

/*
 * Check for var against patterns in the specified environment list.
 * Returns true if the variable was found, else false.
 */
static bool
matches_env_list(const char *var, struct list_members *list, bool *full_match)
{
    struct list_member *cur;
    bool is_logname = false;
    debug_decl(matches_env_list, SUDOERS_DEBUG_ENV);

    switch (*var) {
    case 'L':
	if (strncmp(var, "LOGNAME=", 8) == 0)
	    is_logname = true;
#ifdef _AIX
	else if (strncmp(var, "LOGIN=", 6) == 0)
	    is_logname = true;
#endif
	break;
    case 'U':
	if (strncmp(var, "USER=", 5) == 0)
	    is_logname = true;
	break;
    }

    if (is_logname) {
	/*
	 * We treat LOGIN, LOGNAME and USER specially.
	 * If one is preserved/deleted we want to preserve/delete them all.
	 */
	SLIST_FOREACH(cur, list, entries) {
	    if (matches_env_pattern(cur->value, "LOGNAME", full_match) ||
#ifdef _AIX
		matches_env_pattern(cur->value, "LOGIN", full_match) ||
#endif
		matches_env_pattern(cur->value, "USER", full_match))
		debug_return_bool(true);
	}
    } else {
	SLIST_FOREACH(cur, list, entries) {
	    if (matches_env_pattern(cur->value, var, full_match))
		debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

/*
 * Check the env_delete blocklist.
 * Returns true if the variable was found, else false.
 */
static bool
matches_env_delete(const char *var)
{
    bool full_match;	/* unused */
    debug_decl(matches_env_delete, SUDOERS_DEBUG_ENV);

    /* Skip anything listed in env_delete. */
    debug_return_bool(matches_env_list(var, &def_env_delete, &full_match));
}

/*
 * Verify the TZ environment variable is safe.
 * On many systems it is possible to set this to a pathname.
 */
static bool
tz_is_safe(const char *tzval)
{
    const char *cp;
    char lastch;
    debug_decl(tz_is_safe, SUDOERS_DEBUG_ENV);

    /* tzcode treats a value beginning with a ':' as a path. */
    if (tzval[0] == ':')
	tzval++;

    /* Reject fully-qualified TZ that doesn't being with the zoneinfo dir. */
    if (tzval[0] == '/') {
#ifdef _PATH_ZONEINFO
	if (strncmp(tzval, _PATH_ZONEINFO, sizeof(_PATH_ZONEINFO) - 1) != 0 ||
	    tzval[sizeof(_PATH_ZONEINFO) - 1] != '/')
	    debug_return_bool(false);
#else
	/* Assume the worst. */
	debug_return_bool(false);
#endif
    }

    /*
     * Make sure TZ only contains printable non-space characters
     * and does not contain a '..' path element.
     */
    lastch = '/';
    for (cp = tzval; *cp != '\0'; cp++) {
	if (isspace((unsigned char)*cp) || !isprint((unsigned char)*cp))
	    debug_return_bool(false);
	if (lastch == '/' && cp[0] == '.' && cp[1] == '.' &&
	    (cp[2] == '/' || cp[2] == '\0'))
	    debug_return_bool(false);
	lastch = *cp;
    }

    /* Reject extra long TZ values (even if not a path). */
    if ((size_t)(cp - tzval) >= PATH_MAX)
	debug_return_bool(false);

    debug_return_bool(true);
}

/*
 * Apply the env_check list.
 * Returns true if the variable is allowed, false if denied
 * or -1 if no match.
 */
static int
matches_env_check(const char *var, bool *full_match)
{
    int keepit = -1;
    debug_decl(matches_env_check, SUDOERS_DEBUG_ENV);

    /* Skip anything listed in env_check that includes '/' or '%'. */
    if (matches_env_list(var, &def_env_check, full_match)) {
	if (strncmp(var, "TZ=", 3) == 0) {
	    /* Special case for TZ */
	    keepit = tz_is_safe(var + 3);
	} else {
	    const char *val = strchr(var, '=');
	    if (val != NULL)
		keepit = !strpbrk(val + 1, "/%");
	}
    }
    debug_return_int(keepit);
}

/*
 * Check the env_keep list.
 * Returns true if the variable is allowed else false.
 */
static bool
matches_env_keep(const struct sudoers_context *ctx, const char *var,
    bool *full_match)
{
    bool keepit = false;
    debug_decl(matches_env_keep, SUDOERS_DEBUG_ENV);

    /* Preserve SHELL variable for "sudo -s". */
    if (ISSET(ctx->mode, MODE_SHELL) && strncmp(var, "SHELL=", 6) == 0) {
	keepit = true;
    } else if (matches_env_list(var, &def_env_keep, full_match)) {
	keepit = true;
    }
    debug_return_bool(keepit);
}

/*
 * Look up var in the env_delete and env_check.
 * Returns true if we should delete the variable, else false.
 */
static bool
env_should_delete(const char *var)
{
    int delete_it;
    bool full_match = false;
    debug_decl(env_should_delete, SUDOERS_DEBUG_ENV);

    delete_it = matches_env_delete(var);
    if (!delete_it)
	delete_it = matches_env_check(var, &full_match) == false;

    sudo_debug_printf(SUDO_DEBUG_INFO, "delete %s: %s",
	var, delete_it ? "YES" : "NO");
    debug_return_bool(delete_it);
}

/*
 * Lookup var in the env_check and env_keep lists.
 * Returns true if the variable is allowed else false.
 */
static bool
env_should_keep(const struct sudoers_context *ctx, const char *var)
{
    int keepit;
    bool full_match = false;
    const char *cp;
    debug_decl(env_should_keep, SUDOERS_DEBUG_ENV);

    keepit = matches_env_check(var, &full_match);
    if (keepit == -1)
	keepit = matches_env_keep(ctx, var, &full_match);

    /* Skip bash functions unless we matched on the value as well as name. */
    if (keepit && !full_match) {
	if ((cp = strchr(var, '=')) != NULL) {
	    if (strncmp(cp, "=() ", 4) == 0)
		keepit = false;
	}
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "keep %s: %s",
	var, keepit == true ? "YES" : "NO");
    debug_return_bool(keepit == true);
}

#ifdef HAVE_PAM
/*
 * Merge another environment with our private copy.
 * Only overwrite an existing variable if it is not
 * being preserved from the user's environment.
 * Returns true on success or false on failure.
 */
bool
env_merge(const struct sudoers_context *ctx, char * const envp[])
{
    char * const *ep;
    bool ret = true;
    debug_decl(env_merge, SUDOERS_DEBUG_ENV);

    for (ep = envp; *ep != NULL; ep++) {
	/* XXX - avoid checking value here, should only check name */
	bool overwrite = def_env_reset ? !env_should_keep(ctx, *ep) : env_should_delete(*ep);
	if (sudo_putenv(*ep, true, overwrite) == -1) {
	    /* XXX cannot undo on failure */
	    ret = false;
	    break;
	}
    }
    debug_return_bool(ret);
}
#endif /* HAVE_PAM */

static void
env_update_didvar(const char *ep, unsigned int *didvar)
{
    switch (*ep) {
	case 'H':
	    if (strncmp(ep, "HOME=", 5) == 0)
		SET(*didvar, DID_HOME);
	    break;
	case 'L':
#ifdef _AIX
	    if (strncmp(ep, "LOGIN=", 8) == 0)
		SET(*didvar, DID_LOGIN);
#endif
	    if (strncmp(ep, "LOGNAME=", 8) == 0)
		SET(*didvar, DID_LOGNAME);
	    break;
	case 'M':
	    if (strncmp(ep, "MAIL=", 5) == 0)
		SET(*didvar, DID_MAIL);
	    break;
	case 'P':
	    if (strncmp(ep, "PATH=", 5) == 0)
		SET(*didvar, DID_PATH);
	    break;
	case 'S':
	    if (strncmp(ep, "SHELL=", 6) == 0)
		SET(*didvar, DID_SHELL);
	    break;
	case 'T':
	    if (strncmp(ep, "TERM=", 5) == 0)
		SET(*didvar, DID_TERM);
	    break;
	case 'U':
	    if (strncmp(ep, "USER=", 5) == 0)
		SET(*didvar, DID_USER);
	    break;
    }
}

#define CHECK_PUTENV(a, b, c)	do {					       \
    if (sudo_putenv((char *)(a), (b), (c)) == -1) {			       \
	goto bad;							       \
    }									       \
} while (0)

#define CHECK_SETENV2(a, b, c, d)	do {				       \
    if (sudo_setenv2((char *)(a), (b), (c), (d)) == -1) {		       \
	goto bad;							       \
    }									       \
} while (0)

/*
 * Build a new environment and either clear potentially dangerous
 * variables from the old one or start with a clean slate.
 * Also adds sudo-specific variables (SUDO_*).
 * Returns true on success or false on failure.
 */
bool
rebuild_env(const struct sudoers_context *ctx)
{
    char **ep, *cp, *ps1;
    char idbuf[STRLEN_MAX_UNSIGNED(uid_t) + 1];
    unsigned int didvar;
    bool reset_home = false;
    int len;
    debug_decl(rebuild_env, SUDOERS_DEBUG_ENV);

    /*
     * Either clean out the environment or reset to a safe default.
     */
    ps1 = NULL;
    didvar = 0;
    env.env_len = 0;
    env.env_size = 128;
    sudoers_gc_remove(GC_PTR, env.old_envp);
    free(env.old_envp);
    env.old_envp = env.envp;
    env.envp = reallocarray(NULL, env.env_size, sizeof(char *));
    if (env.envp == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unable to allocate memory");
	env.env_size = 0;
	goto bad;
    }
    sudoers_gc_add(GC_PTR, env.envp);
#ifdef ENV_DEBUG
    memset(env.envp, 0, env.env_size * sizeof(char *));
#else
    env.envp[0] = NULL;
#endif

    /* Reset HOME based on target user if configured to. */
    if (ISSET(ctx->mode, MODE_RUN)) {
	if (def_always_set_home ||
	    ISSET(ctx->mode, MODE_RESET_HOME | MODE_LOGIN_SHELL) || 
	    (ISSET(ctx->mode, MODE_SHELL) && def_set_home))
	    reset_home = true;
    }

    if (def_env_reset || ISSET(ctx->mode, MODE_LOGIN_SHELL)) {
	/*
	 * If starting with a fresh environment, initialize it based on
	 * /etc/environment or login.conf.  For "sudo -i" we want those
	 * variables to override the invoking user's environment, so we
	 * defer reading them until later.
	 */
	if (!ISSET(ctx->mode, MODE_LOGIN_SHELL)) {
#ifdef HAVE_LOGIN_CAP_H
	    /* Insert login class environment variables. */
	    if (ctx->runas.class) {
		login_cap_t *lc = login_getclass(ctx->runas.class);
		if (lc != NULL) {
		    setusercontext(lc, ctx->runas.pw,
			ctx->runas.pw->pw_uid, LOGIN_SETPATH|LOGIN_SETENV);
		    login_close(lc);
		}
	    }
#endif /* HAVE_LOGIN_CAP_H */
#ifdef _PATH_ENVIRONMENT
	    /* Insert system-wide environment variables. */
	    if (!read_env_file(ctx, _PATH_ENVIRONMENT, true, false))
		sudo_warn("%s", _PATH_ENVIRONMENT);
#endif
	    for (ep = env.envp; *ep; ep++)
		env_update_didvar(*ep, &didvar);
	}

	/* Pull in vars we want to keep from the old environment. */
	if (env.old_envp != NULL) {
	    for (ep = env.old_envp; *ep; ep++) {
		bool keepit;

		/*
		 * Look up the variable in the env_check and env_keep lists.
		 */
		keepit = env_should_keep(ctx, *ep);

		/*
		 * Do SUDO_PS1 -> PS1 conversion.
		 * This must happen *after* env_should_keep() is called.
		 */
		if (strncmp(*ep, "SUDO_PS1=", 9) == 0)
		    ps1 = *ep + 5;

		if (keepit) {
		    /* Preserve variable. */
		    CHECK_PUTENV(*ep, true, false);
		    env_update_didvar(*ep, &didvar);
		}
	    }
	}
	didvar |= didvar << 16;		/* convert DID_* to KEPT_* */

	/*
	 * Add in defaults.  In -i mode these come from the runas user,
	 * otherwise they may be from the user's environment (depends
	 * on sudoers options).
	 */
	if (ISSET(ctx->mode, MODE_LOGIN_SHELL)) {
	    CHECK_SETENV2("SHELL", ctx->runas.pw->pw_shell,
		ISSET(didvar, DID_SHELL), true);
#ifdef _AIX
	    CHECK_SETENV2("LOGIN", ctx->runas.pw->pw_name,
		ISSET(didvar, DID_LOGIN), true);
#endif
	    CHECK_SETENV2("LOGNAME", ctx->runas.pw->pw_name,
		ISSET(didvar, DID_LOGNAME), true);
	    CHECK_SETENV2("USER", ctx->runas.pw->pw_name,
		ISSET(didvar, DID_USER), true);
	} else {
	    /* We will set LOGNAME later in the def_set_logname case. */
	    if (!def_set_logname) {
#ifdef _AIX
		if (!ISSET(didvar, DID_LOGIN))
		    CHECK_SETENV2("LOGIN", ctx->user.name, false, true);
#endif
		if (!ISSET(didvar, DID_LOGNAME))
		    CHECK_SETENV2("LOGNAME", ctx->user.name, false, true);
		if (!ISSET(didvar, DID_USER))
		    CHECK_SETENV2("USER", ctx->user.name, false, true);
	    }
	}

	/* If we didn't keep HOME, reset it based on target user. */
	if (!ISSET(didvar, KEPT_HOME))
	    reset_home = true;

	/*
	 * Set MAIL to target user in -i mode or if MAIL is not preserved
	 * from user's environment.
	 */
	if (ISSET(ctx->mode, MODE_LOGIN_SHELL) || !ISSET(didvar, KEPT_MAIL)) {
	    if (_PATH_MAILDIR[sizeof(_PATH_MAILDIR) - 2] == '/') {
		len = asprintf(&cp, "MAIL=%s%s", _PATH_MAILDIR,
		    ctx->runas.pw->pw_name);
	    } else {
		len = asprintf(&cp, "MAIL=%s/%s", _PATH_MAILDIR,
		    ctx->runas.pw->pw_name);
	    }
	    if (len == -1)
		    goto bad;
	    if (sudo_putenv(cp, ISSET(didvar, DID_MAIL), true) == -1) {
		free(cp);
		goto bad;
	    }
	    sudoers_gc_add(GC_PTR, cp);
	}
    } else {
	/*
	 * Copy environ entries as long as they don't match env_delete or
	 * env_check.
	 */
	if (env.old_envp != NULL) {
	    for (ep = env.old_envp; *ep; ep++) {
		/* Add variable unless it matches a blocklist. */
		if (!env_should_delete(*ep)) {
		    if (strncmp(*ep, "SUDO_PS1=", 9) == 0)
			ps1 = *ep + 5;
		    else if (strncmp(*ep, "SHELL=", 6) == 0)
			SET(didvar, DID_SHELL);
		    else if (strncmp(*ep, "PATH=", 5) == 0)
			SET(didvar, DID_PATH);
		    else if (strncmp(*ep, "TERM=", 5) == 0)
			SET(didvar, DID_TERM);
		    CHECK_PUTENV(*ep, true, false);
		}
	    }
	}
    }
    /* Replace the PATH envariable with a secure one? */
    if (def_secure_path && !user_is_exempt(ctx)) {
	CHECK_SETENV2("PATH", def_secure_path, true, true);
	SET(didvar, DID_PATH);
    }

    /*
     * Set LOGIN, LOGNAME, and USER to target if "set_logname" is not
     * disabled.  We skip this if we are running a login shell (because
     * they have already been set).
     */
    if (def_set_logname && !ISSET(ctx->mode, MODE_LOGIN_SHELL)) {
	if ((didvar & KEPT_USER_VARIABLES) == 0) {
	    /* Nothing preserved, set them all. */
#ifdef _AIX
	    CHECK_SETENV2("LOGIN", ctx->runas.pw->pw_name, true, true);
#endif
	    CHECK_SETENV2("LOGNAME", ctx->runas.pw->pw_name, true, true);
	    CHECK_SETENV2("USER", ctx->runas.pw->pw_name, true, true);
	} else if ((didvar & KEPT_USER_VARIABLES) != KEPT_USER_VARIABLES) {
	    /*
	     * Preserved some of LOGIN, LOGNAME, USER but not all.
	     * Make the unset ones match so we don't end up with some
	     * set to the invoking user and others set to the runas user.
	     */
	    if (ISSET(didvar, KEPT_LOGNAME))
		cp = sudo_getenv("LOGNAME");
#ifdef _AIX
	    else if (ISSET(didvar, KEPT_LOGIN))
		cp = sudo_getenv("LOGIN");
#endif
	    else if (ISSET(didvar, KEPT_USER))
		cp = sudo_getenv("USER");
	    else
		cp = NULL;
	    if (cp != NULL) {
#ifdef _AIX
		if (!ISSET(didvar, KEPT_LOGIN))
		    CHECK_SETENV2("LOGIN", cp, true, true);
#endif
		if (!ISSET(didvar, KEPT_LOGNAME))
		    CHECK_SETENV2("LOGNAME", cp, true, true);
		if (!ISSET(didvar, KEPT_USER))
		    CHECK_SETENV2("USER", cp, true, true);
	    }
	}
    }

    /* Set $HOME to target user if not preserving user's value. */
    if (reset_home)
	CHECK_SETENV2("HOME", ctx->runas.pw->pw_dir, true, true);

    /* Provide default values for $SHELL, $TERM and $PATH if not set. */
    if (!ISSET(didvar, DID_SHELL))
	CHECK_SETENV2("SHELL", ctx->runas.pw->pw_shell, false, false);
    if (!ISSET(didvar, DID_TERM))
	CHECK_PUTENV("TERM=unknown", false, false);
    if (!ISSET(didvar, DID_PATH))
	CHECK_SETENV2("PATH", _PATH_STDPATH, false, true);

    /* Set PS1 if SUDO_PS1 is set. */
    if (ps1 != NULL)
	CHECK_PUTENV(ps1, true, true);

    /* Add the SUDO_COMMAND envariable (cmnd + args). */
    if (ctx->user.cmnd_args) {
	/*
	 * We limit ctx->user.cmnd_args to 4096 bytes to avoid an execve(2)
	 * failure for very long argument vectors.  The command's environment
	 * also counts against the ARG_MAX limit.
	 */
	len = asprintf(&cp, "SUDO_COMMAND=%s %.*s", ctx->user.cmnd, 4096,
	    ctx->user.cmnd_args);
	if (len == -1)
	    goto bad;
	if (sudo_putenv(cp, true, true) == -1) {
	    free(cp);
	    goto bad;
	}
	sudoers_gc_add(GC_PTR, cp);
    } else {
	CHECK_SETENV2("SUDO_COMMAND", ctx->user.cmnd, true, true);
    }

    /* Add the SUDO_USER, SUDO_UID, SUDO_GID environment variables. */
    CHECK_SETENV2("SUDO_USER", ctx->user.name, true, true);
    (void)snprintf(idbuf, sizeof(idbuf), "%u", (unsigned int) ctx->user.uid);
    CHECK_SETENV2("SUDO_UID", idbuf, true, true);
    (void)snprintf(idbuf, sizeof(idbuf), "%u", (unsigned int) ctx->user.gid);
    CHECK_SETENV2("SUDO_GID", idbuf, true, true);

    debug_return_bool(true);

bad:
    sudo_warn("%s", U_("unable to rebuild the environment"));
    debug_return_bool(false);
}

/*
 * Insert all environment variables in envp into the private copy
 * of the environment.
 * Returns true on success or false on failure.
 */
bool
insert_env_vars(char * const envp[])
{
    char * const *ep;
    bool ret = true;
    debug_decl(insert_env_vars, SUDOERS_DEBUG_ENV);

    /* Add user-specified environment variables. */
    if (envp != NULL) {
	for (ep = envp; *ep != NULL; ep++) {
	    /* XXX - no undo on failure */
	    if (sudo_putenv(*ep, true, true) == -1) {
		ret = false;
		break;
	    }
	}
    }
    debug_return_bool(ret);
}

/*
 * Validate the list of environment variables passed in on the command
 * line against env_delete, env_check, and env_keep.
 * Calls log_warning() if any specified variables are not allowed.
 * Returns true if allowed, else false.
 */
bool
validate_env_vars(const struct sudoers_context *ctx, char * const env_vars[])
{
    char * const *ep;
    char errbuf[4096];
    char *errpos = errbuf;
    bool okvar, ret = true;
    debug_decl(validate_env_vars, SUDOERS_DEBUG_ENV);

    if (env_vars == NULL)
	debug_return_bool(true);	/* nothing to do */

    /* Add user-specified environment variables. */
    for (ep = env_vars; *ep != NULL; ep++) {
	char *eq = strchr(*ep, '=');
	if (eq == NULL || eq == *ep) {
	    /* Must be in the form var=val. */
	    okvar = false;
	} else if (def_secure_path && !user_is_exempt(ctx) &&
	    strncmp(*ep, "PATH=", 5) == 0) {
	    okvar = false;
	} else if (def_env_reset) {
	    okvar = env_should_keep(ctx, *ep);
	} else {
	    okvar = !env_should_delete(*ep);
	}
	if (okvar == false) {
	    /* Not allowed, append to error buffer if space remains. */
	    if (errpos < &errbuf[sizeof(errbuf)]) {
		const size_t varlen = strcspn(*ep, "=");
		const size_t errsize = sizeof(errbuf) - (size_t)(errpos - errbuf);
		int len = snprintf(errpos, errsize, "%s%.*s",
		    errpos != errbuf ? ", " : "", (int)varlen, *ep);
		if (len >= ssizeof(errbuf) - (errpos - errbuf)) {
		    memcpy(&errbuf[sizeof(errbuf) - 4], "...", 4);
		    errpos = &errbuf[sizeof(errbuf)];
		} else {
		    errpos += len;
		}
	    }
	}
    }
    if (errpos != errbuf) {
	/* XXX - audit? */
	log_warningx(ctx, 0,
	    N_("sorry, you are not allowed to set the following environment variables: %s"), errbuf);
	ret = false;
    }
    debug_return_bool(ret);
}

static void *
env_file_open_local(const char *path)
{
    struct env_file_local *efl;
    debug_decl(env_file_open_local, SUDOERS_DEBUG_ENV);

    efl = calloc(1, sizeof(*efl));
    if (efl != NULL) {
	if ((efl->fp = fopen(path, "r")) == NULL) {
	    if (errno != ENOENT) {
		free(efl);
		efl = NULL;
	    }
	}
    }
    debug_return_ptr(efl);
}

static void
env_file_close_local(void *cookie)
{
    struct env_file_local *efl = cookie;
    debug_decl(env_file_close_local, SUDOERS_DEBUG_ENV);

    if (efl != NULL) {
	if (efl->fp != NULL)
	    fclose(efl->fp);
	free(efl->line);
	free(efl);
    }
    debug_return;
}

/*
 * Parse /etc/environment lines ala AIX and Linux.
 * Lines may be in either of three formats:
 *  NAME=VALUE
 *  NAME="VALUE"
 *  NAME='VALUE'
 * with an optional "export" prefix so the shell can source the file.
 * Invalid lines, blank lines, or lines consisting solely of a comment
 * character are skipped.
 */
static char *
env_file_next_local(void *cookie, int *errnum)
{
    struct env_file_local *efl = cookie;
    char *var, *val, *ret = NULL;
    size_t var_len, val_len;
    debug_decl(env_file_next_local, SUDOERS_DEBUG_ENV);

    *errnum = 0;
    if (efl->fp == NULL)
	debug_return_ptr(NULL);

    for (;;) {
	if (sudo_parseln(&efl->line, &efl->linesize, NULL, efl->fp, PARSELN_CONT_IGN) == -1) {
	    if (!feof(efl->fp))
		*errnum = errno;
	    break;
	}

	/* Skip blank or comment lines */
	if (*(var = efl->line) == '\0')
	    continue;

	/* Skip optional "export " */
	if (strncmp(var, "export", 6) == 0 && isspace((unsigned char) var[6])) {
	    var += 7;
	    while (isspace((unsigned char) *var)) {
		var++;
	    }
	}

	/* Must be of the form name=["']value['"] */
	for (val = var; *val != '\0' && *val != '='; val++)
	    continue;
	if (var == val || *val != '=')
	    continue;
	var_len = (size_t)(val - var);
	val_len = strlen(++val);

	/* Strip leading and trailing single/double quotes */
	if ((val[0] == '\'' || val[0] == '\"') && val_len > 1 && val[0] == val[val_len - 1]) {
	    val[val_len - 1] = '\0';
	    val++;
	    val_len -= 2;
	}

	if ((ret = malloc(var_len + 1 + val_len + 1)) == NULL) {
	    *errnum = errno;
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	} else {
	    memcpy(ret, var, var_len + 1); /* includes '=' */
	    memcpy(ret + var_len + 1, val, val_len + 1); /* includes NUL */
	    sudoers_gc_add(GC_PTR, ret);
	}
	break;
    }
    debug_return_str(ret);
}

static struct sudoers_env_file env_file_sudoers = {
    env_file_open_local,
    env_file_close_local,
    env_file_next_local
};

static struct sudoers_env_file env_file_system = {
    env_file_open_local,
    env_file_close_local,
    env_file_next_local
};

void
register_env_file(void * (*ef_open)(const char *), void (*ef_close)(void *),
    char * (*ef_next)(void *, int *), bool sys)
{
    struct sudoers_env_file *ef = sys ? &env_file_system : &env_file_sudoers;

    ef->open = ef_open;
    ef->close = ef_close;
    ef->next = ef_next;
}

bool
read_env_file(const struct sudoers_context *ctx, const char *path,
    bool overwrite, bool restricted)
{
    struct sudoers_env_file *ef;
    bool ret = true;
    char *envstr;
    void *cookie;
    int errnum;
    debug_decl(read_env_file, SUDOERS_DEBUG_ENV);

    /*
     * The environment file may be handled differently depending on
     * whether it is specified in sudoers or the system.
     */
    if (path == def_env_file || path == def_restricted_env_file)
	ef = &env_file_sudoers;
    else
	ef = &env_file_system;

    cookie = ef->open(path);
    if (cookie == NULL)
	debug_return_bool(false);

    for (;;) {
	/* Keep reading until EOF or error. */
	if ((envstr = ef->next(cookie, &errnum)) == NULL) {
	    if (errnum != 0)
		ret = false;
	    break;
	}

	/*
	 * If the env file is restricted, apply env_check and env_keep
	 * when env_reset is set or env_delete when it is not.
	 */
	if (restricted) {
	    if (def_env_reset ? !env_should_keep(ctx, envstr) : env_should_delete(envstr)) {
		free(envstr);
		continue;
	    }
	}
	if (sudo_putenv(envstr, true, overwrite) == -1) {
	    /* XXX - no undo on failure */
	    ret = false;
	    break;
	}
    }
    ef->close(cookie);

    debug_return_bool(ret);
}

bool
init_envtables(void)
{
    struct list_member *cur;
    const char **p;
    debug_decl(init_envtables, SUDOERS_DEBUG_ENV);

    /* Fill in the "env_delete" list. */
    for (p = initial_badenv_table; *p; p++) {
	cur = calloc(1, sizeof(struct list_member));
	if (cur == NULL || (cur->value = strdup(*p)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    free(cur);
	    debug_return_bool(false);
	}
	SLIST_INSERT_HEAD(&def_env_delete, cur, entries);
    }

    /* Fill in the "env_check" list. */
    for (p = initial_checkenv_table; *p; p++) {
	cur = calloc(1, sizeof(struct list_member));
	if (cur == NULL || (cur->value = strdup(*p)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    free(cur);
	    debug_return_bool(false);
	}
	SLIST_INSERT_HEAD(&def_env_check, cur, entries);
    }

    /* Fill in the "env_keep" list. */
    for (p = initial_keepenv_table; *p; p++) {
	cur = calloc(1, sizeof(struct list_member));
	if (cur == NULL || (cur->value = strdup(*p)) == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"unable to allocate memory");
	    free(cur);
	    debug_return_bool(false);
	}
	SLIST_INSERT_HEAD(&def_env_keep, cur, entries);
    }
    debug_return_bool(true);
}
