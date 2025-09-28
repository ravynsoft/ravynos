/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2020, 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include <sudoers.h>
#include <sudo_dso.h>

#if defined(HAVE_DLOPEN) || defined(HAVE_SHL_LOAD)

static void *group_handle;
static struct sudoers_group_plugin *group_plugin;

/*
 * Check for a fallback path when the original group plugin is not loadable.
 * Returns true on success, rewriting path and filling in sb, else false.
 */
static bool
group_plugin_fallback(char *path, size_t pathsize)
{
#if defined(__LP64__)
    char newpath[PATH_MAX];
    bool ret = false;
    struct stat sb;
    int len;
    debug_decl(group_plugin_fallback, SUDOERS_DEBUG_UTIL);

# if defined(__sun__) || defined(__linux__)
    /*
     * Solaris uses /lib/64 and /usr/lib/64 for 64-bit libraries.
     * Linux may use /lib64 and /usr/lib64 for 64-bit libraries.
     * If dirname(path) ends in /lib, try /lib/64 (Solaris) or /lib64 (Linux).
     */
#  if defined(__sun__)
    const char *lib64 = "lib/64";
#  else
    const char *lib64 = "lib64";
#  endif
    const char *base, *slash;
    int dirlen;

    slash = strrchr(path, '/');
    if (slash == NULL) {
	goto done;
    }
    base = slash + 1;

    /* Collapse consecutive slashes. */
    while (slash > path && slash[-1] == '/') {
	slash--;
    }

    /* If directory ends in /lib/, try again with /lib/64/ or /lib64/. */
    dirlen = (int)(slash - path);
    if (dirlen < 4 || strncmp(slash - 4, "/lib", 4) != 0) {
	goto done;
    }
    dirlen -= 4;
    len = snprintf(newpath, sizeof(newpath), "%.*s/%s/%s", dirlen, path, lib64,
	base);
# else /* !__sun__ && !__linux__ */
    /*
     * Multilib not supported, check for a path of the form libfoo64.so.
     */
    const char *dot;
    int plen;

    dot = strrchr(path, '.');
    if (dot == NULL) {
	goto done;
    }
    plen = (int)(dot - path);

    /* If basename(path) doesn't match libfoo64.so, try adding the 64. */
    if (plen >= 2 && strncmp(dot - 2, "64", 2) == 0) {
	goto done;
    }
    len = snprintf(newpath, sizeof(newpath), "%.*s64%s", plen, path, dot);
# endif /* __sun__ || __linux__ */
    if (len < 0 || len >= ssizeof(newpath)) {
	errno = ENAMETOOLONG;
	goto done;
    }
    if (stat(newpath, &sb) == -1) {
	goto done;
    }
    if (strlcpy(path, newpath, pathsize) >= pathsize) {
	errno = ENAMETOOLONG;
	goto done;
    }
    ret = true;
done:
    debug_return_bool(ret);
#else
    return false;
#endif /* __LP64__ */
}

/*
 * Load the specified plugin and run its init function.
 * Returns -1 if unable to open the plugin, else it returns
 * the value from the plugin's init function.
 */
