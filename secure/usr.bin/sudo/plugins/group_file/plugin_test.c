/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2013 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sudo_plugin.h>

sudo_dso_public int main(int argc, char *argv[]);

/*
 * Simple driver to test sudoer group plugins.
 * usage: plugin_test [-p "plugin.so plugin_args ..."] user:group ...
 */

static void *group_handle;
static struct sudoers_group_plugin *group_plugin;

static int
plugin_printf(int msg_type, const char * restrict fmt, ...)
{
    va_list ap;
    FILE *fp;
	    
    switch (msg_type) {
    case SUDO_CONV_INFO_MSG:
	fp = stdout;
	break;
    case SUDO_CONV_ERROR_MSG:
	fp = stderr;
	break;
    default:
	errno = EINVAL;
	return -1;
    }

    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);

    return 0;
}

/*
 * Load the specified plugin and run its init function.
 * Returns -1 if unable to open the plugin, else it returns
 * the value from the plugin's init function.
 */
static int
group_plugin_load(char *plugin_info)
{
    char *args, path[PATH_MAX], savedch;
    char **argv = NULL;
    int rc;

    /*
     * Fill in .so path and split out args (if any).
     */
    if ((args = strpbrk(plugin_info, " \t")) != NULL) {
	savedch = *args;
	*args = '\0';
    }
    if (strlcpy(path, plugin_info, sizeof(path)) >= sizeof(path)) {
	fprintf(stderr, "path too long: %s\n", plugin_info);
	return -1;
    }
    if (args != NULL)
	*args++ = savedch;

    /* Open plugin and map in symbol. */
    group_handle = dlopen(path, RTLD_LAZY);
    if (!group_handle) {
	fprintf(stderr, "unable to dlopen %s: %s\n", path, dlerror());
	return -1;
    }
    group_plugin = dlsym(group_handle, "group_plugin");
    if (group_plugin == NULL) {
	fprintf(stderr, "unable to find symbol \"group_plugin\" in %s\n", path);
	return -1;
    }

    if (SUDO_API_VERSION_GET_MAJOR(group_plugin->version) != GROUP_API_VERSION_MAJOR) {
	fprintf(stderr,
	    "%s: incompatible group plugin major version %u, expected %u\n",
	    path, SUDO_API_VERSION_GET_MAJOR(group_plugin->version),
	    GROUP_API_VERSION_MAJOR);
	return -1;
    }

    /*
     * Split args into a vector if specified.
     */
    if (args != NULL) {
	int ac = 0, wasblank = 1;
	char *cp, *last;

        for (cp = args; *cp != '\0'; cp++) {
            if (isblank((unsigned char)*cp)) {
                wasblank = 1;
            } else if (wasblank) {
                wasblank = 0;
                ac++;
            }
        }
	if (ac != 0) 	{
	    argv = malloc((ac + 1) * sizeof(char *));
	    if (argv == NULL) {
		perror(NULL);
		return -1;
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

    rc = (group_plugin->init)(GROUP_API_VERSION, plugin_printf, argv);

    free(argv);

    return rc;
}

static void
group_plugin_unload(void)
{
    (group_plugin->cleanup)();
    dlclose(group_handle);
    group_handle = NULL;
}

static int
group_plugin_query(const char *user, const char *group,
    const struct passwd *pwd)
{
    return (group_plugin->query)(user, group, pwd);
}

static void
usage(void)
{
    fputs("usage: plugin_test [-p \"plugin.so plugin_args ...\"] user:group ...\n",
        stderr);
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int ch, found;
    size_t i;
    char *plugin = "group_file.so";
    char *user, *group;
    struct passwd *pwd;

    while ((ch = getopt(argc, argv, "p:")) != -1) {
	switch (ch) {
	case 'p':
	    plugin = optarg;
	    break;
	default:
	    usage();
	}
    }
    argc -= optind;
    argv += optind;

    if (argc < 1)
	usage();

    if (group_plugin_load(plugin) != 1) {
	fprintf(stderr, "unable to load plugin: %s\n", plugin);
	return EXIT_FAILURE;
    }

    for (i = 0; argv[i] != NULL; i++) {
	user = argv[i];
	group = strchr(argv[i], ':');
	if (group == NULL)
	    continue;
	*group++ = '\0';
	pwd = getpwnam(user);
	found = group_plugin_query(user, group, pwd);
	printf("user %s %s in group %s\n", user, found ? "is" : "NOT ", group);
    }
    group_plugin_unload();

    return EXIT_SUCCESS;
}