static int
group_plugin_load(const struct sudoers_context *ctx, const char *plugin_info)
{
    const char *plugin_dir = ctx->settings.plugin_dir;
    char *args, path[PATH_MAX];
    char **argv = NULL;
    int len, rc = -1;
    bool retry = true;
    debug_decl(group_plugin_load, SUDOERS_DEBUG_UTIL);

    /*
     * Fill in .so path and split out args (if any).
     */
    if ((args = strpbrk(plugin_info, " \t")) != NULL) {
	len = snprintf(path, sizeof(path), "%s%.*s",
	    (*plugin_info != '/') ? plugin_dir : "",
	    (int)(args - plugin_info), plugin_info);
	args++;
    } else {
	len = snprintf(path, sizeof(path), "%s%s",
	    (*plugin_info != '/') ? plugin_dir : "", plugin_info);
    }
    if (len < 0 || len >= ssizeof(path)) {
	errno = ENAMETOOLONG;
	sudo_warn("%s%s",
	    (*plugin_info != '/') ? plugin_dir : "", plugin_info);
	goto done;
    }

    for (;;) {
	group_handle = sudo_dso_load(path, SUDO_DSO_LAZY|SUDO_DSO_GLOBAL);
	if (group_handle != NULL) {
	    break;
	}

	if (!retry || !group_plugin_fallback(path, sizeof(path))) {
	    const char *errstr = sudo_dso_strerror();
	    sudo_warnx(U_("unable to load %s: %s"), path,
		errstr ? errstr : "unknown error");
	    goto done;
	}

	/* Retry once with the fallback path. */
	retry = false;
    }

    /* Map in symbol from group plugin. */
    group_plugin = sudo_dso_findsym(group_handle, "group_plugin");
    if (group_plugin == NULL) {
	sudo_warnx(U_("unable to find symbol \"group_plugin\" in %s"), path);
	goto done;
    }

    if (SUDO_API_VERSION_GET_MAJOR(group_plugin->version) != GROUP_API_VERSION_MAJOR) {
	sudo_warnx(U_("%s: incompatible group plugin major version %d, expected %d"),
	    path, SUDO_API_VERSION_GET_MAJOR(group_plugin->version),
	    GROUP_API_VERSION_MAJOR);
	goto done;
    }

    /*
     * Split args into a vector if specified.
     */
    if (args != NULL) {
	int ac = 0;
	bool wasblank = true;
	char *cp, *last;

        for (cp = args; *cp != '\0'; cp++) {
            if (isblank((unsigned char)*cp)) {
                wasblank = true;
            } else if (wasblank) {
                wasblank = false;
                ac++;
            }
        }
	if (ac != 0) {
	    argv = reallocarray(NULL, (size_t)ac + 1, sizeof(char *));
	    if (argv == NULL) {
		sudo_warnx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
		goto done;
	    }
	    ac = 0;
	    cp = strtok_r(args, " \t", &last);
	    while (cp != NULL) {
		argv[ac++] = cp;
		cp = strtok_r(NULL, " \t", &last);
	    }
	    argv[ac] = NULL;
	}
    }

    rc = (group_plugin->init)(GROUP_API_VERSION, sudo_printf, argv);

done:
    free(argv);

    if (rc != true) {
	if (group_handle != NULL) {
	    sudo_dso_unload(group_handle);
	    group_handle = NULL;
	    group_plugin = NULL;
	}
    }

    debug_return_int(rc);
}

void
group_plugin_unload(void)
{
    debug_decl(group_plugin_unload, SUDOERS_DEBUG_UTIL);

    if (group_plugin != NULL) {
	(group_plugin->cleanup)();
	group_plugin = NULL;
    }
    if (group_handle != NULL) {
	sudo_dso_unload(group_handle);
	group_handle = NULL;
    }
    debug_return;
}

int
group_plugin_query(const char *user, const char *group,
    const struct passwd *pwd)
{
    debug_decl(group_plugin_query, SUDOERS_DEBUG_UTIL);

    if (group_plugin == NULL)
	debug_return_int(false);
    debug_return_int((group_plugin->query)(user, group, pwd));
}

#else /* !HAVE_DLOPEN && !HAVE_SHL_LOAD */

/*
 * No loadable shared object support.
 */

static int
group_plugin_load(const struct sudoers_context *ctx, const char *plugin_info)
{
    debug_decl(group_plugin_load, SUDOERS_DEBUG_UTIL);
    debug_return_int(false);
}

void
group_plugin_unload(void)
{
    debug_decl(group_plugin_unload, SUDOERS_DEBUG_UTIL);
    debug_return;
}

int
group_plugin_query(const char *user, const char *group,
    const struct passwd *pwd)
{
    debug_decl(group_plugin_query, SUDOERS_DEBUG_UTIL);
    debug_return_int(false);
}

#endif /* HAVE_DLOPEN || HAVE_SHL_LOAD */

/*
 * Group plugin sudoers callback.
 */
bool
cb_group_plugin(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    bool rc = true;
    debug_decl(cb_group_plugin, SUDOERS_DEBUG_PLUGIN);

    /* Unload any existing group plugin before loading a new one. */
    group_plugin_unload();
    if (sd_un->str != NULL)
	rc = group_plugin_load(ctx, sd_un->str);
    debug_return_bool(rc);
}
